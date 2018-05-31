/***********************************************************************************************//**
 * \file   accori_service.c
 * \brief  Acceleration and Orientation Service
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include "accori_service.h"
#include <stdbool.h>

/* BG stack headers */
#include "gatt_db.h"
#include "native_gecko.h"
#include "infrastructure.h"

/* framework specific headers */

/* profiles */
#include "connection.h"

/* application specific headers */
#include "app_timer.h"
#include "accori_device.h"

/***********************************************************************************************//**
 * @addtogroup Features
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup accor
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

// Measurement periodes in ms.
#define ACCELERATION_MEASUREMENT_PERIOD             200
#define ORIENTATION_MEASUREMENT_PERIOD              200

// Number of axis for acceleration and orientation
#define ACC_AXIS                               3
#define ORI_AXIS                               3

// Acceleration and orientation axis payload length in bytes
#define ACC_AXIS_PAYLOAD_LENGTH                2
#define ORI_AXIS_PAYLOAD_LENGTH                2

// Inertial payload length in bytes
#define ACCELERATION_PAYLOAD_LENGTH  (ACC_AXIS * ACC_AXIS_PAYLOAD_LENGTH)
#define ORIENTATION_PAYLOAD_LENGTH   (ORI_AXIS * ORI_AXIS_PAYLOAD_LENGTH)

// Indicates currently there is no active connection using this service.
// #define NO_CONNECTION                0xFF

#define CP_OPCODE_CALIBRATE             0x01
#define CP_OPCODE_ORIRESET              0x02
#define CP_OPCODE_RESPONSE              0x10
#define CP_OPCODE_CALRESET              0x64

#define CP_RESP_SUCCESS                 0x01
#define CP_RESP_ERROR                   0x02

// Error codes
// Client Characteristic Configuration descriptor improperly configured
#define ERR_CCCD_CONF                   0x81

/***************************************************************************************************
 * Local Type Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

static bool cpIndication = false;
static bool accelerationNotification = false;
static bool orientationNotification  = false;

/***************************************************************************************************
 * Public Variable Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Static Function Definitions
 **************************************************************************************************/
static void calibrateDone(void)
{
  uint8_t respBuf[3];
  uint8_t *respBufp = respBuf;

  if (cpIndication) {
    UINT8_TO_BITSTREAM(respBufp, CP_OPCODE_RESPONSE);
    UINT8_TO_BITSTREAM(respBufp, CP_OPCODE_CALIBRATE);
    UINT8_TO_BITSTREAM(respBufp, CP_RESP_SUCCESS);
    gecko_cmd_gatt_server_send_characteristic_notification(conGetConnectionId(),
                                                           gattdb_accor_cp,
                                                           3,
                                                           respBuf);
  }
}

/***************************************************************************************************
 * Public Function Definitions
 **************************************************************************************************/
void accoriServiceInit(void)
{
  gecko_cmd_hardware_set_soft_timer(0, ACCORI_SERVICE_ACC_TIMER, false);
  gecko_cmd_hardware_set_soft_timer(0, ACCORI_SERVICE_ORI_TIMER, false);

  accelerationNotification = false;
  orientationNotification  = false;
}

void accoriServiceConnectionOpened(void)
{
}

void accoriServiceConnectionClosed(void)
{
  gecko_cmd_hardware_set_soft_timer(0, ACCORI_SERVICE_ACC_TIMER, false);
  gecko_cmd_hardware_set_soft_timer(0, ACCORI_SERVICE_ORI_TIMER, false);
  accelerationNotification = false;
  orientationNotification  = false;
}

void accoriServiceAccelerationCharStatusChange(uint8_t connection,
                                               uint16_t clientConfig)
{
  accelerationNotification = (clientConfig > 0);
  if (accelerationNotification) {
    gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(ACCELERATION_MEASUREMENT_PERIOD), ACCORI_SERVICE_ACC_TIMER, false);
  } else {
    gecko_cmd_hardware_set_soft_timer(0, ACCORI_SERVICE_ACC_TIMER, false);
  }
}

void accoriServiceOrientationCharStatusChange(uint8_t connection,
                                              uint16_t clientConfig)
{
  orientationNotification = (clientConfig > 0);
  if (orientationNotification) {
    gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(ORIENTATION_MEASUREMENT_PERIOD), ACCORI_SERVICE_ORI_TIMER, false);
  } else {
    gecko_cmd_hardware_set_soft_timer(0, ACCORI_SERVICE_ORI_TIMER, false);
  }
}

void accoriServiceCpCharStatusChange(uint8_t connection,
                                     uint16_t clientConfig)
{
  /* Enable / disable indications */
  cpIndication = (clientConfig > 0);
}

void accoriServiceAccelerationTimerEvtHandler(void)
{
  int16_t accX, accY, accZ;

  accoriDeviceAccelerationRead(&accX, &accY, &accZ);

  uint8_t buffer[ACCELERATION_PAYLOAD_LENGTH];

  uint8_t *p = buffer;

  UINT16_TO_BITSTREAM(p, (uint16_t)accX);
  UINT16_TO_BITSTREAM(p, (uint16_t)accY);
  UINT16_TO_BITSTREAM(p, (uint16_t)accZ);

  if (!accelerationNotification) {
    return;
  }

  gecko_cmd_gatt_server_send_characteristic_notification(
    conGetConnectionId(),
    gattdb_accor_acceleration,
    ACCELERATION_PAYLOAD_LENGTH,
    buffer);
}

void accoriServiceOrientationTimerEvtHandler(void)
{
  int16_t oriX, oriY, oriZ;

  accoriDeviceOrientationRead(&oriX, &oriY, &oriZ);

  uint8_t buffer[ORIENTATION_PAYLOAD_LENGTH];

  uint8_t *p = buffer;

  UINT16_TO_BITSTREAM(p, (uint16_t)oriX);
  UINT16_TO_BITSTREAM(p, (uint16_t)oriY);
  UINT16_TO_BITSTREAM(p, (uint16_t)oriZ);

  if (!orientationNotification) {
    return;
  }

  gecko_cmd_gatt_server_send_characteristic_notification(
    conGetConnectionId(),
    gattdb_accor_orientation,
    ORIENTATION_PAYLOAD_LENGTH,
    buffer);
}

void accoriServiceCpWrite(uint8array *writeValue)
{
  uint8_t respBuf[3];
  uint8_t *respBufp = respBuf;
  if (cpIndication) {
    gecko_cmd_gatt_server_send_user_write_response(conGetConnectionId(),
                                                   gattdb_accor_cp,
                                                   0);

    UINT8_TO_BITSTREAM(respBufp, CP_OPCODE_RESPONSE);
    UINT8_TO_BITSTREAM(respBufp, writeValue->data[0]);

    switch (writeValue->data[0]) {
      case CP_OPCODE_CALIBRATE:
        accoriDeviceCalibrate(&calibrateDone);
        // Response is sent by calibrateDone()
        break;

      case CP_OPCODE_ORIRESET:
        accoriDeviceOrientationReset();
        UINT8_TO_BITSTREAM(respBufp, CP_RESP_SUCCESS);
        gecko_cmd_gatt_server_send_characteristic_notification(conGetConnectionId(),
                                                               gattdb_accor_cp,
                                                               3,
                                                               respBuf);
        break;

      case CP_OPCODE_CALRESET:
        accoriDeviceCalibrateReset();
        UINT8_TO_BITSTREAM(respBufp, CP_RESP_SUCCESS);

        gecko_cmd_gatt_server_send_characteristic_notification(conGetConnectionId(),
                                                               gattdb_accor_cp,
                                                               3,
                                                               respBuf);
        break;

      default:
        UINT8_TO_BITSTREAM(respBufp, CP_RESP_ERROR);
        gecko_cmd_gatt_server_send_characteristic_notification(conGetConnectionId(),
                                                               gattdb_accor_cp,
                                                               3,
                                                               respBuf);
        break;
    }
  } else {
    gecko_cmd_gatt_server_send_user_write_response(conGetConnectionId(),
                                                   gattdb_accor_cp,
                                                   ERR_CCCD_CONF);
  }
}

/** @} (end addtogroup accor) */
/** @} (end addtogroup Features) */
