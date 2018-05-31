/***********************************************************************************************//**
 * \file   soc-thunderboard-react/app_ble.c
 * \brief  Application BLE
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include "app_ble.h"

/* BG stack headers */
#include "bg_types.h"
#include "infrastructure.h"
#include "gatt_db.h"
#include "native_gecko.h"

/* framework headers */

/* application specific headers */
#include "app.h"
#include "app_ble_adv.h"
#include "app_ui.h"
#include "app_timer.h"
#include "app_interrupt.h"
#include "app_ble_adv.h"
#include "accori_device.h"
#include "aluv_device.h"
#include "battery_device.h"
#include "csc_device.h"
#include "rd0057.h"

/* profiles */
#include "connection.h"
#include "battery_service.h"
#include "csc_service.h"
#include "es_service.h"
#include "amblight_service.h"
#include "accori_service.h"
#include "aio_service.h"

#include <stdbool.h>
#include <stdio.h>

/***********************************************************************************************//**
 * @addtogroup Thunderboard
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app_ble
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

#define DEVNAME_NAME_TOO_LONG_ERROR  0x0A
#define DEVNAME_NAME_NOT_FOUND_ERROR 0x0B

#define SYSTEM_ID_BYTES 8

#define DEVNAME_PS_KEY 0x4000

// Connection parameters
#define DEFAULT_CON_PARAM_INTERVAL_MS          50
#define DEFAULT_CON_PARAM_LATENCY               0
#define DEFAULT_CON_PARAM_TIMEOUT_MS         1000

#define CON_PARAM_MAX_MIN_DIFF_MS 20

#define DEFAULT_CON_PARAM_MAX_INTERVAL (DEFAULT_CON_PARAM_INTERVAL_MS / 1.25)
#define DEFAULT_CON_PARAM_MIN_INTERVAL ((DEFAULT_CON_PARAM_INTERVAL_MS - CON_PARAM_MAX_MIN_DIFF_MS) / 1.25)
#define DEFAULT_CON_PARAM_TIMEOUT      (DEFAULT_CON_PARAM_TIMEOUT_MS / 10)

#define UPDATE_CON_PARAM_DELAY_MS 3000

/***************************************************************************************************
 * Local Function Declarations
 **************************************************************************************************/

static void devNameChanged(uint8array *newValue);

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

/***************************************************************************************************
 * Global Variables
 **************************************************************************************************/

AppBleGattServerAttributeValue_t AppBleGattServerAttributeValue[] =
{
  /* Terminators. */
  { gattdb_device_name, devNameChanged },
};

AppBleGattServerUserReadRequest_t AppBleGattServerUserReadRequest[] =
{
  { gattdb_battery_measurement, batteryServiceRead },
  { gattdb_es_humidity, esServiceHumidityRead },
  { gattdb_es_temperature, esServiceTemperatureRead },
  { gattdb_es_uvindex, esServiceUvIndexRead },
  { gattdb_amblight_lux, amblightServiceRead },
  { gattdb_aio_digital_in, aioServiceDigitalInRead },
  { gattdb_aio_digital_out, aioServiceDigitalOutRead }
};

AppBleGattServerUserWriteRequest_t AppBleGattServerUserWriteRequest[] =
{
  { gattdb_accor_cp, accoriServiceCpWrite },
  { gattdb_cycling_speed_cp, cscServiceControlPointWrite },
  { gattdb_aio_digital_out, aioServiceDigitalOutWrite }
};

AppBleGattServerCharStatus_t AppBleGattServerCharStatus[] =
{
  { gattdb_aio_digital_in, aioServiceDigitalInCharStatusChange },
  { gattdb_cycling_speed_measurement, cscServiceCharStatusChange },
  { gattdb_cycling_speed_measurement, cscDeviceCharStatusChange },
  { gattdb_cycling_speed_cp, cscServiceCpCharStatusChange },
  { gattdb_accor_acceleration, accoriServiceAccelerationCharStatusChange },
  { gattdb_accor_acceleration, accoriDeviceAccelerationCharStatusChange },
  { gattdb_accor_orientation, accoriServiceOrientationCharStatusChange },
  { gattdb_accor_orientation, accoriDeviceOrientationCharStatusChange },
  { gattdb_accor_cp, accoriServiceCpCharStatusChange },
  { gattdb_battery_measurement, batteryServiceCharStatusChange }
};

AppBleGattServerConfirmation_t AppBleGattServerConfirmation[] =
{
  { gattdb_cycling_speed_cp, cscServiceConfirmation },
};

size_t AppBleGattServerAttributeValueSize = COUNTOF(AppBleGattServerAttributeValue);
size_t AppBleGattServerUserReadRequestSize = COUNTOF(AppBleGattServerUserReadRequest);
size_t AppBleGattServerUserWriteRequestSize = COUNTOF(AppBleGattServerUserWriteRequest);
size_t AppBleGattServerConfirmationSize = COUNTOF(AppBleGattServerConfirmation);
size_t AppBleGattServerCharStatusSize = COUNTOF(AppBleGattServerCharStatus);

/***************************************************************************************************
 * Local Function Definitions
 **************************************************************************************************/
static void devNameChanged(uint8array *newValue)
{
  char devName[DEVNAME_MAX_LEN + 1];

  memcpy(devName, newValue->data, newValue->len);
  devName[newValue->len] = 0;

  gecko_cmd_flash_ps_save(DEVNAME_PS_KEY, strlen(devName), (uint8_t *)devName);

  appBleAdvSetDevName(devName);

  appBleAdvInit();
}

/***************************************************************************************************
 * Global Function Definitions
 **************************************************************************************************/
void appBleInit(void)
{
  struct gecko_msg_flash_ps_load_rsp_t *psResp;
  char devName[DEVNAME_MAX_LEN + 1];
  uint8_t systemId[SYSTEM_ID_BYTES];

  uint32_t uniqueId;
  uint8_t btAddr[6];
  struct gecko_msg_system_get_bt_address_rsp_t *btAddrRsp;

  // Extract unique ID from BT Address
  btAddrRsp = gecko_cmd_system_get_bt_address();
  memcpy(btAddr, btAddrRsp->address.addr, 6);
  memcpy(&uniqueId, btAddrRsp->address.addr, 3);

  // Pad and reverse unique ID to get System ID
  systemId[0] = btAddr[5];
  systemId[1] = btAddr[4];
  systemId[2] = btAddr[3];
  systemId[3] = 0xFF;
  systemId[4] = 0xFE;
  systemId[5] = btAddr[2];
  systemId[6] = btAddr[1];
  systemId[7] = btAddr[0];

  // Check if Persistent Storage contains name already, if not, store it there
  psResp = gecko_cmd_flash_ps_load(DEVNAME_PS_KEY);
  if (psResp->result) {
    // Key not found
    // Concatenate name with unique ID and zero terminate
    snprintf(devName, DEVNAME_MAX_LEN + 1, DEVNAME_DEFAULT_STRING, (uniqueId & 0xFFFF));

    gecko_cmd_flash_ps_save(DEVNAME_PS_KEY, strlen(devName), (uint8_t *)devName);
  } else {
    memcpy(devName, psResp->value.data, DEVNAME_MAX_LEN + 1);
  }

  appBleAdvSetDevName(devName);

  gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name,
                                              0,
                                              strlen(devName),
                                              (uint8_t*) devName);

  gecko_cmd_gatt_server_write_attribute_value(gattdb_system_id,
                                              0,
                                              SYSTEM_ID_BYTES,
                                              systemId);

  appBleAdvSetId(uniqueId);

  appBleAdvInit();

  conConnectionInit();
  amblightServiceInit();
  accoriServiceInit();
  batteryServiceInit();
  cscServiceInit();
  aioServiceInit();
  esServiceInit();
}

void appBleConnectionClosedEvent(uint8_t connection, uint16_t reason)
{
  accoriServiceConnectionClosed();
  conConnectionClosed();

  appBleAdvStart();

  appConnectionClosedEvent(connection, reason);
}

void appBleConnectionOpenedEvent(uint8_t connection, uint8_t bonding)
{
  accoriServiceConnectionOpened();
  conConnectionStarted(connection, bonding);

  gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(UPDATE_CON_PARAM_DELAY_MS), UPDATE_CON_PARAM_TIMER, true);

  appBleAdvStop();

  appConnectionOpenedEvent(connection, bonding);
}

void appBleHandleEvents(struct gecko_cmd_packet *evt)
{
  uint8_t i; /* Cycle index variable */

  if (NULL == evt) {
    return;
  }

  switch (BGLIB_MSG_ID(evt->header)) {
    /* This event indicates the device has started and is ready to receive any command except
     *  Bluetooth-related commands */
    case gecko_evt_system_boot_id:
      appInit();
      break;

    /* This event indicates that a connection was closed. */
    case gecko_evt_le_connection_closed_id:
      appBleConnectionClosedEvent(evt->data.evt_le_connection_closed.connection,
                                  evt->data.evt_le_connection_closed.reason);
      break;

    /* This event is triggered after the connection has been opened */
    case gecko_evt_le_connection_opened_id:
      appBleConnectionOpenedEvent(evt->data.evt_le_connection_opened.connection,
                                  evt->data.evt_le_connection_opened.bonding);

      break;

    /* Value of attribute changed from the local database by remote GATT client */
    case gecko_evt_gatt_server_attribute_value_id:
      for (i = 0; i < AppBleGattServerAttributeValueSize; i++) {
        if ((AppBleGattServerAttributeValue[i].charId
             == evt->data.evt_gatt_server_attribute_value.attribute)
            && (AppBleGattServerAttributeValue[i].fctn)) {
          AppBleGattServerAttributeValue[i].fctn(
            &(evt->data.evt_gatt_server_attribute_value.value));
        }
      }
      break;

    /* Indicates the changed value of CCC or received characteristic confirmation */
    case gecko_evt_gatt_server_characteristic_status_id:
      /* Char status changed */
      if (evt->data.evt_gatt_server_characteristic_status.status_flags == 0x01) {
        for (i = 0; i < AppBleGattServerCharStatusSize; i++) {
          if ((AppBleGattServerCharStatus[i].charId
               == evt->data.evt_gatt_server_characteristic_status.characteristic)
              && (AppBleGattServerCharStatus[i].fctn)) {
            AppBleGattServerCharStatus[i].fctn(
              evt->data.evt_gatt_server_characteristic_status.connection,
              evt->data.evt_gatt_server_characteristic_status.client_config_flags);
          }
        }
      }
      /* Confirmation received */
      else if ((evt->data.evt_gatt_server_characteristic_status.status_flags == 0x02)
               /* must be a response to an indication*/
               && (evt->data.evt_gatt_server_characteristic_status.client_config_flags == 2)) {
        for (i = 0; i < AppBleGattServerConfirmationSize; i++) {
          if ((AppBleGattServerConfirmation[i].charId
               == evt->data.evt_gatt_server_characteristic_status.characteristic)
              && (AppBleGattServerConfirmation[i].fctn)) {
            AppBleGattServerConfirmation[i].fctn(
              evt->data.evt_gatt_server_characteristic_status.connection);
          }
        }
      }
      break;

    /* This event indicates that a remote GATT client is attempting to read a value of an
     *  attribute from the local GATT database, where the attribute was defined in the GATT
     *  XML firmware configuration file to have type="user". */
    case gecko_evt_gatt_server_user_read_request_id:
      for (i = 0; i < AppBleGattServerUserReadRequestSize; i++) {
        if ((AppBleGattServerUserReadRequest[i].charId
             == evt->data.evt_gatt_server_user_read_request.characteristic)
            && (AppBleGattServerUserReadRequest[i].fctn)) {
          AppBleGattServerUserReadRequest[i].fctn();
        }
      }
      break;

    /* This event indicates that a remote GATT client is attempting to write a value of an
     * attribute in to the local GATT database, where the attribute was defined in the GATT
     * XML firmware configuration file to have type="user".  */
    case gecko_evt_gatt_server_user_write_request_id:
      for (i = 0; i < AppBleGattServerUserWriteRequestSize; i++) {
        if ((AppBleGattServerUserWriteRequest[i].charId
             == evt->data.evt_gatt_server_characteristic_status.characteristic)
            && (AppBleGattServerUserWriteRequest[i].fctn)) {
          AppBleGattServerUserWriteRequest[i].fctn(
            &(evt->data.evt_gatt_server_attribute_value.value));
        }
      }
      break;

    /* SW Timer event */
    case gecko_evt_hardware_soft_timer_id:
      switch (evt->data.evt_hardware_soft_timer.handle) {
        case UI_TIMER:
          appUiLedBlinkEvtHandler();
          break;

        case ADV_TIMEOUT_TIMER:
          appAdvTimeoutEvtHandler();
          break;

        case ADV_ALTERNATE_TIMER:
          appBleAdvAlternateEvtHandler();
          break;

        case UPDATE_CON_PARAM_TIMER:
          appBleUpdateConParamEvtHandler();
          break;

        case ACCORI_SERVICE_ACC_TIMER:
          accoriServiceAccelerationTimerEvtHandler();
          break;

        case ACCORI_SERVICE_ORI_TIMER:
          accoriServiceOrientationTimerEvtHandler();
          break;

        case BATTERY_DEVICE_TIMER:
          batteryDeviceMeasureEvtHandler();
          break;

        case BATT_SERVICE_TIMER:
          batteryServiceMeasure();
          break;

        case CSC_SERVICE_TIMER:
          cscServiceMeasure();
          break;

        default:
          break;
      }
      break;

    case gecko_evt_system_external_signal_id:

      if (evt->data.evt_system_external_signal.extsignals & EXTSIGNAL_BUTTON0) {
        boardButtonInterrupt(0);
      }
      if (evt->data.evt_system_external_signal.extsignals & EXTSIGNAL_BUTTON1) {
        boardButtonInterrupt(1);
      }
      if (evt->data.evt_system_external_signal.extsignals & EXTSIGNAL_ACCGYRO) {
        accoriDeviceInterruptEvtHandler();
      }
      if (evt->data.evt_system_external_signal.extsignals & EXTSIGNAL_HALL) {
        cscDeviceInterrupt();
      }
      if (evt->data.evt_system_external_signal.extsignals & EXTSIGNAL_ALUV) {
        aluvDeviceInterruptEvtHandler();
      }
      break;
  }
}

void appBleUpdateConParamEvtHandler(void)
{
  gecko_cmd_le_connection_set_parameters(conGetConnectionId(),
                                         DEFAULT_CON_PARAM_MIN_INTERVAL,
                                         DEFAULT_CON_PARAM_MAX_INTERVAL,
                                         DEFAULT_CON_PARAM_LATENCY,
                                         DEFAULT_CON_PARAM_TIMEOUT);
}

/** @} (end addtogroup app_ble) */
/** @} (end addtogroup Thunderboard) */
