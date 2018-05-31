/** @file mfglib.h
 * See @ref mfglib for documentation.
 *
 * <!-- Copyright 2017 Silicon Laboratories, Inc.
 */

/**
 * @addtogroup mfglib
 * @brief This is a manufacturing and functional test library for
 * testing and verifying the RF component of products at manufacture time.
 *
 * See mfglib.h for source code.
 *
 * Developers can optionally include this library in their
 * application code. The goal is that in most cases, this will
 * eliminate the need for developers to load multiple images into
 * their hardware at manufacturing time.
 *
 * This library can optionally be compiled into the developer's
 * production code and run at manufacturing time. Any interface to
 * the library is handled by the application.
 *
 * This library cannot assist in hardware start up.
 *
 * Many functions in this file return an ::EmberStatus value.
 *
 * This is a universal library for both SoCs and hosts. Since host-NCP
 * communication involves communication of return values from NCP to host,
 * the return types for some of these methods on the host are voids. In
 * this case, we use the corresponding ...Return() methods so the NCP
 * can return status to the host.
 *
 * To account for the differences between the host and SoC interfaces
 * the following macros are defined.
 *
 * Host apps will have these defines:
 * @code
 * #define MfgStatus    void
 * #define MfgStatus_U  void
 * #define MfgStatus_UU void
 * #define MfgStatus_S  void
 * @endcode
 *
 * SoC apps will have these defines:
 * @code
 * #define MfgStatus    EmberStatus
 * #define MfgStatus_U  uint8_t
 * #define MfgStatus_UU uint16_t
 * #define MfgStatus_S  int8_t
 * @endcode
 *
 * See error-def.h for definitions of all  ::EmberStatus values.
 * @{
 */

#ifndef __MFGLIB_H__
#define __MFGLIB_H__

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#if ((defined(RTOS) && !defined(IP_MODEM_LIBRARY)) \
  || (defined(UNIX_HOST) || defined(UNIX_HOST_SIM)))
  #define MfgStatus    void
  #define MfgStatus_U  void
  #define MfgStatus_UU void
  #define MfgStatus_S  void
#else
  #define MfgStatus    EmberStatus
  #define MfgStatus_U  uint8_t
  #define MfgStatus_UU uint16_t
  #define MfgStatus_S  int8_t
#endif
#endif // DOXYGEN_SHOULD_SKIP_THIS

/** @brief Activates use of @ref mfglib test routines and enables
 * the radio receiver to report packets it receives to the
 * caller-specified mfglibRxCallback() routine.
 *
 * It is legal to pass in a NULL.
 * These packets will not be passed up with a CRC failure.
 * The first byte of the packet in the callback is the length.
 * All other functions will return an error until
 * mfglibStart() has been called.
 *
 * @appusage Use this function to enter test mode.
 *
 * Note: This function should only be called shortly after
 *   initialization and prior to forming or joining a network.
 *
 * @param mfglibRxCallback  Function pointer to callback routine. On SoCs this
 * function is invoked whenever a valid packet is received. On Hosts, in order
 * not to flood the serial connection between the NCP and the host, this
 * function is called once for every ::EMBER_MFG_RX_NCP_TO_HOST_INTERVAL
 * packets. ::EMBER_MFG_RX_NCP_TO_HOST_INTERVAL is defined in
 * ember-configuration-defaults.h and can be modified when building NCP images.
 * The default value is 50. The total number of packets received is returned
 * in the mfglibEndReturn() callback. emberTick() must be called routinely for
 * this callback to function correctly.
 *
 * @return For host apps see mfglibStartReturn(). For SoC apps one of the following:
 * - ::EMBER_SUCCESS              if the mfg test mode has been enabled.
 * - ::EMBER_ERR_FATAL            if the mfg test mode is not available.  */
MfgStatus mfglibStart(void (*mfglibRxCallback)(uint8_t *packet, uint8_t linkQuality, int8_t rssi));

/** @brief Deactivates use of @ref mfglib test routines.
 *
 * This restores the hardware to the state it was in prior to mfglibStart() and
 * stops receiving packets started by mfglibStart() at the same time.
 *
 * @appusage Use this function to exit the mfg test mode.
 *
 * Note: It may be desirable to also reboot after use of manufacturing
 *   mode to ensure all application state is properly re-initialized.
 *
 * @return For host apps see mfglibEndReturn(). For SoC apps one of the following:
 * - ::EMBER_SUCCESS           if the mfg test mode has been exited.
 * - ::EMBER_ERR_FATAL         if the mfg test mode cannot be exited.  */
MfgStatus mfglibEnd(void);

/** @brief Starts transmitting the tone feature of the radio.
 *
 * In this mode, the radio will transmit an unmodulated
 * tone on the currently set channel and power level.  Upon
 * successful return, the tone will be transmitting.  To stop
 * transmitting a tone, the application must call mfglibStopTone(),
 * allowing it the flexibility to determine its own criteria for tone
 * duration, such as time, event, and so on.
 *
 * @appusage Use this function to transmit a tone.
 *
 * @return For host apps see mfglibStartToneReturn(). For SoC apps one of the following:
 * - ::EMBER_SUCCESS          if the transmit tone has started.
 * - ::EMBER_ERR_FATAL        if the tone cannot be started.  */
MfgStatus mfglibStartTone(void);

/** @brief Stops transmitting a tone started by mfglibStartTone().
 *
 * @appusage Use this function to stop transmitting a tone.
 *
 * @return For host apps see mfglibStopToneReturn(). For SoC apps one of the following:
 * - ::EMBER_SUCCESS          if the transmit tone has stopped.
 * - ::EMBER_ERR_FATAL        if the tone cannot be stopped.  */
MfgStatus mfglibStopTone(void);

/** @brief Starts transmitting a random stream of characters.
 * This is so that the radio modulation can be measured.
 *
 * @appusage Use this function to enable the measurement of radio
 * modulation.
 *
 * @return For host apps see mfglibStartStreamReturn(). For SoC apps one of the following:
 * - ::EMBER_SUCCESS          if the transmit stream has started.
 * - ::EMBER_ERR_FATAL        if the stream cannot be started.  */
MfgStatus mfglibStartStream(void);

/** @brief Stops transmitting a random stream of characters started by
 * mfglibStartStream().
 *
 * @appusage Use this function to end the measurement of radio
 * modulation.
 *
 * @return For host apps see mfglibStopStreamReturn(). For SoC apps one of the following:
 * - ::EMBER_SUCCESS          if the transmit stream has stopped.
 * - ::EMBER_ERR_FATAL        if the stream cannot be stopped.  */
MfgStatus mfglibStopStream(void);

/** @brief Sends a single packet, (repeat + 1) times.
 *
 * @appusage Use this function to send raw data. Note that <em>packet</em> array
 * must be word-aligned (begin at even address), such that
 * <em>((((uint16_t)packet) & 1) == 0)</em> holds true.  (This is generally done
 * by either declaring <em>packet</em> as a local variable or putting it in a
 * global declaration immediately following the declaration of an uint16_t.)
 *
 * @param packet  Packet to be sent.
 * First byte of the packet is always the length byte, whose value does not
 * include itself but does include the 16-bit CRC in the length calculation.
 * The CRC gets appended automatically by the radio as it transmits the packet,
 * so the host does not need to provide this as part of packetContents.
 * The total length of packet contents (Length Byte+1) going out the radio
 * should not be >128 or <6 bytes.
 * Note that the packet array should not include the CRC, as this appended
 * by the radio automatically.
 *
 * @param repeat  Number of times to repeat sending the packet
 * after having been sent once. A value of 0 means send once and don't repeat.
 *
 * @return For host apps see mfglibSendPacketReturn(). For SoC apps one of the following:
 * - ::EMBER_SUCCESS                 if the packet was sent.
 * - ::EMBER_ERR_FATAL               if the mfg test mode is not available or TONE or STREAM test is running.*/
MfgStatus mfglibSendPacket(uint8_t * packet, uint16_t repeat);

/** @brief Selects the radio channel.  The channel range is from 11 to 26.
 *
 * Customers can set any valid channel they want.
 * Calibration occurs if this is the first time after power up.
 *
 * @appusage Use this function to change channels.
 *
 * @param channel  Valid values depend upon the radio used.
 *
 * @return For host apps see mfglibSetChannelReturn(). For SoC apps one of the following:
 * - ::EMBER_SUCCESS                 if the channel has been set.
 * - ::EMBER_PHY_INVALID_CHANNEL     if the channel requested is invalid.
 * - ::EMBER_ERR_FATAL               if the mfg test mode is not available or TONE or STREAM test is running.*/
MfgStatus mfglibSetChannel(uint8_t channel);

/** @brief Get the current radio channel, as previously
 * set via mfglibSetChannel().
 *
 * @appusage Use this function to get current channel.
 *
 * @return For host apps see mfglibGetChannelReturn(). For SoC apps the current channel. */
MfgStatus_U mfglibGetChannel(void);

/** @brief Set the transmit power mode and the radio transmit power.
 *
 * Valid power settings depend upon the specific radio in use.  Silabs
 * radios have discrete power settings, and then requested power is
 * rounded to a valid power setting. The actual power output is
 * available to the caller via mfglibGetPower().
 *
 * @appusage Use this function to adjust the transmit power.
 *
 * @param txPowerMode  boost mode or external PA.
 *
 * @param power        Power in units of dBm, which can be negative.
 *
 * @return For host apps see mfglibSetPowerReturn(). For SoC apps one of the following:
 * - ::EMBER_SUCCESS                 if the power has been set.
 * - ::EMBER_ERROR_INVALID_POWER     if the power requested is invalid.
 * - ::EMBER_ERR_FATAL               if the mfg test mode is not available or TONE or STREAM test is running.*/
MfgStatus mfglibSetPower(uint16_t txPowerMode, int8_t power);

/** @brief Get the current radio power setting as
 * previously set via mfglibSetPower().
 *
 * @appusage Use this function to get current power setting.
 *
 * @return For host apps see mfglibGetPowerReturn(). For SoC apps the current power setting. */
MfgStatus_S mfglibGetPower(void);

/** @brief Get the radio transmit power mode setting as
 * previously set via mfglibSetPower().
 *
 * @appusage Use this function to get current power mode setting.
 *
 * @return For host apps see mfglibGetPowerModeReturn(). For SoC apps the current power mode setting. */
MfgStatus_UU mfglibGetPowerMode(void);

/** @brief Set the synth offset in 11.7kHz steps.
 *
 * This function does NOT
 * write the new synth offset to the token, it only changes it in memory.  It
 * can be changed as many times as you like, and the setting will be lost when
 * a reset occurs.  The value will survive deep sleep, but will not survive a
 * reset, thus it will not take effect in the bootloader.  If you would like it
 * to be permanent (and accessible to the bootloader), you must write the
 * TOKEN_MFG_SYNTH_FREQ_OFFSET token using the token API or em3xx_load --patch.
 *
 * @appusage Use this function to compensate for tolerances in the crystal
 * oscillator or capacitors.  This function does not effect a permanent change;
 * once you have found the offset you want, you must write it to a token using
 * the token API for it to be permanent.
 *
 * @param synOffset the number of 11.7kHz steps to offset the carrier frequency
 * (may be negative)
 *
 * @return None.
 */
void mfglibSetSynOffset(int8_t synOffset);

/** @brief Get the current synth offset in 11.7kHz steps.
 *
 * See mfglibSetSynOffset() for details.
 *
 * @appusage Use this function to get the current setting for tolerances in the
 * crystal oscillator or capacitors.
 *
 * @return For host apps see mfglibGetSynOffsetReturn(). For SoC apps the synth
 * offset in 11.7kHz steps
 */
MfgStatus_S mfglibGetSynOffset(void);

/** @brief Run mod DAC calibration on the given channel for
 * the given amount of time.
 *
 * If the duration argument == 0, this test will run forever (until the
 * chip is reset).
 *
 * @appusage Use this function to run the active transmit part of
 * mod DAC calibration.
 *
 * @param channel   Selects the channel to transmit on.
 *
 * @param duration  Duration in ms, 0 == infinite.
 */
void mfglibTestContModCal(uint8_t channel, uint32_t duration);

/** @brief Set manufacturing library options.
 *
 * @appusage Use this function to set manufacturing library options.
 *
 * @param options bitmask.  0 == non-CSMA transmits, 1 == CSMA transmits
 *
 * @return For host apps see mfglibSetOptionsReturn(). For SoC apps one of the following:
 * - ::EMBER_SUCCESS                 if the options have been set.
 * - ::EMBER_BAD_ARGUMENT            if any options are unavailable.
 * - ::EMBER_ERR_FATAL               if the mfg test mode is not available or TONE or STREAM test is running.*/
MfgStatus mfglibSetOptions(uint8_t options);

/** @brief Get the current manufacturing library options, as previously
 * set via mfglibSetOptions().
 *
 * @appusage Use this function to get library options.
 *
 * @return For host apps see mfglibGetOptionsReturn(). For SoC apps the current test mode. */
MfgStatus_U mfglibGetOptions(void);

/** @brief This function provides the result of a call to mfglibStart().
 *
 * @param status
 * - ::EMBER_SUCCESS              if the mfg test mode has been enabled.
 * - ::EMBER_ERR_FATAL            if the mfg test mode is not available.
 */
void mfglibStartReturn(EmberStatus status);

/** @brief This function provides the result of a call to mfglibEnd().
 *
 * @param status
 * - ::EMBER_SUCCESS           if the mfg test mode has been exited.
 * - ::EMBER_ERR_FATAL         if the mfg test mode cannot be exited.
 * @param receiveCount The total number of packets received during the test.
 */
void mfglibEndReturn(EmberStatus status, uint32_t receiveCount);

/** @brief This function provides the result of a call to mfglibStartTone().
 *
 * @param status
 * - ::EMBER_SUCCESS          if the transmit tone has started.
 * - ::EMBER_ERR_FATAL        if the tone cannot be started.
 */
void mfglibStartToneReturn(EmberStatus status);

/** @brief This function provides the result of a call to mfglibStopTone().
 *
 * @param status
 * - ::EMBER_SUCCESS          if the transmit tone has stopped.
 * - ::EMBER_ERR_FATAL        if the tone cannot be stopped.
 */
void mfglibStopToneReturn(EmberStatus status);

/** @brief This function provides the result of a call to mfglibStartStream().
 *
 * @param status
 * - ::EMBER_SUCCESS          if the transmit stream has started.
 * - ::EMBER_ERR_FATAL        if the stream cannot be started.
 */
void mfglibStartStreamReturn(EmberStatus status);

/** @brief This function provides the result of a call to mfglibStopStream().
 *
 * @param status
 * - ::EMBER_SUCCESS          if the transmit stream has stopped.
 * - ::EMBER_ERR_FATAL        if the stream cannot be stopped.
 */
void mfglibStopStreamReturn(EmberStatus status);

/** @brief This function provides the result of a call to mfglibSendPacket().
 *
 * @param status
 * - ::EMBER_SUCCESS                 if the packet was sent.
 * - ::EMBER_ERR_FATAL               if the mfg test mode is not available or
 *                                   TONE or STREAM test is running.
 */
void mfglibSendPacketReturn(EmberStatus status);

/** @brief This function provides the result of a call to mfglibSetChannel().
 *
 * @param status
 * - ::EMBER_SUCCESS                 if the channel has been set.
 * - ::EMBER_PHY_INVALID_CHANNEL     if the channel requested is invalid.
 * - ::EMBER_ERR_FATAL               if the mfg test mode is not available or
 *                                   TONE or STREAM test is running.
 */
void mfglibSetChannelReturn(EmberStatus status);

/** @brief This function provides the result of a call to mfglibGetChannel().
 *
 * @param channel The current channel.
 */
void mfglibGetChannelReturn(uint8_t channel);

/** @brief This function provides the result of a call to mfglibSetPower().
 *
 * @param status
 * - ::EMBER_SUCCESS                 if the power has been set.
 * - ::EMBER_ERROR_INVALID_POWER     if the power requested is invalid.
 * - ::EMBER_ERR_FATAL               if the mfg test mode is not available or
 *                                   TONE or STREAM test is running.
 */
void mfglibSetPowerReturn(EmberStatus status);

/** @brief This function provides the result of a call to mfglibGetPower().
 *
 * @param power The current power setting.
 */
void mfglibGetPowerReturn(int8_t power);

/** @brief This function provides the result of a call to mfglibGetPowerMode().
 *
 * @param txPowerMode The current power mode setting.
 */
void mfglibGetPowerModeReturn(uint16_t txPowerMode);

/** @brief This function provides the result of a call to mfglibGetSynOffset().
 *
 * @param synthOffset The synth offset in 11.7kHz steps.
 */
void mfglibGetSynOffsetReturn(int8_t synthOffset);

/** @brief This function provides the result of a call to mfglibSetOptions().
 *
 * @param status
 * - ::EMBER_SUCCESS                 if the options have been set.
 * - ::EMBER_BAD_ARGUMENT            if any options are unavailable.
 * - ::EMBER_ERR_FATAL               if the mfg test mode is not available or TONE or STREAM test is running.
 */
void mfglibSetOptionsReturn(EmberStatus status);

/** @brief This function provides the result of a call to mfglibGetOptions().
 *
 * @param options The current options based on the current test mode.
 */
void mfglibGetOptionsReturn(uint8_t options);

/** @brief RX Handler for the mfglib test library
 *
 * @param packet       incoming packet
 * @param linkQuality  link quality as a numeric value
 * @param rssi         RSSI in dBm
 */
void mfglibRxHandler(uint8_t *packet, uint8_t linkQuality, int8_t rssi);

#endif // __MFGLIB_H__

/** @} // END addtogroup
 */
