/***************************************************************************//**
 * @file button.c
 * @brief Button definitions
 *******************************************************************************
 * # License
 * <b>Copyright 2016 Silicon Laboratories, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silicon Labs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/
#include <stdbool.h>
#include "bg_types.h"
#include "gpiointerrupt.h"
#include "em_gpio.h"
#include "native_gecko.h"
#include "InitDevice.h"
#include "button.h"

/***************************************************************************//**
 * @defgroup Button Buttons
 * @{
 * @brief Buttons driver
 ******************************************************************************/

/***************************************************************************//**
 * @defgroup Button_Functions Button Functions
 * @{
 * @brief Button driver and support functions
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

static void buttonIrqGpioHandler(uint8_t pin);
static void buttonEnableIRQ(bool enable);
static void buttonConfig(void);

/** @endcond DO_NOT_INCLUDE_WITH_DOXYGEN */

/***************************************************************************//**
 * @brief
 *    Initializes button GPIOs and IRQ
 *
 * @return
 *    None
 ******************************************************************************/
void BUTTON_Init(void)
{
  buttonConfig();
  buttonEnableIRQ(true);
}

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/***************************************************************************//**
 * @brief
 *    Emits external signal depending on released button.
 *
 * @param[in] pin
 *    Pin number for released button
 *
 * @return
 *    None
 ******************************************************************************/
static void buttonIrqGpioHandler(uint8_t pin)
{
  switch (pin) {
    case BUTTON_SW1_PIN:
      gecko_external_signal(EXTSIG_BUTTON_SW1_RELEASED);
      break;
    case BUTTON_SW2_PIN:
      gecko_external_signal(EXTSIG_BUTTON_SW2_RELEASED);
      break;
    default:
      break;
  }
}

/***************************************************************************//**
 * @brief
 *    Enable/Disable button IRQ.
 *
 * @param[in] enable
 *    Enable/Disable interrupts.
 *
 * @return
 *    None
 ******************************************************************************/
static void buttonEnableIRQ(bool enable)
{
  GPIOINT_Init();

  GPIOINT_CallbackRegister(BUTTON_SW1_PIN, buttonIrqGpioHandler);
  GPIOINT_CallbackRegister(BUTTON_SW2_PIN, buttonIrqGpioHandler);

  GPIO_IntConfig(gpioPortD, BUTTON_SW1_PIN, false, true, enable);
  GPIO_IntConfig(gpioPortD, BUTTON_SW2_PIN, false, true, enable);
}

/***************************************************************************//**
 * @brief
 *    Button GPIO configuration
 *
 * @return
 *    None
 ******************************************************************************/
static void buttonConfig(void)
{
  GPIO_PinModeSet(BUTTON_SW1_PORT, BUTTON_SW1_PIN, gpioModeInput, 0);
  GPIO_PinModeSet(BUTTON_SW2_PORT, BUTTON_SW2_PIN, gpioModeInput, 0);
}

/** @endcond DO_NOT_INCLUDE_WITH_DOXYGEN */

/** @} {end defgroup Button_Functions} */

/** @} {end defgroup Button} */
