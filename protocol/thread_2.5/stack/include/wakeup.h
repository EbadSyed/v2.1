/**
 * @file wakeup.h
 * @brief A system for waking up a collection of nodes with a very low
 * duty cycle.
 */

#ifndef __WAKEUP_H__
#define __WAKEUP_H__

/** @cond EMBER_WAKEUP_STACK */

/**
 * @addtogroup wakeup_utilities
 *
 * See wakeup.h for source code.
 * @{
 */

/** @brief Enumerate the various "sleep" types for the chip.
 *
 * See section: "Host and NCP Sleep Communication" for more information.
 */
typedef enum {
  EMBER_NO_SLEEP = 0,
  EMBER_SLEEP,
  EMBER_SLEEP_NO_TIMER
} EmberSleepType;

/**
 * @section Host and NCP Sleep Communication
 *
 * When a host sends a SLEEP_NO_TIMER_COMMAND to an NCP, the following occurs:
 *
 * Host sends SLEEP_NO_TIMER_COMMAND
 * NCP sends CB_SLEEP_NO_TIMER_COMMAND
 *
 * If the communication link between the Host and NCP is SPI, then as soon as
 * the NCP finishes sending CB_SLEEP_NO_TIMER_COMMAND, it will allow itself to
 * sleep.
 *
 * If the communication link between the Host and NCP is a UART:
 *   The Host's ASH ACKs the CB_SLEEP_NO_TIMER_COMMAND.
 *   NCP receives the ACK and clears CB_SLEEP_NO_TIMER_COMMAND from its UART
 *   buffers.
 *   Since its UART buffers are now empty, it will allow itself to sleep.
 *
 * For the LISTEN_COMMAND, the behavior is the same as in SLEEP_NO_TIMER_COMMAND,
 * except the NCP sends a  CB_LISTEN_COMMAND in response instead of
 * CB_SLEEP_NO_TIMER_COMMAND.
 */

/** @brief  Instructs the stack to listen for special wakeup messages and
 * to participate in relaying such messages upon receipt in order to wake
 * up sleeping neighbors.
 *
 * On routers, the sleep argument is ignored.  On lurkers, if sleep is
 * EMBER_SLEEP, the device powers down the micro and radio when not actively
 * listening. If the sleep is EMBER_SLEEP_NO_TIMER, the device will awaken
 * when UART traffic is received.
 *
 * emberListenForWakeup can be called repeatedly if the sleep mode needs to
 * be changed.  However, calling it while a wake is in progress will fail
 * with no change to any internal state.
 */
void emberListenForWakeup(EmberSleepType sleep);

/** @brief Provides the result of a call to emberListenForWakeup(). */
void emberListenForWakeupReturn(EmberStatus status);

/** @brief Host only. A notification that the NCP received the sleep_no_timer
 * command.
 */
void emberSleepNoTimerReturn(void);

/** @brief  Initiates the process of waking up the network.  The device
 * broadcasts wakeup messages for the configured wakeup period.  The device
 * remains awake during and after the wakeup period.  The supplied dataByte
 * is included in wakeup messages sent by the device, and is reported
 * to awakened applications via the ::emberWakeupHandler().
 */
void emberStartWakeup(uint8_t dataByte);

/** @brief Provides the result of a call to emberStartWakeup(). */
void emberStartWakeupReturn(EmberStatus status);

/** @brief Aborts the wakeup process and changes the wakeup state to
 * EMBER_WAKEUP_STATE_NOT_LISTENING.  The device will no longer be listening
 * or sleeping.
 */
void emberAbortWakeup(void);

/** @brief Provides the result of a call to emberAbortWakeup(). */
void emberAbortWakeupReturn(EmberStatus status);

/** @brief Enumerate the various states of the device in a low power listening
 * and alarm scheme.
 */
enum {
  /** The device is currently listening for wakeup messages. */
  EMBER_WAKEUP_STATE_LISTENING,
  /** The device is currently participating in a wakeup (transmitting wakeup
   * packets for the configured duration) with other devices. */
  EMBER_WAKEUP_STATE_TRANSMITTING,
  /** The device is in an "idle" state, not listening for wakeup messages. */
  EMBER_WAKEUP_STATE_NOT_LISTENING
};

/** @brief "Wakeup state" data type. */
typedef uint8_t EmberWakeupState;

/** @brief Enumerate the various wakeup states. */
typedef enum {
  /** The device was woken up by an over the air wakeup packet. */
  EMBER_WAKEUP_REASON_OVER_THE_AIR,
  /** The device was woken up by an interrupt. */
  EMBER_WAKEUP_REASON_INTERRUPT,
  /** The device has been told to abort the propagating of wakeup packets. */
  EMBER_WAKEUP_REASON_ABORTED
} EmberWakeupReason;

/** @brief  Notifies the application of a change to the wakeup state, and
 * the reason for the change.  If this handler is called, the Ember chip
 * is no longer sleeping and will not return to sleep unless
 * emberListenForWakeup(true) is called again.
 *
 * The reason and state arguments distinguish between a number of
 * different cases.
 *
 * A reason of EMBER_WAKEUP_REASON_OVER_THE_AIR means that a wakeup message
 * was received over the air.  In this case, a state of
 * EMBER_WAKEUP_STATE_TRANSMITTING means that the wakeup message was just
 * received and the device is now in the process of relaying the message for
 * the number of milliseconds indicated by remainingMs.  The dataByte
 * argument supplies the data byte from the received wakeup message.
 * A state of EMBER_WAKEUP_STATE_NOT_LISTENING means that the wakeup process
 * is over. In this case, remainingMs will be 0, and dataByte still supplies
 * the received data byte.
 *
 * A reason of EMBER_WAKEUP_REASON_INTERRUPT means that a call was previously
 * made to emberListenForWakeup(false) or emberStartWakeup() via an interrupt.
 * In the former case, the the state will be EMBER_WAKEUP_STATE_LISTENING and
 * the other arguments should be ignored. In the latter case, the state
 * argument will be either EMBER_WAKEUP_STATE_TRANSMITTING or
 * EMBER_WAKEUP_STATE_NOT_LISTENING, just as in the over-the-air case.  The
 * dataByte argument will be the byte that was supplied during the call
 * to emberStartWakeup().
 *
 * A reason of EMBER_WAKEUP_REASON_ABORTED means a call was previously made to
 * emberAbortWakeup().  In this case, the state will be
 * EMBER_WAKEUP_STATE_NOT_LISTENING and the other arguments should be ignored.
 */
void emberWakeupHandler(EmberWakeupReason reason,
                        EmberWakeupState state,
                        uint16_t remainingMs,
                        uint8_t dataByte,
                        uint16_t otaSequence);

/** @brief  Configures wakeup system parameters.  Don't use this function
 * unless you know what you are doing.  The values for cca mode are
 * 0=off, 1=off for initiator, 2=on.  The defaults are:
 *
 * onUs:                       3000 us
 * offMs:                      3997 ms
 * totalDurationMs:           21000 ms
 * phase1DurationMs:           4100 ms
 * phase1DurationInitiatorMs:  8100 ms
 * phase1CcaMode:              0 (off)
 * phase1TxDelayUs:             500 us
 * phase1JitterExpUs:       9 (500 ms)
 * phase2CcaMode:               2 (on)
 * phase2TxDelayUs:            1000 us
 * phase2JitterExpUs:                0
 */
void emberConfigureWakeup(uint16_t onUs,
                          uint16_t offMs,
                          uint16_t totalDurationMs,
                          uint16_t phase1DurationMs,
                          uint16_t phase1DurationInitiatorMs,
                          uint8_t  phase1CcaMode,
                          uint16_t phase1TxDelayUs,
                          uint8_t  phase1JitterExpUs,
                          uint8_t  phase2CcaMode,
                          uint16_t phase2TxDelayUs,
                          uint8_t  phase2JitterExpUs);

/** @brief Specifies whether this device propagates wakeup packets after being
 * woken up by an over the air packet.  This feature is enabled by default.
 *
 * If the argument is false, then the device will not propagate wakeup packets.
 * This applies to both routers and lurkers.  This argument is not relevant for
 * the sleep types EMBER_NO_SLEEP and EMBER_SLEEP_NO_TIMER.
 *
 * Note: All devices propagate wakeup synchronization packets and sequence
 * reset packets regardless of this feature being turned on.  Synchronization
 * packets are essential to keep the neighbor tables and frame counters up to
 * date.  Sequence reset packets are sent in response to devices sending wakeup
 * packets with bad or out-of-date sequence numbers.
 *
 * emberActiveWakeup can be called repeatedly in case the feature needs to be
 * turned on or off as desired.
 */
void emberActiveWakeup(bool enableActiveWakeup);

/** @brief Provides the result of a call to emberActiveWakeup(). */
void emberActiveWakeupReturn(EmberStatus status);

/**
 * @brief  Prepares the stack for sleep, then calls halSleep(SLEEPMODE_NOTIMER).
 * The sleep timer clock sources (both RC and XTAL) are turned off.
 * Wakeup is possible from only GPIO.  System time is lost.
 * This function is completely untested.  In particular, the loss of system time
 * may have an adverse affect on stack operation. Therefore, rebooting the chip
 * and resuming the network (if applicable) is necessary to make sure that the
 * device is in a valid state after waking up.
 */
void emberSleepNoTimer(void);

/**
 * @brief Get the wakeup state
 */
void emberWakeupState(void);

/** @brief Provides the result of a call to emberWakeupState(). */
void emberWakeupStateReturn(uint8_t wakeupState, uint16_t wakeupSequenceNumber);

/**
 * @brief Wake up the NCP
 */
void emberFfWakeup(void);

// Filtering mechanism to prioritize legacy traffic (on lurker
// network) over other IP traffic.
//
// The maximum and minimum thresholds are percentages of the heap size.

/** @brief  Set a maximum threshold to prioritize legacy traffic (on the lurker
 * network) over other IP traffic.
 *
 * @param max The maximum threshold as a percentage value indicating heap usage
 * relative to the heap size.
 *
 * For example, if max is 80, then non-legacy traffic will be filtered out if
 * the heap usage is more than 80% of the heap size.
 */
void emberSetLegacyFilterMaxThreshold(uint8_t max);
void emberSetLegacyFilterMaxThresholdReturn(EmberStatus status);

/** @brief  Set a minimum threshold to stop prioritizing legacy traffic (on the
 * lurker network) over other IP traffic.
 *
 * @param min The minimum threshold as a percentage value indicating heap usage
 * relative to the heap size.
 *
 * For example, if min is 30, then non-legacy traffic will stop being filtered
 * only if:
 * -> filtering is already turned on
 * -> heap usage goes below 30% of the heap size.
 */
void emberSetLegacyFilterMinThreshold(uint8_t min);
void emberSetLegacyFilterMinThresholdReturn(EmberStatus status);

/** @} // END addtogroup
 */

/** @endcond */

#endif // __WAKEUP_H__
