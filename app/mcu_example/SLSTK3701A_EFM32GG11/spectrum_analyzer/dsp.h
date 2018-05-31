/***************************************************************************//**
 * @file dsp.h
 * @brief DSP functions for Spectrum Analyzer
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

#ifndef SRC_DSP_H_
#define SRC_DSP_H_

// N samples for N point FFT
// The desired FFT length. Valid entries are 128, 512, 2048
#define FFT_SIZE 512

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * @brief Initialize CMSIS ARM FFT instance
 *
 * @returns void
 *
 * @detail Initializes the CMSIS rfft and cfft instances at the desired size
 *****************************************************************************/
void DSP_InitFFT();

/**************************************************************************//**
 * @brief Perform FFT on time domain data and find greatest frequency bin
 *
 * @param timeBuffer is a pointer to the buffer of time domain data
 * @param freqByffer is a pointer to the buffer where frequency domain data
 * will be stored
 * @returns floating point value of the greatest frequency bin magnitude
 *
 * @detail Performs an N point fft on time domain data as defined by the
 * initialization parameters, the frequency domain data is stored and the
 * function returns the greatest magnitude frequency bin.
 *
 *****************************************************************************/
float DSP_AnalyzeSpectrum(float *timeBuffer, float *freqBuffer);

#ifdef __cplusplus
// *INDENT-OFF*
}
// *INDENT-ON*
#endif

#endif /* SRC_DSP_H_ */
