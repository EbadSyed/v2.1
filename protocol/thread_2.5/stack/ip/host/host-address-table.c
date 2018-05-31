// File: host-adress-table.c
//
// Description: Global address table for hosts.
//
// Copyright 2015 by Silicon Laboratories. All rights reserved.                *80*

#include "stack/core/ember-stack.h"
#include "stack/ip/ip-address.h"
#include "stack/ip/host/host-address-table.h"

GlobalAddressEntry emberHostAddressTable[EMBER_MAX_IPV6_GLOBAL_ADDRESS_COUNT];

void emberInitializeHostAddressTable(void)
{
  MEMSET(&emberHostAddressTable, 0, sizeof(emberHostAddressTable));
  // de-configure addresses on the host interface (unix/tun0 or LWIP)
  emberRemoveAllHostAddresses();
}

EmberStatus emberAddHostGlobalAddress(const EmberIpv6Address *address,
                                      uint32_t preferredLifetime,
                                      uint32_t validLifetime,
                                      uint8_t localConfigurationFlags)
{
  uint8_t i;
  for (i = 0; i < COUNTOF(emberHostAddressTable); i++) {
    GlobalAddressEntry *entry = &emberHostAddressTable[i];
    if (MEMCOMPARE(entry->address, address, 16) == 0) {
      return EMBER_BAD_ARGUMENT; // Need a better error
    } else if (emIsMemoryZero(entry->address, 16)) {
      MEMCOPY(entry->address, address, 16);
      entry->preferredLifetime = preferredLifetime;
      entry->validLifetime = validLifetime;
      entry->localConfigurationFlags = localConfigurationFlags;
      return EMBER_SUCCESS;
    }
  }
  return EMBER_TABLE_FULL;
}

GlobalAddressEntry *emberGetHostGlobalAddress(uint8_t index)
{
  if (!emIsMemoryZero(emberHostAddressTable[index].address, 16)) {
    return &emberHostAddressTable[index];
  } else {
    return NULL;
  }
}

GlobalAddressEntry *emberFindHostGlobalAddress(uint8_t *prefix, uint8_t prefixLength)
{
  uint8_t i;
  for (i = 0; i < COUNTOF(emberHostAddressTable); i++) {
    GlobalAddressEntry *entry = &emberHostAddressTable[i];
    if (MEMCOMPARE(entry->address, prefix, prefixLength) == 0) {
      return entry;
    }
  }
  return NULL;
}

EmberStatus emberRemoveHostGlobalAddress(const EmberIpv6Address *address)
{
  uint8_t i;
  for (i = 0; i < COUNTOF(emberHostAddressTable); i++) {
    GlobalAddressEntry *entry = &emberHostAddressTable[i];
    if (MEMCOMPARE(entry->address, address, 16) == 0) {
      MEMSET(entry->address, 0, 16);
      entry->preferredLifetime = 0;
      entry->validLifetime = 0;
      entry->localConfigurationFlags = 0;
      return EMBER_SUCCESS;
    }
  }
  return EMBER_BAD_ARGUMENT;
}

const GlobalAddressEntry *emberGetHostAddressTable(void)
{
  return emberHostAddressTable;
}

#if (defined(UNIX_HOST) || defined(UNIX_HOST_SIM))
// ip-address helper methods only for non-RTOS hosts
// TODO:  These should ideally live in a separate file.

static uint8_t legacyUla[8];
bool isOnLurkerNetwork = false;

void emStoreLegacyUla(uint8_t *target)
{
  MEMCOPY(target, legacyUla, 8);
}

bool emIsLegacyUla(const uint8_t *prefix)
{
  return (isOnLurkerNetwork
          && memcmp(prefix, legacyUla, 8) == 0);
}

bool emHaveLegacyUla(void)
{
  return (isOnLurkerNetwork
          && !emIsMemoryZero(legacyUla, 8));
}

void emSetLegacyUla(const uint8_t *prefix)
{
  MEMCOPY(legacyUla, prefix, 8);

  // Also store whether it's on the lurker network.
  EmberNetworkParameters networkParams;
  emberGetNetworkParameters(&networkParams);
  uint8_t type = networkParams.nodeType;
  if (type & LURKER_NODE_TYPE_BIT || type == EMBER_LURKER) {
    // top (lurker) bit is set, or it's a lurker.
    isOnLurkerNetwork = true;
  }
}

bool emIsDefaultGlobalPrefix(const uint8_t *prefix)
{
  EmberNetworkParameters networkParams;
  emberGetNetworkParameters(&networkParams);
  return memcmp(prefix, networkParams.ulaPrefix.bytes, 8) == 0;
}

bool emIsLocalIpAddress(const uint8_t *address)
{
  const uint8_t *iid = address + 8;

  if (emIsFe8Address(address)) {
    return (emIsLocalLl64InterfaceId(iid)
            || emIsLocalShortInterfaceId(iid)); // only on legacy networks
  } else if (emIsLegacyUla(address)) {
    return (emIsLocalLl64InterfaceId(iid)
            || emIsLocalShortInterfaceId(iid)
            // Handle case where the legacy prefix is also the ML prefix
            || (emIsDefaultGlobalPrefix(address)
                && emIsLocalMl64InterfaceId(iid)));
  } else if (emIsDefaultGlobalPrefix(address)) {
    return (emIsLocalMl64InterfaceId(iid)
            || emIsLocalShortInterfaceId(iid));
  } else {
    GlobalAddressEntry *entry = emberFindHostGlobalAddress((uint8_t *) address, 16);
    return (entry != NULL
            && (entry->localConfigurationFlags
                & EMBER_GLOBAL_ADDRESS_CONFIGURED));
  }
}

bool emStoreDefaultGlobalPrefix(uint8_t *target)
{
  EmberNetworkParameters networkParams;
  emberGetNetworkParameters(&networkParams);
  MEMCOPY(target, networkParams.ulaPrefix.bytes, 8);
  return true;
}

void emSetDefaultGlobalPrefix(const uint8_t *suppliedPrefix)
{
  emSetAllThreadNodesMulticastAddresses(suppliedPrefix);
}
#endif // (defined (UNIX_HOST) || defined(UNIX_HOST_SIM))

#if (defined (UNIX_HOST) || defined (UNIX_HOST_SIM) || defined (RTOS))
bool emStoreIpSourceAddress(uint8_t *source, const uint8_t *destination)
{
  if (emIsFe8Address(destination) || emIsLegacyUla(destination)) {
    emStoreLongFe8Address(emMacExtendedId, source);
  } else if (emIsDefaultGlobalPrefix(destination)) {
    emStoreLocalMl64(source);
  } else if (destination[0] == 0xFF) {    // multicast
    if (destination[1] < 0x03               // less than subnet-local scope
        || destination[1] == 0x32) {        // link local all thread nodes
      emStoreLongFe8Address(emMacExtendedId, source);
    } else if (destination[1] <= 0x05       // less than site-local scope
               || destination[1] == 0x33) { // realm local all thread nodes
      return emStoreLocalMl64(source);
    } else {
      return false;                       // too wide a multicast scope
    }
  } else {
    GlobalAddressEntry *entry = emberFindHostGlobalAddress((uint8_t *) destination, 8);
    if (entry == NULL) {
      entry = emberGetHostGlobalAddress(0); // Get the first configured address.
    }
    if (entry == NULL) {
      return false;     // we have no appropriate address
    } else {
      MEMCOPY(source, entry->address, 16);
    }
  }
  return true;
}
#endif // (defined (UNIX_HOST) || defined(UNIX_HOST_SIM) || defined (RTOS))
