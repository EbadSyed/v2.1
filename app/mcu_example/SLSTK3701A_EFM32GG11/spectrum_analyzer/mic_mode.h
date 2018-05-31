/***************************************************************************//**
 * @file mic_mode.h
 * @brief microphone input functions for Spectrum Analyzer
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

#ifndef SRC_MICMODE_H_
#define SRC_MICMODE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * @brief Configure and start stereo microphone on USART3
 *
 * @param sampleFrequency is the desired sampling frequency of the microphone
 * @returns void
 *
 * @detail Initializes USART3 and GPIO pins to communicate with the STK
 * microphone codec over I2S
 *****************************************************************************/
void MICMODE_InitMIC(uint32_t sampleFrequency);

/**************************************************************************//**
 * @brief Configure and start DMA
 *
 * @param bufferA is the 'Ping' buffer to transfer ADC data to
 * @param bufferB is the 'Pong' buffer to transfer ADC data to
 * @param size is the length of the buffers
 *
 * @detail Initializes the DMA for tranfer from the USART3 RX register to
 * memory in a Ping-Pong type transfer. The DMA also is configured to read
 * only left microphone data, but clear and discard right microphone data from
 * the RX buffer. The DMA generates an interrupt after each buffer filled event
 *****************************************************************************/
void MICMODE_InitLDMA(uint32_t *bufferA, uint32_t *bufferB, uint32_t size);

#ifdef __cplusplus
// *INDENT-OFF*
}
// *INDENT-ON*
#endif

#endif /* SRC_MICMODE_H_ */
