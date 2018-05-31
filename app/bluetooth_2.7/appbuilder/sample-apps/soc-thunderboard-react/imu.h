/***********************************************************************************************//**
 * \file   imu.h
 * \brief  Inertial Measurement Unit
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef IMU_H
#define IMU_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************//**
 * \defgroup imu Inertial Measurement Unit
 * \brief Inertial Measurement Unit API
 *
 * The purpose of the imu module is to offer all the basic calculations to
 * be able to implement an "Inertial Measurement Unit". Functions are available
 * to calculate the acceleration and orientation in 3 axis based on input from
 * a 3 axis accelerometer and a 3 axis mems gyro.
 *
 * The calculation of angles between the fixed and rotating coordinate system
 * can be done by the "Direction Cosine Matrix" functions.
 *
 * Note: All angles must be entered in radians typically in the range -PI..PI
 * and all calculations will return angles as radians in the same range.
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup Thunderboard
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup imu
 * @{
 **************************************************************************************************/

/**************************************************************************************************
 * Public Macros and Definitions
 *************************************************************************************************/

#define IMU_PI 3.1415926535897932384626433832795
#define IMU_DEG_TO_RAD_FACTOR   (IMU_PI / 180.0)
#define IMU_RAD_TO_DEG_FACTOR   (180.0 / IMU_PI)
#define IMU_DEG_TO_RAD(ang)     (ang * IMU_DEG_TO_RAD_FACTOR)
#define IMU_RAD_TO_DEG(ang)     (ang * IMU_RAD_TO_DEG_FACTOR)

#if 1
#define ImuFloat_t  float
#define imuAsin(x)  asinf(x)
#define imuAtan2(x, y) atan2f(x, y)
#else
#define ImuFloat_t  double
#define imuAsin(x)  asin(x)
#define imuAtan2(x, y) atan2(x, y)
#endif

/**************************************************************************************************
 * Public Type Declarations
 *************************************************************************************************/

typedef struct {
  ImuFloat_t   fusionAngleCorrection[3];
} ImuSensorFusion_t;

/**************************************************************************************************
 * Public Function Declarations
 *************************************************************************************************/

/**********************************************************************************************//**
 * @brief
 *   Normalize the angle, i.e. adjust the range to be within -PI..PI.
 * @param[in] a
 *   Pointer to the angle.
 *************************************************************************************************/
void imuAngleNormalize(ImuFloat_t *a);

/**********************************************************************************************//**
 * @brief
 *   Reset the vector, i.e. set all elements to 0.
 * @param[in,out] inout
 *   Pointer to the vector.
 *************************************************************************************************/
void imuVectorReset(ImuFloat_t inout[3]);

/**********************************************************************************************//**
 * @brief
 *   Normalize the vector, i.e. apply the imuAngleNormalize function on all
 *   elements in the vector.
 * @param[in,out] inout
 *   Pointer to the vector.
 *************************************************************************************************/
void imuVectorAngleNormalize(ImuFloat_t inout[3]);

/**********************************************************************************************//**
 * @brief
 *   Add two vectors.
 * @param[out] out
 *   Pointer to the resultant vector.
 * @param[in] in_a
 *   Pointer to the first term vector.
 * @param[in] in_b
 *   Pointer to the second term vector.
 *************************************************************************************************/
void imuVectorAdd(ImuFloat_t out[3], ImuFloat_t in_a[3], ImuFloat_t in_b[3]);

/**********************************************************************************************//**
 * @brief
 *   Subtract two vectors.
 * @param[out] out
 *   Pointer to the resultant vector.
 * @param[in] in_a
 *   Pointer to the first term vector.
 * @param[in] in_b
 *   Pointer to the second term vector.
 *************************************************************************************************/
void imuVectorSubtract(ImuFloat_t out[3], ImuFloat_t in_a[3], ImuFloat_t in_b[3]);

/**********************************************************************************************//**
 * @brief
 *   Copy and multiply the vector by a scalar.
 * @param[out] out
 *   Pointer to the resultant vector.
 * @param[in] in
 *   Pointer to the input vector.
 * @param[in] scale
 *    Value to scale the vector with.
 *************************************************************************************************/
void imuVectorCopyAndScale(ImuFloat_t out[3], ImuFloat_t in[3], ImuFloat_t scale);

/**********************************************************************************************//**
 * @brief
 *   Multiply the vector by a scalar.
 * @param[in,out] inout
 *   Pointer to the resultant vector.
 * @param[in] scale
 *    Value to scale the vector with.
 *************************************************************************************************/
void imuVectorScale(ImuFloat_t inout[3], ImuFloat_t scale);

/**********************************************************************************************//**
 * @brief
 *   Computes the cross product of two vectors.
 * @param[out] out
 *   Pointer to the resultant vector.
 * @param[in] in_a
 *   Pointer to the first vector.
 * @param[in] in_b
 *   Pointer to the second vector.
 *************************************************************************************************/
void imuVectorCrossProduct(ImuFloat_t out[3], ImuFloat_t in_a[3], ImuFloat_t in_b[3]);

/**********************************************************************************************//**
 * @brief
 *   Reset the dcm-matrix, i.e. make it an identity matrix.
 * @param[in,out] dcmMatrix
 *   Pointer to the matrix.
 *************************************************************************************************/
void imuDcmReset(ImuFloat_t dcmMatrix[3][3]);

/**********************************************************************************************//**
 * @brief
 *   Reset only the z-axix in the dcm-matrix.
 * @param[in,out] dcmMatrix
 *   Pointer to the matrix.
 *************************************************************************************************/
void imuDcmResetZ(ImuFloat_t dcmMatrix[3][3]);

/**********************************************************************************************//**
 * @brief
 *   Rotate the dcm-matrix.
 * @param[in,out] dcmMatrix
 *   Pointer to the matrix.
 * @param[in] ang
 *   Angles to rotate the dcm-matrix. Angles are relative to the rotating
 *   coordinate system.
 *************************************************************************************************/
void imuDcmRotate(ImuFloat_t dcmMatrix[3][3], ImuFloat_t ang[3]);

/**********************************************************************************************//**
 * @brief
 *   Adjust the dcm-matrix to have 3 perpendicular unit vectors.
 * @param[in,out] dcmMatrix
 *   Pointer to the matrix.
 *************************************************************************************************/
void imuDcmNormalize(ImuFloat_t dcmMatrix[3][3]);

/**********************************************************************************************//**
 * @brief
 *   Get the dcm-matrix angles.
 * @param[in] dcmMatrix
 *   Pointer to the matrix.
 * @param[out] ang
 *   The dcm-matrix angles relative to the fixed coordinate system.
 *************************************************************************************************/
void imuDcmGetAngles(ImuFloat_t dcmMatrix[3][3], ImuFloat_t ang[3]);

/*********************************************************************************************//**
 * @brief
 *   Clear the sensor fusion data.
 * @param[in] fus
 *   Pointer to the sensor fusion data.
 ************************************************************************************************/
void imuSensorFusionGyroCorrClr(ImuSensorFusion_t *fus);

/**********************************************************************************************//**
 * @brief
 *   Add correction data to the gyro based on the sensor fusion calculations.
 * @param[in] fus
 *   Pointer to the sensor fusion data.
 * @param[in] gyr
 *   Gyro data
 ************************************************************************************************/
void imuSensorFusionGyroCorrDo(ImuSensorFusion_t *fus, ImuFloat_t gyr[3]);

/**********************************************************************************************//**
 * @brief
 *   Calculate sensor fusion data.
 *   The purpose of the sensor fusion is to calculate adjustments in the
 *   orientation from other sensors than the gyros, in this case from
 *   accelerometers and orientation sensors. These adjustments can than be added
 *   to the gyro data to obtain better orientation values.
 * @param[in] fus
 *   Pointer to the sensor fusion data.
 * @param[in] ori
 *   The current orientation vector.
 * @param[in] accValid
 *   A boolean that says if the accelerometer vector contains valid data.
 * @param[in] accVec
 *   The accelerometer vector.
 * @param[in] dirValid
 *   A boolean that says if the orientation information is valid.
 * @param[in] dirZ
 *   The orientation in the Z-axis.
 * @param[in] freq
 *   The calculation and correction frequency.
 *************************************************************************************************/
void imuSensorFusionGyroCorrCalc(ImuSensorFusion_t *fus, ImuFloat_t ori[3],
                                 bool accValid, ImuFloat_t accVec[3],
                                 bool dirValid, ImuFloat_t dirZ,
                                 int16_t freq);

/** @} (end addtogroup imu) */
/** @} (end addtogroup Thunderboard) */

#ifdef __cplusplus
}
#endif

#endif // IMU_H
