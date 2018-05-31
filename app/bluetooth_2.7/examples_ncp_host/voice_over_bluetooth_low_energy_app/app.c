/***********************************************************************************************//**
 * \file   app.c
 * \brief  Event handling and application code for Empty NCP Host application example
 ***************************************************************************************************
 * <b> (C) Copyright 2016 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* standard library headers */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>

#include <errno.h>
#include <stdlib.h>

/* BG stack headers */
#include "bg_types.h"
#include "gecko_bglib.h"

/* Own header */
#include "config.h"
#include "app.h"
#include "scan.h"
#include "ble-callbacks.h"

// App booted flag
static bool appBooted = false;

/* Bluetooth connection */
uint8_t ble_connection;

/***************************************************************************************************
 * Static Function Declarations
 **************************************************************************************************/
static uint16_t audioPacketCnt = 0;
static bool transferStarted = false;
struct timeval currentTime, startTime;
static action_t action = act_none;

/***************************************************************************************************
 * Local Function Definitions
 **************************************************************************************************/
/***********************************************************************************************//**
 *  \brief  Set action which is started
 *  \param[in]  act  action
 **************************************************************************************************/
static void set_action(action_t act)
{
  action = act;
}

/***********************************************************************************************//**
 *  \brief  Establish connection to remote device
 *  \param[in]  remote_address Remote bluetooth device address
 **************************************************************************************************/
static void connect(bd_addr remote_address)
{
  struct gecko_msg_le_gap_open_rsp_t*rsp;
  struct gecko_msg_le_gap_set_conn_parameters_rsp_t *param_rsp;
  //set default connection parameters
  param_rsp = gecko_cmd_le_gap_set_conn_parameters(6, 6, 0, 300);
  if (param_rsp->result) {
    ERROR_EXIT("Error, set connection parameters failed,%x", param_rsp->result);
  }

  //move to connect state, connect to device address
  rsp = gecko_cmd_le_gap_open(remote_address, le_gap_address_type_public);
  if (rsp->result) {
    ERROR_EXIT("Error, open failed,%x", rsp->result);
  }
  ble_connection = rsp->connection;
  DEBUG_INFO("Connecting...\n");
}

/***********************************************************************************************//**
 *  \brief  Enable/Disable notification for Audio Data Characteristic. Notification has to be enabled to receive audio data.
 *  \param[in]  enable Enable/Disable logic value
 *  \return true if success, otherwise false
 **************************************************************************************************/
static bool enable_notifitacion(bool enable)
{
  DEBUG_INFO("%s notification for audio streaming...\n", enable ? "Enable" : "Disable");
  set_action(act_enable_notification);
  struct gecko_msg_gatt_set_characteristic_notification_rsp_t *rsp;
  rsp = gecko_cmd_gatt_set_characteristic_notification(ble_connection, CHAR_AUDIO_DATA_ATTRIBUTE_HANDLER, enable);

  return rsp->result ? false : true;
}

/***********************************************************************************************//**
 *  \brief  Set sample rate. Sample rate can be 8 or 16 [kHz]
 *  \param[in]  sr sample rate
 *  \return 0 if success, otherwise error code
 **************************************************************************************************/
static uint16 set_adc_sample_rate(adc_sample_rate_t sr)
{
  DEBUG_INFO("Setting %d[kHz] ADC sample rate...\n", (uint8_t)sr);
  set_action(act_set_sample_rate);
  struct gecko_msg_gatt_write_characteristic_value_rsp_t *rsp;
  rsp = gecko_cmd_gatt_write_characteristic_value(ble_connection, CHAR_SAMPLE_RATE_ATTRIBUTE_HANDLER, 1, (uint8 *)&sr);

  return rsp->result;
}

/***********************************************************************************************//**
 *  \brief  Set sample rate. Sample rate can be 8 or 16 [kHz]
 *  \param[in]  sr sample rate
 *  \return 0 if success, otherwise error code
 **************************************************************************************************/
static uint16 set_adc_resolution(adc_resolution_t res)
{
  DEBUG_INFO("Setting %d-bits ADC resolution...\n", (uint8_t)res);
  set_action(act_set_adc_resolution);
  struct gecko_msg_gatt_write_characteristic_value_rsp_t *rsp;
  rsp = gecko_cmd_gatt_write_characteristic_value(ble_connection, CHAR_ADC_RESOLUTION_ATTRIBUTE_HANDLER, 1, (uint8 *)&res);

  return rsp->result;
}

/***********************************************************************************************//**
 *  \brief  Enable/Disable audio data filtering.
 *  \param[in]  enable Enable/Disable logic value
 *  \return 0 if success, otherwise error code
 **************************************************************************************************/
static uint16 enable_filtering(bool enable)
{
  DEBUG_INFO("%s filtering...\n", enable ? "Enable" : "Disable");
  set_action(act_enable_filtering);
  struct gecko_msg_gatt_write_characteristic_value_rsp_t *rsp;
  rsp = gecko_cmd_gatt_write_characteristic_value(ble_connection, CHAR_FILTER_ENABLE_ATTRIBUTE_HANDLER, 1, (uint8 *)&enable);

  return rsp->result;
}

/***********************************************************************************************//**
 *  \brief  Enable/Disable audio data encoding.
 *  \param[in]  enable Enable/Disable logic value
 *  \return 0 if success, otherwise error code
 **************************************************************************************************/
static uint16 enable_encoding(bool enable)
{
  DEBUG_INFO("%s encoding...\n", enable ? "Enable" : "Disable");
  set_action(act_enable_encoding);
  struct gecko_msg_gatt_write_characteristic_value_rsp_t *rsp;
  rsp = gecko_cmd_gatt_write_characteristic_value(ble_connection, CHAR_ENCODING_ENABLE_ATTRIBUTE_HANDLER, 1, (uint8 *)&enable);

  return rsp->result;
}

/***********************************************************************************************//**
 *  \brief  Calculate difference between two time stamps and return elapsed time.
 *  \param[in]  t0 - first time stamp
 *  \param[in]  t1 - second time stamp
 *  \return elapsed time or 0 if elapsed time is negative
 **************************************************************************************************/
static float timediff_sec_msec(struct timeval t0, struct timeval t1)
{
  float elapsed_time = ((t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f) / 1000;
  return (elapsed_time < 0) ? (float)0 : elapsed_time;
}

/***********************************************************************************************//**
 *  \brief  Process procedure complete events. These events comes to application as a reply on previously send command.
 *  This functiion process chain of events and send new commands to configure VoBLE parameters.
 *  Chain is stopped when event result is error or when all parameters are set successfully.
 *  \param[in]  evt - Event pointer
 **************************************************************************************************/
static void process_procedure_complete_event(struct gecko_cmd_packet *evt)
{
  uint16_t result =  evt->data.evt_gatt_procedure_completed.result;

  switch (action) {
    case act_enable_notification:
    {
      set_action(act_none);
      if ( !result ) {
        DEBUG_SUCCESS("Success.");
        set_adc_sample_rate(CONF_get()->adc_sample_rate);
      } else {
        DEBUG_ERROR("Error: %d", result);
      }
    }
    break;

    case act_set_sample_rate:
    {
      set_action(act_none);
      if ( !result ) {
        DEBUG_SUCCESS("Success.");
        set_adc_resolution(CONF_get()->adc_resolution);
      } else {
        DEBUG_ERROR("Error: %d", result);
      }
    }
    break;

    case act_set_adc_resolution:
    {
      set_action(act_none);
      if ( !result ) {
        DEBUG_SUCCESS("Success.");
        enable_filtering(CONF_get()->filter_enabled);
      } else {
        DEBUG_ERROR("Error: %d", result);
      }
    }
    break;

    case act_enable_filtering:
    {
      set_action(act_none);
      if ( !result ) {
        DEBUG_SUCCESS("Success.");
        enable_encoding(CONF_get()->encoding_enabled);
      } else {
        DEBUG_ERROR("Error: %d", result);
      }
    }
    break;

    case act_enable_encoding:
    {
      set_action(act_none);
      if ( !result ) {
        DEBUG_SUCCESS("Success.");
      } else {
        DEBUG_ERROR("Error: %d", result);
      }
    }
    break;

    case act_none:
      break;

    default:
      break;
  }
}

/***********************************************************************************************//**
 *  \brief  Emits EVT_SCAN_TIMEOUT when timeout counter elapsed
 *  \param[in] None.
 *  \return None.
 **************************************************************************************************/
static void emit_timeout_event(void)
{
  static uint8_t scan_timout_counter = DEFAULT_SCAN_TIMEOUT;

  if ( !(scan_timout_counter--) ) {
    gecko_cmd_hardware_set_soft_timer(SW_TIMER_20_MS, EVT_SCAN_TIMEOUT, 1);
  }
}

/***********************************************************************************************//**
 *  \brief  Stop scanning and exit from application
 *  \param[in] None.
 *  \return None.
 **************************************************************************************************/
static void stop_scan_and_exit(void)
{
  gecko_cmd_le_gap_end_procedure();
  uartClose();
  printf("\b \n");
  DEBUG_INFO("No VoBLE devices found. Exiting.\n");
  exit(EXIT_SUCCESS);
}

/***********************************************************************************************//**
 *  \brief  Emit EVT_CONNECT event when VoBLE device found or EVT_SCAN_TIMEOUT when VoBLE did not found.
 *  \param[in] None.
 *  \return None.
 **************************************************************************************************/
static void emit_connect_or_timeout_event(void)
{
  SCAN_Indication();

  if (SCAN_Is_Device_Found()) {
    /** Disable timer */
    gecko_cmd_hardware_set_soft_timer(0, EVT_SCANNING, 1);
    gecko_cmd_hardware_set_soft_timer(SW_TIMER_20_MS, EVT_CONNECT, 1);
  } else {
    emit_timeout_event();
  }
}

/***********************************************************************************************//**
 *  \brief  Software timer event handler
 *  \param[in] evt Event pointer.
 *  \return None.
 **************************************************************************************************/
static void sw_timer_event_handler(struct gecko_cmd_packet* evt)
{
  switch (evt->data.evt_hardware_soft_timer.handle) {
    case EVT_CONNECT:
      connect(CONF_get()->remote_address);
      break;

    case EVT_SCANNING:
      emit_connect_or_timeout_event();
      break;

    case EVT_SCAN_TIMEOUT:
      stop_scan_and_exit();
      break;

    default:
      break;
  }
}

/***********************************************************************************************//**
 *  \brief  Event handler function.
 *  \param[in] evt Event pointer.
 **************************************************************************************************/
void appHandleEvents(struct gecko_cmd_packet *evt)
{
  if (NULL == evt) {
    return;
  }

  // Do not handle any events until system is booted up properly.
  if ((BGLIB_MSG_ID(evt->header) != gecko_evt_system_boot_id)
      && !appBooted) {
#if defined(DEBUG)
    printf("Event: 0x%04x\n", BGLIB_MSG_ID(evt->header));
#endif
    usleep(50000);
    return;
  }

  /* Handle events */
  switch (BGLIB_MSG_ID(evt->header)) {
    case gecko_evt_system_boot_id:

      gecko_cmd_gatt_set_max_mtu(250);

      appBooted = true;
      DEBUG_SUCCESS("System booted.");

      if ( CONF_get()->remote_address_set ) {
        gecko_cmd_hardware_set_soft_timer(SW_TIMER_20_MS, EVT_CONNECT, 1);
      } else {
        DEBUG_INFO("Scanning for VoBLE devices. ");
        gecko_cmd_hardware_set_soft_timer(SW_TIMER_50_MS, EVT_SCANNING, 0);
        gecko_cmd_le_gap_discover(1);
      }
      break;

    case gecko_evt_le_connection_closed_id:

      /* Restart general advertising and re-enable connections after disconnection. */
      gecko_cmd_le_gap_set_mode(le_gap_general_discoverable, le_gap_undirected_connectable);

      break;

    case gecko_evt_le_connection_opened_id:
      DEBUG_SUCCESS("Success.");
      enable_notifitacion(CONF_get()->audio_data_notification);
      break;

    case gecko_evt_gatt_procedure_completed_id:
      process_procedure_complete_event(evt);
      break;

    case gecko_evt_gatt_characteristic_value_id:
    {
      if (!transferStarted) {
        transferStarted = true;
        gettimeofday(&startTime, NULL);
      } else {
        gettimeofday(&currentTime, NULL);
      }

      DEBUG_INFO("%.4f[s] - %d packet. Data length: %d [bytes]\n", timediff_sec_msec(startTime, currentTime), audioPacketCnt++, evt->data.evt_gatt_characteristic_value.value.len);

      FILE *fd;
      if ( (fd = fopen(CONF_get()->out_file_name, "ab+")) != NULL ) {
        fwrite(evt->data.evt_gatt_characteristic_value.value.data, sizeof(uint8_t), evt->data.evt_gatt_characteristic_value.value.len, fd);
        fclose(fd);
      } else {
        DEBUG_ERROR("File can not be open.\n");
        gecko_cmd_endpoint_close(ble_connection);
      }
    }
    break;

    case gecko_evt_le_gap_scan_response_id:
      SCAN_Process_scan_response(evt);
      break;

    case gecko_evt_hardware_soft_timer_id:
      sw_timer_event_handler(evt);
      break;

    default:
      break;
  }
}
