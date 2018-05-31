// Copyright 2017 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_HAL
#include EMBER_AF_API_LED_BLINK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_CONNECTION_MANAGER
#include EMBER_AF_API_OCCUPANCY

#define OCCUPANCY_DETECTED_NUM_BLINKS 5
static uint16_t occupancyDetectedBlinkPattern[] = { 5, 100 };

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

  return emberConnectionManagerIsOkToLongPoll();
}

// Whenever occupancy is detected, the device should do a super short blink
// pattern to provide feedback to the user.  This pattern should not blink when
// the more important identify and mid EZ mode commissioning patterns are
// blinking
void emberZclOccupancySensingServerOccupancyStateChangedCallback(HalOccupancyState occupancyState)
{
  bool isIdentifying = false;

  if (occupancyState == HAL_OCCUPANCY_STATE_OCCUPIED) {
    uint16_t identifyTimeS;
    EmberZclStatus_t status;
    EmberZclEndpointId_t endpoint;
    EmberZclEndpointIndex_t i;

    for (i = 0; i < emZclEndpointCount; i++) {
      endpoint = emberZclEndpointIndexToId(i,
                                           &emberZclClusterOccupancySensingServerSpec);
      if (endpoint != EMBER_ZCL_ENDPOINT_NULL) {
        status = emberZclReadAttribute(endpoint,
                                       &emberZclClusterIdentifyServerSpec,
                                       EMBER_ZCL_CLUSTER_IDENTIFY_SERVER_ATTRIBUTE_IDENTIFY_TIME,
                                       &identifyTimeS,
                                       sizeof(identifyTimeS));
        if (status != EMBER_ZCL_STATUS_SUCCESS) {
          emberAfCorePrintln("End node UI unable to identify on endpoint %d!\n",
                             i);
        } else if (identifyTimeS) {
          isIdentifying = true;
        }
      }
    }

    if (!isIdentifying && !emberZclEzModeIsActive() && !emberConnectionManagerIsSearching()) {
      halLedBlinkPattern(OCCUPANCY_DETECTED_NUM_BLINKS,
                         COUNTOF(occupancyDetectedBlinkPattern),
                         occupancyDetectedBlinkPattern);
    }
  }
}

void emberAfInitCallback(void)
{
#if defined(CORTEXM3_EFR32)
  // Set up the LED configuration to have the LED turned off on boot
  halInternalInitLed();

  // Currently, the BSP init functions will only set the LED to be a push/pull
  // output.  As such, the pin needs to be manually configured if it is
  // intended to be used in any mode other than push/pull
  GPIO_PinModeSet(BSP_LED0_PORT, BSP_LED0_PIN, gpioModeWiredAndPullUp, 1);

  // Currently, the button init functions do not allow for setting the pin mode
  // to internal pull up/down.  As such, the pin needs to be manually
  // configured if it is to be used as intended with the reference design hw.
  GPIO_PinModeSet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN, gpioModeInputPull, 1);
#endif

  halLedBlinkLedOff(0);
}

// EFR32 does not yet support the virtual UART, and the physical UART will
// drop the first character it receives when it is asleep.  As such, the user
// needs a way to get a reliable CLI.  This is implemented through the
// installation jumper, which if in place will prevent the device from going to
// sleep (along with causing occupancy blinks to occur more frequently).
bool emberAfPluginIdleSleepOkToSleepCallback(uint32_t durationMs)
{
#if defined(CORTEXM3_EFR32)
  if (GPIO_PinInGet(BSP_PYD1698_INSTALLATION_JP_PORT,
                    BSP_PYD1698_INSTALLATION_JP_PIN)) {
    return true;
  }
  return false;
#else
  return true;
#endif
}

void emberZclGetDefaultReportingConfigurationCallback(EmberZclEndpointId_t endpointId,
                                                      const EmberZclClusterSpec_t *clusterSpec,
                                                      EmberZclReportingConfiguration_t *configuration)
{
  // We have reportable attributes in the occupancy sensing server
  if (emberZclAreClusterSpecsEqual(&emberZclClusterOccupancySensingServerSpec,
                                   clusterSpec)) {
    configuration->minimumIntervalS =  1;
    configuration->maximumIntervalS = 60 * 30;
  }
}

void emberZclGetDefaultReportableChangeCallback(EmberZclEndpointId_t endpointId,
                                                const EmberZclClusterSpec_t *clusterSpec,
                                                EmberZclAttributeId_t attributeId,
                                                void *buffer,
                                                size_t bufferLength)
{
  uint8_t level = 1;

  if (emberZclAreClusterSpecsEqual(&emberZclClusterOccupancySensingServerSpec,
                                   clusterSpec)
      && (attributeId == EMBER_ZCL_CLUSTER_OCCUPANCY_SENSING_SERVER_ATTRIBUTE_OCCUPANCY)) {
    *(uint8_t *)buffer = level;
    emberAfCorePrintln("getDefRepChangeCallback!\n");
  }
}
