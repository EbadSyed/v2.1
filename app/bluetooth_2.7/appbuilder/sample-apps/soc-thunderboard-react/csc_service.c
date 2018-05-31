/***********************************************************************************************//**
 * \file   csc_service.c
 * \brief  Cycling Speed and Cadence Service
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* standard library headers */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/* BG stack headers */
#include "bg_types.h"
#include "gatt_db.h"
#include "native_gecko.h"
#include "infrastructure.h"

/* application specific headers */
#include "csc_device.h"
#include "app_timer.h"
#include "connection.h"

/* Own header*/
#include "csc_service.h"

/***********************************************************************************************//**
 * @addtogroup Features
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup csc
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

/* Mapping of AppBuilder specific macros */

/* Cyclic Power Measurement flags */
#define CSC_WHEEL_DATA_PRESENT               1
#define CSC_WHEEL_DATA_NOT_PRESENT           0
#define CSC_WHEEL_DATA_FIELD                 CSC_WHEEL_DATA_PRESENT

#define CSC_CRANK_DATA_PRESENT               2
#define CSC_CRANK_DATA_NOT_PRESENT           0
#define CSC_CRANK_DATA_FIELD                 CSC_CRANK_DATA_NOT_PRESENT

/* Function-like macros */
#define initFlags() do { cscCpIndEnabled = false; cscConfirmed = true; } while (0)

/* Other profile specific macros */

/* CP OP Codes */
/* CSC CP Set Cumulative Value Op Code*/
#define CSC_OP_CODE_SET_CUMUL_VAL            1
/* CSC CP Response Code Op Code*/
#define CSC_OP_CODE_RESP                     16

/* CP Response Codes*/
/* CSC CP Response Value Success*/
#define CSC_CP_RESP_SUCCESS                  1
/* CSC CP Response Value Op Code not supported */
#define CSC_CP_RESP_OC_NOT_SUPP              2
/* CSC Response if Parameter received does not meet the requirements of the service */
#define CSC_RESP_OC_INV_PARAM                3

/* Write response codes*/
/* Write successful */
#define CSC_WRITE_OK                         0
/* Error code - Client Characteristic Configuration descriptor improperly configured */
#define CSC_ERR_CCCD_CONF                    0x81
/* Error code - PROCEDURE ALREADY IN PROGRESS */
#define CSC_ERR_PROC_IN_PROGRESS             0x80

#define SILABS_AF_PLUGIN_CSC_NOTIFICATION_INT 200

/** Cyclic Power Measurement period in ms. */
#define CSC_IND_TIMEOUT                      SILABS_AF_PLUGIN_CSC_NOTIFICATION_INT

/** Indicates currently there is no active connection using this service. */
#define CSC_NO_CONNECTION                    0xFF
/** Default maximum payload length for most PDUs. */
#define CSC_MAX_PAYLOAD_LEN                  20

/***************************************************************************************************
 * Local Type Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

static cscServiceMeas_t cscCyclicSpeedMeas; /* Cyclic Power Measurement */

//#ifdef SILABS_AF_PLUGIN_CSC_WHEEL_DATA_SUP

static bool cscCpIndEnabled; /* CSC CP indications enabled */
/* Indicates whether a confirmation was received for the control point indication */

static bool cscConfirmed = true; /* Confirmation for CP indication received? */

//#endif /* SILABS_AF_PLUGIN_CSC_WHEEL_DATA_SUP */

/***************************************************************************************************
 * Static Function Declarations
 **************************************************************************************************/
static uint8_t cscBuildMeas(uint8_t *pBuf);
static uint8_t cscProcMeas(uint8_t *buf);

/***************************************************************************************************
 * Public Variable Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Public Function Definitions
 **************************************************************************************************/
void cscServiceInit(void)
{
  /* Set all flags of Cycling Speed Measurement characteristic */
  cscCyclicSpeedMeas.flags = (CSC_WHEEL_DATA_FIELD  | CSC_CRANK_DATA_FIELD);

  initFlags(); /* Initialize flags needed for cyclic speed control point */

  /* Make sure timer for cyclic speed measurement is stopped */
  gecko_cmd_hardware_set_soft_timer(0, CSC_SERVICE_TIMER, false);
}

void cscServiceCharStatusChange(uint8_t connection, uint16_t clientConfig)
{
  /* if the new value of CCC is not 0 (either indication or notification enabled)
   *  start cyclic cadence and speed measurement */
  if (clientConfig) {
    cscServiceMeasure(); /* make an initial measurement */
    gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(CSC_IND_TIMEOUT), CSC_SERVICE_TIMER, false);
  } else {
    gecko_cmd_hardware_set_soft_timer(0, CSC_SERVICE_TIMER, false);
  }
}

void cscServiceMeasure(void)
{
  uint8_t cscTempBuffer[CSC_MAX_PAYLOAD_LEN];
  uint8_t length;

  /* check if the connection is still open */
  if (CSC_NO_CONNECTION == conGetConnectionId()) {
    return;
  }

  length = cscProcMeas(cscTempBuffer);

  /* Send notification */
  gecko_cmd_gatt_server_send_characteristic_notification(
    conGetConnectionId(), gattdb_cycling_speed_measurement, length, cscTempBuffer);
}

//#ifdef SILABS_AF_PLUGIN_CSC_WHEEL_DATA_SUP
void cscServiceControlPointWrite(uint8array *writeValue)
{
  uint8_t cscScOpCode;
  uint8_t cscRetBuf[3];
  uint8_t *cscpRetBuf = cscRetBuf;
  uint16_t cscCumulValue;
  uint8_t lengthIn;

  lengthIn = writeValue->len;

  if (false == cscConfirmed) {
    /* Error - No confirmation received from last CP indication */
    gecko_cmd_gatt_server_send_user_write_response(conGetConnectionId(), gattdb_cycling_speed_cp,
                                                   CSC_ERR_PROC_IN_PROGRESS);
  } else if (true == cscCpIndEnabled) {
    /* Construct CP indication */
    gecko_cmd_gatt_server_send_user_write_response(conGetConnectionId(), gattdb_cycling_speed_cp, CSC_WRITE_OK);
    cscScOpCode = writeValue->data[0];

    UINT8_TO_BITSTREAM(cscpRetBuf, CSC_OP_CODE_RESP);
    UINT8_TO_BITSTREAM(cscpRetBuf, cscScOpCode);

    /* Construct response based on incoming Op Code */
    switch (cscScOpCode) {
      case CSC_OP_CODE_SET_CUMUL_VAL:
        /* Check if parameters are OK*/
        if (lengthIn != 5) {
          UINT8_TO_BITSTREAM(cscpRetBuf, CSC_RESP_OC_INV_PARAM);
        } else {
          cscCumulValue = writeValue->data[2] << 8 | writeValue->data[1];
          appHwSetRevCount(cscCumulValue); /* perform Set Cumulative Value procedure */
          UINT8_TO_BITSTREAM(cscpRetBuf, CSC_CP_RESP_SUCCESS);
        }
        break;
      default:
        UINT8_TO_BITSTREAM(cscpRetBuf, CSC_CP_RESP_OC_NOT_SUPP);
        break;
    }

    /* Send CP indication */
    gecko_cmd_gatt_server_send_characteristic_notification(conGetConnectionId(),
                                                           gattdb_cycling_speed_cp, 3, cscRetBuf);

    cscConfirmed = false; /* Indication is not yet confirmed */
  } else {
    /* Error - CCCD indications not yet enabled */
    gecko_cmd_gatt_server_send_user_write_response(conGetConnectionId(), gattdb_cycling_speed_cp,
                                                   CSC_ERR_CCCD_CONF);
  }
}

void cscServiceCpCharStatusChange(uint8_t connection, uint16_t clientConfig)
{
  /* if the new value of CCC is not 0 (either indication or notification enabled) start
   * receiving Control Point commands */
  if (clientConfig) {
    cscCpIndEnabled = true;  /* Set flag to start sending indications */
  } else {
    cscCpIndEnabled = false; /* Set flag to stop sending indications */
  }
}

void  cscServiceConfirmation(uint8_t connection)
{
  cscConfirmed = true; /* make an initial measurement */
}

//#endif /* SILABS_AF_PLUGIN_CSC_WHEEL_DATA_SUP */

/***************************************************************************************************
 * Static Function Definitions
 **************************************************************************************************/

/***********************************************************************************************//**
 *  \brief  Build a cycling speed and cadence measurement characteristic.
 *  \param[in]  pBuf  Pointer to buffer to hold the built cycling speed and cadence characteristic.
 *  \return  Length of pBuf in bytes.
 **************************************************************************************************/
static uint8_t cscBuildMeas(uint8_t *pBuf)
{
  uint8_t *p = pBuf;

  /* Feed Cyclic speed flags into the buffer */
  UINT8_TO_BITSTREAM(p, cscCyclicSpeedMeas.flags);

  /* Feed Cyclic speed characteristic's fields into the buffer if corresponding field is present */
  if (cscCyclicSpeedMeas.flags & CSC_WHEEL_DATA_PRESENT) {
    UINT32_TO_BITSTREAM(p, cscCyclicSpeedMeas.cumWheelRev);
    UINT16_TO_BITSTREAM(p, cscCyclicSpeedMeas.lastWheelRevTime);
  }

  if (cscCyclicSpeedMeas.flags & CSC_CRANK_DATA_PRESENT) {
    UINT16_TO_BITSTREAM(p, cscCyclicSpeedMeas.cumCrankRev);
    UINT16_TO_BITSTREAM(p, cscCyclicSpeedMeas.lastCrankRevTime);
  }

  return (uint8_t)(p - pBuf); /* return length of data to be sent */
}

/***********************************************************************************************//**
 *  \brief  This function is called by the application when the periodic measurement timer expires.
 *  \param[in]  buf  Bytestream to be sent out.
 *  \return  Length of cycling speed and cadence measurement.
 **************************************************************************************************/
static uint8_t cscProcMeas(uint8_t *buf)
{
  int8_t len;

  /* Update module global variables needed for cyclic cadence and speed measurement based on sensor
   * values */
  if (cscCyclicSpeedMeas.flags & CSC_WHEEL_DATA_PRESENT) {
    cscCyclicSpeedMeas.cumWheelRev = appHwReadWheelRevCount();
    cscCyclicSpeedMeas.lastWheelRevTime = appHwReadLastWheelTime();
  }

  if (cscCyclicSpeedMeas.flags & CSC_CRANK_DATA_PRESENT) {
    cscCyclicSpeedMeas.cumCrankRev = appHwReadTotalCrankRevs();
    cscCyclicSpeedMeas.lastCrankRevTime = appHwReadLastCrankRevTime();
  }

  len = cscBuildMeas(buf);  /* Build speed and cadence characteristic */

  return len;
}

/** @} (end addtogroup csc) */
/** @} (end addtogroup Features) */
