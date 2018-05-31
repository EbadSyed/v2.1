// *******************************************************************
// * connection-manager.h
// *
// * Implements code to maintain a network connection.  It will implement rejoin
// * algorithms and perform activity LED blinking as required.
// *
// * Copyright 2015 Silicon Laboratories, Inc.                              *80*
// *******************************************************************
//-----------------------------------------------------------------------------
#ifndef SILABS_CONNECTION_MANAGER_H
#define SILABS_CONNECTION_MANAGER_H

//------------------------------------------------------------------------------
// Plugin public function declarations

/** @brief Reset the join attempt counter.
 *
 * This function will reset the internal counter that the connection manager
 * plugin uses to track how many attempts it has made to join a network.  This
 * function can be used to delay the call to
 * @emberAfPluginConnectionManagerFinishedCallback, which normally occurs (with a
 * status of EMBER_NOT_JOINED) after 20 failed join attempts.
 */
void emberAfPluginConnectionManagerResetJoinAttempts(void);

/** @brief Leave the current network and attempt to join a new one
 *
 * This function will cause the plugin to leave the current network or
 * begin searching for a new network to join if it's not currently on
 * a network.
 */
void emberAfPluginConnectionManagerLeaveNetworkAndStartSearchForNewOne(void);

/** @brief Begin searching for a new network to join
 *
 * This function will attempt to join a new network.  It tracks the number
 * of network join attempts that have occurred, and will generate a call to
 * @emberAfPluginConnectionManagerStartSearchForJoinableNetwork with a status of
 * EMBER_NOT_JOINED if a network can not be found within 20 join attempts.  This
 * function will also make sure that a new join attempt occurs 20 seconds after
 * an unsuccessful join attempt occurs (until it encounters 20 failed join
 * attempts).
 */
void emberAfPluginConnectionManagerStartSearchForJoinableNetwork(void);

/** @brief Perform a factory reset.
 *
 * This function will lear all binding, scene, and group tables.  It does not
 * cause a change in network state.
 */
void emberAfPluginConnectionManagerFactoryReset(void);

/** @brief Set the LED behavior for a network join event
 *
 * This function will configure the connection manager plugin to blink the
 * network activity LED a user specified number of times when a successful
 * network join event occurs.
 *
 * @param numBlinks  The number of times to blink the LED on network join
 */
void emberAfPluginConnectionManagerSetNumberJoinBlink(uint8_t numBlinks);

/** @brief Set the LED behavior for a network leave event
 *
 * This function will configure the connection manager plugin to blink the
 * network activity LED a user specified number of times when a network leave
 * event occurs.
 *
 * @param numBlinks  The number of times to blink the LED on network leave
 */
void emberAfPluginConnectionManagerSetNumberLeaveBlink(uint8_t numBlinks);

/** @brief Blink the Network Found LED pattern
 *
 * This function will blink the network found LED pattern
 *
 */
void emberAfPluginConnectionManagerLedNetworkFoundBlink(void);

#endif //__CONNECTION_MANAGER_H__
