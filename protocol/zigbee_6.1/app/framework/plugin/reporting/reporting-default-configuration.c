// *******************************************************************
// * reporting default configuration.c
// *
// *
// * Copyright 2017 Silicon Laboratories, Inc.                              *80*
// *******************************************************************

#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "app/framework/util/attribute-storage.h"
#include "reporting.h"

#define REPORT_FAILED 0xFF

#if !(defined GENERATED_REPORTING_CONFIG_DEFAULTS)
  #include "test-reporting-config-header.h"
#else
PGM EmberAfPluginReportingEntry generatedReportingConfigDefaults[] = GENERATED_REPORTING_CONFIG_DEFAULTS;
  #define GENERATED_TABLE_SIZE (sizeof(generatedReportingConfigDefaults) / sizeof(EmberAfPluginReportingEntry))
#endif

// Load Default reporting configuration from generated table
void emberAfPluginReportingLoadReportingConfigDefaults(void)
{
  int tableSize = GENERATED_TABLE_SIZE;
  if (tableSize == 0) {
    return;
  }
  emberAfCorePrintln("GENERATED_TABLE_SIZE = %d, EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE=%d", tableSize, EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE);
  for (int i = 0; i < tableSize; i++) {
    uint8_t reportEntry = emAfPluginReportingConditionallyAddReportingEntry((EmberAfPluginReportingEntry *)&generatedReportingConfigDefaults[i]);
    emberAfCorePrintln("Dflt Server Rpt reportEntry %d Config[%d] epId=%X, cId=%02X, aId=%02X, mnI=%02X, mxI=%02X, ch=%04X - [%p]",
                       reportEntry, i,
                       generatedReportingConfigDefaults[i].endpoint,
                       generatedReportingConfigDefaults[i].clusterId,
                       generatedReportingConfigDefaults[i].attributeId,
                       generatedReportingConfigDefaults[i].data.reported.minInterval,
                       generatedReportingConfigDefaults[i].data.reported.maxInterval,
                       generatedReportingConfigDefaults[i].data.reported.reportableChange,
                       reportEntry == 0 ? "Skiped" : "Set");
    // This assert will be hit if all the reportable attributes do not
    // have enough space in reporting table to make a default entry
    assert(reportEntry != REPORT_FAILED);
  }
}

// Get default reporting values - returns true if there is default value
// avilable either in the table or a call back to application
bool emberAfPluginReportingGetReportingConfigDefaults(EmberAfPluginReportingEntry *defaultConfiguration)
{
  // NULL error check - return false.
  if (NULL == defaultConfiguration) {
    return false;
  }
  // When there is a table table available - search and read the values,
  // if default config value found, retrun true to the caller to use it
  int tableSize = GENERATED_TABLE_SIZE;
  for (int i = 0; i < tableSize; i++) {
    // All of the serach parameters match then copy the default config.
    if ((defaultConfiguration->endpoint == generatedReportingConfigDefaults[i].endpoint)
        && (defaultConfiguration->clusterId == generatedReportingConfigDefaults[i].clusterId)
        && (defaultConfiguration->attributeId == generatedReportingConfigDefaults[i].attributeId)
        && (defaultConfiguration->mask == generatedReportingConfigDefaults[i].mask)
        && (defaultConfiguration->manufacturerCode == generatedReportingConfigDefaults[i].manufacturerCode)) {
      defaultConfiguration->direction = EMBER_ZCL_REPORTING_DIRECTION_REPORTED;
      defaultConfiguration->data = generatedReportingConfigDefaults[i].data;
      return true;
    }
  }
  // At this point - There is no entry in the generated deafult table,
  // so the application need to be called to get default reporting values
  // The application should provide the default values because , in BDB an
  // implemented reportable attribute will reset its reporting configuration
  // when receives a special case of minInterval and maxInterval for which
  // function gets called.
  if (emberAfPluginReportingGetDefaultReportingConfigCallback(defaultConfiguration)) {
    return true;
  }
  return false;
}
