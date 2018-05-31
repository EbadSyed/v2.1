/***********************************************************************************************//**
 * \file   aluv_device.c
 * \brief  Ambient Light and UV Device
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include "aluv_device.h"
#include "rd0057.h"
#include "si1133.h"
#include "native_gecko.h"
#include <stddef.h>

/***********************************************************************************************//**
 * @addtogroup app_hardware
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup aluv-sensor
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Local Type Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

static bool     si1133Detected = false;

static bool     uviInProgress = false;
static void     (*uviMeasurementDoneCallback)(uint8_t);

static bool     luxInProgress = false;
static void     (*luxMeasurementDoneCallback)(uint32_t);

/***************************************************************************************************
 * Static Function Declarations
 **************************************************************************************************/

/***************************************************************************************************
 * Public Variable Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Public Function Definitions
 **************************************************************************************************/
void aluvDeviceInterruptEvtHandler(void)
{
  Si1133ChData_t      samples;

  if (uviInProgress) {
    uint8_t  uvi;

    si1133_MeasurementRead(i2cInit.port, SI1133_ADDR, &samples);
    uvi = si1133_CalculateUvi(&samples);
    if (uviMeasurementDoneCallback) {
      uviMeasurementDoneCallback(uvi);
      uviMeasurementDoneCallback = NULL;
    }
    uviInProgress = false;

    if (luxInProgress) {
      si1133_MeasurementConfigure(i2cInit.port, SI1133_ADDR, configureForLux);
      si1133_MeasurementStart(i2cInit.port, SI1133_ADDR);
    }
  } else if (luxInProgress) {
    uint32_t            lux;

    si1133_MeasurementRead(i2cInit.port, SI1133_ADDR, &samples);
    lux = si1133_CalculateLux(&samples);
    if (luxMeasurementDoneCallback != NULL) {
      luxMeasurementDoneCallback(lux);
      luxMeasurementDoneCallback = NULL;
    }
    luxInProgress = false;

    if (uviInProgress) {
      si1133_MeasurementConfigure(i2cInit.port, SI1133_ADDR, configureForUvi);
      si1133_MeasurementStart(i2cInit.port, SI1133_ADDR);
    }
  } else {
    // Nothing
  }
}

void aluvDeviceInit(void)
{
  si1133Detected = si1133_Detect(i2cInit.port, SI1133_ADDR);
  if (si1133Detected) {
    si1133_Reset(i2cInit.port, SI1133_ADDR);
  }
}

void aluvDeviceDeInit(void)
{
  // Nothing
}

uint8_t aluvDeviceTest(void)
{
  return si1133Detected ? 0 : 1;
}

void aluvDeviceSleep(void)
{
  // Nothing
}

void aluvDeviceConnectionOpened(void)
{
  // Nothing
}

void aluvDeviceConnectionClosed(void)
{
  // Nothing
}

void aluvDeviceUviMeasure(void (*uviMeasurementDone)(uint8_t))
{
  if (si1133Detected) {
    uviMeasurementDoneCallback = uviMeasurementDone;
    uviInProgress = true;
    if (!luxInProgress) {
      si1133_MeasurementConfigure(i2cInit.port, SI1133_ADDR, configureForUvi);
      si1133_MeasurementStart(i2cInit.port, SI1133_ADDR);
    }
  } else {
    uviMeasurementDone((uint8_t)0);
  }
}

void aluvDeviceLuxMeasure(void (*luxMeasurementDone)(uint32_t))
{
  if (si1133Detected) {
    luxMeasurementDoneCallback = luxMeasurementDone;
    luxInProgress = true;
    if (!uviInProgress) {
      si1133_MeasurementConfigure(i2cInit.port, SI1133_ADDR, configureForLux);
      si1133_MeasurementStart(i2cInit.port, SI1133_ADDR);
    }
  } else {
    luxMeasurementDone((uint32_t)0);
  }
}

/***************************************************************************************************
 * Static Function Definitions
 **************************************************************************************************/

/** @} (end addtogroup aluv-sensor) */
/** @} (end addtogroup app_hardware) */
