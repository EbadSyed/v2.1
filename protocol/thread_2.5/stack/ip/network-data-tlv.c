/**
 * file: network-data-tlv.c
 * Utilities for reading and writing Thread Network Data TLVs.
 *
 * This file contains utility functions for reading and writing Thread
 * Network Data TLVs.
 *
 * Unlike most Thread API functions these return their results directly
 * to the caller.  They have no internal state and can be called from
 * any thread, so there are no emApi... versions.
 */

#include PLATFORM_HEADER
#include "stack/include/network-data-tlv.h"

// ignores 'type's stable flag
uint8_t *emberFindNetworkDataTlv(uint8_t type,
                                 const uint8_t *start,
                                 const uint8_t *end)
{
  type = emberNetworkDataBaseType(type);
  for (; start < end; start = emberNetworkDataNextTlv(start)) {
    if (type == emberNetworkDataBaseType(emberNetworkDataTlvType(start))) {
      return (uint8_t *) start;
    }
  }
  return NULL;
}

uint8_t *emberNetworkDataNextTlv(const uint8_t *tlv)
{
  return (uint8_t *) (tlv + 2 + emberNetworkDataTlvSize(tlv));
}
/*
   uint8_t *emberFindNextPrefixTlv(const uint8_t *start,
                                const uint8_t *end,
                                const uint8_t *prefix,
                                uint8_t prefixLengthInBits,
                                bool exactMatch)
   {
   uint8_t *prefixTlv;
   for (prefixTlv = emberFindNetworkDataTlv(EMBER_NETWORK_DATA_PREFIX_TEMP,
                                           start,
                                           end);
       prefixTlv != NULL;
       prefixTlv = emberFindNetworkDataTlv(EMBER_NETWORK_DATA_PREFIX_TEMP,
                                           prefixTlv,
                                           end)) {
    if ((! exactMatch
 || prefixTlv[NWK_DATA_PREFIX_LENGTH_OFFSET] == prefixLengthInBits)
        && (emberPrefixMatchLength(prefix,
                                   prefixLengthInBits,
                                   prefixTlv + NWK_DATA_PREFIX_BITS_OFFSET,
                                   prefixLengthInBits)
            == prefixLengthInBits)) {
      return prefix;
    }
   }
   return NULL;
   }

   bool emberHaveRoute(const uint8_t *networkData,
                    uint16_t networkDataLength,
                    const uint8_t *prefix,
                    uint8_t prefixLengthInBits)
   {
   if (prefixLengthInBits == 0) {
    return emberHaveDefaultRoute(networkData, networkDataLength);
   }
   }

   bool emberHaveDefaultRoute(const uint8_t *networkData,
                           uint16_t networkDataLength)
   {
   }

   uint8_t emberPrefixMatchLength(const uint8_t *prefixA,
                               uint8_t prefixLengthInBitsA,
                               const uint8_t *prefixB,
                               uint8_t prefixLengthInBitsB)
   {
   }

 */
