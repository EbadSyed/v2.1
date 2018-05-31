/** @file tmsp-ncp-utilities.h
 *
 *  Utilities to support NCP-side processing of TMSP commands and returns.
 *
 * <!--Copyright 2015 by Silicon Labs. All rights reserved.              *80*-->
 */

#ifndef __TMSP_NCP_UTILITIES_H__
#define __TMSP_NCP_UTILITIES_H__

// Convenience functions to generate callbacks.
void sendCommProxyAppPskcHandlerCallback(const uint8_t *pskc);
void sendNetworkStateCallback(bool stateRequested);
void sendSetDriverAddressCallback(uint8_t *address);
void sendNetworkKeysCallback(void);
void modemAddressConfigurationHandler(void);

// Pre/post hooks allow for special handling before/after the default behavior
// of generated NCP command handler/return code.
//

void tmspNcpInitReturnPreHook(void);
void tmspNcpInitReturnPostHook(void);

bool tmspNcpAddressConfigurationChangePreHook(const EmberIpv6Address *address);

bool tmspNcpCommissionNetworkPreHook(uint8_t networkIdLength);

bool tmspNcpNetworkStatusHandlerPreHook(EmberNetworkStatus newNetworkStatus);

bool tmspNcpNodeStatusReturnPreHook(const uint8_t **networkFragmentIdentifier, uint8_t *identifier);

bool tmspNcpPollForDataReturnPreHook(EmberStatus status);
void tmspNcpPollForDataReturnPostHook(EmberStatus status);

bool tmspNcpResetMicroHandlerPreHook(void);

bool tmspNcpResetNetworkStatePreHook(void);

bool tmspNcpResetNetworkStateReturnPreHook(EmberStatus status);

void tmspNcpSetRadioPowerReturnPostHook(void);

bool tmspNcpLaunchStandaloneBootloaderPreHook(uint8_t mode);

// Workaround because the network data may not fit in a single TMSP packet.
// The "emApi" is added by the TMSP code generation system.  This is not
// actually an API procedure.
void emApiNcpGetNetworkData(uint16_t length);

// Ditto, but these are defined in the script-produced tmsp-ncp.c.
void emApiNcpNetworkDataChangeHandler(uint16_t length,
                                      const uint8_t *networkData,
                                      uint8_t bytesSent);
void emApiNcpGetNetworkDataReturn(EmberStatus status,
                                  uint16_t totalLength,
                                  const uint8_t *networkDataFragment,
                                  uint8_t fragmentLength,
                                  uint16_t fragmentOffset);

// Two more work arounds, but in the other direction (host->NCP)
void emApiNcpSetNdData(uint16_t totalLength,
                       const uint8_t *ndDataFragment,
                       uint8_t fragmentLength,
                       uint16_t fragmentOffset);

extern Buffer emNcpSetNdDataBuffer;

void emApiNcpSetLargeData(SetLargeValueId id,
                          uint16_t totalLength,
                          const uint8_t *fragment,
                          uint8_t fragmentLength,
                          uint16_t fragmentOffset);

extern Buffer emNcpSetLocalNetworkDataBuffer;

#endif //__TMSP_NCP_UTILITIES_H__
