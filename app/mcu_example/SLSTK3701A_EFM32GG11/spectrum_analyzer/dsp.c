/**************************************************************************//**
 * @file dsp.c
 * @brief DSP functions for Spectrum Analyzer
 * @version 5.3.5
 ******************************************************************************
 * # License
 * <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
 ******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 *****************************************************************************/
#include "arm_math.h"
#include "em_core.h"
#include "dsp.h"
#include <math.h>

/// Instance structures for float32_t RFFT
static arm_rfft_instance_f32 rfft_instance;

/// Instance structure for float32_t CFFT used by the RFFT
static arm_cfft_radix4_instance_f32 cfft_instance;

// Temporary buffer to store complex frequency data
static float tempBuffer[2 * FFT_SIZE];

/**************************************************************************//**
 * @brief Initialize CMSIS ARM FFT instance
 *****************************************************************************/
void DSP_InitFFT()

{
  // Initialize CMSIS FFT instance structures
  arm_rfft_init_f32(&rfft_instance, &cfft_instance, FFT_SIZE, 0, 1);
}

/**************************************************************************//**
 * @brief Perform FFT and extract signal frequency content. Returns maximum
 * magnitude
 *****************************************************************************/
float DSP_AnalyzeSpectrum(float *timeBuffer, float *freqBuffer)
{
  // Maximum frequency bin
  float maxval;

  // Temporary pointer to principal frequency of bin
  uint32_t* principalIndex = 0;

  // Rectangular window on data
  timeBuffer[0] = 0;
  timeBuffer[FFT_SIZE - 1] = 0;

  // Perform FFT and extract real magnitude
  arm_rfft_f32(&rfft_instance, timeBuffer, tempBuffer);
  arm_cmplx_mag_f32(tempBuffer, freqBuffer, FFT_SIZE);

  // Remove DC component for visualization
  freqBuffer[0] = 0;

  // Calculate highest magnitude bin
  arm_max_f32(freqBuffer, 128, &maxval, principalIndex);

  // Return highest magnitude bin
  return maxval;
}
