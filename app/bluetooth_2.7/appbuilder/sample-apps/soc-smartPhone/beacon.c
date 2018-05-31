/***********************************************************************************************//**
 * \file   beacon.c
 * \brief  Beacon advertisement
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
#include "infrastructure.h"

/* application specific headers*/
#include "app_ui.h"

/* Own header */
#include "beacon.h"

/***********************************************************************************************//**
 * @addtogroup Services
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup beacon
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

/* Text definitions*/
#define BCN_LCD_TEXT                        "\nB E A C O N\n\nM O D E\n"

/* Flags */
/** Length of Flags field of the Beacon. */
#define BCN_FLAGS_LEN                       2
/** Flag bits. */
#define BCN_TYPE_FLAGS                      0x01
/** General discoverable flag. */
#define BCN_FLAG_LE_GENERAL_DISC            0x02
/** BR/EDR not supported flag. */
#define BCN_FLAG_LE_BREDR_NOT_SUP           0x04

/** Manufacturer specific data. */
#define BCN_TYPE_MANUFACTURER               0xFF
/* Beacon major number - used to group related beacons */
#define BCN_MAJ_NO                          34987
/** Beacon Minor Number - used to specify individual beacon within a group  */
#define BCN_MIN_NO                          1025
/** The Beacon's measured RSSI at 1 meter distance in dBm (default value) in two's complement format. */
#define BCN_RSSI                            0xC3
/** Length of Manufacturer specific data field. */
#define BCN_MAN_SPEC_DATA_LEN               26

/** Company ID - 0x004C - Apple. */
#define BCN_COMP_ID                         0x004C
/*  128-bit long ID. */
/* Beacon type - 0x0215 - iBeacon */
#define BCN_TYPE                            0x0215

/* Apple AirLocate Service UUID: e2c56db5-dffb-48d2-b060-d0f5a71096e0 */
#define BCN_UUID                            0xE2, 0xC5, 0x6D, 0xB5, 0xDF, 0xFB, 0x48, 0xD2, \
  0xB0, 0x60, 0xD0, 0xF5, 0xA7, 0x10, 0x96, 0xE0

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

/** Structure that holds Beacon advertisement data.
 *  The Beacon advertisement structure is filled here to serve as an example.
 *	See the iBeacon specification for futher details about the required structure. */

static struct {
  uint8_t flagsLen;     /* Length of the Flags field. */
  uint8_t flagsType;    /* Type of the Flags field. */
  uint8_t flags;        /* Flags field. */
  uint8_t mandataLen;   /* Length of the Manufacturer Data field. */
  uint8_t mandataType;  /* Type of the Manufacturer Data field. */
  uint8_t compId[2];    /* Company ID field. */
  uint8_t beacType[2];  /* Beacon Type field. */
  uint8_t uuid[16];     /* 128-bit Universally Unique Identifier.  */
  uint8_t majNum[2];    /* Beacon major number. */
  uint8_t minNum[2];    /* Beacon minor number. */
  uint8_t txPower;      /* The Beacon's measured RSSI at 1 meter distance in dBm. */
}
bcnBeaconAdvData = {
  /* Flag bits - See Bluetooth 4.0 Core Specification , Volume 3, Appendix C, 18.1 for more details on flags. */
  BCN_FLAGS_LEN,    /* length */
  BCN_TYPE_FLAGS,   /* type */
  BCN_FLAG_LE_BREDR_NOT_SUP | BCN_FLAG_LE_GENERAL_DISC,   /*Flags: BR/EDR is disabled. LE General Discoverable Mode */

  /* Manufacturer specific data */
  BCN_MAN_SPEC_DATA_LEN,    /* Length of field*/
  BCN_TYPE_MANUFACTURER,   /* Type of field */

  /* The first two data octets shall contain a company identifier code from
   * the Assigned Numbers - Company Identifiers document */
  { UINT16_TO_BYTES(BCN_COMP_ID) },

  /* Beacon type */
  { UINT16_TO_BYTE1(BCN_TYPE), UINT16_TO_BYTE0(BCN_TYPE) },

  /* 128 bit / 16 byte UUID */
  { BCN_UUID },
  /* Beacon major number 34987 split into two bytes */
  { UINT16_TO_BYTE1(BCN_MAJ_NO), UINT16_TO_BYTE0(BCN_MAJ_NO) },
  /* Beacon minor number 1025 split into two bytes */
  { UINT16_TO_BYTE1(BCN_MIN_NO), UINT16_TO_BYTE0(BCN_MIN_NO) },
  /* The Beacon's measured RSSI at 1 meter distance in dBm */
  BCN_RSSI
};

void bcnSetupAdvBeaconing(void)
{
  uint8_t len = sizeof(bcnBeaconAdvData);
  uint8_t *pData = (uint8_t*)(&bcnBeaconAdvData);

  /* Display text on LCD screen */
  appUiWriteString(BCN_LCD_TEXT);
  /* Set advertising data */
  gecko_cmd_le_gap_set_adv_data(false, len, pData);
  /* start advertising in user mode */
  gecko_cmd_le_gap_set_mode(le_gap_user_data, le_gap_non_connectable);
}

/** @} (end addtogroup beacon) */
/** @} (end addtogroup Services) */
