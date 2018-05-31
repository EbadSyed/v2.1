/**
 * @file tcp.h
 * @brief Ember TCP API.
 *
 * <!--Copyright 2013 by Silicon Labs. All rights reserved.              *80*-->
 */

#ifndef TCP_H
#define TCP_H

/** @brief Status values for the emberTcpStatusHandler() callback. */
enum {
  EMBER_TCP_OPENED       = 0x01,
  EMBER_TCP_OPEN_FAILED  = 0x02,
  EMBER_TCP_ACCEPTED     = 0x04,
  EMBER_TCP_TX_COMPLETE  = 0x08,
  EMBER_TCP_CLOSED       = 0x10,
  EMBER_TCP_REMOTE_CLOSE = 0x20,
  EMBER_TCP_REMOTE_RESET = 0x40,
  EMBER_TCP_DONE         = 0x80
};

enum {
  NO_TLS,
  FRESH_TLS,
  RESUME_TLS, // valid for connect, not valid for listen
  PRESHARED_TLS
};

extern const char * const emTlsModeStrings[];

// If the value in *portLoc is zero, it will be set to an unused port number.
// Return status:
//  EMBER_SUCCESS: success, *portLoc contains the listened-to port
//  EMBER_BAD_ARGUMENT: the supplied port number is alread in use
//  EMBER_TABLE_FULL: no room at the inn

EmberStatus emTcpListen(uint16_t *portLoc, uint8_t tlsMode);
#define emberTcpListen(portLoc)    (emTcpListen((portLoc), NO_TLS))
#define emberTcpListenTls(portLoc) (emTcpListen((portLoc), FRESH_TLS))

EmberStatus emberTcpStopListening(uint16_t port);

bool emberTcpAmListening(uint16_t port);

// Return an fd, or 0xFF if there are no free connections.

uint8_t emTcpConnect(const EmberIpv6Address *remoteIpAddress,
                     uint16_t remotePort,
                     uint8_t tlsMode);
#define emberTcpConnect(address, port) (emTcpConnect((address), (port), NO_TLS))
#define emberTcpTlsConnect(address, port, resume) \
  (emTcpConnect((address), (port), ((resume) ? RESUME_TLS : FRESH_TLS)))

// Called when a connection is opened to serverPort.
void emberTcpAcceptHandler(uint16_t serverPort, uint8_t fd);

EmberStatus emberTcpClose(uint8_t fd);

void emberTcpReset(uint8_t fd);

void emberTcpReadHandler(uint8_t fd, uint8_t *buffer, uint16_t count);

EmberStatus emberTcpWrite(uint8_t fd, const uint8_t *buffer, uint16_t count);

void emberTcpStatusHandler(uint8_t fd, uint8_t status);

uint16_t emberTcpGetPort(uint8_t fd);

bool emberTcpPortInUse(uint16_t port);

bool emberTcpFdInUse(uint8_t fd);

bool emberTcpConnectionIsOpen(uint8_t fd);

EmberIpv6Address *emberTcpRemoteIpAddress(uint8_t fd);

const char *emberTcpStatusName(uint8_t status);

#endif // TCP_H
