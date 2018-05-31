/***********************************************************************************************//**
 * \file   mpu6500.c
 * \brief  MPU6500 Driver
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include "mpu6500.h"
#include <stdlib.h>

/***************************************************************************************************
 *******************************   DEFINES   *******************************************************
 **************************************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

#define REG_XG_OFFSET                   19
#define REG_YG_OFFSET                   21
#define REG_ZG_OFFSET                   23
#define REG_SMPLRT_DIV                  25
#define REG_CONFIG                      26
#define REG_GYRO_CONFIG                 27
#define REG_ACCEL_CONFIG                28
#define REG_ACCEL_CONFIG_2              29
#define REG_INT_PIN_CFG                 55
#define REG_INT_ENABLE                  56
#define REG_INT_STATUS                  58
#define REG_ACCE_TEMP_GYRO              59
#define REG_PWR_MANAGEMENT_1           107
#define REG_PWR_MANAGEMENT_2           108
#define REG_WHO_AM_I                   117

#define REG_CONFIG_DLPF_CFG_250HZ                       0x00
#define REG_CONFIG_DLPF_CFG_184HX                       0x01
#define REG_CONFIG_DLPF_CFG_92HZ                        0x02
#define REG_CONFIG_DLPF_CFG_41HZ                        0x03
#define REG_CONFIG_DLPF_CFG_20HZ                        0x04
#define REG_CONFIG_DLPF_CFG_10HZ                        0x05
#define REG_CONFIG_DLPF_CFG_5HZ                         0x06
#define REG_CONFIG_DLPF_CFG_3600HZ                      0x07
#define REG_GYRO_CONFIG_FCHOICE_B_USE_DLPF              0x00
#define REG_GYRO_CONFIG_GYRO_FS_SEL_250DPS              0x00
#define REG_GYRO_CONFIG_GYRO_FS_SEL_500DPS              0x08
#define REG_GYRO_CONFIG_GYRO_FS_SEL_1000DPS             0x10
#define REG_GYRO_CONFIG_GYRO_FS_SEL_2000DPS             0x18
#define REG_ACCEL_CONFIG_ACCE_FS_SEL_2G                 0x00
#define REG_ACCEL_CONFIG_ACCE_FS_SEL_4G                 0x08
#define REG_ACCEL_CONFIG_ACCE_FS_SEL_8G                 0x10
#define REG_ACCEL_CONFIG_ACCE_FS_SEL_16G                0x18
#define REG_ACCEL_CONFIG_2_A_DLPF_CFG_460HZ             0x00
#define REG_ACCEL_CONFIG_2_A_DLPF_CFG_184HZ             0x01
#define REG_ACCEL_CONFIG_2_A_DLPF_CFG_92HZ              0x02
#define REG_ACCEL_CONFIG_2_A_DLPF_CFG_41HZ              0x03
#define REG_ACCEL_CONFIG_2_A_DLPF_CFG_20HZ              0x04
#define REG_ACCEL_CONFIG_2_A_DLPF_CFG_10HZ              0x05
#define REG_ACCEL_CONFIG_2_A_DLPF_CFG_5HZ               0x06
#define REG_PWR_MANAGEMENT_1_GYRO_STANDBY               0x10
#define REG_PWR_MANAGEMENT_1_SLEEP                      0x40
#define REG_PWR_MANAGEMENT_2_DISABLE_GYROS              0x07
#define REG_PWR_MANAGEMENT_2_DISABLE_ACCELEROMETERS     0x38
#define REG_INT_PIN_CFG_LATCH_INT_EN                    0x20
#define REG_INT_PIN_CFG_ACTL                            0x80
#define REG_INT_ENABLE_DISABLE                          0x00
#define REG_INT_ENABLE_RAW_RDY_EN                       0x01
#define REG_INT_STATUS_DATA_RDY                         0x01
#define REG_WHO_AM_I_EXPECTED                           0x70

#define DUMMY_INTERRUPT_FREQ            1

#define SHOW_GYROOFS                    0

#define BUFFER_TO_INT16(buf, ofs) ((buf[ofs] << 8) + buf[ofs + 1])

#define ERROR_LEVEL_ACC_REG             30000
#define ERROR_LEVEL_GYR_REG             30000

/***************************************************************************************************
 **************************   LOCAL variables    ***************************************************
 **************************************************************************************************/

static int16_t  accScale = 2;
static bool     accEnable = true;

static int16_t  gyrScale = 250;
static bool     gyrEnable = true;

static int16_t  gyrOfsX = 0;
static int16_t  gyrOfsY = 0;
static int16_t  gyrOfsZ = 0;

static int32_t  gyrSumX = 0;
static int32_t  gyrSumY = 0;
static int32_t  gyrSumZ = 0;
static int32_t  gyroCalibrationTotalCnt = 0;
static int32_t  gyroCalibrationProgressCnt = 0;
static int32_t  gyroCalibrationFactor = 8;

/***************************************************************************************************
 **************************   LOCAL FUNCTIONS   ***************************************************
 **************************************************************************************************/
static I2C_TransferReturn_TypeDef registerRead8(I2C_TypeDef *i2c, uint8_t addr, uint8_t reg, uint8_t *val)
{
  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t                    i2c_write_data[1];
  uint8_t                    i2c_read_data[1];

  seq.addr  = addr;
  seq.flags = I2C_FLAG_WRITE_READ;
  // Select command to issue
  i2c_write_data[0] = reg;
  seq.buf[0].data   = i2c_write_data;
  seq.buf[0].len    = 1;
  // Select location/length of data to be read
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len  = 1;

  ret = I2CSPM_Transfer(i2c, &seq);
  if (ret != i2cTransferDone) {
    return ret;
  }
  if (NULL != val) {
    *val = i2c_read_data[0];
  }
  return ret;
}

static I2C_TransferReturn_TypeDef registerRead16(I2C_TypeDef *i2c, uint8_t addr, uint8_t reg, uint16_t *val)
{
  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t                    i2c_write_data[1];
  uint8_t                    i2c_read_data[2];

  seq.addr  = addr;
  seq.flags = I2C_FLAG_WRITE_READ;
  // Select command to issue
  i2c_write_data[0] = reg;
  seq.buf[0].data   = i2c_write_data;
  seq.buf[0].len    = 1;
  // Select location/length of data to be read
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len  = 2;

  ret = I2CSPM_Transfer(i2c, &seq);
  if (ret != i2cTransferDone) {
    return ret;
  }
  if (NULL != val) {
    *val = BUFFER_TO_INT16(i2c_read_data, 0);
  }
  return ret;
}

static I2C_TransferReturn_TypeDef registerWrite8(I2C_TypeDef *i2c, uint8_t addr, uint8_t reg, uint8_t val)
{
  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t                    i2c_write_data[2];

  seq.addr  = addr;
  seq.flags = I2C_FLAG_WRITE;
  // Select command to issue
  i2c_write_data[0] = reg;
  i2c_write_data[1] = val;
  seq.buf[0].data   = i2c_write_data;
  seq.buf[0].len    = 2;
  // Select location/length of data to be read
  seq.buf[1].data = 0;
  seq.buf[1].len  = 0;

  ret = I2CSPM_Transfer(i2c, &seq);
  return ret;
}

static I2C_TransferReturn_TypeDef registerWrite16(I2C_TypeDef *i2c, uint8_t addr, uint8_t reg, uint16_t val)
{
  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t                    i2c_write_data[3];

  seq.addr  = addr;
  seq.flags = I2C_FLAG_WRITE;
  // Select command to issue
  i2c_write_data[0] = reg;
  i2c_write_data[1] = val >> 8;
  i2c_write_data[2] = val;
  seq.buf[0].data   = i2c_write_data;
  seq.buf[0].len    = 3;
  // Select location/length of data to be read
  seq.buf[1].data = 0;
  seq.buf[1].len  = 0;

  ret = I2CSPM_Transfer(i2c, &seq);
  return ret;
}

static void waitForNewData(I2C_TypeDef *i2c, uint8_t addr)
{
  uint8_t uReg;

  do {
    registerRead8(i2c, addr, REG_INT_STATUS, &uReg);
  } while ((uReg & REG_INT_STATUS_DATA_RDY) == 0);
}

static void gyroCalibrateBegin(uint32_t cnt)
{
  gyrSumX = 0;
  gyrSumY = 0;
  gyrSumZ = 0;
  gyroCalibrationTotalCnt = cnt;
  gyroCalibrationProgressCnt = cnt;
}

static void gyroCalibrateStep(I2C_TypeDef *i2c, uint8_t addr,
                              int16_t gyrX, int16_t gyrY, int16_t gyrZ)
{
  uint16_t reg;

  if (gyroCalibrationProgressCnt) {
    gyrSumX += gyrX;
    gyrSumY += gyrY;
    gyrSumZ += gyrZ;

    gyroCalibrationProgressCnt--;
    if (gyroCalibrationProgressCnt == 0) {
      // Read old values
      registerRead16(i2c, addr, REG_XG_OFFSET, &reg);
      gyrOfsX = (int16_t)reg;
      registerRead16(i2c, addr, REG_YG_OFFSET, &reg);
      gyrOfsY = (int16_t)reg;
      registerRead16(i2c, addr, REG_ZG_OFFSET, &reg);
      gyrOfsZ = (int16_t)reg;
      // Calculate new values
      gyrOfsX -= (gyrSumX * 2 / gyroCalibrationFactor / gyroCalibrationTotalCnt);
      gyrOfsY -= (gyrSumY * 2 / gyroCalibrationFactor / gyroCalibrationTotalCnt);
      gyrOfsZ -= (gyrSumZ * 2 / gyroCalibrationFactor / gyroCalibrationTotalCnt);
      // Write new values
      reg = (uint16_t)gyrOfsX;
      registerWrite16(i2c, addr, REG_XG_OFFSET, reg);
      reg = (uint16_t)gyrOfsY;
      registerWrite16(i2c, addr, REG_YG_OFFSET, reg);
      reg = (uint16_t)gyrOfsZ;
      registerWrite16(i2c, addr, REG_ZG_OFFSET, reg);
    }
  }
}

static bool chipEnable(I2C_TypeDef *i2c, uint8_t addr, bool enable)
{
  uint8_t reg;
  I2C_TransferReturn_TypeDef sta;

  sta = registerRead8(i2c, addr, REG_PWR_MANAGEMENT_1, &reg);
  if (enable) {
    reg &= ~REG_PWR_MANAGEMENT_1_GYRO_STANDBY;
    reg &= ~REG_PWR_MANAGEMENT_1_SLEEP;
  } else {
    reg |= REG_PWR_MANAGEMENT_1_GYRO_STANDBY;
    reg |= REG_PWR_MANAGEMENT_1_SLEEP;
  }
  sta = registerWrite8(i2c, addr, REG_PWR_MANAGEMENT_1, reg);
  return sta == i2cTransferDone;
}

/** @endcond (DO_NOT_INCLUDE_WITH_DOXYGEN) */

/***************************************************************************************************
 **************************   GLOBAL FUNCTIONS   ***************************************************
 **************************************************************************************************/
bool mpu6500_Init(I2C_TypeDef *i2c, uint8_t addr)
{
  bool detected = false;

  detected = mpu6500_Detect(i2c, MPU6500_ADDR);
  if (detected) {
    mpu6500_ConfigureAccelEnable(i2c, MPU6500_ADDR, false);
    mpu6500_ConfigureGyroEnable(i2c, MPU6500_ADDR, false);
    mpu6500_ConfigureInterrupt(i2c, addr, false, DUMMY_INTERRUPT_FREQ);
  }
  return detected;
}

bool mpu6500_Detect(I2C_TypeDef *i2c, uint8_t addr)
{
  uint8_t reg;
  I2C_TransferReturn_TypeDef sta;
  bool detected = false;

  sta = registerRead8(i2c, addr, REG_WHO_AM_I, &reg);
  detected = sta == i2cTransferDone;
  if (detected) {
    if (reg != REG_WHO_AM_I_EXPECTED) {
      detected = false;
    }
  }
  return detected;
}

bool mpu6500_ConfigureInterrupt(I2C_TypeDef *i2c, uint8_t addr, bool on, int16_t freq)
{
  I2C_TransferReturn_TypeDef sta;
  uint16_t reg;

  reg = (1000 / freq) - 1;
  sta = registerWrite8(i2c, addr, REG_SMPLRT_DIV, reg);
  sta = registerWrite8(i2c, addr, REG_INT_PIN_CFG, REG_INT_PIN_CFG_ACTL);
  sta = registerWrite8(i2c, addr, REG_INT_ENABLE,
                       on ? REG_INT_ENABLE_RAW_RDY_EN : REG_INT_ENABLE_DISABLE);

  return sta == i2cTransferDone;
}

bool mpu6500_InterruptAcknowledge(I2C_TypeDef *i2c, uint8_t addr)
{
  I2C_TransferReturn_TypeDef sta;
  uint8_t       reg;

  sta = registerRead8(i2c, addr, REG_INT_STATUS, &reg);
  return sta == i2cTransferDone;
}

bool mpu6500_ConfigureAccelEnable(I2C_TypeDef *i2c, uint8_t addr, bool enable)
{
  uint8_t reg = 0;
  I2C_TransferReturn_TypeDef sta;

  accEnable = enable;
  chipEnable(i2c, addr, accEnable || gyrEnable);

  sta = registerRead8(i2c, addr, REG_PWR_MANAGEMENT_2, &reg);
  if (enable) {
    reg &= ~REG_PWR_MANAGEMENT_2_DISABLE_ACCELEROMETERS;
  } else {
    reg |= REG_PWR_MANAGEMENT_2_DISABLE_ACCELEROMETERS;
  }
  sta = registerWrite8(i2c, addr, REG_PWR_MANAGEMENT_2, reg);
  return sta == i2cTransferDone;
}

bool mpu6500_ConfigureAccelRate(I2C_TypeDef *i2c, uint8_t addr, Mpu6500AccelFreq_t rate)
{
  uint8_t reg;
  I2C_TransferReturn_TypeDef sta;

  switch (rate) {
    case mpu6500AccelFreq_184Hz:
      reg = REG_ACCEL_CONFIG_2_A_DLPF_CFG_184HZ;
      break;

    case mpu6500AccelFreq_92Hz:
      reg = REG_ACCEL_CONFIG_2_A_DLPF_CFG_92HZ;
      break;

    case mpu6500AccelFreq_41Hz:
      reg = REG_ACCEL_CONFIG_2_A_DLPF_CFG_41HZ;
      break;

    case mpu6500AccelFreq_20Hz:
      reg = REG_ACCEL_CONFIG_2_A_DLPF_CFG_20HZ;
      break;

    case mpu6500AccelFreq_10Hz:
      reg = REG_ACCEL_CONFIG_2_A_DLPF_CFG_10HZ;
      break;

    case mpu6500AccelFreq_5Hz:
      reg = REG_ACCEL_CONFIG_2_A_DLPF_CFG_5HZ;
      break;

    case mpu6500AccelFreq_460Hz:
    default:
      reg = REG_ACCEL_CONFIG_2_A_DLPF_CFG_460HZ;
  }
  sta = registerWrite8(i2c, addr, REG_ACCEL_CONFIG_2, reg);
  return sta == i2cTransferDone;
}

bool mpu6500_ConfigureAccelScale(I2C_TypeDef *i2c, uint8_t addr, Mpu6500AccelScale_t scale)
{
  uint8_t reg;
  I2C_TransferReturn_TypeDef sta;

  switch (scale) {
    case mpu6500AccelScale_4g:
      reg = REG_ACCEL_CONFIG_ACCE_FS_SEL_4G;
      accScale = 4;
      break;

    case mpu6500AccelScale_8g:
      reg = REG_ACCEL_CONFIG_ACCE_FS_SEL_8G;
      accScale = 8;
      break;

    case mpu6500AccelScale_16g:
      reg = REG_ACCEL_CONFIG_ACCE_FS_SEL_16G;
      accScale = 16;
      break;

    case mpu6500AccelScale_2g:
    default:
      reg = REG_ACCEL_CONFIG_ACCE_FS_SEL_2G;
      accScale = 2;
      break;
  }
  sta = registerWrite8(i2c, addr, REG_ACCEL_CONFIG, reg);
  return sta == i2cTransferDone;
}

int16_t mpu6500_AccelRegToG(int16_t reg)
{
  int32_t accel;

  accel = (int32_t)reg * 1000 * accScale / 32768;

  return accel;
}

bool mpu6500_ConfigureGyroEnable(I2C_TypeDef *i2c, uint8_t addr, bool enable)
{
  uint8_t reg = 0;
  I2C_TransferReturn_TypeDef sta;

  gyrEnable = enable;
  chipEnable(i2c, addr, accEnable || gyrEnable);

  sta = registerRead8(i2c, addr, REG_PWR_MANAGEMENT_2, &reg);
  if (enable) {
    reg &= ~REG_PWR_MANAGEMENT_2_DISABLE_GYROS;
  } else {
    reg |= REG_PWR_MANAGEMENT_2_DISABLE_GYROS;
  }
  sta = registerWrite8(i2c, addr, REG_PWR_MANAGEMENT_2, reg);

  return sta == i2cTransferDone;
}

bool mpu6500_ConfigureGyroRate(I2C_TypeDef *i2c, uint8_t addr, Mpu6500GyroFreq_t rate)
{
  uint8_t reg;
  I2C_TransferReturn_TypeDef sta;

  switch (rate) {
    case mpu6500GyroFreq_3600Hz:
      reg = REG_CONFIG_DLPF_CFG_3600HZ;
      break;

    case mpu6500GyroFreq_184Hz:
      reg = REG_CONFIG_DLPF_CFG_184HX;
      break;

    case mpu6500GyroFreq_92Hz:
      reg = REG_CONFIG_DLPF_CFG_92HZ;
      break;

    case mpu6500GyroFreq_41Hz:
      reg = REG_CONFIG_DLPF_CFG_41HZ;
      break;

    case mpu6500GyroFreq_20Hz:
      reg = REG_CONFIG_DLPF_CFG_20HZ;
      break;

    case mpu6500GyroFreq_10Hz:
      reg = REG_CONFIG_DLPF_CFG_10HZ;
      break;

    case mpu6500GyroFreq_5Hz:
      reg = REG_CONFIG_DLPF_CFG_5HZ;
      break;

    case mpu6500GyroFreq_250Hz:
    default:
      reg = REG_CONFIG_DLPF_CFG_250HZ;
  }
  sta = registerWrite8(i2c, addr, REG_CONFIG, reg);
  return sta == i2cTransferDone;
}

bool mpu6500_ConfigureGyroScale(I2C_TypeDef *i2c, uint8_t addr, Mpu6500GyroScale_t scale)
{
  uint8_t reg;
  I2C_TransferReturn_TypeDef sta;

  switch (scale) {
    case mpu6500GyroScale_500:
      reg = REG_GYRO_CONFIG_GYRO_FS_SEL_500DPS;
      gyrScale = 500;
      gyroCalibrationFactor = 4;
      break;

    case mpu6500GyroScale_1000:
      reg = REG_GYRO_CONFIG_GYRO_FS_SEL_1000DPS;
      gyrScale = 1000;
      gyroCalibrationFactor = 2;
      break;

    case mpu6500GyroScale_2000:
      reg = REG_GYRO_CONFIG_GYRO_FS_SEL_2000DPS;
      gyrScale = 2000;
      gyroCalibrationFactor = 1;
      break;

    case mpu6500GyroScale_250:
    default:
      reg = REG_GYRO_CONFIG_GYRO_FS_SEL_250DPS;
      gyrScale = 250;
      gyroCalibrationFactor = 8;
      break;
  }
  reg |= REG_GYRO_CONFIG_FCHOICE_B_USE_DLPF;
  sta = registerWrite8(i2c, addr, REG_GYRO_CONFIG, reg);
  return sta == i2cTransferDone;
}

int32_t mpu6500_GyroRegToAngle(int16_t reg)
{
  int32_t angle;

  angle = (int32_t)reg * 100 * gyrScale / 32768;

  return angle;
}

bool mpu6500_Measure(I2C_TypeDef *i2c, uint8_t addr,
                     int16_t *accX, int16_t *accY, int16_t *accZ,
                     bool *accRangeError,
                     int16_t *gyrX, int16_t *gyrY, int16_t *gyrZ,
                     bool *gyrRangeError,
                     bool wait)
{
  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef ret;
  uint8_t                    i2c_write_data[1];
  uint8_t                    i2c_read_data[14];

  if (wait) {
    waitForNewData(i2c, addr);
  }

  seq.addr  = addr;
  seq.flags = I2C_FLAG_WRITE_READ;
  // Select command to issue
  i2c_write_data[0] = REG_ACCE_TEMP_GYRO;
  seq.buf[0].data   = i2c_write_data;
  seq.buf[0].len    = 1;
  // Select location/length of data to be read
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len  = 14;

  ret = I2CSPM_Transfer(i2c, &seq);
  if (ret != i2cTransferDone) {
    return false;
  }

  // Get accelerometer stuff
  *accX = BUFFER_TO_INT16(i2c_read_data, 0);
  *accY = BUFFER_TO_INT16(i2c_read_data, 2);
  *accZ = BUFFER_TO_INT16(i2c_read_data, 4);
  // Test for range error
  *accRangeError = (abs(*accX >= ERROR_LEVEL_ACC_REG)
                    || (abs(*accY) >= ERROR_LEVEL_ACC_REG)
                    || (abs(*accZ) >= ERROR_LEVEL_ACC_REG));

  // Get gyrometer stuff
  *gyrX = BUFFER_TO_INT16(i2c_read_data, 8);
  *gyrY = BUFFER_TO_INT16(i2c_read_data, 10);
  *gyrZ = BUFFER_TO_INT16(i2c_read_data, 12);
  // Test for range error
  *gyrRangeError = (abs(*gyrX >= ERROR_LEVEL_ACC_REG)
                    || (abs(*gyrY) >= ERROR_LEVEL_ACC_REG)
                    || (abs(*gyrZ) >= ERROR_LEVEL_ACC_REG));

  gyroCalibrateStep(i2c, addr, *gyrX, *gyrY, *gyrZ);

#if SHOW_GYROOFS
  *accX = gyrOfsX;
  *accY = gyrOfsY;
  *accZ = gyrOfsZ;
#endif

  return true;
}

void mpu6500_GyroCalibrateBegin(uint32_t cnt)
{
  gyroCalibrateBegin(cnt);
}

bool mpu6500_GyroCalibrateInProgress(void)
{
  return gyroCalibrationProgressCnt > 0;
}

void mpu6500_GyroCalibrateReset(I2C_TypeDef *i2c, uint8_t addr)
{
  registerWrite16(i2c, addr, REG_XG_OFFSET, 0);
  registerWrite16(i2c, addr, REG_YG_OFFSET, 0);
  registerWrite16(i2c, addr, REG_ZG_OFFSET, 0);
}
