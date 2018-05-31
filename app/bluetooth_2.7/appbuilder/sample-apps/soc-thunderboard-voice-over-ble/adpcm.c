/***********************************************************************************************//**
 * @file   adpcm.c
 * @brief  ADPCM coder
 *******************************************************************************
 * # License
 * <b>Copyright 2016 Silicon Laboratories, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silicon Labs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include "bg_types.h"
#include "adpcm.h"

/***************************************************************************//**
 * @defgroup ADPCM ADPCM - Coder
 * @{
 * @brief ADPCM IMA coder
 ******************************************************************************/

/***************************************************************************//**
 * @defgroup ADPCM_Locals ADPCM coder Local Variables
 * @{
 * @brief ADPCM coder local variables
 ******************************************************************************/

static const int8_t adpcmIndexAdjust[16] = {
  -1, -1, -1, -1, 2, 4, 6, 8, /* +0 - +3 => decrease, +4 - +7 => increase step */
  -1, -1, -1, -1, 2, 4, 6, 8  /* -0 - -3 => decrease, -4 - -7 => increase step */
};

static const int16_t adpcmStepTable[89] = {
  7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
  50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230,
  253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963,
  1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
  3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493,
  10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086,
  29794, 32767
};

static adpcm_t adpcm[AUDIO_CHANNEL_MONO];          /**< Structure for encoded audio data */

/** @} {end defgroup ADPCM_Locals} */
/** @endcond DO_NOT_INCLUDE_WITH_DOXYGEN */

/***************************************************************************//**
 * @defgroup ADPCM_Functions ADPCM code Functions
 * @{
 * @brief ADPCM coder functions
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
static uint8_t imaEncodeSample(int16_t sample, adpcm_state_t *adpcmState);

/** @endcond DO_NOT_INCLUDE_WITH_DOXYGEN */

/***************************************************************************//**
 * @brief
 *    Initialize ADPCM state.
 *
 * @return
 *    None
 ******************************************************************************/
void ADPCM_init(void)
{
  adpcm_t *padpcm = adpcm;
  padpcm->comprStateBegin.predictedSample = 0;
  padpcm->comprStateBegin.step = 0;
  padpcm->comprStateCurrent.predictedSample = 0;
  padpcm->comprStateCurrent.step = 0;
  padpcm->payload_size = ADPCM_PAYLOAD_SIZE;
  memset(padpcm->payload, 0, padpcm->payload_size);
  padpcm->payloadIndex = 0;
}

/***************************************************************************//**
 * @brief
 *    Encode buffer.
 *
 * @param[in] buffer
 *    Buffer to be encoded.
 *
 * @param[in] buffer_len
 *     Buffer length.
 *
 * @return
 *    Compressed data.
 ******************************************************************************/
adpcm_t *ADPCM_ima_encodeBuffer(int16_t *buffer, uint16_t buffer_len)
{
  adpcm_t *pADPCM = adpcm;
  uint8_t compressedNibble = 0;
  uint8_t compressedNibbleCount = 0;

  /* Copy IMA ADPCM state */
  memcpy(&pADPCM->comprStateBegin, &pADPCM->comprStateCurrent, sizeof(adpcm_state_t));
  pADPCM->payloadIndex = 0;

  for (int i = 0; i < buffer_len; i++) {
    /* Use IMA_ADPCM compression from 16-bit to 4-bit nibble */
    compressedNibble <<= 4;              /* Prepare for next nibble */
    compressedNibble |= imaEncodeSample(buffer[i], &pADPCM->comprStateCurrent);

    compressedNibbleCount++;             /* Update nibble counter */
    if (compressedNibbleCount < 2) {     /* Both nibbles in byte */
      continue;                          /* No, get next nibble */
    }
    compressedNibbleCount = 0;           /* Reset nibble counter */

    /* Save IDM_ADPCM byte (two compressed samples) to audio buffer */
    pADPCM->payload[pADPCM->payloadIndex] = compressedNibble;
    pADPCM->payloadIndex++;
  }

  return pADPCM;
}

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/***************************************************************************//**
 * @brief
 *    Encode sample IMA.
 *
 * @param[in] sample
 *    Sample to be encoded.
 *
 * @param[in] adpcmState
 *     ADPCM state.
 *
 * @return
 *    Compressed
 ******************************************************************************/
static uint8_t imaEncodeSample(int16_t sample, adpcm_state_t *adpcmState)
{
  uint8_t compressedCode = 0;
  /* Promote the current step size and predicted sample to int */
  int32_t currentStep = (int32_t)adpcmStepTable[adpcmState->step];
  int32_t predictedSample = (int32_t)adpcmState->predictedSample;
  int32_t diff;
  int32_t predictedDiff;

  /* Calculate the current difference */
  diff = (int32_t)sample - predictedSample;

  /* Set compressed code sign bit */
  if (diff < 0) {
    compressedCode = 8;
    diff = -diff;
  }

  /* Calculate the compressed code and predicted difference */
  predictedDiff = currentStep >> 3;
  if (diff >= currentStep) {
    compressedCode |= 4;
    diff -= currentStep;
    predictedDiff += currentStep;
  }
  currentStep >>= 1;
  if (diff >= currentStep) {
    compressedCode |= 2;
    diff -= currentStep;
    predictedDiff += currentStep;
  }
  currentStep >>= 1;
  if (diff >= currentStep) {
    compressedCode |= 1;
    predictedDiff += currentStep;
  }

  /* Calculate the new predicted sample with saturation logic from int32_t->int16_t */
  predictedSample += (compressedCode & 8) ? -predictedDiff : predictedDiff;
  if (predictedSample > 32767) {
    predictedSample = 32767;
  } else if (predictedSample < -32768) {
    predictedSample = -32768;
  }
  adpcmState->predictedSample = (int16_t)predictedSample;

  /* Update the step index for the next sample */
  adpcmState->step += adpcmIndexAdjust[compressedCode];
  if (adpcmState->step < 0) {
    adpcmState->step = 0;
  } else if (adpcmState->step > 88) {
    adpcmState->step = 88;
  }

  return compressedCode;
}

/** @endcond DO_NOT_INCLUDE_WITH_DOXYGEN */
