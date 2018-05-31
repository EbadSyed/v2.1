// File: host-stream.c
//
// Description: host message streams
//
// Copyright 2013 by Silicon Laboratories. All rights reserved.             *80*

#ifndef HOST_STREAM_H
#define HOST_STREAM_H

typedef enum {
  IP_MODEM_READ_DONE,          // There is no input pending.
  IP_MODEM_READ_PROGRESS,      // A command executed but there is input pending.
  IP_MODEM_READ_PENDING,       // There is input pending.
  IP_MODEM_READ_FORMAT_ERROR,
  IP_MODEM_READ_IO_ERROR,
  IP_MODEM_READ_EOF
} IpModemReadStatus;

// Struct for buffering partial messages.
typedef struct stream {
  uint8_t buffer[4096];
  uint16_t index;
  bool needsFrameSynchronization;
#ifdef EMBER_TEST
  bool (* readStatusErrorHandler)(IpModemReadStatus status, struct stream *stream);
#endif
} Stream;

//----------------------------------------------------------------
// Reading IP modem messages.

// 'Type' is one of the UART_LINK_TYPE_... values.  'message' points
// to the first byte of message data, and 'length' is the number of
// message bytes.

typedef void (IpModemMessageHandler)(SerialLinkMessageType type,
                                     const uint8_t *message,
                                     uint16_t length);

// Process IP modem messages from 'stream', passing any complete messages
// to 'handler'.

IpModemReadStatus processIpModemInput(Stream *stream,
                                      IpModemMessageHandler *handler);

// Read from 'fd' into 'stream', then call processIpModemInput().

IpModemReadStatus readIpModemInput(int fd,
                                   Stream *stream,
                                   IpModemMessageHandler *handler);

//----------------------------------------------------------------
// Reading streams of IPv6 packets.

typedef void (Ipv6PacketHandler)(const uint8_t *packet,
                                 SerialLinkMessageType type,
                                 uint16_t length);

// Process IPv6 packets ifrom 'stream', passing any complete packets
// to 'handler'.

IpModemReadStatus processIpv6Input(Stream *stream,
                                   SerialLinkMessageType type,
                                   Ipv6PacketHandler *handler);

// Read from 'fd' into 'stream', then call ipModemProcessIpv6.

IpModemReadStatus readIpv6Input(int fd,
                                Stream *stream,
                                SerialLinkMessageType type,
                                Ipv6PacketHandler *handler);

void emRemoveStreamBytes(Stream *stream, int count);

#endif // HOST_STREAM_H
