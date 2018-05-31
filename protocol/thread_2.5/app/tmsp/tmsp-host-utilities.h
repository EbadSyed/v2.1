/** @file tmsp-host-utilities.h
 *
 *  Utilities to support Host-side processing of TMSP commands and returns.
 *
 * <!--Copyright 2015 by Silicon Labs. All rights reserved.              *80*-->
 */

#ifndef __TMSP_HOST_UTILITIES_H__
#define __TMSP_HOST_UTILITIES_H__

bool tmspHostResetNetworkStatePreHook(void);
bool tmspHostResignGlobalAddressPreHook(const EmberIpv6Address *address);
bool tmspHostResetMicroHandlerPreHook(void);
bool tmspHostAddressConfigurationChangePreHook(const EmberIpv6Address *address,
                                               uint32_t preferredLifetime,
                                               uint32_t validLifetime,
                                               uint8_t addressFlags);
void tmspHostSetEui64PostHook(const EmberEui64 *eui64);
bool tmspHostFormNetworkPreHook(uint16_t *options);
bool tmspStateReturnPreHook(const EmberNetworkParameters *parameters,
                            const EmberEui64 *eui64,
                            const EmberEui64 *extendedMacId,
                            EmberNetworkStatus networkStatus);

//----------------------------------------------------------------
// Workaround because the network data may not fit in a single TMSP packet.
// The "ember" is added by the TMSP code generation system.  These are not
// actually API procedures.

void emberNcpNetworkDataChangeHandler(uint16_t length,
                                      const uint8_t *networkData,
                                      uint8_t bytesSent);
void emberNcpGetNetworkDataReturn(EmberStatus status,
                                  uint16_t totalLength,
                                  const uint8_t *networkDataFragment,
                                  uint8_t fragmentLength,
                                  uint16_t fragmentOffset);

// Defined in tmsp-host.c.
void emberNcpGetNetworkData(uint16_t bufferLength);

// Two more work arounds, but in the other direction (host->NCP)
void emberNcpSetNdData(uint16_t totalLength,
                       const uint8_t *fragment,
                       uint8_t fragmentLength,
                       uint16_t fragmentOffset);

void emberNcpSetLargeData(SetLargeValueId valueId,
                          uint16_t totalLength,
                          const uint8_t *fragment,
                          uint8_t fragmentLength,
                          uint16_t fragmentOffset);

// Declaration for a no-longer-used callback that TMSP still expects to find.
void emberGetDhcpClientReturn(const EmberIpv6Address *address);

#endif //__TMSP_HOST_UTILITIES_H__
