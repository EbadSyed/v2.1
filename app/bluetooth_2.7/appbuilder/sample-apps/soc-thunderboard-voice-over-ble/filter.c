/***************************************************************************//**
 * @file filter.c
 * @brief Filters implementation
 * @version
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

#include <filter.h>
#include <math.h>
#include <stdlib.h>
#include "mic.h"

/***************************************************************************//**
 * @defgroup Filters FIL - Filters
 * @{
 * @brief Filters implementation.
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/***************************************************************************//**
 * @defgroup Filters_Locals Filters Local Variables
 * @{
 * @brief Filters local variables
 ******************************************************************************/

static bool filter_initialized = false;     /**< Filter initialization flag */
biquad_t *filter;                           /**< Biquadratic filter structure */

/** @} {end defgroup Mic_Locals} */

/** @endcond DO_NOT_INCLUDE_WITH_DOXYGEN */

/***************************************************************************//**
 * @defgroup Filters_Functions Filters Functions
 * @{
 * @brief Filters support functions
 ******************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

static sample_t compute(sample_t sample, biquad_t * b);
static void calc_LPF_parameters(filter_coefficient_t *c);
static void calc_HPF_parameters(filter_coefficient_t *c);
static void calc_BPF_parameters(filter_coefficient_t *c);
static void calc_NOTCH_parameters(filter_coefficient_t *c);
static void calc_PEQ_parameters(filter_coefficient_t *c);
static void calc_LSH_parameters(filter_coefficient_t *c);
static void calc_HSH_parameters(filter_coefficient_t *c);

/** @endcond DO_NOT_INCLUDE_WITH_DOXYGEN */

/***************************************************************************//**
 * @brief
 *    Filter Initialization
 *
 * @param[in] fp
 *    Filter parameters
 *
 * @return
 *    Biquadratic filter data or NULL if no available memory.
 ******************************************************************************/
biquad_t *FIL_Init(filter_parameters_t *fp)
{
  biquad_t *b;
  filter_coefficient_t coef;

  filter_initialized = false;

  if (fp == NULL) {
    return (biquad_t *)NULL;
  }

  b = malloc(sizeof(biquad_t));
  if (b == NULL) {
    return NULL;
  }
  memset(b, 0, sizeof(biquad_t));

  /* setup variables */
  coef.A = pow(10, fp->dbGain / 40);
  coef.omega = 2 * M_PI * fp->freq / fp->srate;
  coef.sn = sin(coef.omega);
  coef.cs = cos(coef.omega);
  coef.alpha = coef.sn * sinh(M_LN2 / 2 * fp->bandwidth * coef.omega / coef.sn);
  coef.beta = sqrt(coef.A + coef.A);

  switch (fp->type) {
    case LPF:
      calc_LPF_parameters(&coef);
      break;
    case HPF:
      calc_HPF_parameters(&coef);
      break;
    case BPF:
      calc_BPF_parameters(&coef);
      break;
    case NOTCH:
      calc_NOTCH_parameters(&coef);
      break;
    case PEQ:
      calc_PEQ_parameters(&coef);
      break;
    case LSH:
      calc_LSH_parameters(&coef);
      break;
    case HSH:
      calc_HSH_parameters(&coef);
      break;
    default:
      free(b);
      return NULL;
  }

  /* pre-computed the coefficients */
  b->a0 = coef.b0 / coef.a0;
  b->a1 = coef.b1 / coef.a0;
  b->a2 = coef.b2 / coef.a0;
  b->a3 = coef.a1 / coef.a0;
  b->a4 = coef.a2 / coef.a0;

  /* clean samples */
  b->x1 = b->x2 = 0;
  b->y1 = b->y2 = 0;

  filter_initialized = true;

  return b;
}

/***************************************************************************//**
 * @brief
 *    Filter audio data
 *
 * @param[in] buffIn
 *    Audio data to be filtered.
 *
 * @param[out] buffOut
 *    Filtered audio data.
 *
 * @param[out] size
 *    Length of filtered audio data buffer.
 *
 * @return
 *    True when filtration done, otherwise false
 ******************************************************************************/
bool FIL_filter(int16_t *buffOut, uint16_t *buffIn, uint16_t size)
{
  if ( !filter_initialized ) {
    return false;
  }

  for ( uint16_t i = 0; i < size; i++) {
    buffOut[i] = (int16_t)compute((sample_t)buffIn[i], filter);
  }
  return true;
}

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/***************************************************************************//**
 * @brief
 *    Calculates Low Pass Filter parameters
 *
 * @param[in] A, omega, sn, cs, alpha, beta
 *    Given variables
 *
 * @param[out] b0,b1,b2,a0,a1,a2
 *    Calculated  parameters
 *
 * @return
 *    None
 ******************************************************************************/
static void calc_LPF_parameters(filter_coefficient_t *c)
{
  c->b0 = (1 - c->cs) / 2;
  c->b1 = 1 - c->cs;
  c->b2 = (1 - c->cs) / 2;
  c->a0 = 1 + c->alpha;
  c->a1 = -2 * c->cs;
  c->a2 = 1 - c->alpha;
}

/***************************************************************************//**
 * @brief
 *    Calculates High Pass Filter parameters
 *
 * @param[in] A, omega, sn, cs, alpha, beta
 *    Given variables
 *
 * @param[out] b0,b1,b2,a0,a1,a2
 *    Calculated  parameters
 *
 * @return
 *    None
 ******************************************************************************/
static void calc_HPF_parameters(filter_coefficient_t *c)
{
  c->b0 = (1 + c->cs) / 2;
  c->b1 = -(1 + c->cs);
  c->b2 = (1 + c->cs) / 2;
  c->a0 = 1 + c->alpha;
  c->a1 = -2 * c->cs;
  c->a2 = 1 - c->alpha;
}

/***************************************************************************//**
 * @brief
 *    Calculates Band Pass Filter parameters
 *
 * @param[in] A, omega, sn, cs, alpha, beta
 *    Given variables
 *
 * @param[out] b0,b1,b2,a0,a1,a2
 *    Calculated  parameters
 *
 * @return
 *    None
 ******************************************************************************/
static void calc_BPF_parameters(filter_coefficient_t *c)
{
  c->b0 = c->alpha;
  c->b1 = 0;
  c->b2 = -c->alpha;
  c->a0 = 1 + c->alpha;
  c->a1 = -2 * c->cs;
  c->a2 = 1 - c->alpha;
}

/***************************************************************************//**
 * @brief
 *    Calculates Notch Filter parameters
 *
 * @param[in] A, omega, sn, cs, alpha, beta
 *    Given variables
 *
 * @param[out] b0,b1,b2,a0,a1,a2
 *    Calculated  parameters
 *
 * @return
 *    None
 ******************************************************************************/
static void calc_NOTCH_parameters(filter_coefficient_t *c)
{
  c->b0 = 1;
  c->b1 = -2 * c->cs;
  c->b2 = 1;
  c->a0 = 1 + c->alpha;
  c->a1 = -2 * c->cs;
  c->a2 = 1 - c->alpha;
}

/***************************************************************************//**
 * @brief
 *    Calculates Peaking Band EQ Filter parameters
 *
 * @param[in] A, omega, sn, cs, alpha, beta
 *    Given variables
 *
 * @param[out] b0,b1,b2,a0,a1,a2
 *    Calculated  parameters
 *
 * @return
 *    None
 ******************************************************************************/
static void calc_PEQ_parameters(filter_coefficient_t *c)
{
  c->b0 = 1 + (c->alpha * c->A);
  c->b1 = -2 * c->cs;
  c->b2 = 1 - (c->alpha * c->A);
  c->a0 = 1 + (c->alpha / c->A);
  c->a1 = -2 * c->cs;
  c->a2 = 1 - (c->alpha / c->A);
}

/***************************************************************************//**
 * @brief
 *    Calculates Low Shelf Filter parameters
 *
 * @param[in] A, omega, sn, cs, alpha, beta
 *    Given variables
 *
 * @param[out] b0,b1,b2,a0,a1,a2
 *    Calculated  parameters
 *
 * @return
 *    None
 ******************************************************************************/
static void calc_LSH_parameters(filter_coefficient_t *c)
{
  c->b0 = c->A * ((c->A + 1) - (c->A - 1) * c->cs + c->beta * c->sn);
  c->b1 = 2 * c->A * ((c->A - 1) - (c->A + 1) * c->cs);
  c->b2 = c->A * ((c->A + 1) - (c->A - 1) * c->cs - c->beta * c->sn);
  c->a0 = (c->A + 1) + (c->A - 1) * c->cs + c->beta * c->sn;
  c->a1 = -2 * ((c->A - 1) + (c->A + 1) * c->cs);
  c->a2 = (c->A + 1) + (c->A - 1) * c->cs - c->beta * c->sn;
}

/***************************************************************************//**
 * @brief
 *    Calculates High Shelf Filter parameters
 *
 * @param[in] A, omega, sn, cs, alpha, beta
 *    Given variables
 *
 * @param[out] b0,b1,b2,a0,a1,a2
 *    Calculated  parameters
 *
 * @return
 *    None
 ******************************************************************************/
static void calc_HSH_parameters(filter_coefficient_t *c)
{
  c->b0 = c->A * ((c->A + 1) + (c->A - 1) * c->cs + c->beta * c->sn);
  c->b1 = -2 * c->A * ((c->A - 1) + (c->A + 1) * c->cs);
  c->b2 = c->A * ((c->A + 1) + (c->A - 1) * c->cs - c->beta * c->sn);
  c->a0 = (c->A + 1) - (c->A - 1) * c->cs + c->beta * c->sn;
  c->a1 = 2 * ((c->A - 1) - (c->A + 1) * c->cs);
  c->a2 = (c->A + 1) - (c->A - 1) * c->cs - c->beta * c->sn;
}

/***************************************************************************//**
 * @brief
 *    Computes a filter on a sample.
 *
 * @param[in] sample
 *    Data sample to be filtered.
 *
 * @param[in] b
 *    Biquadratic filter data.
 *
 * @return
 *    Filtered sample
 ******************************************************************************/
static sample_t compute(sample_t sample, biquad_t *b)
{
  sample_t result;

  /* compute result */
  result = b->a0 * sample + b->a1 * b->x1 + b->a2 * b->x2 - b->a3 * b->y1 - b->a4 * b->y2;

  /* shift x1 to x2, sample to x1 */
  b->x2 = b->x1;
  b->x1 = sample;

  /* shift y1 to y2, result to y1 */
  b->y2 = b->y1;
  b->y1 = result;

  return result;
}

/** @endcond DO_NOT_INCLUDE_WITH_DOXYGEN */

/** @} {end defgroup Filters_Functions} */

/** @} {end defgroup Filters} */
