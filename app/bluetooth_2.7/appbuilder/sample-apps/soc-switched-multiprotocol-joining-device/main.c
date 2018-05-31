/***********************************************************************************************//**
 * \file   main.c
 * \brief  Silicon Labs Switched Multiprotocol Joining Device Example Application
 *
 ***************************************************************************************************
 * <b> (C) Copyright 2016 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silicon Labs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include COMMON_HEADER

/* Device initialization header */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "hal-config.h"

#include "platform.h"
#include "mpsi_ble_transport_server.h"
#include "mpsi_handlers.h"
#include "user_command.h"

#if defined(SILABS_AF_PLUGIN_MPSI_STORAGE)
#include "mpsi-storage.h"
#endif

#if defined(SILABS_AF_PLUGIN_SLOT_MANAGER)
#include "slot-manager.h"
#endif

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

// Gecko configuration parameters (see gecko_configuration.h)
#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

static const gecko_configuration_t config = {
  .config_flags = 0,
  .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap),
  .bluetooth.sleep_clock_accuracy = 100, // ppm
  .gattdb = &bg_gattdb_data,
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
  .pa.config_enable = 1, // Enable high power PA
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
};

/**
 * @brief  Main function
 */
void main()
{
  // Initialize device
  initMcu();
  // Initialize board
  initBoard();
  // Initialize application
  initApp();

  // Initialize LEDs
  BSP_LedsInit();

  // Initialize stack
  gecko_init(&config);

  // Test HW init
  hwInit();

  cliInitialize();

#if defined(SILABS_SMP_PRINT_BUILD_DATE_TIME)
  printf("\n\n*** Startup ***\nBuild Date: %s\nBuild Time: %s\n", __DATE__, __TIME__);
#endif // SILABS_SMP_PRINT_BUILD_DATE_TIME

#if defined(SILABS_AF_PLUGIN_SLOT_MANAGER)
  // Initialize Slot Manager plugin.
  emberAfPluginSlotManagerInitCallback();
#endif // SILABS_AF_PLUGIN_SLOT_MANAGER

#if defined(SILABS_AF_PLUGIN_MPSI_STORAGE)
  // Initialize MPSI Storage plugin.
  emberAfPluginMpsiStorageInitCallback();
#endif // SILABS_AF_PLUGIN_MPSI_STORAGE

  while (1) {
    // Event pointer for handling events
    struct gecko_cmd_packet* evt;
    evt = NULL;

    int8_t ret;
    ret = cliHandleInput();

    // Check for stack event.
    evt = gecko_peek_event();
    // MPSI Transport - Handle stack events
    mpsiTransportHandleEvents(evt);

    // External signal event, received from the BLE stack
    // Note: handled outside of the scope of other BLE stack events
    if ( (NULL != evt) && (BGLIB_MSG_ID(evt->header) == gecko_evt_system_external_signal_id)) {
      // sendBleTestData(evt->data.evt_system_external_signal.extsignals&SIGNAL_SWITCH_STACK0);
      bleMpsiTransportConfirmResponse(evt->data.evt_system_external_signal.extsignals & SIGNAL_SWITCH_STACK1);
    }
  }
}

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */
