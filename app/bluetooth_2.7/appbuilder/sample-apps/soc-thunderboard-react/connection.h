/***********************************************************************************************//**
 * \file   connection.h
 * \brief  Header file related to connection
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef CONNECTION_H
#define CONNECTION_H

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************//**
 * \defgroup connection Connection
 * \brief Connection API
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup Features
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup connection
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Public Macros and Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Public Function Declarations
 **************************************************************************************************/

/***********************************************************************************************//**
 *  \brief  Indicate that connection has started.
 *  \param[in]  connection  ConnectionId.
 *  \param[in]  bonding  Bonding ID.
 **************************************************************************************************/
void conConnectionStarted(uint8_t connection, uint8_t bonding);

/***********************************************************************************************//**
 *  \brief  Indicate that connection has closed.
 **************************************************************************************************/
void conConnectionClosed(void);

/***********************************************************************************************//**
 *  \brief  Indicate that connection parameters have changed.
 *  \param[in]  connection  ConnectionId.
 *  \param[in]  interval  Interval.
 *  \param[in]  latency  Latency.
 *  \param[in]  timeout  Timeout.
 *  \param[in]  security_mode  Security mode.
 **************************************************************************************************/
void conConnectionParameters(uint8_t connection,
                             uint16_t interval,
                             uint16_t latency,
                             uint16_t timeout,
                             uint8_t security_mode);

/***********************************************************************************************//**
 *  \brief  Get connection Id.
 *  \return  Connection Id.
 **************************************************************************************************/
uint8_t conGetConnectionId(void);

/***********************************************************************************************//**
 *  \brief  Init connection parameters (bonding, etc).
 **************************************************************************************************/
void conConnectionInit();

/** @} (end addtogroup connection) */
/** @} (end addtogroup Features) */

#ifdef __cplusplus
};
#endif

#endif /* CONNECTION_H */
