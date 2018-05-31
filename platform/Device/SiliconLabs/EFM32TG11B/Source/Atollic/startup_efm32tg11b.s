/**************************************************************************//**
 *  File        : startup_efm32x.s
 *
 *  Abstract    : This file contains interrupt vector and startup code.
 *
 *  Functions   : Reset_Handler
 *
 *  Target      : Silicon Labs EFM32TG11B devices.
 *
 *  Environment : Atollic TrueSTUDIO(R)
 *
 *  Distribution: The file is distributed "as is," without any warranty
 *                of any kind.
 *
 *  (c)Copyright Atollic AB.
 *  You may use this file as-is or modify it according to the needs of your
 *  project. This file may only be built (assembled or compiled and linked)
 *  using the Atollic TrueSTUDIO(R) product. The use of this file together
 *  with other tools than Atollic TrueSTUDIO(R) is not permitted.
 *
 *******************************************************************************
 * Silicon Labs release version
 * @version 5.3.5
 ******************************************************************************/
  .syntax unified
  .thumb

  .global Reset_Handler
  .global InterruptVector
  .global Default_Handler

  /* Linker script definitions */
  /* start address for the initialization values of the .data section */
  .word _sidata
  /* start address for the .data section */
  .word _sdata
  /* end address for the .data section */
  .word _edata
  /* start address for the .bss section */
  .word _sbss
  /* end address for the .bss section */
  .word _ebss

/**
**===========================================================================
**  Program - Reset_Handler
**  Abstract: This code gets called after reset.
**===========================================================================
*/
  .section  .text.Reset_Handler,"ax", %progbits
  .type Reset_Handler, %function
Reset_Handler:
  /* Set stack pointer */
  ldr   r0,=_estack
  mov   sp, r0

  /* Branch to SystemInit function */
  bl    SystemInit

  /* Copy data initialization values */
  ldr   r1,=_sidata
  ldr   r2,=_sdata
  ldr   r3,=_edata

  subs  r3, r2
  ble   CopyLoopEnd
CopyLoop:
  subs  r3, #4
  ldr   r0, [r1, r3]
  str   r0, [r2, r3]
  bgt   CopyLoop
CopyLoopEnd:

  /* Clear BSS section */
  movs  r0, #0
  ldr   r2,=_sbss
  ldr   r3,=_ebss

  subs  r3, r2
  ble   ClearLoopEnd
ClearLoop:
  subs  r3, #4
  str   r0, [r2, r3]
  bgt   ClearLoop
ClearLoopEnd:

  /* Call static constructors */
  bl    __libc_init_array

  /* Branch to main */
  bl    main

  /* If main returns, branch to Default_Handler. */
  b     Default_Handler

  .size  Reset_Handler, .-Reset_Handler

/**
**===========================================================================
**  Program - Default_Handler
**  Abstract: This code gets called when the processor receives an
**    unexpected interrupt.
**===========================================================================
*/
  .section  .text.Default_Handler,"ax", %progbits
Default_Handler:
  b  Default_Handler

  .size  Default_Handler, .-Default_Handler

/**
**===========================================================================
**  Interrupt vector table
**===========================================================================
*/
  .section .isr_vector,"a", %progbits
InterruptVector:
  .word _estack                   /* 0 - Stack pointer */
  .word Reset_Handler             /* 1 - Reset */
  .word NMI_Handler               /* 2 - NMI  */
  .word HardFault_Handler         /* 3 - Hard fault */
  .word 0                         /* 4 - Reserved */
  .word 0                         /* 5 - Reserved */
  .word 0                         /* 6 - Reserved */
  .word 0                         /* 7 - Reserved */
  .word 0                         /* 8 - Reserved */
  .word 0                         /* 9 - Reserved */
  .word 0                         /* 10 - Reserved */
  .word SVC_Handler               /* 11 - SVCall */
  .word 0                         /* 12 - Reserved */
  .word 0                         /* 13 - Reserved */
  .word PendSV_Handler            /* 14 - PendSV */
  .word SysTick_Handler           /* 15 - Systick */

  /* External Interrupts */

  .word   EMU_IRQHandler      /* 0 - EMU */
  .word   WDOG0_IRQHandler      /* 1 - WDOG0 */
  .word   LDMA_IRQHandler      /* 2 - LDMA */
  .word   GPIO_EVEN_IRQHandler      /* 3 - GPIO_EVEN */
  .word   SMU_IRQHandler      /* 4 - SMU */
  .word   TIMER0_IRQHandler      /* 5 - TIMER0 */
  .word   USART0_IRQHandler      /* 6 - USART0 */
  .word   ACMP0_IRQHandler      /* 7 - ACMP0 */
  .word   ADC0_IRQHandler      /* 8 - ADC0 */
  .word   I2C0_IRQHandler      /* 9 - I2C0 */
  .word   I2C1_IRQHandler      /* 10 - I2C1 */
  .word   GPIO_ODD_IRQHandler      /* 11 - GPIO_ODD */
  .word   TIMER1_IRQHandler      /* 12 - TIMER1 */
  .word   USART1_IRQHandler      /* 13 - USART1 */
  .word   USART2_IRQHandler      /* 14 - USART2 */
  .word   UART0_IRQHandler      /* 15 - UART0 */
  .word   LEUART0_IRQHandler      /* 16 - LEUART0 */
  .word   LETIMER0_IRQHandler      /* 17 - LETIMER0 */
  .word   PCNT0_IRQHandler      /* 18 - PCNT0 */
  .word   RTCC_IRQHandler      /* 19 - RTCC */
  .word   CMU_IRQHandler      /* 20 - CMU */
  .word   MSC_IRQHandler      /* 21 - MSC */
  .word   CRYPTO0_IRQHandler      /* 22 - CRYPTO0 */
  .word   CRYOTIMER_IRQHandler      /* 23 - CRYOTIMER */
  .word   USART3_IRQHandler      /* 24 - USART3 */
  .word   WTIMER0_IRQHandler      /* 25 - WTIMER0 */
  .word   WTIMER1_IRQHandler      /* 26 - WTIMER1 */
  .word   VDAC0_IRQHandler      /* 27 - VDAC0 */
  .word   CSEN_IRQHandler      /* 28 - CSEN */
  .word   LESENSE_IRQHandler      /* 29 - LESENSE */
  .word   LCD_IRQHandler      /* 30 - LCD */
  .word   CAN0_IRQHandler      /* 31 - CAN0 */


/**
**===========================================================================
**  Weak interrupt handlers redirected to Default_Handler. These can be
**  overridden in user code.
**===========================================================================
*/
  .weak NMI_Handler
  .thumb_set NMI_Handler, Default_Handler

  .weak HardFault_Handler
  .thumb_set HardFault_Handler, Default_Handler

  .weak SVC_Handler
  .thumb_set SVC_Handler, Default_Handler

  .weak PendSV_Handler
  .thumb_set PendSV_Handler, Default_Handler

  .weak SysTick_Handler
  .thumb_set SysTick_Handler, Default_Handler


  .weak       EMU_IRQHandler
  .thumb_set  EMU_IRQHandler, Default_Handler

  .weak       WDOG0_IRQHandler
  .thumb_set  WDOG0_IRQHandler, Default_Handler

  .weak       LDMA_IRQHandler
  .thumb_set  LDMA_IRQHandler, Default_Handler

  .weak       GPIO_EVEN_IRQHandler
  .thumb_set  GPIO_EVEN_IRQHandler, Default_Handler

  .weak       SMU_IRQHandler
  .thumb_set  SMU_IRQHandler, Default_Handler

  .weak       TIMER0_IRQHandler
  .thumb_set  TIMER0_IRQHandler, Default_Handler

  .weak       USART0_IRQHandler
  .thumb_set  USART0_IRQHandler, Default_Handler

  .weak       ACMP0_IRQHandler
  .thumb_set  ACMP0_IRQHandler, Default_Handler

  .weak       ADC0_IRQHandler
  .thumb_set  ADC0_IRQHandler, Default_Handler

  .weak       I2C0_IRQHandler
  .thumb_set  I2C0_IRQHandler, Default_Handler

  .weak       I2C1_IRQHandler
  .thumb_set  I2C1_IRQHandler, Default_Handler

  .weak       GPIO_ODD_IRQHandler
  .thumb_set  GPIO_ODD_IRQHandler, Default_Handler

  .weak       TIMER1_IRQHandler
  .thumb_set  TIMER1_IRQHandler, Default_Handler

  .weak       USART1_IRQHandler
  .thumb_set  USART1_IRQHandler, Default_Handler

  .weak       USART2_IRQHandler
  .thumb_set  USART2_IRQHandler, Default_Handler

  .weak       UART0_IRQHandler
  .thumb_set  UART0_IRQHandler, Default_Handler

  .weak       LEUART0_IRQHandler
  .thumb_set  LEUART0_IRQHandler, Default_Handler

  .weak       LETIMER0_IRQHandler
  .thumb_set  LETIMER0_IRQHandler, Default_Handler

  .weak       PCNT0_IRQHandler
  .thumb_set  PCNT0_IRQHandler, Default_Handler

  .weak       RTCC_IRQHandler
  .thumb_set  RTCC_IRQHandler, Default_Handler

  .weak       CMU_IRQHandler
  .thumb_set  CMU_IRQHandler, Default_Handler

  .weak       MSC_IRQHandler
  .thumb_set  MSC_IRQHandler, Default_Handler

  .weak       CRYPTO0_IRQHandler
  .thumb_set  CRYPTO0_IRQHandler, Default_Handler

  .weak       CRYOTIMER_IRQHandler
  .thumb_set  CRYOTIMER_IRQHandler, Default_Handler

  .weak       USART3_IRQHandler
  .thumb_set  USART3_IRQHandler, Default_Handler

  .weak       WTIMER0_IRQHandler
  .thumb_set  WTIMER0_IRQHandler, Default_Handler

  .weak       WTIMER1_IRQHandler
  .thumb_set  WTIMER1_IRQHandler, Default_Handler

  .weak       VDAC0_IRQHandler
  .thumb_set  VDAC0_IRQHandler, Default_Handler

  .weak       CSEN_IRQHandler
  .thumb_set  CSEN_IRQHandler, Default_Handler

  .weak       LESENSE_IRQHandler
  .thumb_set  LESENSE_IRQHandler, Default_Handler

  .weak       LCD_IRQHandler
  .thumb_set  LCD_IRQHandler, Default_Handler

  .weak       CAN0_IRQHandler
  .thumb_set  CAN0_IRQHandler, Default_Handler


  .end
