// Copyright 2017 Silicon Laboratories, Inc.

#ifndef __CONNECTION_MANAGER_H__
#define __CONNECTION_MANAGER_H__

typedef enum {
  EMBER_CONNECTION_MANAGER_STATUS_CONNECTED = 0x00,
  EMBER_CONNECTION_MANAGER_STATUS_TIMED_OUT = 0x01,
  EMBER_CONNECTION_MANAGER_STATUS_NO_KEY    = 0X02
} EmberConnectionManagerConnectionStatus;

/** @brief Connection attempt completed.
 *
 * This function is called when an attempt to connect to a network has
 * completed.  It will convery the result of an attempt to join a network using
 * the emberConnectionManagerStartConnect function.  The status will be one of
 * the following values:
 *
 * EMBER_CONNECTION_MANAGER_STATUS_CONNECTED: The device succesfully attached to
 * the network.
 * EMBER_CONNECTION_MANAGER_STATUS_TIMED_OUT: The device was unable to join the
 * network after attempting the number of times specified in the connection
 * manager plugin options.
 */
void emberConnectionManagerConnectCompleteCallback(EmberConnectionManagerConnectionStatus status);

/** @brief Start trying to connect to a network.
 *
 * This function is used to start trying to connect to a network.  If the device
 * is an end device, it will continue to attempt to connect to a network until
 * it exceeds the bounds set in the plugin options.  If the device is a routing
 * routing capable device on an out of band network, it will attempt to join an
 * existing network with parameters matching those set in the plugin options,
 * forming a new network with those parameters if it doesn't find one.  If the
 * routing capable device is attempting to join an in band commissioned network,
 * it will attempt to connect until it exceeds the bounds set in the plugin
 * options.
 */
void emberConnectionManagerStartConnect(void);

/** @brief Stop trying to connect to a network
 *
 * This function is used to stop an ongoing connection process.  It is useful
 * when the connection manager has been configured with no connection timeout
 * as a means of halting connection attempt to save batteries or decrease
 * network traffic.
 */
void emberConnectionManagerStopConnect(void);

/** @brief Leave an existing network
 *
 * This function can be used to leave an existing network.  It is similar in
 * effect to the emberResetNetworkState function, but also updates internal
 * state variables of the connection manager.  As such, this function should be
 * used in favor of emberResetNetworkState whenever the connection manager is
 * used to handle network state transitions.
 *
 */
void emberConnectionManagerLeaveNetwork(void);

/** @brief Search for a parent
 *
 * This function can be used to manually get an orphaned device to search for a
 * new parent router.  If the device is already connected to a network, calling
 * this function will have no effect.  If the device is in an orphaned state,
 * this function will bypass the normal delay between orphan rejoin attempts and
 * immediately search for a new parent.
 */
void emberConnectionManagerSearchForParent(void);

/** @brief Return whether the device is currently orphaned
 *
 * The return value of this function can be used to quickly determine whether
 * the device is currently in an orphaned state.
 */
bool emberConnectionmanagerIsOrphaned(void);

/** @brief Return whether the device is searching for network
 *
 * The return value of this function can be used to quickly determine whether
 * the device is currently searching for a network.
 */
bool emberConnectionManagerIsSearching(void);

/** @brief Return whether the connection state is ready for long polling
 *
 * The return value of this function can be used to quickly determine whether
 * the device is in a state to allow long polling. After a device connects it
 * should short poll for 60 seconds to allow netowrk data to make it to the
 * device. This is only meant to be used with sleepy devices.
 */
bool emberConnectionManagerIsOkToLongPoll(void);

#endif //__CONNECTION_MANAGER_H__
