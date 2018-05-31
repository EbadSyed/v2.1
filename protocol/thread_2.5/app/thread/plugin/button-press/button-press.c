// Copyright 2015 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_HAL
#include EMBER_AF_API_STACK
#include EMBER_AF_API_BUTTON
#include EMBER_AF_API_BUTTON_PRESS

// assume there are 2 buttons, even though there could be as many as 4 and they
// need not be defined contiguously.
#define NUM_BUTTONS 2

static bool buttonPressed[NUM_BUTTONS];

// it's not easy to use an array of control objects with the plugin framework,
// so we do this by hand
EmberEventControl emberAfPluginButton0EventControl;
EmberEventControl emberAfPluginButton1EventControl;

EmberEventControl* const emberAfPluginButtonEventControl[] = {
  &emberAfPluginButton0EventControl,
  &emberAfPluginButton1EventControl
};

uint8_t emberAfPluginButtonIndex(uint8_t button)
{
  switch (button) {
    case BUTTON0:
      return 0;

    case BUTTON1:
      return 1;

    default:
      assert(1 == 0);
      return 0;
  }
}

void halButtonIsr(uint8_t button, uint8_t state)
{
  uint8_t index = emberAfPluginButtonIndex(button);

  if (state != BUTTON_RELEASED) {
    return;
  }

  // if the button was already pressed, it's a double-press
  if (buttonPressed[index]) {
    // clear the flag
    buttonPressed[index] = false;

    // cancel the event
    emberEventControlSetInactive(*emberAfPluginButtonEventControl[index]);

    // call the callback
    emberButtonPressIsr(button, EMBER_DOUBLE_PRESS);
  } else { // the first part of a single press?
    // set the flag
    buttonPressed[index] = true;

    // start the timer event
    emberEventControlSetDelayMS(*emberAfPluginButtonEventControl[index],
                                EMBER_AF_PLUGIN_BUTTON_PRESS_TIMEOUT_MS);
  }
}

void emberAfPluginButtonEventHandler(uint8_t button)
{
  uint8_t index = emberAfPluginButtonIndex(button);

  // cancel the event
  emberEventControlSetInactive(*emberAfPluginButtonEventControl[index]);

  if (buttonPressed[index]) {
    // clear the flag
    buttonPressed[index] = false;

    // call the callback
    emberButtonPressIsr(button, EMBER_SINGLE_PRESS);
  }
}

void emberAfPluginButton0EventHandler(void)
{
  emberAfPluginButtonEventHandler(BUTTON0);
}

void emberAfPluginButton1EventHandler(void)
{
  emberAfPluginButtonEventHandler(BUTTON1);
}
