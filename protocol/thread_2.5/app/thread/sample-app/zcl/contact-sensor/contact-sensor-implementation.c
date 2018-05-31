// Copyright 2017 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_HAL
#include EMBER_AF_API_CONNECTION_MANAGER
#include EMBER_AF_API_COMMAND_INTERPRETER2
#include EMBER_AF_API_GENERIC_INTERRUPT_CONTROL
#include EMBER_AF_API_GPIO_SENSOR
#include EMBER_AF_API_LED_BLINK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

void setContactStateCommand(void);
static void genericMessageResponseHandler(EmberCoapStatus status,
                                          const EmberZclCommandContext_t *context,
                                          void *commandResponse);
static void sendOnOffCommandToBindings(uint16_t command);

// These variables are used to track when it is safe for the device to begin
// long polling
static bool okToLongPoll = true;
static uint8_t outboundMessageCount = 0;

// These defines and variables are used to control the blink patterns used to
// signify a change in state for the contact sensor
#define CONTACT_DETECTED_NUM_BLINKS    1
#define NO_CONTACT_DETECTED_NUM_BLINKS 2
static uint16_t contactDetectedBlinkPattern[] = { 5, 100 };

#if defined(EMBER_AF_API_CONNECTION_MANAGER_JIB)
uint8_t emberConnectionManagerJibGetJoinKeyCallback(uint8_t **returnedJoinKey)
{
  static uint8_t joinKey[EMBER_JOIN_KEY_MAX_SIZE + 1] = { 0 };
  static uint8_t joinKeyLength = 0;

  // Using a join key generated from the EUI64 of the node is a security risk,
  // but is useful for demonstration purposes. See the warning above. This
  // hash generates alphanumeric characters in the ranges 0-9 and A-U. The
  // Thread specification disallows the characters I, O, Q, and Z, for
  // readability. I, O, and Q are therefore remapped to V, W, and X. Z is not
  // generated, so it is not remapped.
  const EmberEui64 *eui64;
  eui64 = emberEui64();
  for (joinKeyLength = 0; joinKeyLength < EUI64_SIZE; (joinKeyLength)++) {
    joinKey[joinKeyLength] = ((eui64->bytes[joinKeyLength] & 0x0F)
                              + (eui64->bytes[joinKeyLength] >> 4));
    joinKey[joinKeyLength] += (joinKey[joinKeyLength] < 10 ? '0' : 'A' - 10);
    if (joinKey[joinKeyLength] == 'I') {
      joinKey[joinKeyLength] = 'V';
    } else if (joinKey[joinKeyLength] == 'O') {
      joinKey[joinKeyLength] = 'W';
    } else if (joinKey[joinKeyLength] == 'Q') {
      joinKey[joinKeyLength] = 'X';
    }
  }

  emberAfCorePrintln("Join device to network using EUI \"%x%x%x%x%x%x%x%x\" and join passphrase \"%s\"",
                     emberEui64()->bytes[7],
                     emberEui64()->bytes[6],
                     emberEui64()->bytes[5],
                     emberEui64()->bytes[4],
                     emberEui64()->bytes[3],
                     emberEui64()->bytes[2],
                     emberEui64()->bytes[1],
                     emberEui64()->bytes[0],
                     joinKey);

  *returnedJoinKey = joinKey;
  return joinKeyLength;
}
#endif //defined(EMBER_AF_API_CONNECTION_MANAGER_JIB)

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
  if (emberZclCompareClusterSpec(&emberZclClusterOnOffServerSpec, clusterSpec)
      && attributeId == EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF) {
    bool onOff = *((bool *)buffer);
    emberAfCorePrintln("Light is %s", (onOff ? "on" : "off"));
  }
}

/** @brief Ok To Long Poll
 *
 * This function is called by the Polling plugin to determine if the node can
 * wait an extended period of time between polls.  Generally, a node can poll
 * infrequently when it does not expect to receive data, via its parent, from
 * other nodes in the network.  When data is expected, the node must poll more
 * frequently to avoid having its parent discard stale data due to the MAC
 * indirect transmission timeout (::EMBER_INDIRECT_TRANSMISSION_TIMEOUT).  The
 * application should return true if it is not expecting data or false
 * otherwise.
 */
bool emberAfPluginPollingOkToLongPollCallback(void)
{
  if (emberZclEzModeIsActive()) {
    return false;
  }

  // Check connection manager in addition to the status here bound to the
  // outboundMessageCount to make sure that long polling is allowed in both
  // cases
  return (emberConnectionManagerIsOkToLongPoll() && okToLongPoll);
}

static void sendOnOffCommandToBindings(uint16_t command)
{
  uint8_t numSends = 0;
  EmberZclDestination_t destination;
  EmberStatus status;

  // Send command to all all matching binding destinations.
  EmberZclBindingId_t bindingIdx = 0;
  while (emberZclGetDestinationFromBinding(&emberZclClusterOnOffServerSpec,
                                           &bindingIdx,
                                           &destination)) {
    switch (command) {
      case EMBER_ZCL_CLUSTER_ON_OFF_SERVER_COMMAND_ON:
        status = emberZclSendClusterOnOffServerCommandOnRequest(&destination,
                                                                NULL,
                                                                (EmberZclClusterOnOffServerCommandOnResponseHandler)(&genericMessageResponseHandler));
        if (status != EMBER_SUCCESS) {
          emberAfCorePrintln("Error 0x%0x when sending on command to bind idx %d",
                             status,
                             bindingIdx);
        } else {
          numSends++;
          outboundMessageCount++;
          okToLongPoll = false;
        }
        break;
      case EMBER_ZCL_CLUSTER_ON_OFF_SERVER_COMMAND_OFF:
        status = emberZclSendClusterOnOffServerCommandOffRequest(&destination,
                                                                 NULL,
                                                                 (EmberZclClusterOnOffServerCommandOffResponseHandler)(&genericMessageResponseHandler));
        if (status != EMBER_SUCCESS) {
          emberAfCorePrintln("Error 0x%0x when sending off command to bind idx %d",
                             status,
                             bindingIdx);
        } else {
          numSends++;
          outboundMessageCount++;
          okToLongPoll = false;
        }
        break;
    } //switch
  }

  if (numSends) {
    emberAfCorePrintln("Sent command 0x%x to %d nodes", command, numSends);
  } else {
    emberAfCorePrintln("Failed to send 0x%0x, no valid binds", command);
  }
}

static void genericMessageResponseHandler(EmberCoapStatus status,
                                          const EmberZclCommandContext_t *context,
                                          void *commandResponse)
{
  if (outboundMessageCount) {
    outboundMessageCount--;
    if (!outboundMessageCount) {
      okToLongPoll = true;
    }
  } else {
    emberAfCorePrintln("ERR! Received more responseHandler calls than cmd/att sends recorded");
  }
}

// When the GPIO assigned to the reed switch changes state, the contact sensor
// needs to send an onOff command to any bound onOff servers.
void emberAfPluginGpioSensorStateChangedCallback(HalGpioSensorState newSensorState)
{
  bool isIdentifying = false;
  uint16_t identifyTimeS;
  EmberZclStatus_t status;
  EmberZclEndpointId_t endpoint;
  EmberZclEndpointIndex_t i;

  for (i = 0; i < emZclEndpointCount; i++) {
    endpoint = emberZclEndpointIndexToId(i, &emberZclClusterIdentifyServerSpec);
    if (endpoint != EMBER_ZCL_ENDPOINT_NULL) {
      status = emberZclReadAttribute(endpoint,
                                     &emberZclClusterIdentifyServerSpec,
                                     EMBER_ZCL_CLUSTER_IDENTIFY_SERVER_ATTRIBUTE_IDENTIFY_TIME,
                                     &identifyTimeS,
                                     sizeof(identifyTimeS));
      if (status != EMBER_ZCL_STATUS_SUCCESS) {
        emberAfCorePrintln("Error querying identify state for endpoint %d!\n",
                           i);
      } else if (identifyTimeS) {
        isIdentifying = true;
      }
    }
  }

  if (newSensorState == HAL_GPIO_SENSOR_ACTIVE) {
    emberAfCorePrintln("Sensor open: Sending on");
    sendOnOffCommandToBindings(EMBER_ZCL_CLUSTER_ON_OFF_SERVER_COMMAND_ON);
    if (!isIdentifying && !emberZclEzModeIsActive()) {
      halLedBlinkPattern(NO_CONTACT_DETECTED_NUM_BLINKS,
                         COUNTOF(contactDetectedBlinkPattern),
                         contactDetectedBlinkPattern);
    }
  } else {
    emberAfCorePrintln("Sensor closed: Sending off");
    sendOnOffCommandToBindings(EMBER_ZCL_CLUSTER_ON_OFF_SERVER_COMMAND_OFF);
    if (!isIdentifying && !emberZclEzModeIsActive()) {
      halLedBlinkPattern(CONTACT_DETECTED_NUM_BLINKS,
                         COUNTOF(contactDetectedBlinkPattern),
                         contactDetectedBlinkPattern);
    }
  }
}

// Custom CLI entry that will manually set the contact state to be a user
// specified value
void setContactStateCommand(void)
{
  bool newContactState;

  newContactState = emberUnsignedCommandArgument(0);

  if ((newContactState != HAL_GPIO_SENSOR_NOT_ACTIVE)
      && (newContactState != HAL_GPIO_SENSOR_ACTIVE)) {
    emberAfCorePrintln("setContactState invoked with invalid argument!");
    emberAfCorePrintln("Expected 'setContactState <0|1>'");
  } else {
    emberAfPluginGpioSensorStateChangedCallback(newContactState);
  }
}

// This will always return true, as the device is always in a state in which it
// is ok for it to sleep, but that can make debugging difficult.  As such,
// this function will remain implemented here, so that it can be easily switched to
// a non-sleepy device for debug purposes
bool emberAfPluginIdleSleepOkToSleepCallback(uint32_t durationMs)
{
  return true;
}

// This will always return true, as the device is always in a state in which it
// is ok for it to idle, but that can make debugging difficult.  As such,
// this function will remain implemented here, so that it can be easily switched to
// a non-sleepy device for debug purposes
bool emberAfPluginIdleSleepOkToIdleCallback(void)
{
  return true;
}
