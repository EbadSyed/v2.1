/***********************************************************************************************//**
 * \file   accori_device.h
 * \brief  Acceleration and Orientation Device
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef ACCGYRO_SENSOR_H
#define ACCGYRO_SENSOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************************************//**
 * \defgroup accgyro-sensor Acceleration and Orientation HW
 * \brief Acceleration and Orientation API.
 *
 * The accgyro-sensor module is the ble application interface to the
 * acceleration and orientation routines supported by underlaying hardware and
 * software libraries.
 *************************************************************************************************/

/**********************************************************************************************//**
 * @addtogroup app_hardware
 * @{
 *************************************************************************************************/

/**********************************************************************************************//**
 * @addtogroup accgyro-sensor
 * @{
 *************************************************************************************************/

/**************************************************************************************************
 * Public Macros and Definitions
 *************************************************************************************************/

/**************************************************************************************************
 * Public Type declarations
 *************************************************************************************************/

/**************************************************************************************************
 * Function Declarations
 *************************************************************************************************/

/**********************************************************************************************//**
 * \brief  Initialize the module.
 *************************************************************************************************/
void accoriDeviceInit(void);

/**********************************************************************************************//**
 * \brief  De-initialize the module.
 *************************************************************************************************/
void accoriDeviceDeInit(void);

/**********************************************************************************************//**
 * \brief  Test if the hardware is reay.
 * @return  0 if no errors detected, else 1.
 *************************************************************************************************/
uint8_t accoriDeviceTest(void);

/**********************************************************************************************//**
 * \brief  Put the hardware in sleep mode (for low power).
 *************************************************************************************************/
void accoriDeviceSleep(void);

/**********************************************************************************************//**
 * \brief  Prepare the module for requests.
 *************************************************************************************************/
void accoriDeviceConnectionOpened(void);

/**********************************************************************************************//**
 * \brief  Prepare the module for no more requests.
 *************************************************************************************************/
void accoriDeviceConnectionClosed(void);

/**********************************************************************************************//**
 * \brief  Read accelerometer data.
 * \param[out] accX  Accelerometer X value.
 * \param[out] accY  Accelerometer Y value.
 * \param[out] accZ  Accelerometer Z value.
 *************************************************************************************************/
void accoriDeviceAccelerationRead(int16_t *accX, int16_t *accY, int16_t *accZ);

/**********************************************************************************************//**
 * \brief  Read orientation data.
 * \param[out] oriX  Orientation X value.
 * \param[out] oriY  Orientation Y value.
 * \param[out] oriZ  Orientation Z value.
 *************************************************************************************************/
void accoriDeviceOrientationRead(int16_t *oriX, int16_t *oriY, int16_t *oriZ);

/**********************************************************************************************//**
 * \brief  Enable or disable accelerometers depending on the characteristics.
 * \param[in]  connection  Connection ID.
 * \param[in] clientConfig  Acceleration characteristic.
 *************************************************************************************************/
void accoriDeviceAccelerationCharStatusChange(uint8_t connection, uint16_t clientConfig);

/**********************************************************************************************//**
 * \brief  Enable or disable gyrometers depending on the characteristics.
 * \param[in]  connection  Connection ID.
 * \param[in] clientConfig  Orientation characteristic.
 *************************************************************************************************/
void accoriDeviceOrientationCharStatusChange(uint8_t connection, uint16_t clientConfig);

/**********************************************************************************************//**
 * \brief  Start a gyrometer calibration.
 * \param[in] calibrateDoneCallback  Function that will be called when the calibration is completed.
 *************************************************************************************************/
void accoriDeviceCalibrate(void (*calibrateDoneCallback)(void));

/**********************************************************************************************//**
 * \brief  Reset gyrometer calibration.
 *************************************************************************************************/
void accoriDeviceCalibrateReset(void);

/**********************************************************************************************//**
 * \brief  Reset the z-axis for the orientation.
 *************************************************************************************************/
void accoriDeviceOrientationReset(void);

/**********************************************************************************************//**
 * \brief  Event handler that is triggered from the interrupt handler to avoid processing in ISR
 *************************************************************************************************/
void accoriDeviceInterruptEvtHandler(void);

/** @} (end addtogroup accgyro-sensor) */
/** @} (end addtogroup app_hardware) */

#ifdef __cplusplus
};
#endif

#endif /* ACCGYRO_SENSOR_H */
