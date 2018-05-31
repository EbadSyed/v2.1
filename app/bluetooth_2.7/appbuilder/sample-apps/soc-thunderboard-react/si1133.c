/***********************************************************************************************//**
 * \file   si1133.c
 * \brief  Si1133 Driver
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include "si1133.h"
#include <string.h>

/***************************************************************************************************
   Local Macros and Definitions
***************************************************************************************************/

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

#define SI1133_DEVICE_ID                0x33

#define SI1133_REG_PART_ID              0x00
#define SI1133_REG_REV_ID               0x01
#define SI1133_REG_MFR_ID               0x02
#define SI1133_REG_HOSTIN0              0x0a
#define SI1133_REG_COMMAND              0x0b
#define SI1133_REG_IRQ_ENABLE           0x0f
#define SI1133_REG_RESPONSE1            0x10
#define SI1133_REG_RESPONSE0            0x11
#define SI1133_REG_IRQ_STATUS           0x12
#define SI1133_REG_HOSTOUT0             0x13
#define SI1133_REG_HOSTOUT1             0x14
#define SI1133_REG_HOSTOUT2             0x15
#define SI1133_REG_HOSTOUT3             0x16
#define SI1133_REG_HOSTOUT4             0x17
#define SI1133_REG_HOSTOUT5             0x18
#define SI1133_REG_HOSTOUT6             0x19
#define SI1133_REG_HOSTOUT7             0x1a
#define SI1133_REG_HOSTOUT8             0x1b
#define SI1133_REG_HOSTOUT9             0x1c

#define SI1133_COMMAND_RESET_CMD_CTR    0x00
#define SI1133_COMMAND_RESET_SW         0x01
#define SI1133_COMMAND_FORCE            0x11
#define SI1133_COMMAND_PAUSE            0x12
#define SI1133_COMMAND_START            0x13
#define SI1133_COMMAND_PARAM_QUERY      0x40
#define SI1133_COMMAND_PARAM_SET        0x80

#define SI1133_PARAM_I2C_ADDR           0x00
#define SI1133_PARAM_CHAN_LIST          0x01
#define SI1133_PARAM_CHANNEL_0_SETUP    0x02

#define SI1133_RESPONSE0_CHIPSTAT_MASK  0xe0
#define SI1133_RESPONSE0_CMND_CTR_MASK  0x0f
#define SI1133_RESPONSE0_SLEEP          0x20

#define SI1133_PARAM_OFFSET_MASK        0x3F

#define SI1133_SIZEOF_CCB               4

#define SI1133_CH_0                     0x01
#define SI1133_CH_1                     0x02
#define SI1133_CH_2                     0x04
#define SI1133_CH_3                     0x08

/** @endcond */

/***************************************************************************************************
 * Local Type Definitions
 **************************************************************************************************/

typedef enum {
  AdcMux_SmallIR    = 0x00,
  AdcMux_MediumIR   = 0x01,
  AdcMux_LargeIR    = 0x02,
  AdcMux_White      = 0x0b,
  AdcMux_LargeWhite = 0x0d,
  AdcMux_UV         = 0x18,
  AdcMux_UVDeep     = 0x19,
  AdcMux_Temp       = 0x1c,
} AdcMux_t;

typedef enum {
  BitsOut_16 = 0,
  BitsOut_24 = 1,
} BitsOut_t;

typedef struct {
  uint8_t       ccbDecimRate;
  AdcMux_t      ccbAdcMux;
  uint8_t       ccbHsig;
  uint8_t       ccbSwGain;
  uint8_t       ccbHwGain;
  BitsOut_t     ccbBitsOut;
  uint8_t       ccbPostShift;
  uint8_t       ccbThresholdSel;
  uint8_t       ccbCounterIndex;
  uint8_t       ccbLedTrim;
  uint8_t       ccbBankSel;
} ChannelConfigBlock_t;

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

static uint8_t  activeChannels = 0;

/***************************************************************************************************
 * Local Function Definitions
 **************************************************************************************************/

// Read a one-byte variable from the register space
static I2C_TransferReturn_TypeDef registerRead8(I2C_TypeDef *i2c, uint8_t addr, uint8_t reg, uint8_t *val)
{
  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef sta;
  uint8_t                    i2c_write_data[1];
  uint8_t                    i2c_read_data[1];

  seq.addr  = addr;
  seq.flags = I2C_FLAG_WRITE_READ;
  /* Select command to issue */
  i2c_write_data[0] = reg;
  seq.buf[0].data   = i2c_write_data;
  seq.buf[0].len    = 1;
  /* Select location/length of data to be read */
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len  = 1;

  sta = I2CSPM_Transfer(i2c, &seq);
  if (sta != i2cTransferDone) {
    return sta;
  }
  if (NULL != val) {
    *val = i2c_read_data[0];
  }
  return sta;
}

// Read a three-byte variable from the register space
static I2C_TransferReturn_TypeDef registerRead24(I2C_TypeDef *i2c, uint8_t addr, uint8_t reg, uint32_t *val)
{
  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef sta;
  uint8_t                    i2c_write_data[1];
  uint8_t                    i2c_read_data[3];

  seq.addr  = addr;
  seq.flags = I2C_FLAG_WRITE_READ;
  /* Select command to issue */
  i2c_write_data[0] = reg;
  seq.buf[0].data   = i2c_write_data;
  seq.buf[0].len    = 1;
  /* Select location/length of data to be read */
  seq.buf[1].data = i2c_read_data;
  seq.buf[1].len  = 3;

  sta = I2CSPM_Transfer(i2c, &seq);
  if (sta != i2cTransferDone) {
    return sta;
  }
  if (NULL != val) {
    *val = (i2c_read_data[0] << 16) + (i2c_read_data[1] << 8) + (i2c_read_data[2]);
  }
  return sta;
}

// Write a one-byte variable to the register space
static I2C_TransferReturn_TypeDef registerWrite8(I2C_TypeDef *i2c, uint8_t addr, uint8_t reg, uint8_t val)
{
  I2C_TransferSeq_TypeDef    seq;
  I2C_TransferReturn_TypeDef sta;
  uint8_t                    i2c_write_data[2];

  seq.addr  = addr;
  seq.flags = I2C_FLAG_WRITE;
  /* Select command to issue */
  i2c_write_data[0] = reg;
  i2c_write_data[1] = val;
  seq.buf[0].data   = i2c_write_data;
  seq.buf[0].len    = 2;
  /* Select location/length of data to be read */
  seq.buf[1].data = 0;
  seq.buf[1].len  = 0;

  sta = I2CSPM_Transfer(i2c, &seq);

  return sta;
}

// Issue a command and wait for the response
static I2C_TransferReturn_TypeDef executeCommand(I2C_TypeDef *i2c, uint8_t addr, uint8_t cmd, bool wait)
{
  I2C_TransferReturn_TypeDef sta;
  uint8_t response_pre;
  uint8_t response_pos;

  if (wait) {
    sta = registerRead8(i2c, addr, SI1133_REG_RESPONSE0, &response_pre);
    if (sta != i2cTransferDone) {
      return sta;
    }
  }
  sta = registerWrite8(i2c, addr, SI1133_REG_COMMAND, cmd);
  if (sta != i2cTransferDone) {
    return sta;
  }
  if (wait) {
    do {
      sta = registerRead8(i2c, addr, SI1133_REG_RESPONSE0, &response_pos);
      if (sta != i2cTransferDone) {
        return sta;
      }
    } while ((response_pos & SI1133_RESPONSE0_CMND_CTR_MASK)
             == (response_pre & SI1133_RESPONSE0_CMND_CTR_MASK));
  }
  return sta;
}

// Write a one-byte variable to the memory space
static void paramSet(I2C_TypeDef *i2c, uint8_t addr, uint8_t ofs, uint8_t val)
{
  uint8_t cmd;

  registerWrite8(i2c, addr, SI1133_REG_HOSTIN0, val);
  cmd = SI1133_COMMAND_PARAM_SET + (ofs & SI1133_PARAM_OFFSET_MASK);
  executeCommand(i2c, addr, cmd, true);
}

// Write a channel configuration block to a given channel.
static void writeChannelConfigBlock(I2C_TypeDef *i2c, uint8_t addr, uint8_t channel, ChannelConfigBlock_t *ccb)
{
  uint8_t ofs;
  uint8_t reg;

  ofs = SI1133_PARAM_CHANNEL_0_SETUP + (channel * SI1133_SIZEOF_CCB);
  reg = (ccb->ccbDecimRate << 5) + (ccb->ccbAdcMux);
  paramSet(i2c, addr, ofs, reg);

  ofs++;
  reg = (ccb->ccbHsig << 7) + (ccb->ccbSwGain << 4) + (ccb->ccbHwGain);
  paramSet(i2c, addr, ofs, reg);

  ofs++;
  reg = (ccb->ccbBitsOut << 6) + (ccb->ccbPostShift << 3) + (ccb->ccbThresholdSel);
  paramSet(i2c, addr, ofs, reg);

  ofs++;
  reg = (ccb->ccbCounterIndex << 6) + (ccb->ccbLedTrim << 4) + (ccb->ccbBankSel << 3);
  paramSet(i2c, addr, ofs, reg);
}

/** @endcond (DO_NOT_INCLUDE_WITH_DOXYGEN) */

/***************************************************************************************************
 * Public Function Definitions
 **************************************************************************************************/
bool si1133_Detect(I2C_TypeDef *i2c, uint8_t addr)
{
  I2C_TransferReturn_TypeDef sta;
  uint8_t dat;

  sta = registerRead8(i2c, addr, 0, &dat);
  if (sta == i2cTransferDone) {
    if (dat != SI1133_DEVICE_ID) {
      return false;
    }
  }
  return sta == i2cTransferDone;
}

void si1133_Reset(I2C_TypeDef *i2c, uint8_t addr)
{
  I2C_TransferReturn_TypeDef sta;
  uint8_t response;

  sta = registerWrite8(i2c, addr, SI1133_REG_COMMAND, SI1133_COMMAND_RESET_SW);

  // Wait until the chip has entered sleep.
  // Accessing registers before sleep will result in chip hang.
  while (sta == i2cTransferDone) {
    sta = registerRead8(i2c, addr, SI1133_REG_IRQ_STATUS, &response);
    if ((response & SI1133_RESPONSE0_CHIPSTAT_MASK) == SI1133_RESPONSE0_SLEEP) {
      break;
    }
  }
}

bool si1133_MeasurementConfigure(I2C_TypeDef *i2c, uint8_t addr, Si1133Configuration_t cfg)
{
  I2C_TransferReturn_TypeDef sta;
  ChannelConfigBlock_t ccb;
  uint8_t       response;

  memset(&ccb, 0, sizeof(ccb));

  if (cfg == configureForUvi) {
    // ccb 0
    ccb.ccbDecimRate = 3;
    ccb.ccbAdcMux = AdcMux_UV;
    ccb.ccbHsig = 0;
    ccb.ccbSwGain = 7;
    ccb.ccbHwGain = 1;
    ccb.ccbBitsOut = BitsOut_24;
    ccb.ccbPostShift = 0;
    ccb.ccbThresholdSel = 0;
    writeChannelConfigBlock(i2c, addr, 0, &ccb);
    // Enable channel 0.
    activeChannels = SI1133_CH_0;
  } else {
    // ccb 0
    ccb.ccbDecimRate = 2;
    ccb.ccbAdcMux = AdcMux_LargeWhite;
    ccb.ccbHsig = 1;
    ccb.ccbSwGain = 6;
    ccb.ccbHwGain = 1;
    ccb.ccbBitsOut = BitsOut_24;
    ccb.ccbPostShift = 0;
    ccb.ccbThresholdSel = 0;
    writeChannelConfigBlock(i2c, addr, 1, &ccb);
    // ccb 1
    ccb.ccbDecimRate = 2;
    ccb.ccbAdcMux = AdcMux_MediumIR;
    ccb.ccbHsig = 1;
    ccb.ccbSwGain = 6;
    ccb.ccbHwGain = 1;
    ccb.ccbBitsOut = BitsOut_24;
    ccb.ccbPostShift = 2;
    ccb.ccbThresholdSel = 0;
    writeChannelConfigBlock(i2c, addr, 2, &ccb);
    // ccb 2
    ccb.ccbDecimRate = 2;
    ccb.ccbAdcMux = AdcMux_LargeWhite;
    ccb.ccbHsig = 1;
    ccb.ccbSwGain = 0;
    ccb.ccbHwGain = 7;
    ccb.ccbBitsOut = BitsOut_24;
    ccb.ccbPostShift = 0;
    ccb.ccbThresholdSel = 0;
    writeChannelConfigBlock(i2c, addr, 3, &ccb);
    // Enable channels 0, 1 and 2.
    activeChannels = SI1133_CH_0 + SI1133_CH_1 + SI1133_CH_2;
  }

  // Read the interrupt status register to clear any pending int.
  sta = registerRead8(i2c, addr, SI1133_REG_IRQ_STATUS, &response);

  paramSet(i2c, addr, SI1133_PARAM_CHAN_LIST, activeChannels);
  sta = registerWrite8(i2c, addr, SI1133_REG_IRQ_ENABLE, activeChannels);

  return sta == i2cTransferDone;
}

bool si1133_MeasurementStart(I2C_TypeDef *i2c, uint8_t addr)
{
  I2C_TransferReturn_TypeDef sta;

  // Force one measurement
  sta = executeCommand(i2c, addr, SI1133_COMMAND_FORCE, false);

  return sta == i2cTransferDone;
}

bool si1133_MeasurementRead(I2C_TypeDef *i2c, uint8_t addr, Si1133ChData_t *chData)
{
  I2C_TransferReturn_TypeDef sta;
  uint8_t response;

  // Read the interrupt status register to clear interrupt.
  sta = registerRead8(i2c, addr, SI1133_REG_IRQ_STATUS, &response);

  sta = registerRead24(i2c, addr, SI1133_REG_HOSTOUT0, (uint32_t *)&(chData->ch0));
  if (chData->ch0 & 0x00800000) {
    chData->ch0 |= 0xff000000;
  }
  sta = registerRead24(i2c, addr, SI1133_REG_HOSTOUT3, (uint32_t *)&(chData->ch1));
  if (chData->ch1 & 0x00800000) {
    chData->ch1 |= 0xff000000;
  }
  sta = registerRead24(i2c, addr, SI1133_REG_HOSTOUT6, (uint32_t *)&(chData->ch2));
  if (chData->ch2 & 0x00800000) {
    chData->ch2 |= 0xff000000;
  }

  return sta == i2cTransferDone;
}

//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
// This code is from the sensors group - and is used as is.

#define X_ORDER_MASK 0x0070
#define Y_ORDER_MASK 0x0007
#define SIGN_MASK    0x0080
#define get_x_order(m)   ( (m & X_ORDER_MASK) >> 4)
#define get_y_order(m)   ( (m & Y_ORDER_MASK)      )
#define get_sign(m)      ( (m & SIGN_MASK) >> 7)

typedef struct {
  int16_t     info;
  uint16_t    mag;
} COEFF;

int32_t poly_inner(int32_t input,
                   int8_t  fraction,
                   uint16_t mag,
                   int8_t  shift)
{
  if (shift < 0) {
    return ( (input << fraction) / mag) >> -shift;
  } else {
    return ( (input << fraction) / mag) << shift;
  }
}

int32_t eval_poly(int32_t x,
                  int32_t y,
                  uint8_t input_fraction,
                  uint8_t output_fraction,
                  uint8_t num_coeff,
                  COEFF  *kp
                  )
{
  uint8_t  info, x_order, y_order, counter;
  int8_t   sign, shift;
  uint16_t mag;
  int32_t  output = 0, x1, x2, y1, y2;

  for (counter = 0; counter < num_coeff; counter++) {
    info    = kp->info;
    x_order = get_x_order(info);
    y_order = get_y_order(info);

    shift   = ((uint16_t)kp->info & 0xff00) >> 8;
    shift  ^= 0x00ff;
    shift  += 1;
    shift   = -shift;

    mag     = kp->mag;

    if ( get_sign(info) ) {
      sign = -1;
    } else {
      sign = 1;
    }

    if ( (x_order == 0) && (y_order == 0) ) {
      output += sign * mag << output_fraction;
    } else {
      if ( x_order > 0 ) {
        x1 = poly_inner(x, input_fraction, mag, shift);
        if ( x_order > 1 ) {
          x2 = poly_inner(x, input_fraction, mag, shift);
        } else {
          x2 = 1;
        }
      } else {
        x1 = 1; x2 = 1;
      }

      if ( y_order > 0 ) {
        y1 = poly_inner(y, input_fraction, mag, shift);
        if ( y_order > 1 ) {
          y2 = poly_inner(y, input_fraction, mag, shift);
        } else {
          y2 = 1;
        }
      } else {
        y1 = 1; y2 = 1;
      }

      output += sign * x1 * x2 * y1 * y2;
    }
    kp++;
  }
  if ( output < 0 ) {
    output = -output;
  }
  return output;
}

//
// Initialize UV coefficients
//
COEFF uk[2] = { { 1281, 30902 }, { -638, 46301 } };

#define UV_INPUT_FRACTION       15
#define UV_OUTPUT_FRACTION      12
#define UV_NUMCOEFF             2

//
// This is the main entry point for computing uv. The value returned by
// get_uv is scaled by UV_OUTPUT_FRACTION
//
// In order to get lux as an integer, do this:
//
//   uvi = get_uv(uv, uk) / ( 1 << UV_OUTPUT_FRACTION )
//
int32_t get_uv(int32_t uv,
               COEFF *uk)
{
  int32_t uvi;

  uvi = eval_poly(0,
                  uv,
                  UV_INPUT_FRACTION,
                  UV_OUTPUT_FRACTION,
                  UV_NUMCOEFF,
                  uk);
  return uvi;
}

typedef struct {
  COEFF   coeff_high[4];
  COEFF   coeff_low[9];
} LUX_COEFF;

//
// Initialize coefficients
//
LUX_COEFF lk = { { { 0, 209 },      // coeff_high[0]
                   { 1665, 93 },   // coeff_high[1]
                   { 2064, 65 },   // coeff_high[2]
                   { -2671, 234 } }, // coeff_high[3]
                 { { 0, 0 },   // coeff_low[0]
                   { 1921, 29053 },   // coeff_low[1]
                   { -1022, 36363 },   // coeff_low[2]
                   { 2320, 20789 },   // coeff_low[3]
                   { -367, 57909 },   // coeff_low[4]
                   { -1774, 38240 },   // coeff_low[5]
                   { -608, 46775 },   // coeff_low[6]
                   { -1503, 51831 },   // coeff_low[7]
                   { -1886, 58928 } } }; // coeff_low[8]

#define ADC_THRESHOLD           16000
#define INPUT_FRACTION_HIGH     7
#define INPUT_FRACTION_LOW      15
#define LUX_OUTPUT_FRACTION     12
#define NUMCOEFF_LOW            9
#define NUMCOEFF_HIGH           4

//
// This is the main entry point for computing lux. The value returned by
// get_lux is scaled by LUX_OUTPUT_FRACTION
//
// In order to get lux as an integer, do this:
//
//   lux = get_lux(vis_high, vis_low, ir, &lk) / ( 1 << LUX_OUTPUT_FRACTION )
//
int32_t get_lux(int32_t vis_high,
                int32_t vis_low,
                int32_t ir,
                LUX_COEFF *lk)
{
  int32_t lux;

  if ( (vis_high > ADC_THRESHOLD) || (ir > ADC_THRESHOLD) ) {
    lux = eval_poly(vis_high,
                    ir,
                    INPUT_FRACTION_HIGH,
                    LUX_OUTPUT_FRACTION,
                    NUMCOEFF_HIGH,
                    &(lk->coeff_high[0]) );
  } else {
    lux = eval_poly(vis_low,
                    ir,
                    INPUT_FRACTION_LOW,
                    LUX_OUTPUT_FRACTION,
                    NUMCOEFF_LOW,
                    &(lk->coeff_low[0]) );
  }
  return lux;
}

// End of code from the sensors group.
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************
//************************************************************************************************

uint32_t si1133_CalculateLux(Si1133ChData_t *chData)
{
  float lux;
  uint32_t result;

  lux = (float)get_lux(chData->ch0,
                       chData->ch2,
                       chData->ch1,
                       &lk);
  lux = lux / (1 << LUX_OUTPUT_FRACTION);
  if (lux < 0) {
    lux = 0;
  }

  // Scale to 0.01 lux
  lux *= 100;
  result = (uint32_t)lux;

  return result;
}

uint8_t si1133_CalculateUvi(Si1133ChData_t *chData)
{
  float uvi;
  uint8_t result;

  uvi = (float)get_uv(chData->ch0, uk);
  uvi = uvi / (1 << UV_OUTPUT_FRACTION);
  if (uvi < 0) {
    uvi = 0;
  } else if (uvi > 255) {
    uvi = 255;
  }

  result = (uint8_t)uvi;
  if (result > 15) {
    result = 15;
  }

  return result;
}
