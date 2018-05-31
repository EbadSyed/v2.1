// File: unix-icmp-wrapper.c
//
// Description: Simple ICMP API implemented using POSIX sockets.
//
// Copyright 2015 by Silicon Laboratories. All rights reserved.                *80*

#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/icmp6.h>
#include <fcntl.h>
#include <time.h>

#include "stack/core/ember-stack.h"
#include "stack/ip/ip-address.h"
#include "stack/ip/ip-header.h"
#include "stack/ip/host/unix-interface.h"
#include "stack/ip/tls/native-test-util.h"
#include "stack/ip/host/host-listener-table.h"
#include "app/ip-ncp/binary-management.h"
#include "app/tmsp/tmsp-enum.h"

static uint16_t prefixToPort(const uint8_t *prefix)
{
  // For some reason using the same port on different addresses isn't working.
  // We also need to avoid the ports used by thread-app for UDP, so we subtract
  // 0x100.
  return emberFetchHighLowInt16u(prefix) - 0x100;
}

EmberStatus emberIcmpListen(const uint8_t *sourceAddress)
{
  uint16_t icmpPort = prefixToPort(sourceAddress);

  if (emIsUnspecifiedAddress(sourceAddress)) {
    return EMBER_ERR_FATAL;
  }
  if (emberFindListener(icmpPort, sourceAddress) != NULL) {
    return EMBER_SUCCESS;
  }

  HostListener *listener = emberAddListener(icmpPort,
                                            sourceAddress,
                                            SOCK_RAW, IPPROTO_ICMPV6); // ICMP

  if (listener == NULL) {
    return EMBER_TABLE_FULL;
  }

  if (listener->socket == INVALID_SOCKET) {
    return EMBER_ERR_FATAL;
  }

  int interfaceIndex = if_nametoindex(emUnixInterface);
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

  return EMBER_SUCCESS;
}

bool emIpPing(uint8_t *destination,
              uint16_t idValue,
              uint16_t sequence,
              uint16_t length,
              uint8_t hopLimit,
              uint8_t options)  // ignored on the host
{
  uint8_t source[16];
  uint16_t icmpSourcePort;

  // Get the source address and port to use with this destination
  if (options & IP_HEADER_IS_LEGACY) {
    emStoreLongFe8Address(emMacExtendedId, source);
  } else if (!emStoreIpSourceAddress(source, destination)) {
    return false;
  }
  icmpSourcePort = prefixToPort(source);

  HostListener *listener = emberFindListener(icmpSourcePort, source);
  if (listener != NULL) {
    struct sockaddr_in6 addr = { 0 };
    struct icmp6_hdr icmp_hdr = { 0 };

    addr.sin6_family = AF_INET6;
    MEMCOPY(addr.sin6_addr.s6_addr, destination, 16);
    int interfaceIndex = if_nametoindex(emUnixInterface);
    addr.sin6_scope_id = interfaceIndex;

    icmp_hdr.icmp6_type = ICMP_ECHO_REQUEST;
    icmp_hdr.icmp6_id = htons(idValue);
    icmp_hdr.icmp6_seq = htons(sequence);

    unsigned char data[2048];
    MEMCOPY(data, &icmp_hdr, sizeof(icmp_hdr));
    uint8_t i;
    for (i = 0; i < length; i++) {
      data[sizeof(icmp_hdr) + i] = i;
    }

    if (sendto(listener->socket,
               data,
               sizeof(icmp_hdr) + length,
               0,
               (struct sockaddr*)&addr,
               sizeof(addr)) > 0) {
      return true;
    } else {
      perror("ICMP sendto failed");
      return false;
    }
  } else {
    char sourceString[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, source, sourceString, sizeof(sourceString));
    fprintf(stderr, "No ICMP listener on %s\n", sourceString);
    return false;
  }
}

bool emberIpPing(uint8_t *destination,
                 uint16_t idValue,
                 uint16_t sequence,
                 uint16_t length,
                 uint8_t hopLimit)
{
  return emIpPing(destination, idValue, sequence, length, hopLimit, 0);
}
