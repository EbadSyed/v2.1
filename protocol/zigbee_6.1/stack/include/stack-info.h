/**
 * @file stack-info.h
 * @brief EmberZNet API for accessing and setting stack information.
 * See @ref stack_info for documentation.
 *
 * <!--Copyright 2004-2007 by Ember Corporation. All rights reserved.    *80*-->
 */

#ifndef SILABS_STACK_INFO_H
#define SILABS_STACK_INFO_H

/**
 * @addtogroup stack_info
 *
 * See stack-info.h for source code.
 * @{
 */

/** @brief A callback invoked when the status of the stack changes.
 * If the
 * status parameter equals ::EMBER_NETWORK_UP, then the
 * ::emberGetNetworkParameters()
 * function can be called to obtain the new network parameters. If any of the
 * parameters are being stored in nonvolatile memory by the application, the
 * stored values should be updated.
 *
 * The application is free to begin messaging once it receives the
 * ::EMBER_NETWORK_UP status.  However, routes discovered immediately after
 * the stack comes up may be suboptimal.  This is because the routes are based
 * on the neighbor table's information about two-way links with neighboring nodes,
 * which is obtained from periodic ZigBee Link Status messages.  It can take
 * two or three link status exchange periods (of 16 seconds each) before the
 * neighbor table has a good estimate of link quality to neighboring nodes.
 * Therefore, the application may improve the quality of initially discovered
 * routes by waiting after startup to give the neighbor table time
 * to be populated.
 *
 * @param status  Stack status. One of the following:
 * - ::EMBER_NETWORK_UP
 * - ::EMBER_NETWORK_DOWN
 * - ::EMBER_JOIN_FAILED
 * - ::EMBER_MOVE_FAILED
 * - ::EMBER_CANNOT_JOIN_AS_ROUTER
 * - ::EMBER_NODE_ID_CHANGED
 * - ::EMBER_PAN_ID_CHANGED
 * - ::EMBER_CHANNEL_CHANGED
 * - ::EMBER_NO_BEACONS
 * - ::EMBER_RECEIVED_KEY_IN_THE_CLEAR
 * - ::EMBER_NO_NETWORK_KEY_RECEIVED
 * - ::EMBER_NO_LINK_KEY_RECEIVED
 * - ::EMBER_PRECONFIGURED_KEY_REQUIRED
 */
void emberStackStatusHandler(EmberStatus status);

/** @brief Returns the current join status.
 *
 *   Returns a value indicating whether the node is joining,
 *   joined to, or leaving a network.
 *
 * @return An ::EmberNetworkStatus value indicating the current join status.
 */
EmberNetworkStatus emberNetworkState(void);

/** @brief Indicates whether the stack is currently up.
 *
 *   Returns true if the stack is joined to a network and
 *   ready to send and receive messages.  This reflects only the state
 *   of the local node; it does not indicate whether or not other nodes are
 *   able to communicate with this node.
 *
 * @return True if the stack is up, false otherwise.
 */
bool emberStackIsUp(void);

/** @brief Determines whether \c eui64 is the local node's EUI64 ID.
 *
 * @param bool Erase the node type or not
 *
 * @return the status of the operation and an error code if unsuccessful
 */
EmberStatus emberWriteNodeData(bool erase);

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief Returns the EUI64 ID of the local node.
 *
 * @return The 64-bit ID.
 */
EmberEUI64 emberGetEui64(void);

/** @brief Determines whether \c eui64 is the local node's EUI64 ID.
 *
 * @param eui64  An EUI64 ID.
 *
 * @return true if \c eui64 is the local node's ID, otherwise false.
 */
bool emberIsLocalEui64(EmberEUI64 eui64);

/** @brief Returns the 16-bit node ID of local node on the current logical
 * network.
 *
 * @return The 16-bit ID.
 */
EmberNodeId emberGetNodeId(void);

/** @brief Returns the 16-bit node ID of local node on the network it is
 * currently tuned on.
 *
 * @return The 16-bit ID.
 */
EmberNodeId emberRadioGetNodeId(void);

/** @brief Sets the manufacturer code to the specified value. The
 * manufacturer code is one of the fields of the node descriptor.
 *
 * @param code  The manufacturer code for the local node.
 */
void emberSetManufacturerCode(uint16_t code);

/** @brief Sets the power descriptor to the specified value. The power
 * descriptor is a dynamic value, therefore you should call this function
 * whenever the value changes.
 *
 * @param descriptor  The new power descriptor for the local node.
 */
void emberSetPowerDescriptor(uint16_t descriptor);

/** @brief Sets the maximum incoming transfer size to the specified value.
 * The maximum incoming transfer size is one of the fields of the node
 * descriptor.
 *
 * @param size  The maximum incoming transfer size for the local node.
 */
void emberSetMaximumIncomingTransferSize(uint16_t size);

/** @brief Sets the maximum outgoing transfer size to the specified value.
 * The maximum outgoing transfer size is one of the fields of the node
 * descriptor.
 *
 * @param size  The maximum outgoing transfer size for the local node.
 */
void emberSetMaximumOutgoingTransferSize(uint16_t size);

/** @brief Sets the descriptor capability field of the node.
 *
 * @param capability  The descriptor capability of the local node.
 */
void emberSetDescriptorCapability(uint8_t capability);

#else   // Doxygen ignores the following
extern EmberEUI64 emLocalEui64;
#define emberGetEui64() (emLocalEui64)
#define emberIsLocalEui64(eui64) \
  (MEMCOMPARE((eui64), emLocalEui64, EUI64_SIZE) == 0)

EmberNodeId emberGetNodeId(void);
EmberNodeId emberRadioGetNodeId(void);

extern uint16_t emManufacturerCode;
extern uint16_t emPowerDescriptor;
extern uint16_t emMaximumIncomingTransferSize;
extern uint16_t emMaximumOutgoingTransferSize;
extern uint8_t emDescriptorCapability;
#define emberSetManufacturerCode(code) \
  (emManufacturerCode = (code))
#define emberSetPowerDescriptor(descriptor) \
  (emPowerDescriptor = (descriptor))
#define emberSetMaximumIncomingTransferSize(size) \
  (emMaximumIncomingTransferSize = (size))
#define emberSetMaximumOutgoingTransferSize(size) \
  (emMaximumOutgoingTransferSize = (size))
#define emberSetDescriptorCapability(capability) \
  (emDescriptorCapability = (capability))
#endif

/** @brief Copies the current network parameters into the structure
 * provided by the caller.
 *
 * @param parameters  A pointer to an EmberNetworkParameters value
 *  into which the current network parameters will be copied.
 *
 * @return An ::EmberStatus value indicating the success or
 *  failure of the command.
 */
EmberStatus emberGetNetworkParameters(EmberNetworkParameters *parameters);

/** @brief Copies the current radio parameters into the structure
 * provided by the caller.
 *
 * @param phyIndex  Desired index of phy interface for radio parameters.
 *            - For 2.4 or Subghz only (switched) device, index will be ignored.
 *            - For simultaneous dual radio, returns radio parameters based on
 *              provided phy index.
 *  parameters  A pointer to an EmberMultiPhyRadioParameters value
 *  into which the current radio parameters will be copied.
 *
 * @return An ::EmberStatus value indicating the success or
 *  failure of the command.
 */
EmberStatus emberGetRadioParameters(uint8_t phyIndex, EmberMultiPhyRadioParameters *parameters);

/** @brief Copies the current node type into the location provided
 * by the caller.
 *
 * @param resultLocation  A pointer to an EmberNodeType value
 *  into which the current node type will be copied.
 *
 * @return An ::EmberStatus value that indicates the success or failure
 *  of the command.
 */
EmberStatus emberGetNodeType(EmberNodeType *resultLocation);

/** @brief Sets the channel to use for sending and receiving messages on the
 * current logical network.
 * For a list of available radio channels, see the technical specification for
 * the RF communication module in your Developer Kit.
 *
 * Note: Care should be taken when using this API,
 * as all devices on a network must use the same channel.
 *
 * @param channel  Desired radio channel.
 *
 * @return An ::EmberStatus value indicating the success or
 *  failure of the command.
 */
EmberStatus emberSetRadioChannel(uint8_t channel);

/** @brief Gets the radio channel to which a node is set on the current
 * logical network. The possible return values depend on the radio in use.
 * For a list of available radio channels, see the technical specification
 * for the RF communication module in your Developer Kit.
 *
 * @return Current radio channel.
 */
uint8_t emberGetRadioChannel(void);

/** @brief Sets the radio output power at which a node is to operate for the
 * current logical network. Ember radios have discrete power settings. For a
 * list of available power settings, see the technical specification for the RF
 * communication module in your Developer Kit.
 * Note: Care should be taken when using this API on a running network,
 * as it will directly impact the established link qualities neighboring nodes
 * have with the node on which it is called.  This can lead to disruption of
 * existing routes and erratic network behavior.
 * Note: If the requested power level is not available on a given radio, this
 * function will use the next higher available power level.
 *
 * @param power  Desired radio output power, in dBm.
 *
 * @return An ::EmberStatus value indicating the success or
 *  failure of the command.  Failure indicates that the requested power level
 *  is out of range.
 */
EmberStatus emberSetRadioPower(int8_t power);

/** @brief Gets the radio output power of the current logical network at which
 * a node is operating. Ember radios have discrete power settings. For a list of
 * available power settings, see the technical specification for the RF
 * communication module in your Developer Kit.
 *
 * @return Current radio output power, in dBm.
 */
int8_t emberGetRadioPower(void);

/** @brief Returns the local node's PAN ID of the current logical network.
 *
 * @return A PAN ID.
 */
EmberPanId emberGetPanId(void);

/** @brief Returns the local node's PAN ID of the current radio network.
 *
 * @return A PAN ID.
 */
EmberPanId emberRadioGetPanId(void);

/** @brief Fetches a node's 8 byte Extended PAN identifier. If this is called
 *  when a device is not currently on a network (see ::emberNetworkState),
 *  then the Extended PAN ID returned will be an invalid value.
 */
void emberGetExtendedPanId(uint8_t *resultLocation);

/** @brief The application must provide a definition for this variable. */
extern PGM uint8_t emberStackProfileId[];

/** @brief Endpoint information (a ZigBee Simple Descriptor).
 *
 * This is a ZigBee Simple Descriptor and contains information
 * about an endpoint.  This information is shared with other nodes in the
 * network by the ZDO.
 */

typedef struct {
  /** Identifies the endpoint's application profile. */
  uint16_t profileId;
  /** The endpoint's device ID within the application profile. */
  uint16_t deviceId;
  /** The endpoint's device version. */
  uint8_t deviceVersion;
  /** The number of input clusters. */
  uint8_t inputClusterCount;
  /** The number of output clusters. */
  uint8_t outputClusterCount;
} EmberEndpointDescription;

/** @brief Gives the endpoint information for a particular endpoint.
 */

typedef struct {
  /** An endpoint of the application on this node. */
  uint8_t endpoint;
  /** The endpoint's description. */
  EmberEndpointDescription PGM * description;
  /** Input clusters the endpoint will accept. */
  uint16_t PGM * inputClusterList;
  /** Output clusters the endpoint may send. */
  uint16_t PGM * outputClusterList;
} EmberEndpoint;

/** @brief The application must provide a definition for this variable. */
extern uint8_t emberEndpointCount;

/** @brief If emberEndpointCount is nonzero, the application must
 * provide descriptions for each endpoint.
 *
 * This can be done either
 * by providing a definition of emberEndpoints or by providing definitions
 * of ::emberGetEndpoint(), ::emberGetEndpointDescription() and
 * ::emberGetEndpointCluster().  Using the array is often simpler, but consumes
 * large amounts of memory if emberEndpointCount is large.
 *
 * If the application provides definitions for the three functions, it must
 * define EMBER_APPLICATION_HAS_GET_ENDPOINT in its
 * CONFIGURATION_HEADER.
 */
extern EmberEndpoint emberEndpoints[];

/** @brief Retrieves the endpoint number for
 * the index'th endpoint.  <code> index </code> must be less than
 * the value of emberEndpointCount.
 *
 * This function is provided by the stack, using the data from
 * emberEndpoints, unless the application defines
 * EMBER_APPLICATION_HAS_GET_ENDPOINT in its CONFIGURATION_HEADER.
 *
 * @param index  The index of an endpoint (as distinct from its endpoint
 * number).  This must be less than the value of emberEndpointCount.
 *
 * @return The endpoint number for the index'th endpoint.
 */
uint8_t emberGetEndpoint(uint8_t index);

/** @brief Retrieves the endpoint description for the
 * given endpoint.
 *
 * This function is provided by the stack, using the data from
 * emberEndpoints, unless the application defines
 * ::EMBER_APPLICATION_HAS_GET_ENDPOINT in its ::CONFIGURATION_HEADER.
 *
 * @param endpoint  The endpoint whose description is to be returned.
 *
 * @param result    A pointer to the location to which to copy the endpoint
 *    description.
 *
 * @return true if the description was copied to result or false if the
 * endpoint is not active.
 */
bool emberGetEndpointDescription(uint8_t endpoint,
                                 EmberEndpointDescription *result);

/** @brief Retrieves a cluster ID from one of the cluster lists associated
 * with the given endpoint.
 *
 * This function is provided by the stack, using the data from
 * emberEndpoints, unless the application defines
 * ::EMBER_APPLICATION_HAS_GET_ENDPOINT in its CONFIGURATION_HEADER.
 *
 * @param endpoint   The endpoint from which the cluster ID is to be read.
 *
 * @param listId     The list from which the cluster ID is to be read.
 *
 * @param listIndex  The index of the desired cluster ID in the list. This value
 * must be less than the length of the list. The length can be found in the
 * EmberEndpointDescription for this endpoint.
 *
 * @return The cluster ID at position listIndex in the specified endpoint
 * cluster list.
 */
uint16_t emberGetEndpointCluster(uint8_t endpoint,
                                 EmberClusterListId listId,
                                 uint8_t listIndex);

/** @brief Determines whether \c nodeId is valid.
 *
 * @param nodeId  A node ID.
 *
 * @return true if \c nodeId is valid, false otherwise.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
bool emberIsNodeIdValid(EmberNodeId nodeId);
#else
#define emberIsNodeIdValid(nodeId) ((nodeId) < EMBER_DISCOVERY_ACTIVE_NODE_ID)
#endif

/** @brief Returns the node ID that corresponds to the specified EUI64.
 * The node ID is found by searching through all stack tables for the specified
 * EUI64.
 *
 * @param eui64  The EUI64 of the node to look up.
 *
 * @return The short ID of the node or ::EMBER_NULL_NODE_ID if the short ID
 * is not known.
 */
EmberNodeId emberLookupNodeIdByEui64(EmberEUI64 eui64);

/** @brief Returns the EUI64 that corresponds to the specified node ID.
 * The EUI64 is found by searching through all stack tables for the specified
 * node ID.
 *
 * @param nodeId       The short ID of the node to look up.
 *
 * @param eui64Return  The EUI64 of the node is copied here if it is known.
 *
 * @return An ::EmberStatus value:\n\n
 * - ::EMBER_SUCCESS - eui64Return has been set to the EUI64 of the node.
 * - ::EMBER_ERR_FATAL - The EUI64 of the node is not known.
 */
EmberStatus emberLookupEui64ByNodeId(EmberNodeId nodeId,
                                     EmberEUI64 eui64Return);

/** @brief A callback invoked to inform the application of the
 * occurrence of an event defined by ::EmberCounterType, for example,
 * transmissions and receptions at different layers of the stack.
 *
 * The application must define ::EMBER_APPLICATION_HAS_COUNTER_HANDLER
 * in its CONFIGURATION_HEADER to use this.
 * This function may be called in ISR context, so processing should
 * be kept to a minimum.
 *
 * @param type  The type of the event.
 * @param info could map to:
 * 1. data: For transmission events, the number of retries used.
 * For most other events, this parameter is unused and is set to zero.
 * 2. phyIndex: use for mac specific counters specifying if they belong to SubGhz or 2.4
 * 3. destinationNodeId: identifying which connection/destinationId a specific counter refers to
 * 4. Any combination of parameters above
 */

void emberCounterHandler(EmberCounterType type, EmberCounterInfo info);
bool emberCounterRequiresPhyIndex(EmberCounterType type);
bool emberCounterRequiresDestinationNodeId(EmberCounterType type);

/** @brief A callback invoked to inform the application that a stack token has
 * changed.
 *
 * @param tokenAddress  The address of the stack token that has changed.
 */
void emberStackTokenChangedHandler(uint16_t tokenAddress);

/** @brief A callback to allow the application to manage idling the MCU.
 *
 * @param idleTimeMs  The time in millisecond the stack is allowed to idle.
 *
 * @return   true if the application is managing idling the MCU, false otherise.
 * If this function returns false, the stack will manage idling the MCU.
 */
bool emberRtosIdleHandler(uint32_t *idleTimeMs);

/** @brief A callback to request the application to wake up the stack task.
 */
void emberRtosStackWakeupIsrHandler(void);

/** @brief Copies a neighbor table entry to the structure that
 * \c result points to.  Neighbor table entries are stored in
 * ascending order by node id, with all unused entries at the end
 * of the table.  The number of active neighbors can be obtained
 * using ::emberNeighborCount().
 *
 * @param index   The index of a neighbor table entry.
 *
 * @param result  A pointer to the location to which to copy the neighbor
 * table entry.
 *
 * @return ::EMBER_ERR_FATAL if the index is greater or equal to the
 * number of active neighbors, or if the device is an end device.
 * Returns ::EMBER_SUCCESS otherwise.
 */
EmberStatus emberGetNeighbor(uint8_t index, EmberNeighborTableEntry *result);

/** @brief Set neighbour's initial outgoing link cost
 *  @param cost The new default cost
 *
 *  @return ::EMBER_BAD_ARGUMENT if the cost is not any of 0,1,3,5,7
 *  Returns ::EMBER_SUCCESS otherwise
 *  NOTE: There is no ezsp version of this function yet
 */
EmberStatus emberSetInitialNeighborOutgoingCost(uint8_t cost);

/** @brief Get neighbour's initial outgoing link cost
 *
 *  @return default cost associatied to new neighbours outgoing links
 *  NOTE: There is no ezsp version of this function yet
 */
uint8_t emberGetInitialNeighborOutgoingCost(void);

/** @brief Should we reset a rejoining neighbours incoming FC or not
 *  @param reset True or False
 *
 *  @return ::void
 *  NOTE: There is no ezsp version of this function yet
 */
void emberResetRejoiningNeighborsFC(bool reset);

/** @brief Checks if resetting the incoming FC for a rejoining neighbour is enabled or not
 *  @param void
 *
 *  @return ::True or False
 *  NOTE: There is no ezsp version of this function yet
 */
bool emberIsResetRejoiningNeighborsFCEnabled(void);

/** @brief Copies a route table entry to the structure that
 * \c result points to. Unused route table entries have destination
 * 0xFFFF.  The route table size
 * can be obtained via ::emberRouteTableSize().
 *
 * @param index   The index of a route table entry.
 *
 * @param result  A pointer to the location to which to copy the route
 * table entry.
 *
 * @return ::EMBER_ERR_FATAL if the index is out of range or the device
 * is an end device, and ::EMBER_SUCCESS otherwise.
 */
EmberStatus emberGetRouteTableEntry(uint8_t index, EmberRouteTableEntry *result);

/** @brief Returns the stack profile of the network which the
 * node has joined.
 *
 * @return  stack profile
 */
uint8_t emberStackProfile(void);

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief Returns the depth of the node in the network.
 *
 * @return current depth
 */
uint8_t emberTreeDepth(void);
#endif
/** @brief Returns the number of active entries in the neighbor table.
 *
 * @return number of active entries in the neighbor table
 */
uint8_t emberNeighborCount(void);
#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief Returns the size of the route table.
 *
 * @return the size of the route table
 */
uint8_t emberRouteTableSize(void);

#else   // Doxgyen ignores the following
// The '+ 0' prevents anyone from accidentally assigning to these.
#define emberTreeDepth()           (emTreeDepth         + 0)
#define emberMaxDepth()            (emMaxDepth          + 0)
#define emberRouteTableSize()      (emRouteTableSize    + 0)

extern uint8_t emDefaultSecurityLevel;
extern uint8_t emMaxHops;
extern uint8_t emRouteTableSize;
extern uint8_t emMaxDepth;                // maximum tree depth
extern uint8_t emTreeDepth;               // our depth
extern uint8_t emEndDeviceConfiguration;

// This was originally a #define declared here. I moved
// emZigbeeNetworkSecurityLevel to the networkInfo struct.This function is now
// defined in ember-stack-common.c
uint8_t emberSecurityLevel(void);
// New function (defined in ember-stack-common.c)
void emberSetSecurityLevel(uint8_t securityLevel);
#endif

/** @brief Increments and returns the ZigBee sequence number.
 *
 * @return the next ZigBee sequence number
 */
uint8_t emberNextZigbeeSequenceNumber(void);

/** @name Radio-specific Functions*/
//@{

/** @brief Enables boost power mode and/or the alternate transmit path.
 *
 * Boost power mode is a high performance radio mode
 * which offers increased transmit power and receive sensitivity at the cost of
 * an increase in power consumption.  The alternate transmit output path allows
 * for simplified connection to an external power amplifier via the
 * RF_TX_ALT_P and RF_TX_ALT_N pins on the em250.  ::emberInit() calls this
 * function using the power mode and transmitter output settings as specified
 * in the MFG_PHY_CONFIG token (with each bit inverted so that the default
 * token value of 0xffff corresponds to normal power mode and bi-directional RF
 * transmitter output).  The application only needs to call
 * ::emberSetTxPowerMode() if it wishes to use a power mode or transmitter output
 * setting different from that specified in the MFG_PHY_CONFIG token.
 * After this initial call to ::emberSetTxPowerMode(), the stack
 * will automatically maintain the specified power mode configuration across
 * sleep/wake cycles.
 *
 * @note This function does not alter the MFG_PHY_CONFIG token.  The
 * MFG_PHY_CONFIG token must be properly configured to ensure optimal radio
 * performance when the standalone bootloader runs in recovery mode.  The
 * MFG_PHY_CONFIG can only be set using external tools.  IF YOUR PRODUCT USES
 * BOOST MODE OR THE ALTERNATE TRANSMITTER OUTPUT AND THE STANDALONE BOOTLOADER
 * YOU MUST SET THE PHY_CONFIG TOKEN INSTEAD OF USING THIS FUNCTION.
 * Contact support@ember.com for instructions on how to set the MFG_PHY_CONFIG
 * token appropriately.
 *
 * @param txPowerMode  Specifies which of the transmit power mode options are
 * to be activated.  This parameter should be set to one of the literal values
 * described in stack/include/ember-types.h.  Any power option not specified
 * in the txPowerMode parameter will be deactivated.
 *
 * @return ::EMBER_SUCCESS if successful; an error code otherwise.
 */
EmberStatus emberSetTxPowerMode(uint16_t txPowerMode);

/** @brief Returns the current configuration of boost power mode and alternate
 * transmitter output.
 *
 * @return the current tx power mode.
 */
uint16_t emberGetTxPowerMode(void);

/** @brief It allows to set the short node ID of the node. Notice that it can
 * only be set if the stack is in the INITAL state.
 *
 * @param nodeId Specifies the short ID to be assigned to the node.
 *
 * @return ::EMBER_SUCCESS if successful; an error code otherwise.
 */
EmberStatus emberSetNodeId(EmberNodeId nodeId);

/** @brief Returns number of phy interfaces present.
 *
 * @return number of phy interface(s).
 */
uint8_t emberGetPhyInterfaceCount(void);

/** @brief The radio calibration callback function.
 *
 * The Voltage Controlled Oscillator (VCO) can drift with
 * temperature changes.  During every call to ::emberTick(), the stack will
 * check to see if the VCO has drifted.  If the VCO has drifted, the stack
 * will call ::emberRadioNeedsCalibratingHandler() to inform the application
 * that it should perform calibration of the current channel as soon as
 * possible.  Calibration can take up to 150ms.  The default callback function
 * implementation provided here performs calibration immediately.  If the
 * application wishes, it can define its own callback by defining
 * ::EMBER_APPLICATION_HAS_CUSTOM_RADIO_CALIBRATION_CALLBACK in its
 * CONFIGURATION_HEADER.  It can then failsafe any critical processes or
 * peripherals before calling ::emberCalibrateCurrentChannel().  The
 * application must call ::emberCalibrateCurrentChannel() in
 * response to this callback to maintain expected radio performance.
 */
void emberRadioNeedsCalibratingHandler(void);

/** @brief Calibrates the current channel.  The stack will notify the
 * application of the need for channel calibration via the
 * ::emberRadioNeedsCalibratingHandler() callback function during
 * ::emberTick().  This function should only be called from within the context
 * of the ::emberRadioNeedsCalibratingHandler() callback function.  Calibration
 * can take up to 150ms.  Note if this function is called when the radio is
 * off, it will turn the radio on and leave it on.
 */
void emberCalibrateCurrentChannel(void);
//@}//END Radio-specific functions

/** @} END addtogroup */

/**
 * <!-- HIDDEN
 * @page 2p5_to_3p0
 * <hr>
 * The file stack-info.h is new and is described in @ref stack_info.
 * <ul>
 * <li> <b>New items</b>
 * <ul>
 * <li> emberIsNodeIdValid()
 * <li> emberLookupEui64ByNodeId()
 * <li> emberLookupNodeIdByEui64()
 * <li> emberCounterHandler()
 * <li> emberGetExtendedPanId()
 * <li> emberGetRouteTableEntry()
 * <li> emberRadioNeedsCalibratingHandler()
 * <li> emberRouteTableSize()
 * </ul>
 * <li> <b>Changed items</b>
 * <ul>
 * <li> EmberEndpoint - Cluster ids changed from uint8_t to uint16_t.
 * </ul>
 * <li> <b>Functions moved from ember.h</b>
 *  - emberSetTxPowerMode()
 *  - emberCalibrateCurrentChannel()
 *  -  emberStackStatusHandler()
 *  - emberNetworkState()
 *  -  emberStackIsUp()
 *  -  emberGetEui64()
 *  -  emberIsLocalEui64()
 *  -  emberGetNodeId()
 *  -  emberSetManufacturerCode()
 *  -  emberSetPowerDescriptor()
 *  -  emberGetNetworkParameters()
 *  -  emberGetNodeType()
 *  -  emberGetRadioChannel()
 *  -  emberGetRadioPower()
 *  -  emberGetPanId()
 *  -  emberGetEndpoint()
 *  -  emberGetEndpointDescription()
 *  -  emberGetEndpointCluster()
 *  -  emberGetNeighbor()
 *  -  emberStackProfile()
 *  -  emberTreeDepth()
 *  -  emberNeighborCount()
 * </ul>
 * HIDDEN -->
 */

#endif // SILABS_STACK_INFO_H
