/***********************************************************************************************//**
 * \file   accori_device.c
 * \brief  Acceleration and Orientation Device
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include "accori_device.h"
#include <math.h>
#include <stdbool.h>
#include "imu.h"
#include "mpu6500.h"
#include "rd0057.h"
#include "native_gecko.h"
#include "app_interrupt.h"

/***********************************************************************************************//**
 * @addtogroup app_hardware
 * @{
 *************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup accgyro-sensor
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

#define USE_SENSOR_ERROR_LED            0
#define USE_ANGULAR_ERROR_LED           0

#define SWAP_AXES_FOR_CAR_DEMO          1

#define USE_MPU6500_INTERRUPT           1
#define MPU6500_INTERRUPT_FREQ          200
#define MPU6500_POLL_FREQ               1
#define MPU6500_MAX_FREQ                1000

// Using a value just below the max value for the sensor
#define ERROR_LEVEL_GYR_ANG             2.0

#define CALIBRATION_DURATION_IN_SEC     4
#define CALIBRATION_LED                 LED_GREEN
#define ERROR_LED_ACC                   LED_BLUE
#define ERROR_LED_GYR                   LED_GREEN
#define ERROR_LED_ANG                   LED_BLUE

/***************************************************************************************************
 * Local Type Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

static bool mpu6500Detected = false;
static bool accelerationEnabled = false;
static bool accelerationNotification = false;
static bool orientationEnabled = false;
static bool orientationNotification = false;

static int16_t  accSensor[3];
static int16_t  gyrSensor[3];
static int32_t  accAccumulator[3];
static int32_t  accAccumulatorCnt;
static int16_t  oriCalculated[3];
static ImuFloat_t   accVec[3];
static ImuFloat_t   dcmMatrix[3][3];
static bool     calibrationInProgress = false;
static void     (*calibrateDoneCallbackG)(void);
static ImuSensorFusion_t sensorFusionData;

#if USE_MPU6500_INTERRUPT
static uint16_t sensorIntFreq = MPU6500_INTERRUPT_FREQ;
#else
static uint16_t sensorPollFreq = MPU6500_POLL_FREQ;
#endif
static uint32_t sensorFreq;

#if USE_SENSOR_ERROR_LED
static uint32_t errorAccCntDown = 0;
static uint32_t errorGyrCntDown = 0;
#endif
#if USE_ANGULAR_ERROR_LED
static uint32_t errorAngCntDown = 0;
#endif

/***************************************************************************************************
 * Public Variables
 **************************************************************************************************/

/***************************************************************************************************
 * Local Function Definitions
 **************************************************************************************************/
static void resetData(void)
{
  for (uint8_t i = 0; i < 3; i++) {
    accSensor[i] = 0;
    gyrSensor[i] = 0;
    accAccumulator[i] = 0;
    oriCalculated[i] = 0;
  }
  imuVectorReset(accVec);
  imuDcmReset(dcmMatrix);
  imuSensorFusionGyroCorrClr(&sensorFusionData);
}

static void interruptEnable(bool enable)
{
  if (enable) {
#if USE_MPU6500_INTERRUPT
    mpu6500_ConfigureInterrupt(i2cInit.port, MPU6500_ADDR, true, sensorIntFreq);
    mpu6500_InterruptAcknowledge(i2cInit.port, MPU6500_ADDR);

    // Trigger interrupt handler in next stack event
    gecko_external_signal(EXTSIGNAL_ACCGYRO);

    sensorFreq = sensorIntFreq;
#else
    mpu6500_ConfigureInterrupt(i2cInit.port, MPU6500_ADDR, false, MPU6500_MAX_FREQ);
    sensorFreq = sensorPollFreq;
#endif
  } else {
#if USE_MPU6500_INTERRUPT
    // Turn off interrupt generation if not used by orientation measurement
    mpu6500_ConfigureInterrupt(i2cInit.port, MPU6500_ADDR, false, sensorIntFreq);
#endif
  }
}

static void accelerationEnable(bool enable)
{
  if (mpu6500Detected) {
    accelerationEnabled = enable;
    mpu6500_ConfigureAccelEnable(i2cInit.port, MPU6500_ADDR, enable);
    if (enable) {
      mpu6500_ConfigureAccelRate(i2cInit.port, MPU6500_ADDR, mpu6500AccelFreq_5Hz);
      mpu6500_ConfigureAccelScale(i2cInit.port, MPU6500_ADDR, mpu6500AccelScale_16g);
    }
    if (!orientationEnabled) {
      interruptEnable(enable);
    }
  }
}

static void orientationEnable(bool enable)
{
  if (mpu6500Detected) {
    orientationEnabled = enable;
    mpu6500_ConfigureGyroEnable(i2cInit.port, MPU6500_ADDR, enable);
    if (enable) {
      mpu6500_ConfigureGyroRate(i2cInit.port, MPU6500_ADDR, mpu6500GyroFreq_184Hz);
      mpu6500_ConfigureGyroScale(i2cInit.port, MPU6500_ADDR, mpu6500GyroScale_2000);
    }
    if (!accelerationEnabled) {
      interruptEnable(enable);
    }
  }
}

static void vReadSensors(bool wait)
{
  bool accRangeError;
  bool gyrRangeError;

  mpu6500_Measure(i2cInit.port, MPU6500_ADDR,
                  &accSensor[0], &accSensor[1], &accSensor[2],
                  &accRangeError,
                  &gyrSensor[0], &gyrSensor[1], &gyrSensor[2],
                  &gyrRangeError,
                  wait);

#if SWAP_AXES_FOR_CAR_DEMO
  int16_t tmp;

  // Swap X and Y, change sign for Z.
  // For accelerometer
  tmp = accSensor[0];
  accSensor[0] = accSensor[1];
  accSensor[1] = tmp;
  accSensor[2] = -accSensor[2];
  // For gyro
  tmp = gyrSensor[0];
  gyrSensor[0] = gyrSensor[1];
  gyrSensor[1] = tmp;
  gyrSensor[2] = -gyrSensor[2];
#endif

  accVec[0] = mpu6500_AccelRegToG(accSensor[0]);
  accVec[1] = mpu6500_AccelRegToG(accSensor[1]);
  accVec[2] = mpu6500_AccelRegToG(accSensor[2]);
  imuVectorScale(accVec, 1.0 / 1000);

  accAccumulator[0] += mpu6500_AccelRegToG(accSensor[0]);
  accAccumulator[1] += mpu6500_AccelRegToG(accSensor[1]);
  accAccumulator[2] += mpu6500_AccelRegToG(accSensor[2]);
  accAccumulatorCnt++;

#if USE_SENSOR_ERROR_LED
  if (accRangeError) {
    // Turn on accelerometer error LED for 2 seconds
    appHwLedOn(ERROR_LED_ACC);
    errorAccCntDown = sensorFreq * 2;
  }
  if (gyrRangeError) {
    // Turn on gyro error LED for 2 seconds
    appHwLedOn(ERROR_LED_GYR);
    errorGyrCntDown = sensorFreq * 2;
  }
#endif
}

static void vCalculateOrientation(int16_t freq)
{
  ImuFloat_t gyr[3];
  ImuFloat_t ori[3];

  gyr[0] = (ImuFloat_t)mpu6500_GyroRegToAngle(gyrSensor[0]);
  gyr[1] = (ImuFloat_t)mpu6500_GyroRegToAngle(gyrSensor[1]);
  gyr[2] = (ImuFloat_t)mpu6500_GyroRegToAngle(gyrSensor[2]);
  imuVectorScale(gyr, IMU_DEG_TO_RAD_FACTOR / 100 / freq);

#if USE_ANGULAR_ERROR_LED
  // Check for large angles
  double maxAng = fmax(fmax(gyr[0], gyr[1]), gyr[2]);
  if (maxAng >= (ERROR_LEVEL_GYR_ANG * IMU_DEG_TO_RAD_FACTOR)) {
    // Turn on angualr error LED for 2 seconds
    appHwLedOn(ERROR_LED_ANG);
    errorAngCntDown = sensorFreq * 2;
  }
#endif

  // Adjust the gyro with values from previous calculation.
  imuSensorFusionGyroCorrDo(&sensorFusionData, gyr);

  imuDcmRotate(dcmMatrix, gyr);
  imuDcmNormalize(dcmMatrix);

  imuDcmGetAngles(dcmMatrix, ori);
  oriCalculated[0] = (int16_t)(ori[0] * IMU_RAD_TO_DEG_FACTOR * 100);
  oriCalculated[1] = (int16_t)(ori[1] * IMU_RAD_TO_DEG_FACTOR * 100);
  oriCalculated[2] = (int16_t)(ori[2] * IMU_RAD_TO_DEG_FACTOR * 100);

  // Calculate new sensor fusion values.
  imuSensorFusionGyroCorrCalc(&sensorFusionData,
                              ori,
                              accelerationEnabled, accVec,
                              false, 0,
                              freq);
}

/***************************************************************************************************
 * Public Function Definitions
 **************************************************************************************************/
void accoriDeviceInit(void)
{
  resetData();
  mpu6500Detected = mpu6500_Detect(i2cInit.port, MPU6500_ADDR);
  accoriDeviceSleep();
}

void accoriDeviceDeInit(void)
{
  accoriDeviceSleep();
}

void accoriDeviceConnectionOpened(void)
{
  // Nothing to do
}

void accoriDeviceConnectionClosed(void)
{
  accoriDeviceSleep();
}

uint8_t accoriDeviceTest(void)
{
  return mpu6500Detected ? 0 : 1;
}

void accoriDeviceSleep(void)
{
  accelerationNotification = false;
  orientationNotification = false;
  if (!calibrationInProgress) {
    accelerationEnable(false);
    orientationEnable(false);
  }
}

void accoriDeviceAccelerationCharStatusChange(uint8_t connection,
                                              uint16_t clientConfig)
{
  accelerationNotification = (clientConfig != 0);
  accelerationEnable(accelerationNotification);
}

void accoriDeviceOrientationCharStatusChange(uint8_t connection,
                                             uint16_t clientConfig)
{
  orientationNotification = (clientConfig != 0);
  orientationEnable(orientationNotification);
}

void accoriDeviceAccelerationRead(int16_t *accX, int16_t *accY, int16_t *accZ)
{
#if USE_MPU6500_INTERRUPT
  // nop
#else
  vReadSensors(true, sensorPollFreq);
#endif
  if (accAccumulatorCnt) {
    *accX = (int16_t)(accAccumulator[0] / accAccumulatorCnt);
    *accY = (int16_t)(accAccumulator[1] / accAccumulatorCnt);
    *accZ = (int16_t)(accAccumulator[2] / accAccumulatorCnt);
    accAccumulator[0] = 0;
    accAccumulator[1] = 0;
    accAccumulator[2] = 0;
    accAccumulatorCnt = 0;
  } else {
    *accX = 0;
    *accY = 0;
    *accZ = 0;
  }
}

void accoriDeviceOrientationRead(int16_t *oriX, int16_t *oriY, int16_t *oriZ)
{
#if USE_MPU6500_INTERRUPT
  // nop
#else
  vReadSensors(true, sensorPollFreq);
  vCalculateOrientation(sensorPollFreq);
#endif
  *oriX = oriCalculated[0];
  *oriY = oriCalculated[1];
  *oriZ = oriCalculated[2];
}

void accoriDeviceInterruptEvtHandler(void)
{
#if USE_MPU6500_INTERRUPT
  vReadSensors(false);
  vCalculateOrientation(sensorIntFreq);
#endif
  mpu6500_InterruptAcknowledge(i2cInit.port, MPU6500_ADDR);

  // Turn LED off when calibration ends
  if (calibrationInProgress && !mpu6500_GyroCalibrateInProgress()) {
    calibrationInProgress = false;
    boardLedOff(CALIBRATION_LED);
    calibrateDoneCallbackG();

    // Turn off sensors if not used by notifications
    if (!accelerationNotification) {
      accelerationEnable(false);
    }
    if (!orientationNotification) {
      orientationEnable(false);
    }
  }

#if USE_SENSOR_ERROR_LED
  // Turn off accelerometer error LED some time after error
  if (errorAccCntDown) {
    errorAccCntDown--;
    if (errorAccCntDown == 0) {
      appHwLedOff(ERROR_LED_ACC);
    }
  }
  // Turn off gyro error LED some time after error
  if (errorGyrCntDown) {
    errorGyrCntDown--;
    if (errorGyrCntDown == 0) {
      appHwLedOff(ERROR_LED_GYR);
    }
  }
#endif
#if USE_ANGULAR_ERROR_LED
  // Turn off angular error LED some time after error
  if (errorAngCntDown) {
    errorAngCntDown--;
    if (errorAngCntDown == 0) {
      appHwLedOff(ERROR_LED_ANG);
    }
  }
#endif
}

void accoriDeviceCalibrate(void (*calibrateDoneCallback)(void))
{
  boardLedOn(CALIBRATION_LED);
  calibrateDoneCallbackG = calibrateDoneCallback;
  accelerationEnable(true);
  orientationEnable(true);
  calibrationInProgress = true;
  mpu6500_GyroCalibrateBegin(sensorFreq * CALIBRATION_DURATION_IN_SEC);
}

void accoriDeviceOrientationReset(void)
{
  imuDcmResetZ(dcmMatrix);
}

void accoriDeviceCalibrateReset(void)
{
  mpu6500_GyroCalibrateReset(i2cInit.port, MPU6500_ADDR);
}

/** @} (end addtogroup accgyro-sensor) */
/** @} (end addtogroup app_hardware) */
