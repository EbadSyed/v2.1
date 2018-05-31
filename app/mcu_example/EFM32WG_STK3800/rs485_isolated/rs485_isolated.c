/**************************************************************************//**
* @file rs485_isolated.c
* @brief RS485 Isolated Example
* @version 5.3.5
******************************************************************************
* @section License
* <b>Copyright 2015 Silicon Labs, Inc. http://www.silabs.com</b>
*******************************************************************************
*
* This file is licensed under the Silabs License Agreement. See the file
* "Silabs_License_Agreement.txt" for details. Before using this software for
* any purpose, you must agree to the terms of that agreement.
*
******************************************************************************/
#include <stdio.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_leuart.h"
#include "retargetserial.h"
#include "segmentlcd.h"
#include "bspconfig.h"

#define RS485_WRITE_EN_PORT         gpioPortD
#define RS485_WRITE_EN_PIN          6

#define RS485_WRITE_EN_SETUP_MS     5

static volatile uint8_t chara = 'A';

static volatile bool sendChara = true;

static volatile uint32_t msTicks; // counts 1ms timeTicks

static void Delay(uint32_t dlyTicks);

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void SysTick_Handler(void)
{
  msTicks++;       // increment counter necessary in Delay()
}

/**************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 *****************************************************************************/
static void Delay(uint32_t dlyTicks)
{
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) {
  }
}

/**************************************************************************//**
 * @brief Setup GPIO
 *****************************************************************************/
static void gpioSetup(void)
{
  // Configure pins for push buttons as inputs
  GPIO_PinModeSet(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, gpioModeInput, 0);
  GPIO_PinModeSet(BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN, gpioModeInput, 0);

  // Set falling edge interrupt for both ports
  GPIO_IntConfig(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, false, true, true);
  GPIO_IntConfig(BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN, false, true, true);

  // Enable interrupt in core for even and odd gpio interrupts
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);

  // Configure write enable pin as output
  GPIO_PinModeSet(RS485_WRITE_EN_PORT, RS485_WRITE_EN_PIN, gpioModePushPull, 0);
}

/**************************************************************************//**
 * @brief Send one character over RS485
 *****************************************************************************/
static void rs485SendChara(uint8_t chara)
{
  int rxChara = -1;

  // Send character over RS485
  GPIO_PinOutSet(RS485_WRITE_EN_PORT, RS485_WRITE_EN_PIN);

  // Wait after write enable before sending data
  Delay(RS485_WRITE_EN_SETUP_MS);

  putchar(chara);

  // Wait until data is sent. As all data sent is received we just wait for
  // the same character to be received
  while (rxChara != chara) {
    rxChara = getchar();
  }

  GPIO_PinOutClear(RS485_WRITE_EN_PORT, RS485_WRITE_EN_PIN);
}

/**************************************************************************//**
 * @brief Common GPIO IRQ handler for both odd and even numbered interrupts
 *****************************************************************************/
static void commonGpioIrqHandler(void)
{
  uint32_t intFlags = GPIO_IntGet();
  GPIO_IntClear(intFlags);

  // Change character according to button pushed
  if (intFlags & (1 << BSP_GPIO_PB0_PIN)) {
    chara--;
  }
  if (intFlags & (1 << BSP_GPIO_PB1_PIN)) {
    chara++;
  }

  // Check if character is out of range
  if (chara > 'Z') {
    chara = 'A';
    putchar('\n'); // New line when wrapping the alphabet
  }

  if (chara < 'A') {
    chara = 'Z';
  }

  sendChara = true;
}

/**************************************************************************//**
 * @brief GPIO Interrupt handler for odd numbered pins
 *****************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  commonGpioIrqHandler();
}

/**************************************************************************//**
 * @brief GPIO Interrupt handler for even numbered pins
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  commonGpioIrqHandler();
}

/**************************************************************************//**
 * @brief Main
 *****************************************************************************/
int main(void)
{
  uint8_t rxChara;
  char lcdString[10];

  // Chip errata
  CHIP_Init();

  // Setup SysTick Timer for 1 msec interrupts
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_HF) / 1000)) {
    while (1) {
    }
  }

  // Initialize UART and map LF to CRLF
  RETARGET_SerialInit();
  RETARGET_SerialCrLf(1);

  gpioSetup();

  SegmentLCD_Init(false);

  while (1) {
    // Retrieve new character and print to LCD
    rxChara = getchar();

    // Convert lower case to upper case
    if ((rxChara >= 'a') && (rxChara <= 'z')) {
      rxChara = rxChara - 32;
    }

    // Print upper case characters to LCD
    if ((rxChara >= 'A') && (rxChara <= 'Z')) {
      chara = rxChara;

      sprintf(lcdString, "IN %c", chara);
      SegmentLCD_Write(lcdString);
    }

    // Send character if button has been pushed
    if (sendChara) {
      sendChara = false;

      rs485SendChara(chara);

      // Print character on LCD
      sprintf(lcdString, "OUT %c", chara);
      SegmentLCD_Write(lcdString);
    }
  }
}