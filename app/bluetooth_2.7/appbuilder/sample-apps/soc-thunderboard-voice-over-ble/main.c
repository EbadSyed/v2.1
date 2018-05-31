/***********************************************************************************************//**
 * @file   main.c
 * @brief  Silicon Labs Voice over Bluetooth Low Energy Example Project
 *
 * This example demonstrates the bare minimum needed for a Blue Gecko C application
 * that allows transmit Voice over Bluetooth Low Energy (VoBLE). The application
 * starts advertising after boot and restarts advertising after a connection is closed.
 * Voice transmission starts after SW1 button is pressed and released and stops after SW2 button is pressed and released.
 * Audio data stream could be filtered depending on configuration and encoded to ADPCM format.
 * Finally encoded data are written into "Audio Data" characteristic being a part of "Voice over BLE" Service.
 *
 * VoBLE Service contains following configuration characteristics:
 * 1. ADC Resolution - Set ADC resolution 8 or 12 bits
 * 2. Sample Rate    - Set sample rate 8 or 16 [kHz]
 * 3. Filter Enable  - Enable/Disable filtering
 *
 * There is also example of NCP application (voice_over_bluetooth_low_energy_app)
 * which demonstrate how to connect to Thunderboard Sense v1,
 * set correct configuration and store data into the file.
 *
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

/* Board headers */
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"

#include "pti.h"

/* Device initialization header */
#include "InitDevice.h"

#ifdef FEATURE_SPI_FLASH
#include "em_usart.h"
#include "mx25flash_spi.h"
#endif /* FEATURE_SPI_FLASH */

#include <sleep.h>
#include "voble_config.h"
#include "button.h"
#include "mic.h"
#include <filter.h>
#include "adpcm.h"
#include "circular_buff.h"

#include "thunderboard/util.h"
#include "thunderboard/board.h"

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

#define EXT_SIGNAL_READY_TO_SEND    (1 << 4)  /**< External signal - Data ready to send in circular buffer */
#define EXT_SIGNAL_NOTHING_TO_SEND  (1 << 5)  /**< External signal - Nothing to send from circular buffer */

#define MIC_SEND_BUFFER_SIZE (224)

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

/* Gecko configuration parameters (see gecko_configuration.h) */
static const gecko_configuration_t config = {
  .config_flags = 0,
  .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap),
  .bluetooth.sleep_clock_accuracy = 100, // ppm
  .gattdb = &bg_gattdb_data,
  .ota.flags = 0,
  .ota.device_name_len = 3,
  .ota.device_name_ptr = "OTA",
};

/* Flag for indicating DFU Reset must be performed */
uint8_t boot_to_dfu = 0;

/* Microphone variable */
extern biquad_t *filter;
static bool micEnabled = false;
static audio_data_buff_t micAudioBuffer;
static uint8_t ble_connection;
voble_config_t voble_config;

/* circular buffer */
circular_buffer_t circular_buffer;

void init(void)
{
  UTIL_init();
  BOARD_init();
  BUTTON_Init();
  cb_init(&circular_buffer, (MIC_SAMPLE_BUFFER_SIZE * 10), sizeof(uint8_t));

  voble_config.adcResolution = adc_8bit;
  voble_config.sampleRate = sr_16k;
  voble_config.filter_enabled = true;
  voble_config.encoding_enabled = true;
}

/***************************************************************************//**
 * @brief
 *    Send response to Write Request
 *
 * @return
 *    Write response result
 ******************************************************************************/
struct gecko_msg_gatt_server_send_user_write_response_rsp_t* send_write_response(struct gecko_cmd_packet* evt, uint16 characteristic, uint8 att_errorcode)
{
  return gecko_cmd_gatt_server_send_user_write_response(evt->data.evt_gatt_server_user_write_request.connection, characteristic, att_errorcode);
}

/***************************************************************************//**
 * @brief
 *    Sets ADC resolution in Voice over BLE configuration structure
 *
 * @return
 *    None
 ******************************************************************************/
void write_request_adc_resolution_handle(struct gecko_cmd_packet* evt)
{
  switch ( evt->data.evt_gatt_server_user_write_request.value.data[0] ) {
    case adc_12bit:
      voble_config.adcResolution = adc_12bit;
      break;
    case adc_8bit:
    default:
      voble_config.adcResolution = adc_8bit;
      break;
  }

  send_write_response(evt, gattdb_adc_resolution, bg_err_success);
}

/***************************************************************************//**
 * @brief
 *    Sets sample rate in Voice over BLE configuration structure
 *
 * @return
 *    None
 ******************************************************************************/
void write_request_sample_rate_handle(struct gecko_cmd_packet* evt)
{
  switch ( evt->data.evt_gatt_server_user_write_request.value.data[0] ) {
    case sr_16k:
      voble_config.sampleRate = sr_16k;
      break;
    case sr_8k:
    default:
      voble_config.sampleRate = sr_8k;
      break;
  }

  send_write_response(evt, gattdb_sample_rate, bg_err_success);
}

/***************************************************************************//**
 * @brief
 *    Set enable/disable filtering flag in Voice over BLE configuration structure
 *
 * @return
 *    None
 ******************************************************************************/
void write_request_filter_enable_handle(struct gecko_cmd_packet* evt)
{
  voble_config.filter_enabled = (bool)evt->data.evt_gatt_server_user_write_request.value.data[0];
  send_write_response(evt, gattdb_filter_enable, bg_err_success);
}

/***************************************************************************//**
 * @brief
 *    Set enable/disable encoding flag in Voice over BLE configuration structure
 *
 * @return
 *    None
 ******************************************************************************/
void write_request_encoding_enable_handle(struct gecko_cmd_packet* evt)
{
  voble_config.encoding_enabled = (bool)evt->data.evt_gatt_server_user_write_request.value.data[0];
  send_write_response(evt, gattdb_encoding_enable, bg_err_success);
}

/***************************************************************************//**
 * @brief
 *    Process data incoming from microphone. Data are filtered or not depending on
 *    voble_config.filter_enabled flag, stored in temporary buffer, encoded and finally
 *    written to circular buffer.
 *
 * @return
 *    None
 ******************************************************************************/
void processData(uint16_t *pSrc)
{
  int16_t buffer[MIC_SAMPLE_BUFFER_SIZE];
  adpcm_t *pEncoded;
  uint16_t cb_error;

  GPIO_PinOutSet(gpioPortF, 4);
  if ( voble_config.filter_enabled) {
    /* Filter data */
    FIL_filter(buffer, pSrc, MIC_SAMPLE_BUFFER_SIZE);
  } else {
    /* Convert PCM data from uint16_t to int16_t */
    for ( uint16_t i = 0; i < MIC_SAMPLE_BUFFER_SIZE; i++ ) {
      buffer[i] = (int16_t)pSrc[i];
    }
  }
  GPIO_PinOutClear(gpioPortF, 4);

  GPIO_PinOutSet(gpioPortF, 5);
  if ( voble_config.encoding_enabled) {
    /* Encode data */
    pEncoded = ADPCM_ima_encodeBuffer(buffer, MIC_SAMPLE_BUFFER_SIZE);
    cb_error = cb_push_buff(&circular_buffer, pEncoded->payload, pEncoded->payload_size);
  } else {
    cb_error = cb_push_buff(&circular_buffer, (uint8_t *)buffer, (sizeof(buffer) / sizeof(uint16_t)) * 2);
  }
  GPIO_PinOutClear(gpioPortF, 5);
  gecko_external_signal(EXT_SIGNAL_READY_TO_SEND);

  return;
}

/***************************************************************************//**
 * @brief
 *    Send audio data from circular buffer. Data are sent in packages of MIC_SEND_BUFFER_SIZE size.
 *    If there is less then MIC_SEND_BUFFER_SIZE in circular buffer data will be send after next DMA readout.
 *
 * @return
 *    None
 ******************************************************************************/
void send_audio_data(void)
{
  uint16_t cb_error;
  uint8_t buffer[MIC_SEND_BUFFER_SIZE];

  GPIO_PinOutSet(gpioPortF, 6);
  cb_error = cb_pop_buff(&circular_buffer, buffer, MIC_SEND_BUFFER_SIZE);

  if ( cb_error == cb_err_insuff_data ) {
    gecko_external_signal(EXT_SIGNAL_NOTHING_TO_SEND);
  } else {
    /* Write data to characteristic */
    gecko_cmd_gatt_server_send_characteristic_notification(ble_connection, gattdb_audio_data, (MIC_SEND_BUFFER_SIZE), buffer);
    gecko_external_signal(EXT_SIGNAL_READY_TO_SEND);
  }
  GPIO_PinOutClear(gpioPortF, 6);
}

/**
 * @brief  Main function
 */
int main(void)
{
#ifdef FEATURE_SPI_FLASH
  /* Put the SPI flash into Deep Power Down mode for those radio boards where it is available */
  MX25_init();
  MX25_DP();
  /* We must disable SPI communication */
  USART_Reset(USART1);

#endif /* FEATURE_SPI_FLASH */

  /* Initialize peripherals */
  enter_DefaultMode_from_RESET();

#if (HAL_PTI_ENABLE == 1) || defined(FEATURE_PTI_SUPPORT)
  /* Configure and enable PTI */
  configEnablePti();
#endif

  /* Initialize stack */
  gecko_init(&config);

  init();

  while (1) {
    /* Event pointer for handling events */
    struct gecko_cmd_packet* evt;

    /* Check for stack event. */
    evt = gecko_wait_event();

    /* Handle events */
    switch (BGLIB_MSG_ID(evt->header)) {
      /* This boot event is generated when the system boots up after reset.
       * Do not call any stack commands before receiving the boot event.
       * Here the system is set to start advertising immediately after boot procedure. */
      case gecko_evt_system_boot_id:

        /* Set advertising parameters. 100ms advertisement interval. All channels used.
         * The first two parameters are minimum and maximum advertising interval, both in
         * units of (milliseconds * 1.6). The third parameter '7' sets advertising on all channels. */
        gecko_cmd_le_gap_set_adv_parameters(160, 160, 7);
        gecko_cmd_gatt_set_max_mtu(250);

        /* Start general advertising and enable connections. */
        gecko_cmd_le_gap_set_mode(le_gap_general_discoverable, le_gap_undirected_connectable);
        break;

      case gecko_evt_le_connection_closed_id:

        /* Check if need to boot to dfu mode */
        if (boot_to_dfu) {
          /* Enter to DFU OTA mode */
          gecko_cmd_system_reset(2);
        } else {
          /* Restart advertising after client has disconnected */
          gecko_cmd_le_gap_set_mode(le_gap_general_discoverable, le_gap_undirected_connectable);
        }
        break;

      /* This event is triggered after the connection has been opened */
      case gecko_evt_le_connection_opened_id:
        ble_connection = evt->data.evt_le_connection_opened.connection;
        //appCfgConnectionOpenedEvent(connection, evt->data.evt_le_connection_opened.bonding);
        break;

      /* Events related to OTA upgrading
         ----------------------------------------------------------------------------- */

      case gecko_evt_gatt_server_user_write_request_id:
        switch (evt->data.evt_gatt_server_user_write_request.characteristic) {
          /* Check if the user-type OTA Control Characteristic was written.
           * If ota_control was written, boot the device into Device Firmware Upgrade (DFU) mode. */
          case gattdb_ota_control:
            /* Set flag to enter to OTA mode */
            boot_to_dfu = 1;

            /* Send response to Write Request */
            send_write_response(evt, gattdb_ota_control, bg_err_success);

            /* Close connection to enter to DFU OTA mode */
            gecko_cmd_endpoint_close(evt->data.evt_gatt_server_user_write_request.connection);
            break;

          /* Check if the user-type ADC Resolution Characteristic was written
             If adc_resolution was written, set correct configuration in voble structure.*/
          case gattdb_adc_resolution:
            write_request_adc_resolution_handle(evt);
            break;

          /* Check if the user-type Sample Rate Characteristic was written
             If sample_rate was written, set correct configuration in voble structure.*/
          case gattdb_sample_rate:
            write_request_sample_rate_handle(evt);
            break;

          /* Check if the user-type Filter Enable Characteristic was written
             If filter_enable was written, set correct configuration in voble structure.*/
          case gattdb_filter_enable:
            write_request_filter_enable_handle(evt);
            break;

          /* Check if the user-type Encoding Enable Characteristic was written
             If encoding_enable was written, set correct configuration in voble structure.*/
          case gattdb_encoding_enable:
            write_request_encoding_enable_handle(evt);
            break;

          default:
            break;
        }
        break;

      case gecko_evt_system_external_signal_id:
        switch (evt->data.evt_system_external_signal.extsignals) {
          case EXTSIG_BUTTON_SW1_RELEASED:
            if (!micEnabled) {
              /* ADPCM encoder initialization */
              ADPCM_init();

              /* Filter initialization. Filter parameters: */
              if ( voble_config.filter_enabled) {
                filter_parameters_t fp = DEFAULT_FILTER;
                filter = FIL_Init(&fp);
              }

              /* Block Energy Mode 2 when audio acquisition starts*/
              SLEEP_SleepBlockBegin(sleepEM2);

              /*Microphone initialization */
              MIC_init(voble_config.sampleRate, &micAudioBuffer);

              /* Start audio data acquisition */
              MIC_start();

              /* Audio data acquisition started*/
              micEnabled = true;
            }
            break;
          case EXTSIG_BUTTON_SW2_RELEASED:
            if (micEnabled) {
              /* Filter deinitialization*/
              free(filter); filter = NULL;

              /*Microphone deinitialization */
              MIC_deInit();

              /* Unblock Energy Mode 2 when audio acquisition stopped*/
              SLEEP_SleepBlockEnd(sleepEM2);

              /* Audio data acquisition stopped*/
              micEnabled = false;
            }
            break;

          /* Process data from buffer0 */
          case EXT_SIGNAL_BUFFER0_READY:
            processData(MIC_getSampleBuffer0());
            GPIO_PinOutClear(gpioPortF, 3);
            break;

          /* Process data from buffer1 */
          case EXT_SIGNAL_BUFFER1_READY:
            processData(MIC_getSampleBuffer1());
            GPIO_PinOutClear(gpioPortF, 3);
            break;

          /* Data ready to send in circular buffer */
          case EXT_SIGNAL_READY_TO_SEND:
            send_audio_data();
            break;

          case EXT_SIGNAL_NOTHING_TO_SEND:
            break;

          default:
            break;
        }
        break;

      default:
        break;
    }
  }

  return 0;
}

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */
