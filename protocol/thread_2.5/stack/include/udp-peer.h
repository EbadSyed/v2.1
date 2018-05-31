/**
 * @file udp-peer.h
 * @brief Connection-oriented UDP API
 *
 * <!--Copyright 2013 by Silicon Labs. All rights reserved.              *80*-->
 */

#ifndef __UDP_PEER_H__
#define __UDP_PEER_H__

#define NULL_UDP_HANDLE 0

typedef uint8_t EmberUdpConnectionHandle;

typedef enum {
  EMBER_UDP_CONNECTED    = 0x01,// matches EMBER_TCP_OPENED below
  EMBER_UDP_OPEN_FAILED  = 0x02,// for DTLS, matches EMBER_TCP_OPEN_FAILED
  EMBER_UDP_DISCONNECTED = 0x10, // matches EMBER_TCP_CLOSED
} UdpStatus;

/** @brief Data stored for each connection. */
typedef struct {
  EmberUdpConnectionHandle connection;
  uint16_t flags;            // for internal use only
  uint16_t internal;         // for internal use only
  uint8_t localAddress[16];
  uint8_t remoteAddress[16];
  uint16_t localPort;
  uint16_t remotePort;
} EmberUdpConnectionData;

/** @brief This function populates the EmberUdpConnectionData structure with the connections's
 * info. */
EmberStatus emberGetUdpConnectionData(EmberUdpConnectionHandle connection,
                                      EmberUdpConnectionData *data);

/** @brief This function is called by the stack when the status of a connection changes. */
typedef void (*EmberUdpConnectionStatusHandler)
  (EmberUdpConnectionData *connection, UdpStatus status);

/** @brief This function is called by the stack when a UDP packet arrives. */
typedef void (*EmberUdpConnectionReadHandler)
  (EmberUdpConnectionData *connection,
  uint8_t *packet,
  uint16_t length);

/////////////////////////////////////////////////////////////////////////////

/** @brief This function listens for incoming UDP messages. */
EmberStatus emberUdpListenLocal(uint16_t port);
bool emberUdpAmListening(uint16_t port);
void emberUdpStopListening(uint16_t port);
EmberStatus emberUdpMulticastListen(uint16_t port, const uint8_t *multicastAddress);

#define UDP_CONNECTED         0x0001
#define UDP_USING_DTLS        0x0002

EmberStatus emUdpListen(uint16_t port, const uint8_t *localAddress);

#endif // __UDP_PEER_H__
