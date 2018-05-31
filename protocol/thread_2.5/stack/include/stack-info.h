/**
 * @file stack-info.h
 * @brief SOC-only radio APIs.
 * @addtogroup utilities
 *
 * <!--Copyright 2004-2007 by Ember Corporation. All rights reserved.    *80*-->
 */

#ifndef __STACK_INFO_H__
#define __STACK_INFO_H__

/**
 * @addtogroup utilities
 *
 * See stack-info.h for source code.
 * @{
 */

/** @brief This function sets the channel for sending and receiving messages.
 * For a list of
 * available radio channels, see the technical specification for
 * the RF communication module in your Developer Kit.
 *
 * Note: Care should be taken when using this API,
 * as all devices on a network must use the same channel.
 *
 * @param channel  A desired radio channel.
 *
 * @return An ::EmberStatus value indicating the success or
 *  failure of the command.
 */
EmberStatus emberSetRadioChannel(uint8_t channel);

/** @brief This function gets the radio channel to which a node is set. The
 * possible return values depend on the radio in use. For a list of
 * available radio channels, see the technical specification for
 * the RF communication module in your Developer Kit.
 *
 * @return A current radio channel.
 */
uint8_t emberGetRadioChannel(void);

/** @name Radio-specific Functions*/
//@{

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
 * ::emberSetTxPowerMode() to use a power mode or transmitter output
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
 * Contact support@ember.com for instructions to set the MFG_PHY_CONFIG
 * token appropriately.
 *
 * @param txPowerMode  Specifies which of the transmit power mode options are
 * to be activated.  This parameter should be set to one of the literal values
 * described in stack/include/ember-types.h.  Any power option not specified
 * in the txPowerMode parameter will be deactivated.
 *
 * @return ::EMBER_SUCCESS if successful; an error code otherwise.
 */
// Moved to network-management.h
// EmberStatus emberSetTxPowerMode(uint16_t txPowerMode);

/** @brief This function returns the current configuration of boost power mode and alternate
 * transmitter output.
 *
 * @return The current tx power mode.
 */
// Moved to network-management.h
// uint16_t emberGetTxPowerMode(void);

/** @brief The radio calibration callback function.
 *
 * The Voltage Controlled Oscillator (VCO) can drift with
 * temperature changes.  During every call to ::emberTick(), the stack will
 * check to see if the VCO has drifted.  If the VCO has drifted, the stack
 * will call ::emberRadioNeedsCalibratingHandler() to inform the application
 * that it should perform calibration of the current channel as soon as
 * possible.  Calibration can take up to 150 ms.  The default callback function
 * implementation provided here performs the calibration immediately.  If the
 * application wishes, it can define its own callback by defining
 * ::EMBER_APPLICATION_HAS_CUSTOM_RADIO_CALIBRATION_CALLBACK in its
 * CONFIGURATION_HEADER.  It can then failsafe any critical processes or
 * peripherals before calling ::emberCalibrateCurrentChannel().  The
 * application must call ::emberCalibrateCurrentChannel() in
 * response to this callback to maintain an expected radio performance.
 */
void emberRadioNeedsCalibratingHandler(void);

/** @brief This function calibrates the current channel.  The stack will notify the
 * application of the need for channel calibration via the
 * ::emberRadioNeedsCalibratingHandler() callback function during
 * ::emberTick().  This function should only be called from within the context
 * of the ::emberRadioNeedsCalibratingHandler() callback function.  Calibration
 * can take up to 150 ms.  Note, if this function is called when the radio is
 * off, it will turn the radio on and leave it on.
 */
void emberCalibrateCurrentChannel(void);
//@}//END Radio-specific functions

/** @} END addtogroup */

#endif // __STACK_INFO_H__
