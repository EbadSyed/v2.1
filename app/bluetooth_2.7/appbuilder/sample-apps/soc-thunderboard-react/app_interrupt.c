/***********************************************************************************************//**
 * \file   soc-thunderboard-react/app_interrupt.c
 * \brief  Application Interrupt
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/
#include "native_gecko.h"
#include "app_interrupt.h"
#include "rd0057.h"
#include "em_gpio.h"
#include "csc_device.h"
#include "accori_device.h"
#include "aluv_device.h"

/***********************************************************************************************//**
 * @addtogroup Thunderboard
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app_interrupt
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

#define BUTTON0_INTERRUPT_NO    0
#define BUTTON1_INTERRUPT_NO    4
#define HALLSENSOR_INTERRUPT_NO 1
#define ACCGYRO_INTERRUPT_NO    5
#define ALUV_INTERRUPT_NO       6

/***************************************************************************************************
 * Type definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Global Variables
 **************************************************************************************************/

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

/***************************************************************************************************
 * Local Function Definitions
 **************************************************************************************************/
static void gpioIrqHandler(void)
{
  uint32_t intFlags = GPIO_IntGet();
  GPIO_IntClear(intFlags);

  // Send external signal to stack to indicate that an interrupt has occurred, which will trigger
  // an external signal event from the stack in non-interrupt context. This is needed as
  // longer tasks and stack API calls (except for gecko_external signal) should not be called in
  // interrupt context.
  if (intFlags & 1 << HALLSENSOR_INTERRUPT_NO) {
    gecko_external_signal(EXTSIGNAL_HALL);
  }

  if (intFlags & 1 << ACCGYRO_INTERRUPT_NO) {
    gecko_external_signal(EXTSIGNAL_ACCGYRO);
  }

  if (intFlags & 1 << BUTTON0_INTERRUPT_NO) {
    gecko_external_signal(EXTSIGNAL_BUTTON0);
  }

  if (intFlags & 1 << BUTTON1_INTERRUPT_NO) {
    gecko_external_signal(EXTSIGNAL_BUTTON1);
  }

  if (intFlags & 1 << ALUV_INTERRUPT_NO) {
    gecko_external_signal(EXTSIGNAL_ALUV);
  }
}

/***************************************************************************************************
 * Public Function Definitions
 **************************************************************************************************/
void appInterruptInit(void)
{
  // Interrupt functionality is implemented using register writes as emlib
  // currently does not support EXTIPINSELx configuration, which we must use
  // as Button 0 and the hall sensor both use pin number 3.
  //
  // Button 0     A3 (interrupt 0)
  // Button 1     F7 (interrupt 4)
  // Hall sensor  F3 (interrupt 1)
  // AccGyro      F4 (interrupt 5)
  // Light        F5 (interrupt 6)

  // Interrupt pin for the si1133 chip. Use pull-up on pin.
  GPIO_PinModeSet(ALUV_INT_PORT, ALUV_INT_PIN, gpioModeInputPull, true);

  // Accelerometer and gyro interrupt pin
  GPIO_PinModeSet(ACCGYRO_INT_PORT, ACCGYRO_INT_PIN, gpioModeInput, false);

  // Hall sensor interrupt pin
  GPIO_PinModeSet(HALLSENSOR_OUT_PORT, HALLSENSOR_OUT_PIN, gpioModeInputPull, false);

  // Select ports for interrupts
  GPIO->EXTIPSELL = GPIO_EXTIPSELL_EXTIPSEL0_PORTA
                    | GPIO_EXTIPSELL_EXTIPSEL4_PORTF
                    | GPIO_EXTIPSELL_EXTIPSEL1_PORTF
                    | GPIO_EXTIPSELL_EXTIPSEL5_PORTF
                    | GPIO_EXTIPSELL_EXTIPSEL6_PORTF;

  // Select which pin numbers to use for each interrupt number
  GPIO->EXTIPINSELL = (GPIO->EXTIPINSELL & ~_GPIO_EXTIPINSELL_EXTIPINSEL0_MASK)
                      | GPIO_EXTIPINSELL_EXTIPINSEL0_PIN3;
  GPIO->EXTIPINSELL = (GPIO->EXTIPINSELL & ~_GPIO_EXTIPINSELL_EXTIPINSEL4_MASK)
                      | GPIO_EXTIPINSELL_EXTIPINSEL4_PIN7;
  GPIO->EXTIPINSELL = (GPIO->EXTIPINSELL & ~_GPIO_EXTIPINSELL_EXTIPINSEL1_MASK)
                      | GPIO_EXTIPINSELL_EXTIPINSEL1_PIN3;
  GPIO->EXTIPINSELL = (GPIO->EXTIPINSELL & ~_GPIO_EXTIPINSELL_EXTIPINSEL5_MASK)
                      | GPIO_EXTIPINSELL_EXTIPINSEL5_PIN4;
  GPIO->EXTIPINSELL = (GPIO->EXTIPINSELL & ~_GPIO_EXTIPINSELL_EXTIPINSEL6_MASK)
                      | GPIO_EXTIPINSELL_EXTIPINSEL6_PIN5;

  // Enable rising and/or falling edge interrupts
  GPIO->EXTIRISE = 1 << BUTTON0_INTERRUPT_NO
                   | 1 << BUTTON1_INTERRUPT_NO;
  GPIO->EXTIFALL = 1 << BUTTON0_INTERRUPT_NO
                   | 1 << BUTTON1_INTERRUPT_NO
                   | 1 << ACCGYRO_INTERRUPT_NO
                   | 1 << HALLSENSOR_INTERRUPT_NO
                   | 1 << ALUV_INTERRUPT_NO;

  // Clear interrupt flags before enabling them
  GPIO_IntClear(1 << BUTTON0_INTERRUPT_NO
                | 1 << BUTTON1_INTERRUPT_NO
                | 1 << HALLSENSOR_INTERRUPT_NO
                | 1 << ACCGYRO_INTERRUPT_NO
                | 1 << ALUV_INTERRUPT_NO);

  // Enable interrupt flags
  GPIO_IntEnable(1 << BUTTON0_INTERRUPT_NO
                 | 1 << BUTTON1_INTERRUPT_NO
                 | 1 << HALLSENSOR_INTERRUPT_NO
                 | 1 << ACCGYRO_INTERRUPT_NO
                 | 1 << ALUV_INTERRUPT_NO);

  // Enable both GPIO IRQs in the NVIC
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

/***************************************************************************************************
 * Interrupt Handlers
 **************************************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  // Common GPIO interrupt handler
  gpioIrqHandler();
}

void GPIO_EVEN_IRQHandler(void)
{
  // Common GPIO interrupt handler
  gpioIrqHandler();
}

/** @} (end addtogroup app_interrupt) */
/** @} (end addtogroup Thunderboard) */
