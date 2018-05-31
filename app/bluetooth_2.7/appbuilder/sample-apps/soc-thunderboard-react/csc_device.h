/***********************************************************************************************//**
 * \file   csc_device.h
 * \brief  Cycling Speed and Cadence Device
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef CSC_DEVICE_H
#define CSC_DEVICE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************//**
 * \defgroup csc_hw Cycling Speed and Cadence HW
 * \brief Cycling Speed and Cadence Rotation Measurement API
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app_hardware
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup csc_hw
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Public Macros and Definitions
 **************************************************************************************************/

// These macros have been defined to let the csc service use functions in this
// module.
#define  appHwSetRevCount(value)        cscDeviceSetRevCount(value)
#define  appHwReadWheelRevCount()       cscDeviceReadWheelRevCount()
#define  appHwReadLastWheelTime()       cscDeviceReadLastWheelTime()
#define  appHwReadTotalCrankRevs()      cscDeviceReadTotalCrankRevs()
#define  appHwReadLastCrankRevTime()    cscDeviceReadLastCrankRevTime()

/***************************************************************************************************
 * Function Declarations
 **************************************************************************************************/

/**********************************************************************************************//**
 * @brief
 *   Initialize the module.
 * @return
 *   None
 *************************************************************************************************/
void cscDeviceInit(void);

/**********************************************************************************************//**
 * @brief
 *   De-initialize the module.
 * @return
 *   None
 *************************************************************************************************/
void cscDeviceDeInit(void);

/**********************************************************************************************//**
 * @brief
 *   Put the device into sleep.
 * @return
 *   None
 *************************************************************************************************/
void cscDeviceSleep(void);

/*********************************************************************************************//**
 * @brief
 *   Must be called whenever the connection is opened.
 * @return
 *   None
 ************************************************************************************************/
void cscDeviceConnectionOpened(void);

/**********************************************************************************************//**
 * @brief
 *   Must be called whenever the connection is closed.
 * @return
 *   None
 *************************************************************************************************/
void cscDeviceConnectionClosed(void);

/**********************************************************************************************//**
 * @brief
 *   Must be called whenever a device interrupt occurs.
 * @return
 *   None
 *************************************************************************************************/
void cscDeviceInterrupt(void);

/**********************************************************************************************//**
 * @brief
 *   Set the wheel revolution count.
 * @param[in]  value
 *   The revolution count.
 * @return
 *   None
 *************************************************************************************************/
void cscDeviceSetRevCount(uint32_t value);

/**********************************************************************************************//**
 * @brief
 *   Get the wheel revolution count.
 * @return
 *   The revolution count.
 *************************************************************************************************/
uint32_t cscDeviceReadWheelRevCount(void);

/**********************************************************************************************//**
 * @brief
 *   Get the time stamp for the last wheel revolution.
 * @return
 *   The revolution count.
 *************************************************************************************************/
uint16_t cscDeviceReadLastWheelTime(void);

/**********************************************************************************************//**
 * @brief
 *   Get the Crank revolution count.
 * @return
 *   The revolution count.
 *************************************************************************************************/
uint16_t cscDeviceReadTotalCrankRevs(void);

/**********************************************************************************************//**
 * @brief
 *   Get the time stamp for the last crank revolution.
 * @return
 *   The revolution count.
 *************************************************************************************************/
uint16_t cscDeviceReadLastCrankRevTime();

/**********************************************************************************************//**
 * @brief
 *   Cec characteristics changed event handler function.
 * @param[in]  connection
 *   Connection ID.
 * @param[in]  clientConfig
 *   New value of characteristics.
 * @return
 *   None
 *************************************************************************************************/
void cscDeviceCharStatusChange(uint8_t connection, uint16_t clientConfig);

/** @} (end addtogroup csc_hw) */
/** @} (end addtogroup app_hardware) */

#ifdef __cplusplus
};
#endif

#endif // CSC_DEVICE_H
