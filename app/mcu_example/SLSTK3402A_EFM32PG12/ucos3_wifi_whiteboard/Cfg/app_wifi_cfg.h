/**************************************************************************//**
 * @file app_wifi_cfg.h
 * @brief Wireless Whiteboard Configuration File
 * WGM110 and GLIB demo for the SLSTK3402A running on uC/OS-III.
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

#ifndef APP_WIFI_CFG_H
#define APP_WIFI_CFG_H

// -----------------------------------------------------------------------------
// Global Variables

// WiFi Access Point Credentials
#define  APP_WIFI_AP_SSID                "Your_SSID"
#define  APP_WIFI_AP_PWD                 "Your_Password"

#endif  // APP_WIFI_CFG_H
