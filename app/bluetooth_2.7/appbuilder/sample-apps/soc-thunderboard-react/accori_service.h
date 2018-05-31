/***********************************************************************************************//**
 * \file   accori_service.h
 * \brief  Acceleration and Orientation Service
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef ACCOR_H
#define ACCOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "bg_types.h"

/***********************************************************************************************//**
 * \defgroup accor Acceleration and Orientation
 * \brief Acceleration and Orientation Service API
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup Features
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup accor
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Public Macros and Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Function Declarations
 **************************************************************************************************/

/**********************************************************************************************//**
 * \brief  Initialize the module.
 *************************************************************************************************/
void accoriServiceInit(void);

/**********************************************************************************************//**
 * \brief  De-initialize the module.
 *************************************************************************************************/
void accoriServiceDeInit(void);

/**********************************************************************************************//**
 * \brief  Must be called whenever the connection is opened.
 *************************************************************************************************/
void accoriServiceConnectionOpened(void);

/**********************************************************************************************//**
 * \brief  Must be called whenever the connection is closed.
 *************************************************************************************************/
void accoriServiceConnectionClosed(void);

/**********************************************************************************************//**
 * \brief  Acceleration characteristics changed event handler function.
 * \param[in]  connection  Connection ID.
 * \param[in]  clientConfig  New value of characteristics.
 *************************************************************************************************/
void accoriServiceAccelerationCharStatusChange(uint8_t connection, uint16_t clientConfig);

/**********************************************************************************************//**
 * \brief  Orientation characteristics changed event handler function.
 * \param[in]  connection  Connection ID.
 * \param[in]  clientConfig  New value of characteristics.
 *************************************************************************************************/
void accoriServiceOrientationCharStatusChange(uint8_t connection, uint16_t clientConfig);

/**********************************************************************************************//**
 * \brief  Control Point characteristics changed event handler function.
 * \param[in]  connection  Connection ID.
 * \param[in]  clientConfig  New value of characteristics.
 *************************************************************************************************/
void accoriServiceCpCharStatusChange(uint8_t connection, uint16_t clientConfig);

/**********************************************************************************************//**
 * \brief  Control Point write, used to start a control point function.
 * \param[in]  writeValue  The function ID. 0x01=Start calibration, 0x02=Reset orientation
 *************************************************************************************************/
void accoriServiceCpWrite(uint8array *writeValue);

/**********************************************************************************************//**
 * \brief  Event to handle periodic acceleration measurements
 *************************************************************************************************/
void accoriServiceAccelerationTimerEvtHandler(void);

/**********************************************************************************************//**
 * \brief  Event to handle periodic orientation measurements
 *************************************************************************************************/
void accoriServiceOrientationTimerEvtHandler(void);

/** @} (end addtogroup accor) */
/** @} (end addtogroup Features) */

#ifdef __cplusplus
};
#endif

#endif /* ACCOR_H */
