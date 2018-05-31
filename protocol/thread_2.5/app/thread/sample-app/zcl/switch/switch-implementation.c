// Copyright 2015 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_HAL
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE

// WARNING: This sample application uses fixed network parameters and the well-
// know sensor/sink network key as the master key.  This is done for
// demonstration purposes, so nodes can join without a commissioner (i.e., out-
// of-band commissioning), and so that packets will decrypt automatically in
// Simplicity Studio.  Predefined network parameters only work for a single
// deployment and using predefined keys is a significant security risk.  Real
// devices MUST use random parameters and keys.
//
// These parameters have been chosen to match the border router sample
// application, which will allow this design to perform out of band joining with
// the border router sample application without need for modification.
static const uint8_t preferredChannel = 19;
static const uint8_t networkId[EMBER_NETWORK_ID_SIZE] = "precommissioned";
static const EmberPanId panId = 0x1075;
static const EmberIpv6Prefix ulaPrefix = {
  { 0xFD, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
};
static const uint8_t extendedPanId[EXTENDED_PAN_ID_SIZE] = {
  0xc6, 0xef, 0xe1, 0xb4, 0x5f, 0xc7, 0x8e, 0x4f
};
static const EmberKeyData masterKey = {
  { 0x65, 0x6D, 0x62, 0x65, 0x72, 0x20, 0x45, 0x4D,
    0x32, 0x35, 0x30, 0x20, 0x63, 0x68, 0x69, 0x70, },
};

#define DOWN_BUTTON BUTTON0
#define UP_BUTTON   BUTTON1
EmberEventControl downEventControl;
EmberEventControl upEventControl;
EmberEventControl ezModeEventControl;
static bool holdingDown = false;
static bool holdingUp = false;
static bool waitingForReleased = false;
#define MIN_LEVEL 0x00
#define MAX_LEVEL 0xFF
#define TRANSITION_TIME_DS 20 // 2.0 seconds
bool haveOnOffServer = false;
bool haveLevelControlServer = false;
static void down(bool useLevelControl);
static void up(bool useLevelControl);
static void stop(void);
static void findLight(void);

static EmberZclDestination_t light;

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
EmberEventControl stateEventControl;
static void setNextState(uint8_t nextState);

void emberAfNetworkStatusCallback(EmberNetworkStatus newNetworkStatus,
                                  EmberNetworkStatus oldNetworkStatus,
                                  EmberJoinFailureReason reason)
{
  // This callback is called whenever the network status changes, like when
  // we finish joining to a network or when we lose connectivity.  If we have
  // no network, we try joining to one.  If we have a saved network, we try to
  // resume operations on that network.  When we are joined and attached to the
  // network, we are in the steady state and wait for input from the user.

  emberEventControlSetInactive(stateEventControl);

  switch (newNetworkStatus) {
    case EMBER_NO_NETWORK:
      if (reason != EMBER_JOIN_FAILURE_REASON_NONE) {
        emberAfCorePrintln("ERR: Joining failed: 0x%x", reason);
      }
      setNextState(COMMISSION_NETWORK);
      break;
    case EMBER_SAVED_NETWORK:
      setNextState(RESUME_NETWORK);
      break;
    case EMBER_JOINING_NETWORK:
      // Wait for either the "attaching" or "no network" state.
      break;
    case EMBER_JOINED_NETWORK_ATTACHING:
      // Wait for either the "attached" or "no parent" state.
      if (oldNetworkStatus == EMBER_JOINED_NETWORK_ATTACHED) {
        emberAfCorePrintln("ERR: Lost connection to network");
      }
      break;
    case EMBER_JOINED_NETWORK_ATTACHED:
      emberAfCorePrintln("Attached");
      state = STEADY;
      break;
    case EMBER_JOINED_NETWORK_NO_PARENT:
      if (state == ATTACH_TO_NETWORK) {
        emberAfCorePrintln("ERR: Reattaching failed");
      } else {
        emberAfCorePrintln("ERR: No connection to network");
      }
      setNextState(ATTACH_TO_NETWORK);
      break;
    default:
      assert(false);
      break;
  }
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
    setNextState(RESET_NETWORK_STATE);
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
    setNextState(JOIN_COMMISSIONED);
  } else {
    emberAfCorePrintln("ERR: Unable to commission: 0x%x", status);
    setNextState(RESET_NETWORK_STATE);
  }
}

static void joinCommissioned(void)
{
  emberAfCorePrintln("Joining...");
  emberJoinCommissioned(3,                       // radio tx power
                        EMBER_SLEEPY_END_DEVICE,
                        false);                  // don't require connectivity
}

void emberJoinNetworkReturn(EmberStatus status)
{
  // This return indicates whether the stack is going to attempt to join.  If
  // so, the result is reported later as a network status change.  Otherwise,
  // we just give up and reset our network state, which will trigger a fresh
  // join attempt.

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: Unable to join: 0x%x", status);
    setNextState(RESET_NETWORK_STATE);
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
    setNextState(RESET_NETWORK_STATE);
  }
}

static void resetNetworkState(void)
{
  emberAfCorePrintln("Resetting...");
  emberResetNetworkState();
}

void emberResetNetworkStateReturn(EmberStatus status)
{
  // If we ever leave the network, we go right back to joining again.  This
  // could be triggered by an external CLI command.

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: Unable to reset: 0x%x", status);
  }
}

bool emberAfPluginIdleSleepOkToSleepCallback(uint32_t durationMs)
{
  // Keep the switch awake, so it is usable as a test device.

  return false;
}

bool emberAfPluginPollingOkToLongPollCallback(void)
{
  // Keep the switch fast polling, so it is usable as a test device.

  return false;
}

void emberZclGetPublicKeyCallback(const uint8_t **publicKey,
                                  uint16_t *publicKeySize)
{
  *publicKey = emberEui64()->bytes;
  *publicKeySize = EUI64_SIZE;
}

void emberZclNotificationCallback(const EmberZclNotificationContext_t *context,
                                  const EmberZclClusterSpec_t *clusterSpec,
                                  EmberZclAttributeId_t attributeId,
                                  const void *buffer,
                                  size_t bufferLength)
{
  if (emberZclAreClusterSpecsEqual(&emberZclClusterOnOffServerSpec, clusterSpec)
      && attributeId == EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF) {
    bool onOff = *((bool *)buffer);
    emberAfCorePrintln("Light is %s", (onOff ? "on" : "off"));
  } else if (emberZclAreClusterSpecsEqual(&emberZclClusterLevelControlServerSpec,
                                          clusterSpec)
             && attributeId == EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL) {
    uint8_t currentLevel = *((uint8_t *)buffer);
    emberAfCorePrintln("Light is at 0x%x", currentLevel);
  }
}

void emberZclIdentifyServerStartIdentifyingCallback(uint16_t identifyTimeS)
{
  // This callback is called whenever the endpoint should identify itself.  The
  // identification procedure is application specific, and could be implemented
  // by blinking an LED, playing a sound, or displaying a message.

  emberAfCorePrintln("Identifying...");
}

void emberZclIdentifyServerStopIdentifyingCallback(void)
{
  // This callback is called whenever the endpoint should stop identifying
  // itself.

  emberAfCorePrintln("Identified");
}

void halButtonIsr(uint8_t button, uint8_t state)
{
  // Buttons can be used to bind to a light and then control that light.
  // Pressing both buttons will cause the node to start EZ Mode commissioning
  // to find other nodes on the network.  If other nodes implement clusters
  // that correspond to ours (e.g., we have the On/Off client and they have the
  // On/Off server) and they are also in EZ Mode, we will create a binding to
  // them and they will create a binding to us.
  // Once bindings are established, a quick press on the down button (usually
  // BUTTON0 or PB0) will send the Off command.  Pressing and holding the down
  // button will send the Move to Level (with On/Off) command with the target
  // level of 0%.  When the button is released, Stop (with On/Off) will be
  // sent.  Similarly, the up button (usually BUTTON1 or PB1) will send On or
  // Move to Level (with On/Off) with a target level of 100%.

  EmberEventControl *thisEvent = (button == DOWN_BUTTON
                                  ? &downEventControl
                                  : &upEventControl);
  bool thisPressed = (state == BUTTON_PRESSED);
  bool thisHeld = (button == DOWN_BUTTON ? holdingDown : holdingUp);

  EmberEventControl *otherEvent = (button == DOWN_BUTTON
                                   ? &upEventControl
                                   : &downEventControl);
  bool otherPressed = (halButtonState(button == DOWN_BUTTON
                                      ? UP_BUTTON
                                      : DOWN_BUTTON)
                       == BUTTON_PRESSED);
  bool otherHeld = (button == DOWN_BUTTON ? holdingUp : holdingDown);

  waitingForReleased = waitingForReleased && (thisPressed || otherPressed);

  if (!otherHeld && !waitingForReleased) {
    if (otherPressed && thisPressed && emberEventControlGetActive(*otherEvent)) {
      emberEventControlSetInactive(*otherEvent);
      emberEventControlSetActive(ezModeEventControl);
      waitingForReleased = true;
    } else if (thisPressed) {
      emberEventControlSetDelayMS(*thisEvent,
                                  MILLISECOND_TICKS_PER_QUARTERSECOND);
    } else if (thisHeld) {
      emberEventControlSetActive(*thisEvent);
    }
  }
}

void downEventHandler(void)
{
  emberEventControlSetInactive(downEventControl);
  if (halButtonState(DOWN_BUTTON) == BUTTON_PRESSED) {
    down(true); // level control
    holdingDown = true;
  } else if (holdingDown) {
    stop();
    holdingDown = false;
  } else {
    down(false); // on/off
  }
}

void upEventHandler(void)
{
  emberEventControlSetInactive(upEventControl);
  if (halButtonState(UP_BUTTON) == BUTTON_PRESSED) {
    up(true); // level/control
    holdingUp = true;
  } else if (holdingUp) {
    stop();
    holdingUp = false;
  } else {
    up(false); // on/off
  }
}

static void down(bool useLevelControl)
{
  findLight();
  if (useLevelControl && haveLevelControlServer) {
    EmberZclClusterLevelControlServerCommandMoveToLevelWithOnOffRequest_t request;
    request.level = MIN_LEVEL;
    request.transitionTime = TRANSITION_TIME_DS;
    emberZclSendClusterLevelControlServerCommandMoveToLevelWithOnOffRequest(&light,
                                                                            &request,
                                                                            NULL);
  } else if (haveOnOffServer) {
    emberZclSendClusterOnOffServerCommandOffRequest(&light, NULL, NULL);
  }
}

static void up(bool useLevelControl)
{
  findLight();
  if (useLevelControl && haveLevelControlServer) {
    EmberZclClusterLevelControlServerCommandMoveToLevelWithOnOffRequest_t request;
    request.level = MAX_LEVEL;
    request.transitionTime = TRANSITION_TIME_DS;
    emberZclSendClusterLevelControlServerCommandMoveToLevelWithOnOffRequest(&light,
                                                                            &request,
                                                                            NULL);
  } else if (haveOnOffServer) {
    emberZclSendClusterOnOffServerCommandOnRequest(&light, NULL, NULL);
  }
}

static void stop(void)
{
  if (haveLevelControlServer) {
    emberZclSendClusterLevelControlServerCommandStopWithOnOffRequest(&light,
                                                                     NULL,
                                                                     NULL);
  }
}

static void findLight(void)
{
  // Find a binding to either a Level Control server or an On/Off server.  This
  // prefers Level Control over On/Off, and assumes that all Level Control
  // servers are also On/Off servers, which probably isn't true.

  for (EmberZclBindingId_t i = 0; i < EMBER_ZCL_BINDING_TABLE_SIZE; i++) {
    EmberZclBindingEntry_t entry = { 0 };
    if (emberZclGetBinding(i, &entry)) {
      if (emberZclAreClusterSpecsEqual(&emberZclClusterLevelControlClientSpec,
                                       &entry.clusterSpec)) {
        haveOnOffServer = true;
        haveLevelControlServer = true;
        emZclReadDestinationFromBinding(&entry, &light);
        break;
      } else if (!haveOnOffServer
                 && emberZclAreClusterSpecsEqual(&emberZclClusterOnOffClientSpec,
                                                 &entry.clusterSpec)) {
        emZclReadDestinationFromBinding(&entry, &light);
        haveOnOffServer = true;
        // don't break - prefer level control
      }
    }
  }
  if (!haveOnOffServer && !haveLevelControlServer) {
    emberAfCorePrintln("ERR: Not paired");
  }
}

void ezModeEventHandler(void)
{
  emberEventControlSetInactive(ezModeEventControl);
  emberZclStartEzMode();
}

void stateEventHandler(void)
{
  emberEventControlSetInactive(stateEventControl);

  switch (state) {
    case RESUME_NETWORK:
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
      resetNetworkState();
      break;
    default:
      assert(false);
      break;
  }
}

static void setNextState(uint8_t nextState)
{
  state = nextState;
  emberEventControlSetActive(stateEventControl);
}
