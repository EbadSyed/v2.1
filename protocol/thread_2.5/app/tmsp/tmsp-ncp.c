// File: tmsp-ncp.c
//
// *** Generated file. Do not edit! ***
//
// run tmsp-update.sh to regenerate
//
// Description: NCP functions for sending Thread management commands
// to the Host.
//
// TMSP Version: 14.0
//
// Copyright 2017 Silicon Laboratories, Inc.                                *80*

#include PLATFORM_HEADER
#ifdef CONFIGURATION_HEADER
  #include CONFIGURATION_HEADER
#endif
#include "stack/core/ember-stack.h"
#include "app/ip-ncp/binary-management.h"
#include "include/define-ember-api.h"
#include "app/util/serial/command-interpreter2.h"
#include "include/thread-debug.h"
#include "include/undefine-ember-api.h"
#include "app/tmsp/tmsp-enum.h"
#include "app/tmsp/tmsp-frame-utilities.h"
#include "app/tmsp/tmsp-ncp-utilities.h"
#include "app/tmsp/tmsp-configuration.h"
#include "app/tmsp/tmsp-mfglib.h"

//------------------------------------------------------------------------------
// Core
//------------------------------------------------------------------------------

#ifdef HAS_EMBER_RESET_MICRO_COMMAND_IDENTIFIER
static void modemResetMicroCommand(void)
{
  emApiResetMicro();
}
#endif // HAS_EMBER_RESET_MICRO_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_RESET_NETWORK_STATE_COMMAND_IDENTIFIER
static void modemResetNetworkStateCommand(void)
{
  bool preHookResult = tmspNcpResetNetworkStatePreHook();
  if (preHookResult) {
    emApiResetNetworkState();
  }
}
#endif // HAS_EMBER_RESET_NETWORK_STATE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_INIT_HOST_COMMAND_IDENTIFIER
static void modemInitHostCommand(void)
{
  emApiInitHost();
}
#endif // HAS_EMBER_INIT_HOST_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_STATE_COMMAND_IDENTIFIER
static void modemStateCommand(void)
{
  emApiState();
}
#endif // HAS_EMBER_STATE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_VERSIONS_COMMAND_IDENTIFIER
static void modemGetVersionsCommand(void)
{
  emApiGetVersions();
}
#endif // HAS_EMBER_GET_VERSIONS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_FORM_NETWORK_COMMAND_IDENTIFIER
static void modemFormNetworkCommand(void)
{
  EmberNetworkParameters parameters;
  uint16_t options;
  uint32_t channelMask;
  MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
  emberGetStringArgument(0, parameters.networkId, sizeof(parameters.networkId), false);
  emberGetStringArgument(1, parameters.ulaPrefix.bytes, sizeof(parameters.ulaPrefix.bytes), false);
  parameters.nodeType = emberUnsignedCommandArgument(2);
  parameters.radioTxPower = emberSignedCommandArgument(3);
  options = emberUnsignedCommandArgument(4);
  channelMask = emberUnsignedCommandArgument(5);
  emberGetStringArgument(6, parameters.legacyUla.bytes, sizeof(parameters.legacyUla.bytes), false);
  emApiFormNetwork(&parameters,
                   options,
                   channelMask);
}
#endif // HAS_EMBER_FORM_NETWORK_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_JOIN_NETWORK_COMMAND_IDENTIFIER
static void modemJoinNetworkCommand(void)
{
  EmberNetworkParameters parameters;
  uint16_t options;
  uint32_t channelMask;
  MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
  emberGetStringArgument(0, parameters.networkId, sizeof(parameters.networkId), false);
  emberGetStringArgument(1, parameters.extendedPanId, sizeof(parameters.extendedPanId), false);
  parameters.panId = emberUnsignedCommandArgument(2);
  parameters.nodeType = emberUnsignedCommandArgument(3);
  parameters.radioTxPower = emberSignedCommandArgument(4);
  emberGetStringArgument(5, parameters.joinKey, sizeof(parameters.joinKey), false);
  parameters.joinKeyLength = emberUnsignedCommandArgument(6);
  options = emberUnsignedCommandArgument(7);
  channelMask = emberUnsignedCommandArgument(8);
  emApiJoinNetwork(&parameters,
                   options,
                   channelMask);
  emNetworkStateChangedHandler();
}
#endif // HAS_EMBER_JOIN_NETWORK_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_RESUME_NETWORK_COMMAND_IDENTIFIER
static void modemResumeNetworkCommand(void)
{
  emApiResumeNetwork();
  emNetworkStateChangedHandler();
}
#endif // HAS_EMBER_RESUME_NETWORK_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER
static void modemAttachToNetworkCommand(void)
{
  emApiAttachToNetwork();
}
#endif // HAS_EMBER_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_HOST_JOIN_CLIENT_COMPLETE_COMMAND_IDENTIFIER
static void modemHostJoinClientCompleteCommand(void)
{
  uint32_t keySequence;
  uint8_t key[EMBER_ENCRYPTION_KEY_SIZE];
  uint8_t ulaPrefix[8];
  keySequence = (uint32_t) emberUnsignedCommandArgument(0);
  emberGetStringArgument(1, key, EMBER_ENCRYPTION_KEY_SIZE, false);
  emberGetStringArgument(2, ulaPrefix, 8, false);
  emApiHostJoinClientComplete(keySequence,
                              key,
                              ulaPrefix);
}
#endif // HAS_EMBER_HOST_JOIN_CLIENT_COMPLETE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER
static void modemSetSecurityParametersCommand(void)
{
  EmberSecurityParameters parameters;
  uint16_t options;
  MEMSET(&parameters, 0, sizeof(EmberSecurityParameters));
  parameters.networkKey = (EmberKeyData *)emberStringCommandArgument(0, NULL);
  parameters.presharedKey = (uint8_t *)emberStringCommandArgument(1, &(parameters.presharedKeyLength));
  options = emberUnsignedCommandArgument(2);
  emApiSetSecurityParameters(&parameters,
                             options);
}
#endif // HAS_EMBER_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER
static void modemSwitchToNextNetworkKeyCommand(void)
{
  emApiSwitchToNextNetworkKey();
}
#endif // HAS_EMBER_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_START_SCAN_COMMAND_IDENTIFIER
static void modemStartScanCommand(void)
{
  EmberNetworkScanType scanType;
  uint32_t channelMask;
  uint8_t duration;
  scanType = (EmberNetworkScanType) emberUnsignedCommandArgument(0);
  channelMask = (uint32_t) emberUnsignedCommandArgument(1);
  duration = (uint8_t) emberUnsignedCommandArgument(2);
  emApiStartScan(scanType,
                 channelMask,
                 duration);
}
#endif // HAS_EMBER_START_SCAN_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_STOP_SCAN_COMMAND_IDENTIFIER
static void modemStopScanCommand(void)
{
  emApiStopScan();
}
#endif // HAS_EMBER_STOP_SCAN_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_RIP_ENTRY_COMMAND_IDENTIFIER
static void modemGetRipEntryCommand(void)
{
  uint8_t index;
  index = (uint8_t) emberUnsignedCommandArgument(0);
  emApiGetRipEntry(index);
}
#endif // HAS_EMBER_GET_RIP_ENTRY_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_MULTICAST_TABLE_COMMAND_IDENTIFIER
static void modemGetMulticastTableCommand(void)
{
  emApiGetMulticastTable();
}
#endif // HAS_EMBER_GET_MULTICAST_TABLE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_COUNTER_COMMAND_IDENTIFIER
static void modemGetCounterCommand(void)
{
  EmberCounterType type;
  type = (EmberCounterType) emberUnsignedCommandArgument(0);
  emApiGetCounter(type);
}
#endif // HAS_EMBER_GET_COUNTER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_CLEAR_COUNTERS_COMMAND_IDENTIFIER
static void modemClearCountersCommand(void)
{
  emApiClearCounters();
}
#endif // HAS_EMBER_CLEAR_COUNTERS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_TX_POWER_MODE_COMMAND_IDENTIFIER
static void modemSetTxPowerModeCommand(void)
{
  uint16_t txPowerMode;
  txPowerMode = (uint16_t) emberUnsignedCommandArgument(0);
  EmberStatus returnValue = emApiSetTxPowerMode(txPowerMode);
  emApiSetTxPowerModeReturn(returnValue);
}
#endif // HAS_EMBER_SET_TX_POWER_MODE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_TX_POWER_MODE_COMMAND_IDENTIFIER
static void modemGetTxPowerModeCommand(void)
{
  emApiGetTxPowerMode();
}
#endif // HAS_EMBER_GET_TX_POWER_MODE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER
static void modemGetCcaThresholdCommand(void)
{
  emApiGetCcaThreshold();
}
#endif // HAS_EMBER_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER
static void modemSetCcaThresholdCommand(void)
{
  int8_t threshold;
  threshold = (int8_t) emberSignedCommandArgument(0);
  emApiSetCcaThreshold(threshold);
}
#endif // HAS_EMBER_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_RADIO_POWER_COMMAND_IDENTIFIER
static void modemGetRadioPowerCommand(void)
{
  emApiGetRadioPower();
}
#endif // HAS_EMBER_GET_RADIO_POWER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_RADIO_POWER_COMMAND_IDENTIFIER
static void modemSetRadioPowerCommand(void)
{
  int8_t power;
  power = (int8_t) emberSignedCommandArgument(0);
  emApiSetRadioPower(power);
}
#endif // HAS_EMBER_SET_RADIO_POWER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_ECHO_COMMAND_IDENTIFIER
static void modemEchoCommand(void)
{
  uint8_t *data;
  uint8_t length;
  data = emberStringCommandArgument(0, &length);
  emApiEcho(data,
            length);
}
#endif // HAS_EMBER_ECHO_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER
static void modemConfigureGatewayCommand(void)
{
  uint16_t borderRouterFlags;
  bool isStable;
  uint8_t prefix[16];
  uint8_t prefixLengthInBits;
  uint8_t domainId;
  uint32_t preferredLifetime;
  uint32_t validLifetime;
  borderRouterFlags = (uint16_t) emberUnsignedCommandArgument(0);
  isStable = (bool) emberUnsignedCommandArgument(1);
  emberGetStringArgument(2, prefix, 16, false);
  prefixLengthInBits = (uint8_t) emberUnsignedCommandArgument(3);
  domainId = (uint8_t) emberUnsignedCommandArgument(4);
  preferredLifetime = (uint32_t) emberUnsignedCommandArgument(5);
  validLifetime = (uint32_t) emberUnsignedCommandArgument(6);
  emApiConfigureGateway(borderRouterFlags,
                        isStable,
                        prefix,
                        prefixLengthInBits,
                        domainId,
                        preferredLifetime,
                        validLifetime);
}
#endif // HAS_EMBER_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_INDEXED_TOKEN_COMMAND_IDENTIFIER
static void modemGetIndexedTokenCommand(void)
{
  EmberTokenId tokenId;
  uint8_t index;
  tokenId = (EmberTokenId) emberUnsignedCommandArgument(0);
  index = (uint8_t) emberUnsignedCommandArgument(1);
  emApiGetIndexedToken(tokenId,
                       index);
}
#endif // HAS_EMBER_GET_INDEXED_TOKEN_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_POLL_FOR_DATA_COMMAND_IDENTIFIER
static void modemPollForDataCommand(void)
{
  emApiPollForData();
}
#endif // HAS_EMBER_POLL_FOR_DATA_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_DEEP_SLEEP_COMMAND_IDENTIFIER
static void modemDeepSleepCommand(void)
{
  bool sleep;
  sleep = (bool) emberUnsignedCommandArgument(0);
  emApiDeepSleep(sleep);
}
#endif // HAS_EMBER_DEEP_SLEEP_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER
static void modemStackPollForDataCommand(void)
{
  uint32_t pollMs;
  pollMs = (uint32_t) emberUnsignedCommandArgument(0);
  emApiStackPollForData(pollMs);
}
#endif // HAS_EMBER_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_OK_TO_NAP_COMMAND_IDENTIFIER
static void modemOkToNapCommand(void)
{
  bool returnValue = emApiOkToNap();
  emApiOkToNapReturn(returnValue);
}
#endif // HAS_EMBER_OK_TO_NAP_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_PING_COMMAND_IDENTIFIER
static void modemPingCommand(void)
{
  uint8_t destination[16];
  uint16_t id;
  uint16_t sequence;
  uint16_t length;
  uint8_t hopLimit;
  emberGetStringArgument(0, destination, 16, false);
  id = (uint16_t) emberUnsignedCommandArgument(1);
  sequence = (uint16_t) emberUnsignedCommandArgument(2);
  length = (uint16_t) emberUnsignedCommandArgument(3);
  hopLimit = (uint8_t) emberUnsignedCommandArgument(4);
  bool returnValue = emApiPing(destination,
                               id,
                               sequence,
                               length,
                               hopLimit);
  (void)returnValue;
}
#endif // HAS_EMBER_PING_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_JOIN_COMMISSIONED_COMMAND_IDENTIFIER
static void modemJoinCommissionedCommand(void)
{
  int8_t radioTxPower;
  EmberNodeType nodeType;
  bool requireConnectivity;
  radioTxPower = (int8_t) emberSignedCommandArgument(0);
  nodeType = (EmberNodeType) emberUnsignedCommandArgument(1);
  requireConnectivity = (bool) emberUnsignedCommandArgument(2);
  emApiJoinCommissioned(radioTxPower,
                        nodeType,
                        requireConnectivity);
}
#endif // HAS_EMBER_JOIN_COMMISSIONED_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_COMMISSION_NETWORK_COMMAND_IDENTIFIER
static void modemCommissionNetworkCommand(void)
{
  uint8_t preferredChannel;
  uint32_t fallbackChannelMask;
  uint8_t *networkId;
  uint8_t networkIdLength;
  uint16_t panId;
  uint8_t ulaPrefix[8];
  uint8_t extendedPanId[EXTENDED_PAN_ID_SIZE];
  EmberKeyData key;
  uint32_t keySequence;
  MEMSET(&key, 0, sizeof(EmberKeyData));
  preferredChannel = (uint8_t) emberUnsignedCommandArgument(0);
  fallbackChannelMask = (uint32_t) emberUnsignedCommandArgument(1);
  networkId = emberStringCommandArgument(2, &networkIdLength);
  panId = (uint16_t) emberUnsignedCommandArgument(3);
  emberGetStringArgument(4, ulaPrefix, 8, false);
  emberGetStringArgument(5, extendedPanId, EXTENDED_PAN_ID_SIZE, false);
  emberKeyDataCommandArgument(6, &key);
  keySequence = (uint32_t) emberUnsignedCommandArgument(7);
  bool preHookResult = tmspNcpCommissionNetworkPreHook(networkIdLength);
  if (preHookResult) {
    emApiCommissionNetwork(preferredChannel,
                           fallbackChannelMask,
                           networkId,
                           networkIdLength,
                           panId,
                           ulaPrefix,
                           extendedPanId,
                           &key,
                           keySequence);
  }
}
#endif // HAS_EMBER_COMMISSION_NETWORK_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER
static void modemRequestDhcpAddressCommand(void)
{
  uint8_t prefix[16];
  uint8_t prefixLengthInBits;
  emberGetStringArgument(0, prefix, 16, false);
  prefixLengthInBits = (uint8_t) emberUnsignedCommandArgument(1);
  emApiRequestDhcpAddress(prefix,
                          prefixLengthInBits);
}
#endif // HAS_EMBER_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_HOST_TO_NCP_NO_OP_COMMAND_IDENTIFIER
static void modemHostToNcpNoOpCommand(void)
{
  uint8_t *bytes;
  uint8_t bytesLength;
  bytes = emberStringCommandArgument(0, &bytesLength);
  emApiHostToNcpNoOp(bytes,
                     bytesLength);
}
#endif // HAS_EMBER_HOST_TO_NCP_NO_OP_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_GLOBAL_ADDRESSES_COMMAND_IDENTIFIER
static void modemGetGlobalAddressesCommand(void)
{
  uint8_t prefix[16];
  uint8_t prefixLengthInBits;
  emberGetStringArgument(0, prefix, 16, false);
  prefixLengthInBits = (uint8_t) emberUnsignedCommandArgument(1);
  emApiGetGlobalAddresses(prefix,
                          prefixLengthInBits);
}
#endif // HAS_EMBER_GET_GLOBAL_ADDRESSES_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_DHCP_CLIENTS_COMMAND_IDENTIFIER
static void modemGetDhcpClientsCommand(void)
{
  emApiGetDhcpClients();
}
#endif // HAS_EMBER_GET_DHCP_CLIENTS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_ADD_STEERING_EUI64_COMMAND_IDENTIFIER
static void modemAddSteeringEui64Command(void)
{
  EmberEui64 eui64;
  MEMSET(&eui64, 0, sizeof(EmberEui64));
  emberGetStringArgument(0, eui64.bytes, EUI64_SIZE, false);
  emApiAddSteeringEui64(&eui64);
}
#endif // HAS_EMBER_ADD_STEERING_EUI64_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_BECOME_COMMISSIONER_COMMAND_IDENTIFIER
static void modemBecomeCommissionerCommand(void)
{
  uint8_t *deviceName;
  uint8_t deviceNameLength;
  deviceName = emberStringCommandArgument(0, &deviceNameLength);
  emApiBecomeCommissioner(deviceName,
                          deviceNameLength);
}
#endif // HAS_EMBER_BECOME_COMMISSIONER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_COMMISSIONER_COMMAND_IDENTIFIER
static void modemGetCommissionerCommand(void)
{
  emApiGetCommissioner();
}
#endif // HAS_EMBER_GET_COMMISSIONER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SEND_STEERING_DATA_COMMAND_IDENTIFIER
static void modemSendSteeringDataCommand(void)
{
  emApiSendSteeringData();
}
#endif // HAS_EMBER_SEND_STEERING_DATA_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_STOP_COMMISSIONING_COMMAND_IDENTIFIER
static void modemStopCommissioningCommand(void)
{
  emApiStopCommissioning();
}
#endif // HAS_EMBER_STOP_COMMISSIONING_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_JOIN_KEY_COMMAND_IDENTIFIER
static void modemSetJoinKeyCommand(void)
{
  EmberEui64 eui64;
  uint8_t *key;
  uint8_t keyLength;
  MEMSET(&eui64, 0, sizeof(EmberEui64));
  emberGetStringArgument(0, eui64.bytes, EUI64_SIZE, false);
  key = (uint8_t *)emberStringCommandArgument(1, &(keyLength));
  emApiSetJoinKey(&eui64,
                  key,
                  keyLength);
}
#endif // HAS_EMBER_SET_JOIN_KEY_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_JOINING_MODE_COMMAND_IDENTIFIER
static void modemSetJoiningModeCommand(void)
{
  EmberJoiningMode mode;
  uint8_t length;
  mode = (EmberJoiningMode) emberUnsignedCommandArgument(0);
  length = (uint8_t) emberUnsignedCommandArgument(1);
  emApiSetJoiningMode(mode,
                      length);
}
#endif // HAS_EMBER_SET_JOINING_MODE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER
static void modemChangeNodeTypeCommand(void)
{
  EmberNodeType newType;
  newType = (EmberNodeType) emberUnsignedCommandArgument(0);
  emApiChangeNodeType(newType);
}
#endif // HAS_EMBER_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_GLOBAL_PREFIXES_COMMAND_IDENTIFIER
static void modemGetGlobalPrefixesCommand(void)
{
  emApiGetGlobalPrefixes();
}
#endif // HAS_EMBER_GET_GLOBAL_PREFIXES_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER
static void modemResignGlobalAddressCommand(void)
{
  EmberIpv6Address address;
  MEMSET(&address, 0, sizeof(EmberIpv6Address));
  emberIpv6AddressCommandArgument(0, &address);
  emApiResignGlobalAddress(&address);
}
#endif // HAS_EMBER_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_EUI64_COMMAND_IDENTIFIER
static void modemSetEui64Command(void)
{
  EmberEui64 eui64;
  MEMSET(&eui64, 0, sizeof(EmberEui64));
  emberGetStringArgument(0, eui64.bytes, EUI64_SIZE, false);
  emApiSetEui64(&eui64);
}
#endif // HAS_EMBER_SET_EUI64_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER
static void modemRequestSlaacAddressCommand(void)
{
  uint8_t prefix[16];
  uint8_t prefixLengthInBits;
  emberGetStringArgument(0, prefix, 16, false);
  prefixLengthInBits = (uint8_t) emberUnsignedCommandArgument(1);
  emApiRequestSlaacAddress(prefix,
                           prefixLengthInBits);
}
#endif // HAS_EMBER_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_CUSTOM_HOST_TO_NCP_MESSAGE_COMMAND_IDENTIFIER
static void modemCustomHostToNcpMessageCommand(void)
{
  uint8_t *message;
  uint8_t messageLength;
  message = emberStringCommandArgument(0, &messageLength);
  emApiCustomHostToNcpMessageHandler(message,
                                     messageLength);
}
#endif // HAS_EMBER_CUSTOM_HOST_TO_NCP_MESSAGE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER
static void modemGetNetworkDataTlvCommand(void)
{
  uint8_t type;
  uint8_t index;
  type = (uint8_t) emberUnsignedCommandArgument(0);
  index = (uint8_t) emberUnsignedCommandArgument(1);
  emApiGetNetworkDataTlv(type,
                         index);
}
#endif // HAS_EMBER_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER
static void modemGetRoutingLocatorCommand(void)
{
  emApiGetRoutingLocator();
}
#endif // HAS_EMBER_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER
static void modemSetRandomizeMacExtendedIdCommand(void)
{
  bool value;
  value = (bool) emberUnsignedCommandArgument(0);
  emApiSetRandomizeMacExtendedId(value);
}
#endif // HAS_EMBER_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER
static void modemConfigureExternalRouteCommand(void)
{
  uint8_t externalRouteFlags;
  bool isStable;
  uint8_t prefix[16];
  uint8_t prefixLengthInBits;
  uint8_t externalRouteDomainId;
  externalRouteFlags = (uint8_t) emberUnsignedCommandArgument(0);
  isStable = (bool) emberUnsignedCommandArgument(1);
  emberGetStringArgument(2, prefix, 16, false);
  prefixLengthInBits = (uint8_t) emberUnsignedCommandArgument(3);
  externalRouteDomainId = (uint8_t) emberUnsignedCommandArgument(4);
  emApiConfigureExternalRoute(externalRouteFlags,
                              isStable,
                              prefix,
                              prefixLengthInBits,
                              externalRouteDomainId);
}
#endif // HAS_EMBER_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER
static void modemAllowNativeCommissionerCommand(void)
{
  bool on;
  on = (bool) emberUnsignedCommandArgument(0);
  emApiAllowNativeCommissioner(on);
}
#endif // HAS_EMBER_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER
static void modemSetCommissionerKeyCommand(void)
{
  uint8_t *commissionerKey;
  uint8_t commissionerKeyLength;
  commissionerKey = emberStringCommandArgument(0, &commissionerKeyLength);
  emApiSetCommissionerKey(commissionerKey,
                          commissionerKeyLength);
}
#endif // HAS_EMBER_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER
static void modemGetStandaloneBootloaderInfoCommand(void)
{
  emApiGetStandaloneBootloaderInfo();
}
#endif // HAS_EMBER_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER
static void modemLaunchStandaloneBootloaderCommand(void)
{
  uint8_t mode;
  mode = (uint8_t) emberUnsignedCommandArgument(0);
  bool preHookResult = tmspNcpLaunchStandaloneBootloaderPreHook(mode);
  if (preHookResult) {
    emApiLaunchStandaloneBootloader(mode);
  }
}
#endif // HAS_EMBER_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_MFG_TOKEN_COMMAND_IDENTIFIER
static void modemGetMfgTokenCommand(void)
{
  EmberMfgTokenId tokenId;
  tokenId = (EmberMfgTokenId) emberUnsignedCommandArgument(0);
  emApiGetMfgToken(tokenId);
}
#endif // HAS_EMBER_GET_MFG_TOKEN_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_MFG_TOKEN_COMMAND_IDENTIFIER
static void modemSetMfgTokenCommand(void)
{
  EmberMfgTokenId tokenId;
  uint8_t *tokenData;
  uint8_t tokenDataLength;
  tokenId = (EmberMfgTokenId) emberUnsignedCommandArgument(0);
  tokenData = emberStringCommandArgument(1, &tokenDataLength);
  emApiSetMfgToken(tokenId,
                   tokenData,
                   tokenDataLength);
}
#endif // HAS_EMBER_SET_MFG_TOKEN_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_ENABLE_HOST_DTLS_CLIENT_COMMAND_IDENTIFIER
static void modemEnableHostDtlsClientCommand(void)
{
  bool enable;
  enable = (bool) emberUnsignedCommandArgument(0);
  emApiEnableHostDtlsClient(enable);
}
#endif // HAS_EMBER_ENABLE_HOST_DTLS_CLIENT_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_CTUNE_COMMAND_IDENTIFIER
static void modemGetCtuneCommand(void)
{
  emApiGetCtune();
}
#endif // HAS_EMBER_GET_CTUNE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_CTUNE_COMMAND_IDENTIFIER
static void modemSetCtuneCommand(void)
{
  uint16_t tune;
  tune = (uint16_t) emberUnsignedCommandArgument(0);
  emApiSetCtune(tune);
}
#endif // HAS_EMBER_SET_CTUNE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER
static void modemSetRadioHoldOffCommand(void)
{
  bool enabled;
  enabled = (bool) emberUnsignedCommandArgument(0);
  emApiSetRadioHoldOff(enabled);
}
#endif // HAS_EMBER_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_PTA_ENABLE_COMMAND_IDENTIFIER
static void modemGetPtaEnableCommand(void)
{
  emApiGetPtaEnable();
}
#endif // HAS_EMBER_GET_PTA_ENABLE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_PTA_ENABLE_COMMAND_IDENTIFIER
static void modemSetPtaEnableCommand(void)
{
  bool enabled;
  enabled = (bool) emberUnsignedCommandArgument(0);
  emApiSetPtaEnable(enabled);
}
#endif // HAS_EMBER_SET_PTA_ENABLE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_ANTENNA_MODE_COMMAND_IDENTIFIER
static void modemGetAntennaModeCommand(void)
{
  emApiGetAntennaMode();
}
#endif // HAS_EMBER_GET_ANTENNA_MODE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_ANTENNA_MODE_COMMAND_IDENTIFIER
static void modemSetAntennaModeCommand(void)
{
  uint8_t mode;
  mode = (uint8_t) emberUnsignedCommandArgument(0);
  emApiSetAntennaMode(mode);
}
#endif // HAS_EMBER_SET_ANTENNA_MODE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER
static void modemRadioGetRandomNumbersCommand(void)
{
  uint8_t count;
  count = (uint8_t) emberUnsignedCommandArgument(0);
  emApiRadioGetRandomNumbers(count);
}
#endif // HAS_EMBER_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_PTA_OPTIONS_COMMAND_IDENTIFIER
static void modemGetPtaOptionsCommand(void)
{
  emApiGetPtaOptions();
}
#endif // HAS_EMBER_GET_PTA_OPTIONS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_PTA_OPTIONS_COMMAND_IDENTIFIER
static void modemSetPtaOptionsCommand(void)
{
  uint32_t options;
  options = (uint32_t) emberUnsignedCommandArgument(0);
  emApiSetPtaOptions(options);
}
#endif // HAS_EMBER_SET_PTA_OPTIONS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER
static void modemNcpGetNetworkDataCommand(void)
{
  uint16_t bufferLength;
  bufferLength = (uint16_t) emberUnsignedCommandArgument(0);
  emApiNcpGetNetworkData(bufferLength);
}
#endif // HAS_EMBER_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_NOTE_EXTERNAL_COMMISSIONER_COMMAND_IDENTIFIER
static void modemNoteExternalCommissionerCommand(void)
{
  uint8_t commissionerId;
  bool available;
  commissionerId = (uint8_t) emberUnsignedCommandArgument(0);
  available = (bool) emberUnsignedCommandArgument(1);
  emStackNoteExternalCommissioner(commissionerId,
                                  available);
}
#endif // HAS_EMBER_NOTE_EXTERNAL_COMMISSIONER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_NCP_SET_ND_DATA_COMMAND_IDENTIFIER
static void modemNcpSetNdDataCommand(void)
{
  uint16_t totalLength;
  uint8_t *ndDataFragment;
  uint8_t fragmentLength;
  uint16_t fragmentOffset;
  totalLength = (uint16_t) emberUnsignedCommandArgument(0);
  ndDataFragment = emberStringCommandArgument(1, &fragmentLength);
  fragmentOffset = (uint16_t) emberUnsignedCommandArgument(2);
  emApiNcpSetNdData(totalLength,
                    ndDataFragment,
                    fragmentLength,
                    fragmentOffset);
}
#endif // HAS_EMBER_NCP_SET_ND_DATA_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_NCP_SET_LARGE_DATA_COMMAND_IDENTIFIER
static void modemNcpSetLargeDataCommand(void)
{
  SetLargeValueId valueId;
  uint16_t totalLength;
  uint8_t *fragment;
  uint8_t fragmentLength;
  uint16_t fragmentOffset;
  valueId = (SetLargeValueId) emberUnsignedCommandArgument(0);
  totalLength = (uint16_t) emberUnsignedCommandArgument(1);
  fragment = emberStringCommandArgument(2, &fragmentLength);
  fragmentOffset = (uint16_t) emberUnsignedCommandArgument(3);
  emApiNcpSetLargeData(valueId,
                       totalLength,
                       fragment,
                       fragmentLength,
                       fragmentOffset);
}
#endif // HAS_EMBER_NCP_SET_LARGE_DATA_COMMAND_IDENTIFIER

void emApiResetMicroHandler(EmberResetCause cause)
{
#ifdef HAS_CB_RESET_MICRO_COMMAND_IDENTIFIER
  bool preHookResult = tmspNcpResetMicroHandlerPreHook();
  if (preHookResult) {
    emSendBinaryManagementCommand(CB_RESET_MICRO_COMMAND_IDENTIFIER,
                                  "u",
                                  cause);
  }
#endif // HAS_CB_RESET_MICRO_COMMAND_IDENTIFIER
}

void emApiStateReturn(const EmberNetworkParameters *parameters,
                      const EmberEui64 *localEui64,
                      const EmberEui64 *macExtendedId,
                      EmberNetworkStatus networkStatus)
{
#ifdef HAS_CB_STATE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_STATE_COMMAND_IDENTIFIER,
                                "bbbvuusubbb",
                                parameters->networkId,
                                sizeof(parameters->networkId),
                                parameters->ulaPrefix.bytes,
                                sizeof(parameters->ulaPrefix.bytes),
                                parameters->extendedPanId,
                                sizeof(parameters->extendedPanId),
                                parameters->panId,
                                parameters->channel,
                                parameters->nodeType,
                                parameters->radioTxPower,
                                networkStatus,
                                localEui64->bytes,
                                sizeof(localEui64->bytes),
                                macExtendedId->bytes,
                                sizeof(macExtendedId->bytes),
                                parameters->legacyUla.bytes,
                                sizeof(parameters->legacyUla.bytes));
#endif // HAS_CB_STATE_COMMAND_IDENTIFIER
}

void emApiGetVersionsReturn(const uint8_t *versionName,
                            uint16_t managementVersionNumber,
                            uint16_t stackVersionNumber,
                            uint16_t stackBuildNumber,
                            EmberVersionType versionType,
                            const uint8_t *buildTimestamp)
{
#ifdef HAS_CB_GET_VERSIONS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_VERSIONS_COMMAND_IDENTIFIER,
                                "bvvvub",
                                versionName, emStrlen(versionName) + 1,
                                managementVersionNumber,
                                stackVersionNumber,
                                stackBuildNumber,
                                versionType,
                                buildTimestamp, emStrlen(buildTimestamp) + 1);
#endif // HAS_CB_GET_VERSIONS_COMMAND_IDENTIFIER
}

void emApiGetRipEntryReturn(uint8_t index,
                            const EmberRipEntry *entry)
{
#ifdef HAS_CB_GET_RIP_ENTRY_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_RIP_ENTRY_COMMAND_IDENTIFIER,
                                "ubusuuuuuu",
                                index,
                                entry->longId,
                                8,
                                entry->type,
                                entry->rollingRssi,
                                entry->nextHopIndex,
                                entry->ripMetric,
                                entry->incomingLinkQuality,
                                entry->outgoingLinkQuality,
                                entry->mleSync,
                                entry->age);
#endif // HAS_CB_GET_RIP_ENTRY_COMMAND_IDENTIFIER
}

void emApiInitReturn(EmberStatus status)
{
#ifdef HAS_CB_INIT_COMMAND_IDENTIFIER
  bool preHookResult = true;
  tmspNcpInitReturnPreHook();
  if (preHookResult) {
    emSendBinaryManagementCommand(CB_INIT_COMMAND_IDENTIFIER,
                                  "u",
                                  status);
  }
  tmspNcpInitReturnPostHook();
#endif // HAS_CB_INIT_COMMAND_IDENTIFIER
}

void emApiGetCounterReturn(EmberCounterType type,
                           uint16_t value)
{
#ifdef HAS_CB_GET_COUNTER_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_COUNTER_COMMAND_IDENTIFIER,
                                "uv",
                                type,
                                value);
#endif // HAS_CB_GET_COUNTER_COMMAND_IDENTIFIER
}

void emApiSetSecurityParametersReturn(EmberStatus status)
{
#ifdef HAS_CB_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER
}

void emApiSwitchToNextNetworkKeyReturn(EmberStatus status)
{
#ifdef HAS_CB_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER
}

void emApiFormNetworkReturn(EmberStatus status)
{
#ifdef HAS_CB_FORM_NETWORK_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_FORM_NETWORK_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_FORM_NETWORK_COMMAND_IDENTIFIER
}

void emApiJoinNetworkReturn(EmberStatus status)
{
#ifdef HAS_CB_JOIN_NETWORK_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_JOIN_NETWORK_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_JOIN_NETWORK_COMMAND_IDENTIFIER
}

void emApiResumeNetworkReturn(EmberStatus status)
{
#ifdef HAS_CB_RESUME_NETWORK_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_RESUME_NETWORK_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_RESUME_NETWORK_COMMAND_IDENTIFIER
}

void emApiAttachToNetworkReturn(EmberStatus status)
{
#ifdef HAS_CB_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER
}

void emApiEnergyScanHandler(uint8_t channel,
                            int8_t maxRssiValue)
{
#ifdef HAS_CB_ENERGY_SCAN_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_ENERGY_SCAN_COMMAND_IDENTIFIER,
                                "us",
                                channel,
                                maxRssiValue);
#endif // HAS_CB_ENERGY_SCAN_COMMAND_IDENTIFIER
}

void emApiActiveScanHandler(const EmberMacBeaconData *beaconData)
{
#ifdef HAS_CB_ACTIVE_SCAN_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_ACTIVE_SCAN_COMMAND_IDENTIFIER,
                                "uuuubvbbuubu",
                                beaconData->allowingJoin,
                                beaconData->channel,
                                beaconData->lqi,
                                beaconData->rssi,
                                beaconData->networkId,
                                sizeof(beaconData->networkId),
                                beaconData->panId,
                                beaconData->extendedPanId,
                                sizeof(beaconData->extendedPanId),
                                beaconData->longId,
                                sizeof(beaconData->longId),
                                beaconData->protocolId,
                                beaconData->version,
                                beaconData->steeringData,
                                sizeof(beaconData->steeringData),
                                beaconData->steeringDataLength);
#endif // HAS_CB_ACTIVE_SCAN_COMMAND_IDENTIFIER
}

void emApiScanReturn(EmberStatus status)
{
#ifdef HAS_CB_SCAN_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SCAN_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_SCAN_COMMAND_IDENTIFIER
}

void emApiSetAddressHandler(const uint8_t *address)
{
#ifdef HAS_CB_SET_ADDRESS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_ADDRESS_COMMAND_IDENTIFIER,
                                "b",
                                address, 16);
#endif // HAS_CB_SET_ADDRESS_COMMAND_IDENTIFIER
}

void emApiSetDriverAddressHandler(const uint8_t *address)
{
#ifdef HAS_CB_SET_DRIVER_ADDRESS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_DRIVER_ADDRESS_COMMAND_IDENTIFIER,
                                "b",
                                address, 16);
#endif // HAS_CB_SET_DRIVER_ADDRESS_COMMAND_IDENTIFIER
}

void emApiStartHostJoinClientHandler(const uint8_t *parentAddress)
{
#ifdef HAS_CB_START_HOST_JOIN_CLIENT_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_START_HOST_JOIN_CLIENT_COMMAND_IDENTIFIER,
                                "b",
                                parentAddress, 16);
#endif // HAS_CB_START_HOST_JOIN_CLIENT_COMMAND_IDENTIFIER
}

void emApiSetTxPowerModeReturn(EmberStatus status)
{
#ifdef HAS_CB_SET_TX_POWER_MODE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_TX_POWER_MODE_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_SET_TX_POWER_MODE_COMMAND_IDENTIFIER
}

void emApiGetTxPowerModeReturn(uint16_t txPowerMode)
{
#ifdef HAS_CB_GET_TX_POWER_MODE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_TX_POWER_MODE_COMMAND_IDENTIFIER,
                                "v",
                                txPowerMode);
#endif // HAS_CB_GET_TX_POWER_MODE_COMMAND_IDENTIFIER
}

void emApiSetNetworkKeysHandler(uint32_t sequence,
                                const uint8_t *masterKey,
                                uint32_t sequence2NotUsed,
                                const uint8_t *masterKey2NotUsed)
{
#ifdef HAS_CB_SET_NETWORK_KEYS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_NETWORK_KEYS_COMMAND_IDENTIFIER,
                                "wbwb",
                                sequence,
                                masterKey, EMBER_ENCRYPTION_KEY_SIZE,
                                sequence2NotUsed,
                                masterKey2NotUsed, EMBER_ENCRYPTION_KEY_SIZE);
#endif // HAS_CB_SET_NETWORK_KEYS_COMMAND_IDENTIFIER
}

void emApiGetMulticastEntryReturn(uint8_t lastSequence,
                                  uint8_t windowBitmask,
                                  uint8_t dwellQs,
                                  const uint8_t *seed)
{
#ifdef HAS_CB_GET_MULTICAST_ENTRY_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_MULTICAST_ENTRY_COMMAND_IDENTIFIER,
                                "uuub",
                                lastSequence,
                                windowBitmask,
                                dwellQs,
                                seed, 8);
#endif // HAS_CB_GET_MULTICAST_ENTRY_COMMAND_IDENTIFIER
}

void emApiGetCcaThresholdReturn(int8_t threshold)
{
#ifdef HAS_CB_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER,
                                "s",
                                threshold);
#endif // HAS_CB_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER
}

void emApiSetCcaThresholdReturn(EmberStatus status)
{
#ifdef HAS_CB_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER
}

void emApiGetRadioPowerReturn(int8_t power)
{
#ifdef HAS_CB_GET_RADIO_POWER_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_RADIO_POWER_COMMAND_IDENTIFIER,
                                "s",
                                power);
#endif // HAS_CB_GET_RADIO_POWER_COMMAND_IDENTIFIER
}

void emApiSetRadioPowerReturn(EmberStatus status)
{
#ifdef HAS_CB_SET_RADIO_POWER_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_RADIO_POWER_COMMAND_IDENTIFIER,
                                "u",
                                status);
  tmspNcpSetRadioPowerReturnPostHook();
#endif // HAS_CB_SET_RADIO_POWER_COMMAND_IDENTIFIER
}

void emApiEchoReturn(const uint8_t *data,
                     uint8_t length)
{
#ifdef HAS_CB_ECHO_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_ECHO_COMMAND_IDENTIFIER,
                                "b",
                                data, length);
#endif // HAS_CB_ECHO_COMMAND_IDENTIFIER
}

void emApiAssertInfoReturn(const uint8_t *fileName,
                           uint32_t line)
{
#ifdef HAS_CB_ASSERT_INFO_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_ASSERT_INFO_COMMAND_IDENTIFIER,
                                "bw",
                                fileName, emStrlen(fileName) + 1,
                                line);
#endif // HAS_CB_ASSERT_INFO_COMMAND_IDENTIFIER
}

void emApiConfigureGatewayReturn(EmberStatus status)
{
#ifdef HAS_CB_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER
}

void emApiGetChannelCalDataTokenReturn(uint8_t lna,
                                       int8_t tempAtLna,
                                       uint8_t modDac,
                                       int8_t tempAtModDac)
{
#ifdef HAS_CB_GET_CHANNEL_CAL_DATA_TOKEN_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_CHANNEL_CAL_DATA_TOKEN_COMMAND_IDENTIFIER,
                                "usus",
                                lna,
                                tempAtLna,
                                modDac,
                                tempAtModDac);
#endif // HAS_CB_GET_CHANNEL_CAL_DATA_TOKEN_COMMAND_IDENTIFIER
}

void emApiPollForDataReturn(EmberStatus status)
{
#ifdef HAS_CB_POLL_FOR_DATA_COMMAND_IDENTIFIER
  bool preHookResult = tmspNcpPollForDataReturnPreHook(status);
  if (preHookResult) {
    emSendBinaryManagementCommand(CB_POLL_FOR_DATA_COMMAND_IDENTIFIER,
                                  "u",
                                  status);
  }
  tmspNcpPollForDataReturnPostHook(status);
#endif // HAS_CB_POLL_FOR_DATA_COMMAND_IDENTIFIER
}

void emApiDeepSleepReturn(EmberStatus status)
{
#ifdef HAS_CB_DEEP_SLEEP_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_DEEP_SLEEP_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_DEEP_SLEEP_COMMAND_IDENTIFIER
}

void emApiStackPollForDataReturn(EmberStatus status)
{
#ifdef HAS_CB_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER
}

void emApiOkToNapReturn(uint8_t stateMask)
{
#ifdef HAS_CB_OK_TO_NAP_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_OK_TO_NAP_COMMAND_IDENTIFIER,
                                "u",
                                stateMask);
#endif // HAS_CB_OK_TO_NAP_COMMAND_IDENTIFIER
}

void emApiDeepSleepCompleteHandler(uint16_t sleepDuration)
{
#ifdef HAS_CB_DEEP_SLEEP_COMPLETE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_DEEP_SLEEP_COMPLETE_COMMAND_IDENTIFIER,
                                "v",
                                sleepDuration);
#endif // HAS_CB_DEEP_SLEEP_COMPLETE_COMMAND_IDENTIFIER
}

void emApiResetNetworkStateReturn(EmberStatus status)
{
#ifdef HAS_CB_RESET_NETWORK_STATE_COMMAND_IDENTIFIER
  bool preHookResult = tmspNcpResetNetworkStateReturnPreHook(status);
  if (preHookResult) {
    emSendBinaryManagementCommand(CB_RESET_NETWORK_STATE_COMMAND_IDENTIFIER,
                                  "u",
                                  status);
  }
#endif // HAS_CB_RESET_NETWORK_STATE_COMMAND_IDENTIFIER
}

void emApiExternalRouteChangeHandler(const uint8_t *prefix,
                                     uint8_t prefixLengthInBits,
                                     bool available)
{
#ifdef HAS_CB_EXTERNAL_ROUTE_CHANGE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_EXTERNAL_ROUTE_CHANGE_COMMAND_IDENTIFIER,
                                "buu",
                                prefix, 16,
                                prefixLengthInBits,
                                available);
#endif // HAS_CB_EXTERNAL_ROUTE_CHANGE_COMMAND_IDENTIFIER
}

void emApiDhcpServerChangeHandler(const uint8_t *prefix,
                                  uint8_t prefixLengthInBits,
                                  bool available)
{
#ifdef HAS_CB_DHCP_SERVER_CHANGE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_DHCP_SERVER_CHANGE_COMMAND_IDENTIFIER,
                                "buu",
                                prefix, 16,
                                prefixLengthInBits,
                                available);
#endif // HAS_CB_DHCP_SERVER_CHANGE_COMMAND_IDENTIFIER
}

void emApiNcpNetworkDataChangeHandler(uint16_t length,
                                      const uint8_t *networkData,
                                      uint8_t bytesSent)
{
#ifdef HAS_CB_NCP_NETWORK_DATA_CHANGE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_NCP_NETWORK_DATA_CHANGE_COMMAND_IDENTIFIER,
                                "vb",
                                length,
                                networkData, bytesSent);
#endif // HAS_CB_NCP_NETWORK_DATA_CHANGE_COMMAND_IDENTIFIER
}

void emApiNcpGetNetworkDataReturn(EmberStatus status,
                                  uint16_t totalLength,
                                  const uint8_t *networkDataFragment,
                                  uint8_t fragmentLength,
                                  uint16_t fragmentOffset)
{
#ifdef HAS_CB_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER,
                                "uvbv",
                                status,
                                totalLength,
                                networkDataFragment, fragmentLength,
                                fragmentOffset);
#endif // HAS_CB_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER
}

void emApiAddressConfigurationChangeHandler(const EmberIpv6Address *address,
                                            uint32_t preferredLifetime,
                                            uint32_t validLifetime,
                                            uint8_t addressFlags)
{
#ifdef HAS_CB_ADDRESS_CONFIGURATION_CHANGE_COMMAND_IDENTIFIER
  bool preHookResult =
    tmspNcpAddressConfigurationChangePreHook(address);
  if (preHookResult) {
    emSendBinaryManagementCommand(CB_ADDRESS_CONFIGURATION_CHANGE_COMMAND_IDENTIFIER,
                                  "bwwu",
                                  address, sizeof(EmberIpv6Address),
                                  preferredLifetime,
                                  validLifetime,
                                  addressFlags);
  }
#endif // HAS_CB_ADDRESS_CONFIGURATION_CHANGE_COMMAND_IDENTIFIER
}

void emApiRequestDhcpAddressReturn(EmberStatus status,
                                   const uint8_t *prefix,
                                   uint8_t prefixLengthInBits)
{
#ifdef HAS_CB_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER,
                                "ubu",
                                status,
                                prefix, 16,
                                prefixLengthInBits);
#endif // HAS_CB_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER
}

void emApiCommissionNetworkReturn(EmberStatus status)
{
#ifdef HAS_CB_COMMISSION_NETWORK_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_COMMISSION_NETWORK_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_COMMISSION_NETWORK_COMMAND_IDENTIFIER
}

void emApiGetGlobalAddressReturn(const EmberIpv6Address *address,
                                 uint32_t preferredLifetime,
                                 uint32_t validLifetime,
                                 uint8_t addressFlags)
{
#ifdef HAS_CB_GET_GLOBAL_ADDRESS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_GLOBAL_ADDRESS_COMMAND_IDENTIFIER,
                                "bwwu",
                                address->bytes,
                                EMBER_NETWORK_ID_SIZE,
                                preferredLifetime,
                                validLifetime,
                                addressFlags);
#endif // HAS_CB_GET_GLOBAL_ADDRESS_COMMAND_IDENTIFIER
}

void emApiGetDhcpClientReturn(const EmberIpv6Address *address)
{
#ifdef HAS_CB_GET_DHCP_CLIENT_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_DHCP_CLIENT_COMMAND_IDENTIFIER,
                                "b",
                                address->bytes,
                                sizeof(address->bytes));
#endif // HAS_CB_GET_DHCP_CLIENT_COMMAND_IDENTIFIER
}

void emApiBecomeCommissionerReturn(EmberStatus status)
{
#ifdef HAS_CB_BECOME_COMMISSIONER_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_BECOME_COMMISSIONER_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_BECOME_COMMISSIONER_COMMAND_IDENTIFIER
}

void emApiSendSteeringDataReturn(EmberStatus status)
{
#ifdef HAS_CB_SEND_STEERING_DATA_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SEND_STEERING_DATA_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_SEND_STEERING_DATA_COMMAND_IDENTIFIER
}

void emApiChangeNodeTypeReturn(EmberStatus status)
{
#ifdef HAS_CB_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER
}

void emApiGetGlobalPrefixReturn(uint8_t borderRouterFlags,
                                bool isStable,
                                const uint8_t *prefix,
                                uint8_t prefixLengthInBits,
                                uint8_t domainId,
                                uint32_t preferredLifetime,
                                uint32_t validLifetime)
{
#ifdef HAS_CB_GET_GLOBAL_PREFIX_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_GLOBAL_PREFIX_COMMAND_IDENTIFIER,
                                "uubuuww",
                                borderRouterFlags,
                                isStable,
                                prefix, 16,
                                prefixLengthInBits,
                                domainId,
                                preferredLifetime,
                                validLifetime);
#endif // HAS_CB_GET_GLOBAL_PREFIX_COMMAND_IDENTIFIER
}

void emApiResignGlobalAddressReturn(EmberStatus status)
{
#ifdef HAS_CB_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER
}

void emApiSlaacServerChangeHandler(const uint8_t *prefix,
                                   uint8_t prefixLengthInBits,
                                   bool available)
{
#ifdef HAS_CB_SLAAC_SERVER_CHANGE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SLAAC_SERVER_CHANGE_COMMAND_IDENTIFIER,
                                "buu",
                                prefix, 16,
                                prefixLengthInBits,
                                available);
#endif // HAS_CB_SLAAC_SERVER_CHANGE_COMMAND_IDENTIFIER
}

void emApiRequestSlaacAddressReturn(EmberStatus status,
                                    const uint8_t *prefix,
                                    uint8_t prefixLengthInBits)
{
#ifdef HAS_CB_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER,
                                "ubu",
                                status,
                                prefix, 16,
                                prefixLengthInBits);
#endif // HAS_CB_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER
}

void emApiCustomNcpToHostMessage(const uint8_t *message,
                                 uint8_t messageLength)
{
#ifdef HAS_CB_CUSTOM_NCP_TO_HOST_MESSAGE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_CUSTOM_NCP_TO_HOST_MESSAGE_COMMAND_IDENTIFIER,
                                "b",
                                message, messageLength);
#endif // HAS_CB_CUSTOM_NCP_TO_HOST_MESSAGE_COMMAND_IDENTIFIER
}

void emApiNcpToHostNoOp(const uint8_t *bytes,
                        uint8_t bytesLength)
{
#ifdef HAS_CB_NCP_TO_HOST_NO_OP_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_NCP_TO_HOST_NO_OP_COMMAND_IDENTIFIER,
                                "b",
                                bytes, bytesLength);
#endif // HAS_CB_NCP_TO_HOST_NO_OP_COMMAND_IDENTIFIER
}

void emApiCommissionerStatusHandler(uint16_t flags,
                                    const uint8_t *commissionerName,
                                    uint8_t commissionerNameLength)
{
#ifdef HAS_CB_COMMISSIONER_STATUS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_COMMISSIONER_STATUS_COMMAND_IDENTIFIER,
                                "vb",
                                flags,
                                commissionerName, commissionerNameLength);
#endif // HAS_CB_COMMISSIONER_STATUS_COMMAND_IDENTIFIER
}

void emApiLeaderDataHandler(const uint8_t *leaderData)
{
#ifdef HAS_CB_LEADER_DATA_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_LEADER_DATA_COMMAND_IDENTIFIER,
                                "b",
                                leaderData, EMBER_NETWORK_DATA_LEADER_SIZE);
#endif // HAS_CB_LEADER_DATA_COMMAND_IDENTIFIER
}

void emApiGetNetworkDataTlvReturn(uint8_t type,
                                  uint8_t index,
                                  uint8_t versionNumber,
                                  const uint8_t *tlv,
                                  uint8_t tlvLength)
{
#ifdef HAS_CB_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER,
                                "uuub",
                                type,
                                index,
                                versionNumber,
                                tlv, tlvLength);
#endif // HAS_CB_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER
}

void emApiGetRoutingLocatorReturn(const EmberIpv6Address *rloc)
{
#ifdef HAS_CB_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER,
                                "b",
                                rloc, sizeof(EmberIpv6Address));
#endif // HAS_CB_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER
}

void emApiSetRandomizeMacExtendedIdReturn(void)
{
#ifdef HAS_CB_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER,
                                "");
#endif // HAS_CB_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER
}

void emApiConfigureExternalRouteReturn(EmberStatus status)
{
#ifdef HAS_CB_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER
}

void emApiAllowNativeCommissionerReturn(EmberStatus status)
{
#ifdef HAS_CB_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER
}

void emApiSetCommissionerKeyReturn(EmberStatus status)
{
#ifdef HAS_CB_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER
}

void emApiSetCommProxyAppParametersHandler(const uint8_t *extendedPanId,
                                           const uint8_t *networkId,
                                           const uint8_t *ulaPrefix,
                                           uint16_t panId,
                                           uint8_t channel,
                                           const EmberEui64 *eui64,
                                           const EmberEui64 *macExtendedId,
                                           EmberNetworkStatus networkStatus)
{
#ifdef HAS_CB_SET_COMM_PROXY_APP_PARAMETERS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_COMM_PROXY_APP_PARAMETERS_COMMAND_IDENTIFIER,
                                "bbbvubbu",
                                extendedPanId, EXTENDED_PAN_ID_SIZE,
                                networkId, EMBER_NETWORK_ID_SIZE,
                                ulaPrefix, 8,
                                panId,
                                channel,
                                eui64, sizeof(EmberEui64),
                                macExtendedId, sizeof(EmberEui64),
                                networkStatus);
#endif // HAS_CB_SET_COMM_PROXY_APP_PARAMETERS_COMMAND_IDENTIFIER
}

void emApiSetCommProxyAppSecurityHandler(const uint8_t *masterKey,
                                         uint32_t sequence)
{
#ifdef HAS_CB_SET_COMM_PROXY_APP_SECURITY_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_COMM_PROXY_APP_SECURITY_COMMAND_IDENTIFIER,
                                "bw",
                                masterKey, EMBER_ENCRYPTION_KEY_SIZE,
                                sequence);
#endif // HAS_CB_SET_COMM_PROXY_APP_SECURITY_COMMAND_IDENTIFIER
}

void emApiSetCommProxyAppAddressHandler(const uint8_t *address)
{
#ifdef HAS_CB_SET_COMM_PROXY_APP_ADDRESS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_COMM_PROXY_APP_ADDRESS_COMMAND_IDENTIFIER,
                                "b",
                                address, 16);
#endif // HAS_CB_SET_COMM_PROXY_APP_ADDRESS_COMMAND_IDENTIFIER
}

void emApiNetworkStatusHandler(EmberNetworkStatus newNetworkStatus,
                               EmberNetworkStatus oldNetworkStatus,
                               EmberJoinFailureReason reason)
{
#ifdef HAS_CB_NETWORK_STATUS_COMMAND_IDENTIFIER
  bool preHookResult = tmspNcpNetworkStatusHandlerPreHook(newNetworkStatus);
  if (preHookResult) {
    emSendBinaryManagementCommand(CB_NETWORK_STATUS_COMMAND_IDENTIFIER,
                                  "uuu",
                                  newNetworkStatus,
                                  oldNetworkStatus,
                                  reason);
  }
#endif // HAS_CB_NETWORK_STATUS_COMMAND_IDENTIFIER
}

void emApiGetStandaloneBootloaderInfoReturn(uint16_t version,
                                            uint8_t platformId,
                                            uint8_t microId,
                                            uint8_t phyId)
{
#ifdef HAS_CB_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER,
                                "vuuu",
                                version,
                                platformId,
                                microId,
                                phyId);
#endif // HAS_CB_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER
}

void emApiLaunchStandaloneBootloaderReturn(EmberStatus status)
{
#ifdef HAS_CB_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER
}

void emApiGetMfgTokenReturn(EmberMfgTokenId tokenId,
                            EmberStatus status,
                            const uint8_t *tokenData,
                            uint8_t tokenDataLength)
{
#ifdef HAS_CB_GET_MFG_TOKEN_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_MFG_TOKEN_COMMAND_IDENTIFIER,
                                "uub",
                                tokenId,
                                status,
                                tokenData, tokenDataLength);
#endif // HAS_CB_GET_MFG_TOKEN_COMMAND_IDENTIFIER
}

void emApiSetMfgTokenReturn(EmberMfgTokenId tokenId,
                            EmberStatus status)
{
#ifdef HAS_CB_SET_MFG_TOKEN_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_MFG_TOKEN_COMMAND_IDENTIFIER,
                                "uu",
                                tokenId,
                                status);
#endif // HAS_CB_SET_MFG_TOKEN_COMMAND_IDENTIFIER
}

void emApiHostStateHandler(const EmberNetworkParameters *parameters,
                           const EmberEui64 *localEui64,
                           const EmberEui64 *macExtendedId,
                           EmberNetworkStatus networkStatus)
{
#ifdef HAS_CB_HOST_STATE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_HOST_STATE_COMMAND_IDENTIFIER,
                                "bbbvuusubbb",
                                parameters->networkId,
                                sizeof(parameters->networkId),
                                parameters->ulaPrefix.bytes,
                                sizeof(parameters->ulaPrefix.bytes),
                                parameters->extendedPanId,
                                sizeof(parameters->extendedPanId),
                                parameters->panId,
                                parameters->channel,
                                parameters->nodeType,
                                parameters->radioTxPower,
                                networkStatus,
                                localEui64->bytes,
                                sizeof(localEui64->bytes),
                                macExtendedId->bytes,
                                sizeof(macExtendedId->bytes),
                                parameters->legacyUla.bytes,
                                sizeof(parameters->legacyUla.bytes));
#endif // HAS_CB_HOST_STATE_COMMAND_IDENTIFIER
}

void emApiGetCtuneReturn(uint16_t tune,
                         EmberStatus status)
{
#ifdef HAS_CB_GET_CTUNE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_CTUNE_COMMAND_IDENTIFIER,
                                "vu",
                                tune,
                                status);
#endif // HAS_CB_GET_CTUNE_COMMAND_IDENTIFIER
}

void emApiSetCtuneReturn(EmberStatus status)
{
#ifdef HAS_CB_SET_CTUNE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_CTUNE_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_SET_CTUNE_COMMAND_IDENTIFIER
}

void emApiSetRadioHoldOffReturn(EmberStatus status)
{
#ifdef HAS_CB_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER
}

void emApiGetPtaEnableReturn(bool enabled)
{
#ifdef HAS_CB_GET_PTA_ENABLE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_PTA_ENABLE_COMMAND_IDENTIFIER,
                                "u",
                                enabled);
#endif // HAS_CB_GET_PTA_ENABLE_COMMAND_IDENTIFIER
}

void emApiSetPtaEnableReturn(EmberStatus status)
{
#ifdef HAS_CB_SET_PTA_ENABLE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_PTA_ENABLE_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_SET_PTA_ENABLE_COMMAND_IDENTIFIER
}

void emApiGetAntennaModeReturn(EmberStatus status,
                               uint8_t mode)
{
#ifdef HAS_CB_GET_ANTENNA_MODE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_ANTENNA_MODE_COMMAND_IDENTIFIER,
                                "uu",
                                status,
                                mode);
#endif // HAS_CB_GET_ANTENNA_MODE_COMMAND_IDENTIFIER
}

void emApiSetAntennaModeReturn(EmberStatus status)
{
#ifdef HAS_CB_SET_ANTENNA_MODE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_ANTENNA_MODE_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_SET_ANTENNA_MODE_COMMAND_IDENTIFIER
}

void emApiRadioGetRandomNumbersReturn(EmberStatus status,
                                      const uint16_t *rn,
                                      uint8_t count)
{
#ifdef HAS_CB_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER,
                                "ubu",
                                status,
                                rn, (count * sizeof(uint16_t)),
                                count);
#endif // HAS_CB_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER
}

void emApiGetPtaOptionsReturn(uint32_t options)
{
#ifdef HAS_CB_GET_PTA_OPTIONS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_PTA_OPTIONS_COMMAND_IDENTIFIER,
                                "w",
                                options);
#endif // HAS_CB_GET_PTA_OPTIONS_COMMAND_IDENTIFIER
}

void emApiSetPtaOptionsReturn(EmberStatus status)
{
#ifdef HAS_CB_SET_PTA_OPTIONS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_PTA_OPTIONS_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_SET_PTA_OPTIONS_COMMAND_IDENTIFIER
}

void emApiSetJoinKeyReturn(EmberStatus status)
{
#ifdef HAS_CB_SET_JOIN_KEY_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_JOIN_KEY_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_SET_JOIN_KEY_COMMAND_IDENTIFIER
}

void emApiSetNdDataReturn(EmberNetworkStatus status,
                          uint16_t length)
{
#ifdef HAS_CB_SET_ND_DATA_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_ND_DATA_COMMAND_IDENTIFIER,
                                "uv",
                                status,
                                length);
#endif // HAS_CB_SET_ND_DATA_COMMAND_IDENTIFIER
}

void emApiSetLocalNetworkDataReturn(EmberNetworkStatus status,
                                    uint16_t length)
{
#ifdef HAS_CB_SET_LOCAL_NETWORK_DATA_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_LOCAL_NETWORK_DATA_COMMAND_IDENTIFIER,
                                "uv",
                                status,
                                length);
#endif // HAS_CB_SET_LOCAL_NETWORK_DATA_COMMAND_IDENTIFIER
}

void emApiSetCommProxyAppPskcHandler(const uint8_t *pskc)
{
#ifdef HAS_CB_SET_COMM_PROXY_APP_PSKC_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_COMM_PROXY_APP_PSKC_COMMAND_IDENTIFIER,
                                "b",
                                pskc, 16);
#endif // HAS_CB_SET_COMM_PROXY_APP_PSKC_COMMAND_IDENTIFIER
}

void emApiSetPskcHandler(const uint8_t *pskc)
{
#ifdef HAS_CB_SET_PSKC_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SET_PSKC_COMMAND_IDENTIFIER,
                                "b",
                                pskc, 16);
  sendCommProxyAppPskcHandlerCallback(pskc);
#endif // HAS_CB_SET_PSKC_COMMAND_IDENTIFIER
}

//------------------------------------------------------------------------------
// MFGLIB
//------------------------------------------------------------------------------
#ifdef MFGLIB

#ifdef HAS_EMBER_MFGLIB_START_COMMAND_IDENTIFIER
static void modemMfglibStartCommand(void)
{
  bool requestRxCallback;
  requestRxCallback = (bool) emberUnsignedCommandArgument(0);
  tmspNcpMfglibStart(requestRxCallback);
}
#endif // HAS_EMBER_MFGLIB_START_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_MFGLIB_END_COMMAND_IDENTIFIER
static void modemMfglibEndCommand(void)
{
  tmspNcpMfglibEnd();
}
#endif // HAS_EMBER_MFGLIB_END_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_MFGLIB_START_ACTIVITY_COMMAND_IDENTIFIER
static void modemMfglibStartActivityCommand(void)
{
  TmspMfglibActivities type;
  type = (TmspMfglibActivities) emberUnsignedCommandArgument(0);
  tmspNcpMfglibStartActivity(type);
}
#endif // HAS_EMBER_MFGLIB_START_ACTIVITY_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_MFGLIB_STOP_ACTIVITY_COMMAND_IDENTIFIER
static void modemMfglibStopActivityCommand(void)
{
  TmspMfglibActivities type;
  type = (TmspMfglibActivities) emberUnsignedCommandArgument(0);
  tmspNcpMfglibStopActivity(type);
}
#endif // HAS_EMBER_MFGLIB_STOP_ACTIVITY_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_MFGLIB_SEND_PACKET_COMMAND_IDENTIFIER
static void modemMfglibSendPacketCommand(void)
{
  uint8_t *packet;
  uint8_t packetLength;
  uint16_t repeat;
  packet = emberStringCommandArgument(0, &packetLength);
  repeat = (uint16_t) emberUnsignedCommandArgument(1);
  tmspNcpMfglibSendPacket(packet,
                          packetLength,
                          repeat);
}
#endif // HAS_EMBER_MFGLIB_SEND_PACKET_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_MFGLIB_SET_COMMAND_IDENTIFIER
static void modemMfglibSetCommand(void)
{
  TmspMfglibValues type;
  uint16_t arg1;
  int8_t arg2;
  type = (TmspMfglibValues) emberUnsignedCommandArgument(0);
  arg1 = (uint16_t) emberUnsignedCommandArgument(1);
  arg2 = (int8_t) emberSignedCommandArgument(2);
  tmspNcpMfglibSet(type,
                   arg1,
                   arg2);
}
#endif // HAS_EMBER_MFGLIB_SET_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_MFGLIB_GET_COMMAND_IDENTIFIER
static void modemMfglibGetCommand(void)
{
  TmspMfglibValues type;
  type = (TmspMfglibValues) emberUnsignedCommandArgument(0);
  tmspNcpMfglibGet(type);
}
#endif // HAS_EMBER_MFGLIB_GET_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_MFGLIB_TEST_CONT_MOD_CAL_COMMAND_IDENTIFIER
static void modemMfglibTestContModCalCommand(void)
{
  uint8_t channel;
  uint32_t duration;
  channel = (uint8_t) emberUnsignedCommandArgument(0);
  duration = (uint32_t) emberUnsignedCommandArgument(1);
  tmspNcpMfglibTestContModCal(channel,
                              duration);
}
#endif // HAS_EMBER_MFGLIB_TEST_CONT_MOD_CAL_COMMAND_IDENTIFIER

void tmspNcpMfglibStartTestReturn(EmberStatus status)
{
#ifdef HAS_CB_MFGLIB_START_TEST_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_MFGLIB_START_TEST_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_MFGLIB_START_TEST_COMMAND_IDENTIFIER
}

void tmspNcpMfglibRxReturn(const uint8_t *payload,
                           uint8_t payloadLength,
                           uint8_t lqi,
                           int8_t rssi)
{
#ifdef HAS_CB_MFGLIB_RX_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_MFGLIB_RX_COMMAND_IDENTIFIER,
                                "bus",
                                payload, payloadLength,
                                lqi,
                                rssi);
#endif // HAS_CB_MFGLIB_RX_COMMAND_IDENTIFIER
}

void tmspNcpMfglibEndTestReturn(EmberStatus status,
                                uint32_t mfgReceiveCount)
{
#ifdef HAS_CB_MFGLIB_END_TEST_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_MFGLIB_END_TEST_COMMAND_IDENTIFIER,
                                "uw",
                                status,
                                mfgReceiveCount);
#endif // HAS_CB_MFGLIB_END_TEST_COMMAND_IDENTIFIER
}

void tmspNcpMfglibStartReturn(TmspMfglibActivities type,
                              EmberStatus status)
{
#ifdef HAS_CB_MFGLIB_START_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_MFGLIB_START_COMMAND_IDENTIFIER,
                                "uu",
                                type,
                                status);
#endif // HAS_CB_MFGLIB_START_COMMAND_IDENTIFIER
}

void tmspNcpMfglibStopReturn(TmspMfglibActivities type,
                             EmberStatus status)
{
#ifdef HAS_CB_MFGLIB_STOP_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_MFGLIB_STOP_COMMAND_IDENTIFIER,
                                "uu",
                                type,
                                status);
#endif // HAS_CB_MFGLIB_STOP_COMMAND_IDENTIFIER
}

void tmspNcpMfglibSendPacketEventHandler(EmberStatus status)
{
#ifdef HAS_CB_MFGLIB_SEND_PACKET_EVENT_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_MFGLIB_SEND_PACKET_EVENT_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_MFGLIB_SEND_PACKET_EVENT_COMMAND_IDENTIFIER
}

void tmspNcpMfglibSetReturn(TmspMfglibValues type,
                            EmberStatus status)
{
#ifdef HAS_CB_MFGLIB_SET_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_MFGLIB_SET_COMMAND_IDENTIFIER,
                                "uu",
                                type,
                                status);
#endif // HAS_CB_MFGLIB_SET_COMMAND_IDENTIFIER
}

void tmspNcpMfglibGetChannelReturn(uint8_t channel)
{
#ifdef HAS_CB_MFGLIB_GET_CHANNEL_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_MFGLIB_GET_CHANNEL_COMMAND_IDENTIFIER,
                                "u",
                                channel);
#endif // HAS_CB_MFGLIB_GET_CHANNEL_COMMAND_IDENTIFIER
}

void tmspNcpMfglibGetPowerReturn(int8_t power)
{
#ifdef HAS_CB_MFGLIB_GET_POWER_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_MFGLIB_GET_POWER_COMMAND_IDENTIFIER,
                                "s",
                                power);
#endif // HAS_CB_MFGLIB_GET_POWER_COMMAND_IDENTIFIER
}

void tmspNcpMfglibGetPowerModeReturn(uint16_t txPowerMode)
{
#ifdef HAS_CB_MFGLIB_GET_POWER_MODE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_MFGLIB_GET_POWER_MODE_COMMAND_IDENTIFIER,
                                "v",
                                txPowerMode);
#endif // HAS_CB_MFGLIB_GET_POWER_MODE_COMMAND_IDENTIFIER
}

void tmspNcpMfglibGetSynOffsetReturn(int8_t synOffset)
{
#ifdef HAS_CB_MFGLIB_GET_SYN_OFFSET_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_MFGLIB_GET_SYN_OFFSET_COMMAND_IDENTIFIER,
                                "s",
                                synOffset);
#endif // HAS_CB_MFGLIB_GET_SYN_OFFSET_COMMAND_IDENTIFIER
}

void tmspNcpMfglibGetOptionsReturn(uint8_t options)
{
#ifdef HAS_CB_MFGLIB_GET_OPTIONS_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_MFGLIB_GET_OPTIONS_COMMAND_IDENTIFIER,
                                "u",
                                options);
#endif // HAS_CB_MFGLIB_GET_OPTIONS_COMMAND_IDENTIFIER
}
#endif /* MFGLIB */

//------------------------------------------------------------------------------
// EMBER_TEST
//------------------------------------------------------------------------------
#ifdef EMBER_TEST

#ifdef HAS_EMBER_CONFIG_UART_COMMAND_IDENTIFIER
static void modemConfigUartCommand(void)
{
  uint8_t dropPercentage;
  uint8_t corruptPercentage;
  dropPercentage = (uint8_t) emberUnsignedCommandArgument(0);
  corruptPercentage = (uint8_t) emberUnsignedCommandArgument(1);
  emApiConfigUart(dropPercentage,
                  corruptPercentage);
}
#endif // HAS_EMBER_CONFIG_UART_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_RESET_NCP_ASH_COMMAND_IDENTIFIER
static void modemResetNcpAshCommand(void)
{
  emApiResetNcpAsh();
}
#endif // HAS_EMBER_RESET_NCP_ASH_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_START_UART_STORM_COMMAND_IDENTIFIER
static void modemStartUartStormCommand(void)
{
  uint16_t rate;
  rate = (uint16_t) emberUnsignedCommandArgument(0);
  emApiStartUartStorm(rate);
}
#endif // HAS_EMBER_START_UART_STORM_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_STOP_UART_STORM_COMMAND_IDENTIFIER
static void modemStopUartStormCommand(void)
{
  emApiStopUartStorm();
}
#endif // HAS_EMBER_STOP_UART_STORM_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SEND_DONE_COMMAND_IDENTIFIER
static void modemSendDoneCommand(void)
{
  uint32_t timeoutMs;
  timeoutMs = (uint32_t) emberUnsignedCommandArgument(0);
  emApiSendDone(timeoutMs);
}
#endif // HAS_EMBER_SEND_DONE_COMMAND_IDENTIFIER

void emApiConfigUartReturn(void)
{
#ifdef HAS_CB_CONFIG_UART_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_CONFIG_UART_COMMAND_IDENTIFIER,
                                "");
#endif // HAS_CB_CONFIG_UART_COMMAND_IDENTIFIER
}

void emApiResetNcpAshReturn(void)
{
#ifdef HAS_CB_RESET_NCP_ASH_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_RESET_NCP_ASH_COMMAND_IDENTIFIER,
                                "");
#endif // HAS_CB_RESET_NCP_ASH_COMMAND_IDENTIFIER
}

void emApiStartUartStormReturn(void)
{
#ifdef HAS_CB_START_UART_STORM_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_START_UART_STORM_COMMAND_IDENTIFIER,
                                "");
#endif // HAS_CB_START_UART_STORM_COMMAND_IDENTIFIER
}

void emApiStopUartStormReturn(void)
{
#ifdef HAS_CB_STOP_UART_STORM_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_STOP_UART_STORM_COMMAND_IDENTIFIER,
                                "");
#endif // HAS_CB_STOP_UART_STORM_COMMAND_IDENTIFIER
}

void emApiSendDoneReturn(void)
{
#ifdef HAS_CB_SEND_DONE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_SEND_DONE_COMMAND_IDENTIFIER,
                                "");
#endif // HAS_CB_SEND_DONE_COMMAND_IDENTIFIER
}
#endif /* EMBER_TEST */

//------------------------------------------------------------------------------
// QA_THREAD_TEST
//------------------------------------------------------------------------------
#if (defined (EMBER_TEST) || defined(QA_THREAD_TEST))

#ifdef HAS_EMBER_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER
static void modemGetNetworkKeyInfoCommand(void)
{
  bool keyInUse;
  keyInUse = (bool) emberUnsignedCommandArgument(0);
  emApiGetNetworkKeyInfo(keyInUse);
}
#endif // HAS_EMBER_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_FORCE_ASSERT_COMMAND_IDENTIFIER
static void modemForceAssertCommand(void)
{
  emApiForceAssert();
}
#endif // HAS_EMBER_FORCE_ASSERT_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_NODE_STATUS_COMMAND_IDENTIFIER
static void modemGetNodeStatusCommand(void)
{
  emApiGetNodeStatus();
}
#endif // HAS_EMBER_GET_NODE_STATUS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER
static void modemAddAddressDataCommand(void)
{
  uint8_t longId[8];
  uint16_t shortId;
  emberGetStringArgument(0, longId, 8, false);
  shortId = (uint16_t) emberUnsignedCommandArgument(1);
  emApiAddAddressData(longId,
                      shortId);
}
#endif // HAS_EMBER_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER
static void modemClearAddressCacheCommand(void)
{
  emApiClearAddressCache();
}
#endif // HAS_EMBER_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER
static void modemLookupAddressDataCommand(void)
{
  uint8_t longId[8];
  emberGetStringArgument(0, longId, 8, false);
  emApiLookupAddressData(longId);
}
#endif // HAS_EMBER_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_START_UART_SPEED_TEST_COMMAND_IDENTIFIER
static void modemStartUartSpeedTestCommand(void)
{
  uint8_t payloadLength;
  uint32_t timeoutMs;
  uint32_t intervalMs;
  payloadLength = (uint8_t) emberUnsignedCommandArgument(0);
  timeoutMs = (uint32_t) emberUnsignedCommandArgument(1);
  intervalMs = (uint32_t) emberUnsignedCommandArgument(2);
  emApiStartUartSpeedTest(payloadLength,
                          timeoutMs,
                          intervalMs);
}
#endif // HAS_EMBER_START_UART_SPEED_TEST_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_NCP_UDP_STORM_COMMAND_IDENTIFIER
static void modemNcpUdpStormCommand(void)
{
  uint8_t totalPackets;
  uint8_t dest[16];
  uint16_t payloadLength;
  uint16_t txDelayMs;
  totalPackets = (uint8_t) emberUnsignedCommandArgument(0);
  emberGetStringArgument(1, dest, 16, false);
  payloadLength = (uint16_t) emberUnsignedCommandArgument(2);
  txDelayMs = (uint16_t) emberUnsignedCommandArgument(3);
  emApiNcpUdpStorm(totalPackets,
                   dest,
                   payloadLength,
                   txDelayMs);
}
#endif // HAS_EMBER_NCP_UDP_STORM_COMMAND_IDENTIFIER

void emApiGetNetworkKeyInfoReturn(EmberStatus status,
                                  uint32_t sequence,
                                  uint8_t state)
{
#ifdef HAS_CB_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER,
                                "uwu",
                                status,
                                sequence,
                                state);
#endif // HAS_CB_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER
}

void emApiGetNodeStatusReturn(EmberStatus status,
                              uint8_t ripId,
                              EmberNodeId nodeId,
                              uint8_t parentRipId,
                              EmberNodeId parentId,
                              const uint8_t *networkFragmentIdentifier,
                              uint32_t networkFrameCounter)
{
#ifdef HAS_CB_GET_NODE_STATUS_COMMAND_IDENTIFIER
  uint8_t identifier[ISLAND_ID_SIZE] = { 0 };
  bool preHookResult = tmspNcpNodeStatusReturnPreHook(&networkFragmentIdentifier, identifier);
  if (preHookResult) {
    emSendBinaryManagementCommand(CB_GET_NODE_STATUS_COMMAND_IDENTIFIER,
                                  "uuvuvbw",
                                  status,
                                  ripId,
                                  nodeId,
                                  parentRipId,
                                  parentId,
                                  networkFragmentIdentifier, ISLAND_ID_SIZE,
                                  networkFrameCounter);
  }
#endif // HAS_CB_GET_NODE_STATUS_COMMAND_IDENTIFIER
}

void emApiAddAddressDataReturn(uint16_t shortId)
{
#ifdef HAS_CB_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER,
                                "v",
                                shortId);
#endif // HAS_CB_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER
}

void emApiClearAddressCacheReturn(void)
{
#ifdef HAS_CB_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER,
                                "");
#endif // HAS_CB_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER
}

void emApiLookupAddressDataReturn(uint16_t shortId)
{
#ifdef HAS_CB_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER,
                                "v",
                                shortId);
#endif // HAS_CB_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER
}

void emApiUartSpeedTestReturn(uint32_t totalBytesSent,
                              uint32_t payloadBytesSent,
                              uint32_t uartStormTimeoutMs)
{
#ifdef HAS_CB_UART_SPEED_TEST_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_UART_SPEED_TEST_COMMAND_IDENTIFIER,
                                "www",
                                totalBytesSent,
                                payloadBytesSent,
                                uartStormTimeoutMs);
#endif // HAS_CB_UART_SPEED_TEST_COMMAND_IDENTIFIER
}

void emApiNcpUdpStormReturn(EmberStatus status)
{
#ifdef HAS_CB_NCP_UDP_STORM_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_NCP_UDP_STORM_COMMAND_IDENTIFIER,
                                "u",
                                status);
#endif // HAS_CB_NCP_UDP_STORM_COMMAND_IDENTIFIER
}

void emApiNcpUdpStormCompleteHandler(void)
{
#ifdef HAS_CB_NCP_UDP_STORM_COMPLETE_COMMAND_IDENTIFIER
  emSendBinaryManagementCommand(CB_NCP_UDP_STORM_COMPLETE_COMMAND_IDENTIFIER,
                                "");
#endif // HAS_CB_NCP_UDP_STORM_COMPLETE_COMMAND_IDENTIFIER
}
#endif /* (defined (EMBER_TEST) || defined(QA_THREAD_TEST)) */

//------------------------------------------------------------------------------
// APP_USES_SOFTWARE_FLOW_CONTROL
//------------------------------------------------------------------------------
#ifdef EMBER_APPLICATION_USES_SOFTWARE_FLOW_CONTROL

#ifdef HAS_EMBER_START_XON_XOFF_TEST_COMMAND_IDENTIFIER
static void modemStartXOnXOffTestCommand(void)
{
  emApiStartXOnXOffTest();
}
#endif // HAS_EMBER_START_XON_XOFF_TEST_COMMAND_IDENTIFIER
#endif /* EMBER_APPLICATION_USES_SOFTWARE_FLOW_CONTROL */

//------------------------------------------------------------------------------
// Command Handler Table

const EmberCommandEntry emApiCommandTable[] = {
#ifdef HAS_EMBER_RESET_MICRO_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_RESET_MICRO_COMMAND_IDENTIFIER,
                                modemResetMicroCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_RESET_NETWORK_STATE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_RESET_NETWORK_STATE_COMMAND_IDENTIFIER,
                                modemResetNetworkStateCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_INIT_HOST_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_INIT_HOST_COMMAND_IDENTIFIER,
                                modemInitHostCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_STATE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_STATE_COMMAND_IDENTIFIER,
                                modemStateCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_VERSIONS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_VERSIONS_COMMAND_IDENTIFIER,
                                modemGetVersionsCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_FORM_NETWORK_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_FORM_NETWORK_COMMAND_IDENTIFIER,
                                modemFormNetworkCommand,
                                "bbusvwb",
                                NULL),
#endif
#ifdef HAS_EMBER_JOIN_NETWORK_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_JOIN_NETWORK_COMMAND_IDENTIFIER,
                                modemJoinNetworkCommand,
                                "bbvusbuvw",
                                NULL),
#endif
#ifdef HAS_EMBER_RESUME_NETWORK_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_RESUME_NETWORK_COMMAND_IDENTIFIER,
                                modemResumeNetworkCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER,
                                modemAttachToNetworkCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_HOST_JOIN_CLIENT_COMPLETE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_HOST_JOIN_CLIENT_COMPLETE_COMMAND_IDENTIFIER,
                                modemHostJoinClientCompleteCommand,
                                "wbb",
                                NULL),
#endif
#ifdef HAS_EMBER_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER,
                                modemSetSecurityParametersCommand,
                                "bbv",
                                NULL),
#endif
#ifdef HAS_EMBER_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER,
                                modemSwitchToNextNetworkKeyCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_START_SCAN_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_START_SCAN_COMMAND_IDENTIFIER,
                                modemStartScanCommand,
                                "uwu",
                                NULL),
#endif
#ifdef HAS_EMBER_STOP_SCAN_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_STOP_SCAN_COMMAND_IDENTIFIER,
                                modemStopScanCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_RIP_ENTRY_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_RIP_ENTRY_COMMAND_IDENTIFIER,
                                modemGetRipEntryCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_MULTICAST_TABLE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_MULTICAST_TABLE_COMMAND_IDENTIFIER,
                                modemGetMulticastTableCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_COUNTER_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_COUNTER_COMMAND_IDENTIFIER,
                                modemGetCounterCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_CLEAR_COUNTERS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_CLEAR_COUNTERS_COMMAND_IDENTIFIER,
                                modemClearCountersCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_SET_TX_POWER_MODE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SET_TX_POWER_MODE_COMMAND_IDENTIFIER,
                                modemSetTxPowerModeCommand,
                                "v",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_TX_POWER_MODE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_TX_POWER_MODE_COMMAND_IDENTIFIER,
                                modemGetTxPowerModeCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER,
                                modemGetCcaThresholdCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER,
                                modemSetCcaThresholdCommand,
                                "s",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_RADIO_POWER_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_RADIO_POWER_COMMAND_IDENTIFIER,
                                modemGetRadioPowerCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_SET_RADIO_POWER_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SET_RADIO_POWER_COMMAND_IDENTIFIER,
                                modemSetRadioPowerCommand,
                                "s",
                                NULL),
#endif
#ifdef HAS_EMBER_ECHO_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_ECHO_COMMAND_IDENTIFIER,
                                modemEchoCommand,
                                "b",
                                NULL),
#endif
#ifdef HAS_EMBER_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER,
                                modemConfigureGatewayCommand,
                                "vubuuww",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_INDEXED_TOKEN_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_INDEXED_TOKEN_COMMAND_IDENTIFIER,
                                modemGetIndexedTokenCommand,
                                "uu",
                                NULL),
#endif
#ifdef HAS_EMBER_POLL_FOR_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_POLL_FOR_DATA_COMMAND_IDENTIFIER,
                                modemPollForDataCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_DEEP_SLEEP_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_DEEP_SLEEP_COMMAND_IDENTIFIER,
                                modemDeepSleepCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER,
                                modemStackPollForDataCommand,
                                "w",
                                NULL),
#endif
#ifdef HAS_EMBER_OK_TO_NAP_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_OK_TO_NAP_COMMAND_IDENTIFIER,
                                modemOkToNapCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_PING_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_PING_COMMAND_IDENTIFIER,
                                modemPingCommand,
                                "bvvvu",
                                NULL),
#endif
#ifdef HAS_EMBER_JOIN_COMMISSIONED_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_JOIN_COMMISSIONED_COMMAND_IDENTIFIER,
                                modemJoinCommissionedCommand,
                                "suu",
                                NULL),
#endif
#ifdef HAS_EMBER_COMMISSION_NETWORK_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_COMMISSION_NETWORK_COMMAND_IDENTIFIER,
                                modemCommissionNetworkCommand,
                                "uwbvbbbw",
                                NULL),
#endif
#ifdef HAS_EMBER_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER,
                                modemRequestDhcpAddressCommand,
                                "bu",
                                NULL),
#endif
#ifdef HAS_EMBER_HOST_TO_NCP_NO_OP_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_HOST_TO_NCP_NO_OP_COMMAND_IDENTIFIER,
                                modemHostToNcpNoOpCommand,
                                "b",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_GLOBAL_ADDRESSES_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_GLOBAL_ADDRESSES_COMMAND_IDENTIFIER,
                                modemGetGlobalAddressesCommand,
                                "bu",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_DHCP_CLIENTS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_DHCP_CLIENTS_COMMAND_IDENTIFIER,
                                modemGetDhcpClientsCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_ADD_STEERING_EUI64_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_ADD_STEERING_EUI64_COMMAND_IDENTIFIER,
                                modemAddSteeringEui64Command,
                                "b",
                                NULL),
#endif
#ifdef HAS_EMBER_BECOME_COMMISSIONER_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_BECOME_COMMISSIONER_COMMAND_IDENTIFIER,
                                modemBecomeCommissionerCommand,
                                "b",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_COMMISSIONER_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_COMMISSIONER_COMMAND_IDENTIFIER,
                                modemGetCommissionerCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_SEND_STEERING_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SEND_STEERING_DATA_COMMAND_IDENTIFIER,
                                modemSendSteeringDataCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_STOP_COMMISSIONING_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_STOP_COMMISSIONING_COMMAND_IDENTIFIER,
                                modemStopCommissioningCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_SET_JOIN_KEY_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SET_JOIN_KEY_COMMAND_IDENTIFIER,
                                modemSetJoinKeyCommand,
                                "bb",
                                NULL),
#endif
#ifdef HAS_EMBER_SET_JOINING_MODE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SET_JOINING_MODE_COMMAND_IDENTIFIER,
                                modemSetJoiningModeCommand,
                                "uu",
                                NULL),
#endif
#ifdef HAS_EMBER_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER,
                                modemChangeNodeTypeCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_GLOBAL_PREFIXES_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_GLOBAL_PREFIXES_COMMAND_IDENTIFIER,
                                modemGetGlobalPrefixesCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER,
                                modemResignGlobalAddressCommand,
                                "b",
                                NULL),
#endif
#ifdef HAS_EMBER_SET_EUI64_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SET_EUI64_COMMAND_IDENTIFIER,
                                modemSetEui64Command,
                                "b",
                                NULL),
#endif
#ifdef HAS_EMBER_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER,
                                modemRequestSlaacAddressCommand,
                                "bu",
                                NULL),
#endif
#ifdef HAS_EMBER_CUSTOM_HOST_TO_NCP_MESSAGE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_CUSTOM_HOST_TO_NCP_MESSAGE_COMMAND_IDENTIFIER,
                                modemCustomHostToNcpMessageCommand,
                                "b",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER,
                                modemGetNetworkDataTlvCommand,
                                "uu",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER,
                                modemGetRoutingLocatorCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER,
                                modemSetRandomizeMacExtendedIdCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER,
                                modemConfigureExternalRouteCommand,
                                "uubuu",
                                NULL),
#endif
#ifdef HAS_EMBER_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER,
                                modemAllowNativeCommissionerCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER,
                                modemSetCommissionerKeyCommand,
                                "b",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER,
                                modemGetStandaloneBootloaderInfoCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER,
                                modemLaunchStandaloneBootloaderCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_MFG_TOKEN_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_MFG_TOKEN_COMMAND_IDENTIFIER,
                                modemGetMfgTokenCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_SET_MFG_TOKEN_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SET_MFG_TOKEN_COMMAND_IDENTIFIER,
                                modemSetMfgTokenCommand,
                                "ub",
                                NULL),
#endif
#ifdef HAS_EMBER_ENABLE_HOST_DTLS_CLIENT_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_ENABLE_HOST_DTLS_CLIENT_COMMAND_IDENTIFIER,
                                modemEnableHostDtlsClientCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_CTUNE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_CTUNE_COMMAND_IDENTIFIER,
                                modemGetCtuneCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_SET_CTUNE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SET_CTUNE_COMMAND_IDENTIFIER,
                                modemSetCtuneCommand,
                                "v",
                                NULL),
#endif
#ifdef HAS_EMBER_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER,
                                modemSetRadioHoldOffCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_PTA_ENABLE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_PTA_ENABLE_COMMAND_IDENTIFIER,
                                modemGetPtaEnableCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_SET_PTA_ENABLE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SET_PTA_ENABLE_COMMAND_IDENTIFIER,
                                modemSetPtaEnableCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_ANTENNA_MODE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_ANTENNA_MODE_COMMAND_IDENTIFIER,
                                modemGetAntennaModeCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_SET_ANTENNA_MODE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SET_ANTENNA_MODE_COMMAND_IDENTIFIER,
                                modemSetAntennaModeCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER,
                                modemRadioGetRandomNumbersCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_PTA_OPTIONS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_PTA_OPTIONS_COMMAND_IDENTIFIER,
                                modemGetPtaOptionsCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_SET_PTA_OPTIONS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SET_PTA_OPTIONS_COMMAND_IDENTIFIER,
                                modemSetPtaOptionsCommand,
                                "w",
                                NULL),
#endif
#ifdef HAS_EMBER_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER,
                                modemNcpGetNetworkDataCommand,
                                "v",
                                NULL),
#endif
#ifdef HAS_EMBER_NOTE_EXTERNAL_COMMISSIONER_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_NOTE_EXTERNAL_COMMISSIONER_COMMAND_IDENTIFIER,
                                modemNoteExternalCommissionerCommand,
                                "uu",
                                NULL),
#endif
#ifdef HAS_EMBER_NCP_SET_ND_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_NCP_SET_ND_DATA_COMMAND_IDENTIFIER,
                                modemNcpSetNdDataCommand,
                                "vbv",
                                NULL),
#endif
#ifdef HAS_EMBER_NCP_SET_LARGE_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_NCP_SET_LARGE_DATA_COMMAND_IDENTIFIER,
                                modemNcpSetLargeDataCommand,
                                "uvbv",
                                NULL),
#endif
#ifdef MFGLIB
#ifdef HAS_EMBER_MFGLIB_START_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_MFGLIB_START_COMMAND_IDENTIFIER,
                                modemMfglibStartCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_MFGLIB_END_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_MFGLIB_END_COMMAND_IDENTIFIER,
                                modemMfglibEndCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_MFGLIB_START_ACTIVITY_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_MFGLIB_START_ACTIVITY_COMMAND_IDENTIFIER,
                                modemMfglibStartActivityCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_MFGLIB_STOP_ACTIVITY_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_MFGLIB_STOP_ACTIVITY_COMMAND_IDENTIFIER,
                                modemMfglibStopActivityCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_MFGLIB_SEND_PACKET_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_MFGLIB_SEND_PACKET_COMMAND_IDENTIFIER,
                                modemMfglibSendPacketCommand,
                                "bv",
                                NULL),
#endif
#ifdef HAS_EMBER_MFGLIB_SET_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_MFGLIB_SET_COMMAND_IDENTIFIER,
                                modemMfglibSetCommand,
                                "uvs",
                                NULL),
#endif
#ifdef HAS_EMBER_MFGLIB_GET_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_MFGLIB_GET_COMMAND_IDENTIFIER,
                                modemMfglibGetCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_MFGLIB_TEST_CONT_MOD_CAL_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_MFGLIB_TEST_CONT_MOD_CAL_COMMAND_IDENTIFIER,
                                modemMfglibTestContModCalCommand,
                                "uw",
                                NULL),
#endif
#endif /* MFGLIB */
#ifdef EMBER_TEST
#ifdef HAS_EMBER_CONFIG_UART_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_CONFIG_UART_COMMAND_IDENTIFIER,
                                modemConfigUartCommand,
                                "uu",
                                NULL),
#endif
#ifdef HAS_EMBER_RESET_NCP_ASH_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_RESET_NCP_ASH_COMMAND_IDENTIFIER,
                                modemResetNcpAshCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_START_UART_STORM_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_START_UART_STORM_COMMAND_IDENTIFIER,
                                modemStartUartStormCommand,
                                "v",
                                NULL),
#endif
#ifdef HAS_EMBER_STOP_UART_STORM_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_STOP_UART_STORM_COMMAND_IDENTIFIER,
                                modemStopUartStormCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_SEND_DONE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_SEND_DONE_COMMAND_IDENTIFIER,
                                modemSendDoneCommand,
                                "w",
                                NULL),
#endif
#endif /* EMBER_TEST */
#if (defined (EMBER_TEST) || defined(QA_THREAD_TEST))
#ifdef HAS_EMBER_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER,
                                modemGetNetworkKeyInfoCommand,
                                "u",
                                NULL),
#endif
#ifdef HAS_EMBER_FORCE_ASSERT_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_FORCE_ASSERT_COMMAND_IDENTIFIER,
                                modemForceAssertCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_GET_NODE_STATUS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_GET_NODE_STATUS_COMMAND_IDENTIFIER,
                                modemGetNodeStatusCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER,
                                modemAddAddressDataCommand,
                                "bv",
                                NULL),
#endif
#ifdef HAS_EMBER_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER,
                                modemClearAddressCacheCommand,
                                "",
                                NULL),
#endif
#ifdef HAS_EMBER_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER,
                                modemLookupAddressDataCommand,
                                "b",
                                NULL),
#endif
#ifdef HAS_EMBER_START_UART_SPEED_TEST_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_START_UART_SPEED_TEST_COMMAND_IDENTIFIER,
                                modemStartUartSpeedTestCommand,
                                "uww",
                                NULL),
#endif
#ifdef HAS_EMBER_NCP_UDP_STORM_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_NCP_UDP_STORM_COMMAND_IDENTIFIER,
                                modemNcpUdpStormCommand,
                                "ubvv",
                                NULL),
#endif
#endif /* (defined (EMBER_TEST) || defined(QA_THREAD_TEST)) */
#ifdef EMBER_APPLICATION_USES_SOFTWARE_FLOW_CONTROL
#ifdef HAS_EMBER_START_XON_XOFF_TEST_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(EMBER_START_XON_XOFF_TEST_COMMAND_IDENTIFIER,
                                modemStartXOnXOffTestCommand,
                                "",
                                NULL),
#endif
#endif /* EMBER_APPLICATION_USES_SOFTWARE_FLOW_CONTROL */
  { { NULL } }  // terminator
};
