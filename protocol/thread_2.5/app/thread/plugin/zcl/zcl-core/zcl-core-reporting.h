#ifndef __ZCL_CORE_REPORTING_H__
#define __ZCL_CORE_REPORTING_H__

#ifndef EM_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES
  #define EM_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES 32
#endif

// Define type for nv (Token based) reporting configurations.
// (The nv reporting configurations table is used to mirror the volatile reporting
// configurations in zcl-core-reporting.c:- volatile configurations are restored
// from nv at power-up, nv configurations are created/modified when the associated
// volatile configuration is modified.
typedef struct {
  EmberZclEndpointId_t endpointId;
  EmberZclClusterSpec_t clusterSpec;
  EmberZclReportingConfigurationId_t reportingConfigurationId;
  size_t sizeReportableChanges;
  uint16_t minimumIntervalS;
  uint16_t maximumIntervalS;
  uint8_t reportableChanges[EM_ZCL_MAX_SIZE_NV_REPORTABLE_CHANGES];
} EmZclNvReportingConfiguration_t;

#endif // __ZCL_CORE_REPORTING_H__
