/**************************************************************************//**
 * @file shell_util.h
 * @brief Interface for utility functions.
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
#ifndef SHELL_UTIL_H
#define SHELL_UTIL_H

#include <rtos/common/include/rtos_err.h>

// -----------------------------------------------------------------------------
// Function prototypes

int shellStrtol(const char *str, RTOS_ERR *err);

#endif /* SHELL_UTIL_H */
