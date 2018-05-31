/**************************************************************************//**
 * @file app_wifi.h
 * @brief Wireless Whiteboard's WiFi Application Header File
 * WGM110 and GLIB demo for the SLSTK3401A running on MicOS.
 * This module besides initializing the UART and BGLIB, it also
 * implements the WiFi state machine, responsible for sending
 * commands and processing the corresponding responses and events.
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

#ifndef APP_WIFI_H
#define APP_WIFI_H

// -----------------------------------------------------------------------------
// Externs

#ifdef APP_WIFI_MODULE
#define APP_WIFI_MODULE_EXT
#else
#define APP_WIFI_MODULE_EXT extern
#endif

// -----------------------------------------------------------------------------
// Global Variables

/// WGM110 MAC address.
APP_WIFI_MODULE_EXT char APP_WIFI_MacAddr[16];

/// WGM110 IP address.
APP_WIFI_MODULE_EXT char APP_WIFI_DeviceIpAddr[16];

/// Access Point SSID name.
APP_WIFI_MODULE_EXT char APP_WIFI_ApSsid[32];

/// Server's IP address.
APP_WIFI_MODULE_EXT char APP_WIFI_TcpServerIpAddr[16];

// -----------------------------------------------------------------------------
// Function Prototypes

void APP_WIFI_Init(void);

#endif  // APP_WIFI_H
