/***********************************************************************************************//**
 * \file   main.c
 * \brief  Silicon Labs ThunderBoard React Sensors Demo
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include "ble-configuration.h"

/* BG stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"

/* application specific files */
#include "app.h"
#include "app_ble.h"

/* Board specific headers */
#include "rd0057.h"

/* libraries containing default gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include "em_gpio.h"

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

// Gecko configuration parameters (see gecko_configuration.h)
static const gecko_configuration_t config = {
  .config_flags = 0,
  .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap),
  .bluetooth.sleep_clock_accuracy = 100, // ppm
  .gattdb = &bg_gattdb_data,
  .max_timers = 16,
};

int main(void)
{
  // Initialize board
  boardInit();

  // Initialize stack
  gecko_init(&config);

  while (1) {
    struct gecko_cmd_packet* evt;
    // Check for stack event.
    evt = gecko_wait_event();
    // Run application and event handler.
    appBleHandleEvents(evt);
  }
}

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */
