/*
 * File: address-cache.h
 * Description: Cache mapping eui64s to 16-bit addresses.
 * Author(s): Richard Kelsey, richard.kelsey@silabs.com
 *
 * Copyright 2013 by Silicon Laboratories. All rights reserved.             *80*
 */

#ifndef ADDRESS_CACHE_H
#define ADDRESS_CACHE_H

// To keep the fields a bit smaller, this is limited to 256 entries.
// That's plenty large enought given that it uses linear search to
// look up addresses.  If there anything like that number of entries
// a different implementation will be needed.

// This will have to include different prefixes as well.

typedef struct {
  uint8_t iid[8];
  EmberNodeId shortId;  // 0xFFFF = unused, 0xFFFE = not known
  uint8_t context;        // 6LoWPAN context for the prefix
  uint16_t key;           // retrieval key
  uint8_t flags;          // flags for discovery state, dhcp, discovery failures
  uint8_t head;           // least-recently-used queue
  uint8_t tail;
  uint32_t lastTransactionSeconds;
} AddressData;

#define DISCOVERY_STATE  0x03 // bottom two bits, don't move
#define DISCOVERY_FAILURES_MASK 0xF0 // top nibble

// Discovery states
#define DISCOVERY_IDLE    0
#define DISCOVERY_PENDING 1
#define DISCOVERY_ACTIVE  2
#define DISCOVERY_FAILED  3

#define discoveryState(data) ((data)->flags & DISCOVERY_STATE)
#define setDiscoveryState(data, state) \
  (data)->flags = (((data)->flags & ~DISCOVERY_STATE) | (state))
#define discoveryFailures(data) \
  (((data)->flags & DISCOVERY_FAILURES_MASK) >> 4)
#define setDiscoveryFailures(data, count) \
  (data)->flags = (((data)->flags & ~DISCOVERY_FAILURES_MASK) | ((count) << 4))

// Multicasts typically can take anywhwere from 5 to a few hundred milliseconds
// to complete.  To be responsive but not waste cycles the retry queue code
// checks back with an exponentially increasing delay. Total checking duration:
// 23 * (2^7 - 1) = 2921 ms, just less than ADDRESS_QUERY_TIMEOUT (3000).
#define ADDRESS_DISCOVERY_INITIAL_RETRY_WAIT_MS 23
#define ADDRESS_DISCOVERY_RETRY_CHECK_COUNT 6

// Thread specifies 3 seconds for timing out address discoveries, which is long
// given how fast thread multicasts are now.  Also you have to wait a minimum
// of 15 seconds after that to try again, so a failed discovery is costly.
#define ADDRESS_QUERY_TIMEOUT_MS 3000
#define ADDRESS_QUERY_INITIAL_RETRY_DELAY_SEC 15
#define ADDRESS_QUERY_MAX_RETRY_DELAY_SEC 28800  // 8 hours

// Not an actual cap on the number of failures, just a cap to keep the exponent
// from blowing up in the delay calculation.  15 * 2^11 is bigger than 8 hours.
#define ADDRESS_QUERY_MAX_FAILURE_COUNT 11

extern Vector emAddressCache;   // so someone can trace it

// Three possible results when trying to find the next hop.
typedef enum {
  DROP,
  DELAYED,
  SEND
} NextHopResult;

void emAddressCacheInit(void);
AddressData *emGetAddressData(uint16_t key);
AddressData *emAddAddressData(uint8_t context,
                              const uint8_t *iid,
                              EmberNodeId shortId,
                              bool update);
void emClearAddressCache(void);
void emClearAddressCacheId(EmberNodeId id);
NextHopResult emGetShortAddress(uint8_t context,
                                const uint8_t *iid,
                                EmberNodeId *resultLoc,
                                bool discover);
void emDiscoverShortAddress(uint16_t cacheIndex);
bool emNoteIdMapping(uint8_t context,
                     const uint8_t *iid,
                     EmberNodeId shortId,
                     uint32_t lastTransactionSeconds,
                     const uint8_t *mlEidTlv);

#endif // ADDRESS_CACHE_H
