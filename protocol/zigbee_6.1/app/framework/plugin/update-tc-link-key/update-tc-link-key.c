// Copyright 2015 Silicon Laboratories, Inc.

#include "app/framework/include/af.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"

#include "update-tc-link-key.h"

// TODO: add plugin option for joining node only functionality?

#define SERVER_MASK_STACK_COMPLIANCE_REVISION_MASK 0xFE00
//Bits 9 to 15 are set to 21. Everything else is 0.
#define SERVER_MASK_STACK_COMPLIANCE_REVISION_21 0x2A00

#define trustCenterAddressIsAllZeros(securityState, allZeros) \
  (!MEMCOMPARE((securityState).trustCenterLongAddress, (allZeros), EUI64_SIZE))

#ifdef EZSP_HOST
  #define lookupEui64ByNodeId ezspLookupEui64ByNodeId
  #define requestLinkKey ezspRequestLinkKey
#else
  #define lookupEui64ByNodeId emberLookupEui64ByNodeId
  #define requestLinkKey emberRequestLinkKey
#endif

#define STATE_NONE     (0x00)
#define STATE_REQUEST  (0x01)
#define STATE_RESPONSE (0x02)
static uint8_t state = STATE_NONE;

// -----------------------------------------------------------------------------
// Public API

EmberStatus emberAfPluginUpdateTcLinkKeyStart(void)
{
  EmberStatus status = EMBER_INVALID_CALL;
  EmberCurrentSecurityState currentSecurityState;
  const EmberEUI64 allZeroEui64 = { 0, 0, 0, 0, 0, 0, 0, 0, };

  if (state == STATE_NONE) {
    status = EMBER_NOT_JOINED;
    if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
      status = EMBER_SECURITY_CONFIGURATION_INVALID;
      emberGetCurrentSecurityState(&currentSecurityState);
      if (!trustCenterAddressIsAllZeros(currentSecurityState, allZeroEui64)) {
        status = EMBER_INVALID_CALL;
        if (emberAfGetNodeId() != EMBER_TRUST_CENTER_NODE_ID) {
          status = emberNodeDescriptorRequest(EMBER_TRUST_CENTER_NODE_ID,
                                              EMBER_AF_DEFAULT_APS_OPTIONS);
          if (status == EMBER_SUCCESS) {
            state = STATE_REQUEST;
          }
        }
      }
    }
  }

  return status;
}

bool emberAfPluginUpdateTcLinkKeyStop(void)
{
  uint8_t oldState = state;

  state = STATE_NONE;

  return (oldState != STATE_NONE);
}

bool emberAfPluginUpdateTcLinkKeyZdoMessageReceivedCallback(EmberNodeId sender,
                                                            EmberApsFrame* apsFrame,
                                                            uint8_t* message,
                                                            uint16_t length)
{
  EmberStatus status;
  EmberEUI64 trustCenterEui64;

  if (state == STATE_REQUEST) {
    if (apsFrame->clusterId == NODE_DESCRIPTOR_RESPONSE) {
      uint16_t serverMask = message[12] | (message[13] << 8);
      if (((serverMask & SERVER_MASK_STACK_COMPLIANCE_REVISION_MASK)
           >= SERVER_MASK_STACK_COMPLIANCE_REVISION_21)) {
        if (lookupEui64ByNodeId(EMBER_TRUST_CENTER_NODE_ID, trustCenterEui64)
            == EMBER_SUCCESS) {
          status = requestLinkKey(trustCenterEui64); // trust center link key
          state = (status == EMBER_SUCCESS ? STATE_RESPONSE : STATE_NONE);
          emberAfCorePrintln("%p: %p: 0x%X",
                             EMBER_AF_PLUGIN_UPDATE_TC_LINK_KEY_PLUGIN_NAME,
                             "Requesting link key from R21 trust center",
                             status);
        }
      } else {
        // If the trust center is pre-R21, then we don't update the link key.
        emberAfPluginUpdateTcLinkKeyStatusCallback(EMBER_TRUST_CENTER_IS_PRE_R21);
        state = STATE_NONE;
      }
    }
  }

  return false;
}

void emberAfPluginUpdateTcLinkKeyZigbeeKeyEstablishmentCallback(EmberEUI64 partner,
                                                                EmberKeyStatus status)
{
  if (state == STATE_RESPONSE) {
    emberAfCorePrintln("%p: %p: 0x%X",
                       EMBER_AF_PLUGIN_UPDATE_TC_LINK_KEY_PLUGIN_NAME,
                       "New key established",
                       status);
    emberAfCorePrint("Partner: ");
    emberAfCorePrintBuffer(partner, EUI64_SIZE, true); // withSpace?
    emberAfCorePrintln("");

    if (emberAfPluginUpdateTcLinkKeyStatusCallback(status)) {
      state = STATE_NONE;
    }
  }
}
