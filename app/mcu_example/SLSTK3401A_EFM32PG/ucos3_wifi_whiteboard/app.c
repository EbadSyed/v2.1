/**************************************************************************//**
 * @file app.c
 * @brief Wireless Whiteboard's Application
 * WGM110 and GLIB demo for the SLSTK3401A running on uC/OS-III.
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

#include <cpu.h>
#include <lib_mem.h>
#include <os.h>
#include <app_cfg.h>
#include <app_os_hooks.h>

#include "bsp.h"
#include "bspconfig.h"
#include "bsp_os.h"

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
 * @brief Initialize uC/OS-III and the application tasks.
 *****************************************************************************/
void APP_Init(void)
{
  OS_ERR err;

  // Initialize Memory Management Module
  Mem_Init();
  // Disable all Interrupts.
  CPU_IntDis();

#if (OS_CFG_TRACE_EN == DEF_ENABLED)
  // Initialize the uC/OS-III Trace recorder.
  OS_TRACE_INIT();
#endif

  // Initialize uC/OS-III.
  OSInit(&err);

  // Configure application hooks
  App_OS_SetAllHooks();

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
  OS_ERR osErr;

  // Prevent 'variable unused' compiler warning.
  (void)&pArg;

  // Initialize BSP functions.
  BSP_OSTickInit();

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
