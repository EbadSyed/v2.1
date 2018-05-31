/**************************************************************************//**
 * \file   user_command.c
 * \brief  Provides high-level integration for character read and the
 *         command-interpreter
 ******************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ******************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include COMMON_HEADER
#include "user_command.h"

CommandState_t state;
char ciBuffer[256];

static const CommandEntry_t commands[] = USER_COMMAND_LIST;

void cliInitialize(void)
{
  ciInitState(&state, ciBuffer, sizeof(ciBuffer), (CommandEntry_t *)commands);
}

int8_t cliHandleInput(void)
{
  uint8_t chr = (uint8_t)readChar();

  if (chr == 0x1B || chr == 0x04) {
    // Return RETCODE_USER_COMMAND_ESCAPE if ESC or Ctrl-D
    return RETCODE_USER_COMMAND_ESCAPE;
  } else if (chr != '\0' && chr != 0xFF) {
    if (ciProcessInput(&state, (char*)&chr, 1) > 0) {
      printf("> ");
    }
  }

  return RETCODE_USER_COMMAND_VALID_CHAR;
}
