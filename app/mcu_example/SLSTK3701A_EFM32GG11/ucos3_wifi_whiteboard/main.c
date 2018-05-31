/**************************************************************************//**
 * @file main.c
 * @brief uC/OS-III + WiFi + Graphics LCD Demo
 * @version 5.3.5
 ******************************************************************************
 * # License * <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"

#include "bspconfig.h"
#include "app.h"

#include <os.h>

/**************************************************************************//**
 * @brief Main entry point.
 *****************************************************************************/
int main(void)
{
  OS_ERR err;
  EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_STK_DEFAULT;

  // Chip errata.
  CHIP_Init();

  // Init DCDC regulator and HFXO with kit specific parameters.
  EMU_DCDCInit(&dcdcInit);

  // Set HFRCO to 64MHz.
  CMU_HFRCOFreqSet(cmuHFRCOFreq_64M0Hz);

  // Initialize uC/OS-III and the application tasks.
  APP_Init();

  // Start uC/OS-III.
  OSStart(&err);

  // We should never get here.
  while (1) {
    ;
  }
}
