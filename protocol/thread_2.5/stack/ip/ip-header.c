/*
 * File: ip-header.c
 * Description: IPv6 packet header utility code
 * Author(s): Richard Kelsey, Matteo Paris
 *
 * Copyright 2013 Silicon Laboratories, Inc.                                *80*
 */

#include "core/ember-stack.h"
#include "framework/ip-packet-header.h"
#include "framework/debug.h"
#include "routing/util/retry.h"
#include "zigbee/zigbee-ip.h"
#include "zigbee/join.h"

#include "6lowpan-header.h"
#include "dispatch.h"
#include "fragment.h"
#include "multicast.h"
#include "ip-address.h"

#include "ip-header.h"

#ifdef EMBER_TEST
#include "core/parcel.h"
#endif

//----------------------------------------------------------------
// Forward declarations
static PacketHeader makeOptionTunnelHeader(Ipv6Header *ipHeader,
                                           Ipv6Header *outerHeader,
                                           uint16_t nextHop,
                                           uint16_t payloadBufferLength);
static PacketHeader makeMplOptionTunnelHeader(Ipv6Header *ipHeader,
                                              uint16_t payloadBufferLength);

//----------------------------------------------------------------

// This would go in ip-address.c except that it is about routing
// and not IP addresses in general (that use of emNodeType) and
// isn't used on hosts.

// Multicast scopes
//  1  interface local - legal for loopback, but not over the air
//  2  link local
//  3  realm local
//  4  admin local
//  5  site local
//  8  organization local
//  E  global
#define ALLOWED_SCOPES (BIT(2) | BIT(3) | BIT(4) | BIT(5) | BIT(8) | BIT(0xE))

bool emIsMulticastForMe(const uint8_t *address)
{
  if (address[0] != 0xFF) {
    return false;       // not a multicast address
  }

  if (emIsFf33MulticastAddress(address)
      || emIsFf32MulticastAddress(address)) {
    return true;
  }

  uint8_t flags = address[1] & 0xF0;
  uint8_t scope = address[1] & 0x0F;

  if (flags != 0) {
    return false;       // we have no support for flagged multicast addresses
  }

  if (!(BIT(scope) & ALLOWED_SCOPES)) {
    return false;       // invalid scope
  }

  switch (HIGH_LOW_TO_INT(address[14], address[15])) {
    case 0x0002:
      // Strangely, the Thread spec calls FF03::2 the "realm-local all-routers"
      // address, but also says it should be deliverd to all FTDs, which
      // can include devices that are not routers.  Oh well.
      return (emAmFullThreadDevice() || emNodeType == EMBER_LURKER);

    default:
      // Deliver them all and let the application sort it out.
      return true;
  }
}

// Thread uses a two-byte seed.
#define MPL_OPTION_SIZE (IPV6_HBH_MPL_SEED_INDEX + 2)

static void storeMulticastOption(uint8_t *finger, uint8_t nextHeader)
{
  finger[IPV6_EXT_NEXT_HEADER_INDEX] = nextHeader;
  finger[IPV6_EXT_LENGTH_INDEX] = 0;
  finger[IPV6_HBH_OPTION_TYPE_INDEX] = EXTENSION_HEADER_OPTION_MPL;
  finger[IPV6_HBH_OPTION_LENGTH_INDEX] = 4;
  finger[IPV6_HBH_MPL_FLAGS_INDEX] = IPV6_HBH_MPL_FLAGS_VALUE;
  emStoreMulticastSequence(finger + IPV6_HBH_MPL_SEQUENCE_INDEX);
  emberStoreHighLowInt16u(finger + IPV6_HBH_MPL_SEED_INDEX,
                          (emUseLongIdOnly()
                           ? 0xFFFF
                           : emberGetNodeId()));
}

// Putting these here reduces the number of stub copies needed.

uint8_t emMaxFragmentLength = MAX_FRAGMENT;
uint8_t emMaxFinalFragmentLength = MAX_FINAL_FRAGMENT;

//------------------------------------------------------------------------------
// Making IP headers

PacketHeader emMakeIpHeader(Ipv6Header *ipHeader,
                            uint8_t tag,
                            uint8_t options,
                            const uint8_t *destination,
                            uint8_t hopLimit,
                            uint16_t payloadBufferLength)
{
  PacketHeader header;
  uint8_t *contents;
  EmberNodeId nextHop = 0;
  uint16_t paddedExtensionHeaderLength = 0;
  bool needsMplHeader = false;
  bool needsMplTunnel = false;
  bool longMacSource;
  bool longMacDest = false;
  bool isLegacy = ((options & IP_HEADER_IS_LEGACY)
                   || emNodeType == EMBER_LURKER);

  bool gleanMacDestFromIpDest =
    (((options & IP_HEADER_ONE_HOP)
      || emIsFe8Address(destination)
      || isLegacy)
     && !emIsMulticastAddress(destination));

  // Determine the MAC destination (nextHop) in certain cases.
  // For lurkers the dest is either gleaned or is multicast (first two cases).

  if (gleanMacDestFromIpDest) {
    longMacDest = !emGleanShortIdFromInterfaceId(destination + 8, &nextHop);
  } else if (emIsFf01MulticastAddress(destination)
             || emIsFf02MulticastAddress(destination)
             || emIsFf32MulticastAddress(destination)) {
    // Leave off the MPL header for link local multicasts. This code has to
    // coordinate with the code that decides whether or not to drop incoming
    // multicast packets that lack a MPL header (in multicast.c).
    nextHop = BROADCAST_ADDRESS;
  } else if (emIsMulticastAddress(destination)) {
    nextHop = (emAmEndDevice()
               ? emParentId
               : BROADCAST_ADDRESS);
    needsMplHeader = true;
  } else {
    nextHop = 0xFFFE;       // real address will be discovered later
  }

  if (needsMplHeader) {
    // Extension headers have to be padded out to a multiple of 8 bytes.
    paddedExtensionHeaderLength = emRoundUpToEight(MPL_OPTION_SIZE);
  }

  if (!emIsMemoryZero(ipHeader->source, 16)) {
    // Use ipHeader->source as already set by caller.
  } else if ((options & IP_HEADER_LL64_SOURCE) || isLegacy) {
    emStoreLongFe8Address(emMacExtendedId, ipHeader->source);
  } else if (options & IP_HEADER_GP64_SOURCE) {
    emStoreLocalMl64(ipHeader->source);
  } else if (options & IP_HEADER_GP16_SOURCE) {
    emStoreDefaultIpAddress(ipHeader->source);
  } else if (options & IP_HEADER_LINK_LOCAL_SOURCE) {
    emStoreLongFe8Address(emMacExtendedId, ipHeader->source);
  } else if (!emStoreIpSourceAddress(ipHeader->source, destination)) {
    emLogLine(DROP, "no source address");
    return NULL_BUFFER;
  }

  longMacSource = (!emNetworkIsUp()
                   || emUseLongIdOnly()
                   || emIsLl64(ipHeader->source)
                   || isLegacy);

  // simPrintStartLine();
  // fprintf(stderr, "emMakeIpHeader() %d ", longMacSource);
  // simPrintIpAddress(ipHeader->source);
  // fprintf(stderr, " -> ");
  // simPrintIpAddress(destination);
  // fprintf(stderr, "]\n");

  MEMMOVE(ipHeader->destination, destination, 16);
  ipHeader->hopLimit = hopLimit;
  ipHeader->ipPayloadLength = (paddedExtensionHeaderLength
                               + ipHeader->transportHeaderLength
                               + ipHeader->transportPayloadLength);

  header = emMakeDataHeader(nextHop == BROADCAST_ADDRESS,
                            longMacSource,
                            longMacDest,
                            NULL,
                            (INTERNAL_IP_HEADER_SIZE
                             + ipHeader->ipPayloadLength
                             - payloadBufferLength));
  if (header == NULL_BUFFER) {
    emLogLine(DROP, "memory");
    return NULL_BUFFER;
  }
  if (isLegacy) {
    emHeaderSetMacInfoField(header,
                            (emHeaderMacInfoField(header)
                             | MAC_INFO_LEGACY_KEY));
  }
  emHeaderSetTag(header, tag);
  contents = emMacPayloadPointer(header);
  contents[0] = LOWPAN_IPV6_HEADER_BYTE;
  MEMSET(contents + 1, 0, INTERNAL_IP_OVERHEAD - 1);

  ipHeader->nextHeader = ipHeader->transportProtocol;
  ipHeader->ipPayload = contents + INTERNAL_IP_HEADER_SIZE;

  // Add an extension header if present.
  if (needsMplHeader) {
    storeMulticastOption(ipHeader->ipPayload, ipHeader->nextHeader);
    ipHeader->nextHeader = IPV6_NEXT_HEADER_HOP_BY_HOP;
  }

  // Set the MAC destination now that we know the next hop.
  if (!longMacDest) {
    emSetMacShortDestination(header, nextHop);
  } else if (gleanMacDestFromIpDest) {
    uint8_t eui64[8];
    emInterfaceIdToLongId(destination + 8, eui64);
    emSetMacLongDestination(header, eui64);
  } else {
    uint8_t *longId = emLookupLongId(nextHop);
    if (longId == NULL) {
      emLogLine(DROP, "can't store mac dest");
      return NULL_BUFFER;
    } else {
      emSetMacLongDestination(header, longId);
    }
  }

  emStoreIpv6Header(ipHeader, header);

  {
    uint8_t *inBuffer = ipHeader->ipPayload + paddedExtensionHeaderLength;
    MEMMOVE(inBuffer,
            ipHeader->transportHeader,
            ipHeader->transportHeaderLength);
    ipHeader->transportHeader = inBuffer;

    inBuffer += ipHeader->transportHeaderLength;
    if (ipHeader->transportPayload != NULL) {
      MEMMOVE(inBuffer,
              ipHeader->transportPayload,
              ipHeader->transportPayloadLength - payloadBufferLength);
    }
    ipHeader->transportPayload = inBuffer;
  }

  if (ipHeader->transportProtocol == IPV6_NEXT_HEADER_IPV6) {
    // For encapsulated headers, the ipHeader must point to the inner header.
    // This is used by the application to add transport payload, and
    // upon submission to calculate the checksum.
    emReallyFetchIpv6Header(ipHeader->transportHeader, ipHeader);
    if (ipHeader->transportProtocol == IPV6_NEXT_HEADER_UDP) {
      emSetIncomingTransportHeaderLength(ipHeader, UDP_HEADER_SIZE);
    } else if (ipHeader->transportProtocol == IPV6_NEXT_HEADER_ICMPV6) {
      emSetIncomingTransportHeaderLength(ipHeader, ICMP_HEADER_SIZE);
    }
  } else if (needsMplTunnel) {
    return makeMplOptionTunnelHeader(ipHeader, payloadBufferLength);
  }

  return header;
}

//----------------------------------------------------------------
// IPv6 Tunnel

// This function encapsulates an IP packet in a one-hop tunnel
// with RPL or MPL option.  It is very wasteful because it copies the
// entire contents to a new buffer rather than chaining.
// The supplied Ipv6Header continues to point to the inner
// ipv6 header, not the new outer one.

static PacketHeader makeOptionTunnelHeader(Ipv6Header *ipHeader,
                                           Ipv6Header *outerHeader,
                                           uint16_t nextHop,
                                           uint16_t payloadBufferLength)
{
  PacketHeader header;
  uint8_t *contents;
  uint16_t payloadLength = ipHeader->ipPayloadLength + IPV6_HEADER_SIZE;
  uint8_t *payloadPointer = ipHeader->ipPayload - IPV6_HEADER_SIZE;
  uint8_t transportPayloadIndex = ipHeader->transportPayload - payloadPointer;
  uint8_t transportHeaderIndex = ipHeader->transportHeader - payloadPointer;

  // Allocate the new buffer
  header = emMakeDataHeader(nextHop == 0xFFFF, // isBroadcast
                            false, // longSource
                            false, // longDest
                            NULL,
                            INTERNAL_IP_HEADER_SIZE
                            + 8
                            + payloadLength
                            - payloadBufferLength);

  if (header == NULL_BUFFER) {
    return NULL_BUFFER;
  }
  contents = emMacPayloadPointer(header);
  contents[0] = LOWPAN_IPV6_HEADER_BYTE;
  MEMSET(contents + 1, 0, INTERNAL_IP_OVERHEAD - 1);

  // Populate the Ipv6Header for the outer tunnel header.
  // This will only be used to write the header to the buffer.
  outerHeader->nextHeader = IPV6_NEXT_HEADER_HOP_BY_HOP;
  outerHeader->ipPayload = contents + INTERNAL_IP_HEADER_SIZE;
  outerHeader->ipPayloadLength = payloadLength + 8;

  // The passed ipHeader will continue to point to the inner header.
  // This is used by the application to add transport payload, and
  // upon submission to calculate the checksum.
  ipHeader->ipPayload = outerHeader->ipPayload + 8 + IPV6_HEADER_SIZE;
  ipHeader->transportHeader = outerHeader->ipPayload + 8 + transportHeaderIndex;
  ipHeader->transportPayload = outerHeader->ipPayload + 8 + transportPayloadIndex;

  // Copy the inner IPv6 packet to the buffer
  MEMMOVE(outerHeader->ipPayload + 8,
          payloadPointer,
          payloadLength - payloadBufferLength);

  // Write the outer IPv6 header to the buffer
  emStoreIpv6Header(outerHeader, header);

  emSetMacShortDestination(header, nextHop);
  return header;
}

static PacketHeader makeMplOptionTunnelHeader(Ipv6Header *ipHeader,
                                              uint16_t payloadBufferLength)
{
  Ipv6Header outerHeader;
  MEMSET(&outerHeader, 0, sizeof(Ipv6Header));
  outerHeader.hopLimit = ipHeader->hopLimit;
  emStoreDefaultIpAddress(outerHeader.source);
  MEMCOPY(outerHeader.destination,
          emFf03AllNodesMulticastAddress.contents,
          16);
  PacketHeader header = makeOptionTunnelHeader(ipHeader,
                                               &outerHeader,
                                               BROADCAST_ADDRESS,
                                               payloadBufferLength);
  if (header == NULL_BUFFER) {
    return NULL_BUFFER;
  }
  storeMulticastOption(outerHeader.ipPayload, IPV6_NEXT_HEADER_IPV6);
  return header;
}
