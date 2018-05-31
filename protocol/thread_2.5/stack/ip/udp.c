// File: udp.c
//
// Description: UDP.
//
// Copyright 2012 by Ember Corporation. All rights reserved.                *80*

#include "core/ember-stack.h"
#include "hal/hal.h"
#include "framework/ip-packet-header.h"
#include "ip-address.h"
#include "ip-header.h"
#include "dispatch.h"
#include "mle.h"
#include "dhcp.h"
#include "address-cache.h"
#include "app/coap/coap.h"
#include "app/coap/coap-stack.h"
#include "udp.h"
#include "tcp.h"
#include "tls/tls.h"
#include "tls/dtls.h"
#include "dns-client.h"
#include "tls/dtls-join.h"
#include "commission.h"

#ifdef EMBER_MBEDTLS_STACK
#include "stack/ip/tls/mbedtls/mbedtls-stack.h"
#endif

//------------------------------------------------------------------------------
// Listening ports.

#define UDP_LISTEN_PORT_TABLE_SIZE 4

static uint16_t listeningPorts[UDP_LISTEN_PORT_TABLE_SIZE];

#define cancelListen(port) (flipListen((port), 0))
#define amListening(port)  (flipListen((port), (port)))

static uint8_t flipListen(uint16_t from, uint16_t to)
{
  uint8_t i;
  for (i = 0; i < UDP_LISTEN_PORT_TABLE_SIZE; i++) {
    if (listeningPorts[i] == from) {
      listeningPorts[i] = to;
      return 1 << i;
    }
  }
  return 0;
}

EmberStatus emberUdpListenLocal(uint16_t port)
{
  if (amListening(port)) {
    return EMBER_SUCCESS;
  } else if (flipListen(0, port) == 0) {
    return EMBER_TABLE_FULL;
  }
  uint8_t localAddress[16];
  emStoreDefaultIpAddress(localAddress);
  assert(emApiUdpListen(port, localAddress) == EMBER_SUCCESS);
  return EMBER_SUCCESS;
}

bool emberUdpAmListening(uint16_t port)
{
  return amListening(port) != 0;
}

void emberUdpStopListening(uint16_t port)
{
  cancelListen(port);
}

EmberStatus emberUdpMulticastListen(uint16_t port, const uint8_t *multicastAddress)
{
  uint8_t localAddress[16];
  emStoreDefaultIpAddress(localAddress);
  assert(emApiUdpListen(port, localAddress) == EMBER_SUCCESS);
  assert(emApiUdpListen(port, multicastAddress) == EMBER_SUCCESS);
  return EMBER_SUCCESS;
}

//------------------------------------------------------------------------------
// A queue of buffers containing peers and connections.

Buffer emIpConnections = NULL_BUFFER;

// We mark this to avoid the need for a separate TLS marking function.
extern Buffer emSavedTlsSessions;

void emInitializeIpConnections(void)
{
  emIpConnections = NULL_BUFFER;
  emSavedTlsSessions = NULL_BUFFER;
}

void emMarkUdpBuffers(void)
{
  Buffer buffer = emBufferQueueHead(&emIpConnections);
  while (buffer != NULL_BUFFER) {
    UdpConnectionData *data =
      (UdpConnectionData *)(void *)emGetBufferPointer(buffer);
    if (data != NULL) {
      if (data->flags & (UDP_USING_DTLS | UDP_USING_TCP)) {
        DtlsConnection *connection = (DtlsConnection *) data;
        bufferUse((data->flags & UDP_USING_DTLS)
                  ? "DTLS"
                  : "TCP");
        emMarkBuffer(&connection->outgoingData);
        emMarkBuffer(&connection->incomingData);
        // If we amalgamate something, start the timer so emTlsStatusHandler()
        // gets called again after amalgamation.
        if (data->flags & UDP_USING_TCP) {
          // Take care not to call both emMarkAmalgamateQueue() and
          // emMarkBuffer() on the same location.  Marking a location
          // twice breaks everything.
          if (emMarkAmalgamateQueue(&connection->incomingTlsData)) {
            emStartConnectionTimerMs(0);
          }
        } else {
          emMarkBuffer(&connection->incomingTlsData);
        }
        endBufferUse();
        emMarkTlsState(&connection->tlsState);      // sets buffer use
      }
      buffer = emBufferQueueNext(&emIpConnections, buffer);
    }
  }

  bufferUse("DTLS");
  emMarkBuffer(&emIpConnections);
  bufferUse("saved TLS sessions");
  emMarkBuffer(&emSavedTlsSessions);
  bufferUse("address cache");
  emMarkBuffer(&(emAddressCache.values));
  endBufferUse();
}

//----------------------------------------------------------------
// Handles.
//
// Handles are used for peers and connections.

static uint8_t previousHandle;

static uint8_t getNextHandle(void)
{
  do {
    previousHandle += 1;
    if (previousHandle == NULL_UDP_HANDLE) {
      previousHandle += 1;
    }
  } while (emLookupHandle(previousHandle, false) != NULL);
  return previousHandle;
}

void *emLookupHandle(uint8_t handle, bool remove)
{
  Buffer finger = emBufferQueueHead(&emIpConnections);
  while (finger != NULL_BUFFER) {
    UdpConnectionData *data =
      (UdpConnectionData *)(void *)emGetBufferPointer(finger);
    if (data != NULL) {
      if (data->connection == handle) {
        if (remove) {
          emBufferQueueRemove(&emIpConnections, finger);
        }
        return data;
      }
    }
    finger = emBufferQueueNext(&emIpConnections, finger);
  }
  return NULL;
}

//------------------------------------------------------------------------------
// Connections.

#define emFindAnyConnection(remoteAddress) \
  ((emFindConnection(remoteAddress, 0xFFFF, 0xFFFF)))

// BUG: For DTLS connections this is not enough.  We also have to match on the
// session ID.  Otherwise we get the messages from different sections mixed
// up with each other.

UdpConnectionData *emFindConnection(const uint8_t *remoteAddress,
                                    uint16_t localPort,
                                    uint16_t remotePort)
{
  Buffer finger = emBufferQueueHead(&emIpConnections);
  while (finger != NULL_BUFFER) {
    UdpConnectionData *data =
      (UdpConnectionData *)(void *)emGetBufferPointer(finger);
    if (data != NULL) {
      if (MEMCOMPARE(remoteAddress, data->remoteAddress, 16) == 0
          && (localPort == 0xFFFF
              || data->localPort == localPort)
          && (remotePort == 0xFFFF
              || data->remotePort == remotePort)) {
        return data;
      }
    }
    finger = emBufferQueueNext(&emIpConnections, finger);
  }
  return NULL;
}

#ifdef EMBER_TEST
static uint16_t connectionIndex = 0;
#endif

UdpConnectionData *emAddUdpConnection
  (const uint8_t *remoteAddress,
  uint16_t localPort,
  uint16_t remotePort,
  uint16_t flags,
  uint16_t recordSize,
  EmberUdpConnectionStatusHandler statusHandler,
  EmberUdpConnectionReadHandler readHandler)
{
  UdpConnectionData *data = NULL;

  if (!(flags & UDP_USING_TCP)) {
    data = emFindConnection(remoteAddress, localPort, remotePort);
    if (data != NULL) {
      return data;
    }
  }

  Buffer buffer = emAllocateBuffer(recordSize);
  if (buffer == NULL_BUFFER) {
    return NULL;
  }
  data = (UdpConnectionData *)(void *)emGetBufferPointer(buffer);
  if (data != NULL) {
    MEMSET(data, 0, recordSize);
    data->connection = getNextHandle();
    if (data->connection == NULL_UDP_HANDLE) {
      return NULL;
    }
    MEMCOPY(data->remoteAddress, remoteAddress, 16);
    // the only legit flag arguments
    data->flags = flags & (UDP_USING_DTLS
                           | UDP_USING_TCP
                           | UDP_DTLS_JOIN
                           | UDP_DTLS_APP_CLIENT
                           | UDP_DTLS_RELAYED_JOIN);
    data->localPort = localPort;
    data->remotePort = remotePort;
    data->statusHandler = statusHandler;
    data->readHandler = readHandler;
    if (flags & (UDP_USING_DTLS | UDP_USING_TLS)) {
      Buffer tlsState =
        emAllocateTlsState(data->connection,
                           ((flags & UDP_USING_DTLS)
                            ? (TLS_USING_DTLS
                               | (flags & (TLS_CRYPTO_SUITE_FLAGS
                                           | TLS_IS_DTLS_JOIN
                                           | TLS_NATIVE_COMMISSION)))
                            : (flags & TLS_CRYPTO_SUITE_FLAGS)));

      if (tlsState == NULL_BUFFER) {
        // what to do?
        return NULL;
      }

      ((DtlsConnection *) data)->tlsState = tlsState;
      // Default to server.
      emOpenDtlsServer((DtlsConnection *) data);
    }
  }
  emBufferQueueAdd(&emIpConnections, buffer);

#ifdef EMBER_TEST
  indexOf(data) = connectionIndex++;
#endif

  return data;
}

EmberStatus emberGetUdpConnectionData(EmberUdpConnectionHandle connection,
                                      EmberUdpConnectionData *data)
{
  UdpConnectionData *found = emFindConnectionFromHandle(connection);
  if (found == NULL) {
    return EMBER_BAD_ARGUMENT;
  } else {
    MEMCOPY(data, found, sizeof(EmberUdpConnectionData));
    return EMBER_SUCCESS;
  }
}

//------------------------------------------------------------------------------
// Incoming.

#define ECHO_PORT 7  // RFC 862. Required for interop testing.

void emHandleIncomingUdp(PacketHeader header, Ipv6Header *ipHeader)
{
  switch (ipHeader->destinationPort) {
    case ECHO_PORT:
      emApiSendUdp(ipHeader->source,
                   ECHO_PORT,
                   ipHeader->sourcePort,
                   ipHeader->transportPayload,
                   ipHeader->transportPayloadLength);
      break;
    case DHCP_CLIENT_PORT:
      emDhcpClientIncomingMessageHandler(ipHeader);
      break;
    case EMBER_COAP_SECURE_PORT:
#ifdef HAVE_TLS_ECDHE_ECDSA
  #ifdef EMBER_MBEDTLS_STACK
      mbedtlsIncomingHandler(header, ipHeader, false, true);
  #else
      dtlsIncomingHandler(header, ipHeader);
  #endif
#endif
      break;
    case EMBER_COAP_PORT:
      emCoapStackIncomingMessageHandler(header,
                                        ipHeader,
                                        NULL,  // not received via DLTS
                                        NULL,
                                        NULL); // use default handler
      break;
    case THREAD_COAP_PORT:
      emCoapStackIncomingMessageHandler(header,
                                        ipHeader,
                                        NULL,  // not received via DLTS
                                        NULL,
                                        ((CoapRequestHandler)
                                         & emCoapRequestHandler));
      break;
    case DNS_PORT:
      emDnsClientIncomingMessageHandler(header, ipHeader);

    default:
      emApiCounterHandler(EMBER_COUNTER_UDP_IN, 1);
      if (emIsMulticastAddress(ipHeader->destination)) {
        emApiUdpMulticastHandler(ipHeader->destination,
                                 ipHeader->source,
                                 ipHeader->destinationPort,
                                 ipHeader->sourcePort,
                                 ipHeader->transportPayload,
                                 ipHeader->transportPayloadLength);
        return;
      }

      UdpConnectionData *connection =
        emFindConnection(ipHeader->source,
                         ipHeader->destinationPort,
                         ipHeader->sourcePort);

      if (connection == NULL) {
        emApiUdpHandler(ipHeader->destination,
                        ipHeader->source,
                        ipHeader->destinationPort,
                        ipHeader->sourcePort,
                        ipHeader->transportPayload,
                        ipHeader->transportPayloadLength);
      } else {
        assert(connection->flags & UDP_USING_DTLS);
        uint16_t ipHeaderLength = (ipHeader->transportPayload
                                   - emGetBufferPointer(header));
        uint16_t oldLength = emGetBufferLength(header);
        uint16_t newLength = oldLength - ipHeaderLength;
        uint8_t *contents = emGetBufferPointer(header);
        MEMMOVE(contents, contents + ipHeaderLength, newLength);
        emSetBufferLength(header, newLength);
        emBufferQueueAdd(&(((DtlsConnection *) connection)->incomingTlsData),
                         header);
        emDtlsStatusHandler((DtlsConnection *) connection);
      }
  }
}
