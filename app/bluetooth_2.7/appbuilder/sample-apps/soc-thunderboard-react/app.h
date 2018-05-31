/***********************************************************************************************//**
 * \file   soc-thunderboard-react/app.h
 * \brief  Application header file
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef APP_H
#define APP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "native_gecko.h"

/***********************************************************************************************//**
 * \defgroup tbr_app Application
 * \brief Sample Application Implementation
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup Thunderboard
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup tbr_app
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Type Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Function Declarations
 **************************************************************************************************/

/**********************************************************************************************//**
 * @brief
 *   Top level initialization of application
 *  return None
 *************************************************************************************************/
void appInit (void);

/**********************************************************************************************//**
 * @brief
 *   Function called by app-ble.c when a connection is opened
 *  param[in]  connection  Connection ID.
 *  param[in]  bonding  Bonding ID.
 *  return  None.
 *************************************************************************************************/
void appConnectionOpenedEvent(uint8_t connection, uint8_t bonding);

/**********************************************************************************************//**
 * @brief
 *   Function called by app-ble.c when a connection is closed
 *  param[in]  connection  Connection ID.
 *  param[in]  reason  Reason for lost connection.
 *  return None
 *************************************************************************************************/
void appConnectionClosedEvent(uint8_t connection, uint16_t reason);

/**********************************************************************************************//**
 * @brief
 *   Function called when advertisement should be stopped
 *  return None
 *************************************************************************************************/
void appAdvTimeoutEvtHandler(void);

/** @} (end addtogroup tbr_app) */
/** @} (end addtogroup Thunderboard) */

#ifdef __cplusplus
};
#endif

#endif // APP_H
