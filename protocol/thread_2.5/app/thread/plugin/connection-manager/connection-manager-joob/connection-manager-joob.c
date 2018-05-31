// Copyright 2017 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_HAL
#ifdef EMBER_AF_API_COMMAND_INTERPRETER2
  #include EMBER_AF_API_COMMAND_INTERPRETER2
#endif
#include EMBER_AF_API_CONNECTION_MANAGER
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

#define ROUTER            EMBER_ROUTER
#define END_DEVICE        EMBER_END_DEVICE
#define SLEEPY_END_DEVICE EMBER_SLEEPY_END_DEVICE

#define NETWORK_JOIN_RETRY_DELAY_MS (1024 * 5)
#define ULA_PREFIX_SIZE 8

#define TIME_TO_WAIT_FOR_LONG_POLLING_MS 60000

//------------------------------------------------------------------------------
// Event function forward declaration
EmberEventControl emConnectionManagerNetworkStateEventControl;
EmberEventControl emConnectionManagerOrphanEventControl;
EmberEventControl emConnectionManagerOkToLongPollEventControl;

static bool okToLongPoll = false;

static void resetOkToLongPoll(void);
static void setNextStateWithDelay(uint8_t nextState, uint32_t delayMs);

// WARNING: This sample application uses fixed network parameters and the well-
// know sensor/sink network key as the master key.  This is done for
// demonstration purposes, so nodes can join without a commissioner (i.e., out-
// of-band commissioning), and so that packets will decrypt automatically in
// Simplicity Studio.  Predefined network parameters only work for a single
// deployment and using predefined keys is a significant security risk.  Real
// devices MUST use random parameters and keys.
//
// These parameters have been chosen to match the border router, which will
// allow this design to perform out of band joining with the border router
// without need for modification.
//TODO: can we parameterize a string?
#if EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_NETWORK_NAME_LENGTH > EMBER_NETWORK_ID_SIZE
  #error "Network name specified in Connection Manager: Out Of Band Joining too long!"
#endif
static uint8_t networkId[EMBER_NETWORK_ID_SIZE] = EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_NETWORK_NAME;

static uint8_t preferredChannel = EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_CHANNEL;
static uint16_t panId = EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_PAN_ID;
static EmberIpv6Prefix ulaPrefix =
{ EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_ULA_PREFIX };

static uint8_t extendedPanId[EXTENDED_PAN_ID_SIZE] = EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_EXTENDED_PAN_ID;

static const EmberKeyData masterKey = {
  { 0x65, 0x6D, 0x62, 0x65, 0x72, 0x20, 0x45, 0x4D,
    0x32, 0x35, 0x30, 0x20, 0x63, 0x68, 0x69, 0x70, },
};

enum {
  INITIAL                 = 0,
  RESUME_NETWORK          = 1,
  COMMISSION_NETWORK      = 2,
  JOIN_COMMISSIONED       = 3,
  ATTACH_TO_NETWORK       = 4,
  STEADY                  = 5,
  RESET_NETWORK_STATE     = 6,
};

static uint8_t state = INITIAL;
static uint8_t failedAttempts = 0;
static bool isOrphaned = false;
static bool isSearching = false;
static bool previouslyConnected = false;
static bool stopSearching = false;

void emberConnectionManagerStartConnect(void)
{
  EmberNetworkStatus networkStatus;

  networkStatus = emberNetworkStatus();

  switch (networkStatus) {
    case EMBER_JOINED_NETWORK_ATTACHED:
      // if the device is already on the network, do nothing
      emberAfCorePrintln("Connected to network: Attached");
      emberConnectionManagerConnectCompleteCallback(EMBER_CONNECTION_MANAGER_STATUS_CONNECTED);
      return;
    default:
      // In all other cases, set the number of fail attempts to 0 and start
      // searching again.
      failedAttempts = 0;
      isOrphaned = false;
      isSearching = true;
      previouslyConnected = false;
      stopSearching = false;
      emberAttachToNetwork();
  }
}

void emberConnectionManagerStopConnect(void)
{
  EmberNetworkStatus networkStatus = emberNetworkStatus();

  // if we're already on the network, then a start connect should do nothing
  if (networkStatus == EMBER_JOINED_NETWORK_ATTACHED) {
    return;
  }

  stopSearching = true;
  isSearching = false;
}

void emConnectionManagerJoobNetworkStatusHandler(EmberNetworkStatus newNetworkStatus,
                                                 EmberNetworkStatus oldNetworkStatus,
                                                 EmberJoinFailureReason reason)
{
  // This callback is called whenever the network status changes, like when
  // we finish joining to a network or when we lose connectivity.  If we have
  // no network, we try joining to one.  If we have a saved network, we try to
  // resume operations on that network.  When we are joined and attached to the
  // network, we are in the steady state and wait for input from the user.

  emberEventControlSetInactive(emConnectionManagerNetworkStateEventControl);

  emberAfCorePrintln("net stat callback with status of %d",
                     emberNetworkStatus());
  switch (newNetworkStatus) {
    case EMBER_NO_NETWORK:
    case EMBER_JOINED_NETWORK_NO_PARENT:
      // if we were connected and now we're not, perform orphan behavior:
      //   On first occurence of this, set orphan event active immediately.
      //   On all subsequent occurences, set orphan event active with a delay
      if (previouslyConnected && !isOrphaned) {
        isOrphaned = true;
        emberEventControlSetActive(emConnectionManagerOrphanEventControl);
        emberAfCorePrintln("Device is orphaned!  Attempting to rejoin");
        break;
      } else if (previouslyConnected && isOrphaned) {
        emberEventControlSetDelayMinutes(emConnectionManagerOrphanEventControl,
                                         EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_ORPHAN_REJOIN_DELAY_MINUTES);
        emberAfCorePrintln("Still orphaned, retrying in %d minutes",
                           EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_ORPHAN_REJOIN_DELAY_MINUTES);
        break;
      }

      // If the device was not previously connected, it is in the process of
      // joining and should either attempt to rejoin again in DELAY seconds, or
      // stop attempting to join if the device has exceeded the number of
      // consecutive network join attempts.
      if (reason != EMBER_JOIN_FAILURE_REASON_NONE) {
        emberAfCorePrintln("Joining attempt %d failed: 0x%x",
                           failedAttempts,
                           reason);
      } else {
        emberAfCorePrintln("Joining attempt %d failed", failedAttempts);
      }
      failedAttempts++;

      // If emberConnectionManagerStopConnect, stop searching
      if (stopSearching) {
        break;
      }

      // If the plugin options specify a limited number of successive join
      // attempts, make sure that the device has not exceeded that limit.
      // Otherwise, always attempt to rejoin.
#if (EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_NUM_REJOIN_ATTEMPTS != 0)
      if (failedAttempts > EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_NUM_REJOIN_ATTEMPTS) {
        emberAfCorePrintln("Unable to join within %d attempts",
                           EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_NUM_REJOIN_ATTEMPTS);
        isSearching = false;
        emberConnectionManagerConnectCompleteCallback(EMBER_CONNECTION_MANAGER_STATUS_TIMED_OUT);
      } else {
        setNextStateWithDelay(COMMISSION_NETWORK, NETWORK_JOIN_RETRY_DELAY_MS);
      }
#else
      setNextStateWithDelay(COMMISSION_NETWORK, NETWORK_JOIN_RETRY_DELAY_MS);
#endif //(EMBER_AF_PLUGIN_CONNECTION_MANAGER_NUM_ATTEMPTS != 0)
      break;

    case EMBER_SAVED_NETWORK:
      previouslyConnected = true;
      setNextStateWithDelay(RESUME_NETWORK, 0);
      break;
    case EMBER_JOINING_NETWORK:
      // Wait for either the "attaching" or "no network" state.
      break;
    case EMBER_JOINED_NETWORK_ATTACHING:
      // Wait for the "attached" state.  If the device was previously on the
      // network, set isOrphaned to true in case this state was somehow
      // entered without first going through the NO_PARENT state
      if (oldNetworkStatus == EMBER_JOINED_NETWORK_ATTACHED) {
        isOrphaned = true;
      }
      break;
    case EMBER_JOINED_NETWORK_ATTACHED:
      emberAfCorePrintln("Connected to network: Attached");
      state = STEADY;
      failedAttempts = 0;
      previouslyConnected = true;
      isOrphaned = false;
      isSearching = false;
      if (EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_ROLE == SLEEPY_END_DEVICE) {
        emberEventControlSetDelayMS(emConnectionManagerOkToLongPollEventControl, TIME_TO_WAIT_FOR_LONG_POLLING_MS);
      }
      emberConnectionManagerConnectCompleteCallback(EMBER_CONNECTION_MANAGER_STATUS_CONNECTED);
      break;
    default:
      assert(false);
      break;
  }
}

void emConnectionManagerOkToLongPollHandler(void)
{
  emberEventControlSetInactive(emConnectionManagerOkToLongPollEventControl);
  okToLongPoll = true;
}

bool emberConnectionManagerIsOkToLongPoll(void)
{
  return okToLongPoll;
}

void emberConnectionManagerSearchForParent(void)
{
  if (isOrphaned) {
    emberEventControlSetActive(emConnectionManagerOrphanEventControl);
  }
}

void emberResetNetworkStateReturn(EmberStatus status)
{
  if (status != EMBER_SUCCESS) {
    emberResetNetworkState();
  } else {
    previouslyConnected = false;
    isOrphaned = false;
  }
}

void emConnectionManagerOrphanEventHandler(void)
{
  emberEventControlSetInactive(emConnectionManagerOrphanEventControl);
  emberAttachToNetwork();
}

bool emberConnectionmanagerIsOrphaned(void)
{
  return isOrphaned;
}

bool emberConnectionManagerIsSearching(void)
{
  return isSearching;
}

void emberConnectionManagerLeaveNetwork(void)
{
  isOrphaned = false;
  previouslyConnected = false;
  emberResetNetworkState();
}

static void resumeNetwork(void)
{
  emberAfCorePrintln("Resuming...");
  emberResumeNetwork();
}

void emberResumeNetworkReturn(EmberStatus status)
{
  // This return indicates whether the stack is going to attempt to resume.  If
  // so, the result is reported later as a network status change.  If we cannot
  // even attempt to resume, we just give up and reset our network state, which
  // will trigger join instead.

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: Unable to resume: 0x%x", status);
    emberAfCorePrintln("Trying again in %d minutes",
                       EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_ORPHAN_REJOIN_DELAY_MINUTES);
    emberEventControlSetDelayMinutes(emConnectionManagerOrphanEventControl,
                                     EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_ORPHAN_REJOIN_DELAY_MINUTES);
  }
}

static void commissionNetwork(void)
{
  emberAfCorePrintln("Commissioning...");
  emberCommissionNetwork(preferredChannel,
                         0,                 // fallback channel mask - ignored
                         networkId,
                         sizeof(networkId),
                         panId,
                         ulaPrefix.bytes,
                         extendedPanId,
                         &masterKey,
                         0);                // key sequence
}

void emberCommissionNetworkReturn(EmberStatus status)
{
  // This return indicates whether the network was commissioned.  If so, we can
  // proceed to joining.  Otherwise, we just give up and reset our network
  // state, which will trigger a fresh join attempt.

  if (status == EMBER_SUCCESS) {
    setNextStateWithDelay(JOIN_COMMISSIONED, 0);
  } else {
    emberAfCorePrintln("ERR: Unable to commission: 0x%x", status);
    setNextStateWithDelay(RESET_NETWORK_STATE, 0);
  }
}

static void joinCommissioned(void)
{
  emberAfCorePrintln("Joining...");
  bool requireConnectivity;

  if (EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_ROLE == ROUTER) {
    requireConnectivity = true;
  } else {
    requireConnectivity = false;
  }

  emberJoinCommissioned(EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_RADIO_TX_POWER,
                        EMBER_AF_PLUGIN_CONNECTION_MANAGER_JOOB_ROLE,
                        requireConnectivity);
}

void emberJoinNetworkReturn(EmberStatus status)
{
  // This return indicates whether the stack is going to attempt to join.  If
  // so, the result is reported later as a network status change.  Otherwise,
  // we just give up and reset our network state, which will trigger a fresh
  // join attempt.

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: Unable to join: 0x%x", status);
    setNextStateWithDelay(RESET_NETWORK_STATE, 0);
  }
}

static void attachToNetwork(void)
{
  emberAfCorePrintln("Reattaching...");
  emberAttachToNetwork();
}

void emberAttachToNetworkReturn(EmberStatus status)
{
  // This return indicates whether the stack is going to attempt to attach.  If
  // so, the result is reported later as a network status change.  If we cannot
  // even attempt to attach, we just give up and reset our network state, which
  // will trigger a fresh join attempt.

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: Unable to reattach: 0x%x", status);
    setNextStateWithDelay(RESET_NETWORK_STATE, 0);
  }
}

static void resetNetworkState(void)
{
  emberAfCorePrintln("Resetting...");
  emberResetNetworkState();
}

void emConnectionManagerNetworkStateEventHandler(void)
{
  emberEventControlSetInactive(emConnectionManagerNetworkStateEventControl);

  switch (state) {
    case RESUME_NETWORK:
      resetOkToLongPoll();
      resumeNetwork();
      break;
    case COMMISSION_NETWORK:
      commissionNetwork();
      break;
    case JOIN_COMMISSIONED:
      joinCommissioned();
      break;
    case ATTACH_TO_NETWORK:
      attachToNetwork();
      break;
    case RESET_NETWORK_STATE:
      resetOkToLongPoll();
      resetNetworkState();
      break;
    default:
      assert(false);
      break;
  }
}

static void resetOkToLongPoll(void)
{
  emberAfCorePrintln("Resetting OK to Long Poll");
  emberEventControlSetInactive(emConnectionManagerOkToLongPollEventControl);
  okToLongPoll = false;
}

static void setNextStateWithDelay(uint8_t nextState, uint32_t delayMs)
{
  state = nextState;

  if (delayMs) {
    emberEventControlSetDelayMS(emConnectionManagerNetworkStateEventControl, delayMs);
  } else {
    emberEventControlSetActive(emConnectionManagerNetworkStateEventControl);
  }
}

//connection-manager-joob set panId <panId:2>
void emConnectionManagerJoobCliSetPanId(void)
{
  uint16_t newPanId = emberUnsignedCommandArgument(0);

  panId = newPanId;
  emberAfCorePrintln("set pan ID to 0x%x.  Net reset to have this take effect",
                     panId);
}

//connection-manager-joob set extPanId <extended Pan Id:8>
void emConnectionManagerJoobCliSetExtPanId(void)
{
  uint8_t bufferLength;
  uint8_t *buffer = emberStringCommandArgument(0, &bufferLength);

  if (bufferLength != EXTENDED_PAN_ID_SIZE) {
    emberAfCorePrintln("Invalid ext pan ID size given: %d.  Expected %d",
                       bufferLength,
                       EXTENDED_PAN_ID_SIZE);
  }
  MEMCOPY(extendedPanId, buffer, bufferLength);

  emberAfCorePrint("New extended pan id set: ");
  emberAfCorePrintBuffer(extendedPanId, EXTENDED_PAN_ID_SIZE, false);
  emberAfCorePrintln("\nNet reset to have this take effect");
}

//connection-manager-joob set ula <ULA prefix:8>
void emConnectionManagerJoobCliSetUla(void)
{
  uint8_t bufferLength;
  uint8_t *buffer = emberStringCommandArgument(0, &bufferLength);
  uint8_t i;

  if (bufferLength != ULA_PREFIX_SIZE) {
    emberAfCorePrintln("Invalid ULA prefix size given: %d.  Expected %d",
                       bufferLength,
                       ULA_PREFIX_SIZE);
  }

  MEMCOPY(ulaPrefix.bytes, buffer, bufferLength);

  emberAfCorePrint("New extended pan id set: ");
  for ( i = 0; i < EXTENDED_PAN_ID_SIZE; i++) {
    emberAfCorePrint("0x%x ", ulaPrefix.bytes[i]);
  }
  emberAfCorePrintln("\nNet reset to have this take effect");
}

//connection-manager-joob set channel <channel:2>
void emConnectionManagerJoobCliSetChannel(void)
{
  uint8_t newChannel = emberUnsignedCommandArgument(0);

  preferredChannel = newChannel;
  emberAfCorePrintln("set channel to %d.  Net reset to have this take effect",
                     preferredChannel);
}
