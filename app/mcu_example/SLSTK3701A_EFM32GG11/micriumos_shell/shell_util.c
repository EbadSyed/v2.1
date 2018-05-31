/**************************************************************************//**
 * @file shell_util.c
 * @brief Utility functions used in this example.
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
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "shell_util.h"

/***************************************************************************//**
 * @brief
 *   strtol() from stdlib.h library with error handling.
 *
 * @param str
 *   string to be converted to int
 *
 * @param p_err
 *   Set to RTOS_ERR_FAIL on error.
 *
 * @return
 *   Converted int or 0 if conversion fails.
 ******************************************************************************/
int shellStrtol(const char *str, RTOS_ERR  *p_err)
{
  char *endptr;
  errno = 0;
  int ret_val = strtol(str, &endptr, 0); // strtol

  if ((errno == ERANGE && (ret_val == INT_MAX || ret_val == INT_MIN))
      || (errno != 0 && ret_val == 0))
  {
    printf("shellStrtol: Over/underflow occurred\n");
    goto error;
  } else if (endptr == str) {
    printf("shellStrtol: No digits were found\n");
    goto error;
  } else {
    RTOS_ERR_SET(*p_err, RTOS_ERR_NONE);
    return ret_val;
  }

  error:
  RTOS_ERR_SET(*p_err, RTOS_ERR_FAIL);
  return 0;
}
