// Copyright 2015 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_EMBER_TYPES
#include EMBER_AF_API_COMMAND_INTERPRETER2
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#ifdef HAL_CONFIG
  #include "hal-config.h"
  #include "ember-hal-config.h"
#endif

void emberAfPluginCliTickCallback(void)
{
  if (emberProcessCommandInput(APP_SERIAL)) {
    emberAfCorePrint("%p> ", EMBER_AF_DEVICE_NAME);
  }
}
