/// *******************************************************************
// * gbcs-device-log.h
// *
// *
// * Copyright 2014 Silicon Laboratories, Inc.                              *80*
// *******************************************************************

#ifndef GBCS_DEVICE_LOG_H_
#define GBCS_DEVICE_LOG_H_

// Printing macros for plugin: GBCS Device Log
#define emberAfPluginGbcsDeviceLogPrint(...)    emberAfAppPrint(__VA_ARGS__)
#define emberAfPluginGbcsDeviceLogPrintln(...)  emberAfAppPrintln(__VA_ARGS__)
#define emberAfPluginGbcsDeviceLogDebugExec(x)  emberAfAppDebugExec(x)

typedef enum {
  EMBER_AF_GBCS_GSME_DEVICE_TYPE   = 0x00,
  EMBER_AF_GBCS_ESME_DEVICE_TYPE   = 0x01,
  EMBER_AF_GBCS_CHF_DEVICE_TYPE    = 0x02,
  EMBER_AF_GBCS_GPF_DEVICE_TYPE    = 0x03,
  EMBER_AF_GBCS_HCALCS_DEVICE_TYPE = 0x04,
  EMBER_AF_GBCS_PPMID_DEVICE_TYPE  = 0x05,
  EMBER_AF_GBCS_TYPE2_DEVICE_TYPE  = 0x06,
} EmberAfGBCSDeviceType;

typedef struct {
  EmberAfGBCSDeviceType deviceType;
} EmberAfGBCSDeviceLogInfo;

/**
 * @brief Clear all entries from the GBCS Device Log.
 */
void emberAfPluginGbcsDeviceLogClear(void);

/**
 * @brief Reset the GBCS Device Log to include just those entries configured with the plugin.
 *
 * This function will clear the device log then add each of the devices configured
 * configured with the plugin.
 */
void emberAfPluginGbcsDeviceLogReset(void);

/**
 * @brief Get the number of entries in the GBCS Device Log.
 *
 * @return The number of entries currently in the GBCS Device Log..
 */
uint8_t emberAfPluginGbcsDeviceLogCount(void);

/**
 * @brief Add or update an entry within the GBCS Device Log.
 *
 * This function can be used to add or update an entry within the GBCS Device Log.
 *
 * @param deviceId The EUI64 of the device to be added or updated.
 * @param deviceInfo Pointer the a structure containing the deviceType of the
 * device to be added or updated.
 * @return true if successfully added or updated, false if there is insufficient
 * room in the device log to add the new entry.  The size of the device log is
 * controlled by the "Device Log Size" plugin configuration parameter.
 */
bool emberAfPluginGbcsDeviceLogStore(EmberEUI64 deviceId,
                                     EmberAfGBCSDeviceLogInfo *deviceInfo);

/**
 * @brief Remove an entry from the GBCS Device Log.
 *
 * This function can be used to remove an entry within the GBCS Device Log.
 *
 * @param deviceId The EUI64 of the device to be removed.
 * @return true if successfully removed, false if the device is not currently
 * in the GBCS Device Log.
 */
bool emberAfPluginGbcsDeviceLogRemove(EmberEUI64 deviceId);

/**
 * @brief Get device info about an entry in the GBCS Device Log
 *
 * This function can be used to retrieve information pertaining to a device
 * in the GBCS Device Log. Note that if a single device implements two
 * device types (i.e comms hub implements both the CHF and GPF) then
 * only the first device found is returned.
 *
 * @param deviceId The EUI64 of the device for which information is requested.
 * @return true if device is in the device log, false otherwise. The contents of
 * the deviceInfo structure will be populated when true is returned.
 */
bool emberAfPluginGbcsDeviceLogGet(EmberEUI64 deviceId,
                                   EmberAfGBCSDeviceLogInfo *deviceInfo);

/**
 * @brief Check to see if the given device exists within the GBCS Device Log.
 *
 * This function can be used to check if the given device and type exists within
 * the GBCS Device Log..
 *
 * @param deviceId The EUI64 of the device in question
 * @param deviceType The device type of the device in question.
 * @return true if device is in the device log, false otherwise.
 */
bool emberAfPluginGbcsDeviceLogExists(EmberEUI64 deviceId,
                                      EmberAfGBCSDeviceType deviceType);

/**
 * @brief Print all entries in the GBCS Device log
 */
void emberAfPluginGbcsDeviceLogPrintEntries(void);

/**
 * @brief Is the given device type a sleepy device.
 *
 * @param deviceType The GBCS device type to be checked.
 * @return true if the deviceType is a sleepy device, false otherwise
 */
#define emberAfPluginGbcsDeviceLogIsSleepyType(deviceType) ((deviceType) == EMBER_AF_GBCS_GSME_DEVICE_TYPE)

/**
 * @brief Retrieves the device from the log at the specified index.
 *
 * @param index The absolute index into the device log.
 * @param returnDeviceId a pointer where the EUI64 return data will be written.
 * @param returnDeviceInfo a pointer where the device info return data will be written.
 * @return true if the index has a valid entry.
 *   false otherwise.
 */
bool emberAfPluginGbcsDeviceLogRetrieveByIndex(uint8_t index,
                                               EmberEUI64 returnDeviceId,
                                               EmberAfGBCSDeviceLogInfo* returnDeviceInfo);

/**
 * @ brief Returns the max size of the device log, including currently empty entries.
 *
 * @return an uint8_t indicating the maximum size.
 */
uint8_t emberAfPluginGbcsDeviceLogMaxSize(void);

#endif /* GBCS_DEVICE_LOG_H_ */
