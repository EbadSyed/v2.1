/*
 * File: ip-address.c
 * Description: IPv6 address utilities
 * Author(s): Richard Kelsey, Matteo Paris
 *
 * Copyright 2013 Silicon Laboratories, Inc.                                *80*
 */

// For strnlen(3) in glibc.
#define _GNU_SOURCE

#include "core/ember-stack.h"
#include "address-management.h"
#include "rip.h"
#include "ip-header.h"
#include "ip-address.h"
#include <string.h>

// Note: The All Thread Nodes multicast addresses are filled in
// once the ULA prefix is known. They are RFC3306 addresses with
// the format FF32:40:<ula prefix>::1 and FF33:40:<ula prefix>::1
#define RFC3306_NETWORK_PREFIX_OFFSET 4

// See RFC 4291 (IPv6 Addressing Architecture)

//----------------------------------------------------------------
// Mesh local identifier

uint8_t emMeshLocalIdentifier[8];

bool emberIsMeshLocalIdentifier(const uint8_t *identifier)
{
  return (MEMCOMPARE(identifier, emMeshLocalIdentifier, 8) == 0);
}

void emSetMeshLocalIdentifierFromLongId(const uint8_t *longId)
{
  emLongIdToInterfaceId(longId, emMeshLocalIdentifier);
}

//----------------------------------------------------------------
// Interface Ids

// This six bytes prepended to a short ID to get an inteface ID.
const uint8_t emShortInterfaceIdPrefix[6] = { 0x00, 0x00, 0x00, 0xFF, 0xFE, 0x00 };

static bool isShortInterfaceId(const uint8_t *interfaceId)
{
  return MEMCOMPARE(emShortInterfaceIdPrefix, interfaceId, 6) == 0;
}

void emStoreShortInterfaceId(uint16_t id, uint8_t *target)
{
  MEMCOPY(target, emShortInterfaceIdPrefix, 6);
  emberStoreHighLowInt16u(target + 6, id);
}

bool emGleanShortIdFromInterfaceId(const uint8_t *interfaceId, uint16_t* shortId)
{
  if (isShortInterfaceId(interfaceId)) {
    if (shortId != NULL) {
      *shortId = emberFetchHighLowInt16u(interfaceId + 6);
    }
    return true;
  }
  return false;
}

bool emIsLocalMl64InterfaceId(const uint8_t* interfaceId)
{
  return MEMCOMPARE(interfaceId, emMeshLocalIdentifier, 8) == 0;
}

bool emIsLocalLl64InterfaceId(const uint8_t* interfaceId)
{
  uint8_t unmodified[8];
  emInterfaceIdToLongId(interfaceId, unmodified);
  return MEMCOMPARE(unmodified, emMacExtendedId, 8) == 0;
}

// Bit 7 of a mesh local identifier (longId) is set to 0 for globally-unique
// addresses (those that have been allocated by some infallible organizational
// system) and 1 for locally-administered ones.  The folks that came up with
// IPv6 reversed the meaning of this bit when an EUI64 is used as an interface
// ID.  The notion was that this would allow sysadmins to type in a local
// address as FE80::xxxx instead of having to type FE80::0200:0:0:xxxx.  So we
// have to flip the bit back and forth. On top of this, 802.15.4 and IPv6 store
// the bytes in the opposite order.

void emLongIdToInterfaceId(const uint8_t *longId, uint8_t *target)
{
  emberReverseMemCopy(target, longId, 8);
  target[0] ^= 0x02;
}

void emInterfaceIdToLongId(const uint8_t *interfaceId, uint8_t *longId)
{
  emberReverseMemCopy(longId, interfaceId, 8);
  longId[7] ^= 0x02;
}

bool emIsLocalShortInterfaceId(const uint8_t *interfaceId)
{
  EmberNodeId shortId;
  EmberNodeId myShortId = emberGetNodeId();
  return (emGleanShortIdFromInterfaceId(interfaceId, &shortId)
          && myShortId != EM_USE_LONG_ADDRESS
          && myShortId == shortId);
}

bool emIsHostToNcpAddress(const uint8_t *address)
{
  EmberNodeId shortId;
  return (emIsGp16(address, &shortId)
          && shortId == 0xFFFE);
}

//----------------------------------------------------------------
// Addresses using the default global prefix.

// GP16 = global prefix 16, the IPv6 address formed from the default global
// prefix, the short interface ID prefix, and the short MAC ID.

bool emStoreGp16(uint16_t id, uint8_t *target)
{
  emStoreShortInterfaceId(id, target + DEFAULT_PREFIX_BYTES);
  return emStoreDefaultGlobalPrefix(target);
}

bool emIsGp16(const uint8_t *address, uint16_t *returnShort)
{
  return (emIsDefaultGlobalPrefix(address)
          && emGleanShortIdFromInterfaceId(address + DEFAULT_PREFIX_BYTES,
                                           returnShort));
}

bool emStoreLocalMl64(uint8_t *target)
{
  MEMCOPY(target + DEFAULT_PREFIX_BYTES, emMeshLocalIdentifier, 8);
  return emStoreDefaultGlobalPrefix(target);
}

bool emIsGp64(const uint8_t *address)
{
  return (emIsDefaultGlobalPrefix(address)
          && !isShortInterfaceId(address + 8));
}

//----------------------------------------------------------------
// Link local addresses

// The link-local prefix.
const Bytes8 emFe8Prefix = { { 0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

void emStoreLongFe8Address(const uint8_t *eui64, uint8_t *target)
{
  MEMCOPY(target, emFe8Prefix.contents, 8);
  emLongIdToInterfaceId(eui64, target + 8);
}

bool emIsFe8Address(const uint8_t *address)
{
  return (MEMCOMPARE(address, emFe8Prefix.contents, 8) == 0);
}

bool emIsLl64(const uint8_t *address)
{
  return (emIsFe8Address(address)
          && !isShortInterfaceId(address + 8));
}

bool emIsLocalLl64(const uint8_t *address)
{
  return (emIsFe8Address(address)
          && emIsLocalLl64InterfaceId(address + 8));
}

bool emStoreDefaultIpAddress(uint8_t *target)
{
  return (emberGetNodeId() < EM_USE_LONG_ADDRESS
          ? emStoreGp16(emberGetNodeId(), target)
          : emStoreLocalMl64(target));
}

bool emIsUnicastForMe(const uint8_t *address)
{
  return (emIsLocalIpAddress(address)
          || emIsLoopbackAddress(address));
}

//----------------------------------------------------------------
// Multicast

// See http://www.iana.org/assignments/ipv6-multicast-addresses/

// FF01::XX = interface-local scope
// FF02::XX = link-local scope
// FF03::XX = subnet-local scope
// FF05::XX = site-local scope
//
// FF0X::1 = all nodes address
// FF0X::2 = all routers address
// FF0X::1A = all RPL nodes address
// FF0X::FB = mDNSv6 address

// FF32:40:<ula prefix>::1 - Link local All Thread Nodes multicast
// FF33:40:<ula prefix>::1 - Realm local All Thread Nodes multicast

bool emIsFf32MulticastAddress(const uint8_t *address)
{
  return (address[0] == 0xFF
          && address[1] == 0x32
          && address[2] == 0x00
          && address[3] == 0x40
          && emIsDefaultGlobalPrefix(address + RFC3306_NETWORK_PREFIX_OFFSET));
}

bool emIsFf33MulticastAddress(const uint8_t *address)
{
  return (address[0] == 0xFF
          && address[1] == 0x33
          && address[2] == 0x00
          && address[3] == 0x40
          && emIsDefaultGlobalPrefix(address + RFC3306_NETWORK_PREFIX_OFFSET));
}

bool emIsFf01MulticastAddress(const uint8_t *address)
{
  return (address[0] == 0xFF && address[1] == 0x01);
}

bool emIsFf02MulticastAddress(const uint8_t *address)
{
  return (address[0] == 0xFF && address[1] == 0x02);
}

bool emIsFf03MulticastAddress(const uint8_t *address)
{
  return (address[0] == 0xFF && address[1] == 0x03);
}

// Once we get enough of these it may become more efficient to create
// them on the fly.

// Link local - all nodes.
const Bytes16 emFf02AllNodesMulticastAddress =
{ { 0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } };

// Link local - all routers.
const Bytes16 emFf02AllRoutersMulticastAddress =
{ { 0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 } };

// Realm (mesh) local - all nodes.
const Bytes16 emFf03AllNodesMulticastAddress =
{ { 0xFF, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } };

// Realm (mesh) local - all routers.
const Bytes16 emFf03AllRoutersMulticastAddress =
{ { 0xFF, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 } };

// Site local - all nodes.
const Bytes16 emFf05AllNodesMulticastAddress =
{ { 0xFF, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } };

// Site local - all routers.
const Bytes16 emFf05AllRoutersMulticastAddress =
{ { 0xFF, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 } };

// Link local - all RPL nodes.
const Bytes16 emFf02AllRplNodesMulticastAddress =
{ { 0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1A } };

// RFC 3315 has a All_DHCP_Relay_Agents_and_Servers (FF02::1:2).
// We need the same thing but with the scope one tick larger.

const Bytes16 emFf03AllDhcpMulticastAddress =
{ { 0xFF, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03 } };

// Note: The All Thread Nodes multicast addresses are filled in
// once the ULA prefix is known. They are RFC3306 addresses with
// the format FF32:40:<ula prefix>::1 and FF33:40:<ula prefix>::1

// Link local - all Thread nodes (see note above)
Bytes16 emFf32AllThreadNodesMulticastAddress =
{ { 0xFF, 0x32, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } };

// Realm (mesh) local - all Thread nodes (see note above)
Bytes16 emFf33AllThreadNodesMulticastAddress =
{ { 0xFF, 0x33, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } };

// Realm (mesh) local - all routers
Bytes16 emFf33AllThreadRoutersMulticastAddress =
{ { 0xFF, 0x33, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 } };

void emSetAllThreadNodesMulticastAddresses(const uint8_t *meshLocalPrefix)
{
  MEMCOPY((emFf32AllThreadNodesMulticastAddress.contents
           + RFC3306_NETWORK_PREFIX_OFFSET),
          meshLocalPrefix, 8);
  MEMCOPY((emFf33AllThreadNodesMulticastAddress.contents
           + RFC3306_NETWORK_PREFIX_OFFSET),
          meshLocalPrefix, 8);
  MEMCOPY((emFf33AllThreadRoutersMulticastAddress.contents
           + RFC3306_NETWORK_PREFIX_OFFSET),
          meshLocalPrefix, 8);
}

static bool clearAddressBits(uint8_t prefixBits, EmberIpv6Address *dst)
{
  if (EMBER_IPV6_BITS < prefixBits) {
    return false;
  } else if (prefixBits < EMBER_IPV6_BITS) {
    const uint8_t lastPrefixByte = prefixBits / 8;
    const uint8_t lastPrefixBit = 8 - (prefixBits - 8 * lastPrefixByte);
    dst->bytes[lastPrefixByte] &= ~(BIT(lastPrefixBit) - 1);
    MEMSET(dst->bytes + lastPrefixByte + 1,
           0,
           EMBER_IPV6_BYTES - lastPrefixByte - 1);
  }
  return true;
}

// This function formats IPv6 fields per the recommendations of RFC5952.  In
// particular, leading zeros within a field are omitted and lowercase is used
// for the letters a through f.  The destination is assumed to be large enough.
static uint8_t fieldToString(uint16_t src, uint8_t *dst)
{
  uint8_t *finger = dst;
  bool writing = false;
  uint8_t i;
  for (i = 0; i < 4; i++) {
    uint8_t nibble = (uint8_t)(((src << (i * 4)) & 0xF000) >> 12);
    if (writing || nibble != 0 || i == 3) {
      *finger++ = (nibble + (nibble < 10 ? '0' : 'a' - 10));
      writing = true;
    }
  }
  return (finger - dst);
}

// This function formats IPv6 addresses per the recommendations of RFC5952.  In
// particular, the longest run of at least two consecutive zero fields is
// shortened to ::, with runs closer to the beginning used in case of a tie.
// The string is terminated with a NUL character.
static bool bytesToString(const EmberIpv6Address *src,
                          uint8_t prefixBits,
                          bool isPrefix,
                          uint8_t *dst,
                          size_t dstSize)
{
  // A destination is required.
  if (dst == NULL) {
    return false;
  }

  // Pretend a NULL address is all zeros; otherwise, use the source address.
  EmberIpv6Address tmpSrc = { { 0 } };
  if (src != NULL) {
    MEMCOPY(&tmpSrc, src, sizeof(EmberIpv6Address));
  }

  // If we are formating a prefix, zero the address bits.
  if (isPrefix && !clearAddressBits(prefixBits, &tmpSrc)) {
    return false;
  }

  // Find the longest, left-most run of at least two consecutive zero fields.
  uint8_t currentStart = UINT8_MAX, currentLength = 0;
  uint8_t maxStart = UINT8_MAX, maxLength = 0;
  uint8_t i;
  for (i = 0; i < EMBER_IPV6_FIELDS; i++) {
    uint16_t field = emberFetchHighLowInt16u(tmpSrc.bytes + i * 2);
    if (field == 0) {
      if (currentStart == UINT8_MAX) {
        currentStart = i;
      }
      currentLength++;
    } else if (currentStart != UINT8_MAX) {
      if (maxStart == UINT8_MAX || maxLength < currentLength) {
        maxStart = currentStart;
        maxLength = currentLength;
      }
      currentStart = UINT8_MAX;
      currentLength = 0;
    }
  }
  if (currentStart != UINT8_MAX
      && (maxStart == UINT8_MAX || maxLength < currentLength)) {
    maxStart = currentStart;
    maxLength = currentLength;
  }
  if (maxLength == 1) {
    maxStart = UINT8_MAX;
  }

  // Format the address, using :: for the zeros found previously.
  uint8_t tmpDst[EMBER_IPV6_PREFIX_STRING_SIZE], *finger = tmpDst;
  for (i = 0; i < EMBER_IPV6_FIELDS; i++) {
    if (i < maxStart || maxStart + maxLength <= i) {
      uint16_t field = emberFetchHighLowInt16u(tmpSrc.bytes + i * 2);
      if (i != 0) {
        *finger++ = ':';
      }
      finger += fieldToString(field, finger);
    } else if (maxStart == i) {
      *finger++ = ':';
    }
  }
  if (maxStart + maxLength == EMBER_IPV6_FIELDS) {
    *finger++ = ':';
  }

  // If we are formating a prefix, append it (e.g., /64).
  if (isPrefix) {
    *finger++ = '/';
    uint8_t remainder = prefixBits;
    if (100 <= prefixBits) {
      *finger++ = '1';
      remainder -= 100;
    }
    if (100 <= prefixBits || 10 <= remainder) {
      *finger++ = (remainder / 10) + '0';
      remainder %= 10;
    }
    *finger++ = remainder + '0';
  }

  // Terminate with a NUL.
  *finger++ = '\0';

  // If the destination is big enough, copy to it.
  if (finger - tmpDst <= dstSize) {
    MEMCOPY(dst, tmpDst, finger - tmpDst);
    return true;
  } else {
    return false;
  }
}

// This is like inet_ntop(AF_INET6, ...), except it treats a NULL src as all
// zeros (i.e., "::") and does not do IPv4-in-IPv6 formatting.
bool emberIpv6AddressToString(const EmberIpv6Address *src,
                              uint8_t *dst,
                              size_t dstSize)
{
  return bytesToString(src, 0, false, dst, dstSize); // address
}

// This is also like inet_ntop(AF_INET6, ...), except it chops off the address
// bits from the address and appends a prefix (e.g., "/64").  There's no
// standard equivalent to this, as far as I know.
bool emberIpv6PrefixToString(const EmberIpv6Address *src,
                             uint8_t srcPrefixBits,
                             uint8_t *dst,
                             size_t dstSize)
{
  return bytesToString(src, srcPrefixBits, true, dst, dstSize); // prefix
}

enum {
  EXPECTING_WORD      = 0,
  EXPECTING_PADDING   = 1,
  EXPECTING_DELIMITER = 2,
};

static bool stringToBytes(const uint8_t *src,
                          EmberIpv6Address *dst,
                          uint8_t *prefixBits,
                          bool isPrefix)
{
  // A destination is required.
  if (dst == NULL || (isPrefix && prefixBits == NULL)) {
    return false;
  }

  // Pretend a NULL string is all zeros.
  if (src == NULL) {
    MEMSET(dst, 0, sizeof(EmberIpv6Address));
    if (isPrefix) {
      *prefixBits = 0;
    }
    return true;
  }

  // Copy the string, so it can be modified to more easily separate the address
  // from the prefix (e.g., /64).
  size_t srcLength = strnlen((const char *)src, EMBER_IPV6_PREFIX_STRING_SIZE);
  if (srcLength >= EMBER_IPV6_PREFIX_STRING_SIZE) {
    return false;
  }
  uint8_t tmpSrc[EMBER_IPV6_PREFIX_STRING_SIZE] = { 0 };
  MEMCOPY(tmpSrc, src, srcLength);

  // Prefixes must have a prefix (e.g., /64).
  uint16_t tmpPrefixBits = 0;
  if (isPrefix) {
    uint8_t *finger = (uint8_t *)strrchr((const char *)tmpSrc, '/');
    if (finger == NULL) {
      return false;
    }
    *finger++ = '\0';
    if (*finger == '\0') {
      return false;
    }
    for (; *finger != '\0'; finger++) {
      uint8_t digit = emberHexToInt(*finger);
      if (digit <= 9) {
        tmpPrefixBits *= 10;
        tmpPrefixBits += digit;
        if (EMBER_IPV6_BITS < tmpPrefixBits) {
          return false;
        }
      } else {
        return false;
      }
    }
  }

  EmberIpv6Address tmpDst = { { 0 } };
  uint8_t padIndex = 0;
  uint8_t index = 0;
  bool padded = false;
  uint16_t word = 0;
  uint8_t wordChars = 0;
  uint8_t state = (tmpSrc[0] == ':' ? EXPECTING_PADDING : EXPECTING_WORD);
  size_t i;
  for (i = 0; tmpSrc[i] != '\0'; i++) {
    uint8_t ch = tmpSrc[i];
    bool isColon = (ch == ':');
    uint8_t value = emberHexToInt(ch);
    bool isLast = (tmpSrc[i + 1] == '\0');
    bool nextIsColon = (tmpSrc[i + 1] == ':');
    //fprintf(stderr,
    //        "i:%lu state:%d ch:%c colon:%d nextIsColon:%d value:%d index:%d\n",
    //        i, state, ch, isColon, nextIsColon, value, index);
    if ((!isColon && value > 15)
        || index == EMBER_IPV6_BYTES) {
      return false;
    }

    switch (state) {
      case EXPECTING_WORD:
        if (isColon) {
          return false;
        } else {
          if (wordChars == 4) {
            return false;
          }
          word <<= 4;
          word += value;
          wordChars += 1;
        }
        if (isLast || nextIsColon) {
          emberStoreHighLowInt16u(tmpDst.bytes + index, word);
          index += 2;
          state = EXPECTING_DELIMITER;
        }
        break;

      case EXPECTING_PADDING:
      case EXPECTING_DELIMITER: {
        if (!isColon
            || isLast // Trailing colon not allowed.
            || (padded && nextIsColon) // Only one padding allowed.
            || (state == EXPECTING_PADDING && !nextIsColon)) {
          return false;
        } else if (nextIsColon) {
          i += 1;
          padded = true;
          padIndex = index;
        }
        word = 0;
        wordChars = 0;
        state = EXPECTING_WORD;
        break;
      }
    }
  }
  if (padded) {
    uint8_t padBytes = EMBER_IPV6_BYTES - index;
    uint8_t suffixLength = index - padIndex;
    if (padBytes == 0) {
      return false;  // Nothing to pad.
    }
    MEMMOVE(tmpDst.bytes + padIndex + padBytes,
            tmpDst.bytes + padIndex,
            suffixLength);
    MEMSET(tmpDst.bytes + padIndex, 0, padBytes);
  } else if (index != EMBER_IPV6_BYTES) {
    return false;
  }

  if (isPrefix) {
    if (!clearAddressBits(tmpPrefixBits, &tmpDst)) {
      return false;
    }
    *prefixBits = (uint8_t)tmpPrefixBits;
  }
  MEMCOPY(dst, &tmpDst, sizeof(EmberIpv6Address));

  return true;
}

// This is like inet_pton(AF_INET6, ...), except it treats a NULL src as all
// zeros (i.e., "::") and does not do IPv4-in-IPv6 formatting.
bool emberIpv6StringToAddress(const uint8_t *src, EmberIpv6Address *dst)
{
  return stringToBytes(src, dst, NULL, false); // address
}

// This is also like inet_pton(AF_INET6, ...), except it expects a prefix
// (e.g., "/64") at the end of the string and chops off the address bits from
// the address.  There's no standard equivalent to this, as far as I know.
bool emberIpv6StringToPrefix(const uint8_t *src,
                             EmberIpv6Address *dst,
                             uint8_t *dstPrefixBits)
{
  return stringToBytes(src, dst, dstPrefixBits, true); // prefix
}

// This will return whether an IPv6 address is null/all zeros.
bool emberIsIpv6UnspecifiedAddress(const EmberIpv6Address *address)
{
  if (!address) {
    return false;
  }

  return emMemoryByteCompare(address->bytes, sizeof(EmberIpv6Address), 0);
}

// The loopback address just has the low bit set.
bool emberIsIpv6LoopbackAddress(const EmberIpv6Address *address)
{
  if (!address) {
    return false;
  }

  return (emIsMemoryZero(address->bytes, (EMBER_IPV6_BYTES - 1)) && address->bytes[EMBER_IPV6_BYTES - 1] == 1);
}
