/***********************************************************************************************//**
 * \file   rd0057.c
 * \brief  RD-0057 ThunderBoard-React Board Handling
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/
#include "rd0057.h"
#include "em_system.h"
#include "em_rtcc.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "flashpwr.h"
#include "mpu6500.h"
#include <stddef.h>

/***************************************************************************************************
 * Public Variables
 **************************************************************************************************/

/***************************************************************************************************
 * Local Type Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

typedef struct {
  GPIO_Port_TypeDef   port;
  uint8_t              pin;
} tGpioArray;

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

static const tGpioArray ledArray[NO_OF_LEDS]       = { { LED0_PORT, LED0_PIN }, { LED1_PORT, LED1_PIN } };
static const tGpioArray buttonArray[NO_OF_BUTTONS] = { { BUTTON0_PORT, BUTTON0_PIN }, { BUTTON1_PORT, BUTTON1_PIN } };
static void (*buttonCb)(uint8_t, bool) = NULL;

/***************************************************************************************************
 * Global Variables
 **************************************************************************************************/

I2CSPM_Init_TypeDef i2cInit = I2CSPM_INIT_DEFAULT;

/***************************************************************************************************
 * Static Function Declarations
 **************************************************************************************************/

static void emuInit();
static void clocksInit();
static void rtccInit();
static void buttonsInit();
static void ledsInit();
static void hallInit();

/***************************************************************************************************
 * Pubic Function Definitions
 **************************************************************************************************/
void boardInit(void)
{
  emuInit();
  clocksInit();
  rtccInit();
  I2CSPM_Init(&i2cInit);
  buttonsInit();
  ledsInit();
  hallInit();

  // Initialize SPI flash and put it in lowest power mode
  flashpwrInit();
  flashpwrDeepPowerDown();

  // Initialize the accelerometers and gyros
  mpu6500_Init(i2cInit.port, MPU6500_ADDR);
}

// NOTE: Callbacks are called in interrupt context
void boardRegisterButtonCallback(void (*buttonCallback)(uint8_t, bool))
{
  buttonCb = buttonCallback;
}

void boardButtonInterrupt(uint8_t buttonNo)
{
  if (buttonCb) {
    buttonCb(buttonNo, boardButtonGet(buttonNo));
  }
}

/***************************************************************************************************
 * Static Function Definitions
 **************************************************************************************************/
static void emuInit(void)
{
  /* Initialize DCDC regulator */
  EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;

  dcdcInit.powerConfig           = emuPowerConfig_DcdcToDvdd;
  dcdcInit.dcdcMode              = emuDcdcMode_LowNoise;
  dcdcInit.mVout                 = 1800;
  dcdcInit.em01LoadCurrent_mA    = 15;
  dcdcInit.em234LoadCurrent_uA   = 10;
  dcdcInit.maxCurrent_mA         = 200;
  dcdcInit.anaPeripheralPower    = emuDcdcAnaPeripheralPower_DCDC;
  dcdcInit.reverseCurrentControl = 160;

  EMU_DCDCInit(&dcdcInit);
  /* Initialize EM2/EM3 mode */
  EMU_EM23Init_TypeDef em23Init = EMU_EM23INIT_DEFAULT;

  em23Init.em23VregFullEn        = 0;

  EMU_EM23Init(&em23Init);
  /* Initialize EM4H/S mode */
  EMU_EM4Init_TypeDef em4Init = EMU_EM4INIT_DEFAULT;

  em4Init.retainLfrco            = 0;
  em4Init.retainLfxo             = 0;
  em4Init.retainUlfrco           = 0;
  em4Init.em4State               = emuEM4Shutoff;
  em4Init.pinRetentionMode       = emuPinRetentionDisable;

  EMU_EM4Init(&em4Init);
}

static void clocksInit(void)
{
  /* Initializing HFXO */
  CMU_HFXOInit_TypeDef hfxoInit = CMU_HFXOINIT_DEFAULT;

  CMU_HFXOInit(&hfxoInit);

  /* Setting system HFXO frequency */
  SystemHFXOClockSet(38400000);

  /* Enable HFXO oscillator, and wait for it to be stable */
  CMU_OscillatorEnable(cmuOsc_HFXO, true, true);

  /* Using HFXO as high frequency clock, HFCLK */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);

  /* HFRCO not needed when using HFXO */
  CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);

  /* Set autostart behaviour */
  CMU_HFXOAutostartEnable(0, true, false);

  /* Enable clock to LE modules */
  CMU_ClockEnable(cmuClock_CORELE, true);
  /* Initializing LFXO */
  CMU_LFXOInit_TypeDef lfxoInit = CMU_LFXOINIT_DEFAULT;

  CMU_LFXOInit(&lfxoInit);

  /* Enable LFXO oscillator, and wait for it to be stable */
  CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

  /* Setting system LFXO frequency */
  SystemLFXOClockSet(32768);

  /* Select LFXO as clock source for LFxCLKs */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
  CMU_ClockSelectSet(cmuClock_LFE, cmuSelect_LFXO);

  CMU_ClockEnable(cmuClock_RTCC, true);

  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Set C-Tune value to zero for EFR32MG1P family as this is the device in the
   * BGM111A256V1 and this module has external load caps.
   * BGM111A256V1.1 and later include an EFR32BG1B and no external load caps
   * hence we use the recommended CTUNE for these parts.
   * By overriding the CTUNE setting from the hardware configurator here, the
   * compiled binary can be used by both versions.
   */
  if (systemPartFamilyMighty1P == SYSTEM_GetFamily()) {
    CMU->HFXOSTEADYSTATECTRL = (CMU->HFXOSTEADYSTATECTRL & ~_CMU_HFXOSTEADYSTATECTRL_CTUNE_MASK)
                               | 0 << _CMU_HFXOSTEADYSTATECTRL_CTUNE_SHIFT;
  } else {
    CMU->HFXOSTEADYSTATECTRL = (CMU->HFXOSTEADYSTATECTRL & ~_CMU_HFXOSTEADYSTATECTRL_CTUNE_MASK)
                               | 322 << _CMU_HFXOSTEADYSTATECTRL_CTUNE_SHIFT;
  }
}

static void rtccInit(void)
{
  RTCC_Init_TypeDef rtccInit = RTCC_INIT_DEFAULT;

  rtccInit.enable                = true;
  rtccInit.debugRun              = false;
  rtccInit.precntWrapOnCCV0      = false;
  rtccInit.cntWrapOnCCV1         = false;
  rtccInit.prescMode             = rtccCntTickPresc;
  rtccInit.presc                 = rtccCntPresc_32;
  rtccInit.enaOSCFailDetect      = false;
  rtccInit.cntMode               = rtccCntModeNormal;

  RTCC_Init(&rtccInit);
}

static void buttonsInit(void)
{
  uint8_t i;

  for (i = 0; i < NO_OF_BUTTONS; i++) {
    GPIO_PinModeSet(buttonArray[i].port,
                    buttonArray[i].pin,
                    gpioModeInput,
                    0);
  }
}

static void ledsInit(void)
{
  uint8_t i;

  for (i = 0; i < NO_OF_LEDS; i++) {
    GPIO_PinModeSet(ledArray[i].port,
                    ledArray[i].pin,
                    gpioModePushPull,
                    1);
  }
}

static void hallInit(void)
{
  // Set enable pin low to turn off hall sensor
  GPIO_PinModeSet(HALLSENSOR_ENABLE_PORT,
                  HALLSENSOR_ENABLE_PIN,
                  gpioModePushPull,
                  0);
}

void boardHallEnable(bool enable)
{
  if (enable) {
    GPIO_PinOutSet(HALLSENSOR_ENABLE_PORT, HALLSENSOR_ENABLE_PIN);
  } else {
    GPIO_PinOutClear(HALLSENSOR_ENABLE_PORT, HALLSENSOR_ENABLE_PIN);
  }
}

uint8_t boardButtonGet(uint8_t buttonNo)
{
  uint8_t retVal = 0;

  if (buttonNo < NO_OF_BUTTONS) {
    retVal = !GPIO_PinInGet(buttonArray[buttonNo].port, buttonArray[buttonNo].pin);
  }
  return retVal;
}

void boardLedOff(uint8_t ledNo)
{
  if (ledNo < NO_OF_LEDS) {
    GPIO_PinOutSet(ledArray[ledNo].port, ledArray[ledNo].pin);
  }
}

void boardLedOn(uint8_t ledNo)
{
  if (ledNo < NO_OF_LEDS) {
    GPIO_PinOutClear(ledArray[ledNo].port, ledArray[ledNo].pin);
  }
}

void boardLedToggle(uint8_t ledNo)
{
  if (ledNo < NO_OF_LEDS) {
    GPIO_PinOutToggle(ledArray[ledNo].port, ledArray[ledNo].pin);
  }
}

uint8_t boardLedGet(uint8_t ledNo)
{
  uint8_t retVal = 0;

  if (ledNo < NO_OF_LEDS) {
    retVal = (int) !GPIO_PinOutGet(ledArray[ledNo].port, ledArray[ledNo].pin);
  }
  return retVal;
}
