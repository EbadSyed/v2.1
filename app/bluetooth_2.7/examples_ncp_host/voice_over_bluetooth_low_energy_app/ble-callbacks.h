/***********************************************************************************************//**
 * \file   ble-callbacks.h
 * \brief  Declaration of BLE callback functions
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef __BLE_CALLBACKS__
#define __BLE_CALLBACKS__

#include COMMON_HEADER

// Called when gecko_evt_le_gap_scan_response_id event received
void SCAN_Process_scan_response(struct gecko_cmd_packet *evt);

#endif // __BLE_CALLBACKS__
