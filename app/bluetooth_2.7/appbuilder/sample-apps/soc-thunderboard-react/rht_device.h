/***********************************************************************************************//**
 * \file   rht_device.h
 * \brief  Relative Humidity and Temperature Device
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef RHT_DEVICE_H
#define RHT_DEVICE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************//**
 * \defgroup rht-sensor Relative Humidity And Temperature HW
 * \brief Relative Humidity And Temperature Sensor API
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app_hardware
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup rht-sensor
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Public Macros and Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Function Declarations
 **************************************************************************************************/

void rhtDeviceInit(void);

void rhtDeviceDeInit(void);

uint8_t rhtDeviceTest(void);

void rhtDeviceSleep(void);

void rhtDeviceConnectionOpened(void);

void rhtDeviceConnectionClosed(void);

void rhtDeviceHumidityMeasure(void (*humidityMeasurementDone)(uint16_t));

void rhtDeviceTemperatureMeasure(void (*temperatureMeasurementDone)(int16_t));

/** @} (end addtogroup rht-sensor) */
/** @} (end addtogroup app_hardware) */

#ifdef __cplusplus
};
#endif

#endif // RHT_DEVICE_H
