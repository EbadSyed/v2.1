/***************************************************************************//**
 * @file mic_config.h
 * @brief SPK0838HT4H-B MEMS Microphone configuration file
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

#ifndef MIC_CONFIG_H
#define MIC_CONFIG_H

// -----------------------------------------------------------------------------
// Global defines

#define MIC_ENABLE_PORT       gpioPortD
#define MIC_ENABLE_PIN        0

#define MIC_PORT_DATA         gpioPortI
#define MIC_PIN_DATA          13
#define MIC_PORT_CLK          gpioPortI
#define MIC_PIN_CLK           14
#define MIC_PORT_WS           gpioPortI
#define MIC_PIN_WS            15

#define MIC_USART             USART3
#define MIC_USART_CLK         cmuClock_USART3

#define MIC_USART_LOC_DATA    USART_ROUTELOC0_RXLOC_LOC5
#define MIC_USART_LOC_CLK     USART_ROUTELOC0_CLKLOC_LOC5
#define MIC_USART_LOC_WS      USART_ROUTELOC0_CSLOC_LOC5

#define MIC_DMA_LEFT_SIGNAL   ldmaPeripheralSignal_USART3_RXDATAV
#define MIC_DMA_RIGHT_SIGNAL  ldmaPeripheralSignal_USART3_RXDATAVRIGHT

#endif // MIC_CONFIG_H
