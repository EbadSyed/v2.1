/**************************************************************************//**
 * @file app.c
 * @brief Wireless Whiteboard's Application
 * WGM110 and GLIB demo for the SLSTK3701A running on MicOS.
 * @version 5.3.5
 ******************************************************************************
 * # License * <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silicon Labs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"

#include <app_cfg.h>

#include <bsp_os.h>
#include "bsp.h"
#include "bspconfig.h"

#include <rtos/cpu/include/cpu.h>
#include <rtos/common/include/common.h>
#include <rtos/kernel/include/os.h>

#include <rtos/common/include/lib_def.h>
#include <rtos/common/include/rtos_utils.h>
#include <rtos/common/include/toolchains.h>

#include "app_wifi.h"

// -----------------------------------------------------------------------------
// Global Variables

/// Kernel startup task TCB (task control block)
static OS_TCB appTaskStartTCB;

/// Kernel startup task stack
static CPU_STK appTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

// -----------------------------------------------------------------------------
// Function Prototypes

static void AppTaskStart(void *pArg);

/**************************************************************************//**
 * @brief Initialize MicOS and the application tasks.
 *****************************************************************************/
void APP_Init(void)
{
  RTOS_ERR err;

  // Create the startup task
  OSTaskCreate(&appTaskStartTCB,
              "App Task Start",
               AppTaskStart,
               0u,
               APP_CFG_TASK_START_PRIO,
              &appTaskStartStk[0u],
               appTaskStartStk[APP_CFG_TASK_START_STK_SIZE / 10u],
               APP_CFG_TASK_START_STK_SIZE,
               0u,
               0u,
               0u,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
              &err);
  
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
}

/**************************************************************************//**
 * @brief Startup task handler
 * This task is the first task to execute and it is
 * responsible for initializing the application.
 * @param p_arg Kernel task optional argument pointer.
 * @returns Void.
 * @note This task needs to have a fairly high priority.
 *****************************************************************************/
static void AppTaskStart(void *pArg)
{
  RTOS_ERR osErr;

  // Prevent 'variable unused' compiler warning.
  (void)&pArg;

  // Initialize Kernel tick source.
  BSP_TickInit();
  Common_Init(&osErr);
  BSP_OS_Init(); 
      
  // Initialize the uC/CPU services.
  CPU_Init();

#if OS_CFG_STAT_TASK_EN > 0u
  // Compute CPU capacity with no task running.
  OSStatTaskCPUUsageInit(&osErr);
  OSStatReset(&osErr);
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
  CPU_IntDisMeasMaxCurReset();
#endif

  // Initialize the Wireless Whiteboard application.
  APP_WIFI_Init();

  // Remove this task from the ready list.
  OSTaskSuspend(DEF_NULL, &osErr);
}