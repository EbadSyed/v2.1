// File: unix-address.c
//
// Description: Unix Host address functionality.
//
// Copyright 2013 by Ember Corporation. All rights reserved.                *80*

// For getaddrinfo(3) in glibc, getline, strdup
#define _GNU_SOURCE

#ifdef WINSOCK
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <netinet/in.h>
  #include <termios.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/types.h>
  #include <errno.h>
  #define INVALID_SOCKET -1
#endif

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <string.h>

#include "stack/core/ember-stack.h"
#include "hal/hal.h"
#include "plugin/serial/serial.h"

EmberIpv6Address emMyIpAddress = { { 0 } };
extern const char *emUnixInterface;

bool emPopulateSockAddr(struct sockaddr_storage *storage,
                        const uint8_t *hostString,
                        int family,
                        int streamType,
                        int protocol)
{
  struct addrinfo *results;
  struct addrinfo *resultIterator;
  struct addrinfo hints = { 0 };
  hints.ai_family = AF_UNSPEC;

  int error = getaddrinfo((const char *)hostString, NULL, &hints, &results);

  if (error != 0) {
    emberSerialPrintfLine(APP_SERIAL, "error during getaddrinfo");
    return false;
  }

  for (resultIterator = results;
       resultIterator != NULL;
       resultIterator = resultIterator->ai_next) {
    if (resultIterator->ai_family == family
        && resultIterator->ai_socktype == streamType
        && resultIterator->ai_protocol == protocol) {
      MEMCOPY(storage,
              resultIterator->ai_addr,
              resultIterator->ai_addrlen);
      freeaddrinfo(results);
      return true;
    }
  }

  freeaddrinfo(results);
  return false;
}

uint8_t emGetIpv6Addresses(struct ifaddrs *addresses[],
                           uint8_t addressesCount,
                           struct ifaddrs **fullIfAddrs)
{
  struct ifaddrs *interfaceList = NULL;
  struct ifaddrs *interface = NULL;
  uint8_t foundCount = 0;

  if (getifaddrs(&interfaceList) < 0) {
    perror("getifaddrs");
    exit(1);
  }

  for (interface = interfaceList;
       interface != NULL;
       interface = interface->ifa_next) {
    if (foundCount >= addressesCount) {
      break;
    }

    if (interface->ifa_addr != NULL
        && interface->ifa_addr->sa_family == AF_INET6) {
      addresses[foundCount] = interface;
      foundCount++;
    }
  }

  *fullIfAddrs = interfaceList;
  return foundCount;
}

//
// emChooseInterface -- choose an interface for subsequent calls to
// emberUdpListen() or emTcpListen().
//
// When interfaceChoice is 0xFF or prefixChoice is NULL,
// they are ignored. If neither interfaceChoice nor prefixChoice
// are given, an interface menu is printed so the user can decide which
// interface to choose.
//
bool emChooseInterface(uint8_t interfaceChoice,
                       const uint8_t *prefixChoice,
                       bool produceOutput)
{
  struct ifaddrs *addresses[10] = { 0 };
  struct in6_addr prefix;
  struct in6_addr *ipv6Addresses[10] = { 0 };
  struct ifaddrs *fullIfAddrs = NULL;

  memset(&prefix, 0, sizeof(struct in6_addr));
  if (prefixChoice != NULL) {
    if (inet_pton(AF_INET6, (char *)prefixChoice, (void*)&prefix) != 1) {
      emberSerialPrintfLine(APP_SERIAL,
                            "Invalid IP address: %s "
                            "(can't change it to network format)",
                            prefixChoice);
      return false;
    }
  }

  int foundCount = emGetIpv6Addresses(addresses, 10, &fullIfAddrs);
  int i;

  if (foundCount == 0) {
    fprintf(stderr, "No global IPv6 interfaces found!\n");
    exit(1);
  }

  if (interfaceChoice == 0xFF) {
    emberSerialPrintfLine(APP_SERIAL, "Choose an interface from below:");
  }

  //
  // print out the interfaces
  //
  for (i = 0; i < foundCount; i++) {
    char addressString[INET6_ADDRSTRLEN] = { 0 };
    ipv6Addresses[i] =
      &((struct sockaddr_in6*)addresses[i]->ifa_addr)->sin6_addr;

    if (inet_ntop(AF_INET6,
                  ipv6Addresses[i],
                  addressString,
                  sizeof(addressString))
        == NULL) {
      perror("inet_ntop");
      exit(1);
    }

    if (MEMCOMPARE(&prefix, ipv6Addresses[i], 8) == 0
        || MEMCOMPARE(&prefix, ipv6Addresses[i], 16) == 0) {
      interfaceChoice = i;
    }

    if ((interfaceChoice == 0xFF
         || interfaceChoice == i)
        && produceOutput) {
      emberSerialPrintfLine(APP_SERIAL,
                            "[%u]: %s on interface %s",
                            i,
                            addressString,
                            addresses[i]->ifa_name);
    }
  }

  if (!produceOutput) {
    assert(interfaceChoice != 0xFF);
  } else {
    while (interfaceChoice >= foundCount) {
      emberSerialPrintf(APP_SERIAL, "Choice: ");
      char *line = malloc(1000);
      size_t lineSize = sizeof(line);
      ssize_t bytesRead = getline(&line, &lineSize, stdin);
      interfaceChoice = (bytesRead > 0
                         ? atoi(line)
                         : 0xFF);
      free(line);
    }
  }

  // free later?
  emUnixInterface = strdup(addresses[interfaceChoice]->ifa_name);
  MEMCOPY(emMyIpAddress.bytes, ipv6Addresses[interfaceChoice], 16);
  freeifaddrs(fullIfAddrs);
  return true;
}

bool emSetIpv6Address(EmberIpv6Address *target, const uint8_t *remoteAddress)
{
  struct sockaddr_storage storage = { 0 };

  if (!emPopulateSockAddr(&storage,
                          remoteAddress,
                          AF_INET6,
                          SOCK_DGRAM,
                          IPPROTO_UDP)) {
    return false;
  } else {
    assert(storage.ss_family == AF_INET6);
    const struct sockaddr_in6 *address = (const struct sockaddr_in6*)&storage;
    MEMCOPY(target->bytes, address->sin6_addr.s6_addr, 16);
    return true;
  }
}
