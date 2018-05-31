/*
 * File: network-data.h
 * Description: Shared network data, distributed via MLE.
 *
 * Copyright 2013 by Silicon Laboratories. All rights reserved.             *80*
 */

#ifndef NETWORK_DATA_H
#define NETWORK_DATA_H

// Data in NWK_DATA_TLVs is encoded as type-length-value (there's
// a new idea!).  One byte of type and one byte of length, as we don't want
// this getting too large.

#define NWK_DATA_STABLE_FLAG 0x01

#define emNetworkDataStableType(type)    (((type) << 1) | NWK_DATA_STABLE_FLAG)
#define emNetworkDataTemporaryType(type) ((type) << 1)

#define emNetworkDataTypeByte(tlv) ((tlv)[0])
#define emNetworkDataIsStable(tlv) ((((tlv)[0]) & NWK_DATA_STABLE_FLAG) != 0)
#define emNetworkDataType(tlv)     (((tlv)[0]) >> 1)

uint16_t emNetworkDataSize(const uint8_t *finger);
#define emNetworkDataPointer(finger) ((finger) + emNetworkDataTlvOverhead((finger)))
void emSetNetworkDataSize(uint8_t *finger, uint16_t length);
uint16_t emNetworkDataTlvSize(const uint8_t *finger);
uint8_t emNetworkDataTlvOverhead(const uint8_t *finger);
uint8_t *emFindTlv(uint8_t type, uint8_t *finger, const uint8_t *end);

#define emFindFirstPrefixTlv(buffer) \
  (emFindNextTlv((buffer), NWK_DATA_PREFIX, NULL))
#define emFindNextPrefixTlv(buffer, previousTlv) \
  (emFindNextTlv((buffer), NWK_DATA_PREFIX, (previousTlv)))
uint8_t *emFindNextTlv(Buffer buffer, uint8_t baseType, uint8_t *prefixTlv);

uint8_t *emNextTlv(uint8_t *tlv);

#define NWK_DATA_HAS_ROUTE     0 // node IDs + flags
#define NWK_DATA_PREFIX        1 // prefix + subTLVs
#define NWK_DATA_BORDER_ROUTER 2 // node IDs + flags (2 bytes worth)
#define NWK_DATA_6LOWPAN_ID    3 // 6LoWPAN prefix ID + flags + lifetime
#define NWK_DATA_COMMISSION    4 // commissioner info
#define NWK_DATA_SERVICE       5 // server TLVs
#define NWK_DATA_SERVER        6 // node ID + data

// network fragment identifier
#define NWK_DATA_ISLAND_ID_OFFSET            0
// network data version
#define NWK_DATA_VERSION_OFFSET              5
// stable network data version
#define NWK_DATA_STABLE_VERSION_OFFSET       6
// leader's RIP router ID
#define NWK_DATA_LEADER_RIP_OFFSET           7
// leader size found in stack/include/ember-types.h

#define BORDER_ROUTER_ENTRY_LENGTH 4
#define HAS_ROUTE_ENTRY_LENGTH 3

#define emBorderRouterFlags(tlv) (emberFetchHighLowInt16u((tlv) + 2))

// Nested TLVs a la DHCPv6.
// Prefix TLV has the prefix, but can also include:
//   - 6LoWPAN ID, flags, and lifetime
//   - router TLV, with node IDs of routers
//   - DHCPv6 TLV, with node IDs of devices that allocate this prefix
//
// HAS_ROUTE TLV on its own gives default routers.
//
// The goal is to provide:
//   - the same prefix information as ND (this is in the context-table data)
//   - the same external routing information as RIP
//   - the same server data as DHCPv6

// Prefix data, followed by sub-TLVs.  The sub-TLVs come after the prefix,
// which is (EMBER_BITS_TO_BYTES(length) bytes.
#define NWK_DATA_PREFIX_DOMAIN_OFFSET 2  // provisioning domain
#define NWK_DATA_PREFIX_LENGTH_OFFSET 3  // prefix length in bits, 1-128
#define NWK_DATA_PREFIX_BITS_OFFSET   4  // prefix itself

#define prefixTlvHeaderSize(data) \
  (NWK_DATA_PREFIX_BITS_OFFSET    \
   + EMBER_BITS_TO_BYTES((data)[NWK_DATA_PREFIX_LENGTH_OFFSET]))

uint8_t *emFindInPrefixTlv(uint8_t *prefixTlv, uint8_t innerType);

// 6LOWPAN_ID has two bytes
// ID in low nibble, flags in high nibble
#define NWK_DATA_6LOWPAN_ID_ID_OFFSET       2
#define NWK_DATA_6LOWPAN_ID_LENGTH_OFFSET   3
#define NWK_DATA_6LOWPAN_ID_SIZE            4

// Service data, followed by sub-TLVs.
#define NWK_DATA_SERVICE_FLAGS_OFFSET             2
#define NWK_DATA_SERVICE_THREAD_LENGTH_OFFSET     3
#define NWK_DATA_SERVICE_ENTERPRISE_NUMBER_OFFSET 3
#define NWK_DATA_SERVICE_OTHER_LENGTH_OFFSET      7
#define NWK_DATA_SERVICE_THREAD_FLAG 0x80
#define NWK_DATA_SERVICE_ID_MASK     0x0F

uint8_t emServiceTlvHeaderSize(const uint8_t *tlv);

// From the Thread spec.
#define MAX_NETWORK_DATA_SIZE 255

// Holds the contents of the newest NWK_DATA_TLV received.
extern Buffer emNetworkData;
// Handy utility for getting a pointer to the network data.  Length is 0xFFFF
// if there is no network data (it is possible to have network data with length
// zero).
uint8_t *emGetNetworkData(uint16_t *lengthLoc);
// Copies the current network data to 'to', returning the number of
// bytes copied.
uint16_t emCopyNetworkData(uint8_t *to, bool stableOnly);

extern bool emAmLeader;

typedef enum {
  NWK_DATA_OTHER,
  NWK_DATA_NEWER,
  NWK_DATA_OLDER,
  NWK_DATA_OTHER_FRAGMENT
} NetworkDataStatus;

void emNoteNetworkDataVersion(uint8_t *source,
                              uint8_t neighborIndex,
                              EmberNodeId macShortSource,
                              const uint8_t *leaderData);
void emUpdateNetworkData(Buffer mleMessage,
                         uint8_t *mleLeaderDataTlv,
                         uint8_t *mleNetworkDataTlv,
                         uint8_t *ipSource);

#ifdef EMBER_TEST
// Allow test code to access this directly.
extern uint8_t localServices;   // bitmask; default is no services
#endif

#define emFindNetworkDataTlv(type) (emFindNextNetworkDataTlv((type), NULL))
uint8_t *emFindNetworkDataSubTlv(uint8_t *outerTlv,
                                 uint8_t outerHeaderSize,
                                 uint8_t innerType);
uint8_t *emFindNextNetworkDataTlv(uint8_t type, uint8_t *start);
const uint8_t *emNetworkDataTlvHasNode(const uint8_t* tlv, EmberNodeId node);
uint8_t *emGetNetworkDataCommissionTlv(void);

// Return false if there is insufficient buffer space.
bool emModifyLeaderNetworkData(uint8_t *tlv, uint8_t tlvLength);

// Make the TLV have the specified length.  If the length is 0xFFFF,
// remove 'tlv' instead.  (A length of zero means a TLV that doesn't
// have any data).  Returns a pointer to the updated TLV, or NULL for
// deletions or if there wasn't enough heap space to make the change.
// Deletion or making a TLV smaller always succeeds.

uint8_t *emSetNetworkDataTlvLength(uint8_t *tlv, uint16_t tlvDataLength);
#define emRemoveNetworkDataTlv(tlv) (emSetNetworkDataTlvLength((tlv), -1))

void emRemoveNetworkDataSubTlv(uint8_t *outerTlv, uint8_t *innerTlv);
bool emReplaceNetworkData(uint8_t *newData, uint16_t length);

void emRemoveServerFromNetworkData(uint8_t routerId);

uint8_t *emFindPrefixSubTlv(Buffer data,
                            const uint8_t *prefix,
                            uint8_t prefixLengthInBits,
                            uint8_t type,
                            bool exactMatch);

EmberNodeId emFindNearestDhcpServer(const uint8_t *prefix, uint8_t prefixLengthInBits, bool match);
EmberNodeId emFindNearestDnsGateway(const uint8_t *sourceAddress);
bool emIsOnMeshPrefix(const uint8_t *prefix);
EmberNodeId emFindNearestGateway(const uint8_t *ipDestination,
                                 const uint8_t *ipSource);

EmberNodeId emGetLeaderNodeId(void);
void emInitializeLeaderNetworkData(void);
void emSetLeaderData(const uint8_t *newLeaderData);
bool emIsLeaderDataBetter(const uint8_t *leaderData);
uint8_t *emLocalIslandId(void);
uint8_t emGetAssignedRipIdSequence(void);
void emNoteAssignedRipIdChange(void);
// Returns false if our local data does not agree with what's in emNetworkData.
bool emVerifyLocalNetworkData(void);
void emProcessNetworkDataChangeRequest(EmberNodeId serverNodeId,
                                       EmberNodeId oldServerNodeId,
                                       uint8_t *data,
                                       uint16_t dataLength,
                                       bool removeChildren);
bool emUpdateLocalServerData(Buffer *bufferLoc,
                             EmberNodeId server,
                             uint8_t *newTlvs,
                             uint8_t newTlvsLength,
                             bool *changeLoc);
void emAddCommissionTlvToNetworkData(uint8_t *tlv);
void emStartNetworkDataVersionEvent(void);
void emAnnounceNewNetworkData(bool incrementStableDataVersion);

extern bool emHaveLeader;
#ifdef EMBER_TEST
extern uint8_t leaderData[];              // HIDDEN
#endif
void emAddLeaderData(uint8_t **toLoc);
bool emLeaderDataMatches(const uint8_t *networkDataTlv);

void incrementLeaderDataVersionNumbers(void);

void emNetworkDataInit(void);
void emStopNetworkData(void);

#endif // NETWORK_DATA_H
