// File: print-utilities.h
//
// Description: Common print functions.
//
// Copyright 2013 by Ember Corporation. All rights reserved.                *80*

#ifndef __PRINT_UTILITIES_H__
#define __PRINT_UTILITIES_H__

void printNetworkId(const uint8_t* networkId);
void printEui(const EmberEui64 *eui);
void printIpHeader(Ipv6Header *ipHeader);
void printUdpHeader(Ipv6Header *ipHeader);
void printIcmpHeader(Ipv6Header *ipHeader);
void printPayload(const uint8_t *payload, uint16_t length);
void printIpPacket(Ipv6Header *ipHeader);
void printDefaultIp(void);
void printAllAddresses(void);
void printAsciiPrintable(uint8_t *bytes, uint16_t length);
void printUdpConnection(EmberUdpConnectionData *connectionData);
void printUdpPacket(EmberUdpConnectionData *connectionData,
                    uint8_t *packet,
                    uint16_t packetLength);
void printUdpPacketVerbose(const uint8_t *destinationIpv6Address,
                           const uint8_t *sourceIpv6Address,
                           uint16_t localPort,
                           uint16_t remotePort,
                           const uint8_t *packet,
                           uint16_t packetLength);
void printStatus(EmberStatus status, const char *name, bool printTime);
void printFormStatus(EmberStatus status);
void printJoinStatus(EmberStatus status);
void printExtendedPanId(uint8_t *extendedPanId);
void printNetworkState(void);
void printResetInfo(void);
void printResetCause(EmberResetCause cause);
const uint8_t *networkStatusString(uint8_t networkStatus);
const uint8_t *nodeTypeString(uint8_t nodeType);
const uint8_t *resetCauseString(uint8_t resetCause);
const uint8_t *wakeupReasonString(uint8_t wakeupReason);
const uint8_t *appWakeupStateString(uint8_t wakeupState);
const uint8_t *wakeupStateString(uint8_t wakeupState);

void printHexWords(const uint8_t *contents, uint8_t wordCount);
void printIpAddress(const uint8_t *address);
void printLurkerNetworkIpAddress(const uint8_t *prefix);
void ipAddressToString(const uint8_t *address, uint8_t *buffer, uint8_t bufferLength);

void printRipEntry(uint8_t index, const EmberRipEntry *entry);
void printWakeupState(uint8_t wakeupState, uint16_t wakeupSequenceNumber);
void printTlvs(const uint8_t *finger, const uint8_t *end);

#endif
