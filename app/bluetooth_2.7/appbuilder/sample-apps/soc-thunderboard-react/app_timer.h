/***********************************************************************************************//**
 * \file   app_timer.h
 * \brief  Application timer header file
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef APP_TIMER_H
#define APP_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Public Macros and Definitions
 **************************************************************************************************/

/** Timer Frequency used. */
#define TIMER_CLK_FREQ ((uint32)32768)
/** Convert msec to timer ticks. */
#define TIMER_MS_2_TIMERTICK(ms) ((TIMER_CLK_FREQ * ms) / 1000)
/** Stop timer. */
#define TIMER_STOP 0

/** Application timer enumeration. */
typedef enum {
  UI_TIMER                 =  0,
  ADV_TIMEOUT_TIMER        =  1,
  ADV_ALTERNATE_TIMER      =  2,
  UPDATE_CON_PARAM_TIMER   =  3,
  ACCORI_SERVICE_ACC_TIMER =  4,
  ACCORI_SERVICE_ORI_TIMER =  5,
  BATTERY_DEVICE_TIMER     =  6,
  BATT_SERVICE_TIMER       =  7,
  CSC_SERVICE_TIMER        =  8,
} appTimer_t;

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */

#endif /* APP_TIMER_H */
