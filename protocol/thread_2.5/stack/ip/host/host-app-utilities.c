// File: host-app-utilities.c
//
// Description: Utilities for a host.
//
// Copyright 2015 by Silicon Laboratories. All rights reserved.                *80*

// For usleep(3) in glibc.
#define _BSD_SOURCE

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stddef.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>

#ifdef __linux__
  #include <sys/sysctl.h>
  #include <linux/sysctl.h>
  #include <sys/syscall.h>
#endif

#include PLATFORM_HEADER
#include "stack/core/ember-stack.h"
#include "hal/hal.h"
#include "phy/phy.h"
#include "stack/ip/ip-header.h"
#include "stack/ip/ip-address.h"
#include "stack/framework/ip-packet-header.h"
#include "stack/mac/802.15.4/mac.h"
#include "stack/ip/host/host-address-table.h"
#include "stack/ip/host/host-listener-table.h"
#include "app/ip-ncp/binary-management.h"
#include "app/tmsp/tmsp-enum.h"
#include "stack/framework/event-queue.h"
#include "stack/ip/tls/psk-table.h"

extern void mbedtlsInitializeConnections(void);
extern void mbedtlsMarkBuffers(void);

//------------------------------------------------------------------------------
// Tasks.

EventQueue emStackEventQueue;

//------------------------------------------------------------------------------
// Framework.

void emberInit(void)
{
  emInitializeBuffers();
  emInitializeEventQueue(&emStackEventQueue);
  emberInitializeListeners();
  emberInitializeHostAddressTable();
  emberEnableHostDtlsClient(EMBER_SECURITY_TO_HOST);
#ifdef EMBER_MBEDTLS_STACK
  mbedtlsInitializeConnections();
#endif
  emSendBinaryManagementCommand(EMBER_INIT_HOST_COMMAND_IDENTIFIER, "");
}

void markEventQueue(void)
{
  emberMarkEventQueue(&emStackEventQueue);
}

void markPskTable(void)
{
  emMarkBuffer(&emPskTable);
}

static BufferMarker const markers[] = {
  emberMarkApplicationBuffersHandler,
  markEventQueue,
  emCoapMarkBuffers,
#ifdef EMBER_MBEDTLS_STACK
  mbedtlsMarkBuffers,
#endif
  markPskTable,
  NULL
};

void emberTick(void)
{
  emberListenerTick();
  emberRunEventQueue(&emStackEventQueue);
  emReclaimUnusedBuffers(markers);
  #ifndef UNIX_HOST_SIM
  usleep(100);
  #endif
}

EmberNodeId emberGetNodeId(void)
{
  return EM_USE_LONG_ADDRESS;
}
bool emIsNdNewNodeId(EmberNodeId shortId)
{
  return false;
}
#ifdef UNIX_HOST
void exitCommand(void)
{
  exit(0);
}
#endif
