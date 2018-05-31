/**************************************************************************//**
 * @file cmd_help.c
 * @brief Help command for the shell.
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

#include <rtos/common/include/shell.h>
#include <rtos/cpu/include/cpu.h>

#include "cmd_declarations.h"

// -----------------------------------------------------------------------------
// Global functions

/***************************************************************************//**
 * @brief
 *   Shell command function for the 'help' command.
 *
 * @param argc
 *   Not used.
 *
 * @param argv
 *   Not used.
 *
 * @param out_fnct
 *   Not used.
 *
 * @param pcmd_param
 *   Not used.
 *
 * @return
 *   SHELL_EXEC_ERR on error, SHELL_EXEC_ERR_NONE otherwise.
 ******************************************************************************/
CPU_INT16S helpCmd(CPU_INT16U argc,
                   CPU_CHAR *argv[],
                   SHELL_OUT_FNCT out_fnct,
                   SHELL_CMD_PARAM *pcmd_param)
{
  int i;

  (void)argc; // Deliberately unused arguments
  (void)argv;
  (void)out_fnct;
  (void)pcmd_param;

  printf("Usage: command [-arg [param]]\n");
  printf("The available commands are:\n");

  for (i = 0; commandTable[i].Name; i++) {
    printf("\t%s\n", commandTable[i].Name);
  }

  return SHELL_EXEC_ERR_NONE;
}
