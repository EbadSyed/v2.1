// File: tmsp-host-utilities.c
//
// Description: Functions to assist processing of TMSP commands and returns.
//
// Copyright 2015 Silicon Laboratories, Inc.                                *80*

#include PLATFORM_HEADER
#include "stack/core/ember-stack.h"
#include "stack/ip/ip-address.h"
#include "stack/ip/host/host-address-table.h"
#include "stack/ip/host/host-listener-table.h"
#include "app/coap/coap.h"
#include "stack/framework/event-queue.h"
#include "tmsp-host-utilities.h"

static EmberNetworkParameters networkCache; // Contains our ULA prefix.
static EmberNetworkStatus statusCache;

EmberIpv6Address hostIpAddress; // Mesh-local 64 address
EmberEui64 localEui64 = { { 0 } };  // Useful to construct Link-local 64 address
#ifndef RTOS
uint8_t emMacExtendedId[8] = { 0 };
uint8_t emHostUlaPrefix[8] = { 0 };
#endif

//------------------------------------------------------------------------------
// Cached Stack info

const EmberEui64 *emberEui64(void)
{
  return &localEui64;
}

EmberPanId emberGetPanId(void)
{
  return networkCache.panId;
}

void emberGetNetworkParameters(EmberNetworkParameters *networkParams)
{
  MEMCOPY(networkParams, &networkCache, sizeof(EmberNetworkParameters));
}

EmberNetworkStatus emberNetworkStatus(void)
{
  return statusCache;
}

bool emberGetLocalIpAddress(uint8_t index, EmberIpv6Address *address)
{
  if (index == 0 && !emIsMemoryZero(hostIpAddress.bytes, 16)) {
    // ML64
    MEMCOPY(address->bytes, hostIpAddress.bytes, 16);
    return true;
  } else if (index == 1) {
    // LL64
    emStoreLongFe8Address(emMacExtendedId, address->bytes);
    return true;
  } else if (index > 1) {
    GlobalAddressEntry *entry = emberGetHostGlobalAddress(index - 2);
    if (entry != NULL) {
      MEMCOPY(address->bytes, entry->address, 16);
      return true;
    }
  }
  return false;
}

void emberSetAddressHandler(const uint8_t *addr)
{
  EmberIpv6Address address;
  MEMCOPY(address.bytes, addr, 16);
  if (emIsDefaultGlobalPrefix(address.bytes)) {
    // Store the ULA.
    MEMCOPY(hostIpAddress.bytes, address.bytes, 16);

    uint8_t meshLocalIdentifier[8];
    emberReverseMemCopy(meshLocalIdentifier,
                        address.bytes + 8,
                        8);
    meshLocalIdentifier[7] ^= 0x02;

    // Configure these for the first time (only once).
    emSetMeshLocalIdentifierFromLongId(meshLocalIdentifier);
#ifndef UNIX_HOST_SIM
    emberConfigureDefaultHostAddress(&address);
#endif
  } else {
#ifndef UNIX_HOST_SIM
    emberConfigureGlobalHostAddress(&address, 0, 0, 0);
#endif
  }
}

void emberHostStateHandler(const EmberNetworkParameters *parameters,
                           const EmberEui64 *eui64,
                           const EmberEui64 *extendedMacId,
                           EmberNetworkStatus networkStatus)
{
  MEMCOPY(&networkCache, parameters, sizeof(EmberNetworkParameters));
  MEMCOPY(localEui64.bytes, eui64->bytes, EUI64_SIZE);
  MEMCOPY(emMacExtendedId, extendedMacId, sizeof(EmberEui64));
  statusCache = networkStatus;

#if (defined(UNIX_HOST) || defined(UNIX_HOST_SIM))
  emSetDefaultGlobalPrefix(networkCache.ulaPrefix.bytes);
  // Set the legacy ULA, and also glean whether it's on the lurker network.
  // See host-address-table.c.
  emSetLegacyUla(networkCache.legacyUla.bytes);
#endif

#ifndef UNIX_HOST_SIM
  // Legacy ULA (if required)
  if ((networkCache.nodeType & LURKER_NODE_TYPE_BIT || networkCache.nodeType == EMBER_LURKER)
      && (!emIsMemoryZero(networkCache.legacyUla.bytes, 8))) {
    EmberIpv6Address legacyAddress;
    MEMCOPY(legacyAddress.bytes, networkCache.legacyUla.bytes, 8);
    // Do not change this to some other interface id, please.
    emLongIdToInterfaceId(emMacExtendedId, legacyAddress.bytes + 8);
    emberConfigureLegacyHostAddress(&legacyAddress);
  }
#endif
}

bool tmspStateReturnPreHook(const EmberNetworkParameters *parameters,
                            const EmberEui64 *eui64,
                            const EmberEui64 *extendedMacId,
                            EmberNetworkStatus networkStatus)
{
  emberHostStateHandler(parameters, eui64, extendedMacId, networkStatus);
  return true;
}

// no-op not currently used NCP-to-Host.
void emberNcpToHostNoOp(const uint8_t *bytes, uint8_t bytesLength)
{
}

//------------------------------------------------------------------------------
// TMSP Hooks
bool tmspHostResetNetworkStatePreHook(void)
{
  // Reset host buffers and tables.
#ifndef RTOS
  emberCancelAllEvents(&emStackEventQueue);
  emInitializeBuffers();
  emCoapInitialize();
#endif
  emberInitializeHostAddressTable();
  emberCloseListeners();
  emberEnableHostDtlsClient(EMBER_SECURITY_TO_HOST);
  MEMSET(hostIpAddress.bytes, 0, 16);
  return true;
}

bool tmspHostResignGlobalAddressPreHook(const EmberIpv6Address *address)
{
  emberRemoveHostGlobalAddress(address);
  return true;
}

bool tmspHostResetMicroHandlerPreHook(void)
{
  emberCloseListeners();
  emberInitializeHostAddressTable();
  emberEnableHostDtlsClient(EMBER_SECURITY_TO_HOST);
  return true;
}

bool tmspHostAddressConfigurationChangePreHook(const EmberIpv6Address *address,
                                               uint32_t preferredLifetime,
                                               uint32_t validLifetime,
                                               uint8_t addressFlags)
{
  // Add it to our global address table.
  EmberStatus status = emberAddHostGlobalAddress(address,
                                                 preferredLifetime,
                                                 validLifetime,
                                                 addressFlags);

#ifdef UNIX_HOST_SIM
  return (status == EMBER_TABLE_FULL ? false : true);
#else
  if (status == EMBER_SUCCESS) {
    emberConfigureGlobalHostAddress(address,
                                    preferredLifetime,
                                    validLifetime,
                                    addressFlags);
  }
  return false;
#endif
}

void tmspHostSetEui64PostHook(const EmberEui64 *eui64)
{
  MEMCOPY(localEui64.bytes, eui64->bytes, 8);
}

bool tmspHostFormNetworkPreHook(uint16_t *options)
{
  // Host->NCP does not convey extended PAN ID.
  // Ensure the corresponding options bit is cleared.
  *options = *options & ~(EMBER_EXTENDED_PAN_ID_OPTION);
  return true;
}

//----------------------------------------------------------------
// Workaround because the network data may not fit in a single TMSP packet.
// The "ember" is added by the TMSP code generation system.  These are not
// actually API procedures.

// The length is always the full length of the data.  We either get sent the
// full data, if it is small enough to fit in a TMSP message, or nothing.

void emberNcpNetworkDataChangeHandler(uint16_t length,
                                      const uint8_t *networkData,
                                      uint8_t bytesSent)
{
  emberNetworkDataChangeHandler((bytesSent == length
                                 ? networkData
                                 : NULL),
                                length);
}

static uint8_t *networkDataBuffer = NULL;
static uint16_t networkDataBufferLength = 0;
static uint16_t networkDataBufferOffset = 0;

void emberGetNetworkData(uint8_t *buffer, uint16_t bufferLength)
{
  networkDataBuffer = buffer;
  networkDataBufferLength = bufferLength;
  networkDataBufferOffset = 0;
  emberNcpGetNetworkData(bufferLength);
}

void emberNcpGetNetworkDataReturn(EmberStatus status,
                                  uint16_t totalLength,
                                  const uint8_t *networkDataFragment,
                                  uint8_t fragmentLength,
                                  uint16_t fragmentOffset)
{
  if (networkDataBuffer == NULL) {
    // stray return - ignore it
  } else if (status != EMBER_SUCCESS) {
    emberGetNetworkDataReturn(status, NULL, 0);
  } else if (networkDataBufferLength < totalLength) {
    // Clear this first because the app may call emberGetNetworkData() from
    // the callback.
    networkDataBuffer = NULL;
    emberGetNetworkDataReturn(EMBER_BAD_ARGUMENT, NULL, 0);
  } else if (networkDataBufferOffset != fragmentOffset) {
    // someone screwed up somewhere - what to do?
    networkDataBuffer = NULL;
    emberGetNetworkDataReturn(EMBER_ERR_FATAL, NULL, 0);
  } else {
    MEMCOPY(networkDataBuffer + networkDataBufferOffset,
            networkDataFragment,
            fragmentLength);
    networkDataBufferOffset += fragmentLength;
    if (networkDataBufferOffset == totalLength) {
      uint8_t *temp = networkDataBuffer;
      // Clear this first because the app may call emberGetNetworkData() from
      // the callback.
      networkDataBuffer = NULL;
      emberGetNetworkDataReturn(EMBER_SUCCESS, temp, totalLength);
    }
  }
}

//----------------------------------------------------------------
// Workaround because the ND data may not fit in a single TMSP packet.
//
// The "ember" in "emberNcpSetNdData", is added by the TMSP code
// generation system.  It is not actually an API procedure.

// Experiments showed...
#define MAX_ND_DATA_FRAGMENT_LENGTH 90

void emberSetNdData(const uint8_t *data, uint16_t length)
{
  if (length == 0) {
    emberNcpSetNdData(0, NULL, 0, 0);
  } else {
    uint16_t offset = 0;
    do {
      uint16_t send =
        (MAX_ND_DATA_FRAGMENT_LENGTH < length
         ? MAX_ND_DATA_FRAGMENT_LENGTH
         : length);
      emberNcpSetNdData(length, data + offset, send, offset);
      offset += send;
    } while (offset < length);
  }
}

void emberSetLocalNetworkData(const uint8_t *data, uint16_t length)
{
  if (length == 0) {
    emberNcpSetLargeData(SET_LOCAL_NETWORK_DATA, 0, NULL, 0, 0);
  } else {
    uint16_t offset = 0;
    do {
      uint16_t send =
        (MAX_ND_DATA_FRAGMENT_LENGTH < length
         ? MAX_ND_DATA_FRAGMENT_LENGTH
         : length);
      emberNcpSetLargeData(SET_LOCAL_NETWORK_DATA,
                           length,
                           data + offset,
                           send,
                           offset);
      offset += send;
    } while (offset < length);
  }
}

// Stub for a no-longer-used callback that TMSP still expects to find.
void emberGetDhcpClientReturn(const EmberIpv6Address *address)
{
}
