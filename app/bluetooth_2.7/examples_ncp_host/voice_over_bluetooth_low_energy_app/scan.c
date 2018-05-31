/***********************************************************************************************//**
 * \file   scan.c
 * \brief  Scan for VoBLE devices source file
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "config.h"
#include "scan.h"
#include "platform.h"

/** Flag indicated if scanning process found device with VoBLE service */
static bool voble_device_found = false;

/***********************************************************************************************//**
 *  \brief  Check if VoBLE service UUID is included at given data buffer  .
 *  \param[in]  data buffer
 *  \param[in] data buffer size
 *  \return  true if VoBLE service UUID exists, otherwise false
 **************************************************************************************************/
static bool is_voble_service(const uint8_t *data, size_t data_len)
{
  uint8_t service_uuid[] = SERVICE_VOICE_OVER_BLE_UUID;
  uint8_t service_uuid_len = sizeof(service_uuid) / sizeof(uint8_t);
  uint8_t offset = 0;

  for ( uint8_t i = 0; i < data_len; i++) {
    for (uint8_t j = 0; j < service_uuid_len; j++) {
      if ( data[i + offset] != service_uuid[j] ) {
        offset = 0;
        break;
      } else {
        offset++;
        if (j == (service_uuid_len - 1)) {
          return true;
        }
      }
    }
  }
  return false;
}

/***********************************************************************************************//**
 *  \brief  Check if device with VoBLE service found.
 *  \return  true if device found, otherwise false
 **************************************************************************************************/
bool SCAN_Is_Device_Found(void)
{
  return voble_device_found;
}

/***********************************************************************************************//**
 *  \brief  Twirled scan indicator.
 *  \return  None.
 **************************************************************************************************/
void SCAN_Indication(void)
{
  static uint32_t cnt = 0;
  const char *fan[] = { "-", "\\", "|", "/" };

  printf("\b%s", fan[((cnt++) % 4)]);
  fflush(stdout);
}

/***********************************************************************************************//**
 *  \brief  Event handler for gecko_evt_le_gap_scan_response_id event.
 *  \param[in] evt Event pointer.
 **************************************************************************************************/
void SCAN_Process_scan_response(struct gecko_cmd_packet *evt)
{
  if ( is_voble_service(evt->data.evt_le_gap_scan_response.data.data, evt->data.evt_le_gap_scan_response.data.len) ) {
    memcpy(&CONF_get()->remote_address, &evt->data.evt_le_gap_scan_response.address, sizeof(bd_addr));
    printf("\r"); DEBUG_INFO("VoBLE device found: "); print_address(CONF_get()->remote_address); printf("\n"); fflush(stdout);
    gecko_cmd_le_gap_end_procedure();

    voble_device_found = true;
  }
}
