/**************************************************************************//**
 * @file http_server_init.c
 * @brief Initializes uC/HTTPs
 * @version 5.3.5
 *******************************************************************************
 * # License
 * <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Dependencies and avail check(s)

#include  <rtos_description.h>

#if (defined(RTOS_MODULE_NET_HTTP_SERVER_AVAIL))

// -----------------------------------------------------------------------------
// Include files

#include <rtos/cpu/include/cpu.h>
#include <rtos/common/include/rtos_err.h>
#include <rtos/common/include/rtos_utils.h>
#include <rtos/net/include/http_server.h>

// -----------------------------------------------------------------------------
// Global functions

/***************************************************************************//**
 * Example of initialization of the HTTP server.
 ******************************************************************************/
void Ex_HTTP_Server_Init(void)
{
  RTOS_ERR  err;

  HTTPs_Init(&err); // Initialize the HTTP server
  APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);
}

// -----------------------------------------------------------------------------
// Dependencies and avail check(s)

#endif // RTOS_MODULE_NET_HTTP_SERVER_AVAIL
