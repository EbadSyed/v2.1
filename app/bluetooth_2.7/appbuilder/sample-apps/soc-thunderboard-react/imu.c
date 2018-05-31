/***********************************************************************************************//**
 * \file   imu.c
 * \brief  Inertial Measurement Unit
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include "imu.h"
#include <math.h>

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

#define MAX_ACCEL_FOR_ANGLE             0.9848

/***************************************************************************************************
 * Local Function Definitions
 **************************************************************************************************/
static void matrixMultiply(ImuFloat_t out[3][3], ImuFloat_t in_a[3][3], ImuFloat_t in_b[3][3])
{
  ImuFloat_t op[3];

  for (uint8_t x = 0; x < 3; x++) {
    for (uint8_t y = 0; y < 3; y++) {
      for (uint8_t w = 0; w < 3; w++) {
        op[w] = in_a[x][w] * in_b[w][y];
      }
      out[x][y] = op[0] + op[1] + op[2];
    }
  }
}

static ImuFloat_t vectorDotProduct(ImuFloat_t in_a[3], ImuFloat_t in_b[3])
{
  ImuFloat_t op = 0;

  for (uint8_t x = 0; x < 3; x++) {
    op += in_a[x] * in_b[x];
  }

  return op;
}

/***************************************************************************************************
 * Public Function Definitions
 **************************************************************************************************/
void imuAngleNormalize(ImuFloat_t *a)
{
  while (*a >= IMU_PI) {
    *a -= (2 * IMU_PI);
  }
  while (*a < -IMU_PI) {
    *a += (2 * IMU_PI);
  }
}

void imuVectorReset(ImuFloat_t inout[3])
{
  for (uint8_t x = 0; x < 3; x++) {
    inout[x] = 0.0;
  }
}

void imuVectorAngleNormalize(ImuFloat_t inout[3])
{
  for (uint8_t x = 0; x < 3; x++) {
    imuAngleNormalize(&inout[x]);
  }
}

void imuVectorAdd(ImuFloat_t out[3], ImuFloat_t in_a[3], ImuFloat_t in_b[3])
{
  for (uint8_t x = 0; x < 3; x++) {
    out[x] = in_a[x] + in_b[x];
  }
}

void imuVectorSubtract(ImuFloat_t out[3], ImuFloat_t in_a[3], ImuFloat_t in_b[3])
{
  for (uint8_t x = 0; x < 3; x++) {
    out[x] = in_a[x] - in_b[x];
  }
}

void imuVectorCopyAndScale(ImuFloat_t out[3], ImuFloat_t in[3], ImuFloat_t scale)
{
  for (uint8_t x = 0; x < 3; x++) {
    out[x] = in[x] * scale;
  }
}

void imuVectorScale(ImuFloat_t inout[3], ImuFloat_t scale)
{
  for (uint8_t x = 0; x < 3; x++) {
    inout[x] *= scale;
  }
}

void imuVectorCrossProduct(ImuFloat_t out[3], ImuFloat_t in_a[3], ImuFloat_t in_b[3])
{
  out[0] = (in_a[1] * in_b[2]) - (in_a[2] * in_b[1]);
  out[1] = (in_a[2] * in_b[0]) - (in_a[0] * in_b[2]);
  out[2] = (in_a[0] * in_b[1]) - (in_a[1] * in_b[0]);
}

void imuDcmReset(ImuFloat_t dcmMatrix[3][3])
{
  for (uint8_t y = 0; y < 3; y++) {
    for (uint8_t x = 0; x < 3; x++) {
      dcmMatrix[y][x] = (x == y) ? 1 : 0;
    }
  }
}

void imuDcmResetZ(ImuFloat_t dcmMatrix[3][3])
{
  dcmMatrix[0][0] = 1;
  dcmMatrix[0][1] = 0;
  dcmMatrix[0][2] = 0;
  imuVectorCrossProduct(&dcmMatrix[1][0], &dcmMatrix[0][0], &dcmMatrix[2][0]);
  imuVectorScale(&dcmMatrix[1][0], -1.0);
  imuVectorCrossProduct(&dcmMatrix[0][0], &dcmMatrix[1][0], &dcmMatrix[2][0]);
}

void imuDcmRotate(ImuFloat_t dcmMatrix[3][3], ImuFloat_t ang[3])
{
  ImuFloat_t um[3][3];
  ImuFloat_t tm[3][3];

  um[0][0] = 0;
  um[0][1] = -ang[2];
  um[0][2] = ang[1];
  um[1][0] = ang[2];
  um[1][1] = 0;
  um[1][2] = -ang[0];
  um[2][0] = -ang[1];
  um[2][1] = ang[0];
  um[2][2] = 0;

  matrixMultiply(tm, dcmMatrix, um);
  for (uint8_t y = 0; y < 3; y++) {
    for (uint8_t x = 0; x < 3; x++) {
      dcmMatrix[y][x] += tm[y][x];
    }
  }
}

void imuDcmNormalize(ImuFloat_t dcmMatrix[3][3])
{
  ImuFloat_t error = 0;
  ImuFloat_t temporary[3][3];
  ImuFloat_t renorm = 0;

  error = -vectorDotProduct(&dcmMatrix[0][0], &dcmMatrix[1][0]) * .5;

  imuVectorCopyAndScale(&temporary[0][0], &dcmMatrix[1][0], error);
  imuVectorCopyAndScale(&temporary[1][0], &dcmMatrix[0][0], error);

  imuVectorAdd(&temporary[0][0], &temporary[0][0], &dcmMatrix[0][0]);
  imuVectorAdd(&temporary[1][0], &temporary[1][0], &dcmMatrix[1][0]);

  imuVectorCrossProduct(&temporary[2][0], &temporary[0][0], &temporary[1][0]);

  renorm = .5 * (3 - vectorDotProduct(&temporary[0][0], &temporary[0][0]));
  imuVectorCopyAndScale(&dcmMatrix[0][0], &temporary[0][0], renorm);

  renorm = .5 * (3 - vectorDotProduct(&temporary[1][0], &temporary[1][0]));
  imuVectorCopyAndScale(&dcmMatrix[1][0], &temporary[1][0], renorm);

  renorm = .5 * (3 - vectorDotProduct(&temporary[2][0], &temporary[2][0]));
  imuVectorCopyAndScale(&dcmMatrix[2][0], &temporary[2][0], renorm);
}

void imuDcmGetAngles(ImuFloat_t dcmMatrix[3][3], ImuFloat_t ang[3])
{
  // Roll
  ang[0] = imuAtan2(dcmMatrix[2][1], dcmMatrix[2][2]);
  // Pitch
  ang[1] = -imuAsin(dcmMatrix[2][0]);
  // Yaw
  ang[2] = imuAtan2(dcmMatrix[1][0], dcmMatrix[0][0]);
}

void imuSensorFusionGyroCorrClr(ImuSensorFusion_t *fus)
{
  imuVectorReset(fus->fusionAngleCorrection);
}

void imuSensorFusionGyroCorrDo(ImuSensorFusion_t *fus, ImuFloat_t gyr[3])
{
  imuVectorAdd(gyr, gyr, fus->fusionAngleCorrection);
}

void imuSensorFusionGyroCorrCalc(ImuSensorFusion_t *fus, ImuFloat_t ori[3],
                                 bool accValid, ImuFloat_t accVec[3],
                                 bool dirValid, ImuFloat_t dirZ,
                                 int16_t freq)
{
  ImuFloat_t   accAng[3];

  imuSensorFusionGyroCorrClr(fus);

  if (accValid
      && (accVec[0] >= -MAX_ACCEL_FOR_ANGLE) && (accVec[0] <= MAX_ACCEL_FOR_ANGLE)
      && (accVec[1] >= -MAX_ACCEL_FOR_ANGLE) && (accVec[1] <= MAX_ACCEL_FOR_ANGLE)) {
    if (accVec[2] >= 0) {
      accAng[0] = imuAsin(accVec[1]);
      accAng[1] = -imuAsin(accVec[0]);
      accAng[2] = dirZ;
      imuVectorSubtract(fus->fusionAngleCorrection, accAng, ori);
      imuVectorAngleNormalize(fus->fusionAngleCorrection);
    } else {
      accAng[0] = IMU_PI - imuAsin(accVec[1]);
      accAng[1] = -imuAsin(accVec[0]);
      accAng[2] = IMU_PI + dirZ;
      imuVectorAngleNormalize(accAng);
      imuVectorSubtract(fus->fusionAngleCorrection, accAng, ori);
      imuVectorAngleNormalize(fus->fusionAngleCorrection);
      fus->fusionAngleCorrection[1] = -fus->fusionAngleCorrection[1];
    }
    if (!dirValid) {
      fus->fusionAngleCorrection[2] = 0;
    }
    imuVectorScale(fus->fusionAngleCorrection, 0.5 / freq);
  }
}
