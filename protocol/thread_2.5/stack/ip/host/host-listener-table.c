// File: host-listener-table.c
//
// Description: Implementation of a listener table for hosts.
//
// Copyright 2015 by Silicon Laboratories. All rights reserved.                *80*

#include "stack/core/ember-stack.h"
#include "hal/hal.h"
#include "stack/ip/ip-address.h"
#include "stack/ip/ip-header.h"
#include "stack/ip/host/host-listener-table.h"

HostListener listeners[EMBER_HOST_LISTENER_TABLE_SIZE];

void emberInitializeListeners(void)
{
  MEMSET(&listeners, 0, sizeof(listeners));
  uint8_t i;
  for (i = 0; i < EMBER_HOST_LISTENER_TABLE_SIZE; i++) {
    listeners[i].socket = INVALID_SOCKET;
  }
}

void emberCloseListeners(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_HOST_LISTENER_TABLE_SIZE; i++) {
    if (listeners[i].socket != INVALID_SOCKET) {
      emberCloseListener(&listeners[i]);
    }
    listeners[i].socket = INVALID_SOCKET;
    listeners[i].port = 0;
    MEMSET(listeners[i].sourceAddress, 0, 16);
  }
}

HostListener *emberFindListener(uint16_t port, const uint8_t *sourceAddress)
{
  uint8_t i;
  for (i = 0; i < EMBER_HOST_LISTENER_TABLE_SIZE; i++) {
    if (port == 0) {
      if (listeners[i].socket == INVALID_SOCKET) {
        return &listeners[i];
      }
    } else {
      if (listeners[i].socket != INVALID_SOCKET
          && listeners[i].port == port
          && (sourceAddress == NULL
              || (MEMCOMPARE(listeners[i].sourceAddress, sourceAddress, 16)
                  == 0))) {
        return &listeners[i];
      }
    }
  }
  return NULL;
}

HostListener *emberAddListener(uint16_t port,
                               const uint8_t *sourceAddress,
                               int type,
                               int protocol)
{
  HostListener *listener = emberFindListener(0, NULL);
  if (listener != NULL) {
    listener->port = port;
    MEMCOPY(listener->sourceAddress, sourceAddress, 16);
    listener->socket = emberBindListener(listener->port,
                                         listener->sourceAddress,
                                         type,
                                         protocol);

    if (listener->socket == INVALID_SOCKET) {
      return NULL;
    }

    listener->type = type;
    listener->protocol = protocol;
  }
  return listener;
}

const HostListener *emberGetHostListeners(void)
{
  return listeners;
}
