/***********************************************************************************************//**
 * \file   aio_service.h
 * \brief  Automation I/O Service
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef AIO_SERVICE_H
#define AIO_SERVICE_H

#include "bg_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************//**
 * \defgroup aio Automation IO
 * \brief Automation IO Service API
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup Features
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup aio
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Public Macros and Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Function Declarations
 **************************************************************************************************/

/**********************************************************************************************//**
 * @brief
 *   Initialize the module.
 * @return
 *   None
 *************************************************************************************************/
void aioServiceInit(void);

/**********************************************************************************************//**
 * @brief
 *   De-initialize the module.
 * @return
 *   None
 *************************************************************************************************/
void aioServiceDeInit(void);

/**********************************************************************************************//**
 * @brief
 *   Must be called whenever the connection is opened.
 * @return
 *   None
 *************************************************************************************************/
void aioServiceConnectionOpened(void);

/**********************************************************************************************//**
 * @brief
 *   Must be called whenever the connection is closed.
 * @return
 *   None
 *************************************************************************************************/
void aioServiceConnectionClosed(void);

/**********************************************************************************************//**
 * @brief
 *   Digital input characteristics changed event handler function.
 * @param[in]  connection
 *   Connection ID.
 * @param[in]  clientConfig
 *   New value of characteristics.
 * @return
 *   None
 *************************************************************************************************/
void aioServiceDigitalInCharStatusChange(uint8_t connection, uint16_t clientConfig);

/**********************************************************************************************//**
 * @brief
 *   Digital output write, used to set digital outputs.
 * @param[in]  writeValue
 *   Array of 2 2-bit values.
 *   0b00 = Inactive state, 0b01 = Active state, 0b10 = Tri state, 0b11 = Unknown
 * @return
 *   None
 *************************************************************************************************/
void aioServiceDigitalOutWrite(uint8array *writeValue);

/**********************************************************************************************//**
 * @brief
 *   Digital output read, used to get digital inputs.
 * @return
 *   None
 *************************************************************************************************/
void aioServiceDigitalOutRead(void);

/**********************************************************************************************//**
 * @brief
 *   Must be called whenever a digital input changes state. Will notify
 *   subscribers about the current state.
 * @return
 *   None
 *************************************************************************************************/
void aioServiceDigitalInUpdate(void);

/**********************************************************************************************//**
 * @brief
 *   Digital input read, used to get digital inputs.
 * @return
 *   None
 *************************************************************************************************/
void aioServiceDigitalInRead(void);

/** @} (end addtogroup aio) */
/** @} (end addtogroup Features) */

#ifdef __cplusplus
};
#endif

#endif // AIO_SERVICE_H
