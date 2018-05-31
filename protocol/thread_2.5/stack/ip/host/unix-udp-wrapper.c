// File: unix-udp-wrapper.c
//
// Description: Simple UDP API implemented using POSIX sockets.
//
// Copyright 2015 by Silicon Laboratories. All rights reserved.                *80*

#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>

#include "stack/core/ember-stack.h"
#include "stack/ip/ip-address.h"
#include "stack/ip/ip-header.h"
#include "stack/ip/host/unix-interface.h"
#include "stack/ip/tls/native-test-util.h"
#include "stack/ip/host/host-listener-table.h"

EmberStatus emberUdpListen(uint16_t port, const uint8_t *sourceAddress)
{
  if (emIsUnspecifiedAddress(sourceAddress)) {
    return EMBER_ERR_FATAL;
  }
  if (emberFindListener(port, sourceAddress) != NULL) {
    return EMBER_SUCCESS;
  }

  HostListener *listener = emberAddListener(port,
                                            sourceAddress,
                                            SOCK_DGRAM, 0); // UDP

  if (listener == NULL) {
    return EMBER_TABLE_FULL;
  }

  if (listener->socket == INVALID_SOCKET) {
    return EMBER_ERR_FATAL;
  }

  int interfaceIndex = if_nametoindex(emUnixInterface);
  struct ipv6_mreq mreq6 = { { { { 0 } } } };
  MEMCOPY(&mreq6.ipv6mr_multiaddr.s6_addr, sourceAddress, 16);
  mreq6.ipv6mr_interface = interfaceIndex;
  int mcastTTL = 10;
  int loopBack = 1;

  if (setsockopt(listener->socket,
                 IPPROTO_IPV6,
                 IPV6_MULTICAST_IF,
                 &interfaceIndex,
                 sizeof(interfaceIndex))
      < 0) {
    perror("setsockopt:: IPV6_MULTICAST_IF:: ");
    return EMBER_ERR_FATAL;
  }

  if (setsockopt(listener->socket,
                 IPPROTO_IPV6,
                 IPV6_MULTICAST_LOOP,
                 &loopBack,
                 sizeof(loopBack))
      < 0) {
    perror("setsockopt:: IPV6_MULTICAST_LOOP:: ");
    return EMBER_ERR_FATAL;
  }

  if (setsockopt(listener->socket,
                 IPPROTO_IPV6,
                 IPV6_MULTICAST_HOPS,
                 &mcastTTL,
                 sizeof(mcastTTL))
      < 0) {
    perror("setsockopt:: IPV6_MULTICAST_HOPS::  ");
    return EMBER_ERR_FATAL;
  }

  if (emIsMulticastAddress(sourceAddress)) {
    if (setsockopt(listener->socket,
                   IPPROTO_IPV6,
                   IPV6_JOIN_GROUP,
                   &mreq6,
                   sizeof(mreq6))
        < 0) {
      perror("setsockopt:: IPV6_JOIN_GROUP:: ");
      return EMBER_ERR_FATAL;
    }
  }

  return EMBER_SUCCESS;
}

// Testing hack: Messages that are meant to go out on the alarm network get
// marked using an alternate destination address before being passed to the
// host IP stack.  The IP driver app unmarks them before forwarding them to
// the NCP on the alarm network data stream.
//
// The unmarking code is in app/ip-ncp/ip-driver-app.c.
//
// This should really be done via separate IP interfaces on the host, but
// that would be too muck work for just a few tests.

static void markLegacyDestination(uint8_t *address)
{
  if (address[0] == 0xFE
      && address[1] == 0x80) {
    address[1] = 0x90;
  } else if (address[0] == 0xFF
             && (address[1] == 0x02)) {
    address[1] = 0x0A;
  } else {
    assert(false);      // no way to mark the destination
  }
}

EmberStatus emSendUdp(const uint8_t *destination,
                      uint16_t sourcePort,
                      uint16_t destinationPort,
                      uint8_t *payload,
                      uint16_t payloadLength,
                      uint8_t options)
{
  uint8_t source[16];
  uint8_t destBuf[16];
  const uint8_t *altDestination;

  if (options & IP_HEADER_IS_LEGACY) {
    emStoreLongFe8Address(emMacExtendedId, source);
    MEMCOPY(destBuf, destination, 16);
    markLegacyDestination(destBuf);
    altDestination = destBuf;
  } else {
    if (!emStoreIpSourceAddress(source, destination)) {
      return EMBER_BAD_ARGUMENT;
    }
    altDestination = destination;
  }

  HostListener *listener = emberFindListener(sourcePort, source);
  if (listener != NULL) {
    struct sockaddr_in6 outSock = { 0 };
    outSock.sin6_family = AF_INET6;
    outSock.sin6_port = htons(destinationPort);
    MEMCOPY(outSock.sin6_addr.s6_addr, altDestination, 16);
    int interfaceIndex = if_nametoindex(emUnixInterface);
    outSock.sin6_scope_id = interfaceIndex;

    nativeWrite(listener->socket,
                payload,
                payloadLength,
                (struct sockaddr *)&outSock,
                sizeof(outSock));
    emberCounterHandler(EMBER_COUNTER_UDP_OUT, 1);

    return EMBER_SUCCESS;
  } else {
    char sourceString[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, source, sourceString, sizeof(sourceString));
    fprintf(stderr, "No UDP listener on %s port %hu\n", sourceString, sourcePort);
    return EMBER_INVALID_CALL;
  }
}

EmberStatus emberSendUdp(const uint8_t *destination,
                         uint16_t sourcePort,
                         uint16_t destinationPort,
                         uint8_t *payload,
                         uint16_t payloadLength)
{
  return emSendUdp(destination,
                   sourcePort,
                   destinationPort,
                   payload,
                   payloadLength,
                   0);
}
