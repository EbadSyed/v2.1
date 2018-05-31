/***********************************************************************************************//**
 * \file   si1133.h
 * \brief  Si1133 Driver
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef SI1133_H
#define SI1133_H

#include <stdint.h>
#include <stdbool.h>
#include "i2cspm.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************//**
 * \defgroup si1133 Si1133
 * \brief Si1133
 *
 * The si1133 is a combined ambient light and UV-index sensor. This is
 * an intrface library that can be used to configure the sensor, start
 * measurements, read the measurement results and calculate the lux and
 * UV-index.
 *
 * Please note:
 * The sensor must be configured to measure either ambient light or UV, only
 * one is available at a time.
 *
 * The software enables device interrupt which signals that measurements are
 * done. The interrupt handler for the device must be implemented elsewhere.
 * Both configuration, start and reading of the results involve I2C bus-
 * accesses and care should be taken to avoid bus contamination if multiple
 * devices share the same I2C bus.
 *
 * The routines for calculating lux and UV-index has been calibrated for use
 * without any diffuser in front of the sensor. The lux calculation is valid
 * for some typical light conditions only and should not be used in any critical
 * applications.
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup tbr
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup si1133
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Public Macros and Definitions
 **************************************************************************************************/

// I2C device address for si1133
#define SI1133_ADDR    0xaa

/***************************************************************************************************
 * Public Type Definitions
 **************************************************************************************************/

typedef struct {
  int32_t     ch0;
  int32_t     ch1;
  int32_t     ch2;
} Si1133ChData_t;

typedef enum {
  configureForUvi,
  configureForLux,
} Si1133Configuration_t;

/***************************************************************************************************
 * Function Declarations
 **************************************************************************************************/

/***********************************************************************************************//**
 * @brief
 *   Checks if a si1133 is present on the I2C bus or not.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to the device.
 * @return
 *   True if a si1133 is detected, false otherwise.
 **************************************************************************************************/
bool si1133_Detect(I2C_TypeDef *i2c, uint8_t addr);

/***********************************************************************************************//**
 * @brief
 *   Do a software reset of the device.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to the device.
 **************************************************************************************************/
void si1133_Reset(I2C_TypeDef *i2c, uint8_t addr);

/***********************************************************************************************//**
 * @brief
 *   Configures the sensor.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to the device.
 * @param[in] cfg
 *   The configuration of the device. It can be either visible light or UV.
 * @return
 *   True if a si1133 is present, false otherwise.
 **************************************************************************************************/
bool si1133_MeasurementConfigure(I2C_TypeDef *i2c, uint8_t addr, Si1133Configuration_t cfg);

/***********************************************************************************************//**
 * @brief
 *   Start a new measurement according to the current measurement configuration.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to the device.
 * @return
 *   True if a measurement is started, false otherwise.
 **************************************************************************************************/
bool si1133_MeasurementStart(I2C_TypeDef *i2c, uint8_t addr);

/***********************************************************************************************//**
 * @brief
 *   Read the results from the last measurement.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to the device.
 * @param[out] chData
 *   The result of the last measurement.
 * @return
 *   True if reagisters were read, false otherwise.
 **************************************************************************************************/
bool si1133_MeasurementRead(I2C_TypeDef *i2c, uint8_t addr, Si1133ChData_t *chData);

/***********************************************************************************************//**
 * @brief
 *   Calculate the lux from measurement data.
 *   Note that the sensor must have been configured for lux before the
 *   measurements are done.
 * @param[in] chData
 *   Channel data measured with the sensor.
 * @return
 *   Light level in 0.01 lux.
 **************************************************************************************************/
uint32_t si1133_CalculateLux(Si1133ChData_t *chData);

/***********************************************************************************************//**
 * @brief
 *   Calculate the UV-index from measurement data.
 *   Note that the sensor must have been configured for UV before the
 *   measurements are done.
 * @param[in] chData
 *   Channel data measured with the sensor.
 * @return
 *   UV-index.
 **************************************************************************************************/
uint8_t si1133_CalculateUvi(Si1133ChData_t *chData);

/** @} (end addtogroup si1133) */
/** @} (end addtogroup tbr) */

#ifdef __cplusplus
}
#endif

#endif // SI1133_H

// END addtogroup
