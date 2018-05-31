/**************************************************************************//**
 * @file common_init.c
 * @brief Initialization of the Common module.
 * @version 5.3.5
 ******************************************************************************
 * # License
 * <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/
#include  <rtos_description.h>

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/common.h>
#include  <rtos/common/include/auth.h>
#include  <rtos/common/include/rtos_err.h>
#include  <rtos/common/include/rtos_utils.h>

#include  <rtos_cfg.h>

#ifdef  RTOS_MODULE_COMMON_CLK_AVAIL
#include  <rtos/common/include/clk.h>
#endif

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include  <rtos/common/include/shell.h>
#endif

// -----------------------------------------------------------------------------
// Global functions

/***************************************************************************//**
 * @brief
 *   Provides example on how to initialize the Common module and how to recover
 *   the default configuration structure, if needed.
 ******************************************************************************/
void commonInit(void)
{
  RTOS_ERR  err;

  Common_Init(&err);
  APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE,; );

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
  Shell_Init(&err);
  APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE,; );
#endif

#ifdef  RTOS_MODULE_COMMON_CLK_AVAIL
  Clk_Init(&err);
  APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE,; );
#endif

  Auth_Init(&err);
  APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE,; );
}
