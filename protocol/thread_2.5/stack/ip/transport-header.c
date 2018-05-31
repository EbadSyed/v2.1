// File: transport-header.c
//
// Description: IP transport (UDP, TCP, ICMP) header utility code
//
// Copyright 2011 by Ember Corporation. All rights reserved.                *80*

#include "core/ember-stack.h"
#include "ip-header.h"
#include "ip-address.h"
#include "app/coap/coap.h"

void emReallyFetchIpv6Header(uint8_t *source, Ipv6Header *destination)
{
  MEMSET(destination, 0, sizeof(Ipv6Header));
  destination->trafficClass = (((source[0] & 0x0F) << 4)
                               | (source[1] >> 4));
  destination->flowLabel = (emberFetchHighLowInt32u(source)
                            & 0x000FFFFF);
  source += 4;
  destination->ipPayloadLength = emberFetchHighLowInt16u(source);
  source += 2;
  destination->nextHeader = *source++;
  destination->hopLimit = *source++;
  MEMCOPY(destination->source, source, 16);
  source += 16;
  MEMCOPY(destination->destination, source, 16);
  destination->ipPayload = source + 16;
  destination->transportHeader = destination->ipPayload;
  destination->transportProtocol = destination->nextHeader;
}

void emReallyStoreIpv6Header(Ipv6Header *source, uint8_t *destination)
{
  emberStoreHighLowInt16u(destination,
                          (0x6000
                           | (source->trafficClass << 4)
                           | (source->flowLabel >> 16)));
  destination += 2;
  emberStoreHighLowInt16u(destination,
                          source->flowLabel & 0xFFFF);
  destination += 2;
  emberStoreHighLowInt16u(destination, source->ipPayloadLength);
  destination += 2;
  *destination++ = source->nextHeader;
  *destination++ = source->hopLimit;
  MEMMOVE(destination, source->source, 16);
  destination += 16;
  MEMMOVE(destination, source->destination, 16);
}

bool emSetIncomingTransportHeaderLength(Ipv6Header *ipHeader,
                                        uint8_t headerLength)
{
  int16_t transportPayloadLength = (ipHeader->ipPayloadLength
                                    - (ipHeader->transportHeader
                                       - ipHeader->ipPayload)
                                    - headerLength);
  if (transportPayloadLength < 0) {
    return false;
  }
  ipHeader->transportHeaderLength = headerLength;
  ipHeader->transportPayload = ipHeader->transportHeader + headerLength;
  ipHeader->transportPayloadLength = (uint16_t) transportPayloadLength;
  return true;
}

//----------------------------------------------------------------
//
// IP checksum used by UDP, TCP, and ICMP.  This just adds up the
// 16-bit chunks, adding any carry out back in at the low bit.
// We add up the low bytes and high bytes separately and then
// rotate the carried bits afterwards.  The input is processed
// in chunks to avoid carrying out of the 16-bit 'low' and 'high'
// accumulators.
//
// Changing the order of aligned uint16_t's does not change the
// result.

static uint16_t checksum(uint8_t *bytes, uint16_t length, uint16_t index, uint16_t start)
{
  uint16_t low = BYTE_0(start);
  uint16_t high = BYTE_1(start);

//  fprintf(stderr, "[checksum([");
//  if (0 < length) {
//    uint16_t i;
//    fprintf(stderr, "%02X", bytes[0]);
//    for (i = 1; i < length; i++)
//      fprintf(stderr, " %02X", bytes[i]);
//  }

  if (index & 1) {
    low += *bytes++;
    length -= 1;
  }

  do {  // We need to run at least once in order to run the carry code
        // in the case where the original 'length' value was one.

    uint16_t todo = length;
    if (250 < todo) {
      todo = 250;
    }
    length -= todo;

    for (; 1 < todo; todo -= 2) {
      high += *bytes++;
      low += *bytes++;
    }

    if (todo == 1) {
      high += *bytes++;
    }

    do {
      low += BYTE_1(high);      // add high carries to low
      high = BYTE_0(high);      // remove high carries
      high += BYTE_1(low);      // add low carries to high
      low = BYTE_0(low);        // remove low carries
    } while (255 < high);       // check for more carrying
  } while (0 < length);

//  fprintf(stderr, "] %04X -> %04X]\n",
//          start,
//          HIGH_LOW_TO_INT(high, low));

  return HIGH_LOW_TO_INT(high, low);
}

#ifdef EMBER_TEST
// Scripted tests that do not want to bother with checksums should set
// this to false.
bool enableIpChecksums = true;
#endif

// The 'upper-layer checksum' from RFC2460(IPv6).  Besides the
// payload, this adds in the source, destination, upper-layer packet
// length, and the upper-layer protocol.
//
// Returns the negation of the checksum.  For outgoing messages,
// this value is written into the checksum field.  For incoming
// messages, the value is compared against 0x0000.

uint16_t emTransportChecksum(PacketHeader header,
                             Ipv6Header *ipHeader)
{
  uint16_t upperLayerLength = (ipHeader->ipPayloadLength
                               - (ipHeader->transportHeader
                                  - ipHeader->ipPayload));

  // emStartLogLine(TEMP);
  // emLogBytes(TEMP, "checksum source:[", ipHeader->source, 16);
  // emLogBytes(TEMP, "] dest:[", ipHeader->destination, 16);
  // emEndLogLine(TEMP);

  uint16_t result =
    checksum(ipHeader->source,
             sizeof(EmberIpv6Address),
             0,
             checksum(ipHeader->destination,
                      sizeof(EmberIpv6Address),
                      0,
                      upperLayerLength + ipHeader->transportProtocol));
  uint16_t remaining = upperLayerLength;
  uint16_t chunk = (emGetBufferPointer(header)
                    + emGetBufferLength(header)
                    - ipHeader->transportHeader);
  uint16_t index = 0;
  result = checksum(ipHeader->transportHeader, chunk, index, result);
  remaining -= chunk;
  index += chunk;
  while (0 < remaining) {
    header = emGetPayloadLink(header);

    if (header == NULL_BUFFER) {
      // failure
      return 0xFFFF;
    }

    chunk = emGetBufferLength(header);
    if (remaining < chunk) {
      chunk = remaining;
    }
    result = checksum(emGetBufferPointer(header), chunk, index, result);
    remaining -= chunk;
    index += chunk;
  }

#ifdef EMBER_TEST
  if (!enableIpChecksums) {
    return 0x0000;
  }
#endif

  return ~result;
}

void emSetTransportChecksum(PacketHeader header, Ipv6Header *ipHeader)
{
  uint8_t checksumIndex = 0;
  switch (ipHeader->transportProtocol) {
    case IPV6_NEXT_HEADER_ICMPV6:
      checksumIndex = ICMP_CHECKSUM_INDEX;
      break;
    case IPV6_NEXT_HEADER_UDP:
      checksumIndex = UDP_CHECKSUM_INDEX;
      break;
    case IPV6_NEXT_HEADER_TCP:
      checksumIndex = TCP_CHECKSUM_INDEX;
      break;
    default:
      return;
  }

  uint8_t *checksumLoc = ipHeader->transportHeader + checksumIndex;
  uint16_t checksum;
  emberStoreHighLowInt16u(checksumLoc, 0);
  checksum = emTransportChecksum(header, ipHeader);

  // The UDP checksum is optional; a sum of 0x0000 indicates the
  // absence of a checksum, so we need to replace 0x0000 with 0xFFFF.
  if (ipHeader->transportProtocol == IPV6_NEXT_HEADER_UDP
      && checksum == 0x0000) {
    checksum = 0xFFFF;
  }
  emberStoreHighLowInt16u(checksumLoc, checksum);
}

//----------------------------------------------------------------
// UDP

PacketHeader emMakeTaggedUdpHeader(Ipv6Header *ipHeader,
                                   uint8_t tag,
                                   uint8_t options,
                                   const uint8_t *source,
                                   const uint8_t *destination,
                                   uint8_t hopLimit,
                                   uint16_t sourcePort,
                                   uint16_t destinationPort,
                                   uint8_t *payload,
                                   uint16_t payloadLength,
                                   uint16_t payloadBufferLength)
{
  uint8_t udpHeader[UDP_HEADER_SIZE];

  // From RFC 2460, "5. Packet Size Issues"
  // Value 1232 is 1280 - 40 (IP) - 8 (UDP).
  if (payloadLength > 1232) {
    //emLogLine(DROP, "length %u", payloadLength);
    //return NULL_BUFFER;
  }

  MEMSET(ipHeader, 0, sizeof(Ipv6Header));

  ipHeader->transportProtocol = IPV6_NEXT_HEADER_UDP;
  ipHeader->sourcePort = sourcePort;
  ipHeader->destinationPort = destinationPort;
  ipHeader->transportHeader = udpHeader;
  ipHeader->transportHeaderLength = UDP_HEADER_SIZE;
  ipHeader->transportPayload = payload;
  ipHeader->transportPayloadLength = payloadLength + payloadBufferLength;

  if (source != NULL) {
    MEMCOPY(ipHeader->source, source, 16);
  }

  emStoreUdpHeader(ipHeader, 0);

  return emMakeIpHeader(ipHeader,
                        tag,
                        options,
                        destination,
                        hopLimit,
                        payloadBufferLength);
}

void emStoreUdpHeader(Ipv6Header *ipHeader, uint16_t myChecksum)
{
  emberStoreHighLowInt16u(ipHeader->transportHeader, ipHeader->sourcePort);
  emberStoreHighLowInt16u(ipHeader->transportHeader + 2,
                          ipHeader->destinationPort);
  emberStoreHighLowInt16u(ipHeader->transportHeader + 4,
                          ipHeader->transportPayloadLength + UDP_HEADER_SIZE);
  emberStoreHighLowInt16u(ipHeader->transportHeader + 6, myChecksum);
}

bool emFetchUdpHeader(Ipv6Header *ipHeader)
{
  uint16_t length;
  ipHeader->sourcePort = emberFetchHighLowInt16u(ipHeader->transportHeader);
  ipHeader->destinationPort = emberFetchHighLowInt16u(ipHeader->transportHeader
                                                      + 2);
  length = emberFetchHighLowInt16u(ipHeader->transportHeader + 4);

  if (!emSetIncomingTransportHeaderLength(ipHeader, UDP_HEADER_SIZE)
      || (ipHeader->ipPayloadLength
          - (ipHeader->transportHeader
             - ipHeader->ipPayload)
          < length)) {
    return false;         // are we supposed to send an ICMP report?
  }
  return true;
}

//----------------------------------------------------------------
// ICMP

PacketHeader emMakeTaggedIcmpHeader(Ipv6Header *ipHeader,
                                    uint8_t tag,
                                    uint8_t options,
                                    uint8_t *source,  // rarely used
                                    const uint8_t *destination,
                                    uint8_t hopLimit,
                                    uint8_t type,
                                    uint8_t code,
                                    uint8_t *payload,
                                    uint16_t payloadLength)
{
  uint8_t icmpHeader[ICMP_HEADER_SIZE];

  icmpHeader[0] = type;
  icmpHeader[1] = code;
  icmpHeader[2] = 0xFF; // two bytes of uninitialized checksum
  icmpHeader[3] = 0xFF;

  MEMSET(ipHeader, 0, sizeof(Ipv6Header));

  ipHeader->transportProtocol = IPV6_NEXT_HEADER_ICMPV6;
  ipHeader->icmpType = type;
  ipHeader->icmpCode = code;
  ipHeader->transportHeader = icmpHeader;
  ipHeader->transportHeaderLength = ICMP_HEADER_SIZE;
  ipHeader->transportPayload = payload;
  ipHeader->transportPayloadLength = payloadLength;

  if (source != NULL) {
    MEMCOPY(ipHeader->source, source, 16);
  }

  return emMakeIpHeader(ipHeader, tag, options, destination, hopLimit, 0);
}

//----------------------------------------------------------------
// CoAP
//
// The stack uses CoAP as a transport.

// Or use 5683, the default CoAP port.
uint16_t emStackCoapPort = THREAD_COAP_PORT;

void emInitStackCoapMessage(CoapMessage *message,
                            const EmberIpv6Address *destination,
                            uint8_t *payload,
                            uint16_t payloadLength)
{
  MEMSET(message, 0, sizeof(CoapMessage));
  if (destination == NULL) {
    message->type = COAP_TYPE_CONFIRMABLE;
  } else {
    message->type = (emIsMulticastAddress(destination->bytes)
                     ? COAP_TYPE_NON_CONFIRMABLE
                     : COAP_TYPE_CONFIRMABLE);
    MEMCOPY(message->remoteAddress.bytes, destination->bytes, 16);
  }
  message->code = EMBER_COAP_CODE_POST;
  message->localPort = emStackCoapPort;
  message->remotePort = emStackCoapPort;
  message->payload = payload;
  message->payloadLength = payloadLength;
}

void emInitStackMl16CoapMessage(CoapMessage *message,
                                EmberNodeId destination,
                                uint8_t *payload,
                                uint16_t payloadLength)
{
  EmberIpv6Address dest;
  emStoreGp16(destination, dest.bytes);
  emInitStackCoapMessage(message, &dest, payload, payloadLength);
}
