/***********************************************************************************************//**
 * \file   aio_device.c
 * \brief  Automation I/O Device
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include <stdint.h>
#include "aio_device.h"
#include "rd0057.h"

/***********************************************************************************************//**
 * @addtogroup app_hardware
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup aio-hw
 * @{
 **************************************************************************************************/

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Local Type Definitions
 **************************************************************************************************/

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

/***************************************************************************************************
 * Static Function Declarations
 **************************************************************************************************/

/***************************************************************************************************
 * Public Function Definitions
 **************************************************************************************************/
void aioDeviceInit(void)
{
}

void aioDeviceSleep(void)
{
}

void aioDeviceDigitalOutWrite(AioDigitalState_t *aioDigitalOutStates)
{
  uint8_t i;

  for (i = 0; i < AIO_NUMBER_OF_DIGITAL_OUTPUTS; i++) {
    switch (aioDigitalOutStates[i]) {
      case AIO_DIGITAL_INACTIVE:
        boardLedOff(i);
        break;

      case AIO_DIGITAL_ACTIVE:
        boardLedOn(i);
        break;
      default:
        break;
    }
  }
}

void aioDeviceDigitalOutRead(AioDigitalState_t *aioDigitalOutStates)
{
  uint8_t i;

  for (i = 0; i < AIO_NUMBER_OF_DIGITAL_OUTPUTS; i++) {
    aioDigitalOutStates[i] = boardLedGet(i)
                             ? AIO_DIGITAL_ACTIVE
                             : AIO_DIGITAL_INACTIVE;
  }
}

void aioDeviceDigitalInRead(AioDigitalState_t *aioDigitalInStates)
{
  uint8_t i;
  for (i = 0; i < AIO_NUMBER_OF_DIGITAL_INPUTS; i++) {
    aioDigitalInStates[i] = boardButtonGet(i)
                            ? AIO_DIGITAL_ACTIVE
                            : AIO_DIGITAL_INACTIVE;
  }
}

/***************************************************************************************************
 * Static Function Definitions
 **************************************************************************************************/

/** @} (end addtogroup aio-hw) */
/** @} (end addtogroup app_hardware) */
