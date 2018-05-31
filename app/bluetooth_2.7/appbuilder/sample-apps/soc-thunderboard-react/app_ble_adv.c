/***********************************************************************************************//**
 * \file   soc-thunderboard-react/app_ble_adv.c
 * \brief  Application BLE Advertisement
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* Own header */
#include "app_ble_adv.h"

/* BG stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "infrastructure.h"

#include <stdbool.h>
#include "app_timer.h"
#include "app_ble.h"

/***********************************************************************************************//**
 * @addtogroup Thunderboard
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app_ble_adv
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

#define ADV_DATA_ALTERNATE_TIME_MS 1000
#define ADV_DATA_SWAP_DELAY_MS      1

/** Company ID reserved. */
#define BEA_PREAMB             0x004C

/** Beacon Type. */
#define BEA_TYPE               0x1502

/** Beacon Major Number. */
#define BEA_MAJ_NO             0x0000
/** Beacon Minor Number. */
#define BEA_MIN_NO             0x0000

/** The Beacon's measured RSSI at 1 meter distance in dBm. */
#define BEA_RSSI               0xC3
/** Length of Manufacturer specific data field. */
#define BEA_MAN_SPEC_DATA_LEN  26
/** Universally Unique ID used in the Beacon.
 *  128-bit long ID. */
#define BEA_UUID                                  \
  0xCE, 0xF7, 0x97, 0xDA, 0x2E, 0x91, 0x4E, 0xA4, \
  0xA4, 0x24, 0xF4, 0x50, 0x82, 0xAC, 0x06, 0x82

/** Lenght of response manufacturer specific data */
#define RESP_MAN_SPEC_DATA_LEN 5

/** Silicon Labs Company ID */
#define COMPANY_ID_SILICON_LABS 0x0047

/** Firmware ID used to uniquely identify capabilities of device */
#define FIRMWARE_ID 0x0001

/** Length of Flags field of the Beacon. */
#define ADV_FLAGS_LEN          2

/** Flag bits. */
#define ADV_TYPE_FLAGS                      0x01

/** Manufacturer specific data. */
#define ADV_TYPE_MANUFACTURER               0xFF

/** Complete local name. */
#define ADV_TYPE_LOCAL_NAME                 0x09

/* Bit mask for flags advertising data type. */

/** Limited discoverable flag. */
#define ADV_FLAG_LE_LIMITED_DISC            0x01
/** General discoverable flag. */
#define ADV_FLAG_LE_GENERAL_DISC            0x02
/** BR/EDR not supported flag. */
#define ADV_FLAG_LE_BREDR_NOT_SUP           0x04

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

static bool beaconData;

/** Structure that holds beacon data */
static struct {
  uint8_t flagsLen;     /**< Length of the Flags field. */
  uint8_t flagsType;    /**< Type of the Flags field. */
  uint8_t flags;        /**< Flags field. */
  uint8_t mandataLen;   /**< Length of the Manufacturer Data field. */
  uint8_t mandataType;  /**< Type of the Manufacturer Data field. */
  uint8_t compId[2];    /**< Company ID field. */
  uint8_t beacType[2];  /**< Beacon Type field. */
  uint8_t uuid[16];     /**< 128-bit Universally Unique Identifier. */
  uint8_t majNum[2];    /**< Beacon major number. */
  uint8_t minNum[2];    /**< Beacon minor number. */
  uint8_t rssi;         /**< The Beacon's measured RSSI at 1 meter distance in dBm. */
} ibeaconData = {
  /* Flag bits */
  ADV_FLAGS_LEN,  /* length */
  ADV_TYPE_FLAGS,
  ADV_FLAG_LE_BREDR_NOT_SUP | ADV_FLAG_LE_GENERAL_DISC,
  /* Manufacturer specific data */
  BEA_MAN_SPEC_DATA_LEN,  /* length */
  ADV_TYPE_MANUFACTURER,
  /* The first two data octets shall contain a company identifier code from
   * the Assigned Numbers - Company Identifiers document */
  { UINT16_TO_BYTES(BEA_PREAMB) },
  /* Beacon type */
  { UINT16_TO_BYTES(BEA_TYPE) },
  /* 128 bit / 16 byte UUID */
  { BEA_UUID },
  /* Beacon major number */
  { UINT16_TO_BYTES(BEA_MAJ_NO) },
  /* Beacon minor number */
  { UINT16_TO_BYTES(BEA_MIN_NO) },
  /* The Beacon's measured RSSI at 1 meter distance in dBm */
  /* 0xC3 is -61dBm */
  BEA_RSSI
};

/** Structure that holds advertisement data */
static struct {
  uint8_t flagsLen;                    /**< Length of the Flags field. */
  uint8_t flagsType;                   /**< Type of the Flags field. */
  uint8_t flags;                       /**< Flags field. */
  uint8_t mandataLen;                  /**< Length of the mandata field. */
  uint8_t mandataType;                 /**< Type of the mandata field. */
  uint8_t compId[2];                   /**< Company ID. */
  uint8_t firmwareId[2];               /**< Firmware ID */
  uint8_t localNameLen;                /**< Length of the local name field. */
  uint8_t localNameType;               /**< Type of the local name field. */
  uint8_t localName[DEVNAME_MAX_LEN];  /**< Local name field. */
} advData = {
  /* Flag bits */
  ADV_FLAGS_LEN,  /* length */
  ADV_TYPE_FLAGS,
  ADV_FLAG_LE_BREDR_NOT_SUP | ADV_FLAG_LE_LIMITED_DISC,
  RESP_MAN_SPEC_DATA_LEN,
  ADV_TYPE_MANUFACTURER,
  { UINT16_TO_BYTES(COMPANY_ID_SILICON_LABS) },
  { UINT16_TO_BYTES(FIRMWARE_ID) },
  /* Name bits */
  DEVNAME_DEFAULT_LEN + 1,  /* length + 1 for type */
  ADV_TYPE_LOCAL_NAME,
  DEVNAME_DEFAULT
};

/***************************************************************************************************
 * Public Variable Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Function Definitions
 **************************************************************************************************/
void appBleAdvInit(void)
{
  gecko_cmd_le_gap_set_adv_data(false,
                                sizeof(advData),
                                (uint8_t*)(&advData));

  beaconData = false;
}

void appBleAdvSetDevName(char *devName)
{
  strcpy((char *) advData.localName, devName);
  advData.localNameLen = strlen(devName) + 1;
}

void appBleAdvSetId(uint32_t uniqueId)
{
  ibeaconData.minNum[1] = UINT32_TO_BYTE0(uniqueId);
  ibeaconData.minNum[0] = UINT32_TO_BYTE1(uniqueId);
  ibeaconData.majNum[1] = UINT32_TO_BYTE2(uniqueId);
}

void appBleAdvStart(void)
{
  gecko_cmd_le_gap_set_mode(le_gap_user_data, le_gap_undirected_connectable);

  // Start timer to change advertisement data
  gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(ADV_DATA_ALTERNATE_TIME_MS), ADV_ALTERNATE_TIMER, false);
}

void appBleAdvStop(void)
{
  gecko_cmd_hardware_set_soft_timer(0, ADV_ALTERNATE_TIMER, false);

  gecko_cmd_le_gap_set_mode(le_gap_non_discoverable, le_gap_non_connectable);
}

void appBleAdvAlternateEvtHandler()
{
  // Swap advertising message, either iBeacon or regular advertisement data
  if (beaconData) {
    gecko_cmd_le_gap_set_adv_data(false,
                                  sizeof(advData),
                                  (uint8_t*)(&advData));

    beaconData = false;
  } else {
    gecko_cmd_le_gap_set_adv_data(false,
                                  sizeof(ibeaconData),
                                  (uint8_t*)(&ibeaconData));

    beaconData = true;
  }
}

/** @} (end addtogroup app_ble_adv) */
/** @} (end addtogroup Thunderboard) */
