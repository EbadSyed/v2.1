/***************************************************************************//**
 * @file lcd.c
 * @brief LCD controller demo
 * @version 5.3.5
 *******************************************************************************
 * # License
 * <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "bsp.h"
#include "segmentlcd.h"

volatile uint32_t msTicks; /* counts 1ms timeTicks */

/* Locatl prototypes */
void Delay(uint32_t dlyTicks);

/***************************************************************************//**
 * @brief SysTick_Handler
 *   Interrupt Service Routine for system tick counter
 * @note
 *   No wrap around protection
 ******************************************************************************/
void SysTick_Handler(void)
{
  msTicks++;       /* increment counter necessary in Delay()*/
}

/***************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 ******************************************************************************/
void Delay(uint32_t dlyTicks)
{
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) ;
}

/***************************************************************************//**
 * @brief  Main function
 ******************************************************************************/
int main(void)
{
  int i;

  /* Chip errata */
  CHIP_Init();

  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) {
    while (1) ;
  }

  /* Enable LCD without voltage boost */
  SegmentLCD_Init(false);

  /* Infinite loop with test pattern. */
  while (1) {
    /* Enable all segments */
    SegmentLCD_AllOn();
    Delay(1000);

    /* Disable all segments */
    SegmentLCD_AllOff();

    /* Write some text */
    SegmentLCD_Write("Silicon");
    Delay(500);
    SegmentLCD_Write("Labs");
    Delay(500);
    SegmentLCD_Write("EFM TG11");
    Delay(1000);

    /* Write some numbers */
    for (i = 0; i < 10; i++) {
      SegmentLCD_Number(i * 111111);
      Delay(200);
    }

    SegmentLCD_LowerNumber(12345678);
    Delay(1000);
    SegmentLCD_LowerNumber(-1234567);
    Delay(1000);
    SegmentLCD_AlphaNumberOff();

    /* Test segments */
    SegmentLCD_AllOff();
    SegmentLCD_Symbol(LCD_SYMBOL_GECKO, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_EFM32, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_COL1, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_COL2, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DEGC, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DEGF, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C1, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C2, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C3, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C4, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C5, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C6, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C7, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C8, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C9, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C10, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C11, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C12, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C13, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C14, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C15, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C16, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C17, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C18, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_C19, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_S2, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_S3, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_S4, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_S5, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_S6, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_S7, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_S8, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_S9, 1);
    Delay(1000);

    for (i = 0; i < 35; i++) {
      SegmentLCD_Array(i, true);
      Delay(50);
    }
    Delay(1000);
  }
}
