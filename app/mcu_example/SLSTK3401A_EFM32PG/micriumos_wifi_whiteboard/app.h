/**************************************************************************//**
 * @file app_uart.h
 * @brief Wireless Whiteboard's Application Header File
 * WGM110 and GLIB demo for the SLSTK3401A running on MicOS.
 * This module initializes the UART to be used by the WiFi module.
 * It also manages the RX interrupts by using a ring buffer.
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

#ifndef APP_H
#define APP_H

void APP_Init(void);

#endif  // APP_H
