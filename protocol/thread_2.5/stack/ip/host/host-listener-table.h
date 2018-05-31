// File: host-listener-table.h
//
// Description: Implementation of a listener table for hosts.
//
// Copyright 2015 by Silicon Laboratories. All rights reserved.                *80*

#ifndef HOST_LISTENER_TABLE_H
#define HOST_LISTENER_TABLE_H

typedef struct {
  int socket;
  uint16_t port;
  uint8_t sourceAddress[16];
  int type;
  int protocol;
} HostListener;

#ifdef RTOS
#define EMBER_HOST_LISTENER_TABLE_SIZE 20
#else
#define EMBER_HOST_LISTENER_TABLE_SIZE 64
#endif

extern HostListener listeners[];

#define INVALID_SOCKET -1
#define UL_ICMP_PORT 9080
#define LL_ICMP_PORT 9081

void emberInitializeListeners(void);
HostListener *emberFindListener(uint16_t port, const uint8_t *sourceAddress);
HostListener *emberAddListener(uint16_t port,
                               const uint8_t *sourceAddress,
                               int type,
                               int protocol);
int emberBindListener(uint16_t port,
                      const uint8_t *sourceAddress,
                      int type,
                      int protocol);
void emberListenerTick(void);
void emberCloseListeners(void);
int emberCloseListener(const HostListener *listener);
void emberCheckIncomingListener(const HostListener *listener);
const HostListener *emberGetHostListeners(void);

#endif // HOST_LISTENER_TABLE_H
