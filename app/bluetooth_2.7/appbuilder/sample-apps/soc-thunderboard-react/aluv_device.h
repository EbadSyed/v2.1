/***********************************************************************************************//**
 * \file   aluv_device.h
 * \brief  Ambient Light and UV Device
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef ALUV_DEVICE_H
#define ALUV_DEVICE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************//**
 * \defgroup aluv-sensor Ambient Light And UV HW
 * \brief Ambient Light and UV Sensor API
 *
 * This is the module that handles ambient light and UV-index requests in the
 * application. It uses the si1133 interface library to do the actual low level
 * configuration and measurements of the sensor.
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app_hardware
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup aluv-sensor
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Public Macros and Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Function Declarations
 **************************************************************************************************/

/***********************************************************************************************//**
 * @brief
 *   The initialization routine for the sensor.
 **************************************************************************************************/
void aluvDeviceInit(void);

/***********************************************************************************************//**
 * @brief
 *   The de-initialization routine for the sensor.
 **************************************************************************************************/
void aluvDeviceDeInit(void);

/***********************************************************************************************//**
 * @brief
 *   Function to test if the sensor is detected.
 * @return
 *   0 if a si1133 is detected, 1 otherwise.
 **************************************************************************************************/
uint8_t aluvDeviceTest(void);

/***********************************************************************************************//**
 * @brief
 *   Put the sensor into sleep mode.
 *   Does nothing for the moment because the sensor is off when not in use.
 **************************************************************************************************/
void aluvDeviceSleep(void);

/***********************************************************************************************//**
 * @brief
 *   Function that is called when the connection is opened.
 **************************************************************************************************/
void aluvDeviceConnectionOpened(void);

/***********************************************************************************************//**
 * @brief
 *   Function that is called when the connection is closed.
 **************************************************************************************************/
void aluvDeviceConnectionClosed(void);

/***********************************************************************************************//**
 * @brief
 *   Start a lux measurement.
 *   The callback function will be called when the measurement is done.
 **************************************************************************************************/
void aluvDeviceLuxMeasure(void (*luxMeasurementDone)(uint32_t));

/***********************************************************************************************//**
 * @brief
 *   Start a UV-index measurement.
 *   The callback function will be called when the measurement is done.
 **************************************************************************************************/
void aluvDeviceUviMeasure(void (*uviMeasurementDone)(uint8_t));

/***********************************************************************************************//**
 * @brief
 *   Triggered by an interrupt from the ALUV device
 **************************************************************************************************/
void aluvDeviceInterruptEvtHandler(void);

/** @} (end addtogroup aluv-sensor) */
/** @} (end addtogroup app_hardware) */

#ifdef __cplusplus
};
#endif

#endif // ALUV_DEVICE_H
