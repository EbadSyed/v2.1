// File: coap-host.c
//
// Description: CoAP Host functionality
//
// Copyright 2015 Silicon Laboratories, Inc.                                *80*

#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "hal/hal.h"
#include "stack/framework/buffer-management.h"
#include "app/coap/coap.h"
#include "app/coap/coap-host.h"
#include "stack/core/log.h"

// for lose()
#include "stack/ip/tls/debug.h"

static EmberStatus submitMessage(const EmberIpv6Address *destination,
                                 uint16_t destinationPort,
                                 Buffer buffer)
{
  return emberSendUdp(destination->bytes,
                      (destinationPort == EMBER_COAP_SECURE_PORT
                       ? EMBER_COAP_SECURE_PORT
                       : EMBER_COAP_PORT),
                      destinationPort,
                      emGetBufferPointer(buffer),
                      emGetBufferLength(buffer));
}

EmberStatus emberFinishCoapMessage(CoapMessage *coapMessage,
                                   uint8_t *coapHeader,
                                   uint16_t coapHeaderLength,
                                   Buffer payloadBuffer,
                                   Buffer *headerLoc)
{
  uint16_t payloadLength = emGetBufferLength(payloadBuffer);
  Buffer message = emAllocateBuffer(coapHeaderLength + payloadLength);

  if (message == NULL_BUFFER) {
    return EMBER_NO_BUFFERS;
  }

  uint8_t *bytes = emGetBufferPointer(message);
  MEMCOPY(bytes, coapHeader, coapHeaderLength);
  MEMCOPY(bytes + coapHeaderLength,
          emGetBufferPointer(payloadBuffer),
          payloadLength);

  EmberStatus result =
    (coapMessage->transmitHandler == NULL
     ? submitMessage(&coapMessage->remoteAddress,
                     coapMessage->remotePort,
                     message)
     : coapMessage->transmitHandler(bytes,
                                    coapHeaderLength + payloadLength,
                                    &coapMessage->localAddress,
                                    coapMessage->localPort,
                                    &coapMessage->remoteAddress,
                                    coapMessage->remotePort,
                                    coapMessage->transmitHandlerData));

  if (result == EMBER_SUCCESS) {
    emLogBytes(COAP, "CoAP TX to [",
               coapMessage->remoteAddress.bytes,
               16);
    emLogLine(COAP, "]");
  }

  if (result == EMBER_SUCCESS) {
    *headerLoc = message;
  }
  return result;
}

EmberStatus emberRetryCoapMessage(CoapRetryEvent *event)
{
  return submitMessage(&event->remoteAddress,
                       event->remotePort,
                       event->packetHeader);
}

void emCoapHostIncomingMessageHandler(const uint8_t *bytes,
                                      uint16_t bytesLength,
                                      const EmberIpv6Address *localAddress,
                                      uint16_t localPort,
                                      const EmberIpv6Address *remoteAddress,
                                      uint16_t remotePort)
{
  EmberCoapRequestInfo info;
  MEMSET(&info, 0, sizeof(info));
  MEMCOPY(&info.localAddress, localAddress, sizeof(EmberIpv6Address));
  MEMCOPY(&info.remoteAddress, remoteAddress, sizeof(EmberIpv6Address));
  info.localPort = localPort;
  info.remotePort = remotePort;
  emProcessCoapMessage(NULL_BUFFER,
                       bytes,
                       bytesLength,
                       (CoapRequestHandler) & emberCoapRequestHandler,
                       &info);
}
