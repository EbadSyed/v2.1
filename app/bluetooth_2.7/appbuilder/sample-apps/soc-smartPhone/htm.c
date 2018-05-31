/***********************************************************************************************//**
 * \file   htm.c
 * \brief  Health Thermometer Service
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/
/* standard library headers */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/* BG stack headers */
#include "bg_types.h"
#include "gatt_db.h"
#include "native_gecko.h"
#include "infrastructure.h"

/* application specific headers */
#include "app_hw.h"
#include "app_ui.h"
#include "app_timer.h"

/* Own header*/
#include "htm.h"

/***********************************************************************************************//**
 * @addtogroup Services
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup htm
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/
/// The macro returns the first digit behind the decimal point. This is to work around the
/// GCC+snprintf() floating point display issues
/// It expects a floating point value and evaluates to unit8_t.
#define FLOAT_FRACTION_1STDIGIT(VALUE) ((uint8)(((VALUE) -(uint8_t)(VALUE)) * 10.0f) % 10u)

/** Temperature Units Flag.
 *  Temperature Measurement Value in units of Celsius. */
#define HTM_FLAG_TEMP_UNIT_C                0x00
/** Temperature Units Flag.
 *  Temperature Measurement Value in units of Fahrenheit. */
#define HTM_FLAG_TEMP_UNIT_F                0x01
#define HTM_FLAG_TEMP_UNIT                  HTM_FLAG_TEMP_UNIT_C
/** Time Stamp Flag */
#define HTM_FLAG_TIMESTAMP_PRESENT          0x02
#define HTM_FLAG_TIMESTAMP_NOT_PRESENT      0
#define HTM_FLAG_TIMESTAMP_FIELD            HTM_FLAG_TIMESTAMP_PRESENT
/** Temperature Type Flag */
#define HTM_FLAG_TEMP_TYPE_PRESENT          0x04
#define HTM_FLAG_TEMP_TYPE_NOT_PRESENT      0
#define HTM_FLAG_TEMP_TYPE_FIELD            HTM_FLAG_TEMP_TYPE_PRESENT

/* Temperature type field definitions. */
/** Armpit. */
#define HTM_TT_ARMPIT                       1
/** Body (general). */
#define HTM_TT_BODY                         2
/** Ear (usually ear lobe). */
#define HTM_TT_EAR                          3
/** Finger. */
#define HTM_TT_FINGER                       4
/** Gastro-intestinal Tract. */
#define HTM_TT_GI                           5
/** Mouth. */
#define HTM_TT_MOUTH                        6
/** Rectum. */
#define HTM_TT_RECTUM                       7
/** Toe. */
#define HTM_TT_TOE                          8
/** Tympanum (ear drum). */
#define HTM_TT_TYMPANUM                     9

#define HTM_TT                              HTM_TT_ARMPIT
/* Other profile specific macros */
/* Text definitions*/
#define HTM_TEMP_VALUE_TEXT                 "\nTemperature:\n %3d.%1d C / %3d.%1d F\n"
#define HTM_TEMP_VALUE_TEXT_DEFAULT         "\nTemperature:\n---.- C / ---.- F\n"
#define HTM_TEMP_VALUE_TEXT_SIZE            (sizeof(HTM_TEMP_VALUE_TEXT_DEFAULT))
/* Temperature Measurement field lengths */
/** Length of Flags field. */
#define HTM_FLAGS_LEN                       1
/** Length of Temperature Measurement Value. */
#define HTM_MEAS_LEN                        4
/** Length of Date Time Characteristic Value. */
#define HTM_TIMESTAMP_LEN                   7
/** Length of Temperature Type Value. */
#define HTM_TEMP_TYPE_LEN                   1

/* Temperature Unit mask */
#define HTM_FLAG_TEMP_UNIT_MASK             0x01
/** Default maximum payload length for most PDUs. */
#define ATT_DEFAULT_PAYLOAD_LEN             20
/** Temperature measurement period in ms. */
#define HTM_TEMP_IND_TIMEOUT                1000
/** Indicates currently there is no active connection using this service. */
#define HTM_NO_CONNECTION                   0xFF
/***************************************************************************************************
 * Local Type Definitions
 **************************************************************************************************/

/** The Date Time characteristic is used to represent time.
 *
 * The Date Time characteristic contains fields for year, month, day, hours, minutes and seconds.
 * Calendar days in Date Time are represented using Gregorian calendar. Hours in Date Time are
 * represented in the 24h system. */
typedef struct {
  uint16_t tm_year; /**< Year */
  uint8_t tm_mon;   /**< Month */
  uint8_t tm_mday;  /**< Day */
  uint8_t tm_hour;  /**< Hour */
  uint8_t tm_min;   /**< Minutes */
  uint8_t tm_sec;   /**< Seconds */
} htmDateTime_t;

/** Temperature measurement structure. */
typedef struct {
  htmDateTime_t timestamp; /**< Date-time */
  uint32_t temperature;    /**< Temperature */
  uint8_t flags;           /**< Flags */
  uint8_t tempType;        /**< Temperature type */
  uint16_t period; /**< Measurement timer expiration period in seconds */
} htmTempMeas_t;

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

static htmTempMeas_t htmTempMeas = {
  .flags = (HTM_FLAG_TEMP_UNIT | HTM_FLAG_TIMESTAMP_FIELD | HTM_FLAG_TEMP_TYPE_FIELD),
  .period = HTM_TEMP_IND_TIMEOUT
};

/* timestamp */
static htmDateTime_t htmDateTime = { 2000, /*! Year, 0 means not known */
                                     4,    /*! Month, 0 means not known */
                                     1,    /*! Day, 0 means not known */
                                     12,   /*! Hour */
                                     0,    /*! Minutes */
                                     0     /*! Seconds */
};

static uint8_t htmClientConnection = HTM_NO_CONNECTION; /* Current connection or 0xFF if invalid */

/***************************************************************************************************
 * Static Function Declarations
 **************************************************************************************************/
static uint8_t htmBuildTempMeas(uint8_t *pBuf, htmTempMeas_t *pTempMeas);
static uint8_t htmProcMsg(uint8_t *buf);

/***************************************************************************************************
 * Public Function Definitions
 **************************************************************************************************/

/***********************************************************************************************//**
 *  \brief Initialize the Health Thermometer.
 **************************************************************************************************/
void htmInit(void)
{
  htmClientConnection = HTM_NO_CONNECTION; /* Initially no connection is set. */
  gecko_cmd_hardware_set_soft_timer(TIMER_STOP, TEMP_TIMER, true); /* Initially stop the timer. */
}

/***********************************************************************************************//**
 *  \brief Function that is called when the temperature characteristic status is changed.
 **************************************************************************************************/
void htmTemperatureCharStatusChange(uint8_t connection, uint16_t clientConfig)
{
  /* If the new value of Client Characteristic Config is not 0 (either indication or
   * notification enabled) update connection ID and start temp. measurement */
  if (clientConfig) {
    htmClientConnection = connection; /* Save connection ID */
    htmTemperatureMeasure(); /* Make an initial measurement */
  } else {
    gecko_cmd_hardware_set_soft_timer(TIMER_STOP, TEMP_TIMER, true);
  }
}

/***********************************************************************************************//**
 *  \brief Function for taking a single temperature measurement with the WSTK Temperature sensor.
 **************************************************************************************************/
void htmTemperatureMeasure(void)
{
  uint8_t htmTempBuffer[ATT_DEFAULT_PAYLOAD_LEN]; /* Stores the temperature data in the HTM format. */
  uint8_t length; /* Length of the temperature measurement characteristic */

  /* Check if the connection is still open */
  if (HTM_NO_CONNECTION == htmClientConnection) {
    return;
  }

  /* Create the temperature measurement characteristic in htmTempBuffer and store its length */
  length = htmProcMsg(htmTempBuffer);

  /* Send indication of the temperature in htmTempBuffer to all "listening" clients.
   * This enables the Health Thermometer in the Blue Gecko app to display the temperature.
   *  0xFF as connection ID will send indications to all connections. */
  gecko_cmd_gatt_server_send_characteristic_notification(
    htmClientConnection, gattdb_temp_measurement, length, htmTempBuffer);

  /* Start the repeating timer */
  gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(htmTempMeas.period), TEMP_TIMER, true);
}

/***************************************************************************************************
 * Static Function Definitions
 **************************************************************************************************/

/***********************************************************************************************//**
 *  \brief  Build a temperature measurement characteristic.
 *  \param[in]  pBuf  Pointer to buffer to hold the built temperature measurement characteristic.
 *  \param[in]  pTempMeas  Temperature measurement values.
 *  \return  Length of pBuf in bytes.
 **************************************************************************************************/
static uint8_t htmBuildTempMeas(uint8_t *pBuf, htmTempMeas_t *pTempMeas)
{
  uint8_t *p = pBuf;
  uint8_t flags = pTempMeas->flags;

  /* Convert HTM flags to bitstream and append them in the HTM temperature data buffer */
  UINT8_TO_BITSTREAM(p, flags);

  /* Convert temperature measurement value to bitstream */
  UINT32_TO_BITSTREAM(p, pTempMeas->temperature);

  /* If time stamp field present in HTM flags, convert timestamp to bitstream. */
  if (flags & HTM_FLAG_TIMESTAMP_PRESENT) {
    UINT16_TO_BITSTREAM(p, pTempMeas->timestamp.tm_year);
    UINT8_TO_BITSTREAM(p, pTempMeas->timestamp.tm_mon);
    UINT8_TO_BITSTREAM(p, pTempMeas->timestamp.tm_mday);
    UINT8_TO_BITSTREAM(p, pTempMeas->timestamp.tm_hour);
    UINT8_TO_BITSTREAM(p, pTempMeas->timestamp.tm_min);
    UINT8_TO_BITSTREAM(p, pTempMeas->timestamp.tm_sec);
  }

  /* If temperature type field present, convert type to bitstream */
  if (flags & HTM_FLAG_TEMP_TYPE_PRESENT) {
    UINT8_TO_BITSTREAM(p, pTempMeas->tempType);
  }

  /* Return length of data to be sent */
  return (uint8_t)(p - pBuf);
}

/***********************************************************************************************//**
 *  \brief  This function is called by the application when the periodic measurement timer expires.
 *  \param[in]  buf  Event message.
 *  \return  length of temp measurement.
 **************************************************************************************************/
static uint8_t htmProcMsg(uint8_t *buf)
{
  uint8_t len; /* Length of the temperature measurement */
  float tempC; /* Temperature in right format for the LCD */
  float tempF; /* Temperature in deg F*/
  char tempString[HTM_TEMP_VALUE_TEXT_SIZE]; /* Temperature as string for the LCD */
  int32_t tempData; /* Temperature data from the sensor */

  /* Read temperature and check if read successfully */
  if (appHwReadTm(&tempData) == 0) {
    /* The temperature read from the sensor is in milli-Celsius format */
    if (HTM_FLAG_TEMP_UNIT_F == (htmTempMeas.flags & HTM_FLAG_TEMP_UNIT_MASK)) {
      /* Conversion to Fahrenheit: F = C * 1.8 + 32
       * Here multiplying with 18 instead of 1.8 will make the result 10^4 (e4) */
      int32_t temp_e4 = tempData * 18 + 320000;
      htmTempMeas.temperature = FLT_TO_UINT32(temp_e4, -4);
    } else {
      htmTempMeas.temperature = FLT_TO_UINT32(tempData, -3);
    }
  }

  /* Convert temperature to the right format for LCD display */
  tempC = tempData / 1000.0;
  tempF = (tempC * 1.8f) + 32.0f;

  /* Temp in C and F should both appear on LCD display */
  snprintf(tempString,
           sizeof(HTM_TEMP_VALUE_TEXT_DEFAULT),
           HTM_TEMP_VALUE_TEXT,
           (uint8_t)(tempC),
           FLOAT_FRACTION_1STDIGIT(tempC),
           (uint8_t)(tempF),
           FLOAT_FRACTION_1STDIGIT(tempF));

  /* Write the string to LCD */
  appUiWriteString(tempString);

  /* Set the timestamp */
  htmTempMeas.timestamp = htmDateTime;

  /* Increment Seconds and Minutes fields to simulate time */
  htmDateTime.tm_sec += htmTempMeas.period / 1000;
  if (htmDateTime.tm_sec > 59) {
    htmDateTime.tm_sec = 0;
    htmDateTime.tm_min = (htmDateTime.tm_min > 59) ? 0 : (htmDateTime.tm_min + 1);
  }

  /* Set temperature type */
  htmTempMeas.tempType = HTM_TT;
  /* Build temperature measurement characteristic and store data length */
  len = htmBuildTempMeas(buf, &htmTempMeas);

  /* Return the length of the data */
  return len;
}

/** @} (end addtogroup htm) */
/** @} (end addtogroup Services) */
