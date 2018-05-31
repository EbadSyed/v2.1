/***************************************************************************//**
 * @file adc_mode.h
 * @brief ADC input functions for Spectrum Analyzer
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

#ifndef SRC_ADCMODE_H_
#define SRC_ADCMODE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * @brief ADC initialization
 *
 * @param sampleFreq is the desired sampling freqency of the ADC
 *
 * @detail Initializes the ADC to continously convert 12 bit samples from
 * GPIO Pin PB11 at the desired sample frequency
 *****************************************************************************/
void ADCMODE_InitADC(uint32_t sampleFreq);

/**************************************************************************//**
 * @brief Configure and start DMA
 *
 * @param bufferA is the 'Ping' buffer to transfer ADC data to
 * @param bufferB is the 'Pong' buffer to transfer ADC data to
 * @param size is the length of the buffers
 * @returns void
 *
 * @detail Initializes the DMA to transfer ADC samples to memory in a
 * Ping-Pong type transfer. Generates an interrupt on each buffer full event
 *****************************************************************************/
void ADCMODE_InitLDMA(uint32_t *bufferA, uint32_t *bufferB, uint32_t size);

#ifdef __cplusplus
// *INDENT-OFF*
}
// *INDENT-ON*
#endif

#endif /* SRC_ADCMODE_H_ */
