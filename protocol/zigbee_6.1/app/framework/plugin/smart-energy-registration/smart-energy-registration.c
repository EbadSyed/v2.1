// *****************************************************************************
// * smart-energy-registration.c
// *
// * This defines the state machine and procedure for a newly joined device to
// * perform the necessary steps to register in a network.
// *
// * Copyright 2014 Silicon Laboratories, Inc.                              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"                     //emberAfIsFullSmartEnergySecurityPresent
#include "app/util/zigbee-framework/zigbee-device-common.h" //emberBindRequest
#ifdef EZSP_HOST                                            //emberIeeeAddressRequest
  #include "app/util/zigbee-framework/zigbee-device-host.h"
#else //EZSP_HOST
  #include "stack/include/ember.h"
#endif //EZSP_HOST

#include "smart-energy-registration.h"
#include "app/framework/plugin/test-harness/test-harness.h"

#include "app/framework/plugin/esi-management/esi-management.h"

extern EmberEventControl emberAfPluginSmartEnergyRegistrationTickNetworkEventControls[1];
void emberAfPluginSmartEnergyRegistrationTickNetworkEventHandler(void);

typedef enum {
  STATE_INITIAL,
  STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER,
  STATE_PERFORM_KEY_ESTABLISHMENT,
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  STATE_DISCOVER_ENERGY_SERVICE_INTERFACES,
  STATE_DISCOVER_IEEE_ADDRESSES,
  STATE_PERFORM_PARTNER_LINK_KEY_EXCHANGE,
  STATE_PERFORM_BINDING,
  STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE,
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  STATE_REGISTRATION_COMPLETE,
  STATE_REGISTRATION_FAILED,
} EmAfPluginSmartEnergyRegistrationState;

#define UNDEFINED_ENDPOINT 0xFF

#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED

#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD
uint32_t emAfPluginSmartEnergyRegistrationDiscoveryPeriod =
  EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD;
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD

typedef struct {
  bool valid;
  EmberNodeId nodeId;
  uint32_t  time;
  uint8_t   timeStatus;
  uint32_t  lastSetTime;
  uint32_t  validUntilTime;
} EmAfPluginSmartEnergyRegistrationTimeSource;

#ifndef SE_PROFILE_ID
  #define SE_PROFILE_ID 0x0109
#endif

#define UNDEFINED_CLUSTER_ID 0xFFFF
static EmberAfClusterId clusterList[] = {
#ifdef ZCL_USING_PRICE_CLUSTER_CLIENT
  ZCL_PRICE_CLUSTER_ID,
#endif //ZCL_USING_PRICE_CLUSTER_CLIENT

#ifdef ZCL_USING_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_CLIENT
  ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID,
#endif //ZCL_USING_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_CLIENT

#ifdef ZCL_USING_SIMPLE_METERING_CLUSTER_CLIENT
  ZCL_SIMPLE_METERING_CLUSTER_ID,
#endif //ZCL_USING_SIMPLE_METERING_CLUSTER_CLIENT

#ifdef ZCL_USING_MESSAGING_CLUSTER_CLIENT
  ZCL_MESSAGING_CLUSTER_ID,
#endif //ZCL_USING_MESSAGING_CLUSTER_CLIENT

#ifdef ZCL_USING_TUNNELING_CLUSTER_CLIENT
  ZCL_TUNNELING_CLUSTER_ID,
#endif //ZCL_USING_TUNNELING_CLUSTER_CLIENT

#ifdef ZCL_USING_PREPAYMENT_CLUSTER_CLIENT
  ZCL_PREPAYMENT_CLUSTER_ID,
#endif //ZCL_USING_PREPAYMENT_CLUSTER_CLIENT

#ifdef ZCL_USING_CALENDAR_CLUSTER_CLIENT
  ZCL_CALENDAR_CLUSTER_ID,
#endif //ZCL_USING_CALENDAR_CLUSTER_CLIENT

#ifdef ZCL_USING_DEVICE_MANAGEMENT_CLUSTER_CLIENT
  ZCL_DEVICE_MANAGEMENT_CLUSTER_ID,
#endif //ZCL_USING_DEVICE_MANAGEMENT_CLUSTER_CLIENT

#ifdef ZCL_USING_EVENTS_CLUSTER_CLIENT
  ZCL_EVENTS_CLUSTER_ID,
#endif //ZCL_USING_EVENTS_CLUSTER_CLIENT

  UNDEFINED_CLUSTER_ID,
};

#define NEXT_STATE STATE_DISCOVER_ENERGY_SERVICE_INTERFACES

#else //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED

  #define NEXT_STATE STATE_REGISTRATION_COMPLETE

#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED

static bool checkErrorCountAndSetEventControl(uint32_t delayMs, bool errorFlag);
static bool resumeAfterDelay(EmberStatus status, uint32_t delayMs);

#define resumeAfterFixedDelay(status) \
  resumeAfterDelay((status),          \
                   EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_RETRY)

#define transition(next) transitionAfterDelay((next), EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_TRANSITION)
static bool transitionAfterDelay(EmAfPluginSmartEnergyRegistrationState next,
                                 uint32_t delay);

static void performKeyEstablishment(void);

#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
static void performPartnerLinkKeyExchange(void);
static void partnerLinkKeyExchangeCallback(bool success);
static void performBinding(void);
static void determineAuthoritativeTimeSource(void);
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED

static void stopRegistration(bool success);

static void performDiscovery(void);
static void discoveryCallback(const EmberAfServiceDiscoveryResult *result);

typedef struct {
  EmAfPluginSmartEnergyRegistrationState state;
  uint8_t errors;
  uint8_t trustCenterKeyEstablishmentEndpoint;
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  EmberAfPluginEsiManagementEsiEntry *esiEntry;
  uint8_t endpointIndex; // performBinding
  uint8_t clusterIndex;  // performBinding
  bool resuming;    // determineAuthoritativeTimeSource
  EmAfPluginSmartEnergyRegistrationTimeSource source;
#endif
} State;
static State states[EMBER_SUPPORTED_NETWORKS];

//------------------------------------------------------------------------------

EmberStatus emberAfRegistrationStartCallback(void)
{
  State *state = &states[emberGetCurrentNetwork()];

  if (!emberAfIsCurrentSecurityProfileSmartEnergy()) {
    return EMBER_INVALID_CALL;
  }

  if (state->state == STATE_REGISTRATION_COMPLETE) {
    // If we got called again after registration has already completed,
    // this means that it was due to a rejoin.  The trust center keepalive
    // may have initiated this rejoin due to a TC failure.
    emberAfTrustCenterKeepaliveUpdateCallback(true);
    return EMBER_SUCCESS;
  }

  if (state->state != STATE_INITIAL) {
    return EMBER_INVALID_CALL;
  }

  if (!emAfTestHarnessAllowRegistration) {
    return EMBER_SECURITY_CONFIGURATION_INVALID;
  }

  // Registration is unnecessary for the trust center.  For other nodes, wait
  // for the network broadcast traffic to die down and neighbor information to
  // be populated before continuing.
  if (emberAfGetNodeId() == EMBER_TRUST_CENTER_NODE_ID) {
    transition(STATE_REGISTRATION_COMPLETE);
  } else {
    transitionAfterDelay(STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER,
                         EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_INITIAL);
  }
  emberAfAddToCurrentAppTasks(EMBER_AF_WAITING_FOR_REGISTRATION);
  return EMBER_SUCCESS;
}

void emberAfRegistrationAbortCallback(void)
{
  if (emberAfIsCurrentSecurityProfileSmartEnergy()) {
    // We need registration to stop immediately because it may be started up
    // again in the same call chain.
    State *state = &states[emberGetCurrentNetwork()];
    state->state = STATE_REGISTRATION_FAILED;
    emberAfPluginSmartEnergyRegistrationTickNetworkEventHandler();
  }
}

void emberAfPluginSmartEnergyRegistrationTickNetworkEventHandler(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  switch (state->state) {
    case STATE_INITIAL:
      emberAfRegistrationStartCallback();
      break;
    case STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER:
      performDiscovery();
      break;
    case STATE_PERFORM_KEY_ESTABLISHMENT:
      performKeyEstablishment();
      break;
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
    case STATE_DISCOVER_ENERGY_SERVICE_INTERFACES:
      performDiscovery();
      break;
    case STATE_DISCOVER_IEEE_ADDRESSES:
      performDiscovery();
      break;
    case STATE_PERFORM_PARTNER_LINK_KEY_EXCHANGE:
      performPartnerLinkKeyExchange();
      break;
    case STATE_PERFORM_BINDING:
      performBinding();
      break;
    case STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE:
      determineAuthoritativeTimeSource();
      break;
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
    case STATE_REGISTRATION_COMPLETE:
    // FALLTHROUGH
    case STATE_REGISTRATION_FAILED:
      stopRegistration(state->state == STATE_REGISTRATION_COMPLETE);
      break;
    default:
      emberAfRegistrationPrintln("ERR: Invalid state (0x%x)", state->state);
      emberAfRegistrationAbortCallback();
      break;
  }
}

uint8_t emAfPluginSmartEnergyRegistrationTrustCenterKeyEstablishmentEndpoint(void)
{
  // When we start, the key establishment endpoint will be zero.  This is okay
  // internally in this plugin, but we really want to use a better "undefined"
  // value for external modules.
  State *state = &states[emberGetCurrentNetwork()];
  if (state->trustCenterKeyEstablishmentEndpoint == 0x00) {
    state->trustCenterKeyEstablishmentEndpoint = UNDEFINED_ENDPOINT;
  }
  return state->trustCenterKeyEstablishmentEndpoint;
}

static bool checkErrorCountAndSetEventControl(uint32_t delayMs, bool errorFlag)
{
  State *state = &states[emberGetCurrentNetwork()];

  // Increment the error count if we're delaying due to an error; otherwise,
  // reset the error count so that failures in any particular state don't affect
  // subsequent states.
  if (errorFlag) {
    state->errors++;
    emberAfRegistrationPrintln("Registration error count %d of %d",
                               state->errors,
                               EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ERROR_LIMIT);
  } else {
    state->errors = 0;
  }
  if (state->errors >= EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ERROR_LIMIT) {
    emberAfRegistrationFlush();
    emberAfRegistrationPrintln("ERR: Aborting registration"
                               " because error limit reached (%d)",
                               EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ERROR_LIMIT);
    emberAfRegistrationAbortCallback();
    return false;
  }

  emberAfNetworkEventControlSetDelayMS(emberAfPluginSmartEnergyRegistrationTickNetworkEventControls,
                                       delayMs);
  return true;
}

static bool resumeAfterDelay(EmberStatus status, uint32_t delayMs)
{
  bool errorFlag = (status != EMBER_SUCCESS);
  return checkErrorCountAndSetEventControl((errorFlag
                                            ? delayMs
                                            : EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_RESUME),
                                           errorFlag);
}

static bool transitionAfterDelay(EmAfPluginSmartEnergyRegistrationState next,
                                 uint32_t delay)
{
  State *state = &states[emberGetCurrentNetwork()];
  state->state = next;
  return checkErrorCountAndSetEventControl(delay, false);
}

static void performKeyEstablishment(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  EmberStatus status;
  emberAfRegistrationPrintln("Performing key establishment");

  // Transient failures may prevent us from performing key establishment.  If
  // so, we will try again later.
  status = emberAfInitiateKeyEstablishment(EMBER_TRUST_CENTER_NODE_ID,
                                           state->trustCenterKeyEstablishmentEndpoint);
  if (status != EMBER_SUCCESS) {
    emberAfRegistrationPrintln("ERR: Failed to start key establishment (0x%x)",
                               status);
    resumeAfterFixedDelay(status);
  }
}

bool emberAfKeyEstablishmentCallback(EmberAfKeyEstablishmentNotifyMessage status,
                                     bool amInitiator,
                                     EmberNodeId partnerShortId,
                                     uint8_t delayInSeconds)
{
  State *state = &states[emberGetCurrentNetwork()];

  // The notification only matters if we are performing key establishment.
  if (state->state == STATE_PERFORM_KEY_ESTABLISHMENT) {
    if (status == LINK_KEY_ESTABLISHED) {
      transition(NEXT_STATE);
    } else if (status >= APP_NOTIFY_ERROR_CODE_START) {
      uint32_t delayMs = delayInSeconds * MILLISECOND_TICKS_PER_SECOND;
      emberAfRegistrationPrintln("ERR: Key establishment failed (0x%x)", status);
      resumeAfterDelay(EMBER_ERR_FATAL,
                       delayMs);
    }
  }

  // Always allow key establishment to continue.
  return true;
}

#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
static void performPartnerLinkKeyExchange(void)
{
  State *state = &states[emberGetCurrentNetwork()];

  for (;
       state->esiEntry != NULL;
       state->esiEntry = emberAfPluginEsiManagementGetNextEntry(state->esiEntry, 0)) {
    assert(state->esiEntry->nodeId != EMBER_NULL_NODE_ID);
    if (state->esiEntry->nodeId != EMBER_TRUST_CENTER_NODE_ID) {
      EmberStatus status;
      emberAfRegistrationPrintln("Performing partner link key exchange"
                                 " with node 0x%2x endpoint 0x%x",
                                 state->esiEntry->nodeId,
                                 state->esiEntry->endpoint);
      status = emberAfInitiatePartnerLinkKeyExchange(state->esiEntry->nodeId,
                                                     state->esiEntry->endpoint,
                                                     partnerLinkKeyExchangeCallback);
      if (status != EMBER_SUCCESS) {
        emberAfRegistrationPrintln("ERR: Failed to initiate partner link key request"
                                   " with node 0x%2x endpoint 0x%x (0x%x)",
                                   state->esiEntry->nodeId,
                                   state->esiEntry->endpoint,
                                   status);
        resumeAfterFixedDelay(status);
      }
      return;
    }
  }

  // Point to the first active entry with age 0 (if any).
  state->esiEntry = emberAfPluginEsiManagementGetNextEntry(NULL, 0);
  transition(STATE_PERFORM_BINDING);
}

static void partnerLinkKeyExchangeCallback(bool success)
{
  State *state = &states[emberGetCurrentNetwork()];
  if (state->state == STATE_PERFORM_PARTNER_LINK_KEY_EXCHANGE) {
    assert(state->esiEntry != NULL && state->esiEntry->nodeId != EMBER_NULL_NODE_ID);
    if (success) {
      emberAfRegistrationPrintln("Performed partner link key exchange"
                                 " with node 0x%2x endpoint 0x%x",
                                 state->esiEntry->nodeId,
                                 state->esiEntry->endpoint);
      state->esiEntry = emberAfPluginEsiManagementGetNextEntry(state->esiEntry, 0);
    } else {
      emberAfRegistrationPrintln("ERR: Failed to perform partner link key exchange"
                                 " with node 0x%2x endpoint 0x%x",
                                 state->esiEntry->nodeId,
                                 state->esiEntry->endpoint);
    }
    resumeAfterFixedDelay(success ? EMBER_SUCCESS : EMBER_ERR_FATAL);
  }
}

static void performBinding(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  EmberEUI64 eui64;
  uint8_t networkIndex = emberGetCurrentNetwork();

  emberAfGetEui64(eui64);

  // The spec requires that we bind in order to inform the ESI that we want
  // unsolicited updates, but the ESI is not required to use binding and may
  // send a negative response.  In addition, there is no recourse if a binding
  // is required but the ESI refuses.  So, send the bindings, but don't pay
  // attention to whether it works or not.
  for (;
       state->esiEntry != NULL;
       state->esiEntry = emberAfPluginEsiManagementGetNextEntry(state->esiEntry, 0)) {
    assert(state->esiEntry->nodeId != EMBER_NULL_NODE_ID);
    for (;
         state->endpointIndex < emberAfEndpointCount();
         state->endpointIndex++) {
      uint8_t endpoint = emberAfEndpointFromIndex(state->endpointIndex);
      if (networkIndex
          != emberAfNetworkIndexFromEndpointIndex(state->endpointIndex)) {
        continue;
      }
      for (;
           clusterList[state->clusterIndex] != UNDEFINED_CLUSTER_ID;
           state->clusterIndex++) {
        EmberAfClusterId clusterId = clusterList[state->clusterIndex];
        if (emberAfContainsClient(endpoint, clusterId)) {
          EmberStatus status;
          emberAfRegistrationPrintln("Performing binding"
                                     " to node 0x%2x endpoint 0x%x"
                                     " from endpoint 0x%x"
                                     " for cluster 0x%2x",
                                     state->esiEntry->nodeId,
                                     state->esiEntry->endpoint,
                                     endpoint,
                                     clusterId);
          status = emberBindRequest(state->esiEntry->nodeId,
                                    state->esiEntry->eui64,
                                    state->esiEntry->endpoint,
                                    clusterId,
                                    UNICAST_BINDING,
                                    eui64,
                                    0, // multicast group identifier - ignored
                                    endpoint,
                                    EMBER_AF_DEFAULT_APS_OPTIONS);
          if (status == EMBER_SUCCESS) {
            state->clusterIndex++;
          } else {
            emberAfRegistrationPrintln("ERR: Failed to send bind request"
                                       " to node 0x%2x endpoint 0x%x"
                                       " from endpoint 0x%x"
                                       " for cluster 0x%2x (0x%x)",
                                       state->esiEntry->nodeId,
                                       state->esiEntry->endpoint,
                                       endpoint,
                                       clusterId,
                                       status);
          }

          // We may hit the error limit if we delay here due to an error.  If
          // so, we have to clear our internal static indices; otherwise, when
          // registration is restarted, we won't pick up from the beginning.
          if (!resumeAfterFixedDelay(status)) {
            state->endpointIndex = state->clusterIndex = 0;
          }
          return;
        }
      }
      state->clusterIndex = 0;
    }
    state->endpointIndex = 0;
  }

  // Point to the first active entry with age 0 (if any).
  state->esiEntry = emberAfPluginEsiManagementGetNextEntry(NULL, 0);
  transition(STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE);
}

static void determineAuthoritativeTimeSource(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  uint8_t attributeIds[] = {
    LOW_BYTE(ZCL_TIME_ATTRIBUTE_ID), HIGH_BYTE(ZCL_TIME_ATTRIBUTE_ID),
    LOW_BYTE(ZCL_TIME_STATUS_ATTRIBUTE_ID), HIGH_BYTE(ZCL_TIME_STATUS_ATTRIBUTE_ID),
    LOW_BYTE(ZCL_LAST_SET_TIME_ATTRIBUTE_ID), HIGH_BYTE(ZCL_LAST_SET_TIME_ATTRIBUTE_ID),
    LOW_BYTE(ZCL_VALID_UNTIL_TIME_ATTRIBUTE_ID), HIGH_BYTE(ZCL_VALID_UNTIL_TIME_ATTRIBUTE_ID),
  };
  uint8_t sourceEndpoint = emberAfPrimaryEndpointForCurrentNetworkIndex();
  assert(sourceEndpoint != 0xFF);

  if (!state->resuming) {
    emberAfRegistrationPrintln("Determining authoritative time source");
    state->source.valid = false;
  }

  while (state->esiEntry != NULL) {
    EmberStatus status;
    assert(state->esiEntry->nodeId != EMBER_NULL_NODE_ID);
    emberAfRegistrationPrintln("Requesting time attributes"
                               " from node 0x%2x endpoint 0x%x",
                               state->esiEntry->nodeId,
                               state->esiEntry->endpoint);

    emberAfFillCommandGlobalClientToServerReadAttributes(ZCL_TIME_CLUSTER_ID,
                                                         attributeIds,
                                                         sizeof(attributeIds));
    emberAfSetCommandEndpoints(sourceEndpoint, state->esiEntry->endpoint);
    status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                                       state->esiEntry->nodeId);

    if (status == EMBER_SUCCESS) {
      state->esiEntry = emberAfPluginEsiManagementGetNextEntry(state->esiEntry, 0);
      if (state->esiEntry == NULL) {
        state->resuming = transitionAfterDelay(STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE,
                                               EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_BRIEF);
        return;
      }
    } else {
      emberAfRegistrationPrintln("ERR: Failed to request time attributes"
                                 " from node 0x%2x endpoint 0x%x (0x%x)",
                                 state->esiEntry->nodeId,
                                 state->esiEntry->endpoint,
                                 status);
    }
    state->resuming = resumeAfterFixedDelay(status);
    return;
  }

  if (!state->source.valid) {
    emberAfRegistrationPrintln("ERR: Failed to determine"
                               " authoritative time source");
    state->resuming = resumeAfterFixedDelay(EMBER_ERR_FATAL);
  } else {
    emberAfRegistrationPrintln("Determined authoritative time source,"
                               " node 0x%2x",
                               state->source.nodeId);
    // Point to the first active entry with age 0 (if any).
    state->esiEntry = emberAfPluginEsiManagementGetNextEntry(NULL, 0);
    transition(STATE_REGISTRATION_COMPLETE);
  }
}

void emAfPluginSmartEnergyRegistrationReadAttributesResponseCallback(uint8_t *buffer,
                                                                     uint16_t bufLen)
{
  State *state = &states[emberGetCurrentNetwork()];
  uint32_t time           = 0x00000000UL;
  uint8_t  timeStatus     = 0x00;
  uint32_t lastSetTime    = 0x00000000UL;
  uint32_t validUntilTime = 0xFFFFFFFFUL;
  uint16_t bufIndex = 0;
  EmberAfClusterCommand *cmd = emberAfCurrentCommand();

  if (state->state != STATE_DETERMINE_AUTHORITATIVE_TIME_SOURCE) {
    return;
  }

  // Each record in the response has a two-byte attribute id and a one-byte
  // status.  If the status is SUCCESS, there will also be a one-byte type and
  // variable-length data.
  while (bufIndex + 3 <= bufLen) {
    EmberAfAttributeId attributeId;
    EmberAfStatus status;
    attributeId = (EmberAfAttributeId)emberAfGetInt16u(buffer, bufIndex, bufLen);
    bufIndex += 2;
    status = (EmberAfStatus)emberAfGetInt8u(buffer, bufIndex, bufLen);
    bufIndex++;

    if (status == EMBER_ZCL_STATUS_SUCCESS) {
      EmberAfAttributeType type;
      type = (EmberAfAttributeType)emberAfGetInt8u(buffer, bufIndex, bufLen);
      bufIndex++;

      switch (attributeId) {
        case ZCL_TIME_ATTRIBUTE_ID:
          time = emberAfGetInt32u(buffer, bufIndex, bufLen);
          break;
        case ZCL_TIME_STATUS_ATTRIBUTE_ID:
          timeStatus = emberAfGetInt8u(buffer, bufIndex, bufLen);
          break;
        case ZCL_LAST_SET_TIME_ATTRIBUTE_ID:
          lastSetTime = emberAfGetInt32u(buffer, bufIndex, bufLen);
          break;
        case ZCL_VALID_UNTIL_TIME_ATTRIBUTE_ID:
          validUntilTime = emberAfGetInt32u(buffer, bufIndex, bufLen);
          break;
      }

      bufIndex += (emberAfIsThisDataTypeAStringType(type)
                   ? emberAfStringLength(buffer + bufIndex) + 1
                   : emberAfGetDataSize(type));
    }
  }

  emberAfRegistrationPrintln("Received time attributes from node 0x%2x",
                             cmd->source);
  emberAfRegistrationPrintln("time 0x%4x", time);
  emberAfRegistrationPrintln("time status 0x%x", timeStatus);

  // The process for determining the most authoritative time source is outlined
  // in section 5.7.2 of 105638r12. Devices shall synchronize to a Time server
  // with the highest rank according to the following rules, listed in order of
  // precedence:
  //   1. A server with both the Master and the Superseding bits set shall be
  //      chosen over a server with only the Master bit set.
  //   2. A server with the Master bit set shall be chosen over a server without
  //      the bit set.
  //   3. The server with the lower short address shall be chosen (note that
  //      this means a coordinator with the Superseding and Master bit set will
  //      always be chosen as the network time server).
  //   4. A Time server with neither the Master nor Synchronized bits set should
  //      not be chosen as the network time server.
  //
  // Timestatus attribute:
  //  bit 0 - Master
  //  bit 1 - Synchronized
  //  bit 2 - MasterZoneDst
  //  bit 3 - Superseding (not sure this is actually bit 3, to be checked).

  // This logic could be reduced if needed. However, this implementation
  // is way more readable.

  // We received an invalid time from our actual time server. We set it to
  // invalid and wait for another server.
  if (time == 0xFFFFFFFFUL
      && state->source.valid
      && state->source.nodeId == cmd->source) {
    state->source.valid = false;
    // TODO: Should we kick off the registration process again here?
  } else if (time != 0xFFFFFFFFUL) {
    if (// step 1.
      (state->source.valid
       && READBIT(timeStatus, 0)
       && READBIT(timeStatus, 3)
       && READBIT(state->source.timeStatus, 0)
       && !READBIT(state->source.timeStatus, 3))
      // step 2.
      || (state->source.valid
          && READBIT(timeStatus, 3) == READBIT(state->source.timeStatus, 3)
          && READBIT(timeStatus, 0)
          && !READBIT(state->source.timeStatus, 0))
      // step 3.
      || (state->source.valid
          && READBIT(timeStatus, 3) == READBIT(state->source.timeStatus, 3)
          && READBIT(timeStatus, 0) == READBIT(state->source.timeStatus, 0)
          && (cmd->source < state->source.nodeId))
      // step 4 (at least one among the superseding, master and synchronized
      // bits should be set).
      || (!state->source.valid
          && (READBIT(timeStatus, 0) || READBIT(timeStatus, 1)))) {
      state->source.valid          = true;
      state->source.nodeId         = cmd->source;
      state->source.time           = time;
      state->source.timeStatus     = timeStatus;
      state->source.lastSetTime    = lastSetTime;
      state->source.validUntilTime = validUntilTime;
      emberAfSetTime(time);

      emberAfRegistrationPrintln("Node 0x%2x chosen as"
                                 " authoritative time source",
                                 cmd->source);
    }
  }
}
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED

static void stopRegistration(bool success)
{
  State *state = &states[emberGetCurrentNetwork()];

  emberAfRemoveFromCurrentAppTasks(EMBER_AF_WAITING_FOR_REGISTRATION);
  emberAfRegistrationCallback(success);

  emberAfTrustCenterKeepaliveUpdateCallback(success);

#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD
  if (success) {
    transitionAfterDelay(STATE_DISCOVER_ENERGY_SERVICE_INTERFACES,
                         emAfPluginSmartEnergyRegistrationDiscoveryPeriod);
    return;
  }
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_DELAY_PERIOD
  state->state = STATE_INITIAL;
}

static void performDiscovery(void)
{
  State *state = &states[emberGetCurrentNetwork()];
  EmberStatus status;
  EmberNodeId target;
  EmberAfClusterId clusterId;

  // When performing key establishment, search the trust center for the Key
  // Establishment cluster.  When searching for ESIs, broadcast for the DRLC
  // server cluster, which only ESIs should have.
  if (state->state == STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER) {
    emberAfRegistrationPrintln("Discovering Key Establishment cluster");
    state->trustCenterKeyEstablishmentEndpoint = UNDEFINED_ENDPOINT;
    target = EMBER_TRUST_CENTER_NODE_ID;
    clusterId = ZCL_KEY_ESTABLISHMENT_CLUSTER_ID;
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  } else if (state->state == STATE_DISCOVER_ENERGY_SERVICE_INTERFACES) {
    emberAfRegistrationPrintln("Discovering Energy Service Interfaces");
    // Aging the entries in the ESI table before starting the discovery process.
    emberAfPluginEsiManagementAgeAllEntries();
    target = EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS;
    clusterId = ZCL_DEMAND_RESPONSE_LOAD_CONTROL_CLUSTER_ID;
  } else if (state->state == STATE_DISCOVER_IEEE_ADDRESSES) {
    assert(state->esiEntry != NULL && state->esiEntry->nodeId != EMBER_NULL_NODE_ID);
    emberAfRegistrationPrintln("Discovering IEEE address"
                               " for node 0x%2x",
                               state->esiEntry->nodeId);
    status = emberAfFindIeeeAddress(state->esiEntry->nodeId, discoveryCallback);
    goto kickout;
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  } else {
    emberAfRegistrationPrintln("ERR: Invalid state for discovery (0x%x)",
                               state->state);
    emberAfRegistrationAbortCallback();
    return;
  }

  emberAfRegistrationPrintln("Calling the FindDevices");
  // Transient failures may prevent us from performing discovery.  If so, we
  // will try again later.
  status = emberAfFindDevicesByProfileAndCluster(target,
                                                 SE_PROFILE_ID,
                                                 clusterId,
                                                 EMBER_AF_SERVER_CLUSTER_DISCOVERY,
                                                 discoveryCallback);
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  kickout:
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  if (status != EMBER_SUCCESS) {
    emberAfRegistrationPrintln("ERR: Failed to start discovery (0x%x)", status);
    resumeAfterFixedDelay(status);
  }
}

static void discoveryCallback(const EmberAfServiceDiscoveryResult *result)
{
  uint8_t networkIndex = emberGetCurrentNetwork();
  State *state = &states[networkIndex];
  if (emberAfHaveDiscoveryResponseStatus(result->status)) {
    if (result->zdoRequestClusterId == MATCH_DESCRIPTORS_REQUEST) {
      const EmberAfEndpointList* endpointList = (const EmberAfEndpointList *)result->responseData;
      uint8_t i;

      // Need to ignore any ESI or KE discovery results from ourselves
      if (result->matchAddress == emberAfGetNodeId()) {
        emberAfRegistrationPrintln("Ignoring discovery result from loopback");
        return;
      }

      for (i = 0; i < endpointList->count; i++) {
        if (state->state == STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER) {
          // Key Establishment is global to the device so we can ignore anything
          // beyond the first endpoint that responds.
          if (state->trustCenterKeyEstablishmentEndpoint
              == UNDEFINED_ENDPOINT) {
            emberAfRegistrationPrintln("Discovered Key Establishment cluster"
                                       " on endpoint 0x%x",
                                       endpointList->list[i]);
            state->trustCenterKeyEstablishmentEndpoint = endpointList->list[i];
          } else {
            emberAfRegistrationPrintln("INFO: Ignored Key Establishment cluster"
                                       " on endpoint 0x%x",
                                       endpointList->list[i]);
          }
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
        } else if (state->state == STATE_DISCOVER_ENERGY_SERVICE_INTERFACES) {
          emberAfRegistrationPrintln("Discovered Energy Server Interface"
                                     " on node 0x%2x endpoint 0x%x",
                                     result->matchAddress,
                                     endpointList->list[i]);

          state->esiEntry = emberAfPluginEsiManagementEsiLookUpByShortIdAndEndpoint(result->matchAddress,
                                                                                    endpointList->list[i]);
          if (state->esiEntry == NULL) {
            state->esiEntry = emberAfPluginEsiManagementGetFreeEntry();
          }

          if (state->esiEntry != NULL) {
            state->esiEntry->nodeId = result->matchAddress;
            state->esiEntry->networkIndex = networkIndex;
            state->esiEntry->endpoint = endpointList->list[i];
            state->esiEntry->age = 0;
          } else {
            emberAfRegistrationPrintln("INFO: Ignored Energy Server Interface"
                                       " on node 0x%2x endpoint 0x%x"
                                       " because table is full",
                                       result->matchAddress,
                                       endpointList->list[i]);
          }
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
        }
      }
    }
  }

  if (state->state == STATE_DISCOVER_KEY_ESTABLISHMENT_CLUSTER) {
    if (state->trustCenterKeyEstablishmentEndpoint == UNDEFINED_ENDPOINT) {
      emberAfRegistrationPrintln("ERR: Failed to find Key Establishment cluster");
      resumeAfterFixedDelay(EMBER_ERR_FATAL);
    } else {
      EmberKeyStruct keyStruct;
      if (emberGetKey(EMBER_TRUST_CENTER_LINK_KEY, &keyStruct)
          != EMBER_SUCCESS) {
        emberAfRegistrationPrintln("ERR: Failed to get trust center link key");
        emberAfRegistrationAbortCallback();
        return;
      }

      // If we don't have full Smart Energy Security or if the key is already
      // authorized, we can skip key establishment and move on to ESI discovery.
      if (emberAfIsFullSmartEnergySecurityPresent() == EMBER_AF_INVALID_KEY_ESTABLISHMENT_SUITE) {
        emberAfRegistrationFlush();
        emberAfRegistrationPrintln("WARN: Skipping key establishment"
                                   " due to missing libraries or certificate"
                                   " - see 'info' command for more detail");
        emberAfRegistrationFlush();
        transition(NEXT_STATE);
      } else if ((keyStruct.bitmask & EMBER_KEY_IS_AUTHORIZED) != 0U) {
        emberAfRegistrationPrintln("Skipping key establishment"
                                   " because key is already authorized");
        transition(NEXT_STATE);
      } else {
        transition(STATE_PERFORM_KEY_ESTABLISHMENT);
      }
    }
#ifdef EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  } else if (state->state == STATE_DISCOVER_ENERGY_SERVICE_INTERFACES) {
    if (result->status == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE) {
      // Point to the first active entry with age 0 (if any).
      state->esiEntry = emberAfPluginEsiManagementGetNextEntry(NULL, 0);
      // There is no entry with age 0, i.e., we did not discover any ESI during
      // this discovery cycle.
      if (state->esiEntry == NULL) {
        // TODO: For now we just return an error. We might consider checking if
        // we have an ESI in the table "young enough" before returning an error.
        emberAfRegistrationPrintln("ERR: Failed to find Energy Service Interfaces");
        resumeAfterFixedDelay(EMBER_ERR_FATAL);
      } else {
        transition(STATE_DISCOVER_IEEE_ADDRESSES);
      }
    }
  } else if (state->state == STATE_DISCOVER_IEEE_ADDRESSES) {
    assert(state->esiEntry != NULL && state->esiEntry->nodeId != EMBER_NULL_NODE_ID);
    if (result->status == EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE) {
      emberAfRegistrationPrintln("Discovered IEEE address"
                                 " for node 0x%2x",
                                 state->esiEntry->nodeId);
      MEMMOVE(state->esiEntry->eui64, result->responseData, EUI64_SIZE);
      if (emberAfAddAddressTableEntry(state->esiEntry->eui64,
                                      state->esiEntry->nodeId)
          == EMBER_NULL_ADDRESS_TABLE_INDEX) {
        emberAfRegistrationPrintln("WARN: Could not add address table entry"
                                   " for node 0x%2x",
                                   state->esiEntry->nodeId);
      }
      state->esiEntry = emberAfPluginEsiManagementGetNextEntry(state->esiEntry, 0);
      if (state->esiEntry == NULL) {
        // Point to the first active entry with age 0 (if any).
        state->esiEntry = emberAfPluginEsiManagementGetNextEntry(NULL, 0);
        transition(STATE_PERFORM_PARTNER_LINK_KEY_EXCHANGE);
      } else {
        resumeAfterFixedDelay(EMBER_SUCCESS);
      }
    } else {
      emberAfRegistrationPrintln("ERR: Failed to discover IEEE address"
                                 " for node 0x%2x",
                                 state->esiEntry->nodeId);
      resumeAfterFixedDelay(EMBER_ERR_FATAL);
    }
#endif //EMBER_AF_PLUGIN_SMART_ENERGY_REGISTRATION_ESI_DISCOVERY_REQUIRED
  }
}
