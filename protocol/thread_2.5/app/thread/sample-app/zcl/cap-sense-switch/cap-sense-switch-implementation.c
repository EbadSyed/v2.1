// Copyright 2017 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_HAL
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

#include EMBER_AF_API_CONNECTION_MANAGER
#include EMBER_AF_API_SB1_GESTURE_SENSOR
#include EMBER_AF_API_LED_BLINK

//------------------------------------------------------------------------------
// Application specific macros

// Macros used to enable/disable application behavior
#ifndef COLOR_CHANGE_ENABLED
#define COLOR_CHANGE_ENABLED            1
#endif

#ifndef LED_ENABLED
#define LED_ENABLED                     1
#endif

#ifndef SWIPING_ENABLED
#define SWIPING_ENABLED                 1
#endif

// Macros used to determine if switch is controlling Hue, Temp, or brightness
#define NORMAL_TAB 0
#define TEMP_TAB   1
#define HUE_TAB    2

// Macros used in color temperature and hue stepping commands
#define COLOR_TEMP_STEP_POSITIVE        1
#define COLOR_TEMP_STEP_NEGATIVE        3
#define COLOR_TEMP_STEP_AMOUNT          50
#define COLOR_HUE_STEP_POSITIVE         1
#define COLOR_HUE_STEP_NEGATIVE         3
#define COLOR_HUE_STEP_AMOUNT           10
#define LEVEL_STEP_POSITIVE             0
#define LEVEL_STEP_NEGATIVE             1
#define LEVEL_STEP_AMOUNT               30
#define LEVEL_STEP_TIME                 3

//------------------------------------------------------------------------------
// Application specific global variables

// State variable for what tab is currently being controlled
static uint8_t tabState = NORMAL_TAB;

EmberEventControl frameTimeoutEventControl;

static void sendCommandFromHoldUp();
static void sendCommandFromHoldDown();
static void genericMessageResponseHandler(EmberCoapStatus status,
                                          const EmberZclCommandContext_t *context,
                                          void *commandResponse);
static void sendCommandToBindings(uint16_t cluster,
                                  uint16_t command,
                                  void *request);

bool okToLongPoll = true;

static uint8_t outboundMessageCount = 0;

#if defined(EMBER_AF_API_CONNECTION_MANAGER_JIB)
uint8_t emberConnectionManagerJibGetJoinKeyCallback(uint8_t **returnedJoinKey)
{
  static uint8_t joinKey[EMBER_JOIN_KEY_MAX_SIZE + 1] = { 0 };
  static uint8_t joinKeyLength = 0;

  // Using a join key generated from the EUI64 of the node is a security risk,
  // but is useful for demonstration purposes. See the warning above. This
  // hash generates alphanumeric characters in the ranges 0-9 and A-U. The
  // Thread specification disallows the characters I, O, Q, and Z, for
  // readability. I, O, and Q are therefore remapped to V, W, and X. Z is not
  // generated, so it is not remapped.
  const EmberEui64 *eui64;
  eui64 = emberEui64();
  for (joinKeyLength = 0; joinKeyLength < EUI64_SIZE; (joinKeyLength)++) {
    joinKey[joinKeyLength] = ((eui64->bytes[joinKeyLength] & 0x0F)
                              + (eui64->bytes[joinKeyLength] >> 4));
    joinKey[joinKeyLength] += (joinKey[joinKeyLength] < 10 ? '0' : 'A' - 10);
    if (joinKey[joinKeyLength] == 'I') {
      joinKey[joinKeyLength] = 'V';
    } else if (joinKey[joinKeyLength] == 'O') {
      joinKey[joinKeyLength] = 'W';
    } else if (joinKey[joinKeyLength] == 'Q') {
      joinKey[joinKeyLength] = 'X';
    }
  }

  emberAfCorePrintln("Join device to network using EUI \"%x%x%x%x%x%x%x%x\" and join passphrase \"%s\"",
                     emberEui64()->bytes[7],
                     emberEui64()->bytes[6],
                     emberEui64()->bytes[5],
                     emberEui64()->bytes[4],
                     emberEui64()->bytes[3],
                     emberEui64()->bytes[2],
                     emberEui64()->bytes[1],
                     emberEui64()->bytes[0],
                     joinKey);

  *returnedJoinKey = joinKey;
  return joinKeyLength;
}
#endif //defined(EMBER_AF_API_CONNECTION_MANAGER_JIB)

void emberZclGetPublicKeyCallback(const uint8_t **publicKey,
                                  uint16_t *publicKeySize)
{
  *publicKey = emberEui64()->bytes;
  *publicKeySize = EUI64_SIZE;
}

void emberZclNotificationCallback(const EmberZclNotificationContext_t *context,
                                  const EmberZclClusterSpec_t *clusterSpec,
                                  EmberZclAttributeId_t attributeId,
                                  const void *buffer,
                                  size_t bufferLength)
{
  if (emberZclAreClusterSpecsEqual(&emberZclClusterOnOffServerSpec, clusterSpec)
      && attributeId == EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF) {
    bool onOff = *((bool *)buffer);
    emberAfCorePrintln("Light is %s", (onOff ? "on" : "off"));
  } else if (emberZclAreClusterSpecsEqual(&emberZclClusterLevelControlServerSpec, clusterSpec)
             && attributeId == EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL) {
    uint8_t currentLevel = *((uint8_t *)buffer);
    emberAfCorePrintln("Light is at 0x%x", currentLevel);
  }
}

/** @brief Ok To Long Poll
 *
 * This function is called by the Polling plugin to determine if the node can
 * wait an extended period of time between polls.  Generally, a node can poll
 * infrequently when it does not expect to receive data, via its parent, from
 * other nodes in the network.  When data is expected, the node must poll more
 * frequently to avoid having its parent discard stale data due to the MAC
 * indirect transmission timeout (::EMBER_INDIRECT_TRANSMISSION_TIMEOUT).  The
 * application should return true if it is not expecting data or false
 * otherwise.
 */
bool emberAfPluginPollingOkToLongPollCallback(void)
{
  if (emberZclEzModeIsActive()) {
    return false;
  }

  // Check connection manager in addition to the status here bound to the
  // outboundMessageCount to make sure that long polling is allowed in both
  // cases
  return (emberConnectionManagerIsOkToLongPoll() && okToLongPoll);
}

static void sendCommandToBindings(uint16_t cluster,
                                  uint16_t command,
                                  void *request)
{
  uint8_t numSends = 0;
  EmberZclDestination_t destination;
  EmberStatus status;
  EmberZclClusterSpec_t clusterSpec;

  // Convert clusterId to server clusterSpec.
  switch (cluster) {
    case EMBER_ZCL_CLUSTER_ON_OFF:
      clusterSpec = emberZclClusterOnOffServerSpec;
      break;
    case EMBER_ZCL_CLUSTER_LEVEL_CONTROL:
      clusterSpec = emberZclClusterLevelControlServerSpec;
      break;
    case EMBER_ZCL_CLUSTER_COLOR_CONTROL:
      clusterSpec = emberZclClusterColorControlServerSpec;
      break;
    default:
      clusterSpec.id = 0xFFFF; //== invalid
      break;
  }

  // Send command to all all matching binding destinations.
  EmberZclBindingId_t bindingIdx = 0;
  while (emberZclGetDestinationFromBinding(&clusterSpec,
                                           &bindingIdx,
                                           &destination)) {
    switch (cluster) {
      case EMBER_ZCL_CLUSTER_ON_OFF:
        // For ON_OFF, support the on command and off command.
        switch (command) {
          case EMBER_ZCL_CLUSTER_ON_OFF_SERVER_COMMAND_OFF:
            numSends++;
            status = emberZclSendClusterOnOffServerCommandOffRequest(
              &destination,
              NULL,
              (EmberZclClusterOnOffServerCommandOffResponseHandler)(&genericMessageResponseHandler));
            if (status != EMBER_SUCCESS) {
              emberAfCorePrintln("Error 0x%x when sending command 0x%x from cluster 0x%x",
                                 status,
                                 command,
                                 cluster);
            } else {
              outboundMessageCount++;
              okToLongPoll = false;
            }
            break;
          case EMBER_ZCL_CLUSTER_ON_OFF_SERVER_COMMAND_ON:
            numSends++;
            status = emberZclSendClusterOnOffServerCommandOnRequest(
              &destination,
              NULL,
              (EmberZclClusterOnOffServerCommandOnResponseHandler)(&genericMessageResponseHandler));
            if (status != EMBER_SUCCESS) {
              emberAfCorePrintln("Error 0x%x when sending command 0x%x from cluster 0x%x",
                                 status,
                                 command,
                                 cluster);
            } else {
              outboundMessageCount++;
              okToLongPoll = false;
            }
            break;
          default:
            emberAfCorePrintln("Unsupported command from OnOff cluster: 0x%x",
                               command);
            break;
        }
        break;

      case EMBER_ZCL_CLUSTER_LEVEL_CONTROL:
        // For Level Control, only the step with on off command is supported
        switch (command) {
          case EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_COMMAND_MOVE_TO_LEVEL:
            numSends++;
            status = emberZclSendClusterLevelControlServerCommandMoveToLevelRequest(
              &destination,
              (EmberZclClusterLevelControlServerCommandMoveToLevelRequest_t *)request,
              (EmberZclClusterLevelControlServerCommandMoveToLevelResponseHandler)(&genericMessageResponseHandler));
            if (status != EMBER_SUCCESS) {
              emberAfCorePrintln("Error 0x%x when sending command 0x%x",
                                 status,
                                 command);
            } else {
              outboundMessageCount++;
              okToLongPoll = false;
            }
            break;
          case EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_COMMAND_STEP:
            numSends++;
            status = emberZclSendClusterLevelControlServerCommandStepRequest(
              &destination,
              (EmberZclClusterLevelControlServerCommandStepRequest_t *)request,
              (EmberZclClusterLevelControlServerCommandStepResponseHandler)(&genericMessageResponseHandler));
            if (status != EMBER_SUCCESS) {
              emberAfCorePrintln("Error 0x%x when sending command 0x%x",
                                 status,
                                 command);
            } else {
              outboundMessageCount++;
              okToLongPoll = false;
            }
            break;
          case EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_COMMAND_STEP_WITH_ON_OFF:
            numSends++;
            status = emberZclSendClusterLevelControlServerCommandStepWithOnOffRequest(
              &destination,
              (EmberZclClusterLevelControlServerCommandStepWithOnOffRequest_t *)request,
              (EmberZclClusterLevelControlServerCommandStepWithOnOffResponseHandler)(&genericMessageResponseHandler));
            if (status != EMBER_SUCCESS) {
              emberAfCorePrintln("Error 0x%x when sending command 0x%x",
                                 status,
                                 command);
            } else {
              outboundMessageCount++;
              okToLongPoll = false;
            }
            break;
          default:
            emberAfCorePrintln("Unsupported command from LevelControl cluster: 0x%x",
                               command);
            break;
        }
        break;

      case EMBER_ZCL_CLUSTER_COLOR_CONTROL:
        switch (command) {
          case EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_COMMAND_STEP_HUE:
            numSends++;
            status = emberZclSendClusterColorControlServerCommandStepHueRequest(
              &destination,
              (EmberZclClusterColorControlServerCommandStepHueRequest_t *)request,
              (EmberZclClusterColorControlServerCommandStepHueResponseHandler)(&genericMessageResponseHandler));
            if (status != EMBER_SUCCESS) {
              emberAfCorePrintln("Error 0x%x when sending command 0x%x",
                                 status,
                                 command);
            } else {
              outboundMessageCount++;
              okToLongPoll = false;
            }
            break;
          case EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_COMMAND_MOVE_TO_SATURATION:
            numSends++;
            status = emberZclSendClusterColorControlServerCommandMoveToSaturationRequest(
              &destination,
              (EmberZclClusterColorControlServerCommandMoveToSaturationRequest_t *)request,
              (EmberZclClusterColorControlServerCommandMoveToSaturationResponseHandler)(&genericMessageResponseHandler));
            if (status != EMBER_SUCCESS) {
              emberAfCorePrintln("Error 0x%x when sending command 0x%x",
                                 status,
                                 command);
            } else {
              outboundMessageCount++;
              okToLongPoll = false;
            }
            break;
          case EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_COMMAND_STEP_COLOR_TEMPERATURE:
            numSends++;
            status = emberZclSendClusterColorControlServerCommandStepColorTemperatureRequest(
              &destination,
              (EmberZclClusterColorControlServerCommandStepColorTemperatureRequest_t *)request,
              (EmberZclClusterColorControlServerCommandStepColorTemperatureResponseHandler)(&genericMessageResponseHandler));
            if (status != EMBER_SUCCESS) {
              emberAfCorePrintln("Error 0x%x when sending command 0x%x",
                                 status,
                                 command);
            } else {
              outboundMessageCount++;
              okToLongPoll = false;
            }
            break;
          default:
            emberAfCorePrintln("Unsupported command from ColorControl cluster: 0x%x",
                               command);
            break;
        }
        break;

      default:
        emberAfCorePrintln("Unsupported command 0x%x on unsupported cluster 0x%x",
                           command,
                           cluster);
        break;
    }
  } //while

  if (numSends) {
    emberAfCorePrintln("Sent cluster 0x%2x command 0x%x to %d nodes",
                       cluster,
                       command,
                       numSends);
  } else {
    emberAfCorePrintln("Failed to send cluster 0x%2x 0x%0x, no valid binds",
                       cluster,
                       command);
  }
}

static void genericMessageResponseHandler(EmberCoapStatus status,
                                          const EmberZclCommandContext_t *context,
                                          void *commandResponse)
{
  if (outboundMessageCount) {
    outboundMessageCount--;
    if (!outboundMessageCount) {
      okToLongPoll = true;
    }
  } else {
    emberAfCorePrintln("ERR! Received more responseHandler calls than cmd/att sends recorded");
  }
}

//------------------------------------------------------------------------------
// Callback triggered when the SB1 gesture plugin receives a new gesture.  This
// will contain the gesture received and the button on which it was received.
// This function will handle all UI based state transitions, and generate radio
// radio traffic based on the user's actions.
void emberAfPluginSb1GestureSensorGestureReceivedCallback(uint8_t gesture,
                                                          uint8_t ui8SwitchNum)
{
  uint8_t blinkAck;

  // Reset the frame timeout on each button press
  emberEventControlSetInactive(frameTimeoutEventControl);
  emberEventControlSetDelayQS(frameTimeoutEventControl, 4 * 10);

  // Clear the flag tracking whether we need to send a command
  blinkAck = 0;

  // If the device is currently orphaned and the user is taking action, attempt
  // to rejoin the parent
  if (emberConnectionmanagerIsOrphaned()) {
    emberConnectionManagerSearchForParent();
  }

  // Form the ZigBee command to send based on the state of the device and which
  // button saw which gesture
  switch (ui8SwitchNum) {
    case SB1_GESTURE_SENSOR_SWITCH_TOP:
      switch (gesture) {
        // A touch on the top button maps to an "on" command
        case SB1_GESTURE_SENSOR_GESTURE_TOUCH:
          emberAfCorePrintln("Top button touched");
          sendCommandToBindings(EMBER_ZCL_CLUSTER_ON_OFF,
                                EMBER_ZCL_CLUSTER_ON_OFF_SERVER_COMMAND_ON,
                                NULL);
          blinkAck = 1;
          break;

        // A hold on the top button maps to an "up" command
        case SB1_GESTURE_SENSOR_GESTURE_HOLD:
          emberAfCorePrintln("Top button held");
          sendCommandFromHoldUp();
          blinkAck = 1;
          break;

        // Swiping right on the top or bottom will move the frame to the right.
        // Frame layout is: TEMP - NORMAL - HUE
        case SB1_GESTURE_SENSOR_GESTURE_SWIPE_R:
          // If the token is set to disable swiping, do nothing
          emberAfCorePrintln("Top button right swiped");
          if (SWIPING_ENABLED == 0) {
            emberAfCorePrintln("Swipe disabled by token!");
          } else {
            if (tabState == NORMAL_TAB) {
              //If the token is set to disable the color hue control, do nothing
              if (COLOR_CHANGE_ENABLED == 1) {
                tabState = HUE_TAB;
                emberAfCorePrintln("Switch to HUE");
                halLedBlinkSetActivityLed(BOARDLED0);
                halLedBlinkLedOn(0);
                halLedBlinkSetActivityLed(BOARDLED1);
                halLedBlinkLedOff(0);
              } else {
                emberAfCorePrintln("Color tab disabled by token!");
              }
            } else if (tabState == TEMP_TAB) {
              tabState = NORMAL_TAB;
              halLedBlinkSetActivityLed(BOARDLED0);
              halLedBlinkLedOff(0);
              halLedBlinkSetActivityLed(BOARDLED1);
              halLedBlinkLedOff(0);
              emberAfCorePrintln("Switch to NORMAL");
            }
          }
          // Swipe commands only modify internal state, so no radio message
          // needs to be sent
          blinkAck = 0;
          break;

        // Swiping right on the top or bottom will move the frame to the right.
        // Frame layout is: TEMP - NORMAL - HUE
        case SB1_GESTURE_SENSOR_GESTURE_SWIPE_L:
          // If the token is set to disable swiping, do nothing
          emberAfCorePrintln("Top button left swiped");
          if (SWIPING_ENABLED == 0) {
            emberAfCorePrintln("Swipe disabled by token!");
          } else {
            if (tabState == NORMAL_TAB) {
              tabState = TEMP_TAB;
              emberAfCorePrintln("Switch to TEMP");
              halLedBlinkSetActivityLed(BOARDLED1);
              halLedBlinkLedOn(0);
              halLedBlinkSetActivityLed(BOARDLED0);
              halLedBlinkLedOff(0);
            } else if (tabState == HUE_TAB) {
              tabState = NORMAL_TAB;
              halLedBlinkSetActivityLed(BOARDLED0);
              halLedBlinkLedOff(0);
              halLedBlinkSetActivityLed(BOARDLED1);
              halLedBlinkLedOff(0);
              emberAfCorePrintln("Switch to NORMAL");
            }
          }
          blinkAck = 0;
          break;

        // If we got here, we likely had a bad i2c transaction.  Ignore read data
        default:
          emberAfCorePrintln("bad gesture: 0x%02x", gesture);
          blinkAck = 0;
          return;
      }
      break;

    case SB1_GESTURE_SENSOR_SWITCH_BOTTOM:
      switch (gesture) {
        // A touch on the bottom button maps to an "off" command
        case SB1_GESTURE_SENSOR_GESTURE_TOUCH:
          emberAfCorePrintln("Bottom button touched");
          sendCommandToBindings(EMBER_ZCL_CLUSTER_ON_OFF,
                                EMBER_ZCL_CLUSTER_ON_OFF_SERVER_COMMAND_OFF,
                                NULL);
          blinkAck = 1;
          break;

        // A hold on the bottom button maps to a "level down" command
        case SB1_GESTURE_SENSOR_GESTURE_HOLD:
          emberAfCorePrintln("Bottom button held");
          sendCommandFromHoldDown();
          blinkAck = 1;
          break;

        // Swiping right on the top or bottom will move the frame to the right.
        // Frame layout is: TEMP - NORMAL - HUE
        case SB1_GESTURE_SENSOR_GESTURE_SWIPE_R:
          // If the token is set to disable swiping, do nothing
          emberAfCorePrintln("ZCL SW_R\r\n");
          if (SWIPING_ENABLED == 0) {
            emberAfCorePrintln("Swipe disabled by token!");
          } else {
            if (tabState == NORMAL_TAB) {
              //If the token is set to disable the color hue control, do nothing
              if (COLOR_CHANGE_ENABLED == 1) {
                tabState = HUE_TAB;
                emberAfCorePrintln("Switch to HUE");
                halLedBlinkSetActivityLed(BOARDLED0);
                halLedBlinkLedOn(0);
                halLedBlinkSetActivityLed(BOARDLED1);
                halLedBlinkLedOff(0);
              } else {
                emberAfCorePrintln("Color tab disabled by token!");
              }
            } else if (tabState == TEMP_TAB) {
              tabState = NORMAL_TAB;
              halLedBlinkSetActivityLed(BOARDLED0);
              halLedBlinkLedOff(0);
              halLedBlinkSetActivityLed(BOARDLED1);
              halLedBlinkLedOff(0);
              emberAfCorePrintln("Switch to NORMAL");
            }
          }
          // Swipe commands only modify internal state, so no radio message
          // needs to be sent
          blinkAck = 0;
          break;

        // Swiping right on the top or bottom will move the frame to the right.
        // Frame layout is: TEMP - NORMAL - HUE
        case SB1_GESTURE_SENSOR_GESTURE_SWIPE_L:
          // If the token is set to disable swiping, do nothing
          emberAfCorePrintln("ZCL SW_L \r\n");
          if (SWIPING_ENABLED == 0) {
            emberAfCorePrintln("Swipe disabled by token!");
          } else {
            if (tabState == NORMAL_TAB) {
              tabState = TEMP_TAB;
              emberAfCorePrintln("Switch to TEMP");
              halLedBlinkSetActivityLed(BOARDLED1);
              halLedBlinkLedOn(0);
              halLedBlinkSetActivityLed(BOARDLED0);
              halLedBlinkLedOff(0);
            } else if (tabState == HUE_TAB) {
              tabState = NORMAL_TAB;
              halLedBlinkSetActivityLed(BOARDLED0);
              halLedBlinkLedOff(0);
              halLedBlinkSetActivityLed(BOARDLED1);
              halLedBlinkLedOff(0);
              emberAfCorePrintln("Switch to NORMAL");
            }
          }
          blinkAck = 0;
          break;

        // If we got here, we likely had a bad i2c transaction.  Ignore read data
        default:
          emberAfCorePrintln("bad gesture: 0x%02x", gesture);
          blinkAck = 0;
          return;
      }
      break;

    // If we got here, we likely had a bad i2c transaction.  Ignore read data
    default:
      emberAfCorePrintln("unknown button: 0x%02x\r\n", ui8SwitchNum);
      blinkAck = 0;
      return;
  }

  // blink the LED to acknowledge a gesture was received
  if (blinkAck) {
    //blink the LED to show that a gesture was recognized
    halLedBlinkBlink(1, 100);
  }
}

static void sendCommandFromHoldUp(void)
{
  EmberZclClusterLevelControlServerCommandStepWithOnOffRequest_t levelRequest;
  EmberZclClusterColorControlServerCommandStepHueRequest_t colorHueRequest;
  EmberZclClusterColorControlServerCommandStepColorTemperatureRequest_t colorTempRequest;

  switch (tabState) {
    case NORMAL_TAB:
      emberAfCorePrintln("DIM UP");
      levelRequest.stepMode = LEVEL_STEP_POSITIVE;
      levelRequest.stepSize = LEVEL_STEP_AMOUNT;
      levelRequest.transitionTime = LEVEL_STEP_TIME;
      sendCommandToBindings(EMBER_ZCL_CLUSTER_LEVEL_CONTROL,
                            EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_COMMAND_STEP_WITH_ON_OFF,
                            (void*)(&levelRequest));
      break;
    case TEMP_TAB:
      emberAfCorePrintln("TEMP UP");
      colorTempRequest.stepMode = COLOR_TEMP_STEP_POSITIVE;
      colorTempRequest.stepSize = COLOR_TEMP_STEP_AMOUNT;
      colorTempRequest.transitionTime = 0;
      colorTempRequest.colorTemperatureMinimum = 0;
      colorTempRequest.colorTemperatureMaximum = 0xFFFF;
      sendCommandToBindings(EMBER_ZCL_CLUSTER_COLOR_CONTROL,
                            EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_COMMAND_STEP_COLOR_TEMPERATURE,
                            (void*)(&colorTempRequest));
      break;
    case HUE_TAB:
      emberAfCorePrintln("HUE UP");

      colorHueRequest.stepMode = COLOR_HUE_STEP_POSITIVE;
      colorHueRequest.stepSize = COLOR_HUE_STEP_AMOUNT;
      colorHueRequest.transitionTime = 0;
      sendCommandToBindings(EMBER_ZCL_CLUSTER_COLOR_CONTROL,
                            EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_COMMAND_STEP_HUE,
                            (void*)(&colorHueRequest));
      break;
  }
}

static void sendCommandFromHoldDown(void)
{
  EmberZclClusterLevelControlServerCommandStepWithOnOffRequest_t levelRequest;
  EmberZclClusterColorControlServerCommandStepHueRequest_t colorHueRequest;
  EmberZclClusterColorControlServerCommandStepColorTemperatureRequest_t colorTempRequest;

  switch (tabState) {
    case NORMAL_TAB:
      emberAfCorePrintln("DIM DOWN");
      levelRequest.stepMode = LEVEL_STEP_NEGATIVE;
      levelRequest.stepSize = LEVEL_STEP_AMOUNT;
      levelRequest.transitionTime = LEVEL_STEP_TIME;
      sendCommandToBindings(EMBER_ZCL_CLUSTER_LEVEL_CONTROL,
                            EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_COMMAND_STEP_WITH_ON_OFF,
                            (void*)(&levelRequest));
      break;
    case TEMP_TAB:
      emberAfCorePrintln("TEMP DOWN");
      colorTempRequest.stepMode = COLOR_TEMP_STEP_NEGATIVE;
      colorTempRequest.stepSize = COLOR_TEMP_STEP_AMOUNT;
      colorTempRequest.transitionTime = 0;
      colorTempRequest.colorTemperatureMinimum = 0;
      colorTempRequest.colorTemperatureMaximum = 0xFFFF;
      sendCommandToBindings(EMBER_ZCL_CLUSTER_COLOR_CONTROL,
                            EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_COMMAND_STEP_COLOR_TEMPERATURE,
                            (void*)(&colorTempRequest));
      break;
    case HUE_TAB:
      emberAfCorePrintln("HUE DOWN");
      colorHueRequest.stepMode = COLOR_HUE_STEP_NEGATIVE;
      colorHueRequest.stepSize = COLOR_HUE_STEP_AMOUNT;
      colorHueRequest.transitionTime = 0;
      sendCommandToBindings(EMBER_ZCL_CLUSTER_COLOR_CONTROL,
                            EMBER_ZCL_CLUSTER_COLOR_CONTROL_SERVER_COMMAND_STEP_HUE,
                            (void*)(&colorHueRequest));
      break;
  }
}

//------------------------------------------------------------------------------
// FrameTimeout event handler
// This handler is called a long time (default 10 seconds) after the user
// swipes left or right.  It will return the switch to the normal on/off tab.
void frameTimeoutEventHandler(void)
{
  //Make sure we don't stay on the HUE/TEMP tab for too long
  if ((tabState == HUE_TAB) || (tabState == TEMP_TAB)) {
    emberAfCorePrintln("Tab timeout, back to normal\n");
    tabState = NORMAL_TAB;
    halLedBlinkSetActivityLed(BOARDLED0);
    halLedBlinkLedOff(0);
    halLedBlinkSetActivityLed(BOARDLED1);
    halLedBlinkLedOff(0);
  }
  emberEventControlSetInactive(frameTimeoutEventControl);
}

//------------------------------------------------------------------------------
// Main Tick
// Whenever main application tick is called, this callback will be called at the
// end of the main tick execution.  It ensure that no LED activity will occur
// when the LED_ENABLED macro is set to disabled mode.
void emberAfTickCallback(void)
{
  //Use blinking led to indicate which tab is active, unless disabled by tokens
  if (LED_ENABLED == 0) {
    halLedBlinkSetActivityLed(BOARDLED0);
    halLedBlinkLedOff(0);
    halLedBlinkSetActivityLed(BOARDLED1);
    halLedBlinkLedOff(0);
  }
}

//------------------------------------------------------------------------------
// Ok To Sleep
//
// This function is called by the Idle/Sleep plugin before sleeping.  It is
// called with interrupts disabled.  The application should return true if the
// device may sleep or false otherwise.
//
// param durationMs The maximum duration in milliseconds that the device will
// sleep.  Ver.: always
bool emberAfPluginIdleSleepOkToSleepCallback(uint32_t durationMs)
{
  if (halSb1GestureSensorCheckForMsg()) {
    return false;
  } else {
    return true;
  }
}

//------------------------------------------------------------------------------
// Ok To Idle
//
// This function is called by the Idle/Sleep plugin before idling.  It is called
// with interrupts disabled.  The application should return true if the device
// may idle or false otherwise.
bool emberAfPluginIdleSleepOkToIdleCallback(void)
{
  if (halSb1GestureSensorCheckForMsg()) {
    return false;
  } else {
    return true;
  }
}
