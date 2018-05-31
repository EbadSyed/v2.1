/***********************************************************************************************//**
 * \file   soc-thunderboard-react/app_ui.c
 * \brief  Application User Interface
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* Own header */
#include "app_ui.h"

/* standard library headers */
#include <stdint.h>

/* BG stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "infrastructure.h"

/* Application headers */
#include "app_timer.h"

/* Board specific headers */
#include "rd0057.h"

/***********************************************************************************************//**
 * @addtogroup Thunderboard
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app_ui
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

#define LED_LONG_PERIOD_MS       1000
#define LED_SHORT_PERIOD_MS        50
#define CON_OPENED_TOGGLE_COUNT     2

/***************************************************************************************************
 * Local Type Definitions
 **************************************************************************************************/

typedef enum {
  BLINK_OFF,
  BLINK_ERROR,
  BLINK_CON_OPENED,
  BLINK_ADVERTISING
} BlinkPattern_t;

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

static uint32_t toggleCount;
static BlinkPattern_t activeBlinkPattern;

/***************************************************************************************************
 * Public Variable Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Public Function Definitions
 **************************************************************************************************/
void appUiInit(void)
{
}

void appUiLedBlinkEvtHandler(void)
{
  uint8_t i;
  bool ledState[NO_OF_LEDS];

  for (i = 0; i < NO_OF_LEDS; i++) {
    ledState[i] = boardLedGet(0);
  }

  switch (activeBlinkPattern) {
    case BLINK_OFF:
      boardLedOff(LED_BLUE);
      boardLedOff(LED_GREEN);
      break;

    case BLINK_ERROR:
      if (ledState[LED_BLUE]) {
        boardLedOff(LED_BLUE);
        boardLedOn(LED_GREEN);
      } else {
        boardLedOn(LED_BLUE);
        boardLedOff(LED_GREEN);
      }
      gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(LED_LONG_PERIOD_MS), UI_TIMER, true);
      break;

    case BLINK_CON_OPENED:
      boardLedOff(LED_BLUE);
      if (toggleCount >= CON_OPENED_TOGGLE_COUNT) {
        activeBlinkPattern = BLINK_OFF;
        break;
      }
      // Short blinks and long periods off
      if (toggleCount % 2) {
        boardLedOff(LED_GREEN);
        gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(LED_LONG_PERIOD_MS), UI_TIMER, true);
      } else {
        boardLedOn(LED_GREEN);
        gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(LED_SHORT_PERIOD_MS), UI_TIMER, true);
      }

      toggleCount++;
      break;

    case BLINK_ADVERTISING:
      boardLedOff(LED_GREEN);
      // Short blinks and long periods off
      if (toggleCount % 2) {
        boardLedOff(LED_BLUE);
        gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(LED_LONG_PERIOD_MS), UI_TIMER, true);
      } else {
        boardLedOn(LED_BLUE);
        gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(LED_SHORT_PERIOD_MS), UI_TIMER, true);
      }
      toggleCount++;
      break;

    default:
      boardLedOff(LED_BLUE);
      boardLedOff(LED_GREEN);
      activeBlinkPattern = BLINK_OFF;
      break;
  }
}

void appUiError(void)
{
  activeBlinkPattern = BLINK_ERROR;
  toggleCount = 0;
  appUiLedBlinkEvtHandler();
}

void appUiConnectionOpened(void)
{
  activeBlinkPattern = BLINK_CON_OPENED;
  toggleCount = 0;
  appUiLedBlinkEvtHandler();
}

void appUiAdvStarted(void)
{
  activeBlinkPattern = BLINK_ADVERTISING;
  toggleCount = 0;
  appUiLedBlinkEvtHandler();
}

void appUiAdvStopped(void)
{
  activeBlinkPattern = BLINK_OFF;
  appUiLedBlinkEvtHandler();
}

void appUiSleep(void)
{
  activeBlinkPattern = BLINK_OFF;
  appUiLedBlinkEvtHandler();
}

/** @} (end addtogroup app_ui) */
/** @} (end addtogroup Thunderboard) */
