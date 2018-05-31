/***************************************************************************//**
 * @file dtm_usart.c
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

#include "em_usart.h"
#include "em_cmu.h"
#include "hal-config.h"

void dtmUsart_Init(void)
{
  USART_InitAsync_TypeDef initasync = USART_INITASYNC_DEFAULT;

  initasync.enable               = usartDisable;
  initasync.baudrate             = 115200;
  initasync.databits             = usartDatabits8;
  initasync.parity               = usartNoParity;
  initasync.stopbits             = usartStopbits1;
  initasync.oversampling         = usartOVS16;
  #if defined(USART_INPUT_RXPRS) && defined(USART_CTRL_MVDIS)
  initasync.mvdis                = 0;
  initasync.prsRxEnable          = 0;
  initasync.prsRxCh              = 0;
  #endif

  USART_InitAsync(BSP_EXP_USART, &initasync);
  USART_PrsTriggerInit_TypeDef initprs = USART_INITPRSTRIGGER_DEFAULT;

  initprs.rxTriggerEnable        = 0;
  initprs.txTriggerEnable        = 0;
  initprs.prsTriggerChannel      = usartPrsTriggerCh0;

  USART_InitPrsTrigger(BSP_EXP_USART, &initprs);
  /* Disable CLK pin */
  BSP_EXP_USART->ROUTELOC0  = (BSP_EXP_USART->ROUTELOC0 & (~_USART_ROUTELOC0_CLKLOC_MASK)) | USART_ROUTELOC0_CLKLOC_LOC0;
  BSP_EXP_USART->ROUTEPEN    = BSP_EXP_USART->ROUTEPEN & (~USART_ROUTEPEN_CLKPEN);
  /* Disable CS pin */
  BSP_EXP_USART->ROUTELOC0  = (BSP_EXP_USART->ROUTELOC0 & (~_USART_ROUTELOC0_CSLOC_MASK)) | USART_ROUTELOC0_CSLOC_LOC0;
  BSP_EXP_USART->ROUTEPEN    = BSP_EXP_USART->ROUTEPEN & (~USART_ROUTEPEN_CSPEN);
  /* Disable CTS pin */
  BSP_EXP_USART->ROUTELOC1  = (BSP_EXP_USART->ROUTELOC1 & (~_USART_ROUTELOC1_CTSLOC_MASK)) | (((uint32_t)BSP_EXP_USART_CTS_LOC) << _USART_ROUTELOC1_CTSLOC_SHIFT);
  BSP_EXP_USART->ROUTEPEN    = BSP_EXP_USART->ROUTEPEN & (~USART_ROUTEPEN_CTSPEN);
  /* Disable RTS pin */
  BSP_EXP_USART->ROUTELOC1  = (BSP_EXP_USART->ROUTELOC1 & (~_USART_ROUTELOC1_RTSLOC_MASK)) | (((uint32_t)BSP_EXP_USART_RTS_LOC) << _USART_ROUTELOC1_RTSLOC_SHIFT);
  BSP_EXP_USART->ROUTEPEN    = BSP_EXP_USART->ROUTEPEN & (~USART_ROUTEPEN_RTSPEN);
  /* Set up RX pin */
  BSP_EXP_USART->ROUTELOC0  = (BSP_EXP_USART->ROUTELOC0 & (~_USART_ROUTELOC0_RXLOC_MASK)) | (((uint32_t)BSP_EXP_USART_RX_LOC) << _USART_ROUTELOC0_RXLOC_SHIFT);
  BSP_EXP_USART->ROUTEPEN    = BSP_EXP_USART->ROUTEPEN | USART_ROUTEPEN_RXPEN;
  /* Set up TX pin */
  BSP_EXP_USART->ROUTELOC0  = (BSP_EXP_USART->ROUTELOC0 & (~_USART_ROUTELOC0_TXLOC_MASK)) | (((uint32_t)BSP_EXP_USART_TX_LOC) << _USART_ROUTELOC0_TXLOC_SHIFT);
  BSP_EXP_USART->ROUTEPEN    = BSP_EXP_USART->ROUTEPEN | USART_ROUTEPEN_TXPEN;

  /* Disable CTS */
  BSP_EXP_USART->CTRLX   = BSP_EXP_USART->CTRLX & (~USART_CTRLX_CTSEN);
  /* Set CTS active low */
  BSP_EXP_USART->CTRLX   = BSP_EXP_USART->CTRLX & (~USART_CTRLX_CTSINV);
  /* Set RTS active low */
  BSP_EXP_USART->CTRLX   = BSP_EXP_USART->CTRLX & (~USART_CTRLX_RTSINV);
  /* Set CS active low */
  BSP_EXP_USART->CTRL    = BSP_EXP_USART->CTRL & (~USART_CTRL_CSINV);
  /* Set TX active high */
  BSP_EXP_USART->CTRL    = BSP_EXP_USART->CTRL & (~USART_CTRL_TXINV);
  /* Set RX active high */
  BSP_EXP_USART->CTRL    = BSP_EXP_USART->CTRL & (~USART_CTRL_RXINV);

  /* Enable USART if opted by user */
  USART_Enable(BSP_EXP_USART, usartEnable);

  /* GPIO configurations */
  GPIO_PinModeSet(BSP_EXP_USART_TX_PORT, BSP_EXP_USART_TX_PIN, gpioModePushPull, 0);
  GPIO_PinModeSet(BSP_EXP_USART_RX_PORT, BSP_EXP_USART_RX_PIN, gpioModeInput, 0);
}
