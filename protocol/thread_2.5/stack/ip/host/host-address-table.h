// File: host-adress-table.h
//
// Description: Global address table for hosts.
//
// Copyright 2015 by Silicon Laboratories. All rights reserved.                *80*

#ifndef HOST_ADDRESS_TABLE_H
#define HOST_ADDRESS_TABLE_H

// Sets up a mesh local address, a link local address, and disables ND.
void emberConfigureDefaultHostAddress(const EmberIpv6Address *mlAddress);

// Sets up a Legacy address.
void emberConfigureLegacyHostAddress(const EmberIpv6Address *address);

// Sets up a GUA address.
void emberConfigureGlobalHostAddress(const EmberIpv6Address *address,
                                     uint32_t preferredLifetime,
                                     uint32_t validLifetime,
                                     uint8_t addressFlags);

void emberRemoveHostAddress(const EmberIpv6Address *address);
void emberRemoveAllHostAddresses(void);

extern GlobalAddressEntry emberHostAddressTable[EMBER_MAX_IPV6_GLOBAL_ADDRESS_COUNT];

void emberInitializeHostAddressTable(void);
EmberStatus emberAddHostGlobalAddress(const EmberIpv6Address *address,
                                      uint32_t preferredLifetime,
                                      uint32_t validLifetime,
                                      uint8_t addressFlags);
GlobalAddressEntry *emberFindHostGlobalAddress(uint8_t *prefix, uint8_t prefixLength);
GlobalAddressEntry *emberGetHostGlobalAddress(uint8_t index);
EmberStatus emberRemoveHostGlobalAddress(const EmberIpv6Address *address);
const GlobalAddressEntry *emberGetHostAddressTable(void);

#endif // HOST_ADDRESS_TABLE_H
