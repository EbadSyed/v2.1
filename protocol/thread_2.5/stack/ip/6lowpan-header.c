/*
 * File: 6lowpan-header.c
 * Description: 6LowPAN header compression and expansion
 * Author(s): Richard Kelsey
 *
 * Copyright 2009 by Ember Corporation. All rights reserved.                *80*
 */

#include "core/ember-stack.h"
#include "framework/ip-packet-header.h"
#include "phy/phy.h"
#include "ip-address.h"
#include "ip-header.h"
#include "6lowpan-header.h"
#include "fragment.h"
#include "context-table.h"
#include "tls/debug.h"
#include "mac/mac-header.h"

#ifdef EMBER_TEST
#include "core/parcel.h"
#endif

#define LINK_LOCAL_CONTEXT 16
#define DEFAULT_CONTEXT     0
#define DEFAULT_CONTEXTS    0
#define NUMBER_OF_CONTEXTS 16

#define IPV6_ADDRESS_LENGTH 16

// If the header doesn't compress, use LOWPAN_IPV6_HEADER_BYTE. Tests can set
// this variable to 0 to disable compression.
HIDDEN uint8_t maxLowpanHeaderSize = IPV6_HEADER_SIZE;

// #undef emLogLine
// #define emLogLine(x, y, ...) fprintf(stderr, y"\n", ##__VA_ARGS__)
// #undef lose
// #define lose(x, y) { fprintf(stderr, "lost at %d\n", __LINE__); return y; }

//----------------------------------------------------------------
// Forward declarations
static bool convertIpv6HeaderToInternal(uint8_t **compressedLoc,
                                        uint8_t *compressedEnd,
                                        uint8_t **writeFingerLoc,
                                        uint16_t fragmentedLength,
                                        PacketHeader header,
                                        MeshHeader *meshHeader,
                                        Ipv6Header *outerIpv6Header,
                                        bool isLegacy);
static uint16_t readUdpPort(uint8_t frameControlBit, uint8_t **compressedLoc);
static uint8_t *internalIpStart(PacketHeader header);

static const uint8_t hopLimits[] = { 0, 1, 64, 255 };

static const struct {
  uint8_t count;
  uint8_t prefixCount;
} inlineMulticastByteCounts[] =
{
  { 16, 0 },    // no compression
  {  6, 1 },    // FFXX::00XX:XXXX:XXXX
  {  4, 1 },    // FFXX::00XX:XXXX
  {  1, 0 },    // FF02::00XX
  {  6, 2 },    // FFXX::XX[plen]:[prefix]:XXXX:XXXX (plen and prefix from
                //   the context)
  {  0, 0 },    // reserved
  {  0, 0 },    // reserved
  {  0, 0 }     // reserved
};

static const uint8_t inlineUnicastByteCounts[] = { 16, 8, 2, 0 };

// Returns the number of bytes read from 'source'.
// This varies due to extension headers.

static uint8_t fetchIpv6Header(uint8_t *source, Ipv6Header *destination)
{
  emReallyFetchIpv6Header(source, destination);
  // Fetch the extension header too.
  // We support a single extension of the following types.
  if (destination->nextHeader == IPV6_NEXT_HEADER_HOP_BY_HOP
      || destination->nextHeader == IPV6_NEXT_HEADER_ROUTING) {
    uint8_t *contents = destination->ipPayload;
    uint8_t extLength = (contents[IPV6_EXT_LENGTH_INDEX] + 1) << 3;
    destination->transportHeader += extLength;
    destination->transportProtocol = contents[IPV6_EXT_NEXT_HEADER_INDEX];
    return IPV6_HEADER_SIZE + extLength;
  } else {
    return IPV6_HEADER_SIZE;
  }
}

bool emConvert6lowpanToInternal(PacketHeader header,
                                uint8_t *headerLengthLoc)
{
  uint8_t *macPayload = emMacPayloadPointer(header);
  uint8_t *compressed = macPayload;
  uint8_t compressedLength = *headerLengthLoc - emMacPayloadIndex(header);
  uint8_t *expanded = macPayload;
  uint16_t fragmentedLength = 0;
  bool isLegacy = (emOnLurkerNetwork()
                   && (emHeaderMacInfoField(header) & MAC_INFO_LEGACY_KEY));

  MeshHeader meshHeader;
  bool haveMeshHeader = false;

  if (emReadMeshHeader(header, &meshHeader)) {
    haveMeshHeader = true;
    compressed += meshHeader.size;
    expanded += meshHeader.size;
    compressedLength -= meshHeader.size;
  }

  if ((compressed[0] & LOWPAN_FRAGMENT_MASK) == LOWPAN_FIRST_FRAGMENT) {
    emLogLine(LOWPAN, "first fragment");
    fragmentedLength = emberFetchHighLowInt16u(compressed) & 0x07FF;
    compressed += FIRST_FRAGMENT_HEADER_LENGTH;
    compressedLength -= FIRST_FRAGMENT_HEADER_LENGTH;
    expanded += FIRST_FRAGMENT_HEADER_LENGTH;
    if (fragmentedLength == 0) {        // avoid confusion if we receive
      emLogLine(LOWPAN, "zero len frag");
      lose(LOWPAN, false);              // something goofy
    }
  } else if ((compressed[0] & LOWPAN_FRAGMENT_MASK) == LOWPAN_LATER_FRAGMENT) {
    emLogLine(LOWPAN, "later frag");
    return true;       // later fragments have no header
  }

// emLogBytesLine(LOWPAN, "in", emGetBufferPointer(header), emGetBufferLength(header));

  if (compressed[0] == LOWPAN_IPV6_HEADER_BYTE) {
    if ((compressed[1] & 0xF0) != 0x60) {
      emLogLine(LOWPAN, "bad version");
      lose(LOWPAN, false);
    }
    if (fragmentedLength == 0) {
      // Leave LOWPAN_IPV6_HEADER_BYTE alone at the front, adding space for
      // the internal IP overhead.
      MEMMOVE(expanded + INTERNAL_IP_OVERHEAD,
              compressed + 1,
              compressedLength - 1);
      MEMSET(expanded + 1, 0, INTERNAL_IP_OVERHEAD - 1);
      *headerLengthLoc += INTERNAL_IP_OVERHEAD - 1;
    } else {
      // remove the LOWPAN_IPV6_HEADER_BYTE
      MEMMOVE(compressed, compressed + 1, compressedLength - 1);
      *headerLengthLoc -= 1;
    }
    return true;
  }

  // If we get here we have an actual compressed IPv6 header.
  {
    uint16_t bufferLength = emGetBufferLength(header);
    uint16_t extraSpace = bufferLength - *headerLengthLoc;
    uint8_t *bufferStart = emGetBufferPointer(header);
    uint8_t *readFinger = compressed + extraSpace;
    uint8_t *readLimit = bufferStart + bufferLength;

    // Copy the compressed data to the end of the buffer to make room
    // at the front for the uncompressed headers.
    MEMMOVE(readFinger, compressed, compressedLength);

    if (fragmentedLength == 0) {
      expanded[0] = LOWPAN_IPV6_HEADER_BYTE;
      MEMSET(expanded + 1, 0, INTERNAL_IP_OVERHEAD - 1);
      expanded += INTERNAL_IP_OVERHEAD;
    }

    uint8_t *writeFinger = expanded;
    if (!convertIpv6HeaderToInternal(&readFinger,
                                     readLimit,
                                     &writeFinger,
                                     fragmentedLength,
                                     header,
                                     (haveMeshHeader
                                      ? &meshHeader
                                      : NULL),
                                     NULL,
                                     isLegacy)) {
      lose(LOWPAN, false);
    }

    {
      // Copy any remaining noncompressed payload forward again.
      uint16_t remaining = readLimit - readFinger;
      MEMMOVE(writeFinger, readFinger, remaining);
      writeFinger += remaining;
      // For safety we zero out the copy of the end of the message that is
      // lying at the end of the buffer.
      MEMSET(writeFinger, 0, readLimit - writeFinger);
    }

    *headerLengthLoc = writeFinger - emGetBufferPointer(header);

    // emLogBytesLine(LOWPAN, "out",
    //                      emGetBufferPointer(header),
    //                      emGetBufferLength(header));
    return true;
  }
}

// This maps the IDs used in compressed extension headers with the
// actual IPV6 next hop values.

static uint8_t extensionNextHeaders[] = {
  IPV6_NEXT_HEADER_HOP_BY_HOP,
  IPV6_NEXT_HEADER_ROUTING,
  IPV6_NEXT_HEADER_FRAGMENT,
  IPV6_NEXT_HEADER_DESTINATION,
  IPV6_NEXT_HEADER_MOBILITY,
  IPV6_NEXT_HEADER_UNKNOWN,
  IPV6_NEXT_HEADER_UNKNOWN,
  IPV6_NEXT_HEADER_IPV6
};

// This expands a single compressed IPv6 header and any following
// extension and/or UDP headers.  If there is a following compressed
// IPv6 header it is handled by a recursive call.
//
// The compressed and writeFinger values are updated to their final
// values.
//
// fragmentedLength is the length of the full uncompressed packet,
// as provided by an initial fragmentation header.  For unfragmented
// packets the compressed packet ends at compressedEnd.
//
// The compressed data has been moved to the end of the buffer, so
// the write finger is ahead of the compressed finger in the packet.

static bool convertIpv6HeaderToInternal(uint8_t **compressedLoc,
                                        uint8_t *compressedEnd,
                                        uint8_t **writeFingerLoc,
                                        uint16_t fragmentedLength,
                                        PacketHeader header,
                                        MeshHeader *meshHeader,
                                        Ipv6Header *outerIpv6Header,
                                        bool isLegacy)
{
  Ipv6Header ipHeader;
  uint8_t *compressed = *compressedLoc;
  uint8_t *writeFinger = *writeFingerLoc;
  uint8_t *headerLoc = writeFinger;
  bool nextHeaderIsCompressed = false;
  uint8_t contexts;
  int8_t shift;
  uint8_t *address;

  uint16_t frameControl = emberFetchHighLowInt16u(compressed);
  bool contextIdFlag = (frameControl & LOWPAN_IPHC_CONTEXT_ID_FLAG);
  compressed += 2;                              // skip frame control
  emLogLine(LOWPAN, "2 (frame control) %2x", frameControl);

  MEMSET(&ipHeader, 0, sizeof(Ipv6Header));

  writeFinger += IPV6_HEADER_SIZE;              // room for header

  if (EM_LOG_LOWPAN_ENABLED && contextIdFlag) {
    emLogLine(LOWPAN, "1 (context) %X", *compressed);
  }

  contexts = (contextIdFlag ? *compressed++ : DEFAULT_CONTEXTS);

  if ((frameControl & LOWPAN_IPHC_TYPE_MASK)
      != LOWPAN_IPHC_TYPE_VALUE) {
    emLogLine(LOWPAN, "%2X & %2X != %2X",
              frameControl,
              LOWPAN_IPHC_TYPE_MASK,
              LOWPAN_IPHC_TYPE_VALUE);
    lose(LOWPAN, false);
  }

  {
    uint32_t first32 = emberFetchHighLowInt32u(compressed);
    emLogLine(LOWPAN, "first32 %4X", first32);
    switch (frameControl & LOWPAN_IPHC_TRAFFIC_CLASS_FLOW_LABEL_MASK) {
      case LOWPAN_IPHC_TRAFFIC_AND_FLOW:
        // Traffic Class + 4-bit pad + Flow Label (4 bytes)
        compressed += 4;
        emLogLine(LOWPAN, "4 (ECN + DCSP + flow label)");
        break;
      case LOWPAN_IPHC_ECN_AND_FLOW: {
        // ECN + 2-bit pad + Flow Label (3 bytes)
        uint32_t ecn = first32 & 0xC0000000;
        first32 &= 0x0FFFFF00; // clear out the ECN and pad bits
        first32 >>= 8;  // move the flow label up to its usual location
        first32 |= ecn; // restore the ECN
        emLogLine(LOWPAN, "3 (ECN + flow label)");
        compressed += 3;
        break;
      }
      case LOWPAN_IPHC_TRAFFIC_AND_NO_FLOW:
        // Traffic Class (1 byte)
        first32 &= 0xFF000000; // clear out the flow label
        emLogLine(LOWPAN, "1 (ECN + DSCP)");
        compressed += 1;
        break;
      case LOWPAN_IPHC_NO_TRAFFIC_AND_NO_FLOW:
        first32 = 0;
        break;
    }
    // Now first32 has the form
    //   ECN:2 DSCP:6 reserved:4 flow label:20
    // and we need
    //   version(=6):4 DSCP:6 ECN:2 flow label:20
    emLogLine(LOWPAN, "first32 %4X", first32);
    ipHeader.trafficClass = (((first32 & 0x3F000000) >> 22)      // DSCP
                             | ((first32 & 0xC0000000) >> 30));  // ECN
    ipHeader.flowLabel = (first32 & 0x000FFFFF);
    emLogLine(LOWPAN, "first32 %4X", first32);
  }

  if (frameControl & LOWPAN_IPHC_NEXT_HEADER_FLAG) {
    nextHeaderIsCompressed = true;
  } else {
    ipHeader.nextHeader = *compressed++;
    nextHeaderIsCompressed = false;
    emLogLine(LOWPAN, "1 (next header)");
  }

  if ((frameControl & LOWPAN_IPHC_HOP_LIMIT_MASK)
      == LOWPAN_IPHC_HOP_LIMIT_IN_LINE) {
    ipHeader.hopLimit = *compressed++;
    emLogLine(LOWPAN, "1 (hop limit)");
  } else {
    ipHeader.hopLimit = hopLimits[(frameControl
                                   & LOWPAN_IPHC_HOP_LIMIT_MASK) >> 8];
  }

  // Two passes, first for source and then for destination
  for (shift = 4, address = ipHeader.source;
       0 <= shift;
       shift -= 4, address = ipHeader.destination) {
    uint8_t bits = (frameControl >> shift) & 0x07;
    uint8_t context = (contexts >> shift) & 0x0F;
    bool isSource = shift == 4;

    if (!isSource
        && (frameControl & LOWPAN_IPHC_MULTICAST_COMPRESSION_FLAG)) {
      uint8_t prefix = inlineMulticastByteCounts[bits].prefixCount;
      uint8_t suffix = (inlineMulticastByteCounts[bits].count
                        - prefix);

      address[0] = 0xFF;
      address[1] = 0x02;
      MEMMOVE(address + 1, compressed, prefix);
      compressed += prefix;
      MEMMOVE(address + 16 - suffix, compressed, suffix);
      compressed += suffix;

      emLogLine(LOWPAN, "%d (multicast)", prefix + suffix);
      emLogLine(LOWPAN, "bits %X prefix %d suffix %d",
                bits,
                prefix, suffix);

      if (bits == LOWPAN_IPHC_CONTEXT_MULTICAST) {
        // FFXX:XXLL:PPPP:PPPP:PPPP:PPPP:XXXX:XXXX where LL is
        // the prefix length and P...P is the prefix, both from the context
        uint8_t tempAddress[16];
        uint8_t len = emUncompressContextPrefix(context, tempAddress);
        if (len == 0) {
          emLogLine(LOWPAN, "no multicast context bytes");
          lose(LOWPAN, false);
        } else if (len > 64) {
          emLogLine(LOWPAN, "multicast context too long");
          lose(LOWPAN, false);
        }
        address[3] = len;
        MEMMOVE(address + 4, tempAddress, 8);
      }
    } else if (bits == 0x00) {
      MEMMOVE(address, compressed, 16);
      compressed += 16;
    } else if (bits == 0x04) {
      if (isSource) {
        emLogLine(LOWPAN, "0 (unspecified source)");    // all zeros
      } else {
        emLogLine(LOWPAN, "dest 0x04 is reserved");
        lose(LOWPAN, false);
      }
    } else {
      uint16_t inlineBytes = inlineUnicastByteCounts[bits & 0x03];
      uint8_t *interfaceId = address + 8;

      // set the interface
      if (inlineBytes == 8) {
        MEMMOVE(interfaceId, compressed, 8);
      } else if (inlineBytes == 2) {
        emStoreShortInterfaceId(emberFetchHighLowInt16u(compressed),
                                interfaceId);
      } else if (outerIpv6Header != NULL) {
        MEMMOVE(interfaceId,
                ((isSource
                  ? outerIpv6Header->source
                  : outerIpv6Header->destination)
                 + 8),
                8);
      } else if (meshHeader != NULL) {
        emStoreShortInterfaceId((isSource
                                 ? meshHeader->source
                                 : meshHeader->destination),
                                interfaceId);
      } else if (isSource) {
        if (emMacSourceMode(header) == MAC_FRAME_SOURCE_MODE_LONG) {
          emLongIdToInterfaceId(emMacSourcePointer(header), interfaceId);
        } else {
          emStoreShortInterfaceId(emMacShortSource(header), interfaceId);
        }
      } else {
        if (emMacDestinationMode(header) == MAC_FRAME_DESTINATION_MODE_LONG) {
          emLongIdToInterfaceId(emMacExtendedId, interfaceId);
        } else {
          emStoreShortInterfaceId(emberGetNodeId(), interfaceId);
        }
      }

      // set the prefix
      // For nonstandard context lengths (between 64 and 128 bits) this
      // will overwrite part of the interface id, as per RFC6282.
      if (!(bits & 0x04)) {
        address[0] = 0xFE;
        address[1] = 0x80;
      } else if (isLegacy && context == 0) {
        emStoreLegacyUla(address);
      } else if (!emUncompressContextPrefix(context, address)) {
        emLogLine(LOWPAN, "no such context %d", context);
        lose(LOWPAN, false);
      }

      compressed += inlineBytes;
      emLogLine(LOWPAN,
                "%d (address %X %d %2X)",
                inlineBytes,
                bits,
                shift,
                frameControl);
    }
  }

  if (nextHeaderIsCompressed) {
    uint8_t *nextHeaderLoc = &ipHeader.nextHeader;
    while (true) {
      uint8_t nextByte = *compressed++;

      if ((nextByte & LOWPAN_UDP_HEADER_MASK)
          == LOWPAN_UDP_HEADER_VALUE) {
        uint16_t checksum = 0x0000; // Indicates that no checksum was sent.
        Ipv6Header thisIpHeader;
        MEMSET(&thisIpHeader, 0, sizeof(Ipv6Header));
        *nextHeaderLoc = IPV6_NEXT_HEADER_UDP;

        if ((nextByte & LOWPAN_UDP_PORT_MASK)
            == LOWPAN_UDP_SHORT_PORTS) {
          uint8_t ports = *compressed++;
          thisIpHeader.sourcePort =
            LOWPAN_ONE_NIBBLE_PORT_PREFIX | (ports >> 4);
          thisIpHeader.destinationPort =
            LOWPAN_ONE_NIBBLE_PORT_PREFIX | (ports & 0x0F);
        } else {
          thisIpHeader.sourcePort =
            readUdpPort(nextByte & LOWPAN_UDP_SHORT_SOURCE_PORT,
                        &compressed);
          thisIpHeader.destinationPort =
            readUdpPort(nextByte & LOWPAN_UDP_SHORT_DESTINATION_PORT,
                        &compressed);
        }

        if (!(nextByte & LOWPAN_UDP_CHECKSUM_ELIDED)) {
          checksum = emberFetchHighLowInt16u(compressed);
          compressed += 2;
        }

        thisIpHeader.transportHeader = writeFinger;
        writeFinger += UDP_HEADER_SIZE;
        thisIpHeader.transportPayloadLength =
          fragmentedLength == 0
          ? compressedEnd - compressed
          : fragmentedLength - (writeFinger - headerLoc);
        emStoreUdpHeader(&thisIpHeader, checksum);
        break;          // no more compressed headers
      } else if ((nextByte & LOWPAN_EXT_HEADER_MASK)
                 == LOWPAN_EXT_HEADER_VALUE) {
        uint8_t id = nextByte & LOWPAN_EXT_HEADER_ID_MASK;
        *nextHeaderLoc = extensionNextHeaders[id >> 1];
        if (*nextHeaderLoc == IPV6_NEXT_HEADER_UNKNOWN) {
          emLogLine(LOWPAN, "unknown compressed header");
          lose(LOWPAN, false);
        } else if (*nextHeaderLoc == IPV6_NEXT_HEADER_IPV6) {
          // Re-enter to expand inner ipv6 header.
          *compressedLoc = compressed;
          *writeFingerLoc = writeFinger;
          if (!convertIpv6HeaderToInternal(compressedLoc,
                                           compressedEnd,
                                           writeFingerLoc,
                                           (fragmentedLength == 0
                                            ? 0
                                            : (fragmentedLength
                                               - (writeFinger - headerLoc))),
                                           header,
                                           NULL,
                                           &ipHeader,
                                           isLegacy)) {
            lose(LOWPAN, false);
          }
          compressed = *compressedLoc;
          writeFinger = *writeFingerLoc;
          break;          // no more compressed headers
        } else {
          uint8_t *extensionStart = writeFinger;
          nextHeaderIsCompressed =
            (nextByte & LOWPAN_EXT_HEADER_NH_MASK) != 0;

          if (nextHeaderIsCompressed) {
            writeFinger += 1;                  // skip next header field
          } else {
            emLogLine(LOWPAN, "extension next nextHeader %X", *compressed);
            *writeFinger++ = *compressed++;    // store the next header
          }

          {
            uint16_t remainingBytes = *compressed++;
            // + 2 for the next header and length
            uint16_t compressedLength = remainingBytes + 2;
            // Round up to a multiple of eight bytes
            uint16_t actualLength = (compressedLength + 7) & ~0x07;
            uint16_t padding = actualLength - compressedLength;
            *writeFinger++ = (actualLength >> 3) - 1;
            MEMMOVE(writeFinger, compressed, remainingBytes);
            writeFinger += remainingBytes;
            compressed += remainingBytes;
            // BUG - this needs to be a Pad1 or PadN option.
            MEMSET(writeFinger, 0, padding);
            writeFinger += padding;
            emLogLine(LOWPAN,
                      "extension remaining %d actual %d padding %d added %d",
                      remainingBytes,
                      actualLength,
                      padding,
                      writeFinger - extensionStart);
            emLogLine(LOWPAN,
                      "extension nextByte %X nextHeader %X remaining %d",
                      nextByte,
                      *nextHeaderLoc,
                      remainingBytes);
          }

          if (nextHeaderIsCompressed) {
            nextHeaderLoc = extensionStart;
          } else {
            break;      // no more compressed headers
          }
        }
      } else {
        lose(LOWPAN, false);
      }
    }
  }

  if (compressedEnd < compressed) {
    emLogLine(LOWPAN, "overran end of packet");
    lose(LOWPAN, false);
  }

  // Now that we know the IP payload length we can write the IP header.
  if (fragmentedLength == 0) {
    ipHeader.ipPayloadLength =
      (writeFinger - (headerLoc + IPV6_HEADER_SIZE)) // expanded payload
      + (compressedEnd - compressed);        // tail that was not compressed
  } else {
    ipHeader.ipPayloadLength =
      fragmentedLength - IPV6_HEADER_SIZE;
  }
  emLogLine(LOWPAN,
            "storing header at %d",
            headerLoc - emGetBufferPointer(header));
  if (outerIpv6Header == NULL) {
    emStoreIpv6Header(&ipHeader, header);       // sets fragment length field
  } else {
    emReallyStoreIpv6Header(&ipHeader, headerLoc);
  }

  *compressedLoc = compressed;
  *writeFingerLoc = writeFinger;

  return true;
}

static uint16_t readUdpPort(uint8_t frameControlBit, uint8_t **compressedLoc)
{
  uint16_t port;
  if (frameControlBit) {
    port = LOWPAN_ONE_BYTE_PORT_PREFIX | **compressedLoc;
    *compressedLoc += 1;
  } else {
    port = emberFetchHighLowInt16u(*compressedLoc);
    *compressedLoc += 2;
  }
  return port;
}

//----------------------------------------------------------------

void emCompressMulticastAddress(uint8_t *address, CompressedAddress *result)
{
  uint8_t i;
  uint8_t contextId;
  emLogLine(LOWPAN, "is multicast");

  result->flags = LOWPAN_IPHC_MULTICAST_COMPRESSION_FLAG;

  for (i = 2; i < 16 && address[i] == 0; i++) {
    ;
  }

  emLogLine(LOWPAN, "i %d", i);
  result->bytes = result->buffer;

  if (14 < i && address[1] == 0x02) {
    result->flags |= LOWPAN_IPHC_1_BYTE_MULTICAST;
    result->bytes = address + 15;
    result->length = 1;
  } else if (12 < i) {
    result->flags |= LOWPAN_IPHC_4_BYTE_MULTICAST;
    result->buffer[0] = address[1];
    result->buffer[1] = address[13];
    result->buffer[2] = address[14];
    result->buffer[3] = address[15];
    result->length = 4;
  } else if (10 < i) {
    result->flags |= LOWPAN_IPHC_6_BYTE_MULTICAST;
    result->buffer[0] = address[1];
    MEMMOVE(result->buffer + 1, address + 11, 5);
    result->length = 6;
  } else if (emCompressContextPrefix(address + 4, 64, &contextId) == 64
             && address[3] == 64) {
    // we only perform multicast context compression for 8 byte prefixes
    result->flags |= LOWPAN_IPHC_CONTEXT_MULTICAST;
    result->context = contextId;
    result->buffer[0] = address[1];
    result->buffer[1] = address[2];
    MEMMOVE(result->buffer + 2, address + 12, 4);
    result->length = 6;
  } else {
    result->flags |= LOWPAN_IPHC_16_BYTE_MULTICAST;
    result->bytes = address;
    result->length = 16;
  }
  emLogLine(LOWPAN, "flags %2X", result->flags);
}

static void compressUnicastAddress(uint8_t *address,
                                   EmberNodeId macShortId,
                                   uint8_t *macLongId,
                                   uint8_t *outerAddress,
                                   CompressedAddress *result)
{
  bool stateless = emIsFe8Address(address);
  uint8_t contextBits = 0;
  result->context = LINK_LOCAL_CONTEXT;
  result->bytes = address;
  result->length = 16;
  if (!stateless) {
    contextBits = emCompressContextPrefix(address, 128, &result->context);
    if (contextBits < 64) {
      contextBits = 0;  // code below can't handle prefixes < 64 bits.
    }
  }

  if (stateless || contextBits) {
    uint8_t interfaceId[8];
    result->length = (contextBits == 128
                      ? 0
                      : (contextBits >= 112
                         ? 2
                         : 8));

    if (outerAddress == NULL) {
      if (macLongId != NULL) {
        emLongIdToInterfaceId(macLongId, interfaceId);
        if (MEMCOMPARE(address + 8, interfaceId, 8) == 0) {
          result->length = 0;
        }
      }
    } else if (MEMCOMPARE(address + 8, outerAddress + 8, 8) == 0) {
      result->length = 0;
    }

    if (result->length != 0) {
      EmberNodeId inlineShortId;
      inlineShortId = emberFetchHighLowInt16u(address + 14);
      emStoreShortInterfaceId(inlineShortId, interfaceId);
      if (result->length == 8) {
        if (MEMCOMPARE(address + 8, interfaceId, 8) == 0) {
          result->length = 2;
        }
      }
      if (result->length == 2) {
        if (outerAddress == NULL
            && inlineShortId == macShortId) {
          result->length = 0;
        }
      }
    }
    result->bytes += (16 - result->length);
  }
  if (result->context != LINK_LOCAL_CONTEXT) {
    if (result->context == DEFAULT_CONTEXT) {
      result->flags = (LOWPAN_IPHC_SOURCE_COMPRESSION_FLAG
                       | LOWPAN_IPHC_DESTINATION_COMPRESSION_FLAG);
    } else {
      result->flags = (LOWPAN_IPHC_SOURCE_COMPRESSION_FLAG
                       | LOWPAN_IPHC_DESTINATION_COMPRESSION_FLAG
                       | LOWPAN_IPHC_CONTEXT_ID_FLAG);
    }
  }
  if (result->length == 16) {
    // Force stateless (no context) for 16 bytes inline.
    result->flags = (LOWPAN_IPHC_16_BYTE_SOURCE
                     | LOWPAN_IPHC_16_BYTE_DESTINATION);
  } else if (result->length == 8) {
    result->flags |= (LOWPAN_IPHC_8_BYTE_SOURCE
                      | LOWPAN_IPHC_8_BYTE_DESTINATION);
  } else if (result->length == 2) {
    result->flags |= (LOWPAN_IPHC_2_BYTE_SOURCE
                      | LOWPAN_IPHC_2_BYTE_DESTINATION);
  } else {
    result->flags |= (LOWPAN_IPHC_0_BYTE_SOURCE
                      | LOWPAN_IPHC_0_BYTE_DESTINATION);
  }
  // Mask out the high nibble of the context now we have set the flag.
  result->context &= 0x0F;
}

// 'useHeader' controls whether or not the MAC header can be
// referenced when doing the compression.  For packets with a mesh
// header the final destination will uncompress using a different MAC
// header, so we can't use the one we have.
//
// When compressing a nested header, 'outerHeader' is the next outermost
// header and is referenced in place of the MAC header when compressing
// addresses.

static uint8_t convertInternalTo6lowpan(PacketHeader header,
                                        EmberNodeId meshSource,
                                        EmberNodeId meshDestination,
                                        bool useHeader,
                                        Ipv6Header *outerHeader,
                                        uint8_t consumed,
                                        uint8_t **txFinger,
                                        bool *isMleLoc)
{
  Ipv6Header ipHeader;
  uint8_t *compressed = *txFinger;
  uint8_t *compressedStart = compressed;
  uint16_t frameControl = LOWPAN_IPHC_TYPE_VALUE;
  CompressedAddress source;
  CompressedAddress destination;
  uint8_t *start = internalIpStart(header);

  if (start == NULL) {
    return consumed;
  }

  fetchIpv6Header(start + consumed, &ipHeader);
  consumed += IPV6_HEADER_SIZE;

  MEMSET(&source, 0, sizeof(CompressedAddress));
  if (emIsMemoryZero(ipHeader.source, 16)) {
    source.flags = LOWPAN_IPHC_SOURCE_COMPRESSION_FLAG;
  } else if (ipHeader.source[0] == 0xFF) {
    // multicast source not allowed. This could happen if the application
    // hands us something malformed to send out.  EMIPSTACK-884.
    return 0xFF;
  } else {
    EmberNodeId macShortId = EMBER_NULL_NODE_ID;
    uint8_t *macLongId = NULL;
    if (useHeader) {
      if (meshSource != 0xFFFE) {
        macShortId = meshSource;
      } else if (emMacSourceMode(header) == MAC_FRAME_SOURCE_MODE_LONG) {
        macLongId = (uint8_t *) emMacExtendedId;
      } else {
        macShortId = emberGetNodeId();
      }
    }
    compressUnicastAddress(ipHeader.source,
                           macShortId,
                           macLongId,
                           (outerHeader == NULL
                            ? NULL
                            : outerHeader->source),
                           &source);
    source.flags &= (LOWPAN_IPHC_SOURCE_COMPRESSION_FLAG
                     | LOWPAN_IPHC_SOURCE_ADDRESS_MODE
                     | LOWPAN_IPHC_CONTEXT_ID_FLAG);
  }

  MEMSET(&destination, 0, sizeof(CompressedAddress));
  if (ipHeader.destination[0] == 0xFF) {
    emCompressMulticastAddress(ipHeader.destination, &destination);
  } else {
    EmberNodeId macShortId = EMBER_NULL_NODE_ID;
    uint8_t *macLongId = NULL;
    if (useHeader) {
      if (meshDestination != 0xFFFE) {
        macShortId = meshDestination;
      } else if (emMacDestinationMode(header) == MAC_FRAME_DESTINATION_MODE_LONG) {
        macLongId = emMacDestinationPointer(header);
      } else {
        macShortId = emMacShortDestination(header);
      }
    }
    compressUnicastAddress(ipHeader.destination,
                           macShortId,
                           macLongId,
                           (outerHeader == NULL
                            ? NULL
                            : outerHeader->destination),
                           &destination);
    destination.flags &= (LOWPAN_IPHC_DESTINATION_COMPRESSION_FLAG
                          | LOWPAN_IPHC_DESTINATION_ADDRESS_MODE
                          | LOWPAN_IPHC_CONTEXT_ID_FLAG);
  }

  frameControl |= (source.flags | destination.flags);

  compressed += 2;      // skip frame control

  if (frameControl & LOWPAN_IPHC_CONTEXT_ID_FLAG) {
    *compressed++ = ((source.context << 4) | destination.context);
  }

  if (ipHeader.trafficClass == 0
      && ipHeader.flowLabel == 0) {
    emLogLine(LOWPAN, "no traffic or flow");
    frameControl |= LOWPAN_IPHC_NO_TRAFFIC_AND_NO_FLOW;
  } else {
    uint8_t ecn = (ipHeader.trafficClass & 0x03) << 6;
    uint8_t trafficClass = ecn | (ipHeader.trafficClass >> 2);
    uint32_t flowLabel = ipHeader.flowLabel;
    emLogLine(LOWPAN, "ecn %01X class %X flow %4X",
              ecn, trafficClass, flowLabel);
    if (flowLabel == 0) {
      frameControl |= LOWPAN_IPHC_TRAFFIC_AND_NO_FLOW;
      emLogLine(LOWPAN, "traffic and no flow");
      *compressed++ = trafficClass;
    } else {
      if (trafficClass == ecn) {
        frameControl |= LOWPAN_IPHC_ECN_AND_FLOW;
        emLogLine(LOWPAN, "ECN and flow");
        *compressed = ecn;
      } else {
        emLogLine(LOWPAN, "traffic and flow");
        frameControl |= LOWPAN_IPHC_TRAFFIC_AND_FLOW;
        *compressed++ = trafficClass;
        *compressed = 0;
      }
      *compressed++ |= flowLabel >> 16;
      *compressed++ = flowLabel >> 8;
      *compressed++ = flowLabel;
    }
  }

  if (ipHeader.nextHeader == IPV6_NEXT_HEADER_ROUTING
      || ipHeader.nextHeader == IPV6_NEXT_HEADER_HOP_BY_HOP
      || ipHeader.nextHeader == IPV6_NEXT_HEADER_UDP
      || ipHeader.nextHeader == IPV6_NEXT_HEADER_IPV6) {
    frameControl |= LOWPAN_IPHC_NEXT_HEADER_FLAG;
  } else {
    *compressed++ = ipHeader.nextHeader;
  }

  {
    switch (ipHeader.hopLimit) {
      case 1:
        frameControl |= LOWPAN_IPHC_HOP_LIMIT_1;
        break;
      case 64:
        frameControl |= LOWPAN_IPHC_HOP_LIMIT_64;
        break;
      case 255:
        frameControl |= LOWPAN_IPHC_HOP_LIMIT_255;
        break;
      default:
        frameControl |= LOWPAN_IPHC_HOP_LIMIT_IN_LINE;
        *compressed++ = ipHeader.hopLimit;
        break;
    }
  }

  MEMMOVE(compressed, source.bytes, source.length);
  compressed += source.length;
  MEMMOVE(compressed, destination.bytes, destination.length);
  compressed += destination.length;

  emberStoreHighLowInt16u(compressedStart, frameControl);

  if (maxLowpanHeaderSize < compressed - compressedStart) {
    *compressedStart = LOWPAN_IPV6_HEADER_BYTE;
    emReallyStoreIpv6Header(&ipHeader, compressedStart + 1);
    compressed = compressedStart + 1 + IPV6_HEADER_SIZE;
  }

  uint16_t thisHeader = ipHeader.nextHeader;

  while (thisHeader == IPV6_NEXT_HEADER_ROUTING
         || thisHeader == IPV6_NEXT_HEADER_HOP_BY_HOP) {
    uint8_t *headerBytes = start + consumed;
    uint8_t headerLength = (headerBytes[IPV6_EXT_LENGTH_INDEX] + 1) << 3;
    uint16_t nextHeader = headerBytes[IPV6_EXT_NEXT_HEADER_INDEX];
    if (nextHeader == IPV6_NEXT_HEADER_ROUTING
        || nextHeader == IPV6_NEXT_HEADER_HOP_BY_HOP
        || nextHeader == IPV6_NEXT_HEADER_UDP
        || nextHeader == IPV6_NEXT_HEADER_IPV6) {
      *compressed++ = (thisHeader == IPV6_NEXT_HEADER_ROUTING
                       ? (LOWPAN_EXT_HEADER_VALUE
                          | LOWPAN_EXT_HEADER_NH_MASK
                          | LOWPAN_ROUTING_EID)
                       : (LOWPAN_EXT_HEADER_VALUE
                          | LOWPAN_EXT_HEADER_NH_MASK
                          | LOWPAN_HOP_BY_HOP_EID));
    } else {
      *compressed++ = (thisHeader == IPV6_NEXT_HEADER_ROUTING
                       ? (LOWPAN_EXT_HEADER_VALUE | LOWPAN_ROUTING_EID)
                       : (LOWPAN_EXT_HEADER_VALUE | LOWPAN_HOP_BY_HOP_EID));
      *compressed++ = nextHeader;     // next header not compressed
    }
    *compressed++ = headerLength - 2;
    MEMMOVE(compressed, headerBytes + 2, headerLength - 2);
    compressed += headerLength - 2;
    consumed += headerLength;
    thisHeader = nextHeader;
  }

  if (thisHeader == IPV6_NEXT_HEADER_IPV6) {
    *compressed++ = LOWPAN_EXT_HEADER_VALUE | LOWPAN_IPV6_EID;
    // Re-enter to compress encapsulated IPv6 header.
    consumed = convertInternalTo6lowpan(header,
                                        0,
                                        0,
                                        false,
                                        &ipHeader,
                                        consumed,
                                        &compressed,
                                        isMleLoc);
  } else if (thisHeader == IPV6_NEXT_HEADER_UDP) {
    if (!emFetchUdpHeader(&ipHeader)) {
      // Invalid packet.  This could happen if the application
      // hands us something malformed to send out.  EMIPSTACK-784.
      return 0xFF;
    }
    uint8_t* control = compressed;
    *compressed++ = LOWPAN_UDP_HEADER_VALUE;
    consumed += UDP_HEADER_SIZE;
    if (isMleLoc != NULL
        && ipHeader.destinationPort == MLE_PORT) {
      *isMleLoc = true;
    }
    if (((ipHeader.sourcePort & 0xFFF0)
         == LOWPAN_ONE_NIBBLE_PORT_PREFIX)
        && ((ipHeader.destinationPort & 0xFFF0)
            == LOWPAN_ONE_NIBBLE_PORT_PREFIX)) {
      *control |= LOWPAN_UDP_SHORT_PORTS;
      *compressed++ = (((ipHeader.sourcePort & 0x000F) << 4)
                       | (ipHeader.destinationPort & 0x000F));
    } else {
      bool sourcePortCompressed = false;
      if ((ipHeader.sourcePort & 0xFF00)
          == LOWPAN_ONE_BYTE_PORT_PREFIX) {
        *control |= LOWPAN_UDP_SHORT_SOURCE_PORT;
        *compressed++ = LOW_BYTE(ipHeader.sourcePort);
        sourcePortCompressed = true;
      } else {
        emberStoreHighLowInt16u(compressed, ipHeader.sourcePort);
        compressed += 2;
      }
      if (!sourcePortCompressed  // only one port can be compressed this way
          && ((ipHeader.destinationPort & 0xFF00)
              == LOWPAN_ONE_BYTE_PORT_PREFIX)) {
        *control |= LOWPAN_UDP_SHORT_DESTINATION_PORT;
        *compressed++ = LOW_BYTE(ipHeader.destinationPort);
      } else {
        emberStoreHighLowInt16u(compressed, ipHeader.destinationPort);
        compressed += 2;
      }
    }
    uint16_t checksum = emberFetchHighLowInt16u(ipHeader.transportHeader + 6);
    emberStoreHighLowInt16u(compressed, checksum);
    compressed += 2;
  }

  *txFinger = compressed;
  return consumed;
}

uint8_t emConvertInternalTo6lowpan(PacketHeader header,
                                   EmberNodeId meshSource,
                                   EmberNodeId meshDestination,
                                   uint8_t consumed,
                                   uint8_t **txFinger,
                                   bool *isMleLoc)
{
  return convertInternalTo6lowpan(header,
                                  meshSource,
                                  meshDestination,
                                  true,
                                  NULL,
                                  consumed,
                                  txFinger,
                                  isMleLoc);
}

static uint8_t *meshHeaderPayload(PacketHeader header)
{
  uint8_t *macPayload = emMacPayloadPointer(header);
  uint8_t byte0 = macPayload[0];

  if ((byte0 & 0xF0) == LOWPAN_MESH_HIGH_NIBBLE) {
    macPayload += LOWPAN_MESH_HEADER_SIZE;
    if ((byte0 & 0x0F) == 0x0F) {
      macPayload += 1;
    }
  }
  return macPayload;
}

static uint8_t *internalIpStart(PacketHeader header)
{
  uint8_t *contents = meshHeaderPayload(header);
  uint8_t byte0 = contents[0];

  if (byte0 == LOWPAN_IPV6_HEADER_BYTE) {
    return contents + INTERNAL_IP_OVERHEAD;
  } else if ((byte0 & LOWPAN_FRAGMENT_MASK) == LOWPAN_FIRST_FRAGMENT) {
    return contents + FIRST_FRAGMENT_HEADER_LENGTH;
  } else {
    return NULL;
  }
}

bool emFetchIpv6Header(PacketHeader source, Ipv6Header *destination)
{
  uint8_t *start = internalIpStart(source);
  if (start == NULL) {
    return false;
  } else {
    fetchIpv6Header(start, destination);
    return true;
  }
}

bool emStoreIpv6Header(Ipv6Header *source, PacketHeader destination)
{
  uint8_t *destStart = internalIpStart(destination);
  if (destStart == NULL) {
    return false;
  }
  emReallyStoreIpv6Header(source, destStart);
  uint8_t *contents = meshHeaderPayload(destination);
  if (contents[0] == LOWPAN_IPV6_HEADER_BYTE) {
    emSetInternalDatagramLength(contents,
                                IPV6_HEADER_SIZE + source->ipPayloadLength);
  }
  return true;
}

bool emHasMeshHeader(PacketHeader header)
{
  return (emMacPayloadPointer(header)[0] & 0xF0) == LOWPAN_MESH_HIGH_NIBBLE;
}

bool emReadMeshHeader(PacketHeader header, MeshHeader *mesh)
{
  uint8_t *macPayload = emMacPayloadPointer(header);
  uint8_t byte0 = macPayload[0];
  if ((byte0 & 0xF0) != LOWPAN_MESH_HIGH_NIBBLE) {
    return false;
  } else if (mesh != NULL) {
    mesh->hopLimit = byte0 & 0x0F;
    uint8_t offset = 0;

    if (mesh->hopLimit == 0x0F) {
      mesh->hopLimit = macPayload[1];
      mesh->size = 6;
      offset = 2;
    } else {
      mesh->size = 5;
      offset = 1;
    }

    mesh->source = HIGH_LOW_TO_INT(macPayload[offset], macPayload[offset + 1]);
    mesh->destination = HIGH_LOW_TO_INT(macPayload[offset + 2],
                                        macPayload[offset + 3]);
  }
  return true;
}

// If the extra hopLimit byte is no longer needed after the decrement
// it will be elided when the packet is copied to the TX buffer.  See
// emMakeNextFragment in fragment.c.

void emDecrementMeshHeaderHopLimit(PacketHeader header)
{
  uint8_t *macPayload = emMacPayloadPointer(header);
  uint8_t byte0 = macPayload[0];
  assert((byte0 & 0xF0) == LOWPAN_MESH_HIGH_NIBBLE);
  assert((byte0 & 0x0F) != 0);  // can't decrement zero
  if ((byte0 & 0x0F) == 0x0F) {
    macPayload[1] -= 1;
  } else {
    macPayload[0] -= 1;
  }
}

#ifdef EMBER_TEST

// For scripted tests.

Parcel *makeIpv6Internal(uint8_t nextHeader,
                         uint8_t hopCount,
                         Bytes16 source,
                         Bytes16 destination,
                         Parcel *ipPayload)
{
  return makeMessage("11221111211ssp",
                     LOWPAN_IPV6_HEADER_BYTE,
                     0,                 // fragment offset
                     0,                 // fragment tag
                     (IPV6_HEADER_SIZE  // datagram length
                      + ipPayload->length),
                     0x60,              // version
                     0, 0, 0,           // traffic class, flow label
                     ipPayload->length,
                     nextHeader,
                     hopCount,
                     source.contents, 16,
                     destination.contents, 16,
                     ipPayload);
}

#endif
