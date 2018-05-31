/***********************************************************************************************//**
 * \file   csc_service.h
 * \brief  Cycling Power Service
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef CSC_SERVICE_H
#define CSC_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************//**
 * \defgroup csc Cycling Speed and Cadence
 * \brief Cycling Speed and Cadence Service API
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup Features
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup csc
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Public Macros and Definitions
 **************************************************************************************************/
/** Cyclic Power Measurement structure. */
typedef struct {
  uint8_t flags;                /**< Flags */
  uint32_t cumWheelRev;          /**< Cumulative Wheel Revolutions */
  uint16_t lastWheelRevTime;     /**< Last Wheel Event Time */
  uint16_t cumCrankRev;          /**< Cumulative Crank Revolutions */
  uint16_t lastCrankRevTime;     /**< Last Crank Event Time */
} cscServiceMeas_t;

/***************************************************************************************************
 * Function Declarations
 **************************************************************************************************/

/***********************************************************************************************//**
 *  \brief  Initialise Cycling Speed and Cadence Service.
 *  \details  Initialise the connection ID, the configuration flags of the Cycling Speed and Cadence
 *  and stop Cycling Speed and Cadence measurement timer.
 **************************************************************************************************/
void cscServiceInit(void);

/***********************************************************************************************//**
 *  \brief  Cycling Speed and Cadence CCCD has changed event handler function.
 *  \param[in]  connection  Connection ID.
 *  \param[in]  clientConfig  New value of CCCD.
 **************************************************************************************************/
void cscServiceCharStatusChange(uint8_t connection, uint16_t clientConfig);

/***********************************************************************************************//**
 *  \brief  Make one Cycling Speed and Cadence measurement.
 **************************************************************************************************/
void cscServiceMeasure(void);

/***********************************************************************************************//**
 *  \brief  Write procedure performed on the CSC Control Point characteristic.
 *  \param[in]  writeValue  New characteristic value.
 **************************************************************************************************/
void cscServiceControlPointWrite(uint8array *writeValue);

/***********************************************************************************************//**
 *  \brief  CSC Control Point CCCD has changed event handler function.
 *  \param[in]  connection  Connection ID.
 *  \param[in]  clientConfig  New value of CCCD.
 **************************************************************************************************/
void cscServiceCpCharStatusChange(uint8_t connection, uint16_t clientConfig);

/***********************************************************************************************//**
 *  \brief  CSC Control Point Indication has been Confirmed (the master sent us the acknowledgement).
 *  \param[in]  connection  Connection ID.
 **************************************************************************************************/
void cscServiceConfirmation(uint8_t connection);

/** @} (end addtogroup csc) */
/** @} (end addtogroup Features) */

#ifdef __cplusplus
};
#endif

#endif /* CSC_SERVICE_H */
