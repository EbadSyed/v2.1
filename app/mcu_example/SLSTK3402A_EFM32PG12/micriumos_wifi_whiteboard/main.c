/**************************************************************************//**
 * @file main.c
 * @brief MicOS + WiFi + Graphics LCD Demo
 * @version 5.3.5
 ******************************************************************************
 * # License * <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"

#include <bsp_os.h>
#include "bspconfig.h"
#include "app.h"

#include <rtos/common/include/rtos_utils.h>
#include <rtos/kernel/include/os.h>

/**************************************************************************//**
 * @brief Main entry point.
 *****************************************************************************/
int main(void)
{
  RTOS_ERR err;

  CPU_Init();
  BSP_SystemInit();

  // Initialize Micrium OS Kernel.
  OS_TRACE_INIT();
  OSInit(&err);

  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  // Initialize the application tasks.
  APP_Init();

  // Start Micrium OS
  OSStart(&err);

  // We should never get here.
  while (1) {
    ;
  }
}
