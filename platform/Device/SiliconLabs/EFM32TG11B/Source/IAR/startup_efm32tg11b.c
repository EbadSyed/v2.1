/**************************************************************************//**
 * @file startup_efm32tg11b.c
 * @brief CMSIS Compatible EFM32TG11B startup file in C for IAR EWARM
 * @version 5.3.5
 ******************************************************************************
 * # License
 * <b>Copyright 2017 Silicon Laboratories, Inc. www.silabs.com</b>
 ******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.@n
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.@n
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Laboratories, Inc.
 * has no obligation to support this Software. Silicon Laboratories, Inc. is
 * providing the Software "AS IS", with no express or implied warranties of any
 * kind, including, but not limited to, any implied warranties of
 * merchantability or fitness for any particular purpose or warranties against
 * infringement of any proprietary rights of a third party.
 *
 * Silicon Laboratories, Inc. will not be liable for any consequential,
 * incidental, or special damages, or any other relief, or for any claim by
 * any third party, arising from your use of this Software.
 *
 *****************************************************************************/

#include "em_device.h"        /* The correct device header file. */

#pragma language=extended
#pragma segment="CSTACK"

/* IAR start function */
extern void __iar_program_start(void);
/* CMSIS init function */
extern void SystemInit(void);

/* Auto defined by linker */
extern unsigned char CSTACK$$Limit;

__weak void Reset_Handler(void)
{
  SystemInit();
  __iar_program_start();
}

__weak void NMI_Handler(void)
{
  while (1)
    ;
}

__weak void HardFault_Handler(void)
{
  while (1)
    ;
}

__weak void SVC_Handler(void)
{
  while (1)
    ;
}

__weak void PendSV_Handler(void)
{
  while (1)
    ;
}

__weak void SysTick_Handler(void)
{
  while (1)
    ;
}

__weak void EMU_IRQHandler(void)
{
  while (1)
    ;
}

__weak void WDOG0_IRQHandler(void)
{
  while (1)
    ;
}

__weak void LDMA_IRQHandler(void)
{
  while (1)
    ;
}

__weak void GPIO_EVEN_IRQHandler(void)
{
  while (1)
    ;
}

__weak void SMU_IRQHandler(void)
{
  while (1)
    ;
}

__weak void TIMER0_IRQHandler(void)
{
  while (1)
    ;
}

__weak void USART0_IRQHandler(void)
{
  while (1)
    ;
}

__weak void ACMP0_IRQHandler(void)
{
  while (1)
    ;
}

__weak void ADC0_IRQHandler(void)
{
  while (1)
    ;
}

__weak void I2C0_IRQHandler(void)
{
  while (1)
    ;
}

__weak void I2C1_IRQHandler(void)
{
  while (1)
    ;
}

__weak void GPIO_ODD_IRQHandler(void)
{
  while (1)
    ;
}

__weak void TIMER1_IRQHandler(void)
{
  while (1)
    ;
}

__weak void USART1_IRQHandler(void)
{
  while (1)
    ;
}

__weak void USART2_IRQHandler(void)
{
  while (1)
    ;
}

__weak void UART0_IRQHandler(void)
{
  while (1)
    ;
}

__weak void LEUART0_IRQHandler(void)
{
  while (1)
    ;
}

__weak void LETIMER0_IRQHandler(void)
{
  while (1)
    ;
}

__weak void PCNT0_IRQHandler(void)
{
  while (1)
    ;
}

__weak void RTCC_IRQHandler(void)
{
  while (1)
    ;
}

__weak void CMU_IRQHandler(void)
{
  while (1)
    ;
}

__weak void MSC_IRQHandler(void)
{
  while (1)
    ;
}

__weak void CRYPTO0_IRQHandler(void)
{
  while (1)
    ;
}

__weak void CRYOTIMER_IRQHandler(void)
{
  while (1)
    ;
}

__weak void USART3_IRQHandler(void)
{
  while (1)
    ;
}

__weak void WTIMER0_IRQHandler(void)
{
  while (1)
    ;
}

__weak void WTIMER1_IRQHandler(void)
{
  while (1)
    ;
}

__weak void VDAC0_IRQHandler(void)
{
  while (1)
    ;
}

__weak void CSEN_IRQHandler(void)
{
  while (1)
    ;
}

__weak void LESENSE_IRQHandler(void)
{
  while (1)
    ;
}

__weak void LCD_IRQHandler(void)
{
  while (1)
    ;
}

__weak void CAN0_IRQHandler(void)
{
  while (1)
    ;
}

/* With IAR, the CSTACK is defined via project options settings */
#pragma data_alignment=256
#pragma location = ".intvec"
const void * const __vector_table[] = {
  &CSTACK$$Limit,
  (void *) Reset_Handler,           /*  1 - Reset (start instruction) */
  (void *) NMI_Handler,             /*  2 - NMI */
  (void *) HardFault_Handler,       /*  3 - HardFault */
  (void *) 0,
  (void *) 0,
  (void *) 0,
  (void *) 0,
  (void *) 0,
  (void *) 0,
  (void *) 0,
  (void *) SVC_Handler,
  (void *) 0,
  (void *) 0,
  (void *) PendSV_Handler,
  (void *) SysTick_Handler,
  (void *) EMU_IRQHandler,       /* 0 - EMU */
  (void *) WDOG0_IRQHandler,     /* 1 - WDOG0 */
  (void *) LDMA_IRQHandler,      /* 2 - LDMA */
  (void *) GPIO_EVEN_IRQHandler, /* 3 - GPIO_EVEN */
  (void *) SMU_IRQHandler,       /* 4 - SMU */
  (void *) TIMER0_IRQHandler,    /* 5 - TIMER0 */
  (void *) USART0_IRQHandler,    /* 6 - USART0 */
  (void *) ACMP0_IRQHandler,     /* 7 - ACMP0 */
  (void *) ADC0_IRQHandler,      /* 8 - ADC0 */
  (void *) I2C0_IRQHandler,      /* 9 - I2C0 */
  (void *) I2C1_IRQHandler,      /* 10 - I2C1 */
  (void *) GPIO_ODD_IRQHandler,  /* 11 - GPIO_ODD */
  (void *) TIMER1_IRQHandler,    /* 12 - TIMER1 */
  (void *) USART1_IRQHandler,    /* 13 - USART1 */
  (void *) USART2_IRQHandler,    /* 14 - USART2 */
  (void *) UART0_IRQHandler,     /* 15 - UART0 */
  (void *) LEUART0_IRQHandler,   /* 16 - LEUART0 */
  (void *) LETIMER0_IRQHandler,  /* 17 - LETIMER0 */
  (void *) PCNT0_IRQHandler,     /* 18 - PCNT0 */
  (void *) RTCC_IRQHandler,      /* 19 - RTCC */
  (void *) CMU_IRQHandler,       /* 20 - CMU */
  (void *) MSC_IRQHandler,       /* 21 - MSC */
  (void *) CRYPTO0_IRQHandler,   /* 22 - CRYPTO0 */
  (void *) CRYOTIMER_IRQHandler, /* 23 - CRYOTIMER */
  (void *) USART3_IRQHandler,    /* 24 - USART3 */
  (void *) WTIMER0_IRQHandler,   /* 25 - WTIMER0 */
  (void *) WTIMER1_IRQHandler,   /* 26 - WTIMER1 */
  (void *) VDAC0_IRQHandler,     /* 27 - VDAC0 */
  (void *) CSEN_IRQHandler,      /* 28 - CSEN */
  (void *) LESENSE_IRQHandler,   /* 29 - LESENSE */
  (void *) LCD_IRQHandler,       /* 30 - LCD */
  (void *) CAN0_IRQHandler,      /* 31 - CAN0 */
};
