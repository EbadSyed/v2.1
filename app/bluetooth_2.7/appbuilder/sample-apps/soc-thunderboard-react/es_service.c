/***********************************************************************************************//**
 * \file   es_ervice.c
 * \brief  Environmental Sensing Service
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include "es_service.h"

/* BG stack headers */
#include "gatt_db.h"
#include "native_gecko.h"
#include "infrastructure.h"
#include "connection.h"

/* application specific headers */
#include "rht_device.h"
#include "aluv_device.h"

/***********************************************************************************************//**
 * @addtogroup Features
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup es
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

#define ES_HUMIDITY_PAYLOAD_LEN           2
#define ES_TEMPERATURE_PAYLOAD_LEN        2
#define ES_UVINDEX_PAYLOAD_LEN            1

/***************************************************************************************************
 * Local Type Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Local Variable Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Public Variable Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Static Function Definitions
 **************************************************************************************************/
static void humidityReadReady(uint16_t humidity)
{
  uint8_t buffer[ES_HUMIDITY_PAYLOAD_LEN];
  uint8_t *p = buffer;

  UINT16_TO_BITSTREAM(p, humidity);
  gecko_cmd_gatt_server_send_user_read_response(conGetConnectionId(),
                                                gattdb_es_humidity,
                                                0,
                                                ES_HUMIDITY_PAYLOAD_LEN,
                                                buffer);
}

static void temperatureReadReady(int16_t temperature)
{
  uint8_t buffer[ES_TEMPERATURE_PAYLOAD_LEN];
  uint8_t *p = buffer;

  UINT16_TO_BITSTREAM(p, (uint16_t) temperature);
  gecko_cmd_gatt_server_send_user_read_response(conGetConnectionId(),
                                                gattdb_es_temperature,
                                                0,
                                                ES_TEMPERATURE_PAYLOAD_LEN,
                                                buffer);
}

static void uviReadReady(uint8_t uvIndex)
{
  gecko_cmd_gatt_server_send_user_read_response(conGetConnectionId(),
                                                gattdb_es_uvindex,
                                                0,
                                                ES_UVINDEX_PAYLOAD_LEN,
                                                &uvIndex);
}

/***************************************************************************************************
 * Public Function Definitions
 **************************************************************************************************/
void esServiceInit(void)
{
}

void esServiceHumidityRead(void)
{
  rhtDeviceHumidityMeasure(&humidityReadReady);
}

void esServiceTemperatureRead(void)
{
  rhtDeviceTemperatureMeasure(&temperatureReadReady);
}

void esServiceUvIndexRead(void)
{
  aluvDeviceUviMeasure(&uviReadReady);
}

/** @} (end addtogroup es) */
/** @} (end addtogroup Features) */
