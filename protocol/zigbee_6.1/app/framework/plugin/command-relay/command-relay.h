// Copyright 2016 Silicon Laboratories, Inc.                                *80*

#ifndef SILABS_COMMAND_RELAY_H
#define SILABS_COMMAND_RELAY_H

typedef struct {
  EmberEUI64 eui64;
  uint8_t endpoint;
  uint16_t clusterId;
} EmberAfPluginCommandRelayDeviceEndpoint;

typedef struct {
  EmberAfPluginCommandRelayDeviceEndpoint inDeviceEndpoint;
  EmberAfPluginCommandRelayDeviceEndpoint outDeviceEndpoint;
} EmberAfPluginCommandRelayEntry;

/** @brief Add a command relay item
 *
 * Add a command relay rule that will forward the command that matches eui,
 * endpoint and cluster in the inDeviceEndpoint struct to where the
 * outDeviceEndpoint defines.
 *
 * @param inDeviceEndpoint eui64, endpoint and cluster that will be matched
 * with incoming commands.
 *
 * @param outDeviceEndpoint eui64, endpoint and cluster that the command will
 * be forwarded.
 */
void emberAfPluginCommandRelayAdd(
  EmberAfPluginCommandRelayDeviceEndpoint* inDeviceEndpoint,
  EmberAfPluginCommandRelayDeviceEndpoint* outDeviceEndpoint);

/** @brief Remove a command relay item
 *
 * Remove the rule item that matches inDeviceEndpoint and outDeviceEndpoint.
 *
 * @param inDeviceEndpoint in endpoint structure
 *
 * @param outDeviceEndpoint out endpoint structure
 */
void emberAfPluginCommandRelayRemove(
  EmberAfPluginCommandRelayDeviceEndpoint* inDeviceEndpoint,
  EmberAfPluginCommandRelayDeviceEndpoint* outDeviceEndpoint);

/** @brief Save command relay rules in a backup file
 *
 * Command relay rules will be saved in backup file in the host once this is
 * called.
 *
 */
void emberAfPluginCommandRelaySave(void);

/** @brief Load command relay rules from a backup file
 *
 * Command relay rules will be loaded from backup file in the host once this is
 * called.
 *
 */
void emberAfPluginCommandRelayLoad(void);

/** @brief Clear command relay rules
 *
 * All command relay rules will be clear, including the backup file.
 *
 */
void emberAfPluginCommandRelayClear(void);

/** @brief Pointer to the relay table structure
 *
 * For some MQTT messages, it is more convenient to use the relay table data
 * structure directly.  Use this API to get a pointer to the relay table
 * structure.
 *
 * @out  Pointer to the address table structure
 */
EmberAfPluginCommandRelayEntry* emberAfPluginCommandRelayTablePointer(void);

#endif //__COMMAND_RELAY_H
