/***********************************************************************************************//**
 * \file   config.h
 * \brief  Configuration header file
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "bg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
 * Type Definitions
 **************************************************************************************************/

/** Supported sample rates */
typedef enum {
  sr_8k = 8,
  sr_16k = 16,
}adc_sample_rate_t;

/** Supported ADC resolutions */
typedef enum {
  adc_res_8b = 8,
  adc_res_12b = 12,
}adc_resolution_t;

/** Internal actions required to set correct configuration and manage incoming events.*/
typedef enum {
  act_none = 0,
  act_enable_notification,
  act_set_sample_rate,
  act_set_adc_resolution,
  act_enable_filtering,
  act_enable_encoding,
}action_t;

/** Configuration structure */
typedef struct {
  uint32_t baud_rate;                 /**< UART baud rate */
  char *uart_port;                    /**< UART port */
  char *out_file_name;                /**< Output file name */
  bool audio_data_notification;       /**< Enable/Disable audio data notification*/
  adc_sample_rate_t adc_sample_rate;  /**< Sample rate*/
  adc_resolution_t adc_resolution;    /**< ADC resolution*/
  bool filter_enabled;                /**< Enable/Disable filtering*/
  bool encoding_enabled;              /**< Enable/Disable encoding*/
  bool remote_address_set;            /**< Remote Device address given as application parameter */
  bd_addr remote_address;             /**< Remote Device address */
}configuration_t;

#define DEFAULT_WIN_OS_UART_PORT "COM0"
#define DEFAULT_APPLE_OS_UART_PORT "/dev/tty.usbmodem14171"
#define DEFAULT_LINUX_OS_UART_PORT "/dev/ttyACM0"

#define DEFAULT_UART_BAUD_RATE   115200
#define DEFAULT_OUTPUT_FILE_NAME "audio_data.raw"

#define SERVICE_VOICE_OVER_BLE_UUID { 0x10, 0x0ad, 0xb3, 0x9e, 0x42, 0xdf, 0xd3, 0x93, 0x62, 0x43, 0x2e, 0xdc, 0x93, 0x11, 0xef, 0xb7 }

/** Characteristic handler numbers. These numbers should correspond to characteristic numbers on remote device */
#define CHAR_AUDIO_DATA_ATTRIBUTE_HANDLER      (22)
#define CHAR_ADC_RESOLUTION_ATTRIBUTE_HANDLER  (25)
#define CHAR_SAMPLE_RATE_ATTRIBUTE_HANDLER     (27)
#define CHAR_FILTER_ENABLE_ATTRIBUTE_HANDLER   (29)
#define CHAR_ENCODING_ENABLE_ATTRIBUTE_HANDLER (31)

/**  Default configuration*/
#define DEFAULT_CONFIGURATION                                                                             \
  {                                                                                                       \
    DEFAULT_UART_BAUD_RATE,   /** The default baud rate to use. */                                        \
    DEFAULT_WIN_OS_UART_PORT, /** The default UART port for WIN. This is overwritten for Apple or Linux*/ \
    DEFAULT_OUTPUT_FILE_NAME, /** The default output file name. */                                        \
    true,                     /** Audio Data notification enabled */                                      \
    sr_16k,                   /** The default ADC sample rate */                                          \
    adc_res_12b,              /** The default ADC resolution */                                           \
    false,                    /** Enable filtering */                                                     \
    true,                     /** Enable encoding */                                                      \
  }

/** SW Timer events definition */
#define EVT_CONNECT         0x1
#define EVT_SCANNING        0x2
#define EVT_SCAN_TIMEOUT    0x3

/** SW timer intervals */
#define SW_TIMER_10_MS   (328)
#define SW_TIMER_20_MS   (SW_TIMER_10_MS * 2)
#define SW_TIMER_50_MS   (SW_TIMER_10_MS * 5)

#define DEFAULT_SCAN_TIMEOUT 10 * 20 /** 10 seconds */

/** Error macro */
#define ERROR_EXIT(...) do { printf(__VA_ARGS__); exit(EXIT_FAILURE); } while (0)

/** Debug macros */
#define DEBUG_INFO(...)     do { printf("[Inf] "); printf(__VA_ARGS__); fflush(stdout); } while (0)
#define DEBUG_WARNING(...)  do { printf("[War] "); printf(__VA_ARGS__); fflush(stdout); } while (0)
#define DEBUG_ERROR(...)    do { printf("[Err] "); printf(__VA_ARGS__); fflush(stdout); } while (0)
#define DEBUG_SUCCESS(...)  do { printf("[Ok ] "); printf(__VA_ARGS__); printf("\n"); fflush(stdout); } while (0)

configuration_t *CONF_get(void);

#ifdef __cplusplus
};
#endif

#endif /* CONFIG_H_ */
