/***************************************************************************//**
 * @file voble_config.h
 * @brief Voice over Bluetooth Low Energy configuration
 * @version 5.2.1
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

#ifndef VOBLE_CONFIG_H_
#define VOBLE_CONFIG_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/***************************************************************************************************
 * Type Definitions
 **************************************************************************************************/
typedef enum {
  adc_8bit = 8,
  adc_12bit = 12
}adc_resolution_t;

typedef enum {
  sr_8k = 8,
  sr_16k = 16,
}sample_rate_t;

typedef struct {
  adc_resolution_t adcResolution;
  sample_rate_t sampleRate;
  bool filter_enabled;
  bool encoding_enabled;
}voble_config_t;

#endif /* VOBLE_CONFIG_H_ */
