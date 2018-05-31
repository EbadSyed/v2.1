/*
 * File: ip-address.h
 * Description: IPv6 address utilities
 * Author(s): Richard Kelsey, Matteo Paris
 *
 * Copyright 2013 Silicon Laboratories, Inc.                                *80*
 */

#ifndef IP_ADDRESS_H
#define IP_ADDRESS_H

// This is used on both hosts and SOCs.  It does not include any reference
// to the local node ID, which is not available on hosts.  That code is in
// ip-header.c.

//------------------------------------------------------------------------------
// For global prefixes without an associated border router.
#define NULL_BORDER_ROUTER_INDEX 0xFF

typedef struct {
  uint16_t localConfigurationFlags;  // LocalServerFlag_e in the low byte,
                                     // internal state (see below) in high byte
  uint8_t address[16];

  // Values used for DHCP addresses
  //
  // Keeping track of request failures might need to keep track of
  // multiple servers.  On the other hand, multiple servers and
  // failures should both be rare, so simpler is probably better.
  uint8_t prefixLengthInBits;     // must be 64 for SLAAC
  uint8_t serverLongId[8];
  uint32_t preferredLifetime;     // in seconds
  uint32_t validLifetime;         // in seconds
  uint16_t requestTimeout;        // Seconds until we can retry
                                  // a failed address request.
} GlobalAddressEntry;

#define emberGetScopeFromAddress(addr)                            \
  ((emIsDefaultGlobalPrefix((const uint8_t*)(addr)) ? REALM_SCOPE \
    : (emIsFe8Address((const uint8_t*)(addr)) ? LINK_SCOPE : GLOBAL_SCOPE)))

// internal flags - low byte is LocalServerFlag_e.
#define GLOBAL_ADDRESS_IID_FROM_APP       0x8000 // the app supplied the IID
#define GLOBAL_ADDRESS_CONFIGURE_ADDRESS  0x4000 // app wants address maintained
#define GLOBAL_ADDRESS_REMOTE_GATEWAY     0x2000 // another node is a gateway
// This one is redundant with EMBER_GLOBAL_ADDRESS_AM_GATEWAY.
#define GLOBAL_ADDRESS_AM_GATEWAY         0x1000 // this node is a gateway
#define GLOBAL_ADDRESS_ACTIVE_GATEWAY     0x0800 // internal flag

extern Buffer emGlobalAddressTable;

/*

   These have now been moved to network-management.h:

   // We have to enforce this limit ourselves in order to avoid overflowing
   // when converting lifetimes from seconds to milliseconds.
   // #define MAX_LIFETIME_DELAY_SEC (EMBER_MAX_EVENT_DELAY_MS / 1000)

   // Must have at least half an hour of preferred lifetime remaining in order
   // to advertise a DHCP server.  This is just a guess.  If there are sleepy
   // nodes in the network it might need to be considerably higher.
   // #define MIN_PREFERRED_LIFETIME_SEC 1800

   // Renew when we are down to one minute of valid lifetime.  I have no idea
   // what the normal value for this is (rakelsey, git commit 6ec4f514).
   // #define MIN_VALID_LIFETIME_SEC 60

 */

// The actual SLAAC (preferred or valid) lifetime is not advertised, so
// we use this value.  We don't actually time out SLAAC addresses, but we
// need some value to report in DHCP LEASEQUERY replies.
#define SLAAC_LIFETIME_DELAY_SEC 1800

// Wait five minutes before trying again.  Errors should be rare (DHCP
// itself retries the request multiple times), and we want to give plenty
// of time for the network and/or gateway data to stabilize.
#define DHCP_REQUEST_TIMEOUT_SEC 300

// In RTOS builds we need to re-map the emStoreIpSourceAddress
// so that we can have different implementations on either side of things
#if defined(RTOS) && !defined(IP_MODEM_LIBRARY)
  #define emStoreIpSourceAddress emStoreIpSourceAddressHost
#endif //defined(RTOS) && !defined(IP_MODEM_LIBRARY)

void emDefaultPrefixInit(void);
void emGlobalAddressTableInit(void);
bool emIsDefaultGlobalPrefix(const uint8_t *prefix);
void emSetDefaultGlobalPrefix(const uint8_t *suppliedPrefix);
bool emStoreDefaultGlobalPrefix(uint8_t *target);
void emSetLegacyUla(const uint8_t *prefix);
void emStoreLegacyUla(uint8_t *target);
bool emIsLegacyUla(const uint8_t *prefix);
bool emHaveLegacyUla(void);
bool emNeedDhcpAddressRequest(GlobalAddressEntry *entry);
bool emStoreIpSourceAddress(uint8_t *source, const uint8_t *destination);

//----------------------------------------------------------------

void emUpdateGlobalAddresses(Buffer networkData, bool localServer);
GlobalAddressEntry *emGetPrefixAddressEntry(const uint8_t *prefix,
                                            uint8_t prefixLengthInBits,
                                            bool match);
void emDeleteGlobalAddressEntry(const uint8_t *prefix, uint8_t prefixLengthInBits);
void emDeleteGatewayAddressEntry(const uint8_t *prefix, uint8_t prefixLengthInBits);
bool emIsOurOnMeshPrefix(const uint8_t *prefix);

GlobalAddressEntry *emAddGlobalAddress(const uint8_t *address,
                                       uint8_t prefixLengthInBits,
                                       uint32_t preferredLifetime,
                                       uint32_t validLifetime,
                                       const uint8_t *serverLongId,
                                       uint16_t statusFlags);
// Return true if the address was one of ours.
bool emNoteDuplicateAddress(const uint8_t *address);
void emNoteEndDeviceAddressChange(void);

//----------------------------------------------------------------
// For printing out the global prefix entries.
void emPrintGlobalAddresses(void);

//------------------------------------------------------------------------------

// Unicast addresses

bool emStoreGp16(uint16_t id, uint8_t *target);
bool emStoreLocalMl64(uint8_t *target);
bool emIsGp16(const uint8_t *address, uint16_t *returnShort);
bool emIsGp64(const uint8_t *address);
#define emIsUnspecifiedAddress(addr) (emIsMemoryZero((addr), 16))
#define emIsLoopbackAddress(address) emberIsIpv6LoopbackAddress((EmberIpv6Address *)(address))
bool emStoreDefaultIpAddress(uint8_t *target);
bool emIsLocalIpAddress(const uint8_t *address);
bool emIsGlobalIpAddress(const uint8_t *address);
bool emIsUnicastForMe(const uint8_t *address);
bool emIsHostToNcpAddress(const uint8_t *address);

// Link local addresses

extern const Bytes8 emFe8Prefix;
void emStoreLongFe8Address(const uint8_t *longId, uint8_t *target);
bool emIsFe8Address(const uint8_t *address);
bool emIsLl64(const uint8_t *address);
bool emIsLocalLl64(const uint8_t *address);

// Mesh local identifier
extern uint8_t emMeshLocalIdentifier[8];
bool emberIsMeshLocalIdentifier(const uint8_t *identifier);
void emSetMeshLocalIdentifierFromLongId(const uint8_t *identifier);

// Interface Ids

void emStoreShortInterfaceId(uint16_t id, uint8_t *target);
bool emIsLocalMl64InterfaceId(const uint8_t* interfaceId);
bool emIsLocalLl64InterfaceId(const uint8_t* interfaceId);
bool emIsLocalShortInterfaceId(const uint8_t *interfaceId);
bool emGleanShortIdFromInterfaceId(const uint8_t *interfaceId, uint16_t* shortId);
void emLongIdToInterfaceId(const uint8_t *longId, uint8_t *target);
void emInterfaceIdToLongId(const uint8_t *interfaceId, uint8_t *longId);

// Multicast addresses

#define emIsMulticastAddress(ipAddress) ((ipAddress)[0] == 0xFF)
bool emIsAllNodesMulticastAddress(const uint8_t *address);
bool emIsFf01MulticastAddress(const uint8_t *address);
bool emIsFf02MulticastAddress(const uint8_t *address);
bool emIsFf03MulticastAddress(const uint8_t *address);
bool emIsFf32MulticastAddress(const uint8_t *address);
bool emIsFf33MulticastAddress(const uint8_t *address);
extern const Bytes16 emFf02AllNodesMulticastAddress;
extern const Bytes16 emFf02AllRoutersMulticastAddress;
extern const Bytes16 emFf03AllNodesMulticastAddress;
extern const Bytes16 emFf03AllRoutersMulticastAddress;
extern const Bytes16 emFf05AllNodesMulticastAddress;
extern const Bytes16 emFf05AllRoutersMulticastAddress;
extern const Bytes16 emFf02AllRplNodesMulticastAddress;
extern const Bytes16 emFf03AllDhcpMulticastAddress;
extern Bytes16 emFf32AllThreadNodesMulticastAddress;
extern Bytes16 emFf33AllThreadNodesMulticastAddress;
extern Bytes16 emFf33AllThreadRoutersMulticastAddress;
bool emIsMulticastForMe(const uint8_t *address);
void emSetAllThreadNodesMulticastAddresses(const uint8_t *meshLocalPrefix);

#ifdef EMBER_TEST
// For installing a global prefix for testing.
void emSetTestPrefix(const uint8_t *prefix);
#endif

#endif // IP_ADDRESS_H
