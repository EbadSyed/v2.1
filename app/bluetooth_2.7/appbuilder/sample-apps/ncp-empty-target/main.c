/***************************************************************************//**
 * \file   main.c
 * \brief  Silabs Network Co-Processor (NCP) Empty Target Sample Application.
 * This application allows the user to use as an NCP target device if connected
 * to an appropriate host application.
 *******************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 ******************************************************************************/

#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"

/* BG stack headers */
#include "bg_types.h"
#include "ncp_gecko.h"
#include "gatt_db.h"
#include "ncp_usart.h"
#include "em_core.h"

/* libraries containing default gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"

/* Device initialization header */
#include "hal-config.h"

#ifdef FEATURE_BOARD_DETECTED
#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif // HAL_CONFIG
#endif // FEATURE_BOARD_DETECTED

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 8
#endif
#ifndef MAX_ADVERTISERS
#define MAX_ADVERTISERS 2
#endif
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

// Gecko configuration parameters (see gecko_configuration.h)
static const gecko_configuration_t config = {
  .config_flags = 0,
#if defined(NCP_DEEP_SLEEP_ENABLED)
  .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
#endif
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.max_advertisers = MAX_ADVERTISERS,
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
 * Handle events meant to be handled locally
 */
static uint32_t local_handle_event(struct gecko_cmd_packet *evt)
{
  bool evt_handled = false;
  switch (BGLIB_MSG_ID(evt->header)) {
    default:
      break;
  }
  return evt_handled;
}

void main(void)
{
  // Initialize device
  initMcu();
  // Initialize board
  initBoard();
  // Initialize application
  initApp();

  // Initialize stack
  gecko_init(&config);

  // NCP USART init
  ncp_usart_init();

  while (1) {
    struct gecko_cmd_packet *evt;
    ncp_handle_command();
    /* Check for stack event. */
    evt = gecko_peek_event();
    while (evt) {
      if (!ncp_handle_event(evt) && !local_handle_event(evt)) {
        // send out the event if not handled either by NCP or locally
        ncp_transmit_enqueue(evt);
      }
      evt = gecko_peek_event();
    }
    ncp_transmit();

    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_ATOMIC();
    gecko_sleep_for_ms(gecko_can_sleep_ms());
    CORE_EXIT_ATOMIC();
  }
}

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */
