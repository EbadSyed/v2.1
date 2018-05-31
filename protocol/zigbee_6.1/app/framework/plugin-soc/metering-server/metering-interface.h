// Copyright 2015 Silicon Laboratories, Inc.                                *80*

enum {
  EMBER_AF_PLUGIN_METERING_SERVER_DISABLE_PROFILING   = 0,
  EMBER_AF_PLUGIN_METERING_SERVER_ENABLE_PROFILING    = 1,
  EMBER_AF_PLUGIN_METERING_SERVER_CLEAR_PROFILES      = 2,
};

typedef uint8_t EmberAfPluginMeteringServerProfileState;

/** @brief tick function for metering server tick counter
 *
 * This function will be called to every secound for the
 * metering server to calculate the energy consumed by the
 * metering device.
 *
 * @param endpoint Endpoint that is affected.
 */
void emberAfPluginMeteringServerInterfaceTick(uint8_t endpoint);

/** @brief init function of the metering interface
 *
 * This function initialized all parameters and attributes
 * needed for metering server
 *
 * @param endpoint Endpoint that is affected.
 */
void emberAfPluginMeteringServerInterfaceInit(uint8_t endpoint);

/** @brief enable profiling of metering server
 *
 * This function will enable and disable profiling of metering server
 *
 * @param profileState
 *        EMBER_AF_PLUGIN_METERING_SERVER_DISABLE_PROFILING: disable profiling,
 *        EMBER_AF_PLUGIN_METERING_SERVER_ENABLE_PROFILING: enable profiling,
 *        EMBER_AF_PLUGIN_METERING_SERVER_CLEAR_PROFILES: clear all profiles,
 *
 * @return false: if number of profiles is 0,
 *         true: if number of profiles larger than 0 and it is set
 *               by the parameter profileState.
 */
bool emberAfPluginMeteringServerInterfaceSetProfiles(
  EmberAfPluginMeteringServerProfileState profileState);

/** @brief initialize function of metering server attributes
 *
 * This function will initialize all attributes of simple metering server
 *
 * @param endpoint Endpoint that is affected.
 */
void emberAfPluginMeteringServerAttributeInit(uint8_t endpoint);

/** @brief to get profiles
 *
 * This function is called upon receiving of the GetProfile command for the
 * simple metering client and will get the profiles and response to the client
 *
 * @param intervalChannel interval channel to get
 *
 * @param endTime requested end time
 *
 * @param numberOfPeriods number of periods requested
 */
bool emberAfPluginMeteringServerInterfaceGetProfiles(
  uint8_t intervalChannel,
  uint32_t endTime,
  uint8_t numberOfPeriods);
