/***********************************************************************************************//**
 * \file   battery_device.h
 * \brief  Battery Device
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef BATTERY_HW_H
#define BATTERY_HW_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************//**
 * \defgroup battery-hw Battery HW
 * \brief Battery Service HW API
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app_hardware
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup battery-hw
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Public Macros and Definitions
 **************************************************************************************************/

/***********************************************************************************************//**
 *  @brief
 *    This macro has been defined to let the batt service read the battery
 *    level with the function in this module.
 **************************************************************************************************/
#define appHwReadBatteryLevel()         batteryDeviceReadBatteryLevel()

/***************************************************************************************************
 * Function Declarations
 **************************************************************************************************/

/***********************************************************************************************//**
 *  @brief
 *    Initialize the module.
 **************************************************************************************************/
void batteryDeviceInit(void);

/***********************************************************************************************//**
 *  @brief
 *    De-initialize the module.
 **************************************************************************************************/
void batteryDeviceDeInit(void);

/***********************************************************************************************//**
 *  @brief
 *    Must be called when a connectin is opened. It starts polling the battery
 *    voltage at regular intervals to calculate a sliding average.
 **************************************************************************************************/
void batteryDeviceConnectionOpened(void);

/***********************************************************************************************//**
 *  @brief
 *    Must be called when the connection is closed. It turns off the polling
 *    of the battery voltage.
 **************************************************************************************************/
void batteryDeviceConnectionClosed(void);

/***********************************************************************************************//**
 *  @brief
 *    Calculate the battery level in % based on the filtered battery voltage.
 *    It is assumed a very low current consumption and a CR2032 battery.
 **************************************************************************************************/
uint8_t batteryDeviceReadBatteryLevel(void);

/***********************************************************************************************//**
 *  @brief
 *    Event to handle periodic battery level measurements
 **************************************************************************************************/
void batteryDeviceMeasureEvtHandler(void);

/** @} (end addtogroup battery-hw) */
/** @} (end addtogroup app_hardware) */

#ifdef __cplusplus
};
#endif

#endif // BATTERY_HW_H
