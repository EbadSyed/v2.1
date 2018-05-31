/***************************************************************************//**
 * @file crc16.h
 * @brief CRC16 functionality (CRC16-CCITT)
 * @author Silicon Labs
 * @version 1.0.0-alpha
 *******************************************************************************
 * @section License
 * <b>Copyright 2016 Silicon Laboratories, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/
#ifndef __CRC16_H__
#define __CRC16_H__

#include <stdint.h>
#include <stddef.h>

/***************************************************************************//**
 * @addtogroup Plugin
 * @{
 * @addtogroup Security
 * @{
 * @addtogroup CRC16
 * @{
 * @brief CRC16 functionality for the bootloader
 * @details
 ******************************************************************************/

/// CRC16 start value
#define CRC16_START             0xFFFFU

/***************************************************************************//**
 * Calculate CRC16 on input
 *
 * @param newByte    Byte to append to CRC16 calculation
 * @param prevResult Previous output from CRC algorithm. @ref BTL_CRC16_START
 *                   when starting a new calculation.
 * @return Result of the CRC16 operation
 ******************************************************************************/
uint16_t crc16(const uint8_t newByte, uint16_t prevResult);

/***************************************************************************//**
 * Calculate CRC16 on input stream
 *
 * @param buffer     buffer containing bytes to append to CRC16 calculation
 * @param length     Size of the buffer in bytes
 * @param prevResult Previous output from CRC algorithm. @ref BTL_CRC16_START
 *                   when starting a new calculation.
 * @returns Result of the CRC16 operation
 ******************************************************************************/
uint16_t crc16Stream(const uint8_t* buffer, size_t length, uint16_t prevResult);

/** @} addtogroup CRC16 */
/** @} addtogroup Security */
/** @} addtogroup Plugin */

#endif // __CRC16_H__
