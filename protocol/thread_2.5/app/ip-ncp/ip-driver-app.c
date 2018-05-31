// File: ip-driver-app.c
//
// Description: host-side driver application for the IP NCP.
//
// Copyright 2013 by Silicon Laboratories. All rights reserved.             *80*
//----------------------------------------------------------------

// This file is the main dispatch for the IP modem driver.  This file
// manages communication with three separate entities:
//   - serial connection to the IP modem
//   - connection to a management application
//   - connection to an IP stack, acting as IP interface
// The first two connections use the message format described in
// uart-link-protocol.h.  The connection with the IP stack send
// and receives IPv6 packets.
//
// Utilities for reading and writing to these connections are in
// host-stream.c. All of the platform-specific code should be in
// either this file or that one.
//
// The platform-independent message processing code is in ip-driver.c.

#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#ifdef __linux__
  #include <linux/if_tun.h>
#endif

#include PLATFORM_HEADER
#include "stack/core/ember-stack.h"
#include "hal/hal.h"
#include "phy/phy.h"
#include "plugin/serial/serial.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/util/serial/command-interpreter2-util.h"

#include "uart-link-protocol.h"
#include "host-stream.h"
#include "data-client.h"
#include "ip-driver.h"
#include "app/tmsp/tmsp-enum.h"
#include "hal/micro/generic/ash-v3.h"
#include "app/ip-ncp/ncp-uart-interface.h"
#include "stack/framework/event-queue.h"
#include "stack/ip/ip-address.h"

#ifdef UNIX_HOST
  #define EMBER_READ   read
  #define EMBER_WRITE  write
  #define EMBER_SELECT select
  #include "ip-driver-log.h"
  #define LOG(x) x
#else
// simulated I/O for testing
  #include "tool/simulator/child/posix-sim.h"
  #define LOG(x)
#endif

bool logEnabled = true;

//----------------------------------------------------------------
// Interface to ip-driver.c.

// connects to Host
int driverHostAppMgmtFd = -1;
// connects to mgmt port of commission-proxy-app
int driverCommProxyAppMgmtFd = -1;

Stream dataStream = { { 0 } };
Stream managementStream = { { 0 } };

// What management clients connect to.
int mgmtListenFd = -1;   // listen for managment client

// listen for commission-proxy-app mgmt/data messages
int commProxyAppMgmtListenFd = -1;
int commProxyAppDataListenFd = -1;

// Data messages from the IP stack are forwarded to the NCP.

#define IPV6_DESTINATION_ADDRESS_INDEX  24

void txBufferFullHandler(const uint8_t *packet,
                         uint16_t packetLength,
                         uint16_t lengthRemaining)
{
  assert(false);
}

void txFailedHandler(uint8_t fd,
                     const uint8_t *packet,
                     uint16_t packetLength,
                     uint16_t lengthRemaining)
{
  assert(false);
}

// Testing hack: Messages that are meant to go out on the alarm network get
// marked using an alternate destination address before being passed to the
// host IP stack.  The IP driver app unmarks them before forwarding them to
// the NCP on the alarm network data stream.
//
// See marking code is in stack/ip/host/unix-udp-wrapper.c.

static bool unmarkLegacyDestination(uint8_t *address)
{
  uint8_t before = address[1];  // for fixing the checksum
  if (address[0] == 0xFE
      && address[1] == 0x90) {
    address[1] = 0x80;
  } else if (address[0] == 0xFF
             && (address[1] == 0x0A)) {
    address[1] = 0x02;
  } else {
    return false;
  }

  // The UDP header is after the source and destination addresses (16
  // bytes each), and the checksum is the sixth and seventh bytes of
  // the UDP header.  Only UDP packets have marked addresses.
  //
  // Our checksum code is based on packets buffers, so we just tweak
  // the value here rather than recompute it from scratch.
  uint8_t *checksumLoc = address + 38;
  uint16_t checksum = emberFetchHighLowInt16u(checksumLoc);
  uint16_t newChecksum = checksum - (before - address[1]);
  // The checksum is a circular uint16, so if we carried in we need
  // to subtract the carry as the low bit.
  if (checksum < newChecksum) {
    newChecksum -= 1;
  }
  emberStoreHighLowInt16u(checksumLoc, newChecksum);
  return true;
}

// This receives packets from the Thread interface on the host IP stack
// and forwards them down to the NCP.

void dataHandler(const uint8_t *packet,
                 SerialLinkMessageType type,
                 uint16_t length)
{
  if (unmarkLegacyDestination((uint8_t *) packet
                              + IPV6_DESTINATION_ADDRESS_INDEX)) {
    // Need to tweak the checksum as well.
    type = UART_LINK_TYPE_ALT_DATA;
  }

  LOG(ipDriverLogEvent(LOG_IP_STACK_TO_NCP, packet, length, type); )
  emLogBytesLine(IP_MODEM, "DATA app->ncp ", packet, length);

  ipModemWrite(driverNcpFd, type, packet, length);
}

// Messages from the management client are forwarded to either the NCP, commission-proxy-app, or host.

void managementHandler(SerialLinkMessageType type,
                       const uint8_t *message,
                       uint16_t length)
{
  LOG(ipDriverLogEvent(LOG_APP_TO_NCP, message, length, type); )
  const uint8_t * body = message + 1;
  uint16_t identifier = emberFetchHighLowInt16u(body);
  if (type == UART_LINK_TYPE_COMMISSIONER_DATA) {
    // Pick off the commissioner data type first.
    ipModemWrite(driverNcpFd, type, message, length);
  } else if (driverCommProxyAppMgmtFd != -1
             // If host is doing DTLS, then the joiner passphrase
             // needs to be stored on commission-proxy-app
             // (The commissioner passphrase needs to be sent down to the NCP
             // first, for the PSKc TLV, and so is handled separately)
             && identifier == EMBER_SET_JOIN_KEY_COMMAND_IDENTIFIER) {
    emLogBytesLine(IP_MODEM, "MGMT app->comm-proxy-app:", message, length);
    // forwarded to commission-proxy-app
    LOG(ipDriverLogEvent(LOG_DRIVER_TO_COMM_PROXY_APP, message, length, type); )
    ipModemWrite(driverCommProxyAppMgmtFd, type, message, length);
  } else if (driverHostAppMgmtFd != -1
             && identifier == CB_SET_JOIN_KEY_COMMAND_IDENTIFIER) {
    // commission-proxy-app sends any related callbacks back.
    emLogBytesLine(IP_MODEM, "MGMT comm-proxy-app->app:", message, length);
    // forwarded to the host
    LOG(ipDriverLogEvent(LOG_DRIVER_TO_APP, message, length, type); )
    ipModemWrite(driverHostAppMgmtFd, type, message, length);
  } else {
    emLogBytesLine(IP_MODEM, "MGMT app->ncp:", message, length);
    if (identifier == EMBER_FF_WAKEUP_COMMAND_IDENTIFIER) {
      // 0xFF byte to wake the NCP
      uint8_t ffByte = 0xFF;
      assert(EMBER_WRITE(driverNcpFd, &ffByte, 1) == 1);
    } else if (identifier == EMBER_RESET_IP_DRIVER_ASH_COMMAND_IDENTIFIER
               && ncpUartUseAsh) {
      uartLinkReset();
    } else if (identifier == EMBER_RESET_NCP_GPIO_COMMAND_IDENTIFIER
               && ncpUartUseAsh) {
      resetNcp();
    } else if (identifier == EMBER_ENABLE_RESET_NCP_GPIO_COMMAND_IDENTIFIER
               && ncpUartUseAsh) {
      body += sizeof(identifier);
      enableResetNcp(*body);
    } else {
      // Everything else.
      ipModemWrite(driverNcpFd, type, message, length);
    }
  }
}

bool halHostUartTxIdle(void)
{
#ifdef UNIX_HOST
  int value;
  if (rtsCts) {
    ioctl(driverNcpFd, TIOCMGET, &value);
    return ((value & TIOCM_CTS) != 0);
  }
  // No way to check software flow control state, so assume okay to send
  return true;
#else
  // we're always idle in simulation
  return true;
#endif
}

void emNotifyTxComplete(void)
{
  // nothing to do
}

void halHostUartLinkTx(const uint8_t *data, uint16_t length)
{
  uint32_t written = EMBER_WRITE(driverNcpFd, data, length);

  if (written != length) {
    emLogLine(IP_MODEM,
              "Error: only wrote %u out of %u bytes to the driverNcpFd (%u)",
              written,
              length,
              driverNcpFd);
    return;
  }

  if (ncpUartUseAsh) {
    emAshReallyNotifyTxComplete(false);
  }
}

void emberResetSerialState(void)
{
}

//----------------------------------------------------------------
// Command interpreter

// No actual commands, but we are required to supply a default command table.
EmberCommandEntry emberCommandTable[] = {
  emberCommandEntryTerminator(),
};

//----------------------------------------------------------------
// Main

const char * const emAppName = "ip-driver";

bool initIpDriver(int argc, char **argv);

#ifdef UNIX_HOST
  #define RETURN_TYPE int
#else
  #define RETURN_TYPE void
  #define argc 0
  #define argv NULL
#endif

void ipDriverTick(void);

EventQueue emApiAppEventQueue;

RETURN_TYPE main(MAIN_FUNCTION_PARAMETERS)
{
  halInit();
  INTERRUPTS_ON();

  emInitializeEventQueue(&emApiAppEventQueue);

  openNcpReset();
  resetNcp();

  emberSerialInit(1, BAUD_115200, PARITY_NONE, 1);
  assert(initIpDriver(argc, argv));

  if (ncpUartUseAsh) {
    uartLinkReset();
  }

  emberInit();

  while (true) {
    halResetWatchdog();
    emberTick();
    emberRunEventQueue(&emApiAppEventQueue);
    ipDriverTick();
  }

#ifdef UNIX_HOST
  return 0;
#endif
}

// stubs
EmberNodeId emberGetNodeId(void)
{
  return EM_USE_LONG_ADDRESS;
}
EmberPanId emApiGetPanId(void)
{
  return EM_BROADCAST_PAN_ID;
}

#ifndef EMBER_TEST
#define USE_STUB_emMarkBufferPointer
#endif
#define USE_STUB_emIsDefaultGlobalPrefix
#define USE_STUB_emIsLocalIpAddress
#define USE_STUB_emLocalEui64
#define USE_STUB_emMacExtendedId
#define USE_STUB_emStoreDefaultGlobalPrefix
#define USE_STUB_halButtonIsr
#include "stack/ip/stubs.c"
