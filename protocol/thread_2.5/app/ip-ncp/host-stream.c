// File: host-stream.c
//
// Description: host message streams
//
// Copyright 2013 by Silicon Laboratories. All rights reserved.             *80*

#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>

#include PLATFORM_HEADER
#include "stack/core/ember-stack.h"
#include "hal/hal.h"
#include "stack/ip/ip-header.h"
#include "uart-link-protocol.h"
#include "host-stream.h"

#ifdef UNIX_HOST
  #define EMBER_READ   read
  #define EMBER_WRITE  write
  #define EMBER_SELECT select
#else
// simulated I/O for testing
  #include "tool/simulator/child/posix-sim.h"
#endif

void emRemoveStreamBytes(Stream *stream, int count)
{
  assert(count <= stream->index);
  memmove(stream->buffer, stream->buffer + count, stream->index - count);
  stream->index -= count;

  // help the debugger
  MEMSET(stream->buffer + stream->index,
         0,
         sizeof(stream->buffer) - stream->index);
}

IpModemReadStatus readIpModemInput(int fd,
                                   Stream *stream,
                                   IpModemMessageHandler *handler)
{
  int got = EMBER_READ(fd,
                       stream->buffer + stream->index,
                       sizeof(stream->buffer) - stream->index);
  IpModemReadStatus result = IP_MODEM_READ_PENDING;

  if (got < 0) {
    return IP_MODEM_READ_IO_ERROR;
  } else if (0 == got) {
    return IP_MODEM_READ_EOF;
  } else {
    stream->index += got;
    // if (0 < got) {
    //   emLogBytesLine(IP_MODEM, "ipModemRead(%d, ...) ",
    //                  stream->buffer,
    //                  stream->index,
    //                  fd);
    // }
    while (4 <= stream->index) {
      result = processIpModemInput(stream, handler);
      if (result != IP_MODEM_READ_PROGRESS) {
        break;
      }
    }
  }

  return result;
}

IpModemReadStatus processIpModemInput(Stream *stream,
                                      IpModemMessageHandler *handler)
{
  IpModemReadStatus result = IP_MODEM_READ_PENDING;
  uint8_t messageType;
  uint16_t length;
  uint16_t i = 0;

  while (stream->needsFrameSynchronization) {
    // If the ip-driver-app was restarted but the NCP was not we could
    // receive the end of a previous frame that was dropped by the
    // serial interface while the ip-driver-app was not running.
    // So we'll ignore bytes until we get a start-of-frame character.
    for (; i < stream->index && stream->buffer[i] != '['; i++) {
      ;
    }
    if (i != 0) {
      emRemoveStreamBytes(stream, i);
    }
    if (UART_LINK_HEADER_SIZE > stream->index) {
      // we can't claim to be synchronized yet, not until we receive
      // the what looks to be a valid serial frame header so just return
      // Next time we receive data we'll again go through this logic and
      // hopefully synchronize.
      break;
    }
    // Let's make sure the rest of the serial link header looks OK.
    // If message type or length is not valid that means the '[' that
    // we found must not have been the start of a new frame.  So we'll
    // loop and skip the first byte by setting i to 1.
    messageType = stream->buffer[1];
    uint16_t length = emberFetchHighLowInt16u(stream->buffer + 2);
    if (MAX_UART_LINK_TYPE < messageType
        || length == 0
        || (length + UART_LINK_HEADER_SIZE) > sizeof(stream->buffer)) {
      i = 1;
      continue;
    }
    // The header checks out as best we can tell.
    // TODO: it is still possible that we falsely detected a start of frame.
    stream->needsFrameSynchronization = false;
  }

  if (!stream->needsFrameSynchronization
      && UART_LINK_HEADER_SIZE <= stream->index) {
    messageType = stream->buffer[1];
    length = emberFetchHighLowInt16u(stream->buffer + 2);

    if (stream->buffer[0] != '['
        || MAX_UART_LINK_TYPE < messageType
        || length == 0) {
      // We get here if there is a framing error
      result = IP_MODEM_READ_FORMAT_ERROR;
    } else if (UART_LINK_HEADER_SIZE + length <= stream->index) {
      handler(messageType, stream->buffer + UART_LINK_HEADER_SIZE, length);
      emRemoveStreamBytes(stream, length + UART_LINK_HEADER_SIZE);
      result = IP_MODEM_READ_PROGRESS;
    }
  }

  return (stream->index == 0 ? IP_MODEM_READ_DONE : result);
}

IpModemReadStatus readIpv6Input(int fd,
                                Stream *stream,
                                SerialLinkMessageType type,
                                Ipv6PacketHandler *handler)
{
  int got = EMBER_READ(fd,
                       stream->buffer + stream->index,
                       sizeof(stream->buffer) - stream->index);
  if (got < 0) {
    return IP_MODEM_READ_IO_ERROR;
  } else {
    stream->index += got;
    return processIpv6Input(stream, type, handler);
  }
}

IpModemReadStatus processIpv6Input(Stream *stream,
                                   SerialLinkMessageType type,
                                   Ipv6PacketHandler *handler)
{
  IpModemReadStatus result = IP_MODEM_READ_PENDING;

  while (IPV6_HEADER_SIZE <= stream->index) {
    uint16_t length =
      IPV6_HEADER_SIZE
      + emberFetchHighLowInt16u(stream->buffer + IPV6_PAYLOAD_LENGTH_INDEX);
    assert(length <= EMBER_IPV6_MTU);
    if (length <= stream->index) {
      handler(stream->buffer, type, length);
      emRemoveStreamBytes(stream, length);
      result = IP_MODEM_READ_PROGRESS;
    } else {
      break;
    }
  }
  return (stream->index == 0
          ? IP_MODEM_READ_DONE
          : result);
}
