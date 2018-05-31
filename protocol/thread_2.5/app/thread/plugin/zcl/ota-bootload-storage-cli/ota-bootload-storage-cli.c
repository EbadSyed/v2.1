// Copyright 2017 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_DEBUG_PRINT
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_CORE
#include EMBER_AF_API_ZCL_OTA_BOOTLOAD_STORAGE_CORE

#define MAX_FILE_SPECS 4

// ota-bootload-storage info
void otaBootloadStorageInfoCommand(void)
{
  EmberZclOtaBootloadStorageInfo_t info;
  EmberZclOtaBootloadFileSpec_t fileSpecs[MAX_FILE_SPECS];
  emberZclOtaBootloadStorageGetInfo(&info, fileSpecs, MAX_FILE_SPECS);
  emberAfAppPrintln("OTA Bootload Storage Info: maximumFileSize = %d, fileCount = %d",
                    info.maximumFileSize,
                    info.fileCount);
  for (size_t i = 0; i < info.fileCount; i++) {
    emberAfAppPrint("File %d:", i);

    EmberZclOtaBootloadStorageFileInfo_t fileInfo;
    EmberZclOtaBootloadStorageStatus_t storageStatus
      = emberZclOtaBootloadStorageFind(&fileSpecs[i], &fileInfo);
    if (storageStatus != EMBER_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS) {
      emberAfAppPrintln(" ERROR");
    } else {
      emberAfAppPrintln(" m=0x%2X t=0x%2X v=0x%4X size = %d",
                        fileSpecs[i].manufacturerCode,
                        fileSpecs[i].type,
                        fileSpecs[i].version,
                        fileInfo.size);
    }
  }
}
