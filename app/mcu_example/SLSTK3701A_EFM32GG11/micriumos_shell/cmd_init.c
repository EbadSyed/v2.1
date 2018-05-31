/**************************************************************************//**
* @file cmd_init.c
* @brief Init command for the shell.
* @version 5.3.5
******************************************************************************
* # License
* <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
*******************************************************************************
*
* This file is licensed under the Silabs License Agreement. See the file
* "Silabs_License_Agreement.txt" for details. Before using this software for
* any purpose, you must agree to the terms of that agreement.
*
******************************************************************************/
#include <stdio.h>

#include "bsp.h"
#include "cmd_declarations.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_cryotimer.h"
#include "em_emu.h"
#include "em_rtcc.h"
#include "retargetserial.h"
#include "shell_util.h"

// -----------------------------------------------------------------------------
// Local function prototypes

static void initChip(void);
static void initHfxo(void);
static CPU_INT16S initHfrco(int f);

// -----------------------------------------------------------------------------
// Global functions

/***************************************************************************//**
 * @brief
 *   Shell command function for the 'init' command.
 *
 * @param argc
 *   Number of arguments
 *
 * @param argv
 *   Array of pointers to strings which are the arguments.
 *
 * @param out_fnct
 *   Not used.
 *
 * @param pcmd_param
 *   Not used.
 ******************************************************************************/
CPU_INT16S initCmd(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT out_fnct,
                   SHELL_CMD_PARAM *pcmd_param)
{
  int i, a;
  RTOS_ERR err;

  (void)out_fnct; // Deliberately unused arguments
  (void)pcmd_param;

  for (i = 1; i < argc; i++) {
    if (!Str_Cmp(argv[i], "-hfxo")) {
      initHfxo();
    } else if (!Str_Cmp(argv[i], "-hfrco")) {
      a = shellStrtol(argv[++i], &err); // Convert argument to int

      if (RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE) { // Check if conversion succeeded
        printf("initCmd: Could not convert '%s'.\n", argv[i - 1]);
        return SHELL_EXEC_ERR;
      }

      return initHfrco(a);
    } else if (!Str_Cmp(argv[i], "-help")) {
      printf("init - Various initialization.\n"
             "Valid arguments:\n"
             " -chip         : Init of chip, energy mode, dcdc, etc.\n"
             " -help         : This help page.\n"
             " -hfrco [band] : Set HFRCO as clock source calibrated for 'band'.\n"
             " -hfxo         : Set HFXO as clock source\n");
    } else if (!Str_Cmp(argv[i], "-chip")) {
      initChip();
    } else {
      printf("initCmd: Invalid argument '%s'.\n", argv[i]);
      return SHELL_EXEC_ERR;
    }
  }

  return SHELL_EXEC_ERR_NONE;
}

// -----------------------------------------------------------------------------
// Local functions

/***************************************************************************//**
 * @brief
 *   Initializes dcdc, hfxo and energy modes.
 ******************************************************************************/
void initChip(void)
{
  EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_STK_DEFAULT;
  CMU_HFXOInit_TypeDef hfxoInit = CMU_HFXOINIT_DEFAULT;
  EMU_EM01Init_TypeDef em01Init = EMU_EM01INIT_DEFAULT;
  EMU_EM23Init_TypeDef em23Init = EMU_EM23INIT_DEFAULT;
  EMU_EM4Init_TypeDef em4Init = EMU_EM4INIT_DEFAULT;
  CRYOTIMER_Init_TypeDef init = CRYOTIMER_INIT_DEFAULT;

  printf("Initializing chip and dcdc regulator ... ");

  CHIP_Init(); // Errata workarounds
  dcdcInit.anaPeripheralPower = emuDcdcAnaPeripheralPower_AVDD;

  em01Init.vScaleEM01LowPowerVoltageEnable = true;
  em23Init.vScaleEM23Voltage = emuVScaleEM23_LowPower;
  em4Init.vScaleEM4HVoltage = emuVScaleEM4H_LowPower;

  EMU_DCDCInit(&dcdcInit); // Initialize DCDC regulator
  CMU_HFXOInit(&hfxoInit); // Initialize the HFXO to ensure valid start state
  EMU_EM01Init(&em01Init); // Initialize EMU energy modes
  EMU_EM23Init(&em23Init);
  EMU_EM4Init(&em4Init);

  CMU_ClockEnable(cmuClock_CRYOTIMER, true); // Enable cryotimer clock
  init.enable = false;
  CRYOTIMER_Init(&init); // Reset Cryotimer
  CRYOTIMER_IntClear(CRYOTIMER_IFC_PERIOD);
  CMU_ClockEnable(cmuClock_CRYOTIMER, false); // Disable cryotimer clock

  CMU_ClockSelectSet(cmuClock_LFE, cmuSelect_LFRCO); // Select LFRCO oscillator
  CMU_ClockEnable(cmuClock_RTCC, true); // Enable RTCC clock
  RTCC_IntClear(RTCC_IFC_CC1);
  CMU_ClockEnable(cmuClock_RTCC, false); // Disable RTCC clock

  printf("done.\n");
}

/***************************************************************************//**
 * @brief
 *   Sets reference for HF clock branch to crystall oscillator.
 ******************************************************************************/
static void initHfxo(void)
{
  printf("Setting HFXO as HF clock source ... ");
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO); // Set new reference
  RETARGET_SerialInit(); // Re-enable VCOM
  printf("done.\n");
}

/***************************************************************************//**
 * @brief
 *   Shell Sets reference for HF clock branch to HFRCO at a given band.
 *
 * @param f
 *   Frequency band of the HFRCO.
 *
 * @return
 *   SHELL_EXEC_ERR on error, SHELL_EXEC_ERR_NONE otherwise.
 ******************************************************************************/
CPU_INT16S initHfrco(int f)
{
  CMU_HFRCOFreq_TypeDef band;

  printf("Setting HFRCO at %d MHz as clock source ... ", f);

  switch (f) {
    case 1:
      band = cmuHFRCOFreq_1M0Hz;
      break;
    case 2:
      band = cmuHFRCOFreq_2M0Hz;
      break;
    case 4:
      band = cmuHFRCOFreq_4M0Hz;
      break;
    case 7:
      band = cmuHFRCOFreq_7M0Hz;
      break;
    case 13:
      band = cmuHFRCOFreq_13M0Hz;
      break;
    case 16:
      band = cmuHFRCOFreq_16M0Hz;
      break;
    case 19:
      band = cmuHFRCOFreq_19M0Hz;
      break;
    case 26:
      band = cmuHFRCOFreq_26M0Hz;
      break;
    case 32:
      band = cmuHFRCOFreq_32M0Hz;
      break;
    case 38:
      band = cmuHFRCOFreq_38M0Hz;
      break;
#if defined(_DEVINFO_HFRCOCAL16_MASK)
    case 48:
      band = cmuHFRCOFreq_48M0Hz;
      break;
    case 56:
      band = cmuHFRCOFreq_56M0Hz;
      break;
    case 64:
      band = cmuHFRCOFreq_64M0Hz;
      break;
    case 72:
      band = cmuHFRCOFreq_72M0Hz;
      break;
#endif
    default:
      printf("Invalid band specified.\n");
      return SHELL_EXEC_ERR;
  }

  CMU_HFRCOBandSet(band);
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
  RETARGET_SerialInit(); // Re-enable VCOM
  printf("done.\n");

  return SHELL_EXEC_ERR_NONE;
}
