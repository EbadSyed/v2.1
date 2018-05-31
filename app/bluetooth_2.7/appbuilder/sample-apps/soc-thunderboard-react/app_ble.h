/***********************************************************************************************//**
 * \file   soc-thunderboard-react/app_ble.h
 * \brief  Application BLE
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef APP_BLE_H
#define APP_BLE_H

#include "native_gecko.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************//**
 * \defgroup app_ble Application Configuration
 * \brief Application configuration file.
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup Thunderboard
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app_ble
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Public Macros and Definitions
 **************************************************************************************************/

#define DEVNAME_MAX_LEN 20

/* Must match length of APP_DEVNAME_DEFAULT after printf formatting */
#define DEVNAME_DEFAULT_STRING       "Thunder React #%05lu"
#define DEVNAME_DEFAULT              "Thunder React #00000"

/* Subtract 1 because of terminating NULL character */
#define DEVNAME_DEFAULT_LEN         (sizeof(DEVNAME_DEFAULT) - 1)

/***************************************************************************************************
 * Type Definitions
 **************************************************************************************************/

/** GATT Server Attribute Value Write Configuration.
 *  Structure to register handler functions to characteristic write events. */
typedef struct {
  uint16_t charId;                      /**< ID of the Characteristic. */
  void (*fctn)(uint8array *writeValue); /**< Handler function. */
} AppBleGattServerAttributeValue_t;

/** GATT Server Attribute User Read Configuration.
 *  Structure to register handler functions to user read events. */
typedef struct {
  uint16_t charId;                      /**< ID of the Characteristic. */
  void (*fctn)(void);                   /**< Handler function. */
} AppBleGattServerUserReadRequest_t;

/** GATT Server Attribute  User Write Configuration.
 *  Structure to register handler functions to user write events. */
typedef struct {
  uint16_t charId;                      /**< ID of the Characteristic. */
  void (*fctn)(uint8array *writeValue); /**< Handler function. */
} AppBleGattServerUserWriteRequest_t;

/** GATT Server CCC Status Change Configuration.
 *  Structure to register handler functions to client characteristic
 *  configuration status change events. */
typedef struct {
  uint16_t charId;                                          /**< ID of the Characteristic. */
  void (*fctn)(uint8_t connection, uint16_t clientConfig);  /**< Handler function. */
} AppBleGattServerCharStatus_t;

/** GATT Server Confirmation.
 *  Structure to register handler functions to be called when a confirmation is received. */
typedef struct {
  uint16_t charId;                      /**< ID of the Characteristic. */
  void (*fctn)(uint8_t connection);     /**< Handler function. */
} AppBleGattServerConfirmation_t;

/***************************************************************************************************
 * Extern Variables
 **************************************************************************************************/

extern AppBleGattServerAttributeValue_t AppBleGattServerAttributeValue[];
extern AppBleGattServerUserReadRequest_t AppBleGattServerUserReadRequest[];
extern AppBleGattServerUserWriteRequest_t AppBleGattServerUserWriteRequest[];
extern AppBleGattServerConfirmation_t AppBleGattServerConfirmation[];
extern AppBleGattServerCharStatus_t AppBleGattServerCharStatus[];

extern size_t AppBleGattServerAttributeValueSize;
extern size_t AppBleGattServerUserReadRequestSize;
extern size_t AppBleGattServerUserWriteRequestSize;
extern size_t AppBleGattServerConfirmationSize;
extern size_t AppBleGattServerCharStatusSize;

/***************************************************************************************************
 * Public Function Declarations
 **************************************************************************************************/

/**********************************************************************************************//**
 * @brief
 *   Application BLE initialization called by app.c. This function extracts the
 *  unique ID of the device and applies this to the System ID characteristic
 *  in the Device Information Service, along with applying it to the iBeacon
 *  payload and the local/device name used in the Device Information Service
 *  and the advertisement payload.
 *  return None
 *************************************************************************************************/
void appBleInit(void);

/**********************************************************************************************//**
 * @brief
 *   Function called by when a connection is closed.
 *  param[in]  connection  Connection ID.
 *  param[in]  reason  Reason for lost connection.
 *  return None
 *************************************************************************************************/
void appBleConnectionClosedEvent(uint8_t connection, uint16_t reason);

/**********************************************************************************************//**
 * @brief
 *   Function called when a connection is opened.
 *  param[in]  connection  Connection ID.
 *  param[in]  bonding  Bonding ID.
 *  return  None.
 *************************************************************************************************/
void appBleConnectionOpenedEvent(uint8_t connection, uint8_t bonding);

/**********************************************************************************************//**
 * @brief
 *  Function to handle a BLE event. The function checks if there is a valid event and handles it
 *  accordingly.
 *  param[in]  evt  Event packet
 *  return  None.
 *************************************************************************************************/
void appBleHandleEvents(struct gecko_cmd_packet *evt);

/**********************************************************************************************//**
 * @brief
 *  Function to send an connection parameter update request to the master.
 *  This function should only be called when in a connected state.
 *  return  None.
 *************************************************************************************************/
void appBleUpdateConParamEvtHandler(void);

/** @} (end addtogroup app_ble) */
/** @} (end addtogroup Thunderboard) */

#ifdef __cplusplus
};
#endif

#endif /* APP_BLE_H */
