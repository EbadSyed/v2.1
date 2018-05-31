// Copyright 2015 Silicon Laboratories, Inc.

#ifndef SILABS_NETWORK_CREATOR_H
#define SILABS_NETWORK_CREATOR_H

// -----------------------------------------------------------------------------
// Constants

#define EMBER_AF_PLUGIN_NETWORK_CREATOR_PLUGIN_NAME "NWK Creator"

// -----------------------------------------------------------------------------
// Globals

extern uint32_t emAfPluginNetworkCreatorPrimaryChannelMask;
extern uint32_t emAfPluginNetworkCreatorSecondaryChannelMask;

// -----------------------------------------------------------------------------
// API

/** @brief Start
 *
 *  Commands the network creator to form a network with the following qualities.
 *
 *  @param centralizedNetwork Whether or not to form a network usingn
 *  centralized security. If this argument is false, then a network with
 *  distributed security will be formed.
 *
 *  @return Status of the commencement of the network creator process.
 */
EmberStatus emberAfPluginNetworkCreatorStart(bool centralizedNetwork);

/** @brief Stop
 *
 * Stops the network creator formation process.
 */
void emberAfPluginNetworkCreatorStop(void);

/** @brief Start
 *
 *  Commands the network creator to form a centralized or distributed
 *  network with specified pan ID, tx power and channel
 *
 *  @param centralizedNetwork Whether or not to form a network usingn
 *  centralized security. If this argument is false, then a network with
 *  distributed security will be formed.
 *
 *  @param panId, the pan ID of the network to be formed.
 *  @param radioTxPower, the tx power of the network to be formed.
 *  @param channel, channel of the network to be formed.
 *
 *  @return Status of the commencement of the network creator process.
 */
EmberStatus emberAfPluginNetworkCreatorNetworkForm(bool centralizedNetwork,
                                                   EmberPanId panId,
                                                   int8_t radioTxPower,
                                                   uint8_t channel);
#endif /* __NETWORK_CREATOR_H__ */
