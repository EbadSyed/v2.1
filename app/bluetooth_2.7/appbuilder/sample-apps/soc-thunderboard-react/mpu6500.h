/***********************************************************************************************//**
 * \file   mpu6500.h
 * \brief  MPU6500 Driver
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef __MPU6500_H
#define __MPU6500_H

#include "i2cspm.h"

/***********************************************************************************************//**
 * \defgroup mpu6500 MPU6500
 * \brief MPU6500
 *
 * The mpu6500 is a software library to interface to the Invensense MPU-6500
 * combined 3-axis accelerometer and 3-axis gyrometer "Motion Tracking Device".
 *
 * Software routines have been used to configure and use either the
 * accelerometers or the gyrometers or both at the same time.
 *
 * A typical use will enable interrupts at a given frequency and let the
 * interrupt signal when new values are ready. The interrupt handler for the
 * device must be implemented elsewhere. Reading of the results involve I2C bus-
 * accesses and care should be taken to avoid bus contamination if multiple
 * devices share the same I2C bus.
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup tbr
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup mpu6500
 * @{
 **************************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
 *******************************   DEFINES   *******************************************************
 **************************************************************************************************/

/** I2C device address for mpu6500 */
#define MPU6500_ADDR      0xd0

typedef enum {
  mpu6500AccelFreq_460Hz,
  mpu6500AccelFreq_184Hz,
  mpu6500AccelFreq_92Hz,
  mpu6500AccelFreq_41Hz,
  mpu6500AccelFreq_20Hz,
  mpu6500AccelFreq_10Hz,
  mpu6500AccelFreq_5Hz,
} Mpu6500AccelFreq_t;

typedef enum {
  mpu6500AccelScale_2g,
  mpu6500AccelScale_4g,
  mpu6500AccelScale_8g,
  mpu6500AccelScale_16g,
} Mpu6500AccelScale_t;

typedef enum {
  mpu6500GyroFreq_3600Hz,
  mpu6500GyroFreq_250Hz,
  mpu6500GyroFreq_184Hz,
  mpu6500GyroFreq_92Hz,
  mpu6500GyroFreq_41Hz,
  mpu6500GyroFreq_20Hz,
  mpu6500GyroFreq_10Hz,
  mpu6500GyroFreq_5Hz,
} Mpu6500GyroFreq_t;

typedef enum {
  mpu6500GyroScale_250,
  mpu6500GyroScale_500,
  mpu6500GyroScale_1000,
  mpu6500GyroScale_2000
} Mpu6500GyroScale_t;

/***************************************************************************************************
 *****************************   PROTOTYPES   ******************************************************
 **************************************************************************************************/

/**********************************************************************************************//**
 * @brief
 *   Initializes the mpu6500.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to use.
 * @return
 *   True if a mpu6500 is detected, false otherwise.
 *************************************************************************************************/
bool mpu6500_Init(I2C_TypeDef *i2c, uint8_t addr);

/**********************************************************************************************//**
 * @brief
 *   Initializes the mpu6500.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to use.
 * @return
 *   True if a mpu6500 is detected, false otherwise.
 *************************************************************************************************/
bool mpu6500_Detect(I2C_TypeDef *i2c, uint8_t addr);

/**********************************************************************************************//**
 * @brief
 *   Configure interrupt handling.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to use.
 * @param[in] on
 *   True if interrupts shall be turned on.
 * @param[in] freq
 *   The interrupt frequency. Note that the actual frequency is determined by
 *   the sample rate of 1 kHz and a divider.
 * @return
 *   True if a mpu6500 is present, false otherwise.
 *************************************************************************************************/
bool mpu6500_ConfigureInterrupt(I2C_TypeDef *i2c, uint8_t addr, bool on, int16_t freq);

/**********************************************************************************************//**
 * @brief
 *   Acknowledge the mpu6500 interrupt. This will turn off the current interrupt
 *   request and make the device ready for the next interrupt.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to use.
 * @return
 *   True if a mpu6500 is present, false otherwise.
 *************************************************************************************************/
bool mpu6500_InterruptAcknowledge(I2C_TypeDef *i2c, uint8_t addr);

/**********************************************************************************************//**
 * @brief
 *   Enable or disable the accelerometers.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to use.
 * @param[in] enable
 *   True if the accelerometers shall be enabled.
 * @return
 *   True if a mpu6500 is present, false otherwise.
 *************************************************************************************************/
bool mpu6500_ConfigureAccelEnable(I2C_TypeDef *i2c, uint8_t addr, bool enable);

/**********************************************************************************************//**
 * @brief
 *   Set the accelerometer low pass filter frequency.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to use.
 * @param[in] rate
 *   The filter frequency.
 * @return
 *   True if a mpu6500 is present, false otherwise.
 *************************************************************************************************/
bool mpu6500_ConfigureAccelRate(I2C_TypeDef *i2c, uint8_t addr, Mpu6500AccelFreq_t rate);

/**********************************************************************************************//**
 * @brief
 *   Set the accelerometer full scale range.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to use.
 * @param[in] scale
 *   The accelerometers full scale range.
 * @return
 *   True if a mpu6500 is present, false otherwise.
 *************************************************************************************************/
bool mpu6500_ConfigureAccelScale(I2C_TypeDef *i2c, uint8_t addr, Mpu6500AccelScale_t scale);

/**********************************************************************************************//**
 * @brief
 *   Convert accelerometer register readout to g.
 * @param[in] reg
 *   The accelerometer register value.
 * @return
 *   Accelerometer value in g (1 g = 9.8 m/s2).
 *************************************************************************************************/
int16_t mpu6500_AccelRegToG(int16_t reg);

/**********************************************************************************************//**
 * @brief
 *   Enable or disable the gyrometers.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to use.
 * @param[in] enable
 *   True if the gyrometers shall be enabled.
 * @return
 *   True if a mpu6500 is present, false otherwise.
 *************************************************************************************************/
bool mpu6500_ConfigureGyroEnable(I2C_TypeDef *i2c, uint8_t addr, bool enable);

/**********************************************************************************************//**
 * @brief
 *   Set the gyrometer low pass filter frequency.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to use.
 * @param[in] rate
 *   The filter frequency.
 * @return
 *   True if a mpu6500 is present, false otherwise.
 *************************************************************************************************/
bool mpu6500_ConfigureGyroRate(I2C_TypeDef *i2c, uint8_t addr, Mpu6500GyroFreq_t rate);

/**********************************************************************************************//**
 * @brief
 *   Set the gyrometer full scale range.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to use.
 * @param[in] scale
 *   The gyrometers full scale range.
 * @return
 *   True if a mpu6500 is present, false otherwise.
 *************************************************************************************************/
bool mpu6500_ConfigureGyroScale(I2C_TypeDef *i2c, uint8_t addr, Mpu6500GyroScale_t scale);

/**********************************************************************************************//**
 * @brief
 *   Convert gyrometer register readout to angular velocity.
 * @param[in] reg
 *   The gyrometer register value.
 * @return
 *   Gyrometer value in angular velocity (in 0.01 degres/sec).
 *************************************************************************************************/
int32_t mpu6500_GyroRegToAngle(int16_t reg);

/**********************************************************************************************//**
 * @brief
 *   Start a gyrometer calibration.
 *   Note: Both interrupt and gyro must have been enabled to do a calibration.
 *   When the calibration is done, the calibration result will automatically
 *   be used.
 * @param[in] cnt
 *   The number of measurements that should be used for the calibration.
 *   A typical value should let the calibration take 4 to 10 seconds. If the
 *   frequency is 100 Hz, a typical value will be from 400 to 1000.
 *************************************************************************************************/
void mpu6500_GyroCalibrateBegin(uint32_t cnt);

/**********************************************************************************************//**
 * @brief
 *   Check if a gyrometer calibration is in progress.
 * @return
 *   True if gyrometer calibration is in progress, else false.
 *************************************************************************************************/
bool mpu6500_GyroCalibrateInProgress(void);

/**********************************************************************************************//**
 * @brief
 *   Reset the gyrometer calibration.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to use.
 *************************************************************************************************/
void mpu6500_GyroCalibrateReset(I2C_TypeDef *i2c, uint8_t addr);

/**********************************************************************************************//**
 * @brief
 *  Read the accelerometors and gyrometers.
 * @param[in] i2c
 *   The I2C peripheral to use.
 * @param[in] addr
 *   The I2C address to use.
 * @param[out] accX
 *   The raw accelerometer value for the X-axix.
 * @param[out] accY
 *   The raw accelerometer value for the Y-axix.
 * @param[out] accZ
 *   The raw accelerometer value for the Z-axix.
 * @param[out] accRangeError
 *   True if any of the accelerometers reached max value.
 * @param[out] gyrX
 *   The raw gyrometer value for the X-axix.
 * @param[out] gyrY
 *   The raw gyrometer value for the Y-axix.
 * @param[out] gyrZ
 *   The raw gyrometer value for the Z-axix.
 * @param[out] gyrRangeError
 *   True if any of the gyros reached max value.
 * @param[in] wait
 *   If true, wait for the measurement to complete, else use current values.
 * @return
 *   Returns true if the sensors were read else false.
 *************************************************************************************************/
bool mpu6500_Measure(I2C_TypeDef *i2c, uint8_t addr,
                     int16_t *accX, int16_t *accY, int16_t *accZ,
                     bool *accRangeError,
                     int16_t *gyrX, int16_t *gyrY, int16_t *gyrZ,
                     bool *gyrRangeError,
                     bool wait);

#ifdef __cplusplus
}
#endif

/** @} (end group mpu6500) */
/** @} (end group tbr) */
#endif /* __MPU6500_H */
