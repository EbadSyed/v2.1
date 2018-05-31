/***********************************************************************************************//**
 * \file   app_hw.c
 * \brief  Hardware specific application code
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* BG stack headers */
#include "bg_types.h"

/* STK header files. */
#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif
#include "bsp.h"

/* Temp sensor and I2c*/
#if defined(HAL_CONFIG)
#include "i2cspmhalconfig.h"
#else
#include "i2cspmconfig.h"
#endif
#include "i2cspm.h"
#include "si7013.h"
#include "tempsens.h"

/* application specific headers */
#include "advertisement.h"
#include "app_ui.h"

/* Own headers*/
#include "app_hw.h"

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app_hw
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

/* Text definitions*/
#define APP_HW_SENSOR_FAIL_TEXT         "Failed to detect\nsi7021 sensor."

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

/** Status flag of the Temperature Sensor. */
static bool si7013_status = false;

/** I2C init structure. */

/***************************************************************************************************
 * Static Function Declarations
 **************************************************************************************************/

static void appBtnCback(AppUiBtnEvt_t btn);

/***************************************************************************************************
 * Public Function Definitions
 **************************************************************************************************/
void appHwInit(void)
{
  /* Register button callback */
  appUiBtnRegister(appBtnCback);

  /* Initialize temperature sensor. */
  if (!appHwInitTempSens()) {
    appUiWriteString(APP_HW_SENSOR_FAIL_TEXT); /* Display error message on screen. */
  }
}
int32_t appHwReadTm(int32_t* tempData)
{
  uint32_t rhData = 0;
  return Si7013_MeasureRHAndTemp(I2C0, SI7021_ADDR, &rhData, tempData);
}

bool appHwInitTempSens(void)
{
  /* Get initial sensor status */
  si7013_status = Si7013_Detect(I2C0, SI7021_ADDR, NULL);
  return si7013_status;
}

/***************************************************************************************************
 * Static Function Definitions
 **************************************************************************************************/

/***********************************************************************************************//**
 *  \brief  Button press callback.
 *  \param[in]  btn  Button press length and button number
 **************************************************************************************************/
static void appBtnCback(AppUiBtnEvt_t btn)
{
  if (APP_UI_BTN_0_SHORT == btn) {
    advSwitchAdvMessage();
  }
}

/** @} (end addtogroup app_hw) */
/** @} (end addtogroup Application) */
