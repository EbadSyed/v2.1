/***********************************************************************************************//**
 * \file   ia.c
 * \brief  Immediate Alert Service
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/
/* BG stack headers */
#include "bg_types.h"
#include "native_gecko.h"

/* application specific headers */
#include "app_ui.h"

/* Own header */
#include "ia.h"

/***********************************************************************************************//**
 * @addtogroup Services
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup ia
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/
/* Text definitions*/
#define IA_HIGH_ALERT_TEXT          "\nAlert level:\n\nHIGH\n"
#define IA_LOW_ALERT_TEXT           "\nAlert level:\n\nLOW\n"
#define IA_NO_ALERT_TEXT            "\nAlert level:\n\nNo Alert\n"

/** Immediate Alert Level.
 *  Client sent a No Immediate Alert message. */
#define ALERT_NO                    0
/** Immediate Alert Level.
 *  Client sent a Mild Immediate Alert Level message. */
#define ALERT_MILD                  1
/** Immediate Alert Level.
 *  Client sent a High Immediate Alert Level message. */
#define ALERT_HIGH                  2

/***************************************************************************************************
 * Type Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

/***************************************************************************************************
 * Function Definitions
 **************************************************************************************************/
void iaImmediateAlertWrite(uint8array *writeValue)
{
  switch (writeValue->data[0]) {
    default:
    case ALERT_NO:
      /* No Alert received */
      appUiWriteString(IA_NO_ALERT_TEXT);
      appUiLedOff();
      break;

    case ALERT_MILD:
      /* Low Alert received */
      appUiWriteString(IA_LOW_ALERT_TEXT);
      appUiLedLowAlert();
      break;

    case ALERT_HIGH:
      /* High Alert received */
      appUiWriteString(IA_HIGH_ALERT_TEXT);
      appUiLedHighAlert();
      break;
  }
}

/** @} (end addtogroup ia) */
/** @} (end addtogroup Services) */
