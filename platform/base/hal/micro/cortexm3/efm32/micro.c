// -----------------------------------------------------------------------------
// @file
// @brief EFM micro specific full HAL functions
//
// @author Silicon Laboratories Inc.
// @version 1.0.0
//
// @section License
// <b>(C) Copyright 2014 Silicon Laboratories, www.silabs.com</b>
//
// This file is licensed under the Silabs License Agreement. See the file
// "Silabs_License_Agreement.txt" for details. Before using this software for
// any purpose, you must agree to the terms of that agreement.
//
// -----------------------------------------------------------------------------
#include PLATFORM_HEADER
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_core.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_prs.h"
#include "em_cryotimer.h"
#include "rtcdriver.h"
#include "em_chip.h"
#include "gpiointerrupt.h"

#include "stack/include/ember.h"
#include "include/error.h"
#include "hal/hal.h"
#include "serial/serial.h"
#include "hal/micro/cortexm3/diagnostic.h"
#include "hal/micro/cortexm3/memmap.h"
#include "hal/micro/cortexm3/flash.h"

#ifdef _EFR_DEVICE
#include "hal/plugin/glib/graphics.h"
#ifndef PHY_RAIL
#include "pa.h"
#endif//PHY_RAIL
#include "tempdrv.h"
#include "sleep-efm32.h"
#endif

#ifdef _EZR_DEVICE
#include "ezr32.h"
#endif

#ifdef BSP_STK
#include "bsp.h"
#endif

#ifdef RTOS
  #include "rtos/rtos.h"
#endif

extern void halStackRadioHoldOffPowerDown(void); // fwd ref
extern void halStackRadioHoldOffPowerUp(void);   // fwd ref

// Declares the PA curves only if we're in RAIL
#ifdef PHY_RAIL
#include "../plugin/pa-conversions/pa_conversions_efr32.h"
#if HAL_PA_VOLTAGE == 3300
RAIL_DECLARE_TX_POWER_VBAT_CURVES(RAIL_PiecewiseSegments, RAIL_CurvesSg, RAIL_Curves24Hp, RAIL_Curves24Lp);
#else
RAIL_DECLARE_TX_POWER_DCDC_CURVES(RAIL_PiecewiseSegments, RAIL_CurvesSg, RAIL_Curves24Hp, RAIL_Curves24Lp);
#endif
#endif

#ifndef RHO_GPIO
WEAK(void halStackRadioHoldOffPowerDown(void)) {
}
WEAK(void halStackRadioHoldOffPowerUp(void)) {
}
WEAK(bool halGetRadioHoldOff(void)) {
  return false;
}
WEAK(EmberStatus halSetRadioHoldOff(bool enable)) {
  return (enable ? EMBER_BAD_ARGUMENT : EMBER_SUCCESS);
}
#endif //!RHO_GPIO

#if !defined(PTA_REQ_GPIO) && !defined(PTA_GNT_GPIO)
WEAK(HalPtaOptions halPtaGetOptions(void)) {
  return 0;
}
WEAK(EmberStatus halPtaSetOptions(HalPtaOptions options)) {
  return EMBER_ERR_FATAL;
}
WEAK(EmberStatus halPtaSetBool(HalPtaOptions option, bool value)) {
  return EMBER_ERR_FATAL;
}
WEAK(EmberStatus halPtaSetEnable(bool enable)) {
  return EMBER_ERR_FATAL;
}
WEAK(bool halPtaIsEnabled(void)) {
  return false;
}
WEAK(EmberStatus halPtaSetTxRequest(halPtaReq_t ptaReq, halPtaCb_t ptaCb)) {
  return EMBER_ERR_FATAL;
}
WEAK(EmberStatus halPtaSetRxRequest(halPtaReq_t ptaReq, halPtaCb_t ptaCb)) {
  return EMBER_ERR_FATAL;
}
WEAK(halPtaReq_t halPtaFrameDetectReq(void)) {
  return 0;
}
WEAK(halPtaReq_t halPtaFilterPassReq(void)) {
  return 0;
}
#endif //!defined(PTA_REQ_GPIO) && !defined(PTA_GNT_GPIO)

#ifdef _EFR_DEVICE
// Provide HAL pointers to board-header-defined PA configuration(s)
// for use by App, RAIL, or PHY library.
#ifdef  HAL_PA_ENABLE
static const RAIL_TxPowerConfig_t paInit2p4 =
{
#if HAL_PA_2P4_LOWPOWER
  .mode = RAIL_TX_POWER_MODE_2P4_LP, /* Power Amplifier mode */
#else
  .mode = RAIL_TX_POWER_MODE_2P4_HP, /* Power Amplifier mode */
#endif
  .voltage = HAL_PA_VOLTAGE,         /* Power Amplifier vPA Voltage mode */
  .rampTime = HAL_PA_RAMP,           /* Desired ramp time in us */
};
const RAIL_TxPowerConfig_t* halInternalPa2p4GHzInit = &paInit2p4;
static const RAIL_TxPowerConfig_t paInitSub =
{
  .mode = RAIL_TX_POWER_MODE_SUBGIG, /* Power Amplifier mode */
  .voltage = HAL_PA_VOLTAGE,         /* Power Amplifier vPA Voltage mode */
  .rampTime = HAL_PA_RAMP,           /* Desired ramp time in us */
};
const RAIL_TxPowerConfig_t* halInternalPaSubGHzInit = &paInitSub;
#else//!HAL_PA_ENABLE
const RAIL_TxPowerConfig_t* halInternalPa2p4GHzInit = NULL;
const RAIL_TxPowerConfig_t* halInternalPaSubGHzInit = NULL;
#endif//HAL_PA_ENABLE
#endif// _EFR_DEVICE

#if defined(_EZR_DEVICE) || HAL_EZRADIOPRO_ENABLE
#if BSP_EZRADIOPRO_USART == HAL_SPI_PORT_USART0
#define PRO2_USART USART0
#elif BSP_EZRADIOPRO_USART == HAL_SPI_PORT_USART1
#define PRO2_USART USART1
#elif BSP_EZRADIOPRO_USART == HAL_SPI_PORT_USART2
#define PRO2_USART USART2
#elif BSP_EZRADIOPRO_USART == HAL_SPI_PORT_USART3
#define PRO2_USART USART3
#else
#error "Invalid EZRADIOPRO USART"
#endif
const uint8_t pro2SpiClockMHz = HAL_EZRADIOPRO_FREQ / 1000000;
#ifdef  _EFR_DEVICE
#include "spidrv.h"
const SPIDRV_Init_t pro2SpiConfig = {
  .port             = PRO2_USART,
  .portLocationTx   = BSP_EZRADIOPRO_MOSI_LOC,
  .portLocationRx   = BSP_EZRADIOPRO_MISO_LOC,
  .portLocationClk  = BSP_EZRADIOPRO_CLK_LOC,
  .portLocationCs   = 0, //not used by application
  .bitRate          = HAL_EZRADIOPRO_FREQ,
/**** Below fields should NOT be modified by customers ****/
  .frameLength      = 8,
  .dummyTxValue     = 0xFF,
  .type             = spidrvMaster,
  .bitOrder         = spidrvBitOrderMsbFirst,
  .clockMode        = spidrvClockMode0,
  .csControl        = spidrvCsControlApplication,
  .slaveStartMode   = spidrvSlaveStartImmediate,
};
#endif//_EFR_DEVICE
#endif

// halInit is called on first initial boot, not on wakeup from sleep.
void halInit(void)
{
  /* Configure board. Select either EBI or SPI mode. */
  CHIP_Init();
#ifdef _EFR_DEVICE
  EMU_UnlatchPinRetention();
#endif
  halConfigInit();
  RTCDRV_Init();
#ifdef _EFR_DEVICE
  #ifndef HAL_CONFIG
  halInternalInitBoard();
  #endif
  TEMPDRV_Init();
  EMU_EM4Init_TypeDef em4Init = EMU_EM4INIT_DEFAULT;
  em4Init.em4State = emuEM4Hibernate;
  EMU_EM4Init(&em4Init);
  halInternalEm4Wakeup();
#elif defined(_EZR_DEVICE)
  halInternalStartSymbolTimer();      // TODO: move to macInit or emRadioInit
#endif

  halInternalStartSystemTimer();

#ifdef PHY_RAIL
  RAIL_TxPowerCurvesConfig_t txPowerCurvesConfig = { RAIL_Curves24Hp, RAIL_CurvesSg, RAIL_Curves24Lp, RAIL_PiecewiseSegments };
  RAIL_InitTxPowerCurves(&txPowerCurvesConfig);
#endif
}

void halReboot(void)
{
  halInternalSysReset(RESET_SOFTWARE_REBOOT);
}

void halPowerDown(void)
{
  #if HAL_EZRADIOPRO_SHUTDOWN_SLEEP
  extern void emRadioPowerDown(void);
  emRadioPowerDown();
  #endif
  #ifndef HAL_CONFIG
  halInternalPowerDownBoard();
  #endif
  halConfigPowerDownGpio();
}

// halPowerUp is called from sleep state, not from first initial boot.
void halPowerUp(void)
{
  halConfigPowerUpGpio();
  #ifndef HAL_CONFIG
  halInternalPowerUpBoard();
  #endif
  #if HAL_EZRADIOPRO_SHUTDOWN_SLEEP
  extern void emRadioPowerUp(void);
  emRadioPowerUp();
  #endif
}

void halStackRadioPowerDownBoard(void)
{
  (void) halPtaSetRxRequest(PTA_REQ_OFF, NULL);
  // Don't touch PTA Tx Request here
  halStackRadioHoldOffPowerDown();
}

void halStackRadio2PowerDownBoard(void)
{
  // Neither PTA nor RHO are supported on Radio2
}

void halStackRadioPowerUpBoard(void)
{
  halStackRadioHoldOffPowerUp();
}

void halStackRadio2PowerUpBoard(void)
{
  // Neither PTA nor RHO are supported on Radio2
}

void halStackRadioPowerMainControl(bool powerUp)
{
  if (powerUp) {
    halRadioPowerUpHandler();
  } else {
    halRadioPowerDownHandler();
  }
}

void halStackProcessBootCount(void)
{
  //Note: Because this always counts up at every boot (called from emberInit),
  //and non-volatile storage has a finite number of write cycles, this will
  //eventually stop working.  Disable this token call if non-volatile write
  //cycles need to be used sparingly.
#if defined(CREATOR_STACK_BOOT_COUNTER)
  halCommonIncrementCounterToken(TOKEN_STACK_BOOT_COUNTER);
#endif
}

PGM_P halGetResetString(void)
{
  // Table used to convert from reset types to reset strings.
  #define RESET_BASE_DEF(basename, value, string)  string,
  #define RESET_EXT_DEF(basename, extname, extvalue, string)     /*nothing*/
  static PGM char resetStringTable[][4] = {
    #include "reset-def.h"
  };
  #undef RESET_BASE_DEF
  #undef RESET_EXT_DEF
  uint8_t resetInfo = halGetResetInfo();
  if (resetInfo >= (sizeof(resetStringTable) / sizeof(resetStringTable[0]))) {
    return resetStringTable[0x00];   // return unknown
  } else {
    return resetStringTable[resetInfo];
  }
}

// Note that this API should be used in conjunction with halGetResetString
//  to get the full information, as this API does not provide a string for
//  the base reset type
PGM_P halGetExtendedResetString(void)
{
  // Create a table of reset strings for each extended reset type
  typedef PGM char ResetStringTableType[][4];
  #define RESET_BASE_DEF(basename, value, string) \
  }; static ResetStringTableType basename##ResetStringTable = {
  #define RESET_EXT_DEF(basename, extname, extvalue, string)  string,
  {
    #include "reset-def.h"
  };
  #undef RESET_BASE_DEF
  #undef RESET_EXT_DEF

  // Create a table of pointers to each of the above tables
  #define RESET_BASE_DEF(basename, value, string)  (ResetStringTableType *)basename##ResetStringTable,
  #define RESET_EXT_DEF(basename, extname, extvalue, string)     /*nothing*/
  static ResetStringTableType * PGM extendedResetStringTablePtrs[] = {
    #include "reset-def.h"
  };
  #undef RESET_BASE_DEF
  #undef RESET_EXT_DEF

  uint16_t extResetInfo = halGetExtendedResetInfo();
  // access the particular table of extended strings we are interested in
  ResetStringTableType *extendedResetStringTable =
    extendedResetStringTablePtrs[RESET_BASE_TYPE(extResetInfo)];

  // return the string from within the proper table
  return (*extendedResetStringTable)[((extResetInfo) & 0xFF)];
}

// Translate EM3xx reset codes to the codes previously used by the EM2xx.
// If there is no corresponding code, return the EM3xx base code with bit 7 set.
uint8_t halGetEm2xxResetInfo(void)
{
  uint8_t reset = halGetResetInfo();

  // Any reset with an extended value field of zero is considered an unknown
  // reset, except for FIB resets.
  if ((RESET_EXTENDED_FIELD(halGetExtendedResetInfo()) == 0)
      && (reset != RESET_FIB)) {
    return EM2XX_RESET_UNKNOWN;
  }

  switch (reset) {
    case RESET_UNKNOWN:
      return EM2XX_RESET_UNKNOWN;
    case RESET_BOOTLOADER:
      return EM2XX_RESET_BOOTLOADER;
    case RESET_EXTERNAL:    // map pin resets to poweron for EM2xx compatibility
//    return EM2XX_RESET_EXTERNAL;
    case RESET_POWERON:
      return EM2XX_RESET_POWERON;
    case RESET_WATCHDOG:
      return EM2XX_RESET_WATCHDOG;
    case RESET_SOFTWARE:
      return EM2XX_RESET_SOFTWARE;
    case RESET_CRASH:
      return EM2XX_RESET_ASSERT;
    default:
      return (reset | 0x80);    // set B7 for all other reset codes
  }
}

#if defined(CRYOTIMER_PRESENT) && (CRYOTIMER_COUNT == 1)
static uint32_t closestPowerOfTwo(uint32_t duration)
{
  //for duration 0 , this would return 0
  if (duration > 0u) {
    duration--;
    duration |= duration >> 1;
    duration |= duration >> 2;
    duration |= duration >> 4;
    duration |= duration >> 8;
    duration |= duration >> 16;
    duration++;
  }
  return duration;
}

void setEm4WakeupTimer(uint32_t duration)
{
  CRYOTIMER_Enable(false);
  CRYOTIMER_Init_TypeDef cryotimerInit = CRYOTIMER_INIT_DEFAULT;
  cryotimerInit.em4Wakeup = true;
  cryotimerInit.osc = cryotimerOscULFRCO;
  cryotimerInit.period = closestPowerOfTwo(duration);

  CMU_ClockEnable(cmuClock_CORELE, true);
  CMU_ClockEnable(cmuClock_CRYOTIMER, true);

  CRYOTIMER_IntClear(CRYOTIMER_IFC_PERIOD);
  CRYOTIMER_Init(&cryotimerInit);
  CRYOTIMER_Enable(true);
}

void halCommonWriteRtccRam(uint8_t index, void* data, uint8_t len)
{
  // for now we always assume it is an integer we write
  uint32_t *ram = (uint32_t *) data;
  RTCC->RET[index].REG = *ram;
}

void halCommonReadRtccRam(uint8_t index, void* data, uint8_t len)
{
  // for now we always assume it is an integer we read
  uint32_t *ram = (uint32_t *) data;
  *ram = RTCC->RET[index].REG;
}

/*
   1. write the outgoing nwk counter , incoming parent framecounter into rtcc ram
   2. set wakeup timer
 */
void halBeforeEM4(uint32_t duration, RTCCRamData input)
{
  //read the outgoing NWK counter and write it into rtcc ram
  //the first RTCC register is used for outgoing nwk counter
  // and the second one could be used for incoming nwk counter
  halCommonWriteRtccRam(0, &input.outgoingNwkFrameCounter, 4);
  halCommonWriteRtccRam(1, &input.incomingParentNwkFrameCounter, 4);
  halCommonWriteRtccRam(2, &input.outgoingLinkKeyFrameCounter, 4);
  halCommonWriteRtccRam(3, &input.incomingLinkKeyFrameCounter, 4);
  //set the wakeup timer
  setEm4WakeupTimer(duration);
}

/*
   1. read the outgoing nwk counter, incoming parent framecounter from rtcc ram
 */
RTCCRamData halAfterEM4(void)
{
  RTCCRamData output;
  output.outgoingNwkFrameCounter = 0;
  output.incomingParentNwkFrameCounter = 0;
  uint8_t index = 0;
  halCommonReadRtccRam(index, &output.outgoingNwkFrameCounter, 4);
  index++;
  halCommonReadRtccRam(index, &output.incomingParentNwkFrameCounter, 4);
  index++;
  halCommonReadRtccRam(index, &output.outgoingLinkKeyFrameCounter, 4);
  index++;
  halCommonReadRtccRam(index, &output.incomingLinkKeyFrameCounter, 4);
  return output;
}
#endif //defined(CRYOTIMER_PRESENT) && (CRYOTIMER_COUNT == 1)
