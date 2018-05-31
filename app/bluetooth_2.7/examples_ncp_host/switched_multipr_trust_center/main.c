/***********************************************************************************************//**
 * \file   main.c
 * \brief  Silicon Labs Switched Multiprotocol Trust Center for BLE Example Application
 *
 * This BLE NCP host server application shows how to provide a Multiprotocol
 * Stack Interface (MPSI) based application that serves as a bridge between the
 * BLE client (ie. Mobile Application) and the NCP host application of an other
 * Silicon Labs stack.
 ***************************************************************************************************
 * <b> (C) Copyright 2016 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silicon Labs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include COMMON_HEADER
BGLIB_DEFINE();
#include "platform.h"
#include "mpsi_ble_transport_server.h"
#include "mpsi_handlers.h"
#include "user_command.h"

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

/* Gecko configuration parameters (see gecko_configuration.h) */
#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

#if defined (SILABS_AF_PLUGIN_MPSI_IPC)
void emberAfPluginMpsiIpcInitCallback(void);
void emberAfPluginMpsiIpcTickCallback(void);
#endif // SILABS_AF_PLUGIN_MPSI_IPC

/**
 * @brief  Main function
 */
int main(int argc, char* argv[])
{
  //Initialize BGLIB with our output function for sending messages.
  BGLIB_INITIALIZE_NONBLOCK(on_message_send, uartRx, uartRxPeek);

  if (hw_init(argc, argv) < 0) {
    printf("HW init failure\n");
    exit(EXIT_FAILURE);
  }

  // Flush std output
  fflush(stdout);

  // Reset the NCP target device.
  gecko_cmd_system_reset(0);

#if defined (SILABS_AF_PLUGIN_MPSI_IPC)
  emberAfPluginMpsiIpcInitCallback();
#endif // SILABS_AF_PLUGIN_MPSI_IPC

  cliInitialize();

#if defined(SILABS_SMP_PRINT_BUILD_DATE_TIME)
  printf("\n\n*** Startup ***\nBuild Date: %s\nBuild Time: %s\n", __DATE__, __TIME__);
#endif // SILABS_SMP_PRINT_BUILD_DATE_TIME

  while (1) {
    /* Event pointer for handling events */
    struct gecko_cmd_packet* evt;
    evt = NULL;

    int8_t ret;
    ret = cliHandleInput();

    usleep(100);

    if (RETCODE_USER_COMMAND_ESCAPE == ret) {
      break;
    }

    // Check for stack event.
    evt = gecko_peek_event();

    // MPSI Transport - Handle stack events
    mpsiTransportHandleEvents(evt);

#if defined (SILABS_AF_PLUGIN_MPSI_IPC)
    emberAfPluginMpsiIpcTickCallback();
#endif // SILABS_AF_PLUGIN_MPSI_IPC
  }

  fclose(dfu_file);
  return 0;
}

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */
