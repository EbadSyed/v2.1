/**
 * @file network-data-tlv.h
 * @brief  Utilities for reading and writing Thread Network Data TLVs.
 *
 * This file contains utility functions for reading and writing Thread
 * Network Data TLVs.
 *
 * Unlike most Thread API functions these return their results directly
 * to the caller.
 */

#ifndef NETWORK_DATA_TLV_H
#define NETWORK_DATA_TLV_H

/** @brief The maximum size for Thread Network Data. */
#define EMBER_MAX_NETWORK_DATA_SIZE 255

#define EMBER_NETWORK_DATA_HAS_ROUTE_TEMP        0
#define EMBER_NETWORK_DATA_HAS_ROUTE_STABLE      1
#define EMBER_NETWORK_DATA_PREFIX_TEMP           2
#define EMBER_NETWORK_DATA_PREFIX_STABLE         3
#define EMBER_NETWORK_DATA_BORDER_ROUTER_TEMP    4
#define EMBER_NETWORK_DATA_BORDER_ROUTER_STABLE  5
#define EMBER_NETWORK_DATA_6LOWPAN_ID_TEMP       6
#define EMBER_NETWORK_DATA_6LOWPAN_ID_STABLE     7
#define EMBER_NETWORK_DATA_COMMISSION_TEMP       8
#define EMBER_NETWORK_DATA_COMMISSION_STABLE     9
#define EMBER_NETWORK_DATA_SERVICE_TEMP         10
#define EMBER_NETWORK_DATA_SERVICE_STABLE       11
#define EMBER_NETWORK_DATA_SERVER_TEMP          12
#define EMBER_NETWORK_DATA_SERVER_STABLE        13

#define EMBER_NETWORK_DATA_STABLE_FLAG 0x01

/** @brief Clears the 'stable' flag from a Network Data TLV type.
 *
 * @param type A Network Data TLV type.
 */
#define emberNetworkDataBaseType(type) \
  ((type) & ~EMBER_NETWORK_DATA_STABLE_FLAG)

/** @brief Returns the type of a Network Data TLV.
 *
 * @param tlv A pointer to a Network Data TLV.
 */
#define emberNetworkDataTlvType(tlv) ((tlv)[0])

/** @brief True if the Network Data TLV has the 'stable' flag set.
 *
 * @param tlv A pointer to a Network Data TLV.
 */
#define emberNetworkDataTlvIsStable(tlv) \
  ((((tlv)[0]) & EMBER_NETWORK_DATA_STABLE_FLAG) != 0)

/** @brief Returns the length of the data in a Network Data TLV.
 *
 * @param tlv A pointer to a Network Data TLV.
 */
#define emberNetworkDataTlvSize(tlv) ((tlv)[1])

/** @brief Returns a pointer to the data in a Network Data TLV.
 *
 * @param tlv A pointer to a Network Data TLV.
 */
#define emberNetworkDataTlvData(tlv) ((tlv) + 2)

/** @brief Returns a pointer to the first TLV found of the given type,
 *  or NULL if no such TLV is found.  The "stable" flag of \a type is
 *  ignored; both stable and temporary TLVs will be returned.
 *
 * @param type The type of TLV to be searched for.
 * @param start The beginning fo the block of TLVs to be searched.
 * @param end The end fo the block of TLVs to be searched.
 */
uint8_t *emberFindNetworkDataTlv(uint8_t type,
                                 const uint8_t *start,
                                 const uint8_t *end);

/** @brief Returns a pointer to the next TLV after the given one.
 *
 * @param tlv A pointer to a Network Data TLV.
 */
uint8_t *emberNetworkDataNextTlv(const uint8_t *tlv);

#define EMBER_NETWORK_DATA_BORDER_ROUTER_ENTRY_LENGTH 4

#define EMBER_NETWORK_DATA_HAS_ROUTE_ENTRY_LENGTH 3

#define EMBER_NETWORK_DATA_PREFIX_DOMAIN_OFFSET 2
#define EMBER_NETWORK_DATA_PREFIX_LENGTH_OFFSET 3
#define EMBER_NETWORK_DATA_PREFIX_BITS_OFFSET   4

/** @brief The length of the prefix in a Prefix TLV, in bits.
 *
 * @param tlv A pointer to a Network Data Prefix TLV.
 */
#define emberPrefixLengthInBits(prefixTlv) \
  ((prefixTlv)[EMBER_NETWORK_DATA_PREFIX_LENGTH_OFFSET])

/** @brief The length of the prefix in a Prefix TLV, in bytes.
 *
 * @param tlv A pointer to a Network Data Prefix TLV.
 */
#define emberPrefixLengthInBytes(prefixTlv) \
  (EMBER_BITS_TO_BYTES(emberPrefixLengthInBits(prefixTlv)))

/** @brief A pointer to the prefix in a Prefix TLV.
 *
 * @param tlv A pointer to a Network Data Prefix TLV.
 */
#define emberPrefixBits(prefixTlv) \
  ((prefixTlv) + EMBER_NETWORK_DATA_PREFIX_BITS_OFFSET)

/** @brief A pointer to the sub-TLVs in a Prefix TLV.
 *
 * @param tlv A pointer to a Network Data Prefix TLV.
 */
#define emberNetworkDataPrefixTlvHeaderSize(prefixTlv) \
  (EMBER_NETWORK_DATA_PREFIX_BITS_OFFSET + emberPrefixLengthInBytes(prefixTlv))

#endif // NETWORK_DATA_TLV_H
