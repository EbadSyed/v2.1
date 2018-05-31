// Copyright 2015 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE
#ifdef EMBER_AF_API_ZCL_LEVEL_CONTROL_SERVER
  #include EMBER_AF_API_ZCL_LEVEL_CONTROL_SERVER
  #define SET_ON_OFF(endpointId, onOff) emberZclLevelControlServerSetOnOff(endpointId, onOff)
#else
static void setOnOff(EmberZclEndpointId_t endpointId, bool onOff);
  #define SET_ON_OFF(endpointId, onOff) setOnOff(endpointId, onOff)
#endif

static void setOnOffHandler(const EmberZclCommandContext_t *context, bool onOff);
static bool getOnOff(EmberZclEndpointId_t endpointId);

void emberZclClusterOnOffServerCommandOffRequestHandler(const EmberZclCommandContext_t *context,
                                                        const EmberZclClusterOnOffServerCommandOffRequest_t *request)
{
  emberAfCorePrintln("RX: Off");
  setOnOffHandler(context, false); // off
}

void emberZclClusterOnOffServerCommandOnRequestHandler(const EmberZclCommandContext_t *context,
                                                       const EmberZclClusterOnOffServerCommandOnRequest_t *request)
{
  emberAfCorePrintln("RX: On");
  setOnOffHandler(context, true); // on
}

void emberZclClusterOnOffServerCommandToggleRequestHandler(const EmberZclCommandContext_t *context,
                                                           const EmberZclClusterOnOffServerCommandToggleRequest_t *request)
{
  emberAfCorePrintln("RX: Toggle");
  setOnOffHandler(context, !getOnOff(context->endpointId));
}

static bool getOnOff(EmberZclEndpointId_t endpointId)
{
  bool onOff;
  if (emberZclReadAttribute(endpointId,
                            &emberZclClusterOnOffServerSpec,
                            EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                            &onOff,
                            sizeof(onOff))
      == EMBER_ZCL_STATUS_SUCCESS) {
    return onOff;
  } else {
    return false;
  }
}

#ifndef EMBER_AF_API_ZCL_LEVEL_CONTROL_SERVER
static void setOnOff(EmberZclEndpointId_t endpointId, bool onOff)
{
  emberZclWriteAttribute(endpointId,
                         &emberZclClusterOnOffServerSpec,
                         EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                         &onOff,
                         sizeof(onOff));
}
#endif

static void setOnOffHandler(const EmberZclCommandContext_t *context, bool onOff)
{
  SET_ON_OFF(context->endpointId, onOff);
  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
}
