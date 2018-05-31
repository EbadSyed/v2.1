/***********************************************************************************************//**
 * \file   aio_service.c
 * \brief  Automation I/O Service
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include "aio_service.h"
/* standard library headers */
#include <stdbool.h>

/* BG stack headers */
#include "gatt_db.h"
#include "native_gecko.h"
#include "infrastructure.h"

#include "aio_device.h"
#include "connection.h"

/***********************************************************************************************//**
 * @addtogroup Features
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup aio
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

/** Indicates currently there is no active connection using this service. */
#define AIO_NO_CONNECTION               0xFF

/***************************************************************************************************
 * Local Type Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

/***************************************************************************************************
 * Static Function Declarations
 **************************************************************************************************/

static bool digitalInNotification = false;

/***************************************************************************************************
 * Public Function Definitions
 **************************************************************************************************/
void aioServiceInit(void)
{
}

void aioServiceConnectionOpened(void)
{
}

void aioServiceConnectionClosed(void)
{
  digitalInNotification = false;
}

void aioServiceDigitalInCharStatusChange(uint8_t connection, uint16_t clientConfig)
{
  /* If the new value of CCC is not 0 (either indication or notification enabled) */
  if (clientConfig) {
    digitalInNotification = true;
  } else {
    digitalInNotification = false;
  }
}

void aioServiceDigitalOutWrite(uint8array *writeValue)
{
  uint8_t i;
  AioDigitalState_t aioDigitalOutStates[AIO_NUMBER_OF_DIGITAL_OUTPUTS];

  for (i = 0; i < AIO_NUMBER_OF_DIGITAL_OUTPUTS; i++) {
    aioDigitalOutStates[i] = (AioDigitalState_t)
                             ((writeValue->data[0] >> (AIO_STATE_NUMBER_OF_BITS * i))
                              & AIO_STATE_BITS_MASK);
  }
  aioDeviceDigitalOutWrite(aioDigitalOutStates);

  gecko_cmd_gatt_server_send_user_write_response(conGetConnectionId(),
                                                 gattdb_aio_digital_out,
                                                 0);
}

void aioServiceDigitalOutRead(void)
{
  uint8_t i;
  uint8_t outStates = 0;

  AioDigitalState_t aioDigitalOutStates[AIO_NUMBER_OF_DIGITAL_OUTPUTS];

  aioDeviceDigitalOutRead(aioDigitalOutStates);

  for (i = 0; i < AIO_NUMBER_OF_DIGITAL_OUTPUTS; i++) {
    outStates |= aioDigitalOutStates[i] << (AIO_STATE_NUMBER_OF_BITS * i);
  }

  gecko_cmd_gatt_server_send_user_read_response(conGetConnectionId(),
                                                gattdb_aio_digital_out,
                                                0,
                                                1,
                                                &outStates);
}

void aioServiceDigitalInUpdate(void)
{
  uint8_t i;
  uint8_t inStates = 0;

  AioDigitalState_t aioDigitalInStates[AIO_NUMBER_OF_DIGITAL_INPUTS];

  aioDeviceDigitalInRead(aioDigitalInStates);

  for (i = 0; i < AIO_NUMBER_OF_DIGITAL_INPUTS; i++) {
    inStates |= aioDigitalInStates[i] << (AIO_STATE_NUMBER_OF_BITS * i);
  }

  // Notify any subscribers
  if (digitalInNotification) {
    gecko_cmd_gatt_server_send_characteristic_notification(
      conGetConnectionId(),
      gattdb_aio_digital_in,
      AIO_DIGITAL_INPUT_PAYLOAD_LEN,
      &inStates);
  }
}

void aioServiceDigitalInRead(void)
{
  uint8_t i;
  uint8_t inStates = 0;

  AioDigitalState_t aioDigitalInStates[AIO_NUMBER_OF_DIGITAL_INPUTS];

  aioDeviceDigitalInRead(aioDigitalInStates);

  for (i = 0; i < AIO_NUMBER_OF_DIGITAL_INPUTS; i++) {
    inStates |= aioDigitalInStates[i] << (AIO_STATE_NUMBER_OF_BITS * i);
  }

  gecko_cmd_gatt_server_send_user_read_response(conGetConnectionId(),
                                                gattdb_aio_digital_in,
                                                0,
                                                1,
                                                &inStates);
}

/***************************************************************************************************
 * Static Function Definitions
 **************************************************************************************************/

/** @} (end addtogroup aio) */
/** @} (end addtogroup Features) */
