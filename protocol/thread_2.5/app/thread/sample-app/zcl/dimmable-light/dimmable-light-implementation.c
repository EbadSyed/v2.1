// Copyright 2017 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_CONNECTION_MANAGER
#include EMBER_AF_API_HAL
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE

#define DEFAULT_REPORTABLE_LEVEL_CHANGE   10

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

void emberZclGetDefaultReportingConfigurationCallback(EmberZclEndpointId_t endpointId,
                                                      const EmberZclClusterSpec_t *clusterSpec,
                                                      EmberZclReportingConfiguration_t *configuration)
{
  // We have reportable attributes in the On/Off and Level Control servers.

  if (emberZclAreClusterSpecsEqual(&emberZclClusterOnOffServerSpec,
                                   clusterSpec)) {
    configuration->minimumIntervalS =  1;
    configuration->maximumIntervalS = 60;
  } else if (emberZclAreClusterSpecsEqual(&emberZclClusterLevelControlServerSpec,
                                          clusterSpec)) {
    configuration->minimumIntervalS = 10;
    configuration->maximumIntervalS = 60;
  }
}

bool emberAfPluginIdleSleepOkToSleepCallback(uint32_t durationMs)
{
  // The device can be a router, so it should never sleep.

  return false;
}

void emberZclGetDefaultReportableChangeCallback(EmberZclEndpointId_t endpointId,
                                                const EmberZclClusterSpec_t *clusterSpec,
                                                EmberZclAttributeId_t attributeId,
                                                void *buffer,
                                                size_t bufferLength)
{
  int8u level = DEFAULT_REPORTABLE_LEVEL_CHANGE;

  if (emberZclAreClusterSpecsEqual(&emberZclClusterLevelControlServerSpec, clusterSpec)
      && (attributeId == EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL)) {
    *(int8u *)buffer = level;
  }
}

// If an occupancy sensor is bound to the light, then write the onOff attribute
// based on occupancy sensor reports
void emberZclNotificationCallback(const EmberZclNotificationContext_t *context,
                                  const EmberZclClusterSpec_t *clusterSpec,
                                  EmberZclAttributeId_t attributeId,
                                  const void *buffer,
                                  size_t bufferLength)
{
  // If this notification is about a new occupancy state from a sensor bound to
  // this light, update the onOff endpoint to either on (when room is occupied)
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
