/**************************************************************************//**
 * \file   common_includes.h
 * \brief  Infrastructure for commonly included files in the BLE SDK
 ******************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ******************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *****************************************************************************/

#ifndef __COMMON_INCLUDES__
#define __COMMON_INCLUDES__

#include <string.h>
#include <stdio.h>

// Common defines for NCP host applications
#if defined(BLE_NCP_HOST)
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include "gecko_bglib.h"
#include "uart.h"
#include <unistd.h>

// Common defines for SOC and NCP target applications
#else // BLE_NCP_HOST

// GATT database
#include "gatt_db.h"

// Board Headers
#include "ble-configuration.h"
#include "board_features.h"

// Bluetooth stack headers
#include "bg_types.h"
#include "native_gecko.h"
#include "infrastructure.h"

// EM library (EMlib)
#include "em_system.h"
#include "em_gpio.h"

// Libraries containing default Gecko configuration values
#include "em_emu.h"
#include "em_cmu.h"

#ifdef FEATURE_BOARD_DETECTED
#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif
#include "pti.h"
#else // FEATURE_BOARD_DETECTED
#error This sample app only works with a Silicon Labs Board
#endif // FEATURE_BOARD_DETECTED

#include "bsp.h"

#ifdef FEATURE_IOEXPANDER
#include "bsp_stk_ioexp.h"
#endif // FEATURE_IOEXPANDER

#endif //BLE_NCP_HOST

#include "infrastructure.h"
#include "stack_bridge.h"
#include "ble-callbacks.h"

#endif // __COMMON_INCLUDES__
