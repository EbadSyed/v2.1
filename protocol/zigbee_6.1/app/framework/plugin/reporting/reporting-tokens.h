/**
 * Custom Application Tokens
 */
#define CREATOR_REPORT_TABLE  (0x8725)

#ifdef DEFINETYPES
// Include or define any typedef for tokens here
#endif //DEFINETYPES
#ifdef DEFINETOKENS
// Define the actual token storage information here

DEFINE_INDEXED_TOKEN(REPORT_TABLE,
                     EmberAfPluginReportingEntry,
                     EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE,
                     { EMBER_ZCL_REPORTING_DIRECTION_REPORTED,
                       EMBER_AF_PLUGIN_REPORTING_UNUSED_ENDPOINT_ID })

#endif //DEFINETOKENS
