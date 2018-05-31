// File: tmsp-host.c
//
// *** Generated file. Do not edit! ***
//
// run tmsp-update.sh to regenerate
//
// Description: Host functions for sending Thread management commands
// to the NCP.
//
// TMSP Version: 14.0
//
// Copyright 2017 Silicon Laboratories, Inc.                                *80*

#include PLATFORM_HEADER
#ifdef CONFIGURATION_HEADER
  #include CONFIGURATION_HEADER
#endif
#include "include/ember.h"
#include "include/thread-debug.h"
#include "app/ip-ncp/binary-management.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/tmsp/tmsp-enum.h"
#include "app/tmsp/tmsp-frame-utilities.h"
#include "app/tmsp/tmsp-host-utilities.h"
#include "app/tmsp/tmsp-configuration.h"
#include "app/tmsp/tmsp-mfglib.h"

//------------------------------------------------------------------------------
// Core
//------------------------------------------------------------------------------

#ifdef HAS_EMBER_RESET_MICRO_COMMAND_IDENTIFIER
void emberResetMicro(void)
{
  emSendBinaryManagementCommand(EMBER_RESET_MICRO_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_RESET_MICRO_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_RESET_NETWORK_STATE_COMMAND_IDENTIFIER
void emberResetNetworkState(void)
{
  bool preHookResult = tmspHostResetNetworkStatePreHook();
  if (preHookResult) {
    emSendBinaryManagementCommand(EMBER_RESET_NETWORK_STATE_COMMAND_IDENTIFIER,
                                  "");
  }
}
#endif // HAS_EMBER_RESET_NETWORK_STATE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_INIT_HOST_COMMAND_IDENTIFIER
void emberInitHost(void)
{
  emSendBinaryManagementCommand(EMBER_INIT_HOST_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_INIT_HOST_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_STATE_COMMAND_IDENTIFIER
void emberState(void)
{
  emSendBinaryManagementCommand(EMBER_STATE_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_STATE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_VERSIONS_COMMAND_IDENTIFIER
void emberGetVersions(void)
{
  emSendBinaryManagementCommand(EMBER_GET_VERSIONS_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_GET_VERSIONS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_FORM_NETWORK_COMMAND_IDENTIFIER
void emberFormNetwork(const EmberNetworkParameters *parameters,
                      uint16_t options,
                      uint32_t channelMask)
{
  bool preHookResult = tmspHostFormNetworkPreHook(&options);
  if (preHookResult) {
    emSendBinaryManagementCommand(EMBER_FORM_NETWORK_COMMAND_IDENTIFIER,
                                  "bbusvwb",
                                  parameters->networkId,
                                  sizeof(parameters->networkId),
                                  parameters->ulaPrefix.bytes,
                                  sizeof(parameters->ulaPrefix.bytes),
                                  parameters->nodeType,
                                  parameters->radioTxPower,
                                  options,
                                  channelMask,
                                  parameters->legacyUla.bytes,
                                  sizeof(parameters->legacyUla.bytes));
  }
}
#endif // HAS_EMBER_FORM_NETWORK_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_JOIN_NETWORK_COMMAND_IDENTIFIER
void emberJoinNetwork(const EmberNetworkParameters *parameters,
                      uint16_t options,
                      uint32_t channelMask)
{
  emSendBinaryManagementCommand(EMBER_JOIN_NETWORK_COMMAND_IDENTIFIER,
                                "bbvusbuvw",
                                parameters->networkId,
                                sizeof(parameters->networkId),
                                parameters->extendedPanId,
                                sizeof(parameters->extendedPanId),
                                parameters->panId,
                                parameters->nodeType,
                                parameters->radioTxPower,
                                parameters->joinKey,
                                sizeof(parameters->joinKey),
                                parameters->joinKeyLength,
                                options,
                                channelMask);
}
#endif // HAS_EMBER_JOIN_NETWORK_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_RESUME_NETWORK_COMMAND_IDENTIFIER
void emberResumeNetwork(void)
{
  emSendBinaryManagementCommand(EMBER_RESUME_NETWORK_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_RESUME_NETWORK_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER
void emberAttachToNetwork(void)
{
  emSendBinaryManagementCommand(EMBER_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_HOST_JOIN_CLIENT_COMPLETE_COMMAND_IDENTIFIER
void emberHostJoinClientComplete(uint32_t keySequence,
                                 const uint8_t *key,
                                 const uint8_t *ulaPrefix)
{
  emSendBinaryManagementCommand(EMBER_HOST_JOIN_CLIENT_COMPLETE_COMMAND_IDENTIFIER,
                                "wbb",
                                keySequence,
                                key, EMBER_ENCRYPTION_KEY_SIZE,
                                ulaPrefix, 8);
}
#endif // HAS_EMBER_HOST_JOIN_CLIENT_COMPLETE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER
void emberSetSecurityParameters(const EmberSecurityParameters *parameters,
                                uint16_t options)
{
  emSendBinaryManagementCommand(EMBER_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER,
                                "bbv",
                                parameters->networkKey,
                                EMBER_ENCRYPTION_KEY_SIZE,
                                parameters->presharedKey,
                                parameters->presharedKeyLength,
                                options);
}
#endif // HAS_EMBER_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER
void emberSwitchToNextNetworkKey(void)
{
  emSendBinaryManagementCommand(EMBER_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_START_SCAN_COMMAND_IDENTIFIER
void emberStartScan(EmberNetworkScanType scanType,
                    uint32_t channelMask,
                    uint8_t duration)
{
  emSendBinaryManagementCommand(EMBER_START_SCAN_COMMAND_IDENTIFIER,
                                "uwu",
                                scanType,
                                channelMask,
                                duration);
}
#endif // HAS_EMBER_START_SCAN_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_STOP_SCAN_COMMAND_IDENTIFIER
void emberStopScan(void)
{
  emSendBinaryManagementCommand(EMBER_STOP_SCAN_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_STOP_SCAN_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_RIP_ENTRY_COMMAND_IDENTIFIER
void emberGetRipEntry(uint8_t index)
{
  emSendBinaryManagementCommand(EMBER_GET_RIP_ENTRY_COMMAND_IDENTIFIER,
                                "u",
                                index);
}
#endif // HAS_EMBER_GET_RIP_ENTRY_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_MULTICAST_TABLE_COMMAND_IDENTIFIER
void emberGetMulticastTable(void)
{
  emSendBinaryManagementCommand(EMBER_GET_MULTICAST_TABLE_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_GET_MULTICAST_TABLE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_COUNTER_COMMAND_IDENTIFIER
void emberGetCounter(EmberCounterType type)
{
  emSendBinaryManagementCommand(EMBER_GET_COUNTER_COMMAND_IDENTIFIER,
                                "u",
                                type);
}
#endif // HAS_EMBER_GET_COUNTER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_CLEAR_COUNTERS_COMMAND_IDENTIFIER
void emberClearCounters(void)
{
  emSendBinaryManagementCommand(EMBER_CLEAR_COUNTERS_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_CLEAR_COUNTERS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_TX_POWER_MODE_COMMAND_IDENTIFIER
EmberStatus emberSetTxPowerMode(uint16_t txPowerMode)
{
  emSendBinaryManagementCommand(EMBER_SET_TX_POWER_MODE_COMMAND_IDENTIFIER,
                                "v",
                                txPowerMode);
  return EMBER_SUCCESS;
}
#endif // HAS_EMBER_SET_TX_POWER_MODE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_TX_POWER_MODE_COMMAND_IDENTIFIER
void emberGetTxPowerMode(void)
{
  emSendBinaryManagementCommand(EMBER_GET_TX_POWER_MODE_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_GET_TX_POWER_MODE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_FF_WAKEUP_COMMAND_IDENTIFIER
void emberFfWakeup(void)
{
  emSendBinaryManagementCommand(EMBER_FF_WAKEUP_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_FF_WAKEUP_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER
void emberGetCcaThreshold(void)
{
  emSendBinaryManagementCommand(EMBER_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER
void emberSetCcaThreshold(int8_t threshold)
{
  emSendBinaryManagementCommand(EMBER_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER,
                                "s",
                                threshold);
}
#endif // HAS_EMBER_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_RADIO_POWER_COMMAND_IDENTIFIER
void emberGetRadioPower(void)
{
  emSendBinaryManagementCommand(EMBER_GET_RADIO_POWER_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_GET_RADIO_POWER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_RADIO_POWER_COMMAND_IDENTIFIER
void emberSetRadioPower(int8_t power)
{
  emSendBinaryManagementCommand(EMBER_SET_RADIO_POWER_COMMAND_IDENTIFIER,
                                "s",
                                power);
}
#endif // HAS_EMBER_SET_RADIO_POWER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_ECHO_COMMAND_IDENTIFIER
void emberEcho(const uint8_t *data,
               uint8_t length)
{
  emSendBinaryManagementCommand(EMBER_ECHO_COMMAND_IDENTIFIER,
                                "b",
                                data, length);
}
#endif // HAS_EMBER_ECHO_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER
void emberConfigureGateway(uint16_t borderRouterFlags,
                           bool isStable,
                           const uint8_t *prefix,
                           uint8_t prefixLengthInBits,
                           uint8_t domainId,
                           uint32_t preferredLifetime,
                           uint32_t validLifetime)
{
  emSendBinaryManagementCommand(EMBER_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER,
                                "vubuuww",
                                borderRouterFlags,
                                isStable,
                                prefix, 16,
                                prefixLengthInBits,
                                domainId,
                                preferredLifetime,
                                validLifetime);
}
#endif // HAS_EMBER_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_INDEXED_TOKEN_COMMAND_IDENTIFIER
void emberGetIndexedToken(EmberTokenId tokenId,
                          uint8_t index)
{
  emSendBinaryManagementCommand(EMBER_GET_INDEXED_TOKEN_COMMAND_IDENTIFIER,
                                "uu",
                                tokenId,
                                index);
}
#endif // HAS_EMBER_GET_INDEXED_TOKEN_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_POLL_FOR_DATA_COMMAND_IDENTIFIER
void emberPollForData(void)
{
  emSendBinaryManagementCommand(EMBER_POLL_FOR_DATA_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_POLL_FOR_DATA_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_DEEP_SLEEP_COMMAND_IDENTIFIER
void emberDeepSleep(bool sleep)
{
  emSendBinaryManagementCommand(EMBER_DEEP_SLEEP_COMMAND_IDENTIFIER,
                                "u",
                                sleep);
}
#endif // HAS_EMBER_DEEP_SLEEP_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER
void emberStackPollForData(uint32_t pollMs)
{
  emSendBinaryManagementCommand(EMBER_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER,
                                "w",
                                pollMs);
}
#endif // HAS_EMBER_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_OK_TO_NAP_COMMAND_IDENTIFIER
bool emberOkToNap(void)
{
  emSendBinaryManagementCommand(EMBER_OK_TO_NAP_COMMAND_IDENTIFIER,
                                "");
  return false;
}
#endif // HAS_EMBER_OK_TO_NAP_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_PING_COMMAND_IDENTIFIER
bool emberPing(const uint8_t *destination,
               uint16_t id,
               uint16_t sequence,
               uint16_t length,
               uint8_t hopLimit)
{
  emSendBinaryManagementCommand(EMBER_PING_COMMAND_IDENTIFIER,
                                "bvvvu",
                                destination, 16,
                                id,
                                sequence,
                                length,
                                hopLimit);
  return true;
}
#endif // HAS_EMBER_PING_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_JOIN_COMMISSIONED_COMMAND_IDENTIFIER
void emberJoinCommissioned(int8_t radioTxPower,
                           EmberNodeType nodeType,
                           bool requireConnectivity)
{
  emSendBinaryManagementCommand(EMBER_JOIN_COMMISSIONED_COMMAND_IDENTIFIER,
                                "suu",
                                radioTxPower,
                                nodeType,
                                requireConnectivity);
}
#endif // HAS_EMBER_JOIN_COMMISSIONED_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_COMMISSION_NETWORK_COMMAND_IDENTIFIER
void emberCommissionNetwork(uint8_t preferredChannel,
                            uint32_t fallbackChannelMask,
                            const uint8_t *networkId,
                            uint8_t networkIdLength,
                            uint16_t panId,
                            const uint8_t *ulaPrefix,
                            const uint8_t *extendedPanId,
                            const EmberKeyData *key,
                            uint32_t keySequence)
{
  emSendBinaryManagementCommand(EMBER_COMMISSION_NETWORK_COMMAND_IDENTIFIER,
                                "uwbvbbbw",
                                preferredChannel,
                                fallbackChannelMask,
                                networkId, networkIdLength,
                                panId,
                                ulaPrefix, 8,
                                extendedPanId, EXTENDED_PAN_ID_SIZE,
                                key, sizeof(EmberKeyData),
                                keySequence);
}
#endif // HAS_EMBER_COMMISSION_NETWORK_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER
void emberRequestDhcpAddress(const uint8_t *prefix,
                             uint8_t prefixLengthInBits)
{
  emSendBinaryManagementCommand(EMBER_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER,
                                "bu",
                                prefix, 16,
                                prefixLengthInBits);
}
#endif // HAS_EMBER_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_HOST_TO_NCP_NO_OP_COMMAND_IDENTIFIER
void emberHostToNcpNoOp(const uint8_t *bytes,
                        uint8_t bytesLength)
{
  emSendBinaryManagementCommand(EMBER_HOST_TO_NCP_NO_OP_COMMAND_IDENTIFIER,
                                "b",
                                bytes, bytesLength);
}
#endif // HAS_EMBER_HOST_TO_NCP_NO_OP_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_GLOBAL_ADDRESSES_COMMAND_IDENTIFIER
void emberGetGlobalAddresses(const uint8_t *prefix,
                             uint8_t prefixLengthInBits)
{
  emSendBinaryManagementCommand(EMBER_GET_GLOBAL_ADDRESSES_COMMAND_IDENTIFIER,
                                "bu",
                                prefix, 16,
                                prefixLengthInBits);
}
#endif // HAS_EMBER_GET_GLOBAL_ADDRESSES_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_DHCP_CLIENTS_COMMAND_IDENTIFIER
void emberGetDhcpClients(void)
{
  emSendBinaryManagementCommand(EMBER_GET_DHCP_CLIENTS_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_GET_DHCP_CLIENTS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_ADD_STEERING_EUI64_COMMAND_IDENTIFIER
void emberAddSteeringEui64(const EmberEui64 *eui64)
{
  emSendBinaryManagementCommand(EMBER_ADD_STEERING_EUI64_COMMAND_IDENTIFIER,
                                "b",
                                eui64->bytes,
                                EUI64_SIZE);
}
#endif // HAS_EMBER_ADD_STEERING_EUI64_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_BECOME_COMMISSIONER_COMMAND_IDENTIFIER
void emberBecomeCommissioner(const uint8_t *deviceName,
                             uint8_t deviceNameLength)
{
  emSendBinaryManagementCommand(EMBER_BECOME_COMMISSIONER_COMMAND_IDENTIFIER,
                                "b",
                                deviceName, deviceNameLength);
}
#endif // HAS_EMBER_BECOME_COMMISSIONER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_COMMISSIONER_COMMAND_IDENTIFIER
void emberGetCommissioner(void)
{
  emSendBinaryManagementCommand(EMBER_GET_COMMISSIONER_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_GET_COMMISSIONER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SEND_STEERING_DATA_COMMAND_IDENTIFIER
void emberSendSteeringData(void)
{
  emSendBinaryManagementCommand(EMBER_SEND_STEERING_DATA_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_SEND_STEERING_DATA_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_STOP_COMMISSIONING_COMMAND_IDENTIFIER
void emberStopCommissioning(void)
{
  emSendBinaryManagementCommand(EMBER_STOP_COMMISSIONING_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_STOP_COMMISSIONING_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_JOIN_KEY_COMMAND_IDENTIFIER
void emberSetJoinKey(const EmberEui64 *eui64,
                     const uint8_t *key,
                     uint8_t keyLength)
{
  emSendBinaryManagementCommand(EMBER_SET_JOIN_KEY_COMMAND_IDENTIFIER,
                                "bb",
                                eui64->bytes,
                                EUI64_SIZE,
                                key,
                                keyLength);
}
#endif // HAS_EMBER_SET_JOIN_KEY_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_JOINING_MODE_COMMAND_IDENTIFIER
void emberSetJoiningMode(EmberJoiningMode mode,
                         uint8_t length)
{
  emSendBinaryManagementCommand(EMBER_SET_JOINING_MODE_COMMAND_IDENTIFIER,
                                "uu",
                                mode,
                                length);
}
#endif // HAS_EMBER_SET_JOINING_MODE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER
void emberChangeNodeType(EmberNodeType newType)
{
  emSendBinaryManagementCommand(EMBER_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER,
                                "u",
                                newType);
}
#endif // HAS_EMBER_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_GLOBAL_PREFIXES_COMMAND_IDENTIFIER
void emberGetGlobalPrefixes(void)
{
  emSendBinaryManagementCommand(EMBER_GET_GLOBAL_PREFIXES_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_GET_GLOBAL_PREFIXES_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER
void emberResignGlobalAddress(const EmberIpv6Address *address)
{
  bool preHookResult = tmspHostResignGlobalAddressPreHook(address);
  if (preHookResult) {
    emSendBinaryManagementCommand(EMBER_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER,
                                  "b",
                                  address, sizeof(EmberIpv6Address));
  }
}
#endif // HAS_EMBER_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_EUI64_COMMAND_IDENTIFIER
void emberSetEui64(const EmberEui64 *eui64)
{
  emSendBinaryManagementCommand(EMBER_SET_EUI64_COMMAND_IDENTIFIER,
                                "b",
                                eui64->bytes,
                                EUI64_SIZE);
  tmspHostSetEui64PostHook(eui64);
}
#endif // HAS_EMBER_SET_EUI64_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER
void emberRequestSlaacAddress(const uint8_t *prefix,
                              uint8_t prefixLengthInBits)
{
  emSendBinaryManagementCommand(EMBER_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER,
                                "bu",
                                prefix, 16,
                                prefixLengthInBits);
}
#endif // HAS_EMBER_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_CUSTOM_HOST_TO_NCP_MESSAGE_COMMAND_IDENTIFIER
void emberCustomHostToNcpMessage(const uint8_t *message,
                                 uint8_t messageLength)
{
  emSendBinaryManagementCommand(EMBER_CUSTOM_HOST_TO_NCP_MESSAGE_COMMAND_IDENTIFIER,
                                "b",
                                message, messageLength);
}
#endif // HAS_EMBER_CUSTOM_HOST_TO_NCP_MESSAGE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER
void emberGetNetworkDataTlv(uint8_t type,
                            uint8_t index)
{
  emSendBinaryManagementCommand(EMBER_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER,
                                "uu",
                                type,
                                index);
}
#endif // HAS_EMBER_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER
void emberGetRoutingLocator(void)
{
  emSendBinaryManagementCommand(EMBER_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER
void emberSetRandomizeMacExtendedId(bool value)
{
  emSendBinaryManagementCommand(EMBER_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER,
                                "u",
                                value);
}
#endif // HAS_EMBER_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER
void emberConfigureExternalRoute(uint8_t externalRouteFlags,
                                 bool isStable,
                                 const uint8_t *prefix,
                                 uint8_t prefixLengthInBits,
                                 uint8_t externalRouteDomainId)
{
  emSendBinaryManagementCommand(EMBER_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER,
                                "uubuu",
                                externalRouteFlags,
                                isStable,
                                prefix, 16,
                                prefixLengthInBits,
                                externalRouteDomainId);
}
#endif // HAS_EMBER_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER
void emberAllowNativeCommissioner(bool on)
{
  emSendBinaryManagementCommand(EMBER_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER,
                                "u",
                                on);
}
#endif // HAS_EMBER_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER
void emberSetCommissionerKey(const uint8_t *commissionerKey,
                             uint8_t commissionerKeyLength)
{
  emSendBinaryManagementCommand(EMBER_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER,
                                "b",
                                commissionerKey, commissionerKeyLength);
}
#endif // HAS_EMBER_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER
void emberGetStandaloneBootloaderInfo(void)
{
  emSendBinaryManagementCommand(EMBER_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER
void emberLaunchStandaloneBootloader(uint8_t mode)
{
  emSendBinaryManagementCommand(EMBER_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER,
                                "u",
                                mode);
}
#endif // HAS_EMBER_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_MFG_TOKEN_COMMAND_IDENTIFIER
void emberGetMfgToken(EmberMfgTokenId tokenId)
{
  emSendBinaryManagementCommand(EMBER_GET_MFG_TOKEN_COMMAND_IDENTIFIER,
                                "u",
                                tokenId);
}
#endif // HAS_EMBER_GET_MFG_TOKEN_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_MFG_TOKEN_COMMAND_IDENTIFIER
void emberSetMfgToken(EmberMfgTokenId tokenId,
                      const uint8_t *tokenData,
                      uint8_t tokenDataLength)
{
  emSendBinaryManagementCommand(EMBER_SET_MFG_TOKEN_COMMAND_IDENTIFIER,
                                "ub",
                                tokenId,
                                tokenData, tokenDataLength);
}
#endif // HAS_EMBER_SET_MFG_TOKEN_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_ENABLE_HOST_DTLS_CLIENT_COMMAND_IDENTIFIER
void emberEnableHostDtlsClient(bool enable)
{
  emSendBinaryManagementCommand(EMBER_ENABLE_HOST_DTLS_CLIENT_COMMAND_IDENTIFIER,
                                "u",
                                enable);
}
#endif // HAS_EMBER_ENABLE_HOST_DTLS_CLIENT_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_CTUNE_COMMAND_IDENTIFIER
void emberGetCtune(void)
{
  emSendBinaryManagementCommand(EMBER_GET_CTUNE_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_GET_CTUNE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_CTUNE_COMMAND_IDENTIFIER
void emberSetCtune(uint16_t tune)
{
  emSendBinaryManagementCommand(EMBER_SET_CTUNE_COMMAND_IDENTIFIER,
                                "v",
                                tune);
}
#endif // HAS_EMBER_SET_CTUNE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER
void emberSetRadioHoldOff(bool enabled)
{
  emSendBinaryManagementCommand(EMBER_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER,
                                "u",
                                enabled);
}
#endif // HAS_EMBER_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_PTA_ENABLE_COMMAND_IDENTIFIER
void emberGetPtaEnable(void)
{
  emSendBinaryManagementCommand(EMBER_GET_PTA_ENABLE_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_GET_PTA_ENABLE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_PTA_ENABLE_COMMAND_IDENTIFIER
void emberSetPtaEnable(bool enabled)
{
  emSendBinaryManagementCommand(EMBER_SET_PTA_ENABLE_COMMAND_IDENTIFIER,
                                "u",
                                enabled);
}
#endif // HAS_EMBER_SET_PTA_ENABLE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_ANTENNA_MODE_COMMAND_IDENTIFIER
void emberGetAntennaMode(void)
{
  emSendBinaryManagementCommand(EMBER_GET_ANTENNA_MODE_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_GET_ANTENNA_MODE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_ANTENNA_MODE_COMMAND_IDENTIFIER
void emberSetAntennaMode(uint8_t mode)
{
  emSendBinaryManagementCommand(EMBER_SET_ANTENNA_MODE_COMMAND_IDENTIFIER,
                                "u",
                                mode);
}
#endif // HAS_EMBER_SET_ANTENNA_MODE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER
void emberRadioGetRandomNumbers(uint8_t count)
{
  emSendBinaryManagementCommand(EMBER_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER,
                                "u",
                                count);
}
#endif // HAS_EMBER_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_PTA_OPTIONS_COMMAND_IDENTIFIER
void emberGetPtaOptions(void)
{
  emSendBinaryManagementCommand(EMBER_GET_PTA_OPTIONS_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_GET_PTA_OPTIONS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SET_PTA_OPTIONS_COMMAND_IDENTIFIER
void emberSetPtaOptions(uint32_t options)
{
  emSendBinaryManagementCommand(EMBER_SET_PTA_OPTIONS_COMMAND_IDENTIFIER,
                                "w",
                                options);
}
#endif // HAS_EMBER_SET_PTA_OPTIONS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER
void emberNcpGetNetworkData(uint16_t bufferLength)
{
  emSendBinaryManagementCommand(EMBER_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER,
                                "v",
                                bufferLength);
}
#endif // HAS_EMBER_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_NOTE_EXTERNAL_COMMISSIONER_COMMAND_IDENTIFIER
void emNoteExternalCommissioner(uint8_t commissionerId,
                                bool available)
{
  emSendBinaryManagementCommand(EMBER_NOTE_EXTERNAL_COMMISSIONER_COMMAND_IDENTIFIER,
                                "uu",
                                commissionerId,
                                available);
}
#endif // HAS_EMBER_NOTE_EXTERNAL_COMMISSIONER_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_NCP_SET_ND_DATA_COMMAND_IDENTIFIER
void emberNcpSetNdData(uint16_t totalLength,
                       const uint8_t *ndDataFragment,
                       uint8_t fragmentLength,
                       uint16_t fragmentOffset)
{
  emSendBinaryManagementCommand(EMBER_NCP_SET_ND_DATA_COMMAND_IDENTIFIER,
                                "vbv",
                                totalLength,
                                ndDataFragment, fragmentLength,
                                fragmentOffset);
}
#endif // HAS_EMBER_NCP_SET_ND_DATA_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_NCP_SET_LARGE_DATA_COMMAND_IDENTIFIER
void emberNcpSetLargeData(SetLargeValueId valueId,
                          uint16_t totalLength,
                          const uint8_t *fragment,
                          uint8_t fragmentLength,
                          uint16_t fragmentOffset)
{
  emSendBinaryManagementCommand(EMBER_NCP_SET_LARGE_DATA_COMMAND_IDENTIFIER,
                                "uvbv",
                                valueId,
                                totalLength,
                                fragment, fragmentLength,
                                fragmentOffset);
}
#endif // HAS_EMBER_NCP_SET_LARGE_DATA_COMMAND_IDENTIFIER

#ifdef HAS_CB_RESET_MICRO_COMMAND_IDENTIFIER
static void resetMicroHandlerCallback(void)
{
  EmberResetCause cause;
  cause = (EmberResetCause) emberUnsignedCommandArgument(0);
  bool preHookResult = tmspHostResetMicroHandlerPreHook();
  if (preHookResult) {
    emberResetMicroHandler(cause);
  }
}
#endif // HAS_CB_RESET_MICRO_COMMAND_IDENTIFIER

#ifdef HAS_CB_STATE_COMMAND_IDENTIFIER
static void stateReturnCallback(void)
{
  EmberNetworkParameters parameters;
  EmberEui64 localEui64;
  EmberEui64 macExtendedId;
  EmberNetworkStatus networkStatus;
  MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
  MEMSET(&localEui64, 0, sizeof(EmberEui64));
  MEMSET(&macExtendedId, 0, sizeof(EmberEui64));
  emberGetStringArgument(0, parameters.networkId, sizeof(parameters.networkId), false);
  emberGetStringArgument(1, parameters.ulaPrefix.bytes, sizeof(parameters.ulaPrefix.bytes), false);
  emberGetStringArgument(2, parameters.extendedPanId, sizeof(parameters.extendedPanId), false);
  parameters.panId = emberUnsignedCommandArgument(3);
  parameters.channel = emberUnsignedCommandArgument(4);
  parameters.nodeType = emberUnsignedCommandArgument(5);
  parameters.radioTxPower = emberSignedCommandArgument(6);
  networkStatus = emberUnsignedCommandArgument(7);
  emberGetStringArgument(8, localEui64.bytes, sizeof(localEui64.bytes), false);
  emberGetStringArgument(9, macExtendedId.bytes, sizeof(macExtendedId.bytes), false);
  emberGetStringArgument(10, parameters.legacyUla.bytes, sizeof(parameters.legacyUla.bytes), false);
  bool preHookResult = tmspStateReturnPreHook(&parameters,
                                              &localEui64,
                                              &macExtendedId,
                                              networkStatus);
  if (preHookResult) {
    emberStateReturn(&parameters,
                     &localEui64,
                     &macExtendedId,
                     networkStatus);
  }
}
#endif // HAS_CB_STATE_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_VERSIONS_COMMAND_IDENTIFIER
static void getVersionsReturnCallback(void)
{
  uint8_t *versionName;
  uint16_t managementVersionNumber;
  uint16_t stackVersionNumber;
  uint16_t stackBuildNumber;
  EmberVersionType versionType;
  uint8_t *buildTimestamp;
  versionName = (uint8_t *) emberStringCommandArgument(0, NULL);
  managementVersionNumber = (uint16_t) emberUnsignedCommandArgument(1);
  stackVersionNumber = (uint16_t) emberUnsignedCommandArgument(2);
  stackBuildNumber = (uint16_t) emberUnsignedCommandArgument(3);
  versionType = (EmberVersionType) emberUnsignedCommandArgument(4);
  buildTimestamp = (uint8_t *) emberStringCommandArgument(5, NULL);
  emberGetVersionsReturn(versionName,
                         managementVersionNumber,
                         stackVersionNumber,
                         stackBuildNumber,
                         versionType,
                         buildTimestamp);
}
#endif // HAS_CB_GET_VERSIONS_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_RIP_ENTRY_COMMAND_IDENTIFIER
static void getRipEntryReturnCallback(void)
{
  uint8_t index;
  EmberRipEntry entry;
  MEMSET(&entry, 0, sizeof(EmberRipEntry));
  index = emberUnsignedCommandArgument(0);
  emberGetStringArgument(1, entry.longId, 8, false);
  entry.type = emberUnsignedCommandArgument(2);
  entry.rollingRssi = emberSignedCommandArgument(3);
  entry.nextHopIndex = emberUnsignedCommandArgument(4);
  entry.ripMetric = emberUnsignedCommandArgument(5);
  entry.incomingLinkQuality = emberUnsignedCommandArgument(6);
  entry.outgoingLinkQuality = emberUnsignedCommandArgument(7);
  entry.mleSync = emberUnsignedCommandArgument(8);
  entry.age = emberUnsignedCommandArgument(9);
  emberGetRipEntryReturn(index,
                         &entry);
}
#endif // HAS_CB_GET_RIP_ENTRY_COMMAND_IDENTIFIER

#ifdef HAS_CB_INIT_COMMAND_IDENTIFIER
static void initReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberInitReturn(status);
}
#endif // HAS_CB_INIT_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_COUNTER_COMMAND_IDENTIFIER
static void getCounterReturnCallback(void)
{
  EmberCounterType type;
  uint16_t value;
  type = (EmberCounterType) emberUnsignedCommandArgument(0);
  value = (uint16_t) emberUnsignedCommandArgument(1);
  emberGetCounterReturn(type,
                        value);
}
#endif // HAS_CB_GET_COUNTER_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER
static void setSecurityParametersReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberSetSecurityParametersReturn(status);
}
#endif // HAS_CB_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER

#ifdef HAS_CB_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER
static void switchToNextNetworkKeyReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberSwitchToNextNetworkKeyReturn(status);
}
#endif // HAS_CB_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER

#ifdef HAS_CB_FORM_NETWORK_COMMAND_IDENTIFIER
static void formNetworkReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberFormNetworkReturn(status);
}
#endif // HAS_CB_FORM_NETWORK_COMMAND_IDENTIFIER

#ifdef HAS_CB_JOIN_NETWORK_COMMAND_IDENTIFIER
static void joinNetworkReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberJoinNetworkReturn(status);
}
#endif // HAS_CB_JOIN_NETWORK_COMMAND_IDENTIFIER

#ifdef HAS_CB_RESUME_NETWORK_COMMAND_IDENTIFIER
static void resumeNetworkReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberResumeNetworkReturn(status);
}
#endif // HAS_CB_RESUME_NETWORK_COMMAND_IDENTIFIER

#ifdef HAS_CB_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER
static void attachToNetworkReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberAttachToNetworkReturn(status);
}
#endif // HAS_CB_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER

#ifdef HAS_CB_ENERGY_SCAN_COMMAND_IDENTIFIER
static void energyScanHandlerCallback(void)
{
  uint8_t channel;
  int8_t maxRssiValue;
  channel = (uint8_t) emberUnsignedCommandArgument(0);
  maxRssiValue = (int8_t) emberSignedCommandArgument(1);
  emberEnergyScanHandler(channel,
                         maxRssiValue);
}
#endif // HAS_CB_ENERGY_SCAN_COMMAND_IDENTIFIER

#ifdef HAS_CB_ACTIVE_SCAN_COMMAND_IDENTIFIER
static void activeScanHandlerCallback(void)
{
  EmberMacBeaconData beaconData;
  MEMSET(&beaconData, 0, sizeof(EmberMacBeaconData));
  beaconData.allowingJoin = emberUnsignedCommandArgument(0);
  beaconData.channel = emberUnsignedCommandArgument(1);
  beaconData.lqi = emberUnsignedCommandArgument(2);
  beaconData.rssi = emberUnsignedCommandArgument(3);
  emberGetStringArgument(4, beaconData.networkId, sizeof(beaconData.networkId), false);
  beaconData.panId = emberUnsignedCommandArgument(5);
  emberGetStringArgument(6, beaconData.extendedPanId, sizeof(beaconData.extendedPanId), false);
  emberGetStringArgument(7, beaconData.longId, sizeof(beaconData.longId), false);
  beaconData.protocolId = emberUnsignedCommandArgument(8);
  beaconData.version = emberUnsignedCommandArgument(9);
  emberGetStringArgument(10, beaconData.steeringData, sizeof(beaconData.steeringData), false);
  beaconData.steeringDataLength = emberUnsignedCommandArgument(11);
  emberActiveScanHandler(&beaconData);
}
#endif // HAS_CB_ACTIVE_SCAN_COMMAND_IDENTIFIER

#ifdef HAS_CB_SCAN_COMMAND_IDENTIFIER
static void scanReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberScanReturn(status);
}
#endif // HAS_CB_SCAN_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_ADDRESS_COMMAND_IDENTIFIER
static void setAddressHandlerCallback(void)
{
  uint8_t address[16];
  emberGetStringArgument(0, address, 16, false);
  emberSetAddressHandler(address);
}
#endif // HAS_CB_SET_ADDRESS_COMMAND_IDENTIFIER

#ifdef HAS_CB_START_HOST_JOIN_CLIENT_COMMAND_IDENTIFIER
static void startHostJoinClientHandlerCallback(void)
{
  uint8_t parentAddress[16];
  emberGetStringArgument(0, parentAddress, 16, false);
  emberStartHostJoinClientHandler(parentAddress);
}
#endif // HAS_CB_START_HOST_JOIN_CLIENT_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_TX_POWER_MODE_COMMAND_IDENTIFIER
static void setTxPowerModeReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberSetTxPowerModeReturn(status);
}
#endif // HAS_CB_SET_TX_POWER_MODE_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_TX_POWER_MODE_COMMAND_IDENTIFIER
static void getTxPowerModeReturnCallback(void)
{
  uint16_t txPowerMode;
  txPowerMode = (uint16_t) emberUnsignedCommandArgument(0);
  emberGetTxPowerModeReturn(txPowerMode);
}
#endif // HAS_CB_GET_TX_POWER_MODE_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_MULTICAST_ENTRY_COMMAND_IDENTIFIER
static void getMulticastEntryReturnCallback(void)
{
  uint8_t lastSequence;
  uint8_t windowBitmask;
  uint8_t dwellQs;
  uint8_t seed[8];
  lastSequence = (uint8_t) emberUnsignedCommandArgument(0);
  windowBitmask = (uint8_t) emberUnsignedCommandArgument(1);
  dwellQs = (uint8_t) emberUnsignedCommandArgument(2);
  emberGetStringArgument(3, seed, 8, false);
  emberGetMulticastEntryReturn(lastSequence,
                               windowBitmask,
                               dwellQs,
                               seed);
}
#endif // HAS_CB_GET_MULTICAST_ENTRY_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER
static void getCcaThresholdReturnCallback(void)
{
  int8_t threshold;
  threshold = (int8_t) emberSignedCommandArgument(0);
  emberGetCcaThresholdReturn(threshold);
}
#endif // HAS_CB_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER
static void setCcaThresholdReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberSetCcaThresholdReturn(status);
}
#endif // HAS_CB_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_RADIO_POWER_COMMAND_IDENTIFIER
static void getRadioPowerReturnCallback(void)
{
  int8_t power;
  power = (int8_t) emberSignedCommandArgument(0);
  emberGetRadioPowerReturn(power);
}
#endif // HAS_CB_GET_RADIO_POWER_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_RADIO_POWER_COMMAND_IDENTIFIER
static void setRadioPowerReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberSetRadioPowerReturn(status);
}
#endif // HAS_CB_SET_RADIO_POWER_COMMAND_IDENTIFIER

#ifdef HAS_CB_ECHO_COMMAND_IDENTIFIER
static void echoReturnCallback(void)
{
  uint8_t *data;
  uint8_t length;
  data = emberStringCommandArgument(0, &length);
  emberEchoReturn(data,
                  length);
}
#endif // HAS_CB_ECHO_COMMAND_IDENTIFIER

#ifdef HAS_CB_ASSERT_INFO_COMMAND_IDENTIFIER
static void assertInfoReturnCallback(void)
{
  uint8_t *fileName;
  uint32_t line;
  fileName = (uint8_t *) emberStringCommandArgument(0, NULL);
  line = (uint32_t) emberUnsignedCommandArgument(1);
  emberAssertInfoReturn(fileName,
                        line);
}
#endif // HAS_CB_ASSERT_INFO_COMMAND_IDENTIFIER

#ifdef HAS_CB_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER
static void configureGatewayReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberConfigureGatewayReturn(status);
}
#endif // HAS_CB_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_CHANNEL_CAL_DATA_TOKEN_COMMAND_IDENTIFIER
static void getChannelCalDataTokenReturnCallback(void)
{
  uint8_t lna;
  int8_t tempAtLna;
  uint8_t modDac;
  int8_t tempAtModDac;
  lna = (uint8_t) emberUnsignedCommandArgument(0);
  tempAtLna = (int8_t) emberSignedCommandArgument(1);
  modDac = (uint8_t) emberUnsignedCommandArgument(2);
  tempAtModDac = (int8_t) emberSignedCommandArgument(3);
  emberGetChannelCalDataTokenReturn(lna,
                                    tempAtLna,
                                    modDac,
                                    tempAtModDac);
}
#endif // HAS_CB_GET_CHANNEL_CAL_DATA_TOKEN_COMMAND_IDENTIFIER

#ifdef HAS_CB_POLL_FOR_DATA_COMMAND_IDENTIFIER
static void pollForDataReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberPollForDataReturn(status);
}
#endif // HAS_CB_POLL_FOR_DATA_COMMAND_IDENTIFIER

#ifdef HAS_CB_DEEP_SLEEP_COMMAND_IDENTIFIER
static void deepSleepReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberDeepSleepReturn(status);
}
#endif // HAS_CB_DEEP_SLEEP_COMMAND_IDENTIFIER

#ifdef HAS_CB_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER
static void stackPollForDataReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberStackPollForDataReturn(status);
}
#endif // HAS_CB_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER

#ifdef HAS_CB_OK_TO_NAP_COMMAND_IDENTIFIER
static void okToNapReturnCallback(void)
{
  uint8_t stateMask;
  stateMask = (uint8_t) emberUnsignedCommandArgument(0);
  emberOkToNapReturn(stateMask);
}
#endif // HAS_CB_OK_TO_NAP_COMMAND_IDENTIFIER

#ifdef HAS_CB_DEEP_SLEEP_COMPLETE_COMMAND_IDENTIFIER
static void deepSleepCompleteHandlerCallback(void)
{
  uint16_t sleepDuration;
  sleepDuration = (uint16_t) emberUnsignedCommandArgument(0);
  emberDeepSleepCompleteHandler(sleepDuration);
}
#endif // HAS_CB_DEEP_SLEEP_COMPLETE_COMMAND_IDENTIFIER

#ifdef HAS_CB_RESET_NETWORK_STATE_COMMAND_IDENTIFIER
static void resetNetworkStateReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberResetNetworkStateReturn(status);
}
#endif // HAS_CB_RESET_NETWORK_STATE_COMMAND_IDENTIFIER

#ifdef HAS_CB_EXTERNAL_ROUTE_CHANGE_COMMAND_IDENTIFIER
static void externalRouteChangeHandlerCallback(void)
{
  uint8_t prefix[16];
  uint8_t prefixLengthInBits;
  bool available;
  emberGetStringArgument(0, prefix, 16, false);
  prefixLengthInBits = (uint8_t) emberUnsignedCommandArgument(1);
  available = (bool) emberUnsignedCommandArgument(2);
  emberExternalRouteChangeHandler(prefix,
                                  prefixLengthInBits,
                                  available);
}
#endif // HAS_CB_EXTERNAL_ROUTE_CHANGE_COMMAND_IDENTIFIER

#ifdef HAS_CB_DHCP_SERVER_CHANGE_COMMAND_IDENTIFIER
static void dhcpServerChangeHandlerCallback(void)
{
  uint8_t prefix[16];
  uint8_t prefixLengthInBits;
  bool available;
  emberGetStringArgument(0, prefix, 16, false);
  prefixLengthInBits = (uint8_t) emberUnsignedCommandArgument(1);
  available = (bool) emberUnsignedCommandArgument(2);
  emberDhcpServerChangeHandler(prefix,
                               prefixLengthInBits,
                               available);
}
#endif // HAS_CB_DHCP_SERVER_CHANGE_COMMAND_IDENTIFIER

#ifdef HAS_CB_NCP_NETWORK_DATA_CHANGE_COMMAND_IDENTIFIER
static void ncpNetworkDataChangeHandlerCallback(void)
{
  uint16_t length;
  uint8_t *networkData;
  uint8_t bytesSent;
  length = (uint16_t) emberUnsignedCommandArgument(0);
  networkData = emberStringCommandArgument(1, &bytesSent);
  emberNcpNetworkDataChangeHandler(length,
                                   networkData,
                                   bytesSent);
}
#endif // HAS_CB_NCP_NETWORK_DATA_CHANGE_COMMAND_IDENTIFIER

#ifdef HAS_CB_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER
static void ncpGetNetworkDataReturnCallback(void)
{
  EmberStatus status;
  uint16_t totalLength;
  uint8_t *networkDataFragment;
  uint8_t fragmentLength;
  uint16_t fragmentOffset;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  totalLength = (uint16_t) emberUnsignedCommandArgument(1);
  networkDataFragment = emberStringCommandArgument(2, &fragmentLength);
  fragmentOffset = (uint16_t) emberUnsignedCommandArgument(3);
  emberNcpGetNetworkDataReturn(status,
                               totalLength,
                               networkDataFragment,
                               fragmentLength,
                               fragmentOffset);
}
#endif // HAS_CB_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER

#ifdef HAS_CB_ADDRESS_CONFIGURATION_CHANGE_COMMAND_IDENTIFIER
static void addressConfigurationChangeHandlerCallback(void)
{
  EmberIpv6Address address;
  uint32_t preferredLifetime;
  uint32_t validLifetime;
  uint8_t addressFlags;
  MEMSET(&address, 0, sizeof(EmberIpv6Address));
  emberIpv6AddressCommandArgument(0, &address);
  preferredLifetime = (uint32_t) emberUnsignedCommandArgument(1);
  validLifetime = (uint32_t) emberUnsignedCommandArgument(2);
  addressFlags = (uint8_t) emberUnsignedCommandArgument(3);
  bool preHookResult =
    tmspHostAddressConfigurationChangePreHook(&address,
                                              preferredLifetime,
                                              validLifetime,
                                              addressFlags);
  if (preHookResult) {
    emberAddressConfigurationChangeHandler(&address,
                                           preferredLifetime,
                                           validLifetime,
                                           addressFlags);
  }
}
#endif // HAS_CB_ADDRESS_CONFIGURATION_CHANGE_COMMAND_IDENTIFIER

#ifdef HAS_CB_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER
static void requestDhcpAddressReturnCallback(void)
{
  EmberStatus status;
  uint8_t prefix[16];
  uint8_t prefixLengthInBits;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberGetStringArgument(1, prefix, 16, false);
  prefixLengthInBits = (uint8_t) emberUnsignedCommandArgument(2);
  emberRequestDhcpAddressReturn(status,
                                prefix,
                                prefixLengthInBits);
}
#endif // HAS_CB_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER

#ifdef HAS_CB_COMMISSION_NETWORK_COMMAND_IDENTIFIER
static void commissionNetworkReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberCommissionNetworkReturn(status);
}
#endif // HAS_CB_COMMISSION_NETWORK_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_GLOBAL_ADDRESS_COMMAND_IDENTIFIER
static void getGlobalAddressReturnCallback(void)
{
  EmberIpv6Address address;
  uint32_t preferredLifetime;
  uint32_t validLifetime;
  uint8_t addressFlags;
  MEMSET(&address, 0, sizeof(EmberIpv6Address));
  emberGetStringArgument(0, address.bytes, EMBER_NETWORK_ID_SIZE, false);
  preferredLifetime = emberUnsignedCommandArgument(1);
  validLifetime = emberUnsignedCommandArgument(2);
  addressFlags = emberUnsignedCommandArgument(3);
  emberGetGlobalAddressReturn(&address,
                              preferredLifetime,
                              validLifetime,
                              addressFlags);
}
#endif // HAS_CB_GET_GLOBAL_ADDRESS_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_DHCP_CLIENT_COMMAND_IDENTIFIER
static void getDhcpClientReturnCallback(void)
{
  EmberIpv6Address address;
  MEMSET(&address, 0, sizeof(EmberIpv6Address));
  emberGetStringArgument(0, address.bytes, sizeof(address.bytes), false);
  emberGetDhcpClientReturn(&address);
}
#endif // HAS_CB_GET_DHCP_CLIENT_COMMAND_IDENTIFIER

#ifdef HAS_CB_BECOME_COMMISSIONER_COMMAND_IDENTIFIER
static void becomeCommissionerReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberBecomeCommissionerReturn(status);
}
#endif // HAS_CB_BECOME_COMMISSIONER_COMMAND_IDENTIFIER

#ifdef HAS_CB_SEND_STEERING_DATA_COMMAND_IDENTIFIER
static void sendSteeringDataReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberSendSteeringDataReturn(status);
}
#endif // HAS_CB_SEND_STEERING_DATA_COMMAND_IDENTIFIER

#ifdef HAS_CB_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER
static void changeNodeTypeReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberChangeNodeTypeReturn(status);
}
#endif // HAS_CB_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_GLOBAL_PREFIX_COMMAND_IDENTIFIER
static void getGlobalPrefixReturnCallback(void)
{
  uint8_t borderRouterFlags;
  bool isStable;
  uint8_t prefix[16];
  uint8_t prefixLengthInBits;
  uint8_t domainId;
  uint32_t preferredLifetime;
  uint32_t validLifetime;
  borderRouterFlags = (uint8_t) emberUnsignedCommandArgument(0);
  isStable = (bool) emberUnsignedCommandArgument(1);
  emberGetStringArgument(2, prefix, 16, false);
  prefixLengthInBits = (uint8_t) emberUnsignedCommandArgument(3);
  domainId = (uint8_t) emberUnsignedCommandArgument(4);
  preferredLifetime = (uint32_t) emberUnsignedCommandArgument(5);
  validLifetime = (uint32_t) emberUnsignedCommandArgument(6);
  emberGetGlobalPrefixReturn(borderRouterFlags,
                             isStable,
                             prefix,
                             prefixLengthInBits,
                             domainId,
                             preferredLifetime,
                             validLifetime);
}
#endif // HAS_CB_GET_GLOBAL_PREFIX_COMMAND_IDENTIFIER

#ifdef HAS_CB_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER
static void resignGlobalAddressReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberResignGlobalAddressReturn(status);
}
#endif // HAS_CB_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER

#ifdef HAS_CB_SLAAC_SERVER_CHANGE_COMMAND_IDENTIFIER
static void slaacServerChangeHandlerCallback(void)
{
  uint8_t prefix[16];
  uint8_t prefixLengthInBits;
  bool available;
  emberGetStringArgument(0, prefix, 16, false);
  prefixLengthInBits = (uint8_t) emberUnsignedCommandArgument(1);
  available = (bool) emberUnsignedCommandArgument(2);
  emberSlaacServerChangeHandler(prefix,
                                prefixLengthInBits,
                                available);
}
#endif // HAS_CB_SLAAC_SERVER_CHANGE_COMMAND_IDENTIFIER

#ifdef HAS_CB_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER
static void requestSlaacAddressReturnCallback(void)
{
  EmberStatus status;
  uint8_t prefix[16];
  uint8_t prefixLengthInBits;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberGetStringArgument(1, prefix, 16, false);
  prefixLengthInBits = (uint8_t) emberUnsignedCommandArgument(2);
  emberRequestSlaacAddressReturn(status,
                                 prefix,
                                 prefixLengthInBits);
}
#endif // HAS_CB_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER

#ifdef HAS_CB_CUSTOM_NCP_TO_HOST_MESSAGE_COMMAND_IDENTIFIER
static void customNcpToHostMessageCallback(void)
{
  uint8_t *message;
  uint8_t messageLength;
  message = emberStringCommandArgument(0, &messageLength);
  emberCustomNcpToHostMessageHandler(message,
                                     messageLength);
}
#endif // HAS_CB_CUSTOM_NCP_TO_HOST_MESSAGE_COMMAND_IDENTIFIER

#ifdef HAS_CB_NCP_TO_HOST_NO_OP_COMMAND_IDENTIFIER
static void ncpToHostNoOpCallback(void)
{
  uint8_t *bytes;
  uint8_t bytesLength;
  bytes = emberStringCommandArgument(0, &bytesLength);
  emberNcpToHostNoOp(bytes,
                     bytesLength);
}
#endif // HAS_CB_NCP_TO_HOST_NO_OP_COMMAND_IDENTIFIER

#ifdef HAS_CB_COMMISSIONER_STATUS_COMMAND_IDENTIFIER
static void commissionerStatusHandlerCallback(void)
{
  uint16_t flags;
  uint8_t *commissionerName;
  uint8_t commissionerNameLength;
  flags = (uint16_t) emberUnsignedCommandArgument(0);
  commissionerName = emberStringCommandArgument(1, &commissionerNameLength);
  emberCommissionerStatusHandler(flags,
                                 commissionerName,
                                 commissionerNameLength);
}
#endif // HAS_CB_COMMISSIONER_STATUS_COMMAND_IDENTIFIER

#ifdef HAS_CB_LEADER_DATA_COMMAND_IDENTIFIER
static void leaderDataHandlerCallback(void)
{
  uint8_t leaderData[EMBER_NETWORK_DATA_LEADER_SIZE];
  emberGetStringArgument(0, leaderData, EMBER_NETWORK_DATA_LEADER_SIZE, false);
  emberLeaderDataHandler(leaderData);
}
#endif // HAS_CB_LEADER_DATA_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER
static void getNetworkDataTlvReturnCallback(void)
{
  uint8_t type;
  uint8_t index;
  uint8_t versionNumber;
  uint8_t *tlv;
  uint8_t tlvLength;
  type = (uint8_t) emberUnsignedCommandArgument(0);
  index = (uint8_t) emberUnsignedCommandArgument(1);
  versionNumber = (uint8_t) emberUnsignedCommandArgument(2);
  tlv = emberStringCommandArgument(3, &tlvLength);
  emberGetNetworkDataTlvReturn(type,
                               index,
                               versionNumber,
                               tlv,
                               tlvLength);
}
#endif // HAS_CB_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER
static void getRoutingLocatorReturnCallback(void)
{
  EmberIpv6Address rloc;
  MEMSET(&rloc, 0, sizeof(EmberIpv6Address));
  emberIpv6AddressCommandArgument(0, &rloc);
  emberGetRoutingLocatorReturn(&rloc);
}
#endif // HAS_CB_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER
static void setRandomizeMacExtendedIdReturnCallback(void)
{
  emberSetRandomizeMacExtendedIdReturn();
}
#endif // HAS_CB_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER

#ifdef HAS_CB_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER
static void configureExternalRouteReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberConfigureExternalRouteReturn(status);
}
#endif // HAS_CB_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER

#ifdef HAS_CB_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER
static void allowNativeCommissionerReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberAllowNativeCommissionerReturn(status);
}
#endif // HAS_CB_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER
static void setCommissionerKeyReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberSetCommissionerKeyReturn(status);
}
#endif // HAS_CB_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER

#ifdef HAS_CB_NETWORK_STATUS_COMMAND_IDENTIFIER
static void networkStatusHandlerCallback(void)
{
  EmberNetworkStatus newNetworkStatus;
  EmberNetworkStatus oldNetworkStatus;
  EmberJoinFailureReason reason;
  newNetworkStatus = (EmberNetworkStatus) emberUnsignedCommandArgument(0);
  oldNetworkStatus = (EmberNetworkStatus) emberUnsignedCommandArgument(1);
  reason = (EmberJoinFailureReason) emberUnsignedCommandArgument(2);
  emberNetworkStatusHandler(newNetworkStatus,
                            oldNetworkStatus,
                            reason);
}
#endif // HAS_CB_NETWORK_STATUS_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER
static void getStandaloneBootloaderInfoReturnCallback(void)
{
  uint16_t version;
  uint8_t platformId;
  uint8_t microId;
  uint8_t phyId;
  version = (uint16_t) emberUnsignedCommandArgument(0);
  platformId = (uint8_t) emberUnsignedCommandArgument(1);
  microId = (uint8_t) emberUnsignedCommandArgument(2);
  phyId = (uint8_t) emberUnsignedCommandArgument(3);
  emberGetStandaloneBootloaderInfoReturn(version,
                                         platformId,
                                         microId,
                                         phyId);
}
#endif // HAS_CB_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER

#ifdef HAS_CB_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER
static void launchStandaloneBootloaderReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberLaunchStandaloneBootloaderReturn(status);
}
#endif // HAS_CB_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_MFG_TOKEN_COMMAND_IDENTIFIER
static void getMfgTokenReturnCallback(void)
{
  EmberMfgTokenId tokenId;
  EmberStatus status;
  uint8_t *tokenData;
  uint8_t tokenDataLength;
  tokenId = (EmberMfgTokenId) emberUnsignedCommandArgument(0);
  status = (EmberStatus) emberUnsignedCommandArgument(1);
  tokenData = emberStringCommandArgument(2, &tokenDataLength);
  emberGetMfgTokenReturn(tokenId,
                         status,
                         tokenData,
                         tokenDataLength);
}
#endif // HAS_CB_GET_MFG_TOKEN_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_MFG_TOKEN_COMMAND_IDENTIFIER
static void setMfgTokenReturnCallback(void)
{
  EmberMfgTokenId tokenId;
  EmberStatus status;
  tokenId = (EmberMfgTokenId) emberUnsignedCommandArgument(0);
  status = (EmberStatus) emberUnsignedCommandArgument(1);
  emberSetMfgTokenReturn(tokenId,
                         status);
}
#endif // HAS_CB_SET_MFG_TOKEN_COMMAND_IDENTIFIER

#ifdef HAS_CB_HOST_STATE_COMMAND_IDENTIFIER
static void hostStateHandlerCallback(void)
{
  EmberNetworkParameters parameters;
  EmberEui64 localEui64;
  EmberEui64 macExtendedId;
  EmberNetworkStatus networkStatus;
  MEMSET(&parameters, 0, sizeof(EmberNetworkParameters));
  MEMSET(&localEui64, 0, sizeof(EmberEui64));
  MEMSET(&macExtendedId, 0, sizeof(EmberEui64));
  emberGetStringArgument(0, parameters.networkId, sizeof(parameters.networkId), false);
  emberGetStringArgument(1, parameters.ulaPrefix.bytes, sizeof(parameters.ulaPrefix.bytes), false);
  emberGetStringArgument(2, parameters.extendedPanId, sizeof(parameters.extendedPanId), false);
  parameters.panId = emberUnsignedCommandArgument(3);
  parameters.channel = emberUnsignedCommandArgument(4);
  parameters.nodeType = emberUnsignedCommandArgument(5);
  parameters.radioTxPower = emberSignedCommandArgument(6);
  networkStatus = emberUnsignedCommandArgument(7);
  emberGetStringArgument(8, localEui64.bytes, sizeof(localEui64.bytes), false);
  emberGetStringArgument(9, macExtendedId.bytes, sizeof(macExtendedId.bytes), false);
  emberGetStringArgument(10, parameters.legacyUla.bytes, sizeof(parameters.legacyUla.bytes), false);
  emberHostStateHandler(&parameters,
                        &localEui64,
                        &macExtendedId,
                        networkStatus);
}
#endif // HAS_CB_HOST_STATE_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_CTUNE_COMMAND_IDENTIFIER
static void getCtuneReturnCallback(void)
{
  uint16_t tune;
  EmberStatus status;
  tune = (uint16_t) emberUnsignedCommandArgument(0);
  status = (EmberStatus) emberUnsignedCommandArgument(1);
  emberGetCtuneReturn(tune,
                      status);
}
#endif // HAS_CB_GET_CTUNE_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_CTUNE_COMMAND_IDENTIFIER
static void setCtuneReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberSetCtuneReturn(status);
}
#endif // HAS_CB_SET_CTUNE_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER
static void setRadioHoldOffReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberSetRadioHoldOffReturn(status);
}
#endif // HAS_CB_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_PTA_ENABLE_COMMAND_IDENTIFIER
static void getPtaEnableReturnCallback(void)
{
  bool enabled;
  enabled = (bool) emberUnsignedCommandArgument(0);
  emberGetPtaEnableReturn(enabled);
}
#endif // HAS_CB_GET_PTA_ENABLE_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_PTA_ENABLE_COMMAND_IDENTIFIER
static void setPtaEnableReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberSetPtaEnableReturn(status);
}
#endif // HAS_CB_SET_PTA_ENABLE_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_ANTENNA_MODE_COMMAND_IDENTIFIER
static void getAntennaModeReturnCallback(void)
{
  EmberStatus status;
  uint8_t mode;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  mode = (uint8_t) emberUnsignedCommandArgument(1);
  emberGetAntennaModeReturn(status,
                            mode);
}
#endif // HAS_CB_GET_ANTENNA_MODE_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_ANTENNA_MODE_COMMAND_IDENTIFIER
static void setAntennaModeReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberSetAntennaModeReturn(status);
}
#endif // HAS_CB_SET_ANTENNA_MODE_COMMAND_IDENTIFIER

#ifdef HAS_CB_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER
static void radioGetRandomNumbersReturnCallback(void)
{
  EmberStatus status;
  uint16_t *rn;
  uint8_t count;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  rn = (uint16_t *) emberStringCommandArgument(1, NULL);
  count = (uint8_t) emberUnsignedCommandArgument(2);
  emberRadioGetRandomNumbersReturn(status,
                                   rn,
                                   count);
}
#endif // HAS_CB_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_PTA_OPTIONS_COMMAND_IDENTIFIER
static void getPtaOptionsReturnCallback(void)
{
  uint32_t options;
  options = (uint32_t) emberUnsignedCommandArgument(0);
  emberGetPtaOptionsReturn(options);
}
#endif // HAS_CB_GET_PTA_OPTIONS_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_PTA_OPTIONS_COMMAND_IDENTIFIER
static void setPtaOptionsReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberSetPtaOptionsReturn(status);
}
#endif // HAS_CB_SET_PTA_OPTIONS_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_JOIN_KEY_COMMAND_IDENTIFIER
static void setJoinKeyReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberSetJoinKeyReturn(status);
}
#endif // HAS_CB_SET_JOIN_KEY_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_ND_DATA_COMMAND_IDENTIFIER
static void setNdDataReturnCallback(void)
{
  EmberNetworkStatus status;
  uint16_t length;
  status = (EmberNetworkStatus) emberUnsignedCommandArgument(0);
  length = (uint16_t) emberUnsignedCommandArgument(1);
  emberSetNdDataReturn(status,
                       length);
}
#endif // HAS_CB_SET_ND_DATA_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_LOCAL_NETWORK_DATA_COMMAND_IDENTIFIER
static void setLocalNetworkDataReturnCallback(void)
{
  EmberNetworkStatus status;
  uint16_t length;
  status = (EmberNetworkStatus) emberUnsignedCommandArgument(0);
  length = (uint16_t) emberUnsignedCommandArgument(1);
  emberSetLocalNetworkDataReturn(status,
                                 length);
}
#endif // HAS_CB_SET_LOCAL_NETWORK_DATA_COMMAND_IDENTIFIER

#ifdef HAS_CB_SET_PSKC_COMMAND_IDENTIFIER
static void setPskcHandlerCallback(void)
{
  uint8_t pskc[16];
  emberGetStringArgument(0, pskc, 16, false);
  emberSetPskcHandler(pskc);
}
#endif // HAS_CB_SET_PSKC_COMMAND_IDENTIFIER

//------------------------------------------------------------------------------
// MFGLIB
//------------------------------------------------------------------------------
#ifdef MFGLIB

#ifdef HAS_EMBER_MFGLIB_START_COMMAND_IDENTIFIER
void tmspMfglibStart(bool requestRxCallback)
{
  emSendBinaryManagementCommand(EMBER_MFGLIB_START_COMMAND_IDENTIFIER,
                                "u",
                                requestRxCallback);
}
#endif // HAS_EMBER_MFGLIB_START_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_MFGLIB_END_COMMAND_IDENTIFIER
void tmspMfglibEnd(void)
{
  emSendBinaryManagementCommand(EMBER_MFGLIB_END_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_MFGLIB_END_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_MFGLIB_START_ACTIVITY_COMMAND_IDENTIFIER
void tmspMfglibStartActivity(TmspMfglibActivities type)
{
  emSendBinaryManagementCommand(EMBER_MFGLIB_START_ACTIVITY_COMMAND_IDENTIFIER,
                                "u",
                                type);
}
#endif // HAS_EMBER_MFGLIB_START_ACTIVITY_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_MFGLIB_STOP_ACTIVITY_COMMAND_IDENTIFIER
void tmspMfglibStopActivity(TmspMfglibActivities type)
{
  emSendBinaryManagementCommand(EMBER_MFGLIB_STOP_ACTIVITY_COMMAND_IDENTIFIER,
                                "u",
                                type);
}
#endif // HAS_EMBER_MFGLIB_STOP_ACTIVITY_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_MFGLIB_SEND_PACKET_COMMAND_IDENTIFIER
void tmspMfglibSendPacket(const uint8_t *packet,
                          uint8_t packetLength,
                          uint16_t repeat)
{
  emSendBinaryManagementCommand(EMBER_MFGLIB_SEND_PACKET_COMMAND_IDENTIFIER,
                                "bv",
                                packet, packetLength,
                                repeat);
}
#endif // HAS_EMBER_MFGLIB_SEND_PACKET_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_MFGLIB_SET_COMMAND_IDENTIFIER
void tmspMfglibSet(TmspMfglibValues type,
                   uint16_t arg1,
                   int8_t arg2)
{
  emSendBinaryManagementCommand(EMBER_MFGLIB_SET_COMMAND_IDENTIFIER,
                                "uvs",
                                type,
                                arg1,
                                arg2);
}
#endif // HAS_EMBER_MFGLIB_SET_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_MFGLIB_GET_COMMAND_IDENTIFIER
void tmspMfglibGet(TmspMfglibValues type)
{
  emSendBinaryManagementCommand(EMBER_MFGLIB_GET_COMMAND_IDENTIFIER,
                                "u",
                                type);
}
#endif // HAS_EMBER_MFGLIB_GET_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_MFGLIB_TEST_CONT_MOD_CAL_COMMAND_IDENTIFIER
void tmspMfglibTestContModCal(uint8_t channel,
                              uint32_t duration)
{
  emSendBinaryManagementCommand(EMBER_MFGLIB_TEST_CONT_MOD_CAL_COMMAND_IDENTIFIER,
                                "uw",
                                channel,
                                duration);
}
#endif // HAS_EMBER_MFGLIB_TEST_CONT_MOD_CAL_COMMAND_IDENTIFIER

#ifdef HAS_CB_MFGLIB_START_TEST_COMMAND_IDENTIFIER
static void mfglibStartTestReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  tmspMfglibStartTestReturn(status);
}
#endif // HAS_CB_MFGLIB_START_TEST_COMMAND_IDENTIFIER

#ifdef HAS_CB_MFGLIB_RX_COMMAND_IDENTIFIER
static void mfglibRxReturnCallback(void)
{
  uint8_t *payload;
  uint8_t payloadLength;
  uint8_t lqi;
  int8_t rssi;
  payload = emberStringCommandArgument(0, &payloadLength);
  lqi = (uint8_t) emberUnsignedCommandArgument(1);
  rssi = (int8_t) emberSignedCommandArgument(2);
  tmspMfglibRxReturn(payload,
                     payloadLength,
                     lqi,
                     rssi);
}
#endif // HAS_CB_MFGLIB_RX_COMMAND_IDENTIFIER

#ifdef HAS_CB_MFGLIB_END_TEST_COMMAND_IDENTIFIER
static void mfglibEndTestReturnCallback(void)
{
  EmberStatus status;
  uint32_t mfgReceiveCount;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  mfgReceiveCount = (uint32_t) emberUnsignedCommandArgument(1);
  tmspMfglibEndTestReturn(status,
                          mfgReceiveCount);
}
#endif // HAS_CB_MFGLIB_END_TEST_COMMAND_IDENTIFIER

#ifdef HAS_CB_MFGLIB_START_COMMAND_IDENTIFIER
static void mfglibStartReturnCallback(void)
{
  TmspMfglibActivities type;
  EmberStatus status;
  type = (TmspMfglibActivities) emberUnsignedCommandArgument(0);
  status = (EmberStatus) emberUnsignedCommandArgument(1);
  tmspMfglibStartReturn(type,
                        status);
}
#endif // HAS_CB_MFGLIB_START_COMMAND_IDENTIFIER

#ifdef HAS_CB_MFGLIB_STOP_COMMAND_IDENTIFIER
static void mfglibStopReturnCallback(void)
{
  TmspMfglibActivities type;
  EmberStatus status;
  type = (TmspMfglibActivities) emberUnsignedCommandArgument(0);
  status = (EmberStatus) emberUnsignedCommandArgument(1);
  tmspMfglibStopReturn(type,
                       status);
}
#endif // HAS_CB_MFGLIB_STOP_COMMAND_IDENTIFIER

#ifdef HAS_CB_MFGLIB_SEND_PACKET_EVENT_COMMAND_IDENTIFIER
static void mfglibSendPacketEventHandlerCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  tmspMfglibSendPacketEventHandler(status);
}
#endif // HAS_CB_MFGLIB_SEND_PACKET_EVENT_COMMAND_IDENTIFIER

#ifdef HAS_CB_MFGLIB_SET_COMMAND_IDENTIFIER
static void mfglibSetReturnCallback(void)
{
  TmspMfglibValues type;
  EmberStatus status;
  type = (TmspMfglibValues) emberUnsignedCommandArgument(0);
  status = (EmberStatus) emberUnsignedCommandArgument(1);
  tmspMfglibSetReturn(type,
                      status);
}
#endif // HAS_CB_MFGLIB_SET_COMMAND_IDENTIFIER

#ifdef HAS_CB_MFGLIB_GET_CHANNEL_COMMAND_IDENTIFIER
static void mfglibGetChannelReturnCallback(void)
{
  uint8_t channel;
  channel = (uint8_t) emberUnsignedCommandArgument(0);
  tmspMfglibGetChannelReturn(channel);
}
#endif // HAS_CB_MFGLIB_GET_CHANNEL_COMMAND_IDENTIFIER

#ifdef HAS_CB_MFGLIB_GET_POWER_COMMAND_IDENTIFIER
static void mfglibGetPowerReturnCallback(void)
{
  int8_t power;
  power = (int8_t) emberSignedCommandArgument(0);
  tmspMfglibGetPowerReturn(power);
}
#endif // HAS_CB_MFGLIB_GET_POWER_COMMAND_IDENTIFIER

#ifdef HAS_CB_MFGLIB_GET_POWER_MODE_COMMAND_IDENTIFIER
static void mfglibGetPowerModeReturnCallback(void)
{
  uint16_t txPowerMode;
  txPowerMode = (uint16_t) emberUnsignedCommandArgument(0);
  tmspMfglibGetPowerModeReturn(txPowerMode);
}
#endif // HAS_CB_MFGLIB_GET_POWER_MODE_COMMAND_IDENTIFIER

#ifdef HAS_CB_MFGLIB_GET_SYN_OFFSET_COMMAND_IDENTIFIER
static void mfglibGetSynOffsetReturnCallback(void)
{
  int8_t synOffset;
  synOffset = (int8_t) emberSignedCommandArgument(0);
  tmspMfglibGetSynOffsetReturn(synOffset);
}
#endif // HAS_CB_MFGLIB_GET_SYN_OFFSET_COMMAND_IDENTIFIER

#ifdef HAS_CB_MFGLIB_GET_OPTIONS_COMMAND_IDENTIFIER
static void mfglibGetOptionsReturnCallback(void)
{
  uint8_t options;
  options = (uint8_t) emberUnsignedCommandArgument(0);
  tmspMfglibGetOptionsReturn(options);
}
#endif // HAS_CB_MFGLIB_GET_OPTIONS_COMMAND_IDENTIFIER
#endif /* MFGLIB */

//------------------------------------------------------------------------------
// EMBER_TEST
//------------------------------------------------------------------------------
#ifdef EMBER_TEST

#ifdef HAS_EMBER_CONFIG_UART_COMMAND_IDENTIFIER
void emberConfigUart(uint8_t dropPercentage,
                     uint8_t corruptPercentage)
{
  emSendBinaryManagementCommand(EMBER_CONFIG_UART_COMMAND_IDENTIFIER,
                                "uu",
                                dropPercentage,
                                corruptPercentage);
}
#endif // HAS_EMBER_CONFIG_UART_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_RESET_NCP_ASH_COMMAND_IDENTIFIER
void emberResetNcpAsh(void)
{
  emSendBinaryManagementCommand(EMBER_RESET_NCP_ASH_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_RESET_NCP_ASH_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_RESET_IP_DRIVER_ASH_COMMAND_IDENTIFIER
void emberResetIpDriverAsh(void)
{
  emSendBinaryManagementCommand(EMBER_RESET_IP_DRIVER_ASH_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_RESET_IP_DRIVER_ASH_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_START_UART_STORM_COMMAND_IDENTIFIER
void emberStartUartStorm(uint16_t rate)
{
  emSendBinaryManagementCommand(EMBER_START_UART_STORM_COMMAND_IDENTIFIER,
                                "v",
                                rate);
}
#endif // HAS_EMBER_START_UART_STORM_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_STOP_UART_STORM_COMMAND_IDENTIFIER
void emberStopUartStorm(void)
{
  emSendBinaryManagementCommand(EMBER_STOP_UART_STORM_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_STOP_UART_STORM_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_SEND_DONE_COMMAND_IDENTIFIER
void emberSendDone(uint32_t timeoutMs)
{
  emSendBinaryManagementCommand(EMBER_SEND_DONE_COMMAND_IDENTIFIER,
                                "w",
                                timeoutMs);
}
#endif // HAS_EMBER_SEND_DONE_COMMAND_IDENTIFIER

#ifdef HAS_CB_CONFIG_UART_COMMAND_IDENTIFIER
static void configUartReturnCallback(void)
{
  emberConfigUartReturn();
}
#endif // HAS_CB_CONFIG_UART_COMMAND_IDENTIFIER

#ifdef HAS_CB_RESET_NCP_ASH_COMMAND_IDENTIFIER
static void resetNcpAshReturnCallback(void)
{
  emberResetNcpAshReturn();
}
#endif // HAS_CB_RESET_NCP_ASH_COMMAND_IDENTIFIER

#ifdef HAS_CB_START_UART_STORM_COMMAND_IDENTIFIER
static void startUartStormReturnCallback(void)
{
  emberStartUartStormReturn();
}
#endif // HAS_CB_START_UART_STORM_COMMAND_IDENTIFIER

#ifdef HAS_CB_STOP_UART_STORM_COMMAND_IDENTIFIER
static void stopUartStormReturnCallback(void)
{
  emberStopUartStormReturn();
}
#endif // HAS_CB_STOP_UART_STORM_COMMAND_IDENTIFIER

#ifdef HAS_CB_SEND_DONE_COMMAND_IDENTIFIER
static void sendDoneReturnCallback(void)
{
  emberSendDoneReturn();
}
#endif // HAS_CB_SEND_DONE_COMMAND_IDENTIFIER
#endif /* EMBER_TEST */

//------------------------------------------------------------------------------
// QA_THREAD_TEST
//------------------------------------------------------------------------------
#if (defined (EMBER_TEST) || defined(QA_THREAD_TEST))

#ifdef HAS_EMBER_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER
void emberGetNetworkKeyInfo(bool keyInUse)
{
  emSendBinaryManagementCommand(EMBER_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER,
                                "u",
                                keyInUse);
}
#endif // HAS_EMBER_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_RESET_NCP_GPIO_COMMAND_IDENTIFIER
void emberResetNcpGpio(void)
{
  emSendBinaryManagementCommand(EMBER_RESET_NCP_GPIO_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_RESET_NCP_GPIO_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_ENABLE_RESET_NCP_GPIO_COMMAND_IDENTIFIER
void emberEnableResetNcpGpio(uint8_t enable)
{
  emSendBinaryManagementCommand(EMBER_ENABLE_RESET_NCP_GPIO_COMMAND_IDENTIFIER,
                                "u",
                                enable);
}
#endif // HAS_EMBER_ENABLE_RESET_NCP_GPIO_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_FORCE_ASSERT_COMMAND_IDENTIFIER
void emberForceAssert(void)
{
  emSendBinaryManagementCommand(EMBER_FORCE_ASSERT_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_FORCE_ASSERT_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_GET_NODE_STATUS_COMMAND_IDENTIFIER
void emberGetNodeStatus(void)
{
  emSendBinaryManagementCommand(EMBER_GET_NODE_STATUS_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_GET_NODE_STATUS_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER
void emberAddAddressData(const uint8_t *longId,
                         uint16_t shortId)
{
  emSendBinaryManagementCommand(EMBER_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER,
                                "bv",
                                longId, 8,
                                shortId);
}
#endif // HAS_EMBER_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER
void emberClearAddressCache(void)
{
  emSendBinaryManagementCommand(EMBER_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER
void emberLookupAddressData(const uint8_t *longId)
{
  emSendBinaryManagementCommand(EMBER_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER,
                                "b",
                                longId, 8);
}
#endif // HAS_EMBER_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_START_UART_SPEED_TEST_COMMAND_IDENTIFIER
void emberStartUartSpeedTest(uint8_t payloadLength,
                             uint32_t timeoutMs,
                             uint32_t intervalMs)
{
  emSendBinaryManagementCommand(EMBER_START_UART_SPEED_TEST_COMMAND_IDENTIFIER,
                                "uww",
                                payloadLength,
                                timeoutMs,
                                intervalMs);
}
#endif // HAS_EMBER_START_UART_SPEED_TEST_COMMAND_IDENTIFIER

#ifdef HAS_EMBER_NCP_UDP_STORM_COMMAND_IDENTIFIER
void emberNcpUdpStorm(uint8_t totalPackets,
                      const uint8_t *dest,
                      uint16_t payloadLength,
                      uint16_t txDelayMs)
{
  emSendBinaryManagementCommand(EMBER_NCP_UDP_STORM_COMMAND_IDENTIFIER,
                                "ubvv",
                                totalPackets,
                                dest, 16,
                                payloadLength,
                                txDelayMs);
}
#endif // HAS_EMBER_NCP_UDP_STORM_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER
static void getNetworkKeyInfoReturnCallback(void)
{
  EmberStatus status;
  uint32_t sequence;
  uint8_t state;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  sequence = (uint32_t) emberUnsignedCommandArgument(1);
  state = (uint8_t) emberUnsignedCommandArgument(2);
  emberGetNetworkKeyInfoReturn(status,
                               sequence,
                               state);
}
#endif // HAS_CB_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER

#ifdef HAS_CB_GET_NODE_STATUS_COMMAND_IDENTIFIER
static void getNodeStatusReturnCallback(void)
{
  EmberStatus status;
  uint8_t ripId;
  EmberNodeId nodeId;
  uint8_t parentRipId;
  EmberNodeId parentId;
  uint8_t networkFragmentIdentifier[ISLAND_ID_SIZE];
  uint32_t networkFrameCounter;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  ripId = (uint8_t) emberUnsignedCommandArgument(1);
  nodeId = (EmberNodeId) emberUnsignedCommandArgument(2);
  parentRipId = (uint8_t) emberUnsignedCommandArgument(3);
  parentId = (EmberNodeId) emberUnsignedCommandArgument(4);
  emberGetStringArgument(5, networkFragmentIdentifier, ISLAND_ID_SIZE, false);
  networkFrameCounter = (uint32_t) emberUnsignedCommandArgument(6);
  emberGetNodeStatusReturn(status,
                           ripId,
                           nodeId,
                           parentRipId,
                           parentId,
                           networkFragmentIdentifier,
                           networkFrameCounter);
}
#endif // HAS_CB_GET_NODE_STATUS_COMMAND_IDENTIFIER

#ifdef HAS_CB_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER
static void addAddressDataReturnCallback(void)
{
  uint16_t shortId;
  shortId = (uint16_t) emberUnsignedCommandArgument(0);
  emberAddAddressDataReturn(shortId);
}
#endif // HAS_CB_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER

#ifdef HAS_CB_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER
static void clearAddressCacheReturnCallback(void)
{
  emberClearAddressCacheReturn();
}
#endif // HAS_CB_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER

#ifdef HAS_CB_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER
static void lookupAddressDataReturnCallback(void)
{
  uint16_t shortId;
  shortId = (uint16_t) emberUnsignedCommandArgument(0);
  emberLookupAddressDataReturn(shortId);
}
#endif // HAS_CB_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER

#ifdef HAS_CB_UART_SPEED_TEST_COMMAND_IDENTIFIER
static void uartSpeedTestReturnCallback(void)
{
  uint32_t totalBytesSent;
  uint32_t payloadBytesSent;
  uint32_t uartStormTimeoutMs;
  totalBytesSent = (uint32_t) emberUnsignedCommandArgument(0);
  payloadBytesSent = (uint32_t) emberUnsignedCommandArgument(1);
  uartStormTimeoutMs = (uint32_t) emberUnsignedCommandArgument(2);
  emberUartSpeedTestReturn(totalBytesSent,
                           payloadBytesSent,
                           uartStormTimeoutMs);
}
#endif // HAS_CB_UART_SPEED_TEST_COMMAND_IDENTIFIER

#ifdef HAS_CB_NCP_UDP_STORM_COMMAND_IDENTIFIER
static void ncpUdpStormReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  emberNcpUdpStormReturn(status);
}
#endif // HAS_CB_NCP_UDP_STORM_COMMAND_IDENTIFIER

#ifdef HAS_CB_NCP_UDP_STORM_COMPLETE_COMMAND_IDENTIFIER
static void ncpUdpStormCompleteHandlerCallback(void)
{
  emberNcpUdpStormCompleteHandler();
}
#endif // HAS_CB_NCP_UDP_STORM_COMPLETE_COMMAND_IDENTIFIER
#endif /* (defined (EMBER_TEST) || defined(QA_THREAD_TEST)) */

//------------------------------------------------------------------------------
// APP_USES_SOFTWARE_FLOW_CONTROL
//------------------------------------------------------------------------------
#ifdef EMBER_APPLICATION_USES_SOFTWARE_FLOW_CONTROL

#ifdef HAS_EMBER_START_XON_XOFF_TEST_COMMAND_IDENTIFIER
void emberStartXOnXOffTest(void)
{
  emSendBinaryManagementCommand(EMBER_START_XON_XOFF_TEST_COMMAND_IDENTIFIER,
                                "");
}
#endif // HAS_EMBER_START_XON_XOFF_TEST_COMMAND_IDENTIFIER
#endif /* EMBER_APPLICATION_USES_SOFTWARE_FLOW_CONTROL */

//------------------------------------------------------------------------------
// Command Handler Table

const EmberCommandEntry managementCallbackCommandTable[] = {
#ifdef HAS_CB_RESET_MICRO_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_RESET_MICRO_COMMAND_IDENTIFIER,
                                resetMicroHandlerCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_STATE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_STATE_COMMAND_IDENTIFIER,
                                stateReturnCallback,
                                "bbbvuusubbb",
                                NULL),
#endif
#ifdef HAS_CB_GET_VERSIONS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_VERSIONS_COMMAND_IDENTIFIER,
                                getVersionsReturnCallback,
                                "bvvvub",
                                NULL),
#endif
#ifdef HAS_CB_GET_RIP_ENTRY_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_RIP_ENTRY_COMMAND_IDENTIFIER,
                                getRipEntryReturnCallback,
                                "ubusuuuuuu",
                                NULL),
#endif
#ifdef HAS_CB_INIT_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_INIT_COMMAND_IDENTIFIER,
                                initReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_GET_COUNTER_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_COUNTER_COMMAND_IDENTIFIER,
                                getCounterReturnCallback,
                                "uv",
                                NULL),
#endif
#ifdef HAS_CB_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER,
                                setSecurityParametersReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER,
                                switchToNextNetworkKeyReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_FORM_NETWORK_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_FORM_NETWORK_COMMAND_IDENTIFIER,
                                formNetworkReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_JOIN_NETWORK_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_JOIN_NETWORK_COMMAND_IDENTIFIER,
                                joinNetworkReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_RESUME_NETWORK_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_RESUME_NETWORK_COMMAND_IDENTIFIER,
                                resumeNetworkReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER,
                                attachToNetworkReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_ENERGY_SCAN_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_ENERGY_SCAN_COMMAND_IDENTIFIER,
                                energyScanHandlerCallback,
                                "us",
                                NULL),
#endif
#ifdef HAS_CB_ACTIVE_SCAN_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_ACTIVE_SCAN_COMMAND_IDENTIFIER,
                                activeScanHandlerCallback,
                                "uuuubvbbuubu",
                                NULL),
#endif
#ifdef HAS_CB_SCAN_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SCAN_COMMAND_IDENTIFIER,
                                scanReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_SET_ADDRESS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_ADDRESS_COMMAND_IDENTIFIER,
                                setAddressHandlerCallback,
                                "b",
                                NULL),
#endif
#ifdef HAS_CB_START_HOST_JOIN_CLIENT_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_START_HOST_JOIN_CLIENT_COMMAND_IDENTIFIER,
                                startHostJoinClientHandlerCallback,
                                "b",
                                NULL),
#endif
#ifdef HAS_CB_SET_TX_POWER_MODE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_TX_POWER_MODE_COMMAND_IDENTIFIER,
                                setTxPowerModeReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_GET_TX_POWER_MODE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_TX_POWER_MODE_COMMAND_IDENTIFIER,
                                getTxPowerModeReturnCallback,
                                "v",
                                NULL),
#endif
#ifdef HAS_CB_GET_MULTICAST_ENTRY_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_MULTICAST_ENTRY_COMMAND_IDENTIFIER,
                                getMulticastEntryReturnCallback,
                                "uuub",
                                NULL),
#endif
#ifdef HAS_CB_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER,
                                getCcaThresholdReturnCallback,
                                "s",
                                NULL),
#endif
#ifdef HAS_CB_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER,
                                setCcaThresholdReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_GET_RADIO_POWER_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_RADIO_POWER_COMMAND_IDENTIFIER,
                                getRadioPowerReturnCallback,
                                "s",
                                NULL),
#endif
#ifdef HAS_CB_SET_RADIO_POWER_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_RADIO_POWER_COMMAND_IDENTIFIER,
                                setRadioPowerReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_ECHO_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_ECHO_COMMAND_IDENTIFIER,
                                echoReturnCallback,
                                "b",
                                NULL),
#endif
#ifdef HAS_CB_ASSERT_INFO_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_ASSERT_INFO_COMMAND_IDENTIFIER,
                                assertInfoReturnCallback,
                                "bw",
                                NULL),
#endif
#ifdef HAS_CB_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER,
                                configureGatewayReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_GET_CHANNEL_CAL_DATA_TOKEN_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_CHANNEL_CAL_DATA_TOKEN_COMMAND_IDENTIFIER,
                                getChannelCalDataTokenReturnCallback,
                                "usus",
                                NULL),
#endif
#ifdef HAS_CB_POLL_FOR_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_POLL_FOR_DATA_COMMAND_IDENTIFIER,
                                pollForDataReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_DEEP_SLEEP_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_DEEP_SLEEP_COMMAND_IDENTIFIER,
                                deepSleepReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER,
                                stackPollForDataReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_OK_TO_NAP_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_OK_TO_NAP_COMMAND_IDENTIFIER,
                                okToNapReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_DEEP_SLEEP_COMPLETE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_DEEP_SLEEP_COMPLETE_COMMAND_IDENTIFIER,
                                deepSleepCompleteHandlerCallback,
                                "v",
                                NULL),
#endif
#ifdef HAS_CB_RESET_NETWORK_STATE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_RESET_NETWORK_STATE_COMMAND_IDENTIFIER,
                                resetNetworkStateReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_EXTERNAL_ROUTE_CHANGE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_EXTERNAL_ROUTE_CHANGE_COMMAND_IDENTIFIER,
                                externalRouteChangeHandlerCallback,
                                "buu",
                                NULL),
#endif
#ifdef HAS_CB_DHCP_SERVER_CHANGE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_DHCP_SERVER_CHANGE_COMMAND_IDENTIFIER,
                                dhcpServerChangeHandlerCallback,
                                "buu",
                                NULL),
#endif
#ifdef HAS_CB_NCP_NETWORK_DATA_CHANGE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_NCP_NETWORK_DATA_CHANGE_COMMAND_IDENTIFIER,
                                ncpNetworkDataChangeHandlerCallback,
                                "vb",
                                NULL),
#endif
#ifdef HAS_CB_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER,
                                ncpGetNetworkDataReturnCallback,
                                "uvbv",
                                NULL),
#endif
#ifdef HAS_CB_ADDRESS_CONFIGURATION_CHANGE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_ADDRESS_CONFIGURATION_CHANGE_COMMAND_IDENTIFIER,
                                addressConfigurationChangeHandlerCallback,
                                "bwwu",
                                NULL),
#endif
#ifdef HAS_CB_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER,
                                requestDhcpAddressReturnCallback,
                                "ubu",
                                NULL),
#endif
#ifdef HAS_CB_COMMISSION_NETWORK_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_COMMISSION_NETWORK_COMMAND_IDENTIFIER,
                                commissionNetworkReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_GET_GLOBAL_ADDRESS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_GLOBAL_ADDRESS_COMMAND_IDENTIFIER,
                                getGlobalAddressReturnCallback,
                                "bwwu",
                                NULL),
#endif
#ifdef HAS_CB_GET_DHCP_CLIENT_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_DHCP_CLIENT_COMMAND_IDENTIFIER,
                                getDhcpClientReturnCallback,
                                "b",
                                NULL),
#endif
#ifdef HAS_CB_BECOME_COMMISSIONER_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_BECOME_COMMISSIONER_COMMAND_IDENTIFIER,
                                becomeCommissionerReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_SEND_STEERING_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SEND_STEERING_DATA_COMMAND_IDENTIFIER,
                                sendSteeringDataReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER,
                                changeNodeTypeReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_GET_GLOBAL_PREFIX_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_GLOBAL_PREFIX_COMMAND_IDENTIFIER,
                                getGlobalPrefixReturnCallback,
                                "uubuuww",
                                NULL),
#endif
#ifdef HAS_CB_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER,
                                resignGlobalAddressReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_SLAAC_SERVER_CHANGE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SLAAC_SERVER_CHANGE_COMMAND_IDENTIFIER,
                                slaacServerChangeHandlerCallback,
                                "buu",
                                NULL),
#endif
#ifdef HAS_CB_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER,
                                requestSlaacAddressReturnCallback,
                                "ubu",
                                NULL),
#endif
#ifdef HAS_CB_CUSTOM_NCP_TO_HOST_MESSAGE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_CUSTOM_NCP_TO_HOST_MESSAGE_COMMAND_IDENTIFIER,
                                customNcpToHostMessageCallback,
                                "b",
                                NULL),
#endif
#ifdef HAS_CB_NCP_TO_HOST_NO_OP_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_NCP_TO_HOST_NO_OP_COMMAND_IDENTIFIER,
                                ncpToHostNoOpCallback,
                                "b",
                                NULL),
#endif
#ifdef HAS_CB_COMMISSIONER_STATUS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_COMMISSIONER_STATUS_COMMAND_IDENTIFIER,
                                commissionerStatusHandlerCallback,
                                "vb",
                                NULL),
#endif
#ifdef HAS_CB_LEADER_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_LEADER_DATA_COMMAND_IDENTIFIER,
                                leaderDataHandlerCallback,
                                "b",
                                NULL),
#endif
#ifdef HAS_CB_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER,
                                getNetworkDataTlvReturnCallback,
                                "uuub",
                                NULL),
#endif
#ifdef HAS_CB_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER,
                                getRoutingLocatorReturnCallback,
                                "b",
                                NULL),
#endif
#ifdef HAS_CB_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER,
                                setRandomizeMacExtendedIdReturnCallback,
                                "",
                                NULL),
#endif
#ifdef HAS_CB_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER,
                                configureExternalRouteReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER,
                                allowNativeCommissionerReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER,
                                setCommissionerKeyReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_NETWORK_STATUS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_NETWORK_STATUS_COMMAND_IDENTIFIER,
                                networkStatusHandlerCallback,
                                "uuu",
                                NULL),
#endif
#ifdef HAS_CB_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER,
                                getStandaloneBootloaderInfoReturnCallback,
                                "vuuu",
                                NULL),
#endif
#ifdef HAS_CB_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER,
                                launchStandaloneBootloaderReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_GET_MFG_TOKEN_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_MFG_TOKEN_COMMAND_IDENTIFIER,
                                getMfgTokenReturnCallback,
                                "uub",
                                NULL),
#endif
#ifdef HAS_CB_SET_MFG_TOKEN_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_MFG_TOKEN_COMMAND_IDENTIFIER,
                                setMfgTokenReturnCallback,
                                "uu",
                                NULL),
#endif
#ifdef HAS_CB_HOST_STATE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_HOST_STATE_COMMAND_IDENTIFIER,
                                hostStateHandlerCallback,
                                "bbbvuusubbb",
                                NULL),
#endif
#ifdef HAS_CB_GET_CTUNE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_CTUNE_COMMAND_IDENTIFIER,
                                getCtuneReturnCallback,
                                "vu",
                                NULL),
#endif
#ifdef HAS_CB_SET_CTUNE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_CTUNE_COMMAND_IDENTIFIER,
                                setCtuneReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER,
                                setRadioHoldOffReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_GET_PTA_ENABLE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_PTA_ENABLE_COMMAND_IDENTIFIER,
                                getPtaEnableReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_SET_PTA_ENABLE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_PTA_ENABLE_COMMAND_IDENTIFIER,
                                setPtaEnableReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_GET_ANTENNA_MODE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_ANTENNA_MODE_COMMAND_IDENTIFIER,
                                getAntennaModeReturnCallback,
                                "uu",
                                NULL),
#endif
#ifdef HAS_CB_SET_ANTENNA_MODE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_ANTENNA_MODE_COMMAND_IDENTIFIER,
                                setAntennaModeReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER,
                                radioGetRandomNumbersReturnCallback,
                                "ubu",
                                NULL),
#endif
#ifdef HAS_CB_GET_PTA_OPTIONS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_PTA_OPTIONS_COMMAND_IDENTIFIER,
                                getPtaOptionsReturnCallback,
                                "w",
                                NULL),
#endif
#ifdef HAS_CB_SET_PTA_OPTIONS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_PTA_OPTIONS_COMMAND_IDENTIFIER,
                                setPtaOptionsReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_SET_JOIN_KEY_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_JOIN_KEY_COMMAND_IDENTIFIER,
                                setJoinKeyReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_SET_ND_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_ND_DATA_COMMAND_IDENTIFIER,
                                setNdDataReturnCallback,
                                "uv",
                                NULL),
#endif
#ifdef HAS_CB_SET_LOCAL_NETWORK_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_LOCAL_NETWORK_DATA_COMMAND_IDENTIFIER,
                                setLocalNetworkDataReturnCallback,
                                "uv",
                                NULL),
#endif
#ifdef HAS_CB_SET_PSKC_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SET_PSKC_COMMAND_IDENTIFIER,
                                setPskcHandlerCallback,
                                "b",
                                NULL),
#endif
#ifdef MFGLIB
#ifdef HAS_CB_MFGLIB_START_TEST_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_MFGLIB_START_TEST_COMMAND_IDENTIFIER,
                                mfglibStartTestReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_MFGLIB_RX_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_MFGLIB_RX_COMMAND_IDENTIFIER,
                                mfglibRxReturnCallback,
                                "bus",
                                NULL),
#endif
#ifdef HAS_CB_MFGLIB_END_TEST_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_MFGLIB_END_TEST_COMMAND_IDENTIFIER,
                                mfglibEndTestReturnCallback,
                                "uw",
                                NULL),
#endif
#ifdef HAS_CB_MFGLIB_START_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_MFGLIB_START_COMMAND_IDENTIFIER,
                                mfglibStartReturnCallback,
                                "uu",
                                NULL),
#endif
#ifdef HAS_CB_MFGLIB_STOP_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_MFGLIB_STOP_COMMAND_IDENTIFIER,
                                mfglibStopReturnCallback,
                                "uu",
                                NULL),
#endif
#ifdef HAS_CB_MFGLIB_SEND_PACKET_EVENT_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_MFGLIB_SEND_PACKET_EVENT_COMMAND_IDENTIFIER,
                                mfglibSendPacketEventHandlerCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_MFGLIB_SET_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_MFGLIB_SET_COMMAND_IDENTIFIER,
                                mfglibSetReturnCallback,
                                "uu",
                                NULL),
#endif
#ifdef HAS_CB_MFGLIB_GET_CHANNEL_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_MFGLIB_GET_CHANNEL_COMMAND_IDENTIFIER,
                                mfglibGetChannelReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_MFGLIB_GET_POWER_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_MFGLIB_GET_POWER_COMMAND_IDENTIFIER,
                                mfglibGetPowerReturnCallback,
                                "s",
                                NULL),
#endif
#ifdef HAS_CB_MFGLIB_GET_POWER_MODE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_MFGLIB_GET_POWER_MODE_COMMAND_IDENTIFIER,
                                mfglibGetPowerModeReturnCallback,
                                "v",
                                NULL),
#endif
#ifdef HAS_CB_MFGLIB_GET_SYN_OFFSET_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_MFGLIB_GET_SYN_OFFSET_COMMAND_IDENTIFIER,
                                mfglibGetSynOffsetReturnCallback,
                                "s",
                                NULL),
#endif
#ifdef HAS_CB_MFGLIB_GET_OPTIONS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_MFGLIB_GET_OPTIONS_COMMAND_IDENTIFIER,
                                mfglibGetOptionsReturnCallback,
                                "u",
                                NULL),
#endif
#endif /* MFGLIB */
#ifdef EMBER_TEST
#ifdef HAS_CB_CONFIG_UART_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_CONFIG_UART_COMMAND_IDENTIFIER,
                                configUartReturnCallback,
                                "",
                                NULL),
#endif
#ifdef HAS_CB_RESET_NCP_ASH_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_RESET_NCP_ASH_COMMAND_IDENTIFIER,
                                resetNcpAshReturnCallback,
                                "",
                                NULL),
#endif
#ifdef HAS_CB_START_UART_STORM_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_START_UART_STORM_COMMAND_IDENTIFIER,
                                startUartStormReturnCallback,
                                "",
                                NULL),
#endif
#ifdef HAS_CB_STOP_UART_STORM_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_STOP_UART_STORM_COMMAND_IDENTIFIER,
                                stopUartStormReturnCallback,
                                "",
                                NULL),
#endif
#ifdef HAS_CB_SEND_DONE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_SEND_DONE_COMMAND_IDENTIFIER,
                                sendDoneReturnCallback,
                                "",
                                NULL),
#endif
#endif /* EMBER_TEST */
#if (defined (EMBER_TEST) || defined(QA_THREAD_TEST))
#ifdef HAS_CB_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER,
                                getNetworkKeyInfoReturnCallback,
                                "uwu",
                                NULL),
#endif
#ifdef HAS_CB_GET_NODE_STATUS_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_GET_NODE_STATUS_COMMAND_IDENTIFIER,
                                getNodeStatusReturnCallback,
                                "uuvuvbw",
                                NULL),
#endif
#ifdef HAS_CB_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER,
                                addAddressDataReturnCallback,
                                "v",
                                NULL),
#endif
#ifdef HAS_CB_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER,
                                clearAddressCacheReturnCallback,
                                "",
                                NULL),
#endif
#ifdef HAS_CB_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER,
                                lookupAddressDataReturnCallback,
                                "v",
                                NULL),
#endif
#ifdef HAS_CB_UART_SPEED_TEST_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_UART_SPEED_TEST_COMMAND_IDENTIFIER,
                                uartSpeedTestReturnCallback,
                                "www",
                                NULL),
#endif
#ifdef HAS_CB_NCP_UDP_STORM_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_NCP_UDP_STORM_COMMAND_IDENTIFIER,
                                ncpUdpStormReturnCallback,
                                "u",
                                NULL),
#endif
#ifdef HAS_CB_NCP_UDP_STORM_COMPLETE_COMMAND_IDENTIFIER
  emberBinaryCommandEntryAction(CB_NCP_UDP_STORM_COMPLETE_COMMAND_IDENTIFIER,
                                ncpUdpStormCompleteHandlerCallback,
                                "",
                                NULL),
#endif
#endif /* (defined (EMBER_TEST) || defined(QA_THREAD_TEST)) */
#ifdef EMBER_APPLICATION_USES_SOFTWARE_FLOW_CONTROL
#endif /* EMBER_APPLICATION_USES_SOFTWARE_FLOW_CONTROL */
  { { NULL } }  // terminator
};
