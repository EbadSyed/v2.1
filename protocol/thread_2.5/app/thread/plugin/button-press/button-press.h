// Copyright 2015 Silicon Laboratories, Inc.

#ifndef __BUTTON_PRESS_H__
#define __BUTTON_PRESS_H__

/** @brief Constants for indicating single or double button press
 */
typedef enum EmberButtonPress {
  EMBER_SINGLE_PRESS,
  EMBER_DOUBLE_PRESS
} EmberButtonPress;

/** @brief A callback called when a button is pressed. It is sometimes called
 * in ISR context.
 *
 * @appusage Must be implemented by the application.  This function should
 * contain the functionality to be executed in response to a button press, or
 * callbacks to the appropriate functionality.
 *
 * @param button  The button which was pressed, either BUTTON0 or BUTTON1
 * as defined in the appropriate BOARD_HEADER.
 *
 * @param press  Either EMBER_SINGLE_PRESS if it was a single press, or
 * EMBER_DOUBLE_PRESS if it was a double press.
 */
void emberButtonPressIsr(uint8_t button, EmberButtonPress press);

#endif
