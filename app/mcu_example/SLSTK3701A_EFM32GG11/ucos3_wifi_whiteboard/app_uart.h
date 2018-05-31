/**************************************************************************//**
 * @file app_uart.h
 * @brief Wireless Whiteboard's UART Application Header File
 * WGM110 and GLIB demo for the SLSTK3701A running on uC/OS-III.
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

#ifndef APP_UART_H
#define APP_UART_H

// -----------------------------------------------------------------------------
// Externs

#ifdef APP_UART_MODULE
#define APP_UART_MODULE_EXT
#else
#define APP_UART_MODULE_EXT extern
#endif

// -----------------------------------------------------------------------------
// Data Types

// Callback function pointer data type
typedef void (*APP_UART_RxCallback_t)(void);

// -----------------------------------------------------------------------------
// Global Variables

/// Number of unread bytes in the ring buffer
APP_UART_MODULE_EXT volatile uint16_t APP_UART_RxCount;

// -----------------------------------------------------------------------------
// Function Prototypes

void APP_UART_Init(void);
void APP_UART_RegisterCallback(APP_UART_RxCallback_t callbackPtr);
char APP_UART_Read(void);
void APP_UART_Write(uint8 len1, uint8* data1, uint16 len2, uint8* data2);
void APP_UART_ClearBuffer(void);
void APP_UART_Start(void);
void APP_UART_Stop(void);

#endif  // APP_UART_H
