// Copyright 2017 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE

// These values are communicated to the user through the plugin option
// descriptions.
#define INVALID_MANUFACTURER_CODE 0xFFFF
#define INVALID_TYPE 0xFFFF
#define INVALID_VERSION 0xFFFFFFFF

// This is super duper arbitrary. It should be updated as needed.
#define MAX_FILE_SPEC_COUNT 16

static void printFileSpec(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                          bool newline)
{
  emberAfCorePrint(" m=0x%2X t=0x%2X v=0x%4X",
                   fileSpec->manufacturerCode,
                   fileSpec->type,
                   fileSpec->version);
  if (newline) {
    emberAfCorePrintln("");
  }
}

static bool useNextVersion(EmberZclOtaBootloadFileVersion_t currentVersion,
                           EmberZclOtaBootloadFileVersion_t nextVersion)
{
  return (false
#ifdef EMBER_AF_PLUGIN_OTA_BOOTLOAD_SERVER_POLICY_ALLOW_UPGRADES
          || (currentVersion < nextVersion)
#endif
#ifdef EMBER_AF_PLUGIN_OTA_BOOTLOAD_SERVER_POLICY_ALLOW_DOWNGRADES
          || (currentVersion > nextVersion)
#endif
#ifdef EMBER_AF_PLUGIN_OTA_BOOTLOAD_SERVER_POLICY_ALLOW_REINSTALLS
          || (currentVersion == nextVersion)
#endif
          );
}

bool emberZclOtaBootloadServerGetImageNotifyInfoCallback(EmberIpv6Address *address,
                                                         EmberZclOtaBootloadFileSpec_t *fileSpec)
{
  emberAfCorePrint("OTA Bootload Server Policy: GetImageNotifyInfo");
  emberAfCorePrint(" a=");
  emberAfCoreDebugExec(emberAfPrintIpv6Address(address));
  printFileSpec(fileSpec, true); // newline?

  // TODO: implement me.
  return false;
}

EmberZclStatus_t emberZclOtaBootloadServerGetNextImageCallback(const EmberIpv6Address *source,
                                                               const EmberZclOtaBootloadFileSpec_t *currentFileSpec,
                                                               EmberZclOtaBootloadFileSpec_t *nextFileSpec)
{
  EmberZclStatus_t status = EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE;
  EmberZclOtaBootloadStorageInfo_t storageInfo;
  EmberZclOtaBootloadFileSpec_t fileSpecs[MAX_FILE_SPEC_COUNT];
  emberZclOtaBootloadStorageGetInfo(&storageInfo, fileSpecs, MAX_FILE_SPEC_COUNT);
  assert(storageInfo.fileCount <  MAX_FILE_SPEC_COUNT);
  for (size_t i = 0; i < storageInfo.fileCount; i++) {
    if (fileSpecs[i].manufacturerCode == currentFileSpec->manufacturerCode
        && (fileSpecs[i].type == currentFileSpec->type
            || fileSpecs[i].type == EMBER_ZCL_OTA_BOOTLOAD_FILE_TYPE_WILDCARD
            || currentFileSpec->type == EMBER_ZCL_OTA_BOOTLOAD_FILE_TYPE_WILDCARD)
        && useNextVersion(currentFileSpec->version, fileSpecs[i].version)) {
      if ((status == EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE) || (nextFileSpec->version < fileSpecs[i].version)) {
        *nextFileSpec = fileSpecs[i];
        status = EMBER_ZCL_STATUS_SUCCESS;
      }
    }
  }

  emberAfCorePrint("OTA Bootload Server Policy: GetNextImageCallback");
  emberAfCorePrint(" a=");
  emberAfCoreDebugExec(emberAfPrintIpv6Address(source));
  printFileSpec(currentFileSpec, false); // newline?
  printFileSpec(nextFileSpec, false); // newline?
  emberAfCorePrintln(" s=0x%X", status);

  return status;
}

uint32_t emberZclOtaBootloadServerUpgradeEndRequestCallback(const EmberIpv6Address *source,
                                                            const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                                            EmberZclStatus_t status)
{
  uint32_t upgradeTime = 0;

  emberAfCorePrint("OTA Bootload Server Policy: UpgradeEndRequest");
  emberAfCorePrint(" a=");
  emberAfCoreDebugExec(emberAfPrintIpv6Address(source));
  printFileSpec(fileSpec, false); // newline?
  emberAfCorePrintln(" s=0x%X t=%d", status, upgradeTime);

  // TODO: implement me.
  return upgradeTime;
}