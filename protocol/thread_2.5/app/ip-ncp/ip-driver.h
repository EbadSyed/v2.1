// File: ip-driver.c
//
// Description: host-side driver for the IP NCP.
//
// Copyright 2013 by Silicon Laboratories. All rights reserved.             *80*

#ifndef IP_DRIVER_H
#define IP_DRIVER_H

// Handle messages received from the NCP.

void ncpDriverMessageHandler(SerialLinkMessageType type,
                             const uint8_t *message,
                             uint16_t length);

// Send messages to the NCP or management client.  This callback is
// implemented in ip-driver-app.c.

extern bool logEnabled;
extern int driverNcpFd;
extern int driverHostAppMgmtFd;
extern int driverCommProxyAppMgmtFd;
extern int driverCommProxyAppDataFd;
extern int driverDataFd;

uint16_t ipModemWrite(int fd,
                      SerialLinkMessageType type,
                      const uint8_t *buffer,
                      uint16_t length);

void dataHandler(const uint8_t *packet,
                 SerialLinkMessageType type,
                 uint16_t length);

extern Stream dataStream;
extern Stream managementStream;
extern bool ipDriverUseAsh;

void managementHandler(SerialLinkMessageType type,
                       const uint8_t *message,
                       uint16_t length);

void ncpMessageHandler(SerialLinkMessageType type,
                       const uint8_t *message,
                       uint16_t length);

IpModemReadStatus readIpModemAshInput
  (int fd, IpModemMessageHandler ncpMessageHandler);

void txBufferFullHandler(const uint8_t *packet,
                         uint16_t packetLength,
                         uint16_t written);

void txFailedHandler(uint8_t fd,
                     const uint8_t *packet,
                     uint16_t packetLength,
                     uint16_t written);

#endif // IP_DRIVER_H
