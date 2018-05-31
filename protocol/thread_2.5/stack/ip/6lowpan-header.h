/*
 * File: 6lowpan-header.h
 * Description: 6LowPAN-specific header definitions and function declarations.
 * Author(s): Richard Kelsey
 *
 * Copyright 2009 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef IP_6LOWPAN_HEADER_H
#define IP_6LOWPAN_HEADER_H

//-------------------------------------------------------------
// 6LowPAN frame format
//
// The 6LowPAN frame format is described in RFC4944 and RFC6282.
//
// All 6LowPAN headers start with a dispatch byte.  There are currently
// four types of 6LowPAN headers:
//  - encapsulated IPv6 datagram
//  - HC1 (Header Compression 1?)
//  - mesh routing
//  - fragmentation
//  - broadcast
//
// An encapsulated IPv6 datagram is a complete, uncompressed IPv6 packet
// preceded by a dispatch byte.  These contain no other 6LowPAN headers.
//
// All other packets contain a HC1 header followed by a payload, with
// other headers optionally preceding the HC1 header in the order
// given above.  A packet may have a fragmentation header or a broadcast
// header, but not both.
//
// IP formats are all big-endian.

#define LOWPAN_NALP_HEADER_TYPE         0x00
#define LOWPAN_DISPATCH_HEADER_TYPE     0x40
#define LOWPAN_MESH_HEADER_TYPE         0x80
#define LOWPAN_OTHER_HEADER_TYPE        0xC0
#define LOWPAN_HEADER_TYPE_MASK         0xC0

// encapsulated, uncompressed header
#define LOWPAN_IPV6_HEADER_BYTE         0x41

// RFC4944 HC1 compression (probably won't be used)
#define LOWPAN_HC1_HEADER_BYTE          0x42

// broadcast compressed header
#define LOWPAN_BC0_HEADER_BYTE          0x60

// additional dispatch byte follows (redefined to this in the hc-06 draft)
#define LOWPAN_ESC_HEADER_BYTE          0x40

// Mesh addressing header
//                           1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |1 0|V|F|HopsLft| originator address, final address
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// V = 0 if originator is 64 bits, 1 if 16 bits
// F = 0 if final is 64 bits, 1 if 16 bits
//
// HopsLft = 0xF means that there is an following byte with the actual
// hops-left value.  This allows values 15...256.
//
// We will only use 16-bit addresses in mesh headers and not use HopsLft
// values greater than 14.

#define LOWPAN_MESH_HOP_COUNT_MASK 0x0F
#define LOWPAN_MESH_HIGH_NIBBLE    0xB0         // V = 1, F = 1
#define LOWPAN_MESH_HEADER_SIZE    5
#define LOWPAN_MESH_MAX_HOPS       10

typedef struct {
  uint8_t size;         // size of the header
  uint8_t hopLimit;
  EmberNodeId source;
  EmberNodeId destination;
} MeshHeader;

bool emReadMeshHeader(PacketHeader header, MeshHeader *mesh);
void emDecrementMeshHeaderHopLimit(PacketHeader header);

// fragments are distinguished by the first five bits
// The next eleven bits are the size of the complete datagram.
#define LOWPAN_FRAGMENT_MASK            0xF8
#define LOWPAN_FIRST_FRAGMENT           0xC0
#define LOWPAN_LATER_FRAGMENT           0xE0

// 011 in the first three (big-endian) bits indicates an IPHC header.
#define LOWPAN_IPHC_TYPE_MASK           0xE000
#define LOWPAN_IPHC_TYPE_VALUE          0x6000

// IPHC header is two bytes, plus uncompressed fields.

//   0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// | 0 | 1 | 1 |  TF   |NH | HLIM  |CID|SAC|  SAM  | M |DAC|  DAM  |
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

#define LOWPAN_IPHC_NEXT_HEADER_FLAG              0x0400
#define LOWPAN_IPHC_HOP_LIMIT_MASK                0x0300
#define LOWPAN_IPHC_CONTEXT_ID_FLAG               0x0080
#define LOWPAN_IPHC_SOURCE_COMPRESSION_FLAG       0x0040
#define LOWPAN_IPHC_SOURCE_ADDRESS_MODE           0x0030
#define LOWPAN_IPHC_MULTICAST_COMPRESSION_FLAG    0x0008
#define LOWPAN_IPHC_DESTINATION_COMPRESSION_FLAG  0x0004
#define LOWPAN_IPHC_DESTINATION_ADDRESS_MODE      0x0003

// Traffic Class field has two bits of ECN (Explict Congestion Notification,
// RFC 3168) and 6 bits of diffserv (RFC2474).
// Flow Labels are 20 bits
// 0x0000       // Traffic Class + 4-bit pad + Flow Label (4 bytes)
// 0x0800       // ECN           + 2-bit pad + Flow Label (3 bytes)
// 0x1000       // Traffic Class                          (1 byte)
// 0x1800       // Version, Traffic Class, and Flow Label are compressed

#define LOWPAN_IPHC_TRAFFIC_CLASS_FLOW_LABEL_MASK 0x1800
#define LOWPAN_IPHC_TRAFFIC_AND_FLOW              0x0000
#define LOWPAN_IPHC_ECN_AND_FLOW                  0x0800
#define LOWPAN_IPHC_TRAFFIC_AND_NO_FLOW           0x1000
#define LOWPAN_IPHC_NO_TRAFFIC_AND_NO_FLOW        0x1800

// Hop Limit
#define LOWPAN_IPHC_HOP_LIMIT_IN_LINE   0x0000
#define LOWPAN_IPHC_HOP_LIMIT_1         0x0100
#define LOWPAN_IPHC_HOP_LIMIT_64        0x0200
#define LOWPAN_IPHC_HOP_LIMIT_255       0x0300

#define LOWPAN_IPHC_16_BYTE_SOURCE      0x0000
#define LOWPAN_IPHC_8_BYTE_SOURCE       0x0010
#define LOWPAN_IPHC_2_BYTE_SOURCE       0x0020
#define LOWPAN_IPHC_0_BYTE_SOURCE       0x0030

#define LOWPAN_IPHC_16_BYTE_DESTINATION 0x0000
#define LOWPAN_IPHC_8_BYTE_DESTINATION  0x0001
#define LOWPAN_IPHC_2_BYTE_DESTINATION  0x0002
#define LOWPAN_IPHC_0_BYTE_DESTINATION  0x0003

#define LOWPAN_IPHC_16_BYTE_MULTICAST   0x0000
#define LOWPAN_IPHC_6_BYTE_MULTICAST    0x0001
#define LOWPAN_IPHC_4_BYTE_MULTICAST    0x0002
#define LOWPAN_IPHC_1_BYTE_MULTICAST    0x0003
#define LOWPAN_IPHC_CONTEXT_MULTICAST   0x0004

// Figure 6: IPv6 Extension Header Encoding
//
//                     +---+---+---+---+---+---+---+---+
//                     | 1 | 1 | 1 | 0 |    EID    |NH |
//                     +---+---+---+---+---+---+---+---+

#define LOWPAN_EXT_HEADER_MASK 0xF0
#define LOWPAN_EXT_HEADER_VALUE 0xE0

#define LOWPAN_EXT_HEADER_ID_MASK 0x0E
#define LOWPAN_HOP_BY_HOP_EID          0x0
#define LOWPAN_ROUTING_EID             0x2
#define LOWPAN_FRAGMENT_EID            0x4   // not used by us
#define LOWPAN_DESTINATION_OPTIONS_EID 0x6   // not used by us
#define LOWPAN_MOBILITY_EID            0x8   // not used by us
//#define LOWPAN_???_EID               0xA   // reserved
//#define LOWPAN_???_EID               0xD   // reserved
#define LOWPAN_IPV6_EID                0xE

#define LOWPAN_EXT_HEADER_NH_MASK 0x01

//----------------------------------------------------------------

// 4.3.3.  UDP LOWPAN_NHC Format
//
//                       0   1   2   3   4   5   6   7
//                     +---+---+---+---+---+---+---+---+
//                     | 1 | 1 | 1 | 1 | 0 | C |   P   |
//                     +---+---+---+---+---+---+---+---+
//
//   C: Checksum:
//      0: All 16 bits of Checksum are carried in-line.
//      1: All 16 bits of Checksum are elided.  The Checksum is recovered
//         by recomputing it on the 6LoWPAN termination point.
//
//   P: Ports:
//      00:  All 16 bits for both Source Port and Destination Port are
//         carried in-line.
//      01:  All 16 bits for Source Port are carried in-line.  First 8
//         bits of Destination Port is 0xF0 and elided.  The remaining 8
//         bits of Destination Port are carried in-line.
//      10:  First 8 bits of Source Port are 0xF0 and elided.  The
//         remaining 8 bits of Source Port are carried in-line.  All 16
//         bits for Destination Port are carried in-line.
//      11:  First 12 bits of both Source Port and Destination Port are
//         0xF0B and elided.  The remaining 4 bits for each are carried
//         in-line.
//
//   Fields carried in-line (in part or in whole) appear in the same order
//   as they do in the UDP header format [RFC0768].  The UDP Length field
//   MUST always be elided and is inferred from lower layers using the
//   6LoWPAN Fragmentation header or the IEEE 802.15.4 header.

#define LOWPAN_UDP_HEADER_MASK            0xF8
#define LOWPAN_UDP_HEADER_VALUE           0xF0

#define LOWPAN_UDP_CHECKSUM_ELIDED        0x04

#define LOWPAN_UDP_PORT_MASK              0x03
#define LOWPAN_UDP_FULL_PORTS             0x00
#define LOWPAN_UDP_SHORT_DESTINATION_PORT 0x01
#define LOWPAN_UDP_SHORT_SOURCE_PORT      0x02
#define LOWPAN_UDP_SHORT_PORTS            0x03

#define LOWPAN_ONE_BYTE_PORT_PREFIX       0xF000
#define LOWPAN_ONE_NIBBLE_PORT_PREFIX     0xF0B0

//-------------------------------------------------------------
// Internal Header
//
//  [LOWPAN_IPV6_HEADER_BYTE (1)] [fragment data (5)] [IPV6 header (20)] ...
//
// Having the space for the fragment data always be included allows the
// fragment-or-not decision to be delayed until the packet is actually
// being sent.
//
// The one exception to this is incoming fragmented packets, where the
// header is left as an initial fragment, which is one byte shorter,
// until the entire packet is reassembled in to a single buffer.
//
// The fragment data consists of an offset byte (scaled by 8; the
// actual offset is (offset * 8) bytes), an uint16_t tag and an uint16_t
// total datagram length.  The uint16_t values are stored in network
// byte order.
//
// There are three cases for mesh headers:
//  - For originating packets the mesh headers is added at transmission time,
//    and is not included in the in-memory format.
//  - For incoming packets that are being forwarded, the in-memory packet
//    begins with a mesh header and the entire packet is in its over-the-air
//    format (the forwarding node cannot expand the headers because doing
//    so might push a fragment out past the maximum packet size).
//  - For incoming packets where the local node is the mesh destination,
//    the packet is initially uncompressed to have the mesh header first
//    followed by the usual in-memory format.  When the packet reaches the
//    networking layer the mesh header is removed (so that it won't confuse
//    the fragmentation code).

#define INTERNAL_IP_OVERHEAD       6
#define INTERNAL_IP_HEADER_SIZE    (INTERNAL_IP_OVERHEAD + IPV6_HEADER_SIZE)
#define FRAGMENT_DATA_OFFSET_INDEX 1
#define FRAGMENT_DATA_TAG_INDEX    2
#define FRAGMENT_DATA_LENGTH_INDEX 4

// How much extra space we have to reserve to be sure everything fits.
#define PHY_RX_BUFFER_PADDING \
  (INTERNAL_IP_OVERHEAD + IPV6_HEADER_SIZE + UDP_HEADER_SIZE)

#define emInternalDatagramOffset(header) \
  (*emInternalDatagramOffsetPointer(header))

#define emInternalDatagramOffsetPointer(header) \
  ((header) + FRAGMENT_DATA_OFFSET_INDEX)

#define emInternalDatagramLength(header) \
  (emberFetchHighLowInt16u((header) + FRAGMENT_DATA_LENGTH_INDEX))

#define emSetInternalDatagramLength(header, length) \
  (emberStoreHighLowInt16u((header) + FRAGMENT_DATA_LENGTH_INDEX, (length)))

#define emInternalDatagramTag(header) \
  (emberFetchHighLowInt16u((header) + FRAGMENT_DATA_TAG_INDEX))

#define emSetInternalDatagramTag(header, tag) \
  (emberStoreHighLowInt16u((header) + FRAGMENT_DATA_TAG_INDEX, (tag)))

//----------------------------------------------------------------

bool emConvert6lowpanToInternal(PacketHeader header,
                                uint8_t *headerLength);
uint8_t emConvertInternalTo6lowpan(PacketHeader header,
                                   EmberNodeId meshSource,
                                   EmberNodeId meshDestination,
                                   uint8_t consumed,
                                   uint8_t **txFinger,
                                   bool *isMleLoc);

bool emFetchIpv6Header(PacketHeader packet, Ipv6Header *ipv6HeaderStruct);
bool emStoreIpv6Header(Ipv6Header *ipv6HeaderStruct, PacketHeader packet);

void emFetchHopByHopHeader(Ipv6Header *ipHeader);
void emFetchRoutingHeader(Ipv6Header *ipHeader);

//-------------------------------------------------------------
// Contexts.

typedef struct {
  uint16_t flags;
  uint8_t context;
  uint8_t *bytes;
  uint8_t length;
  uint8_t buffer[6];              // for multicast destinations
} CompressedAddress;

void emCompressMulticastAddress(uint8_t *address, CompressedAddress *result);

bool emFetchDestLongIdAfterMeshHeader(PacketHeader header, EmberEui64 *longId);
uint8_t *emFetchIcmpMessageAfterMeshHeader(PacketHeader header);

#ifdef EMBER_TEST

// For scripted tests.

Parcel *makeIpv6Internal(uint8_t nextHeader,
                         uint8_t hopCount,
                         Bytes16 source,
                         Bytes16 destination,
                         Parcel *ipPayload);

#endif

#endif // IP_6LOWPAN_HEADER_H
