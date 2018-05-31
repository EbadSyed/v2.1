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

#ifndef __USER_COMMAND__
#define __USER_COMMAND__

#include "read_char.h"
#include "command_interpreter.h"
#include "user_command_config.h"

#define RETCODE_USER_COMMAND_ESCAPE     -2
#define RETCODE_USER_COMMAND_NO_CHAR    -1
#define RETCODE_USER_COMMAND_VALID_CHAR 0

void cliInitialize(void);
int8_t cliHandleInput(void);

#endif // __USER_COMMAND__
