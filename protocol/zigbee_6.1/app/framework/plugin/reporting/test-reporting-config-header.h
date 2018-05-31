// *******************************************************************
// * test reporting default configuration header file
// * this file is hand created to run the default reporting tests
// *
// * Copyright 2017 Silicon Laboratories, Inc.                              *80*
// *******************************************************************
#ifndef SILABS_TEST_REPORTING_CONFIG_HEADER_H
#define SILABS_TEST_REPORTING_CONFIG_HEADER_H

#define GENERATED_TABLE_SIZE generateTable()

#define REPORTING_DEFAULT_MIN_INTERVAL_VALUE 1
#define REPORTING_DEFAULT_MAX_INTERVAL_VALUE 0xFFFE
#define REPORTING_DEFAULT_CHANGE_VALUE       1

typedef struct {
  uint16_t clusterId;
  uint16_t attributeId;
  uint16_t minInterval;
  uint16_t maxInterval;
  uint32_t reportableChange;
}EmberAfPluginReportingMandatoryReportableAttributeEntry;

PGM EmberAfPluginReportingMandatoryReportableAttributeEntry mandatoryReportableTable[] = {
  // On Off
  #ifdef ZCL_USING_ON_OFF_CLUSTER_SERVER
  { ZCL_ON_OFF_CLUSTER_ID,
    ZCL_ON_OFF_ATTRIBUTE_ID,
    REPORTING_DEFAULT_MIN_INTERVAL_VALUE,
    REPORTING_DEFAULT_MAX_INTERVAL_VALUE,
    REPORTING_DEFAULT_CHANGE_VALUE, },
  #endif

  // Level Control
  #ifdef ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
  { ZCL_LEVEL_CONTROL_CLUSTER_ID,
    ZCL_CURRENT_LEVEL_ATTRIBUTE_ID,
    REPORTING_DEFAULT_MIN_INTERVAL_VALUE,
    REPORTING_DEFAULT_MAX_INTERVAL_VALUE,
    REPORTING_DEFAULT_CHANGE_VALUE, },
  #endif

  // Color Control
  #ifdef ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
    #ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV
  { ZCL_COLOR_CONTROL_CLUSTER_ID,
    ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID,
    EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_MIN_REPORT_INTERVAL,
    EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_MAX_REPORT_INTERVAL,
    EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HUE_CHANGE, },
  { ZCL_COLOR_CONTROL_CLUSTER_ID,
    ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
    EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_MIN_REPORT_INTERVAL,
    EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_MAX_REPORT_INTERVAL,
    EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_SATURATION_CHANGE, },
    #endif
    #ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY
  { ZCL_COLOR_CONTROL_CLUSTER_ID,
    ZCL_COLOR_CONTROL_CURRENT_X_ATTRIBUTE_ID,
    EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_MIN_REPORT_INTERVAL,
    EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_MAX_REPORT_INTERVAL,
    EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_COLOR_XY_CHANGE, },
  { ZCL_COLOR_CONTROL_CLUSTER_ID,
    ZCL_COLOR_CONTROL_CURRENT_Y_ATTRIBUTE_ID,
    EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_MIN_REPORT_INTERVAL,
    EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_MAX_REPORT_INTERVAL,
    EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_COLOR_XY_CHANGE, },
    #endif
    #ifdef EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP
  { ZCL_COLOR_CONTROL_CLUSTER_ID,
    ZCL_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE_ID,
    EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_MIN_REPORT_INTERVAL,
    EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_MAX_REPORT_INTERVAL,
    EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_COLOR_TEMP_CHANGE, },
    #endif
  #endif
  // End of the table to take care of not to generate a blank array
  // when non of the above clusters are defined - Temporary fix, this file will
  // not be included when app builder support is in to genetrate this table.
  #if (!defined ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER    \
       && !defined ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER \
       && !defined ZCL_USING_ON_OFF_CLUSTER_SERVER)
  { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFFFFFF },
  #endif
};

static EmberAfPluginReportingEntry generatedReportingConfigDefaults[EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE];

static void emberAfPluginReportingSetDefaultReportingConfiguration(EmberAfPluginReportingEntry *defaultConfiguration, int *count)
{
  int size = sizeof(mandatoryReportableTable) / sizeof(EmberAfPluginReportingMandatoryReportableAttributeEntry);

  for (int i = 0; i < size; i++) {
    // Find if the endpoint has a cluster that can have mandatory attributes
    if (emberAfContainsCluster(defaultConfiguration->endpoint, mandatoryReportableTable[i].clusterId)) {
      defaultConfiguration->clusterId = mandatoryReportableTable[i].clusterId;
      // Find if the cluster has the mandatory reportable attribute
      if ( emberAfContainsServer(defaultConfiguration->endpoint, defaultConfiguration->clusterId)) {
        defaultConfiguration->attributeId = mandatoryReportableTable[i].attributeId;
        defaultConfiguration->data.reported.minInterval = mandatoryReportableTable[i].minInterval;
        defaultConfiguration->data.reported.maxInterval = mandatoryReportableTable[i].maxInterval;
        defaultConfiguration->data.reported.reportableChange = mandatoryReportableTable[i].reportableChange;
        if ((EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE) > *count ) {
          MEMMOVE(&generatedReportingConfigDefaults[*count],
                  defaultConfiguration,
                  sizeof(EmberAfPluginReportingEntry));
          *count = (*count) + 1;
        } else {
          return;
        }
      }
    }
  }
}

static void emberAfPluginReportingInitSetupDefaultConfiguration(int *count)
{
  // set up common reporting entry parameters here.
  EmberAfPluginReportingEntry defaultConfiguration = {
    EMBER_ZCL_REPORTING_DIRECTION_REPORTED, //direction
    0, //endpoint, will be populated below
    0, //clusterId, will be populated
    0, //attributeId, will be populated
    CLUSTER_MASK_SERVER, //mask, look for servers
    EMBER_AF_NULL_MANUFACTURER_CODE, //manufacturerCode
    .data.reported = {
      1, //minInterval
      0xffff, //maxInterval
      1 //reportableChange
    }
  };

  for (int i = 0; i < emberAfEndpointCount(); i++) {
    defaultConfiguration.endpoint = emberAfEndpointFromIndex(i);
    emberAfPluginReportingSetDefaultReportingConfiguration(&defaultConfiguration,
                                                           count);
  }
}

static int generateTable(void)
{
#if defined (__APP_FULLTH_H__)
  return 0;
#endif
  static bool alreadyGenerated = false;
  static int count = 0;
  if (false == alreadyGenerated) {
    emberAfPluginReportingInitSetupDefaultConfiguration(&count);
    alreadyGenerated = true;
  }
  return count;
}
#endif // SILABS_TEST_REPORTING_CONFIG_HEADER_H
