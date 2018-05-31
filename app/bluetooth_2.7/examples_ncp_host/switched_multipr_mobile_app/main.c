/***********************************************************************************************//**
 * \file   main.c
 * \brief  Silicon Labs Switched Multiprotocol Mobile App Emulator Application
 *
 * This BLE NCP host client application shows how to connect and interract with
 * Switched Multiprotocol server (ie. Joining Device and Trust Center) devices.
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
#include "mpsi_handlers.h"
#include "mpsi_ble_transport_client.h"
#include "user_command.h"

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

FILE *otaFile = NULL;

// Gecko configuration parameters (see gecko_configuration.h)
#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

/**
 * @brief  Main function
 */
int main(int argc, char* argv[])
{
  ///Initialize BGLIB with our output function for sending messages.
  BGLIB_INITIALIZE_NONBLOCK(on_message_send, uartRx, uartRxPeek);

  if (hw_init(argc, argv) < 0) {
    printf("HW init failure\n");
    exit(EXIT_FAILURE);
  }

  //Open OTA file if available
  otaFile = fopen(argv[3], "rb");
  if (otaFile == NULL) {
    printf("Cannot open OTA file: %s\n", argv[3]);
    exit(-1);
  }

  // Flush std output.
  fflush(stdout);

  // Reset the NCP target device.
  gecko_cmd_system_reset(0);

  cliInitialize();

#if defined(SILABS_SMP_PRINT_BUILD_DATE_TIME)
  printf("\n*** Startup ***\nBuild Date: %s\nBuild Time: %s\n", __DATE__, __TIME__);
#endif // SILABS_SMP_PRINT_BUILD_DATE_TIME

  while (1) {
    // Event pointer for handling events
    struct gecko_cmd_packet* evt;
    evt = NULL;

    int8_t ret;
    ret = cliHandleInput();

    usleep(100);

    // Exit if ESC is pressed
    if (RETCODE_USER_COMMAND_ESCAPE == ret) {
      // Stop device discovery if exiting
      bleStopDeviceDiscovery();
      break;
    }

    // Check for stack event.
    evt = gecko_peek_event();

    // MPSI Transport - Handle stack events
    mpsiTransportHandleEvents(evt);
  }

  fclose(otaFile);
  return 0;
}

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */
