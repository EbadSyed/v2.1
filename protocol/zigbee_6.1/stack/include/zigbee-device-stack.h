/**
 * @file zigbee-device-stack.h
 * @brief ZigBee Device Object (ZDO) functions included in the stack.
 *
 * See @ref zdo for documentation.
 *
 * <!--Copyright 2005 by Ember Corporation. All rights reserved.         *80*-->
 */

#ifndef SILABS_ZIGBEE_DEVICE_STACK_H
#define SILABS_ZIGBEE_DEVICE_STACK_H

/**
 * @addtogroup zdo
 *
 * See zigbee-device-stack.h for source code.
 * @{
 */

/** @brief Request the 16 bit network address of a node whose EUI64 is known.
 *
 * @param target           The EUI64 of the node.
 * @param reportKids       true to request that the target list their children
 *                         in the response.
 * @param childStartIndex  The index of the first child to list in the response.
 *                         Ignored if @c reportKids is false.
 *
 * @return An ::EmberStatus value.
 * - ::EMBER_SUCCESS - The request was transmitted successfully.
 * - ::EMBER_NO_BUFFERS - Insuffient message buffers were available to construct
 * the request.
 * - ::EMBER_NETWORK_DOWN - The node is not part of a network.
 * - ::EMBER_NETWORK_BUSY - Transmission of the request failed.
 */
EmberStatus emberNetworkAddressRequest(EmberEUI64 target,
                                       bool reportKids,
                                       uint8_t childStartIndex);

/** @brief Request the EUI64 of a node whose 16 bit network address is known.
 *
 * @param target           The network address of the node.
 * @param reportKids       true to request that the target list their children
 *                         in the response.
 * @param childStartIndex  The index of the first child to list in the response.
 *                         Ignored if reportKids is false.
 * @param options          The options to use when sending the request.
 *                         See ::emberSendUnicast() for a description.
 *
 * @return An ::EmberStatus value.
 * - ::EMBER_SUCCESS
 * - ::EMBER_NO_BUFFERS
 * - ::EMBER_NETWORK_DOWN
 * - ::EMBER_NETWORK_BUSY
 */
EmberStatus emberIeeeAddressRequest(EmberNodeId target,
                                    bool reportKids,
                                    uint8_t childStartIndex,
                                    EmberApsOption options);

/** @brief Request that an energy scan be performed and its results returned.
 *  This request may only be sent by the current network manager and must be
 *  unicast, not broadcast.
 *
 * @param target           The network address of the node to perform the scan.
 * @param scanChannels     A mask of the channels to be scanned.
 * @param scanDuration     How long to scan on each channel.  Allowed
 *                         values are 0..5, with the scan times as specified
 *                         by 802.15.4 (0 = 31ms, 1 = 46ms, 2 = 77 ms,
 *                         3 = 138ms, 4 = 261ms, 5 = 507ms).
 * @param scanCount        The number of scans to be performed on each
 *                         channel (1 .. 8).
 *
 * @return An ::EmberStatus value.
 * - ::EMBER_SUCCESS
 * - ::EMBER_NO_BUFFERS
 * - ::EMBER_NETWORK_DOWN
 * - ::EMBER_NETWORK_BUSY
 */
EmberStatus emberEnergyScanRequest(EmberNodeId target,
                                   uint32_t scanChannels,
                                   uint8_t  scanDuration,
                                   uint16_t scanCount);

/** @brief Broadcasts a request to set the identity of the network manager and
 * the active channel mask.  The mask is used when scanning
 *  for the network after missing a channel update.
 *
 * @param networkManager   The network address of the network manager.
 * @param activeChannels   The new active channel mask.
 *
 * @return An ::EmberStatus value.
 * - ::EMBER_SUCCESS
 * - ::EMBER_NO_BUFFERS
 * - ::EMBER_NETWORK_DOWN
 * - ::EMBER_NETWORK_BUSY
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
EmberStatus emberSetNetworkManagerRequest(EmberNodeId networkManager,
                                          uint32_t activeChannels);
#else
#define emberSetNetworkManagerRequest(manager, channels)  \
  (emberEnergyScanRequest(EMBER_SLEEPY_BROADCAST_ADDRESS, \
                          (channels),                     \
                          0xFF,                           \
                          (manager)))
#endif

/** @brief Broadcasts a request to change the channel.  This request may
 * only be sent by the current network manager.  There is a delay of
 * several seconds from receipt of the broadcast to changing the channel,
 * to allow time for the broadcast to propagate.
 *
 * @param channel  The channel to change to.
 *
 * @return An ::EmberStatus value.
 * - ::EMBER_SUCCESS
 * - ::EMBER_NO_BUFFERS
 * - ::EMBER_NETWORK_DOWN
 * - ::EMBER_NETWORK_BUSY
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
EmberStatus emberChannelChangeRequest(uint8_t channel);
#else
#define emberChannelChangeRequest(channel)                \
  (emberEnergyScanRequest(EMBER_SLEEPY_BROADCAST_ADDRESS, \
                          BIT32(channel),                 \
                          0xFE,                           \
                          0))
#endif

/** @brief Sends a broadcast for a ZDO Device announcement.  Normally
 *    it is NOT required to call this as the stack automatically sends
 *    a device announcement during joining or rejoining, as per the spec.
 *    However if the device wishes to re-send its device announcement
 *    they can use this call.
 *
 *    @return An ::EmberStatus value.
 *    - ::EMBER_SUCCESS
 *    - ::EMBER_INVALID_CALL
 */
EmberStatus emberSendDeviceAnnouncement(void);

/** @brief Sends a broadcast for a ZDO Parent Announcement.  Normally
 *    it is NOT required to call this as the stack automatically sends
 *    a Parent Announce when a Zigbee Router/Coordinator reboots, is in
 *    a joined or authenticated state and has atleast one device.
 *    However if the device wishes to re-send its Parent announcement
 *    they can use this call.
 *
 *    @return An ::EmberStatus value.
 *    - ::EMBER_SUCCESS
 *    - ::EMBER_INVALID_CALL
 */
EmberStatus emberSendParentAnnouncement(void);

/**
 * @brief Provide access to the stack ZDO transaction sequence number for
 * last request.
 *
 * @return  Last stack ZDO transaction sequence number used
 */
uint8_t emberGetLastStackZigDevRequestSequence(void);

/** @} END addtogroup */

#endif  // SILABS_ZIGBEE_DEVICE_STACK_H
