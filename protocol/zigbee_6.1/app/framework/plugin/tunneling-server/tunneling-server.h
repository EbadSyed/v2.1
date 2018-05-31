// *******************************************************************
// * tunneling-server.h
// *
// *
// * Copyright 2010 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************

#define ZCL_TUNNELING_CLUSTER_INVALID_TUNNEL_ID        0xFFFF
#define ZCL_TUNNELING_CLUSTER_UNUSED_MANUFACTURER_CODE 0xFFFF
#define CLOSE_INITIATED_BY_CLIENT true
#define CLOSE_INITIATED_BY_SERVER false

/**
 * @brief Transfer data to a client through a Tunneling cluster tunnel.
 *
 * This function can be used to transfer data to a client through a tunnel. The
 * Tunneling server plugin will send the data to the endpoint on the node that
 * opened the given tunnel.
 *
 * @param tunnelIndex The identifier of the tunnel through which to send the data.
 * @param data Buffer containing the raw octets of the data.
 * @param dataLen The length in octets of the data.
 * @return ::EMBER_ZCL_STATUS_SUCCESS if the data was sent,
 * ::EMBER_ZCL_STATUS_FAILURE if an error occurred, or
 * ::EMBER_ZCL_STATUS_NOT_FOUND if the tunnel does not exist.
 */
EmberAfStatus emberAfPluginTunnelingServerTransferData(uint16_t tunnelIndex,
                                                       uint8_t *data,
                                                       uint16_t dataLen);

/**
 * @brief Toggle a "server busy" status for running as a test harness
 *
 * This function can be used to set the server in to a busy state, where it
 * will respond to all request tunnel commands with a busy status.  NOTE:
 * existing tunnels will continue to operate as normal at this point in time.
 */
void emberAfPluginTunnelingServerToggleBusyCommand(void);

/**
 * @brief Cleanup a Tunneling cluster tunnel.
 *
 * This function can be used to cleanup all state associated with a tunnel.
 * The Tunneling server plugin will not send the close notification command.
 *
 * @param tunnelId The identifier of the tunnel to cleanup.
 */
void emberAfPluginTunnelingServerCleanup(uint8_t tunnelId);

void emAfPluginTunnelingServerPrint(void);
