// Copyright 2017 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_HAL
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE

#include EMBER_AF_API_CONNECTION_MANAGER
#include EMBER_AF_API_LED_BLINK
#include EMBER_AF_API_POWER_METER_CS5463

#define STATUS_LED 0
#define POWER_LED 1
#define LED_ACTIVE_HIGH

// defines for simulation tests
#if defined(EMBER_TEST)
#define P_RELAY_ON_PORT      1
#define P_RELAY_OFF_PORT     1
#define P_RELAY_ON_PIN       1
#define P_RELAY_OFF_PIN      1
#define P_RELAY_ON           1
#define P_RELAY_OFF          1
#undef POWER_LED
#define POWER_LED            1
#undef STATUS_LED
#define STATUS_LED           1
#define gpioModePushPull     1

void GPIO_PinOutClear(uint8_t a, uint8_t b)
{
  return;
}
void GPIO_PinOutSet(uint8_t a, uint8_t b)
{
  return;
}
void GPIO_PinModeSet(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
  return;
}
#endif //defined(EMBER_TEST)

#define RELAY_SWTICH_STATE_KEEP_TIME_MS 20

#define POWER_STATE_OFF                  0
#define POWER_STATE_TOGGLE               2

#define POWER_EXCEPTION_TIMEOUT_MS                 120000
#define POWER_EXCEPTION_BIT_CS5463_OVERHEAT             0
#define POWER_EXCEPTION_OVER_HEAT_MASK               0x03

#define POWER_EXCEPTION_BIT_CS5463_OVERCURRENT          2

#define POWER_EXCEPTION_BIT_POWERSHUTDOWN               3

#define DEFAULT_ON_OFF_REPORTING_PERIOD_S              60

static uint16_t powerOffBlinkPattern[] =
{ 250, 250 };
static uint16_t powerOnBlinkPattern[] =
{ 1000, 250 };

#define DEFAULT_NUM_OVERHEAT_BLINKS \
  ((2 * 60) * (1000 / (250 + 250)))
#define DEFAULT_NUM_OVERCURRENT_BLINKS \
  ((2 * 60) * (1000 / (250 + 750)))
static uint16_t overHeatBlinkPattern[] =
{ 250, 250 };
static uint16_t overCurrentBlinkPattern[] =
{ 250, 750 };

static uint8_t powerExceptionMask;

EmberEventControl powerExceptionEventControl;

static void powerOnOff(uint8_t status);

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

void emberAfPluginButtonInterfaceButton0PressingCallback(void)
{
  emberAfCorePrintln("toggling power at user's request");
  powerOnOff(POWER_STATE_TOGGLE);
}

void emberZclPostAttributeChangeCallback(EmberZclEndpointId_t endpointId,
                                         const EmberZclClusterSpec_t *clusterSpec,
                                         EmberZclAttributeId_t attributeId,
                                         const void *buffer,
                                         size_t bufferLength)
{
  uint8_t onOff;

  // This callback is called whenever an attribute changes.  When the OnOff
  // attribute changes, set the power relay appropriately.  If an error
  // occurs, ignore it because there's really nothing we can do.
  if (emberZclAreClusterSpecsEqual(clusterSpec,
                                   &emberZclClusterOnOffServerSpec)) {
    onOff = ((uint8_t*)buffer)[0];
    emberAfCorePrintln("on/off attribute changed to %d", onOff);

    // If there is currently a power exception, the on/off is to be ignored and
    // the attribute should be set to off
    if ((READBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN))
        && (onOff != POWER_STATE_OFF)) {
      onOff = POWER_STATE_OFF;
      emberZclWriteAttribute(endpointId,
                             &emberZclClusterOnOffServerSpec,
                             EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                             (uint8_t *) &onOff,
                             sizeof(onOff));
    } else {
      powerOnOff(onOff);
    }
  }
}

void emberZclGetDefaultReportingConfigurationCallback(EmberZclEndpointId_t endpointId,
                                                      const EmberZclClusterSpec_t *clusterSpec,
                                                      EmberZclReportingConfiguration_t *configuration)
{
  // We have reportable attributes in the On/Off cluster server
  if (emberZclAreClusterSpecsEqual(&emberZclClusterOnOffServerSpec,
                                   clusterSpec)) {
    configuration->minimumIntervalS =  1;
    configuration->maximumIntervalS = DEFAULT_ON_OFF_REPORTING_PERIOD_S;
  }
}

static void relayOnOff(uint8_t status)
{
  if (status) {
    GPIO_PinOutClear(P_RELAY_ON_PORT, P_RELAY_ON_PIN);
    GPIO_PinOutSet(P_RELAY_OFF_PORT, P_RELAY_OFF_PIN);
    halCommonDelayMilliseconds(RELAY_SWTICH_STATE_KEEP_TIME_MS);
    GPIO_PinOutClear(P_RELAY_ON_PORT, P_RELAY_ON_PIN);
    GPIO_PinOutClear(P_RELAY_OFF_PORT, P_RELAY_OFF_PIN);
  } else {
    GPIO_PinOutClear(P_RELAY_OFF_PORT, P_RELAY_OFF_PIN);
    GPIO_PinOutSet(P_RELAY_ON_PORT, P_RELAY_ON_PIN);
    halCommonDelayMilliseconds(RELAY_SWTICH_STATE_KEEP_TIME_MS);
    GPIO_PinOutClear(P_RELAY_ON_PORT, P_RELAY_ON_PIN);
    GPIO_PinOutClear(P_RELAY_OFF_PORT, P_RELAY_OFF_PIN);
  }
  return;
}

static void powerOnOff(uint8_t status)
{
  static bool powerState;
  EmberZclStatus_t writeStatus;
  EmberZclEndpointId_t endpoint;
  EmberZclEndpointIndex_t i;

  if (status == POWER_STATE_TOGGLE) {
    powerState ^= 1;
  } else {
    if ((uint8_t)powerState == status) {
      return;
    } else {
      powerState = status;
    }
  }
  relayOnOff(powerState);
  if (!(READBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN))) {
    if (powerState) {
      halMultiLedBlinkPattern(1,
                              2,
                              powerOnBlinkPattern,
                              POWER_LED);
    } else {
      halMultiLedBlinkPattern(1,
                              2,
                              powerOffBlinkPattern,
                              POWER_LED);
    }
  }

  for (i = 0; i < emZclEndpointCount; i++) {
    endpoint = emberZclEndpointIndexToId(i,
                                         &emberZclClusterOnOffServerSpec);
    if (endpoint != EMBER_ZCL_ENDPOINT_NULL) {
      writeStatus
        = emberZclWriteAttribute(endpoint,
                                 &emberZclClusterOnOffServerSpec,
                                 EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                                 (bool *) &powerState,
                                 sizeof(bool));
      if (writeStatus != EMBER_ZCL_STATUS_SUCCESS) {
        emberAfCorePrintln("Smart Outlet failed to write value 0x%x to onOff "
                           "attribute of endpoint %d",
                           powerState,
                           endpoint);
      }
    }
  }
}

static void powerExceptionHelper(void)
{
  if (READBITS(powerExceptionMask, POWER_EXCEPTION_OVER_HEAT_MASK)) {
    halMultiLedBlinkPattern(DEFAULT_NUM_OVERHEAT_BLINKS,
                            2,
                            overHeatBlinkPattern,
                            POWER_LED);
  } else if (READBIT(powerExceptionMask,
                     POWER_EXCEPTION_BIT_CS5463_OVERCURRENT)) {
    halMultiLedBlinkPattern(DEFAULT_NUM_OVERCURRENT_BLINKS,
                            2,
                            overCurrentBlinkPattern,
                            POWER_LED);
  }
  if (READBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN)) {
    powerOnOff(POWER_STATE_OFF);
  }
  emberEventControlSetDelayMS(powerExceptionEventControl,
                              POWER_EXCEPTION_TIMEOUT_MS);
}

void powerExceptionEventHandler(void)
{
  emberEventControlSetInactive(powerExceptionEventControl);
  if (READBITS(powerExceptionMask, POWER_EXCEPTION_OVER_HEAT_MASK)) {
    halMultiLedBlinkPattern(DEFAULT_NUM_OVERHEAT_BLINKS,
                            2,
                            overHeatBlinkPattern,
                            POWER_LED);
    emberEventControlSetDelayMS(powerExceptionEventControl,
                                POWER_EXCEPTION_TIMEOUT_MS);
  } else if (READBIT(powerExceptionMask,
                     POWER_EXCEPTION_BIT_CS5463_OVERCURRENT)) {
    halMultiLedBlinkPattern(DEFAULT_NUM_OVERCURRENT_BLINKS,
                            2,
                            overCurrentBlinkPattern,
                            POWER_LED);
    emberEventControlSetDelayMS(powerExceptionEventControl,
                                POWER_EXCEPTION_TIMEOUT_MS);
  } else {
    CLEARBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN);
  }
}

/** @brief Over Current Callback
 *
 * This function is called upon the status change of over current condition.
 *
 * @param status OVER_CURRENT_TO_NORMAL: changed from over current to normal;
 *               NORMAL_TO_OVER_CURRENT: over current occured.
 *
 */
void halPowerMeterOverCurrentStatusChangeCallback(uint8_t status)
{
  if (status == CS5463_NORMAL_TO_OVER_CURRENT) { // over current happened
    emberAfAppPrintln("Over Current!!\r\n");
    SETBIT(powerExceptionMask, POWER_EXCEPTION_BIT_CS5463_OVERCURRENT);
    SETBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN);
    powerExceptionHelper();
  } else { // over current gone
    emberAfAppPrintln("Over Current Lifted!!\r\n");
    powerExceptionHelper();
    CLEARBIT(powerExceptionMask, POWER_EXCEPTION_BIT_CS5463_OVERCURRENT);
  }
}

/** @brief Over Heat Callback
 *
 * This function is called upon the status change of over heat condition.
 *
 * @param status  OVER_HEAT_TO_NORMAL: changed from over heat to normal;
 *                NORMAL_TO_OVER_CURRENT: over heat occured.
 *
 */
void halPowerMeterOverHeatStatusChangeCallback(uint8_t status)
{
  if (status == CS5463_NORMAL_TO_OVER_HEAT) { // over heat happened
    emberAfAppPrintln("CS5463 Over Heat!!\r\n");
    SETBIT(powerExceptionMask, POWER_EXCEPTION_BIT_CS5463_OVERHEAT);
    SETBIT(powerExceptionMask, POWER_EXCEPTION_BIT_POWERSHUTDOWN);
    powerExceptionHelper();
  } else {
    emberAfAppPrintln("CS5463 Over Heat Lifted!!\r\n");
    powerExceptionHelper();
    CLEARBIT(powerExceptionMask, POWER_EXCEPTION_BIT_CS5463_OVERHEAT);
  }
}

// If an occupancy sensor is bound to the outlet, then write the onOff attribute
// based on occupancy sensor reports
void emberZclNotificationCallback(const EmberZclNotificationContext_t *context,
                                  const EmberZclClusterSpec_t *clusterSpec,
                                  EmberZclAttributeId_t attributeId,
                                  const void *buffer,
                                  size_t bufferLength)
{
  // If this notification is about a new occupancy state from a sensor bound to
  // the outlet, update the onOff endpoint to either on (when room is occupied)
  // or off (when not occupied)
  if ((emberZclCompareClusterSpec(&emberZclClusterOccupancySensingServerSpec,
                                  clusterSpec) == 0)
      && attributeId == EMBER_ZCL_CLUSTER_OCCUPANCY_SENSING_SERVER_ATTRIBUTE_OCCUPANCY) {
    bool onOff;
    EmberZclEndpointId_t endpointId = context->endpointId;
    EmberZclStatus_t status;
    uint8_t occupancyState = *((uint8_t *)buffer);

    if (occupancyState) {
      onOff = true;
      emberAfCorePrintln("bound occupancy sensor reports room is occupied");
    } else {
      onOff = false;
      emberAfCorePrintln("bound occupancy sensor reports room is empty");
    }

    status = emberZclWriteAttribute(endpointId,
                                    &emberZclClusterOnOffServerSpec,
                                    EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                                    &onOff,
                                    sizeof(onOff));

    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      emberAfCorePrintln("Failed to update ep %d based on occupancy report",
                         endpointId);
    }
  }
}

void emberAfInitCallback(void)
{
  GPIO_PinModeSet(P_RELAY_ON_PORT, P_RELAY_ON_PIN, gpioModePushPull, 0);
  GPIO_PinModeSet(P_RELAY_OFF_PORT, P_RELAY_OFF_PIN, gpioModePushPull, 0);
  halMultiLedBlinkSetActivityLeds(POWER_LED);
  halMultiLedBlinkSetActivityLeds(STATUS_LED);
  powerExceptionMask = 0;
  halMultiLedBlinkPattern(1,
                          2,
                          powerOffBlinkPattern,
                          POWER_LED);
}
