/**
 * @file network-management.h
 * @brief  Network Management API.
 *
 * This file contains network management API functions that are
 * available on both host and SOC platforms.
 *
 * Callback naming conventions:
 *   ...Handler()       // These are used for asynchronous status notifications
 *                      // as a result of a unilateral event on the stack.
 *
 *   ...Return()        // These are explicitly the result of an API call, and
 *                      // constitute a command/return event initiated by the
 *                      // application.
 *
 * The network APIs with a command/response model were designed to be the same,
 * both for SoC and host contexts.  On a host, every command follows the
 * asynchronous model as we have to wait for the NCP stack to respond via a
 * callback.  Because of this design, on SoCs, ...Return() callbacks may be
 * called within the command request, and before such a request completes.
 */

#ifndef __NETWORK_MANAGEMENT_H__
#define __NETWORK_MANAGEMENT_H__

#include <stddef.h>
#include "stack/include/ember-types.h"

/**
 * @addtogroup network_utilities
 *
 * See network-management.h for source code.
 * @{
 */

/** @brief  This function initializes the Ember stack. */
void emberInit(void);
/** @brief  This function provides the result of a call to emberInit(). */
void emberInitReturn(EmberStatus status);

/** @brief  A periodic tick routine that must be called in the application's
 * main event loop.
 */
void emberTick(void);

/** @brief  An application structure to hold useful network parameters. */
typedef struct {
  /** @brief  This will only be NUL terminated if the length of the network id
   * is less than EMBER_NETWORK_ID_SIZE.
   */
  uint8_t  networkId[EMBER_NETWORK_ID_SIZE];
  EmberIpv6Prefix ulaPrefix;
  EmberIpv6Prefix legacyUla;
  uint8_t  extendedPanId[EXTENDED_PAN_ID_SIZE];
  uint16_t panId;
  uint8_t  channel;
  EmberNodeType nodeType;
  int8_t  radioTxPower;
  EmberKeyData masterKey;
  /** @brief  This will only be NUL terminated if the length of the key is less
   * than EMBER_JOIN_KEY_MAX_SIZE.
   */
  uint8_t joinKey[EMBER_JOIN_KEY_MAX_SIZE];
  uint8_t joinKeyLength;
} EmberNetworkParameters;

/** @brief  This function erases the network state stored in nonvolatile memory
 * after which the network status will be EMBER_NO_NETWORK.
 * This function should not be called to rejoin a former network;
 * use emberResumeNetwork() instead.  There may be
 * difficulties joining a former network after resetting the network state,
 * due to security considerations.
 */
void emberResetNetworkState(void);

/** @brief This function provides the result of a call to emberResetNetworkState(). */
void emberResetNetworkStateReturn(EmberStatus status);

// Power Management for sleepy end devices.

/** @brief An application handler for deep sleep on sleepy end devices.
 * This call is ignored for non-sleepy devices.  The device may or may not sleep
 * depending on the internal state.
 *
 * @return true if going to deep sleep.
 */
bool emberDeepSleepTick(void);

/** @brief This function turns chip deep sleep on or off for sleepy end devices.
 * This call is ignored on non-sleepy devices.  The device may or may not sleep
 * depending on the internal state */
void emberDeepSleep(bool sleep);

/** @brief This function provides the result of a call to emberDeepSleep(). */
void emberDeepSleepReturn(EmberStatus status);

/** @brief For a sleepy end device, report how long the chip went to deep sleep.
 * In a NCP + host setup, the stack reports this to the host app.
 */
void emberDeepSleepCompleteHandler(uint16_t sleepDuration);

/** @brief Required radio state while stack is idle.
 */
typedef enum {
  /** Incoming messages are expected and the radio must be left on. */
  IDLE_WITH_RADIO_ON,
  /** Incoming messages are expected and must be polled for. */
  IDLE_WITH_POLLING,
  /** No messages are expected and the radio may be left off. */
  IDLE_WITH_RADIO_OFF
} EmberIdleRadioState;

/** @brief This function returns the time the stack will be idle, in milliseconds.
 * Also sets radioStateResult to the required radio state while the
 * stack is idle.
 *
 * This function returns directly, rather than having a ...Return()
 * callback, because it is only available on SOCs.
 *
 * @param radioStateResult      Used to return the required radio
 *  state while the stack is idle.
 *
 * @return The number of milliseonds for which the stack will be idle.
 */
uint32_t emberStackIdleTimeMs(EmberIdleRadioState *radioStateResult);

/** @brief This function defines tasks that prevent the stack from sleeping.
 */
enum {
  /** There are messages waiting for transmission. */
  EMBER_OUTGOING_MESSAGES = 0x01,
  /** One or more incoming messages are being processed. */
  EMBER_INCOMING_MESSAGES = 0x02,
  /** The radio is currently powered on.  On sleepy devices the radio is
   *  turned off when not in use.  On non-sleepy devices (::EMBER_ROUTER or
   * ::EMBER_END_DEVICE) the radio is always on.
   */
  EMBER_RADIO_IS_ON = 0x04,
};

/** @brief A mask of the high priority tasks that prevent a device from sleeping.
 * Devices should not sleep if any high priority tasks are active.
 */
#define EMBER_HIGH_PRIORITY_TASKS \
  (EMBER_OUTGOING_MESSAGES | EMBER_INCOMING_MESSAGES | EMBER_RADIO_IS_ON)

/** @brief This function indicates whether the stack is currently in a state where
 * there are no high priority tasks and may sleep.
 *
 * There may be tasks expecting incoming messages, in which case the device should
 * periodically wake up and call ::emberPollForData() in order to receive messages.
 * This function can only be called for sleepy end devices.
 */
bool emberOkToNap(void);

/** @brief  If implementing event-driven sleep on an NCP host, this method will
 * return the bitmask indicating the stack's current tasks. (see enum above)
 *
 *  The mask ::EMBER_HIGH_PRIORITY_TASKS defines which tasks are high
 *  priority.  Devices should not sleep if any high priority tasks are active.
 *  Active tasks that are not high priority are waiting for
 *  messages to arrive from other devices.  If there are active tasks,
 *  but no high priority ones, the device may sleep but should periodically
 *  wake up and call ::emberPollForData() in order to receive messages.  Parents
 *  will hold messages for ::EMBER_INDIRECT_TRANSMISSION_TIMEOUT (in quarter
 *  seconds) before discarding them.
 *
 * @return A bitmask of the stack's active tasks.
 */
void emberOkToNapReturn(uint8_t stateMask);

/** @brief This method is called any time an event is scheduled from within an
 * ISR context. It can be used to determine when to stop a long running sleep
 * to see what application or stack events now need to be processed.
 * @param event The event that was scheduled by the ISR.
 */
void emberEventDelayUpdatedFromIsrHandler(Event *event);

// Stack power down utilities

/** @brief  This function gets the stack ready for power down, or deep sleep.  Purges the MAC
 *  indirect queue, and empties the phy-to-mac and mac-to-network queues.
 */
void emberStackPrepareForPowerDown(void);

/** @brief  This function returns true if the stack is currently emptying any message queues
 * or false if the MAC queue is currently not empty.
 */
bool emberStackPreparingForPowerDown(void);

/** @brief Immediately turns the radio power completely off.
 *
 * After calling this function, you must not call any other stack function
 * except emberStackPowerUp(). This is because all other stack
 * functions require that the radio is powered on for their
 * proper operation.
 */
void emberStackPowerDown(void);

/** @brief This function initializes the radio.  Typically called coming out of deep sleep.
 *
 * For non-sleepy devices, also turns the radio on and leaves it in rx mode.
 */
void emberStackPowerUp(void);

/** @brief For sleepy hosts, use this call to have the stack manage polling for
 * sleepy end devices.  In a host/NCP setup, this means that the NCP app will
 * take care of periodic data polling.
 */
void emberStackPollForData(uint32_t pollMs);

/** @brief This function provides the result of a call to emberStackPollForData(). */
void emberStackPollForDataReturn(EmberStatus status);

/** @brief Use this call if setting up polling for sleepy end devices on the
 * application.
 *
 * This function allows a sleepy end device to query its parent for any
 * pending data.
 *
 * Sleepy end devices must call this function periodically to maintain contact
 * with their parent.  The parent will remove a sleepy end device from its child
 * table if it has not received a poll from it within the last
 * ::EMBER_SLEEPY_CHILD_POLL_TIMEOUT seconds.
 *
 * If the sleepy end device has lost contact with its parent, it re-joins then
 * network using another router.
 *
 * The default values for the timeouts are set in
 * config/ember-configuration-defaults.h, and can be overridden in the
 * application's configuration header.
 */
void emberPollForData(void);

/** @brief This function provides the result of a call to emberPollForData().
 * @param An EmberStatus value:
 * - ::EMBER_SUCCESS      - The poll message has been submitted for transmission
 * - ::EMBER_INVALID_CALL - The node is not a sleepy end device.
 * - ::EMBER_NOT_JOINED   - The node is not part of a network.
 */
void emberPollForDataReturn(EmberStatus status);

// Network info.

/** @brief  This function returns the EUI64 of the Ember chip. */
const EmberEui64 *emberEui64(void);

/** @brief  This function returns the current status of the network.
 * Prior to calling emberInitNetwork(), the status is EMBER_NETWORK_UNINITIALIZED.
 */
EmberNetworkStatus emberNetworkStatus(void);

/** @brief  This function reports a change to the network status.  For example, the network
 * status changes while going through the joining process, or while reattaching
 * to the network, which can happen for a variety of reasons.  In particular,
 * after issuing a form, join, resume, or attach command, the application
 * knows that the device is on the network and ready to communicate when
 * this handler is called with a newNetworkStatus of
 * EMBER_JOINED_NETWORK_ATTACHED.
 *
 * If the status handler is reporting a join failure, then the newNetworkStatus
 * argument will have a value of EMBER_NO_NETWORK and the reason argument
 * will contain an appropriate value.  For other network status reports, the
 * reason argument does not apply and is set to EMBER_JOIN_FAILURE_REASON_NONE.
 */
void emberNetworkStatusHandler(EmberNetworkStatus newNetworkStatus,
                               EmberNetworkStatus oldNetworkStatus,
                               EmberJoinFailureReason reason);

/** @brief This function fetches the current network parameters into the supplied pointer. */
void emberGetNetworkParameters(EmberNetworkParameters *parameters);

/** @brief This function returns the pan id of the network.
 */
EmberPanId emberGetPanId(void);

/** @brief Structure that holds information about a routing table entry for use
 * on the application.  See ::emberGetRipEntry
 */
typedef struct {
  uint8_t longId[8];
  EmberNodeType type;
  int8_t rollingRssi;
  uint8_t nextHopIndex;
  uint8_t ripMetric;
  uint8_t incomingLinkQuality;
  uint8_t outgoingLinkQuality;
  bool mleSync;
  uint8_t age;
  uint8_t routeDelta;
} EmberRipEntry;

/** @brief  This function gets the EmberRipEntry at the specified index of the RIP table.
 * The result is returned to the application via the emberGetRipEntryReturn()
 * callback.
 *
 * The index is between 0 and 31 inclusive, but there may be fewer than 32
 * valid entries depending on the number of routers in the network.
 *
 * The caller can pass in a 0xFF index to request all valid RIP table entries.
 * Note that the stack will ONLY return valid entries when 0xFF is passed. Once
 * all valid entries have been returned by this method, an extra zeroed-out entry
 * is returned to indicate completion.
 *
 * When the application requests an EmberRipEntry at a certain index, it can
 * check for the validity of the returned EmberRipEntry by checking whether
 * it is zeroed out.  For example, the 'type' parameter should never be zero.
 * (it should be a valid node type: EMBER_ROUTER@cond EMBER_WAKEUP_STACK
 * or EMBER_LURKER@endcond)
 */
void emberGetRipEntry(uint8_t index);

/** @brief This function provides the result of a call to emberGetRipEntry(). */
void emberGetRipEntryReturn(uint8_t index, const EmberRipEntry *entry);

/** @brief  This function gets the value for the specified counter.  The result is returned
 * to the application via emberGetCounterReturn().
 */
void emberGetCounter(EmberCounterType type);

/** @brief This function provides the result of a call to emberGetCounter(). */
void emberGetCounterReturn(EmberCounterType type, uint16_t value);

/** @brief  This function resets all counter values to 0. */
void emberClearCounters(void);

/** @brief A callback invoked to inform the application of the
 * occurrence of an event defined by EmberCounterType, for example,
 * transmissions and receptions at different layers of the stack.
 *
 * The application must define EMBER_APPLICATION_HAS_COUNTER_HANDLER
 * in its CONFIGURATION_HEADER to use this.
 * This function may be called in ISR context, so processing should
 * be kept to a minimum.
 *
 * @param type       The type of the event.
 * @param increment  Specify the increase in the counter's tally.
 *
 */
void emberCounterHandler(EmberCounterType type, uint16_t increment);

/** @brief A callback invoked to query the application for the
 * countervalue of an event defined by EmberCounterType.
 *
 * The application must define EMBER_APPLICATION_HAS_COUNTER_VALUE_HANDLER
 * in its CONFIGURATION_HEADER to use this.
 *
 * @param   type       The type of the event.
 * @returns The counter's tally.
 *
 */
uint16_t emberCounterValueHandler(EmberCounterType type);

/** @brief
 * This API provides a means to forward a raw IPv6 packet on the mesh.
 *
 * @param packet       Raw bytes of the IPv6 packet
 * @param packetLength Length of the packet
 */
bool emberForwardIpv6Packet(const uint8_t *packet,
                            const uint16_t packetLength);

// Scanning.

/** @brief This function starts a scan.  Note that while a scan can be initiated
 * while the node is currently joined to a network, the node will generally
 * be unable to communicate with its PAN during the scan period, so care
 * should be taken when performing scans of any significant duration while
 * presently joined to an existing PAN.
 *
 * Upon completion of the scan, a status is returned via ::emberScanReturn().
 * Possible EmberStatus values and their meanings:
 * - ::EMBER_SUCCESS, the scan completed succesffully.
 * - ::EMBER_MAC_SCANNING, we are already scanning.
 * - ::EMBER_MAC_BAD_SCAN_DURATION, we have set a duration value that is
 *   not 0..14 inclusive.
 * - ::EMBER_MAC_INCORRECT_SCAN_TYPE, we have requested an undefined
 *   scanning type;
 * - ::EMBER_MAC_INVALID_CHANNEL_MASK, our channel mask did not specify any
 *   valid channels on the current platform.
 *
 * @param scanType  Indicates the type of scan to be performed.
 *  Possible values:  ::EMBER_ENERGY_SCAN, ::EMBER_ACTIVE_SCAN.
 *
 * @param channelMask  Bits set as 1 indicate that this particular channel
 * should be scanned. Bits set to 0 indicate that this particular channel
 * should not be scanned.  For example, a channelMask value of 0x00000001
 * would indicate that only channel 0 should be scanned.  Valid channels range
 * from 11 to 26 inclusive.  This translates to a channel mask value of 0x07
 * FF F8 00.  As a convenience, a channelMask of 0 is reinterpreted as the
 * mask for the current channel.
 *
 * @param duration  Sets the exponent of the number of scan periods,
 * where a scan period is 960 symbols, and a symbol is 16 microseconds.
 * The scan will occur for ((2^duration) + 1) scan periods.  The value
 * of duration must be less than 15.  The time corresponding to the first
 * few values are as follows: 0 = 31 msec, 1 = 46 msec, 2 = 77 msec,
 * 3 = 138 msec, 4 = 261 msec, 5 = 507 msec, 6 = 998 msec.
 */
void emberStartScan(EmberNetworkScanType scanType,
                    uint32_t channelMask,
                    uint8_t duration);

/** @brief  This function reports the maximum RSSI value measured on the channel.
 *
 * @param channel  The 802.15.4 channel on which the RSSI value was measured.
 *
 * @param maxRssiValue  The maximum RSSI value measured (in units of dBm).
 */
void emberEnergyScanHandler(uint8_t channel, int8_t maxRssiValue);

/** @brief  Size of the island (aka network fragment) ID. */
#define ISLAND_ID_SIZE 5

/** @brief  Structure to hold information about an 802.15.4 beacon for use
 * on the application.
 */
typedef struct {
  uint8_t   networkId[16];
  uint8_t   extendedPanId[8];
  uint8_t   longId[8];
  uint16_t  panId;
  uint8_t   protocolId;
  uint8_t   channel;
  bool      allowingJoin;
  uint8_t   lqi;
  int8_t    rssi;
  uint8_t   version;
  uint16_t  shortId; // deprecated, beacons now use 64-bit source
  uint8_t   steeringData[16];
  uint8_t   steeringDataLength;
} EmberMacBeaconData;

/** @brief  This function reports an incoming beacon during an active scan. */
void emberActiveScanHandler(const EmberMacBeaconData *beaconData);

/** @brief  This function provides the status upon completion of a scan. */
void emberScanReturn(EmberStatus status);

/** @brief  This function terminates a scan in progress. */
void emberStopScan(void);

// Other.

/** @brief  This function resets the Ember chip. */
void emberResetMicro(void);

/** @brief  Enumerate the various chip reset causes. */
typedef enum {
  EMBER_RESET_UNKNOWN,
  EMBER_RESET_FIB,
  EMBER_RESET_BOOTLOADER,
  EMBER_RESET_EXTERNAL,
  EMBER_RESET_POWERON,
  EMBER_RESET_WATCHDOG,
  EMBER_RESET_SOFTWARE,
  EMBER_RESET_CRASH,
  EMBER_RESET_FLASH,
  EMBER_RESET_FATAL,
  EMBER_RESET_FAULT,
  EMBER_RESET_BROWNOUT
} EmberResetCause;

/** @brief  This function notifies the application of a reset on the Ember chip
 * due to the indicated cause.
 */
void emberResetMicroHandler(EmberResetCause cause);

/** @brief
 * This function detects if the standalone bootloder is installed, and if so
 * returns the installed version and info about the platform,
 * micro and phy. If not version will be set to 0xffff. A
 * returned version of 0x1234 would indicate version 1.2 build 34.
 */
void emberGetStandaloneBootloaderInfo(void);

/** @brief
 * This function provides the result of a call to ::emberGetStandaloneBootloaderInfo.
 *
 * @param version   BOOTLOADER_INVALID_VERSION if the standalone
 *                  bootloader is not present, or the version of
 *                  the installed standalone bootloader.
 * @param nodePlat  The value of PLAT on the node.
 * @param nodeMicro The value of MICRO on the node.
 * @param nodePhy   The value of PHY on the node.
 */
void emberGetStandaloneBootloaderInfoReturn(uint16_t version,
                                            uint8_t platformId,
                                            uint8_t microId,
                                            uint8_t phyId);

/** @brief
 * This function launches the standalone bootloader (if installed).
 * The function returns an error if the standalone bootloader
 * is not present
 *
 * @param mode Controls the mode in which the standalone
 *             bootloader will run. See the app. note for full
 *             details. Options are:
 *             STANDALONE_BOOTLOADER_NORMAL_MODE: Will listen for
 *             an over-the-air image transfer on the current
 *             channel with current power settings.
 *             STANDALONE_BOOTLOADER_RECOVERY_MODE: Will listen for
 *             an over-the-air image transfer on the default
 *             channel with default power settings. Both modes
 *             also allow an image transfer to begin with
 *             XMODEM over the serial protocol's Bootloader
 *             Frame.
 */
void emberLaunchStandaloneBootloader(uint8_t mode);

/** @brief
 * This function provides the result of a call to ::emberLaunchStandaloneBootloader.
 *
 * @param status An EmberStatus value indicating success or the
 *               reason for failure.
 */
void emberLaunchStandaloneBootloaderReturn(EmberStatus status);

/** @brief In a host/NCP setup, inform the NCP to send the network state and
 * version information.
 */
void emberInitHost(void);

/**
 * @brief In a host/NCP setup, get the network parameters, the network status and eui64 all at once.
 */
void emberState(void);

/**
 * @brief In a host/NCP setup, provides the result of a call to emberState() on the host.
 *
 * @param parameters Current network parameters
 * @param localEui64 The EUI64 of the Ember chip
 * @param macExtendedId The extended MAC ID of the Ember chip
 * @param networkStatus The current status of the network
 */
void emberStateReturn(const EmberNetworkParameters *parameters,
                      const EmberEui64 *localEui64,
                      const EmberEui64 *macExtendedId,
                      EmberNetworkStatus networkStatus);

/**
 * @brief In a host/NCP setup, notifies the host to changes in the network parameters.
 *
 * @param parameters Current network parameters
 * @param localEui64 The EUI64 of the Ember chip
 * @param macExtendedId The extended MAC ID of the Ember chip
 * @param networkStatus The current status of the network
 */
void emberHostStateHandler(const EmberNetworkParameters *parameters,
                           const EmberEui64 *localEui64,
                           const EmberEui64 *macExtendedId,
                           EmberNetworkStatus networkStatus);

/** @brief This function sets the radio output power at which a node is to operate. Ember
 * radios have discrete power settings. For a list of available power settings,
 * see the technical specification for the RF communication module in
 * your Developer Kit.
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
void emberSetRadioPower(int8_t power);

/** @brief This function provides the result of a call to emberSetRadioPower() on the host. */
void emberSetRadioPowerReturn(EmberStatus status);

/** @brief This function gets the radio output power at which a node is operating. Ember
 * radios have discrete power settings. For a list of available power settings,
 * see the technical specification for the RF communication module in
 * your Developer Kit.
 *
 * @return Current radio output power, in dBm.
 */
void emberGetRadioPower(void);

/** @brief This function provides the result of a call to emberGetRadioPower() on the host. */
void emberGetRadioPowerReturn(int8_t power);

/** @brief This function enables boost power mode and/or the alternate transmit path.
 *
 * Boost power mode is a high-performance radio mode
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

/** @brief This function provides the result of a call to emberSetTxPowerMode() on the host. */
void emberSetTxPowerModeReturn(EmberStatus status);

/** @brief This function requests the current configuration of boost power mode and alternate
 * transmitter output.
 */
void emberGetTxPowerMode(void);

/** @brief This function provides the result of a call to emberGetTxPowerMode() on the host.
 * @return the current tx power mode.
 */
void emberGetTxPowerModeReturn(uint16_t txPowerMode);

/** @brief Values of security parameters for use in forming or joining
 * a network. */

/** @brief Structure to hold information about pre-shared network security
 * parameters.
 */
typedef struct {
  EmberKeyData *networkKey;
  uint8_t *presharedKey;
  uint8_t presharedKeyLength;
} EmberSecurityParameters;

/** @brief Define the various options for setting network parameters.
 * Note: Only the EMBER_NETWORK_KEY_OPTION works at this time.
 */
#define EMBER_NETWORK_KEY_OPTION        BIT(0)
#define EMBER_PSK_JOINING_OPTION        BIT(1)
#define EMBER_ECC_JOINING_OPTION        BIT(2)

/** @brief
 * This function is called before forming or joining.  Fails if already formed or joined
 * or if the arguments are inconsistent with the stack (i.e. if ECC is
 * wanted and we have no ECC).
 *
 * *** Only the EMBER_NETWORK_KEY_OPTION works at this time. ***
 */
void emberSetSecurityParameters(const EmberSecurityParameters *parameters,
                                uint16_t options);

/** @brief This function provides the result of a call to emberSetSecurityParameters(). */
void emberSetSecurityParametersReturn(EmberStatus status);

/** @brief
 * This function changes MAC encryption over to the next key.  Fails if there is no
 * next network key.
 *
 */
void emberSwitchToNextNetworkKey(void);

/** @brief This function provides the result of a call to emberSwitchToNextNetworkKey(). */
void emberSwitchToNextNetworkKeyReturn(EmberStatus status);

/** @brief
 * This function can be stubbed out on the SoC and host app.  It is used by the
 * NCP to update security on the driver when it is instructed to switch
 * the network key by an over the air update.
 *
 */
void emberSwitchToNextNetworkKeyHandler(EmberStatus status);

/** @brief
 *  This function gets various versions:
 *  The stack version name (versionName)
 *  The management version number (managementVersionNumber, if applicable,
 *  otherwise set to 0xFFFF)
 *  The stack version number (stackVersionNumber)
 *  The stack build number (stackBuildNumber)
 *  The version type (versionType)
 *  The date / time of the build (buildTimestamp)
 */
void emberGetVersions(void);

/** @brief Provides the result of a call to emberGetVersions(). */
void emberGetVersionsReturn(const uint8_t *versionName,
                            uint16_t managementVersionNumber,
                            uint16_t stackVersionNumber,
                            uint16_t stackBuildNumber,
                            EmberVersionType versionType,
                            const uint8_t *buildTimestamp);

/** @brief
 *  This function sets the CCA threshold level - the noise floor above which the channel
 *  is normally considered busy. The threshold parameter is expected to be
 *  a signed 2's complement value, in dBm.
 */
void emberSetCcaThreshold(int8_t threshold);

/** @brief This function provides the result of a call to emberSetCcaThreshold(). */
void emberSetCcaThresholdReturn(EmberStatus status);

/** @brief
 *  This function gets the current CCA threshold level.
 */
void emberGetCcaThreshold(void);

/** @brief This function provides the result of a call to emberGetCcaThreshold(). */
void emberGetCcaThresholdReturn(int8_t threshold);

/** @brief Application handler to intercept "passthrough" packets and
 * handle them at the application.
 *
 * @note This API is for SoCs only.
 * @note The application must define
 *       EMBER_APPLICATION_HAS_MAC_PASSTHROUGH_MESSAGE_HANDLER
 *
 * @param header The message buffer pointing to the full 802.15.4 frame
 *               to be handled by the application.
 */
void emberMacPassthroughMessageHandler(PacketHeader header);

/** @brief Application handler to define "passthrough" packets.
 *
 * @note This API is for SoCs only.
 * @note The application must define
 *       EMBER_APPLICATION_HAS_MAC_PASSTHROUGH_FILTER_HANDLER
 *
 * @param macHeader A pointer to the initial portion of the incoming MAC
 *                  header, in the standard 802.15.4 format.  The first
 *                  two bytes comprise the frame control, which dictates
 *                  source / destination PAN and addressing formats.
 *                  (See the MAC sublayer definition in the standards
 *                   definition 802.15.4e/2012)
 *
 *                  The relevant bytes of the header are:
 *  ----------------------------------------------------------------------
 *  | octets: |  2  |  1  |  0/2    |   0/2/8   |   0/2   |   0/2/8  |
 *  ----------------------------------------------------------------------
 *  |         | ctl | seq | dst.pan |  dst.addr | src.pan | src.addr | ...
 *  ----------------------------------------------------------------------
 *                  Note that subsequent MAC fields, and the MAC payload,
 *                  may not yet be present at this point.
 *
 * @return true if the message is an application MAC passthrough message.
 */
bool emberMacPassthroughFilterHandler(uint8_t *macHeader);

/** @brief Application handler to filter 802.15.4 packets to be observed
 * for signal strength.
 *
 * @note This API is for SoCs only.
 * @note The application must define
 *       EMBER_APPLICATION_HAS_RSSI_FILTER_HANDLER
 *
 * @param macHeader A pointer to the initial portion of the incoming MAC
 *                  header, in the standard 802.15.4 format.  The first
 *                  two bytes comprise the frame control, which dictates
 *                  source / destination PAN and addressing formats.
 *                  (See the MAC sublayer definition in the standards
 *                   definition 802.15.4e/2012)
 *
 *                  The relevant bytes of the header are:
 *  ----------------------------------------------------------------------
 *  | octets: |  2  |  1  |  0/2    |   0/2/8   |   0/2   |   0/2/8  |
 *  ----------------------------------------------------------------------
 *  |         | ctl | seq | dst.pan |  dst.addr | src.pan | src.addr | ...
 *  ----------------------------------------------------------------------
 *                  Note that subsequent MAC fields, and the MAC payload,
 *                  may not yet be present at this point.
 *
 * @return true if the application wants to peek at the RSSI for this message.
 */
bool emberMacRssiFilterHandler(uint8_t *macHeader);

/** @brief Gets the received signal strength indication (RSSI) for the
 * last 802.15.4 packet received by the stack.
 *
 * @note This is called on the application for all packets that match
 * the rule defined in ::emberMacRssiFilterHandler()
 *
 * The quantity referenced by currentRssi will contain the energy level
 * (in units of dBm) observed during the last 802.15.4 packet received
 * in that handler.
 *
 * @note This API is for SoCs only.
 * @note The application must define
 *       EMBER_APPLICATION_HAS_RSSI_FILTER_HANDLER
 *
 * @note This functionality is not available for packets such as 802.15.4
 * data requests or acknowledgements.  Data requests must be handled quickly
 * due to strict 15.4 timing requirements, and so the RSSI information is not
 * recorded.  Similarly, 802.15.4 ACKs are handled by the hardware and the
 * information does not make it up to the stack.
 *
 * @param currentRssi  The RSSI for the last incoming message processed.
 */
void emberMacRssiHandler(int8_t currentRssi);

/** @brief
 *  Read token values stored on the Ember chip.
 */
typedef uint8_t EmberTokenId;

/** @brief
 * Enumerate the various token values that can be retrieved by the
 * application.
 */
enum {
  EMBER_CHANNEL_CAL_DATA_TOKEN // arg needed: channel (uint8_t)
};

/** @brief
 * This function gets the indexed token stored in non-volatile memory on the Ember chip.
 * The result is returned depending on the tokenId provided
 * (see enum above) to the appropriate Return() API.
 */
void emberGetIndexedToken(EmberTokenId tokenId, uint8_t index);

/** @brief
 * This function gets the token information for tokenId = EMBER_CHANNEL_CAL_DATA_TOKEN
 *
 * @param lna          [msb: cal needed? | bit 0-5: lna tune value]
 * @param tempAtLna    [the temp (degC) when the LNA was calibrated]
 * @param modDac       [msb: cal needed? | bit 0-5: modulation DAC tune value]
 * @param tempAtModDac [the temp (degC) when the mod DAC was calibrated]
 */
void emberGetChannelCalDataTokenReturn(uint8_t lna,
                                       int8_t tempAtLna,
                                       uint8_t modDac,
                                       int8_t tempAtModDac);

/** @brief
 *  Token identifier used when reading and writing manufacturing tokens.
 */
typedef uint8_t EmberMfgTokenId;

/** @brief
 * Enumerate the various manufacturing token values that can be read
 * or written by the application.
 */
enum {
  EMBER_CUSTOM_EUI_64_MFG_TOKEN,
  EMBER_EZSP_STORAGE_MFG_TOKEN,
  EMBER_CTUNE_MFG_TOKEN
};

/** @brief
 * This function gets the manufacturer token stored in non-volatile memory on the Ember chip.
 *
 * @param tokenId Which manufacturing token to read.
 */
void emberGetMfgToken(EmberMfgTokenId tokenId);

/** @brief
 * This function provides the result of a call to ::emberGetMfgToken.
 *
 * @param tokenId         Which manufacturing token read.
 * @param status          An EmberStatus value indicating success or the
 *                        reason for failure.
 * @param tokenData       The manufacturing token data.
 * @param tokenDataLength The length of the <i>tokenData</i> parameter in
 *                        bytes.
 */
void emberGetMfgTokenReturn(EmberMfgTokenId tokenId,
                            EmberStatus status,
                            const uint8_t *tokenData,
                            uint8_t tokenDataLength);

/** @brief
 * This function sets the manufacturer token stored in non-volatile memory on the Ember chip.
 *
 * @param tokenId         Which manufacturing token to set.
 * @param tokenData       The manufacturing token data.
 * @param tokenDataLength The length of the <i>tokenData</i> parameter in
 *                        bytes.
 */
void emberSetMfgToken(EmberMfgTokenId tokenId,
                      const uint8_t *tokenData,
                      uint8_t tokenDataLength);

/** @brief
 * This function provides the result of a call to ::emberSetMfgToken.
 *
 * @param tokenId         Which manufacturing token set.
 * @param status An EmberStatus value indicating success or the
 *               reason for failure.
 */
void emberSetMfgTokenReturn(EmberMfgTokenId tokenId,
                            EmberStatus status);

/**
 * @brief This function gets the CTUNE value. (Only valid on EFR32)
 */
void emberGetCtune(void);

/** @brief
 * This function provides the result of a call to ::emberGetCtune.
 *
 * @param tune   The current CTUNE value.
 * @param status An EmberStatus value indicating success or the
 *               reason for failure.
 */
void emberGetCtuneReturn(uint16_t tune,
                         EmberStatus status);

/**
 * @brief This function changes the CTUNE value. Involves switching to HFRCO and turning off
 * the HFXO temporarily. (Only valid on EFR32)
 *
 * @param tune   Value to set CTUNE to.
 */
void emberSetCtune(uint16_t tune);

/** @brief
 * This function provides the result of a call to ::emberSetCtune.
 *
 * @param status An EmberStatus value indicating success or the
 *               reason for failure.
 */
void emberSetCtuneReturn(EmberStatus status);

/**
 * @brief This function registers a callback function so that the application can define
 * rules to drop incoming packets.  The callback function MUST be of
 * the form:
 * bool func_name(PacketHeader header, Ipv6Header *ipHeader)
 * {
 *   ...
 * }
 */
void emberRegisterDropIncomingMessageCallback
  (bool (*drop)(PacketHeader header, Ipv6Header *ipHeader));

/**
 * @brief This function registers a callback function so that the application can define
 * serial transmit logic.  This should only be used for NCPs, and will have no
 * effect for SoCs.  The callback function MUST be of the form:
 * void uartTransmit(uint8_t type, Buffer b)
 * {
 *   ...
 * }
 */
void emberRegisterSerialTransmitCallback
  (void (*serialTransmit)(uint8_t type, PacketHeader header));

/** @} // END addtogroup
 */

/**
 * @addtogroup ipv6
 *
 * See network-management.h for source code.
 * @{
 */
// IPv6 addressing APIs

/** @brief  The maximum number of IPv6 addresses configured for the device.
 * See ::emberGetLocalIpAddress
 */
// Mesh-local 64 + Link-local 64 + global address table
#define EMBER_MAX_IPV6_ADDRESS_COUNT 10
#define EMBER_MAX_IPV6_GLOBAL_ADDRESS_COUNT (EMBER_MAX_IPV6_ADDRESS_COUNT - 2)
#define EMBER_MAX_IPV6_EXTERNAL_ROUTE_COUNT (EMBER_MAX_IPV6_ADDRESS_COUNT - 2)

typedef enum {
  REALM_SCOPE = 0,
  LINK_SCOPE = 1,
  GLOBAL_SCOPE = 2, // any index greater than GLOBAL is global scope
} EmberLocalAddressScope;

/** @brief  This function fetches one of the device IPv6 addresses into the supplied pointer.
 * Since there may be multiple addresses, an index argument between 0 and
 * ::EMBER_MAX_IPV6_ADDRESS_COUNT must be supplied.
 *
 * Index 0 contains the mesh-local 64 address of the node.
 * Index 1 contains the link-local 64 address of the node.
 * Index 2 and greater will return any global unicast addresses (GUAs) of this
 * node.
 *
 * @return false if no IPv6 address is stored at the given index.
 */
bool emberGetLocalIpAddress(uint8_t index, EmberIpv6Address *address);

/** @brief  This function fetches the Thread Routing Locator (RLOC).
 *
 * A Thread Routing Locator (RLOC) is an IPv6 address that identifies the
 * location of a Thread interface within a Thread partition.  Thread devices
 * use RLOCs internally for communicating control traffic.
 *
 * NOTE:
 * Using RLOCs for application messaging is NOT recommended since device
 * identifiers used to build these RLOC addresses may change at any time
 * based on the current network state.  Please note that message delivery
 * is not guaranteed when an RLOC address is used.
 *
 * It is recommended that application developers use ::emberGetLocalIpAddress.
 */
void emberGetRoutingLocator(void);

/** @brief This function provides the result of a call to ::emberGetRoutingLocator.
 *
 * @param rloc The Routing Locator as a full IPv6 address.
 */
void emberGetRoutingLocatorReturn(const EmberIpv6Address *rloc);

/** @brief Sets the Network Data that describes the local node's Border Router
 * and server capabilities.  This is passed a set of Network Data TLVs that
 * may include Prefix, Has Route, Border Router, Service and Server TLVS.
 *
 * The stack will set the correct local node ID into the TLVs.
 *
 * This function is an alternative to the ::emberConfigureGateway
 * ::emberConfigureExternalRoute functions that provides full access to
 * Network Data configuration.  A call to this function removes any
 * previous configuration, including uses of ::emberConfigureGateway
 * and ::emberConfigureExternalRoute.
 *
 * @param networkData A pointer to a set of Thread Network Data TLVs that
 *   describe the local nodes Border Router and Server capabilities.
 * @param length The number of bytes in the supplied network data.
 */
void emberSetLocalNetworkData(const uint8_t *networkData, uint16_t length);

/** @brief Provides the result of a call to ::emberSetServerNetworkData.
 *
 */
void emberSetLocalNetworkDataReturn(EmberStatus status, uint16_t length);

/** @brief Border router flags (see Thread spec chapter 5 for more information)
 */
typedef enum {
  EMBER_BORDER_ROUTER_ND_DNS_FLAG          = 0x0080,
  EMBER_BORDER_ROUTER_ON_MESH_FLAG         = 0x0100,
  EMBER_BORDER_ROUTER_DEFAULT_ROUTE_FLAG   = 0x0200,
  EMBER_BORDER_ROUTER_CONFIGURE_FLAG       = 0x0400,
  EMBER_BORDER_ROUTER_DHCP_FLAG            = 0x0800,
  EMBER_BORDER_ROUTER_SLAAC_FLAG           = 0x1000,
  EMBER_BORDER_ROUTER_PREFERRED_FLAG       = 0x2000,
  EMBER_BORDER_ROUTER_PREFERENCE_MASK      = 0xC000,
  EMBER_BORDER_ROUTER_HIGH_PREFERENCE      = 0x4000,
  EMBER_BORDER_ROUTER_MEDIUM_PREFERENCE    = 0x0000,
  EMBER_BORDER_ROUTER_LOW_PREFERENCE       = 0xC000,
} EmberBorderRouterTlvFlag_e;

typedef uint16_t EmberBorderRouterTlvFlag;

/** @brief We enforce this limit to avoid overflow when converting
 * lifetimes from seconds to milliseconds.
 */
#define EMBER_MAX_LIFETIME_DELAY_SEC ((HALF_MAX_INT32U_VALUE - 1) / 1000)

/** @brief There should be at least half an hour of preferred lifetime remaining
 * to advertise a DHCP server.
 */
#define EMBER_MIN_PREFERRED_LIFETIME_SEC 1800

/** @brief Renew when we are down to one minute of valid lifetime.
 */
#define EMBER_MIN_VALID_LIFETIME_SEC 60

/** @brief This function configures the border router behavior, such as whether this device has a
 * default route to the Internet, and whether it have a prefix that can be used
 * by network devices to configure routable addresses.
 *
 * Also de-configures a border router prefix (if it exists), if the prefix and
 * prefixLengthInBits are specified, and borderRouterFlags equals 0x00FF.
 *
 * This triggers an address configuration change on the border router, and the
 * application is informed of this by ::emberAddressConfigurationChangeHandler.
 *
 * Note:  If the application wants to manually configure an address and not have
 * the stack create one, then it should pass in the entire IPv6 address (in
 * bytes) for the prefix argument, with prefixLengthInBits as 128.
 *
 * Examples:
 *
 * SLAAC:
 * -----
 * To configure a valid SLAAC border router, use:
 * EMBER_BORDER_ROUTER_SLAAC_FLAG
 * | EMBER_BORDER_ROUTER_DEFAULT_ROUTE_FLAG
 *
 * The preference of the SLAAC prefix can be set using
 * EMBER_BORDER_ROUTER_HIGH_PREFERENCE or EMBER_BORDER_ROUTER_LOW_PREFERENCE
 *
 * NOTE: Preferred and valid lifetimes are ignored for SLAAC prefixes.
 *       -------------------------------------------------------------
 *
 * Configuring a SLAAC prefix will trigger
 * ::emberAddressConfigurationChangeHandler on other nodes that may choose to
 * configure a SLAAC address.
 *
 * DHCP:
 * ----
 * To configure a valid DHCP border router, use:
 * EMBER_BORDER_ROUTER_DHCP_FLAG
 * | EMBER_BORDER_ROUTER_DEFAULT_ROUTE_FLAG)
 *
 * Note that this function only informs the network that this device
 * is a DHCP server.  The application is responsible for handling all
 * messages send to the DHCP server port.
 *
 * EMBER_BORDER_ROUTER_CONFIGURE_FLAG may be set if this border router is a
 * DHCP server that supplies other configurationd data, such as the identity
 * of DNS servers.
 *
 * Configuring a DHCP prefix will trigger ::emberDhcpServerChangeHandler and
 * other devices may choose to request a DHCP address by calling
 * ::emberRequestDhcpAddress. If they get an address, they are informed
 * via ::emberAddressConfigurationChangeHandler.
 *
 * @param borderRouterFlags   -> See ::EmberBorderRouterTlvFlag
 *                            -> 0x00FF to de-configure the specified prefix.
 * @param isStable            If true, the border router that uses this prefix
 *                            offers a route that is expected to be available
 *                            for at least MIN_STABLE_LIFETIME (168) hours.
 * @param prefix              Prefix for the border router.
 * @param prefixLengthInBits  Prefix length in bits.
 * @param domainId            Domain ID.
 * @param preferredLifetime   Ignored; included for backward compatibility.
 * @param validLifetime       Ignored; included for backward compatibility.
 */
void emberConfigureGateway(EmberBorderRouterTlvFlag borderRouterFlags,
                           bool isStable,
                           const uint8_t *prefix,
                           const uint8_t prefixLengthInBits,
                           uint8_t domainId,
                           uint32_t preferredLifetime,
                           uint32_t validLifetime);

/** @brief This function provides the result of a call to ::emberConfigureGateway. */
void emberConfigureGatewayReturn(EmberStatus status);

void emberSetNdData(const uint8_t *data, uint16_t length);

/** @brief This function provides the result of a call to ::emberSetNdData. */
void emberSetNdDataReturn(EmberStatus status, uint16_t length);

/** @brief External route router flags (see Thread spec chapter 5 for
 *  more information)
 */

typedef enum {
  EMBER_EXTERNAL_ROUTE_PREFERENCE_MASK      = 0xC0,
  EMBER_EXTERNAL_ROUTE_HIGH_PREFERENCE      = 0x40,
  EMBER_EXTERNAL_ROUTE_MEDIUM_PREFERENCE    = 0x00,
  EMBER_EXTERNAL_ROUTE_LOW_PREFERENCE       = 0xC0,
} EmberExternalRouteTlvFlag_e;

typedef uint8_t EmberDefaultRouteTlvFlag;

/** @brief This function defines an external route set, a route for a Thread network IPv6
 * packet that must traverse a border router and be forwarded to an exterior
 * network.
 *
 * Also de-configures an external route prefix (if it exists), if the
 * extRoutePrefix and extRoutePrefixLengthInBits arguments are specified, and
 * externalRouteFlags equals 0xFF.
 *
 * @param extRouteFlags               -> See ::EmberDefaultRouteTlvFlag
 *                                    -> 0xFF to de-configure the specified route.
 * @param isStable                    If true, the route is expected to be
 *                                    available for at least MIN_STABLE_LIFETIME
 *                                    (168) hours.
 * @param extRoutePrefix              Prefix for the route
 * @param extRoutePrefixLengthInBits  Prefix length in bits
 * @param extRouteDomainId            Domain ID
 */
void emberConfigureExternalRoute(EmberDefaultRouteTlvFlag extRouteFlags,
                                 bool isStable,
                                 const uint8_t *extRoutePrefix,
                                 uint8_t extRoutePrefixLengthInBits,
                                 uint8_t extRouteDomainId);

/** @brief This function provides the result of a call to ::emberConfigureExternalRoute */
void emberConfigureExternalRouteReturn(EmberStatus status);

/** @brief
 * Address configuration flags.
 * These flags denote the properties of a Thread IPv6 address.
 *
 * The EMBER_GLOBAL_ADDRESS_AM_ flags are set for a border router that is
 * supplying prefixes.
 *
 * The rest of the EMBER_GLOBAL_ADDRESS_ flags are set for prefixes that
 * have been administered on other devices.
 *
 * EMBER_LOCAL_ADDRESS is supplied if this a Thread mesh-local or link-local
 * IPv6 address.  No other flags are set in this case.
 */

typedef enum {
  // Flags for the device supplying prefixes.
  EMBER_GLOBAL_ADDRESS_AM_GATEWAY      = 0x01,
  EMBER_GLOBAL_ADDRESS_AM_DHCP_SERVER  = 0x02,
  EMBER_GLOBAL_ADDRESS_AM_SLAAC_SERVER = 0x04,

  // Flags for administered prefixes.
  EMBER_GLOBAL_ADDRESS_DHCP       = 0x08,
  EMBER_GLOBAL_ADDRESS_SLAAC      = 0x10,
  EMBER_GLOBAL_ADDRESS_CONFIGURED = 0x20,

  // DHCP request status flags
  EMBER_GLOBAL_ADDRESS_REQUEST_SENT   = 0x40,
  EMBER_GLOBAL_ADDRESS_REQUEST_FAILED = 0x80,

  EMBER_LOCAL_ADDRESS                 = 0xFF
} LocalServerFlag_e;

typedef uint8_t LocalServerFlag;

/** @brief
 * This function is called when a new address is configured on the application.
 *
 * If addressFlags is EMBER_LOCAL_ADDRESS, it means that the address configured
 * is a Thread-local address.
 *
 * Otherwise, it means that the address assigned is a global address (DHCP or
 * SLAAC).
 *
 * In either case, if the valid lifetime is zero, then the address is no
 * longer available.
 *
 * @param address            the address
 * @param preferredLifetime  the preferred lifetime of the address (in seconds)
 * @param validLifetime      the valid lifetime of the address (in seconds)
 * @param addressFlags       address configuration flags (see LocalServerFlag_e)
 */
void emberAddressConfigurationChangeHandler(const EmberIpv6Address *address,
                                            uint32_t preferredLifetime,
                                            uint32_t validLifetime,
                                            uint8_t addressFlags);

/** @brief
 * This function returns the list of global prefixes that we know about.
 *
 * ::emberGetGlobalPrefixReturn callbacks contain information about the border
 * routers.
 *
 * Once all valid entries have been returned, an extra zeroed-out entry is
 * returned to indicate completion.
 */
void emberGetGlobalPrefixes(void);

/** @brief This function provides the result of a call to ::emberGetGlobalPrefix.
 *
 * @param flags               Please ignore this param, it is currently unused.
 *                            (returns 0)
 * @param isStable            Stable or temporary prefix
 * @param prefix              Border router prefix
 * @param prefixLengthInBits  Prefix length in bits
 * @param domainId            Provisioning domain ID
 * @param preferredLifetime   Preferred lifetime (in seconds)
 * @param validLifetime       Valid lifetime (in seconds)
 */
void emberGetGlobalPrefixReturn(uint8_t flags,
                                bool isStable,
                                const uint8_t *prefix,
                                uint8_t prefixLengthInBits,
                                uint8_t domainId,
                                uint32_t preferredLifetime,
                                uint32_t validLifetime);

/** @brief
 * This function is called when the stack knows about a new dhcp server or if
 * a dhcp server has become unavailable.
 *
 * "available" means the DHCP server can offer us an address if
 * requested.
 *
 * @param prefix                  dhcp server prefix
 * @param prefixLengthInBits      length in bits of the prefix
 * @param available               whether this dhcp server is available
 */
void emberDhcpServerChangeHandler(const uint8_t *prefix,
                                  uint8_t prefixLengthInBits,
                                  bool available);

/** @brief
 * The application can choose to request a new DHCP address when it is
 * informed via ::emberDhcpServerChangeHandler of an available DHCP server.
 *
 * The application can also call ::emberGetGlobalPrefixes to look for
 * DHCP servers that it can request for an address.
 *
 * When the address is obtained, the application is informed of this via
 * ::emberAddressConfigurationChangeHandler.
 *
 * @param prefix                  dhcp server prefix
 * @param prefixLengthInBits      length in bits of the prefix
 */
void emberRequestDhcpAddress(const uint8_t *prefix, uint8_t prefixLengthInBits);

/** @brief
 * This function provides the result of a call to ::emberRequestDhcpAddress.
 *
 * This call only indicates the status of the request (EMBER_ERR_FATAL if no
 * DHCP server is found, and EMBER_SUCCESS otherwise).  The assigned IPv6
 * address is returned via ::emberAddressConfigurationChangeHandler
 *
 * @param status                  Status of DHCP Address Request
 * @param prefix                  Prefix requested in ::emberRequestDhcpAddress
 * @param prefixLengthInBits      Prefix length in bits requested in
 *                                ::emberRequestDhcpAddress
 */
void emberRequestDhcpAddressReturn(EmberStatus status,
                                   const uint8_t *prefix,
                                   uint8_t prefixLengthInBits);

/** @brief
 * This function is called when the stack knows about a new SLAAC prefix or if
 * a SLAAC server has become unavailable.
 *
 * "available" means we can configure a SLAAC address.
 *
 * @param prefix                  SLAAC prefix
 * @param prefixLengthInBits      length in bits of the prefix
 * @param available               whether we can configure an address
 */
void emberSlaacServerChangeHandler(const uint8_t *prefix,
                                   uint8_t prefixLengthInBits,
                                   bool available);

/** @brief
 * The application can choose to request a new SLAAC address when it is
 * informed via ::emberSlaacServerChangeHandler of an available SLAAC prefix.
 *
 * The application can also call ::emberGetGlobalPrefixes to look for
 * SLAAC prefixes that it can use to configure an address.
 *
 * If the application wants to manually configure an address and not have
 * the stack create one, then it should pass in the entire IPv6 address (in
 * bytes) for the prefix argument, with prefixLengthInBits as 128.
 *
 * When the address is obtained, the application is informed of this via
 * ::emberAddressConfigurationChangeHandler.
 *
 * @param prefix                  SLAAC prefix
 * @param prefixLengthInBits      Length in bits of the prefix
 */
void emberRequestSlaacAddress(const uint8_t *prefix, uint8_t prefixLengthInBits);

/** @brief
 * This function provides the result of a call to ::emberRequestSlaacAddress.
 *
 * This call only indicates the status of the request (EMBER_ERR_FATAL if no
 * SLAAC server is found, and EMBER_SUCCESS otherwise).  The assigned IPv6
 * address is returned via ::emberAddressConfigurationChangeHandler
 *
 * @param status                  Status of SLAAC Address Request
 * @param prefix                  Prefix requested in ::emberRequestSlaacAddress
 * @param prefixLengthInBits      Prefix length in bits requested in
 *                                ::emberRequestSlaacAddress
 */
void emberRequestSlaacAddressReturn(EmberStatus status,
                                    const uint8_t *prefix,
                                    uint8_t prefixLengthInBits);

/** @brief
 * This function returns the list of global addresses configured on this device.
 *
 * ::emberGetGlobalAddressReturn callbacks contain information about these
 * global addresses.
 *
 * Once all valid entries have been returned, an extra zeroed-out entry is
 * returned to indicate completion.
 *
 * @param prefix                  Address prefix
 * @param prefixLengthInBits      Length in bits of the prefix
 */
void emberGetGlobalAddresses(const uint8_t *prefix, uint8_t prefixLengthInBits);

/** @brief This function provides the result of a call to ::emberGetGlobalAddresses
 *
 * @param address             IPv6 global address
 * @param preferredLifetime   Preferred lifetime (in seconds)
 * @param validLifetime       Valid lifetime (in seconds)
 * @param addressFlags        Address configuration flags (EMBER_GLOBAL_ADDRESS_*)
 */
void emberGetGlobalAddressReturn(const EmberIpv6Address *address,
                                 uint32_t preferredLifetime,
                                 uint32_t validLifetime,
                                 uint8_t addressFlags);

/** @brief
 * This function resigns this IPv6 global address from this node.  If this is a DHCP address,
 * then the server is informed about it.  If it is a SLAAC address, we remove it
 * locally.
 */
void emberResignGlobalAddress(const EmberIpv6Address *address);

/** @brief
 * This function provides the result of a call to emberResignGlobalAddress().
 */
void emberResignGlobalAddressReturn(EmberStatus status);

/** @brief
 * This function is called when the stack knows about a border router that has
 * an external route to a prefix.
 *
 * @param prefix                  External route prefix
 * @param prefixLengthInBits      Length in bits of the prefix
 * @param available               Whether this external route is available.
 */
void emberExternalRouteChangeHandler(const uint8_t *prefix,
                                     uint8_t prefixLengthInBits,
                                     bool available);

/** @brief
 * This function is called when the stack receives new Thread Network Data.  The
 * networkData argument may be NULL, in which case ::emberGetNetworkData
 * can be used to obtain the new Thread Network Data.
 *
 * @param networkData             Network Data
 * @param length                  Length in bytes of the Network Data
 */
void emberNetworkDataChangeHandler(const uint8_t *networkData, uint16_t length);

/** @brief
 * This function is called to obtain the current Thread Network Data.
 *
 * @param networkDataBuffer       Network Data will be copied to here
 * @param bufferLength            Length in bytes of the buffer
 */
void emberGetNetworkData(uint8_t *networkDataBuffer, uint16_t bufferLength);

/** @brief
 * This function provides the result of a call to ::emberGetNetworkData.
 *
 * The status value is one of:
 *  - EMBER_SUCCESS
 *  - EMBER_NETWORK_DOWN
 *  - EMBER_BAD_ARGUMENT (the supplied buffer was too small)
 *
 * @param status
 * @param networkData             Location of the Network Data
 * @param dataLength              Length in bytes of the Network Data
 */
void emberGetNetworkDataReturn(EmberStatus status,
                               uint8_t *networkData,
                               uint16_t bufferLength);

/** @brief The maximum length of a domain name that may be passed
 * to emberDnsLookup().
 */

#define EMBER_MAX_DNS_NAME_LENGTH 128

/** @brief The maximum number of bytes of application data that may be
 * passed to emberDnsLookup().
 */

#define EMBER_MAX_DNS_QUERY_APP_DATA_LENGTH 64

/** @brief  Structure for returning information from a DNS lookup.
 * A structure is used to make it easier to add additional values.
 */

typedef struct {
  EmberIpv6Address ipAddress;  /*!< The returned address */
} EmberDnsResponse;

/** @brief Status values passed to DNS response handlers.
 */

typedef enum {
  EMBER_DNS_LOOKUP_SUCCESS,
  EMBER_DNS_LOOKUP_NO_BORDER_ROUTER,
  EMBER_DNS_LOOKUP_NO_BORDER_ROUTER_RESPONSE,
  EMBER_DNS_LOOKUP_BORDER_ROUTER_RESPONSE_ERROR,
  EMBER_DNS_LOOKUP_NO_DNS_SERVER,
  EMBER_DNS_LOOKUP_NO_DNS_RESPONSE,
  EMBER_DNS_LOOKUP_NO_DNS_RESPONSE_ERROR,
  EMBER_DNS_LOOKUP_NO_ENTRY_FOR_NAME,
  EMBER_DNS_LOOKUP_NO_BUFFERS,
} EmberDnsLookupStatus;

/** @brief Type definition for callback handlers for DNS responses.
 *
 * @param status A EmberDnsLookupSatus indicating success or failure.
 * @param domainName The name that was looked up.
 * @param domainNameLength Length of domainName in bytes.
 * @param response Response information, NULL if no response was received.
 * @param applicationData Application data that was passed to ::emberDnsLookup.
 * @param applicationDataLength Length of applicationData in bytes.
 */

typedef void (*EmberDnsResponseHandler)(EmberDnsLookupStatus status,
                                        const uint8_t *domainName,
                                        uint8_t domainNameLength,
                                        const EmberDnsResponse *response,
                                        void *applicationData,
                                        uint16_t applicationDataLength);

/** @brief This function initiates a DNS name lookup.
 *
 * @param domainName The name to be looked up.
 * @param domainNameLength Length of domainName in bytes.
 * @param prefix64 A 64-bit prefix that specifies the domain in which to
 *   perform the lookup.
 * @param responseHandler Handler to which the response will be passed.
 * @param applicationData Application data to be passed to responseHandler.
 * @param applicationDataLength Length of applicationData in bytes.
 */

EmberStatus emberDnsLookup(const uint8_t *domainName,
                           uint8_t domainNameLength,
                           const uint8_t *prefix64,
                           EmberDnsResponseHandler responseHandler,
                           uint8_t *appData,
                           uint16_t appDataLength);

/** @} // END addtogroup
 */

/**
 * @addtogroup form_and_join
 *
 * See network-management.h for source code.
 * @{
 */

/** @brief  The following denotes which network parameters to use when forming
 *  or joining a network.  Construct an uint16_t "options" flag for use in various
 *  network formation calls.
 */
#define EMBER_USE_DEFAULTS            0
#define EMBER_NETWORK_ID_OPTION       BIT(0)
#define EMBER_ULA_PREFIX_OPTION       BIT(1)
#define EMBER_EXTENDED_PAN_ID_OPTION  BIT(2)
#define EMBER_PAN_ID_OPTION           BIT(3)
#define EMBER_NODE_TYPE_OPTION        BIT(4)
#define EMBER_TX_POWER_OPTION         BIT(5)
#define EMBER_MASTER_KEY_OPTION       BIT(6)
#define EMBER_LEGACY_ULA_OPTION       BIT(7)
#define EMBER_JOIN_KEY_OPTION         BIT(8)

/** @brief  This function configures network parameters.
 *
 * This function assigns the network configuration values that will be used when the
 * device forms or joins a network.
 *
 * This function may only be called when the network status is EMBER_NO_NETWORK.
 * If the node was previously part of a network, use ::emberResumeNetwork() to
 * recover after a reboot.  To forget the network and return to a status
 * of EMBER_NO_NETWORK, please read cautions for ::emberResetNetworkState().
 *
 * @param parameters Some parameters may be supplied by the caller.
 *
 * @param options A bitmask indicating which network parameters are being
 * supplied by the caller.  The following list enumerates the options that
 * can be set.
 *
 * - EMBER_NETWORK_ID_OPTION
 * - EMBER_EXTENDED_PAN_ID_OPTION
 * - EMBER_ULA_PREFIX_OPTION
 * - EMBER_MASTER_KEY_OPTION
 * - EMBER_PAN_ID_OPTION
 */
void emberConfigureNetwork(const EmberNetworkParameters *parameters,
                           uint16_t options);

/** @brief  This function forms a new network.
 *
 * The forming node chooses a random extended pan ID, network ULA prefix,
 * and pan ID for the new network.  It peforms an energy scan of the channels
 * indicated by the channelMask argument and selects the one with lowest
 * detected energy.  It performs an active scan on that channel to ensure
 * there is no pan ID conflict.  ::emberFormNetworkReturn() indicates whether the form process
 * was initiated.
 * Changes to the network status resulting from the form process are reported
 * to the application via ::emberNetworkStatusHandler().
 *
 * This function may only be called when the network status is EMBER_NO_NETWORK,
 * and when a scan is not underway.
 * If the node was previously part of a network, use ::emberResumeNetwork() to
 * recover after a reboot.  To forget the network and return to a status
 * of EMBER_NO_NETWORK, read cautions for ::emberResetNetworkState().
 *
 * @param parameters Some parameters may be supplied by the caller.
 *
 * @param options A bitmask indicating which network parameters are being
 * supplied by the caller.  The following list enumerates the allowed options
 * and the default value used if the option is not specified:
 * - EMBER_NETWORK_ID_OPTION  default: ember
 * - EMBER_ULA_PREFIX_OPTION  default: random
 * - EMBER_NODE_TYPE_OPTION   default: EMBER_ROUTER
 * - EMBER_TX_POWER_OPTION    default: 3
 *
 * @param channelMask A mask indicating the channels to be scanned.
 * See ::emberStartScan() for format details.
 */
void emberFormNetwork(const EmberNetworkParameters *parameters,
                      uint16_t options,
                      uint32_t channelMask);

/** @brief A callback that indicates whether a prior call to
 * ::emberFormNetwork() successfully initiated the form process.
 * The status argument is either EMBER_SUCCSS, or EMBER_INVALID_CALL
 * if resume was called when the network status was not EMBER_NO_NETWORK,
 * or a scan was underway.
 */
void emberFormNetworkReturn(EmberStatus status);

/** @brief  This function joins an existing network.
 *
 * The joining node performs an active scan of the channels indicated by
 * the channelMask argument.  It looks for networks matching the criteria
 * specified via the supplied parameters, and which currently allow joining.
 * The status of whether the join process was initiated is reported to the
 * application via ::emberResumeNetworkReturn().  Changes to the network
 * status resulting from the join process are reported to the application via
 * ::emberNetworkStatusHandler().
 *
 * This function may only be called when the network status is EMBER_NO_NETWORK,
 * and when a scan is not underway.
 * If the node was previously part of a network, use ::emberResumeNetwork() to
 * recover after a reboot.  To forget the network and return to a status
 * of EMBER_NO_NETWORK, please read cautions for ::emberResetNetworkState().
 *
 * @param parameters Some parameters may be supplied by the caller.
 *
 * @param options A bitmask indicating which network parameters are being
 * supplied by the caller.  The following list enumerates the allowed options
 * and the default value used if the option is not specified:
 * - EMBER_NETWORK_ID_OPTION       default: looks for any network id
 * - EMBER_EXTENDED_PAN_ID_OPTION  default: looks for any extended pan id
 * - EMBER_PAN_ID_OPTION           default: looks for any pan id
 * - EMBER_NODE_TYPE_OPTION        default: EMBER_ROUTER
 * - EMBER_TX_POWER_OPTION         default: 3
 * - EMBER_JOIN_KEY_OPTION         default: empty
 *
 * @param channelMask A mask indicating the channels to be scanned.
 * See ::emberStartScan() for format details.
 */
void emberJoinNetwork(const EmberNetworkParameters *parameters,
                      uint16_t options,
                      uint32_t channelMask);

/** @brief  This function joins an already-commissioned network.
 *
 * This function assumes that commissioning data has already been cached
 * via a call to ::emberConfigureNetwork().
 *
 * @param radioTxPower        Desired radio output power, in dBm.
 * @param nodeType            Type of device.
 * @param requireConnectivity If commissioned join fails, specify whether
 *                            this node should start a new fragment.
 *                            Note: The short PAN ID MUST be commissioned
 *                                  if this is true.
 */
void emberJoinCommissioned(int8_t radioTxPower,
                           EmberNodeType nodeType,
                           bool requireConnectivity);

/** @brief  A callback that indicates whether the join process was successfully
 * initiated via a prior call to ::emberJoinNetwork() or
 * ::emberJoinCommissioned().  The possible EmberStatus values are:
 * EMBER_SUCCESS, EMBER_BAD_ARGUMENT, or EMBER_INVALID_CALL (if join was called
 * when the network status was something other than EMBER_NO_NETWORK).
 */
void emberJoinNetworkReturn(EmberStatus status);

/** @brief  This function resumes network operation after a reboot of the Ember micro.
 *
 * If the device was previously part of a network, it will recover its former
 * network parameters including pan id, extended pan id, node type, etc.
 * and resume participation in the network.  The status of whether the resume
 * process was initiated is reported to the application via
 * ::emberResumeNetworkReturn().  Changes to the network status resulting
 * from the resume process are reported to the application via
 * ::emberNetworkStatusHandler().
 *
 * This function may only be called when the network status is
 * EMBER_SAVED_NETWORK and when a scan is not underway.
 */
void emberResumeNetwork(void);

/** @brief A callback that indicates whether a prior call to
 * ::emberResumeNetwork() successfully initiated the resume process.
 * The status argument is either EMBER_SUCCESS, or
 * EMBER_INVALID_CALL if resume was called when the network status was
 * not EMBER_SAVED_NETWORK, or while a scan was underway.
 */
void emberResumeNetworkReturn(EmberStatus status);

/** @brief  On an end device, this initiates an attach with any available
 * router-eligible devices in the network.  This call must only be made if the
 * network materials have been pre-commissioned on this device, or if previously
 * completed obtaining the commissioning materials from another device.
 *
 * The status of whether the attach process was initiated is reported
 * to the application via ::emberAttachToNetworkReturn().  Changes to
 * the network status resulting from the attach process are reported
 * to the application via ::emberNetworkStatusHandler().
 *
 * This function may only be called when the network status is
 * EMBER_JOINED_NETWORK_NO_PARENT and an attach is not already underway.
 */
void emberAttachToNetwork(void);

/** @brief A callback that indicates whether the attach process was
 * successfully initiated via a prior call to ::emberAttachToNetwork().
 * The status argument is either EMBER_SUCCESS, or EMBER_INVALID_CALL
 * if attach was called when the network status was not
 * EMBER_JOINED_NETWORK_NO_PARENT, or while an attach was underway.
 */
void emberAttachToNetworkReturn(EmberStatus status);

/** @brief A callback that is generated when the host's address changes.
 *
 * @param address IP address, 16 bytes
 */
void emberSetAddressHandler(const uint8_t *address);

/** @brief A callback to the IP driver to tell it to change its address.
 *
 * @param address IP address, 16 bytes
 */
void emberSetDriverAddressHandler(const uint8_t *address);

/** @brief A callback to tell the host to start security commissioning.
 *
 * @param address parent IP address, 16 bytes
 */
void emberStartHostJoinClientHandler(const uint8_t *parentAddress);

/** @brief A callback to the IP driver to tell it the network keys.
 *
 * @param sequence sequence number
 * @param masterKey master key, 16 bytes
 * @param sequence2 second sequence number
 * @param masterKey2NotUsed second key, 16 bytes
 */
void emberSetNetworkKeysHandler(uint32_t sequence,
                                const uint8_t *masterKey,
                                uint32_t sequence2,
                                const uint8_t *masterKey2);

/** @brief A callback to provide the commission-proxy-app on the host with the
 * requisite network parameters.
 */
void emberSetCommProxyAppParametersHandler(const uint8_t *extendedPanId,
                                           const uint8_t *networkId,
                                           const uint8_t *ulaPrefix,
                                           uint16_t panId,
                                           uint8_t channel,
                                           const EmberEui64 *eui64,
                                           const EmberEui64 *macExtendedId,
                                           EmberNetworkStatus networkStatus);

/** @brief A callback to provide the commission-proxy-app on the host with the
 * requisite security material.
 */
void emberSetCommProxyAppSecurityHandler(const uint8_t *masterKey,
                                         uint32_t sequenceNumber);

/** @brief A callback to provide the commission-proxy-app on the host with
 * our mesh local address.
 */
void emberSetCommProxyAppAddressHandler(const uint8_t *address);

/** @brief A callback to provide the commission-proxy-app on the host with
 * the pskc.
 */
void emberSetCommProxyAppPskcHandler(const uint8_t *pskc);

/** @brief  This function changes the node type of a joined device.
 *
 * The device must be joined to a network prior to making this call.
 *
 * @param newType  The node type to change to.
 */
void emberChangeNodeType(EmberNodeType newType);

/** @brief This function provides the result of a call to emberChangeNodeType():
 * either EMBER_SUCCESS, or EMBER_INVALID_CALL.
 */
void emberChangeNodeTypeReturn(EmberStatus status);
/** @} // END addtogroup
 */

/**
 * @addtogroup commissioning
 *
 * See network-management.h for source code.
 * @{
 */
/** @brief This function petitions to make this device the commissioner for the network.
 * This will succeed if there is no active commissioner and fail if there
 * is one.
 *
 * @param deviceName    A name for this device as a human-readable string.
 *  If this device becomes the commissioner this name is sent to any other
 *  would-be commissioners so that the user can identify the current
 *  commissioner.
 *
 * @param deviceNameLength    The length of the name.
 */
void emberBecomeCommissioner(const uint8_t *deviceName,
                             uint8_t deviceNameLength);

/** @brief Return call for emberBecomeCommissioner().  The status is
 * EMBER_SUCCESS if a petition was sent or EMBER_ERR_FATAL if some
 * temporary resource shortage prevented doing so.
 */
void emberBecomeCommissionerReturn(EmberStatus status);

/** @brief This function causes this device to cease being the active commissioner.  This
 * call always succeeds and has no return.
 *
 * When this call is made, emberCommissionerStatusHandler will not return
 * the EMBER_AM_COMMISSIONER flag anymore.
 */
void emberStopCommissioning(void);      // no return

/** @brief This function causes the stack to call emberCommissionerStatusHandler() to
 * report the current commissioner status.  This always succeeds and has
 * no return.
 */
void emberGetCommissioner(void);        // no return

/** @brief This function causes the stack to allow a connection to a native commissioner.
 *
 * Note: This call must be made on the leader before forming a network.
 *
 * @param on  Enable / disable connections to native commissioners
 */
void emberAllowNativeCommissioner(bool on);

/** @brief This function provides the result of a call to emberAllowNativeCommissioner():
 * either EMBER_SUCCESS or EMBER_INVALID_CALL.
 */
void emberAllowNativeCommissionerReturn(EmberStatus status);

/** @brief This function sets the key that a native commissioner must use to
 * establish a connection to a Thread router.  The commissionerKey argument
 * is known as the "commissioning credential" in the Thread spec and must
 * be between 6 and 255 bytes in length.  Internally, it is hashed to derive
 * the 16-byte Pre-Shared Key for the commissioner, known as the PSKc.
 *
 * Note: This call must be made on the leader before forming a network, or
 * on an on-mesh commissioner that wants to set the PSKc in the active dataset.
 *
 * @param commissionerKey        the key
 * @param commissionerKeyLength  key length
 */
void emberSetCommissionerKey(const uint8_t *commissionerKey,
                             uint8_t commissionerKeyLength);

/** @brief This function provides the result of a call to emberSetCommissionerKey():
 * either EMBER_SUCCESS or EMBER_INVALID_CALL.
 */
void emberSetCommissionerKeyReturn(EmberStatus status);

/** @brief Handler to let application know that a PSKc TLV was successfully set.
 *
 * @param pskc                   PSKc: 16 bytes in length
 */
void emberSetPskcHandler(const uint8_t *pskc);

/**
 * @brief Flag values for emberCommissionerStatusHandler().
 */
enum {
  EMBER_NO_COMMISSIONER            = 0,
  EMBER_HAVE_COMMISSIONER          = BIT(0),
  EMBER_AM_COMMISSIONER            = BIT(1),
  EMBER_JOINING_ENABLED            = BIT(2),
  EMBER_JOINING_WITH_EUI_STEERING  = BIT(3)
};

/** @brief This function reports on the current commissioner state.
 *
 * @param flags    A combination of zero or more of the following:
 * - EMBER_HAVE_COMMISSIONER           a commissioner is active in the network
 * - EMBER_AM_COMMISSIONER             this device is the active commissioner
 *                                     if emberStopCommissioning is called, then
 *                                     this flag is not returned as we are open
 *                                     to commissioner petitions
 * - EMBER_JOINING_ENABLED             joining is enabled
 * - EMBER_JOINING_WITH_EUI_STEERING   steering data restricts which devices can
 *                                     join.  if not set, no restriction, any
 *                                     device can join (significant only when
 *                                     EMBER_JOINING_ENABLED is set)
 *
 * @param commissionerName    The name of the active commissioner, or
 *                            NULL if there is none or the name is not
 *                            known.
 *
 * @param commissionerNameLength    The length of commissonerName.
 */
void emberCommissionerStatusHandler(uint16_t flags,
                                    const uint8_t *commissionerName,
                                    uint8_t commissionerNameLength);

/**
 * @brief Joining modes, passed to emberSetJoiningMode() on the commissioner.
 * No change takes place until emberSendSteeringData() is called.
 * If steering is used, the EUI-64s of the joining devices should be passed
 * to emberAddSteeringEui64() before calling emberSendSteeringData().
 */
typedef enum {
  /** @brief Disable joining. */
  EMBER_NO_JOINING,
  /** @brief Allow joining, with no steering information. */
  EMBER_JOINING_ALLOW_ALL_STEERING,
  /** @brief Allow joining, clearing steering data. */
  EMBER_JOINING_ALLOW_EUI_STEERING,
  /** @brief Allow joining, clearing steering data. Only the low three bytes
     of EUI-64s will be used for steering. */
  /** Note: This option is deprecated in Thread 1.1. */
  EMBER_JOINING_ALLOW_SMALL_EUI_STEERING,
} EmberJoiningMode;

/** @brief This function sets the joining mode, clearing the steering data if steering
 * is to be used.
 *
 * No change takes place until emberSendSteeringData() is called.
 * If steering is used, the EUI-64s of the joining devices should be passed
 * to emberAddSteeringEui64() before calling emberSendSteeringData().
 *
 * @param mode The joining mode
 * @param length The length in bytes of the Bloom filter to be included in
 * the Steering Data TLV. This field is only applicable when mode is set to
 * EMBER_JOINING_ALLOW_EUI_STEERING. Refer to the Thread specification for
 * details on the Bloom filter and the probability of collisions given the
 * number of bits in the Bloom filter and the number of identifiers included.
 */

void emberSetJoiningMode(EmberJoiningMode mode, uint8_t length);

/** @brief This function adds the given EUI64 to the steering data if this device
 * is the active commissioner; has no effect otherwise.
 *
 * The steering data is a Bloom filter for the EUI64s of the
 * devices that are expected to join the network.  Each added EUI64 is
 * passed to a hash function to choose a set of bits in the filter,
 * and those bits are set.  Each potential joiner can then hash their
 * own EUI64 and check if the resulting bits are set in the advertised
 * filter.  If so, the device is (probably) expected to join; if not,
 * it definitely is not expected to join.
 */
void emberAddSteeringEui64(const EmberEui64 *eui64);

/** @brief This function sends the current steering data to the network, enabling
 * joining in the process.
 */
void emberSendSteeringData(void);

/** @brief This function provides the result of a call to emberSendSteeringData(). */
void emberSendSteeringDataReturn(EmberStatus status);

// Need a call to turn joining off again.
//void emberRescindSteeringData(void); ?

/** @brief This function supplies the commissioner with the key a joining node will be using.
 *
 * @param eui64     The EUI64 of the next node expected to join.  NULL
 *                  may be used if the EUI64 is not known.
 *
 * @param key       The joining key that the device will be using.
 *
 * @param keyLength The length of the joining key.
 */
void emberSetJoinKey(const EmberEui64 *eui64,
                     const uint8_t *key,
                     uint8_t keyLength);

/** @brief This function provides the result of a call to emberSetJoinKey(). */
void emberSetJoinKeyReturn(EmberStatus status);

/** @brief
 * This function allows DTLS implementations on the host.
 *
 * This call is made in order to force the host to interface with an
 * external commissioner if available, or use DTLS capabilities on the
 * host (if they exist) for Thread joining or other security handshakes.
 *
 * This is enabled by default for the Thread Border Router implementation.
 * However, if the device (Border Router or otherwise) wants to use
 * existing DTLS capabilities on the NCP stack, such as for joining,
 * this should be toggled to false.
 *
 * @param enable   If true, this call allows the host to perform DTLS.
 */
void emberEnableHostDtlsClient(bool enable);

/** @brief
 * This function commissions the network.
 *
 * This call must be made prior to calling emberJoinCommissioned().  It
 * will not be successful if the node is already on a network.
 *
 * All options except panId are REQUIRED.  If a REQUIRED option is not
 * provided, the callback emberJoinNetworkReturn will be sent to the app
 * with an EMBER_BAD_ARGUMENT status.
 *
 * Notes:  If preferredChannel is 0, EMBER_ALL_802_15_4_CHANNELS_MASK is used
 *         instead of fallbackChannelMask.  If preferredChannel is valid, it
 *         will automatically be added to the fallbackChannelMask.
 *
 * @param preferredChannel    [the preferred channel]
 * @param fallbackChannelMask [the fallback channel mask]
 * @param networkId           [the network ID]
 * @param networkIdLength     [the string length of networkId]
 * @param panId               [the short pan ID]
 * @param ulaPrefix           [the 8-byte ULA prefix]
 * @param extendedPanId       [the 8-byte extended pan ID]
 * @param key                 [the master key]
 * @param keySequence         [starting key sequence, default: 0]
 */
void emberCommissionNetwork(uint8_t preferredChannel,
                            uint32_t fallbackChannelMask,
                            const uint8_t *networkId,
                            uint8_t networkIdLength,
                            uint16_t panId,
                            const uint8_t *ulaPrefix,
                            const uint8_t *extendedPanId,
                            const EmberKeyData *key,
                            uint32_t keySequence);

/**
 * @brief This function provides the result of a call to emberCommissionNetwork.
 *
 * Returns EMBER_SUCCESS if successful
 *         EMBER_BAD_ARGUMENT if any of the options are wrong
 *         EMBER_INVALID_CALL if the node is already on a network
 *
 * @param status Whether the call to emberCommissionNetwork was successful
 */
void emberCommissionNetworkReturn(EmberStatus status);

/** @} // END addtogroup
 */

/**
 * @brief  Deprecated, not for use by Thread networks.
 * Tells the stack to allow other nodes to join the network
 * with this node as their parent.  Joining is initially disabled by default.
 * This function may only be called when the network status is EMBER_JOINED.
 *
 * @param durationSeconds  A value of 0 disables joining.
 * Any other value enables joining for that number of seconds.
 */
void emberPermitJoining(uint16_t durationSeconds);

/** @brief This function provides the result of a call to emberPermitJoining(). */
void emberPermitJoiningReturn(EmberStatus status);

/** @brief  This function informs the application when the permit joining value changes.
 *
 * @param joiningAllowed  Set to true when permit joining is allowed.
 */
void emberPermitJoiningHandler(bool joiningAllowed);

/** @brief This function sends a custom message from the Host to the NCP
 *
 * @param message message to send
 * @param messageLength length of message
 */
void emberCustomHostToNcpMessage(const uint8_t *message,
                                 uint8_t messageLength);

/** @brief NCP handler called to process a custom message from the Host.
 *
 * @param message message received
 * @param messageLength length of message
 */
void emberCustomHostToNcpMessageHandler(const uint8_t *message,
                                        uint8_t messageLength);

/** @brief This function sends a custom message from the NCP to the Host
 *
 * @param message message to send
 * @param messageLength length of message
 */
void emberCustomNcpToHostMessage(const uint8_t *message,
                                 uint8_t messageLength);

/** @brief Host handler called to process a custom message from the NCP.
 *
 * @param message message received
 * @param messageLength length of message
 */
void emberCustomNcpToHostMessageHandler(const uint8_t *message,
                                        uint8_t messageLength);

/** @brief This function sets the EUI.
 *
 * @param eui64  Value of EUI to be set.
 */
void emberSetEui64(const EmberEui64 *eui64);

/** @brief This function sends a no-op with data payload from the Host to the NCP.
 *
 * @param bytes bytes of payload
 * @param bytesLength length of payload
 */
void emberHostToNcpNoOp(const uint8_t *bytes, uint8_t bytesLength);

/** @brief This function sends a no-op with data payload from the NCP to the Host.
 *
 * @param bytes bytes of payload
 * @param bytesLength length of payload
 */
void emberNcpToHostNoOp(const uint8_t *bytes, uint8_t bytesLength);

/** @brief A callback invoked when the leader data changes.
 *
 * @param leaderData the leader data
 */
void emberLeaderDataHandler(const uint8_t *leaderData);

/** @brief This function gets a Network Data TLV.
 *
 * @param type the type for requested TLV
 * @param index if there are multiple TLVs of the given type then this value
 * indicates which one to return. A value of 0 will return the first TLV of
 * the given type.
 */
void emberGetNetworkDataTlv(uint8_t type, uint8_t index);

/** @brief This function provides the result of a call to emberGetNetworkDataTlv().
 *
 * @param type the type of TLV returned. This is the same value as
 * the value specified in the emberGetNetworkDataTlv() call.
 * @param index the instance number of the TLV. This is the same value as
 * the value specified in the emberGetNetworkDataTlv() call.
 * @param versionNumber the network data version
 * @param tlv the TLV corresponding to type or NULL.
 * @param tlvLength length of tlv
 */
void emberGetNetworkDataTlvReturn(uint8_t type,
                                  uint8_t index,
                                  uint8_t versionNumber,
                                  const uint8_t *tlv,
                                  uint8_t tlvLength);

/** @brief
 * Test command. Echo data to the NCP.
 */
void emberEcho(const uint8_t *data, uint8_t length);
void emberEchoReturn(const uint8_t *data, uint8_t length);

/** @brief Sent from the NCP to the host when an assert occurs.
 */
void emberAssertInfoReturn(const uint8_t *fileName, uint32_t lineNumber);

/** Need documentation comments for these.
 */
void emberStartXonXoffTest(void);
bool emberPing(const uint8_t *destination,
               uint16_t id,
               uint16_t sequence,
               uint16_t length,
               uint8_t hopLimit);
void emberEnableNetworkFragmentation(void);
void emberHostJoinClientComplete(uint32_t keySequence,
                                 const uint8_t *key,
                                 const uint8_t *ulaPrefix);

/** @brief Network data values.
 */
#define EMBER_NETWORK_DATA_LEADER_SIZE 8

/** @brief Maximum size of a string written by ::emberIpv6AddressToString,
 * including a NUL terminator.  It is sufficient to store the string
 * "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff" and a NUL terminator.
 */
#define EMBER_IPV6_ADDRESS_STRING_SIZE 40

/** @brief Maximum size of a string written by ::emberIpv6PrefixToString,
 * including a NUL terminator.  It is sufficient to store the string
 * "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff/128" and a NUL terminator.
 */
#define EMBER_IPV6_PREFIX_STRING_SIZE 44

/** @brief Number of bits in an IPv6 address. */
#define EMBER_IPV6_BITS 128

/** @brief Number of bytes in an IPv6 address. */
#define EMBER_IPV6_BYTES (EMBER_IPV6_BITS / 8)

/** @brief Number of fields in an IPv6 address. */
#define EMBER_IPV6_FIELDS (EMBER_IPV6_BYTES / 2)

/** @brief Size of the largest supported IPv6 packet. */
#define EMBER_IPV6_MTU 1280

/** @brief This function converts an ::EmberIpv6Address to a NUL-terminated string.
 *
 * @param src the ::EmberIpv6Address to convert
 * @param dst the buffer where the string will be written
 * @param dstSize the size of buffer in bytes
 *
 * @return true if the address was converted.
 */
bool emberIpv6AddressToString(const EmberIpv6Address *src,
                              uint8_t *dst,
                              size_t dstSize);

/** @brief This function converts an ::EmberIpv6Address and prefix length to a NUL-terminated
 *  string.
 *
 * @param src the ::EmberIpv6Address to convert
 * @param srcPrefixBits the size of the prefix in bits
 * @param dst the buffer where the string will be written
 * @param dstSize the size of buffer in bytes
 *
 * @return true if the prefix was converted.
 */
bool emberIpv6PrefixToString(const EmberIpv6Address *src,
                             uint8_t srcPrefixBits,
                             uint8_t *dst,
                             size_t dstSize);

/** @brief This function converts a NUL-terminated string to an ::EmberIpv6Address.
 *
 * @param src the string to convert
 * @param dst the ::EmberIpv6Address where the address will be written
 *
 * @return true if the string was converted.
 */
bool emberIpv6StringToAddress(const uint8_t *src, EmberIpv6Address *dst);

/** @brief This function converts a NUL-terminated string to an ::EmberIpv6Address with a
 *  prefix length.
 *
 * @param src the string to convert
 * @param dst the ::EmberIpv6Address where the address will be written
 * @param dstPrefixBits the number of prefix bits in the string
 *
 * @return true if the string was converted.
 */
bool emberIpv6StringToPrefix(const uint8_t *src,
                             EmberIpv6Address *dst,
                             uint8_t *dstPrefixBits);

/** @brief This function checks an ::EmberIpv6Address to see if it is set to all zeroes
 * which represents an unspecified address.
 *
 * @param address the ::EmberIpv6Address to check
 *
 * @return true if the address is all zeroes (unspecified address).
 */
bool emberIsIpv6UnspecifiedAddress(const EmberIpv6Address *address);

/** @brief This function checks an ::EmberIpv6Address to see if it is all zeroes, except
 * the last byte, which is set to one, representing the loopback address.
 *
 * @param address the ::EmberIpv6Address to check
 *
 * @return true if the address is zero for bytes 0-14 and one for byte 15
 */
bool emberIsIpv6LoopbackAddress(const EmberIpv6Address *address);

/** @brief This function enables or disables Radio HoldOff support.
 *
 * @param enable  When true, configures ::RHO_GPIO in BOARD_HEADER
 * as an input which, when asserted, will prevent the radio from
 * transmitting.  When false, configures ::RHO_GPIO for its original
 * default purpose.
 */
void emberSetRadioHoldOff(bool enable);

/** @brief
 * This function provides the result of a call to ::emberSetRadioHoldOff.
 *
 * @param status An EmberStatus value indicating success or the
 *               reason for failure. EMBER_SUCCESS if Radio HoldOff
 *               was configured as desired or EMBER_BAD_ARGUMENT
 *               if requesting it be enabled but RHO has not been
 *               configured by the BOARD_HEADER.
 */
void emberSetRadioHoldOffReturn(EmberStatus status);

/** @brief
 * This function fetches whether packet traffic arbitration is enabled or disabled.
 */
void emberGetPtaEnable(void);

/** @brief
 * This function provides the result of a call to ::emberGetPtaEnable.
 *
 * @param enabled When true, indicates packet traffic arbitration
 * is enabed. When false, indicates packet traffic arbitration is
 * disabled.
 */
void emberGetPtaEnableReturn(bool enabled);

/** @brief
 * This function enables or disables packet traffic arbitration.
 *
 * @param enabled When true, enables packet traffic arbitration.
 * When false, disables packet traffic arbitration.
 */
void emberSetPtaEnable(bool enabled);

/** @brief
 * This function provides the result of a call to ::emberSetPtaEnable.
 *
 * @param status An EmberStatus value indicating success or the
 *               reason for failure.
 */
void emberSetPtaEnableReturn(EmberStatus status);

/** @brief
 * This function fetches packet traffic arbitration configuration options.
 */
void emberGetPtaOptions(void);

/** @brief
 * This function provides the result of a call to ::emberGetPtaOptions.
 *
 * @param indicates packet traffic arbitration options
 * bit field.
 * Field                              Bit Position      Size(bits)
 * RX retry timeout ms                0                 8
 * Enable ack radio holdoff           8                 1
 * Abort mid TX if grant is lost      9                 1
 * TX request is high priority        10                1
 * RX request is high prioirity       11                1
 * RX retry request is high priority  12                1
 * RX retry request is enabled        13                1
 * Radio holdoff is enabled           14                1
 * Toggle request on mac retransmit   15                1
 * Reserved                           16                15
 * Hold request across CCA failures   31                1
 */
void emberGetPtaOptionsReturn(uint32_t options);

/** @brief
 * This function configures packet traffic arbitration options.
 *
 * @param indicates packet traffic arbitration options
 * bit field.
 * Field                              Bit Position      Size(bits)
 * RX retry timeout ms                0                 8
 * Enable ack radio holdoff           8                 1
 * Abort mid TX if grant is lost      9                 1
 * TX request is high priority        10                1
 * RX request is high prioirity       11                1
 * RX retry request is high priority  12                1
 * RX retry request is enabled        13                1
 * Radio holdoff is enabled           14                1
 * Toggle request on mac retransmit   15                1
 * Reserved                           16                15
 * Hold request across CCA failures   31                1
 */
void emberSetPtaOptions(uint32_t options);

/** @brief
 * This function provides the result of a call to ::emberSetPtaOptions.
 *
 * @param status An EmberStatus value indicating success or the
 *               reason for failure.
 */
void emberSetPtaOptionsReturn(EmberStatus status);

/** @brief
 * This function fetches the current antenna mode.
 */
void emberGetAntennaMode(void);

/** @brief
 * This function provides the result of a call to ::emberGetAntennaMode.
 *
 * @param status An EmberStatus value indicating success or the
 *               reason for failure.
 * @param mode the current antenna mode, 0-primary,
 *             1-secondary, 2-toggle on tx ack fail
 */
void emberGetAntennaModeReturn(EmberStatus status,
                               uint8_t mode);

/** @brief
 * This function configures the antenna mode.
 *
 * @param mode 0-primary, 1-secondary, 2-toggle on tx ack fail
 */
void emberSetAntennaMode(uint8_t mode);

/** @brief
 * This function provides the result of a call to ::emberSetAntennaMode.
 *
 * @param EMBER_SUCCESS if antenna mode is configured as desired
 * or EMBER_BAD_ARGUMENT if antenna mode is not supported.
 */
void emberSetAntennaModeReturn(EmberStatus status);

/** @brief
 * This function gets a true random number out of radios that support this.
 * This will typically take a while, and so should be used to
 * seed a PRNG and not as a source of random numbers for regular
 * use.
 *
 * @param count - the count of uint16_t values to be returned.
 */
void emberRadioGetRandomNumbers(uint8_t count);

/** @brief
 * This function provides the result of a call to ::emberRadioGetRandomNumbers.
 *
 * @param status An EmberStatus value indicating success or the
 *               reason for failure. When EMBER_SUCCESS is returned
 *               ::rn and ::count will contain valid data.  ::rn
                 and ::count are undefined when EMBER_SUCCESS is not
 *               returned.
 * @param rn the uint16_t random values
 * @param count - the count of uint16_t values located at ::rn
 */
void emberRadioGetRandomNumbersReturn(EmberStatus status,
                                      const uint16_t *rn,
                                      uint8_t count);

/** @brief
 * Callback informing the application running on the micro of interruptions
 * to normal processing. If ::busy is true, the micro will be busy processing
 * and unavailable for an indefinite period of time. If ::busy is false, the
 * micro has resumed normal operation.  The main use case is jpake crypto
 * on EM3xx processors. This gives the application a chance to prepare for
 * the pause in regular processing.
 *
 * This callback is not available on a host processor.
 * Note that if ::busy is true, the micro may become busy as soon as this
 * handler exits. In a host/ncp setup, one solution for informing the
 * host is to implement this handler in your own xNCP image and use it to
 * toggle the serial CTS line.
 */
void emberMicroBusyHandler(bool busy);
#endif
