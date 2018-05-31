/*
 * File: dispatch.h
 * Description: definitions for IP packet dispatching.
 * Author(s): Matteo Paris, matteo@ember.com
 *
 * Copyright 2013 by Silicon Laboratories. All rights reserved.             *80*
 */

#ifndef DISPATCH_H
#define DISPATCH_H

// Cache of the source addresses from the packet we are currently
// processing.  Used for sending replies.
extern uint8_t *emCurrentIpSource;
extern EmberNodeId emCurrentMeshSource;

#define ALLOW_LOOPBACK     true
#define NO_LOOPBACK       false
#define NO_DELAY              0
#define USE_DEFAULT_RETRIES 255

#define emSubmitIpHeader(header, ipHeader) \
  (emReallySubmitIpHeader((header),        \
                          (ipHeader),      \
                          ALLOW_LOOPBACK,  \
                          USE_DEFAULT_RETRIES, NO_DELAY))

#define emSubmitIpHeaderNoRetries(header, ipHeader) \
  (emReallySubmitIpHeader((header), (ipHeader), ALLOW_LOOPBACK, 0, NO_DELAY))

bool emForwardPacketFromHost(PacketHeader header, Ipv6Header *ipHeader);
bool emForwardIpv6Packet(PacketHeader header, Ipv6Header *ipHeader);

// This sets the transport checksum, does any necessary loopback, and
// calls emSubmitIndirectOrRetry().
bool emReallySubmitIpHeader(PacketHeader header,
                            Ipv6Header *ipHeader,
                            bool allowLoopback,
                            uint8_t retries,
                            uint16_t delayMs);

// Used by folks that don't have an Ipv6Header to pass to
// emReallySumbitIpHeader().
bool emSubmitIndirectOrRetry(PacketHeader header,
                             uint8_t retries,
                             uint16_t delayMs,
                             bool isMulticast);

// Ditto, because those folks need to set their own checksum.
void emSetTransportChecksum(PacketHeader header, Ipv6Header *ipHeader);

void emNetworkCheckLoopbackQueue(void);

// The incoming RSSI of the message currently being processed.
extern int8_t emCurrentRssi;

// Callback for incoming beacons.
void emProcessIncomingBeacon(PacketHeader header);
void emIpIncomingBeaconHandler(PacketHeader header);

extern bool emIncomingForMeToUart;
extern bool emSecurityToUart;

void emHandleIncomingUdp(PacketHeader header, Ipv6Header *ipHeader);

void emNetworkIncomingMessageHandler(PacketHeader header);

#endif // DISPATCH_H
