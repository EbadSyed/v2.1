// File: uart-link-protocol.h
//
// Description: serial link protocol values, used over both SPI and UART links
//
// Copyright 2013 by Silicon Laboratories. All rights reserved.             *80*

#ifndef UART_LINK_PROTOCOL_H
#define UART_LINK_PROTOCOL_H

//  Each message has a four-byte header:
//   '[' type high-length-byte low-length-byte
//
// (With only three types and a two-byte length we could encode the type in
// the high byte of the length.)

#define UART_LINK_HEADER_SIZE 4

typedef enum {
  UART_LINK_TYPE_MANAGEMENT        = 1,
  UART_LINK_TYPE_THREAD_DATA       = 2,
  UART_LINK_TYPE_UNSECURED_DATA    = 3,
  UART_LINK_TYPE_ALT_DATA          = 4,
  UART_LINK_TYPE_COMMISSIONER_DATA = 5
} SerialLinkMessageType; // use 'serial' in new names, for both UART and SPI

#define MAX_UART_LINK_TYPE UART_LINK_TYPE_COMMISSIONER_DATA
#define EMBER_APPLICATION_HAS_NOTIFY_SERIAL_RX

#endif // UART_LINK_PROTOCOL_H
