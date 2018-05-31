/*
 * File: ip-header.h
 * Description: IPv6 packet header definitions.
 * Author(s): Matteo Paris, matteo@ember.com
 *
 * Copyright 2013 Silicon Laboratories, Inc.                                *80*
 */

#ifndef IP_HEADER_H
#define IP_HEADER_H

// IPv6 header (40 bytes)
//
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |Version| Traffic Class |           Flow Label                  |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |         Payload Length        |  Next Header  |   Hop Limit   |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                                                               |
//   +                                                               +
//   |                                                               |
//   +                         Source Address                        +
//   |                                                               |
//   +                                                               +
//   |                                                               |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                                                               |
//   +                                                               +
//   |                                                               |
//   +                      Destination Address                      +
//   |                                                               |
//   +                                                               +
//   |                                                               |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//   Version              4-bit Internet Protocol version number = 6.
//
//   Traffic Class        8-bit traffic class field.  The high two bits
//                        are Explicit Congestion Notification (ECN,
//                        RFC3168) and the low six bits are diffserv
//                        extension (RFC3168).  ECN is used to signal
//                        congestion to transport endpoints.  The four
//                        possible values are:
//                           0x00       no ECN
//                           0x01       endpoint is ECN capable
//                           0x02       endpoint is ECN capable
//                           0x03       congestion
//                        0x01 and 0x02 are equivalent.  Using two values
//                        makes it possible to detect if intermediate
//                        routers are not forwarding the ECN field properly.
//
//                        The first eight diffserv values (0x00-0x07)
//                        are priority values such that packets with
//                        higher diffserv values should be forwarded
//                        in preference to those with lower values.
//
//   Flow Label           20-bit flow label.
//
//   Payload Length       16-bit unsigned integer.  Length of the IPv6
//                        payload, i.e., the rest of the packet following
//                        this IPv6 header, in octets.  (Note that any
//                        extension headers [section 4] present are
//                        considered part of the payload, i.e., included
//                        in the length count.)
//
//   Next Header          8-bit selector.  Identifies the type of header
//                        immediately following the IPv6 header.  Uses the
//                        same values as the IPv4 Protocol field [RFC-1700
//                        et seq.].
//
//   Hop Limit            8-bit unsigned integer.  Decremented by 1 by
//                        each node that forwards the packet. The packet
//                        is discarded if Hop Limit is decremented to
//                        zero.
//
//   Source Address       128-bit address of the originator of the packet.
//                        See [ADDRARCH].
//
//   Destination Address  128-bit address of the intended recipient of the
//                        packet (possibly not the ultimate recipient, if
//                        a Routing header is present).  See [ADDRARCH]
//                        and section 4.4.

#define IPV6_HEADER_SIZE                40
#define IPV6_PAYLOAD_LENGTH_INDEX        4
#define IPV6_NEXT_HEADER_INDEX           6
#define IPV6_HOP_LIMIT_INDEX             7
#define IPV6_SOURCE_ADDRESS_INDEX        8
#define IPV6_DESTINATION_ADDRESS_INDEX  24

#define IPV6_DEFAULT_HOP_LIMIT 64

#define ipVersion(frame) ((frame)[0] >> 4)
#define ipPayloadLength(frame) \
  (emberFetchHighLowInt16u((frame) + IPV6_PAYLOAD_LENGTH_INDEX))

// IPv6 Extension Headers

// Hop-by-hop and destination extension headers contain type-length-value
// triples.  There are two types of padding for these.

#define EXTENSION_HEADER_OPTION_PAD1   0
#define EXTENSION_HEADER_OPTION_PADN   1
#define EXTENSION_HEADER_OPTION_RPL   99 // 0x63, in binary: 01100011
#define EXTENSION_HEADER_OPTION_MPL  109 // 0x6d, in binary: 01101101

// The first two bytes are common to all extension headers.
#define IPV6_EXT_NEXT_HEADER_INDEX 0
#define IPV6_EXT_LENGTH_INDEX 1

// The next two bytes are common to all Hop-by-Hop Options headers.
#define IPV6_HBH_OPTION_TYPE_INDEX 2
#define IPV6_HBH_OPTION_LENGTH_INDEX 3

// RPL Option carried in IPv6 Hop-by-Hop Options header
// draft-hui-6man-rpl-option-01
//
//     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//     |  Next Header  |  Hdr Ext Len  |  Option Type  |  Opt Data Len |
//     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//     |O|R|F|0|0|0|0|0| RPLInstanceID |          SenderRank           |
//     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#define IPV6_HBH_RPL_FLAGS_INDEX 4
#define IPV6_HBH_RPL_INSTANCE_ID_INDEX 5
#define IPV6_HBH_RPL_RANK_INDEX 6

#define IPV6_HBH_RPL_OPTION_LENGTH 8 // The whole thing.

#define IPV6_HBH_RPL_DOWN_BIT 0x80
#define IPV6_HBH_RPL_RANK_ERROR_BIT 0x40
#define IPV6_HBH_RPL_FORWARDING_ERROR_BIT 0x20

// MPL Option carried in IPv6 Hop-by-Hop Options header
// draft-ietf-roll-trickle-mcast-03
//
//     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//     |  Next Header  |  Hdr Ext Len  |  Option Type  |  Opt Data Len |
//     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//     | S |M|   rsv   |   sequence    |      seed-id (optional)       |
//     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#define IPV6_HBH_MPL_FLAGS_INDEX 4
#define IPV6_HBH_MPL_SEQUENCE_INDEX 5
#define IPV6_HBH_MPL_SEED_INDEX 6

#define IPV6_HBH_MPL_FLAGS_VALUE 0x40 // Set S=1 for 16-bit seed-id.

// RPL Routing Header
// draft-hui-6man-rpl-routing-header-07
//
//     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//     |  Next Header  |  Hdr Ext Len  | Routing Type=4| Segments Left |
//     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//     | CmprI | CmprE |  Pad  |       Reserved        | RPLInstanceID |
//     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//     |                                                               |
//     .                                                               .
//     .                        Addresses[1..n]                        .
//     .                                                               .
//     |                                                               |
//     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#define IPV6_RH_TYPE_INDEX 2
#define IPV6_RH_SEGMENTS_LEFT_INDEX 3
#define IPV6_RH_COMPRESSION_INDEX 4
#define IPV6_RH_PAD_INDEX 5
#define IPV6_RH_RESERVED_INDEX 7
#define IPV6_RH_INSTANCE_ID_INDEX 7
#define IPV6_RH_ADDRESSES_INDEX 8

#define IPV6_RH_COMPRESSION_DEFAULT 14  // Elides this many bytes.
#define IPV6_RH_TYPE 3

//------------------------------------------------------------------------------
// ICMP (V6)

//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |     Type      |     Code      |          Checksum             |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                                                               |
//       +                         Message Body                          +
//       |                                                               |

#define ICMP_HEADER_SIZE 4
#define ICMP_CHECKSUM_INDEX 2

#define RPL_CONTROL_MESSAGE_CODE_DIS 0x00
#define RPL_CONTROL_MESSAGE_CODE_DIO 0x01
#define RPL_CONTROL_MESSAGE_CODE_DAO 0x02
#define RPL_CONTROL_MESSAGE_CODE_DAO_ACK 0x03

//------------------------------------------------------------------------------
// UDP
//
//                 +--------+--------+--------+--------+
//                 |     Source      |   Destination   |
//                 |      Port       |      Port       |
//                 +--------+--------+--------+--------+
//                 |                 |                 |
//                 |     Length      |    Checksum     |
//                 +--------+--------+--------+--------+
//                 |
//                 |          data octets ...
//                 +---------------- ...

#define UDP_HEADER_SIZE 8
#define UDP_CHECKSUM_INDEX 6

// Well-known UDP port numbers.
#define MDNS_PORT 5353
#define PANA_PORT 716
#define MLE_PORT  19788  // 4D4C == "ML"

// TCP

#define TCP_HEADER_SIZE 20
#define TCP_CHECKSUM_INDEX 16

//------------------------------------------------------------------------------
// Utilities

extern bool emShortIdInterface;

#define emRoundDownToEight(len) ((len) & - 8)
#define emRoundUpToEight(len) (emRoundDownToEight((len) + 7))
// This uses len twice; if it gets used much it should be made into a function.
#define emPaddingToEight(len) (emRoundUpToEight(len) - (len))

enum {
  IP_HEADER_NO_OPTIONS                  = 0x00,
  IP_HEADER_LINK_LOCAL_SOURCE           = 0x01,  // lots of places
  IP_HEADER_LL64_SOURCE                 = 0x02,  // once in mle.c
  IP_HEADER_ONE_HOP                     = 0x04,  // nd.c, mle.c, rip.c
  IP_HEADER_GP64_SOURCE                 = 0x08,  // application messages
  IP_HEADER_IS_LEGACY                   = 0x20,  // mle.c
  IP_HEADER_GP16_SOURCE                 = 0x40,  // dhcp.c
};

// payloadBufferLength is the number of bytes of the transportPayload
// that will be in a separate payload buffer.  No space is allocated
// in the PacketHeader for these bytes.

PacketHeader emMakeIpHeader(Ipv6Header *ipHeader,
                            uint8_t tag,
                            uint8_t options,
                            const uint8_t *destination,
                            uint8_t hopLimit,
                            uint16_t payloadBufferLength);

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
                                   uint16_t payloadBufferLength);

// 0 instead of HEADER_TAG_NONE to avoid adding additional includes
#define emMakeUdpHeader(ipHeader,            \
                        options,             \
                        destination,         \
                        limit,               \
                        sourcePort,          \
                        destinationPort,     \
                        contents,            \
                        payloadLength,       \
                        payloadBufferLength) \
  (emMakeTaggedUdpHeader((ipHeader),         \
                         0,                  \
                         (options),          \
                         NULL,               \
                         (destination),      \
                         (limit),            \
                         (sourcePort),       \
                         (destinationPort),  \
                         (contents),         \
                         (payloadLength),    \
                         (payloadBufferLength)))

PacketHeader emMakeTaggedIcmpHeader(Ipv6Header *ipHeader,
                                    uint8_t tag,
                                    uint8_t options,
                                    uint8_t *source,
                                    const uint8_t *destination,
                                    uint8_t hopLimit,
                                    uint8_t type,
                                    uint8_t code,
                                    uint8_t *contents,
                                    uint16_t length);

// 0 instead of HEADER_TAG_NONE to avoid adding additional includes
#define emMakeIcmpHeader(ipHeader,       \
                         options,        \
                         source,         \
                         destination,    \
                         limit,          \
                         type,           \
                         code,           \
                         contents,       \
                         length)         \
  (emMakeTaggedIcmpHeader((ipHeader),    \
                          0,             \
                          (options),     \
                          (source),      \
                          (destination), \
                          (limit),       \
                          (type),        \
                          (code),        \
                          (contents),    \
                          (length)))

PacketHeader emMakeIpv6TunnelHeader(Ipv6Header *ipHeader,
                                    PacketHeader payload,
                                    uint8_t hopLimit,
                                    uint16_t payloadBufferLength);

void emReallyFetchIpv6Header(uint8_t *source, Ipv6Header *destination);
void emReallyStoreIpv6Header(Ipv6Header *source, uint8_t *destination);

bool emFetchUdpHeader(Ipv6Header *ipHeader);
void emStoreUdpHeader(Ipv6Header *ipHeader, uint16_t checksum);

#ifdef EMBER_TEST
// Scripted tests that do not want to bother with checksums should set
// this to false.
extern bool enableIpChecksums;
#endif

uint16_t emTransportChecksum(PacketHeader header, Ipv6Header *ipHeader);
void emSetTransportChecksum(PacketHeader header, Ipv6Header *ipHeader);

bool emSetIncomingTransportHeaderLength(Ipv6Header *ipHeader,
                                        uint8_t headerLength);

#ifdef EMBER_TEST

Parcel *makeIpv6HopByHop(uint8_t nextHeader, uint8_t instanceId, uint16_t rank);
void makeLl16(uint16_t id, uint8_t *target);

#endif

#endif // IP_HEADER_H
