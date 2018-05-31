/***********************************************************************************************//**
 * \file   config.h
 * \brief  Configuration header file
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include "config.h"

/* Default configuration */
configuration_t configuration = DEFAULT_CONFIGURATION;

configuration_t *CONF_get(void)
{
  return &configuration;
}
