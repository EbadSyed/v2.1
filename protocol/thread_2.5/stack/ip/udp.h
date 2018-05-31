// File: udp.h
//
// Description: UDP.
//
// Copyright 2012 by Ember Corporation. All rights reserved.                *80*

// Internal version of the UDP connection data.

#ifndef __UDP_H__
#define __UDP_H__

#define UDP_USING_TCP          0x04
#define UDP_USING_TLS          0x08
#define UDP_DTLS_JOIN          TLS_IS_DTLS_JOIN     // why reuse the TLS value?
#define UDP_DTLS_RELAYED_JOIN  0x10
#define UDP_DELETE             0x20
#define UDP_DTLS_APP_CLIENT    0x40

// Used to throttle DTLS resends.
#define UDP_DTLS_NEED_RESEND 0x0400
#define UDP_CLOSE_TIMEOUT    0x0800
#define UDP_DTLS_JOIN_KEK    0x1000

// Some internal read handlers get the buffer (if any) as well.
typedef void (*UdpConnectionReadHandler)
  (EmberUdpConnectionData *connection,
  uint8_t *packet,
  uint16_t length,
  Buffer buffer);

typedef struct {
  EmberUdpConnectionHandle connection;
  uint16_t flags;
  uint16_t ticksRemaining;        // time until next whatever; stack internal
  uint8_t localAddress[16];
  uint8_t remoteAddress[16];
  uint16_t localPort;
  uint16_t remotePort;

  // These fields are not in the external struct.
  uint8_t events;                 // bitmask of things to tell the application
  EmberUdpConnectionStatusHandler statusHandler;
  EmberUdpConnectionReadHandler readHandler;

#ifdef EMBER_TEST
  uint16_t index;         // helps keep track of connections when debugging
  #define indexOf(x) (((UdpConnectionData *) x)->index)
#else
  #define indexOf(x) 0
#endif
} UdpConnectionData;

extern Buffer emIpConnections;

void emMarkUdpBuffers(void);
void emHandleIncomingApplicationUdp(PacketHeader header, Ipv6Header *ipHeader);

void *emLookupHandle(uint8_t handle, bool remove);
#define emFindConnectionFromHandle(handle) \
  ((UdpConnectionData *) emLookupHandle(handle, false))

#define isDtlsConnection(connection) \
  (((connection)->udpData.flags & UDP_USING_DTLS) != 0)

UdpConnectionData *emFindConnection(const uint8_t *remoteAddress,
                                    uint16_t localPort,
                                    uint16_t remotePort);

UdpConnectionData *emAddUdpConnection(
  const uint8_t *remoteAddress,
  uint16_t localPort,
  uint16_t remotePort,
  uint16_t flags,
  uint16_t recordSize,
  EmberUdpConnectionStatusHandler statusHandler,
  EmberUdpConnectionReadHandler readHandler);

typedef EmberUdpConnectionHandle (*UdpAcceptHandler)
  (const uint8_t *remoteAddress,
  uint16_t localPort,
  uint16_t remotePort,
  uint8_t flags,
  const uint8_t *packet,
  uint16_t length);

#endif
