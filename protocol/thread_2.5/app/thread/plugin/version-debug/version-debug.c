// Copyright 2015 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_HAL
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include "stack/config/config.h"

#ifndef ALIAS
  #define ALIAS(x) x
#endif

static const char * const versionTypeNames[] = {
  EMBER_VERSION_TYPE_NAMES
};

static void printVersion(const uint8_t *versionName,
                         uint16_t managementVersionNumber,
                         uint16_t stackVersionNumber,
                         uint16_t stackBuildNumber,
                         EmberVersionType versionType,
                         const uint8_t *buildTimestamp)
{
  emberAfCorePrint("%s %u.%u.%u.%u",
                   versionName,
                   (stackVersionNumber & 0xF000) >> 12,
                   (stackVersionNumber & 0x0F00) >> 8,
                   (stackVersionNumber & 0x00F0) >> 4,
                   (stackVersionNumber & 0x000F));
  if (versionType <= EMBER_VERSION_TYPE_MAX) {
    emberAfCorePrint(" %s", versionTypeNames[versionType]);
  }
  emberAfCorePrint(" build %u", stackBuildNumber);
#ifdef EMBER_HOST
  emberAfCorePrint(" management %u", managementVersionNumber);
#endif
  emberAfCorePrint(" (%s)", buildTimestamp);
  emberAfCorePrintln("");
}

void ALIAS(emberGetVersionsReturn)(const uint8_t * versionName,
                                   uint16_t managementVersionNumber,
                                   uint16_t stackVersionNumber,
                                   uint16_t stackBuildNumber,
                                   EmberVersionType versionType,
                                   const uint8_t * buildTimestamp)
{
#ifdef EMBER_HOST
  emberAfCorePrint("Host: ");
  printVersion((const uint8_t *)EMBER_VERSION_NAME,
               EMBER_MANAGEMENT_VERSION,
               EMBER_FULL_VERSION,
               EMBER_BUILD_NUMBER,
               EMBER_VERSION_TYPE,
               (const uint8_t *)(__DATE__ " "__TIME__));
  emberAfCorePrint("NCP:  ");
#endif

  printVersion((const uint8_t *)versionName,
               managementVersionNumber,
               stackVersionNumber,
               stackBuildNumber,
               versionType,
               buildTimestamp);
}
