/***********************************************************************************************//**
 * \file   aio_device.h
 * \brief  Automation I/O Device
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef AIO_DEVICE_H
#define AIO_DEVICE_H

#include "aio_service.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************//**
 * \defgroup aio-hw Automation IO HW
 * \brief Automation IO Service HW API
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app_hardware
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup aio-hw
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Public Macros and Definitions
 **************************************************************************************************/

#define AIO_NUMBER_OF_DIGITAL_INPUTS  2 // Implementation supports max 4 inputs
#define AIO_NUMBER_OF_DIGITAL_OUTPUTS 2 // Implementation supports max 4 outputs
#define AIO_DIGITAL_INPUT_PAYLOAD_LEN  (1 + AIO_NUMBER_OF_DIGITAL_INPUTS / 8)
#define AIO_DIGITAL_OUTPUT_PAYLOAD_LEN (1 + AIO_NUMBER_OF_DIGITAL_OUTPUTS / 8)

#define AIO_NUMBER_OF_DIGITALS (AIO_NUMBER_OF_DIGITAL_INPUTS + AIO_NUMBER_OF_DIGITAL_OUTPUTS)
#define AIO_STATE_NUMBER_OF_BITS      2
#define AIO_STATE_BITS_MASK         0x3

/***************************************************************************************************
 * Typedefs
 **************************************************************************************************/

typedef enum {
  AIO_DIGITAL_INACTIVE = 0,
  AIO_DIGITAL_ACTIVE   = 1,
  AIO_DIGITAL_TRISTATE = 2,
  AIO_DIGITAL_UNKNOWN  = 3,
} AioDigitalState_t;

/***************************************************************************************************
 * Function Declarations
 **************************************************************************************************/

/**********************************************************************************************//**
 * @brief
 *   Initialize the module.
 *************************************************************************************************/
void aioDeviceInit(void);

/**********************************************************************************************//**
 * @brief
 *   Put the hardware in sleep mode (for low power).
 *************************************************************************************************/
void aioDeviceSleep(void);

/**********************************************************************************************//**
 * @brief
 *   Set the state for digital outputs.
 * @param[out] aioDigitalOutStates
 *   A pointer to an array of digital output state descriptors.
 *************************************************************************************************/
void aioDeviceDigitalOutWrite(AioDigitalState_t *aioDigitalOutStates);

/**********************************************************************************************//**
 * @brief
 *   Get the state for digital outputs.
 * @param[out] aioDigitalOutStates
 *   A pointer to an array of digital output state descriptors.
 *************************************************************************************************/
void aioDeviceDigitalOutRead(AioDigitalState_t *aioDigitalOutStates);

/**********************************************************************************************//**
 * @brief
 *   Get the state for digital inputs.
 * @param[out] aioDigitalInStates
 *   A pointer to an array of digital input state descriptors.
 *************************************************************************************************/
void aioDeviceDigitalInRead(AioDigitalState_t *aioDigitalInStates);

/** @} (end addtogroup aio-hw) */
/** @} (end addtogroup app_hardware) */

#ifdef __cplusplus
};
#endif

#endif // AIO_DEVICE_H
