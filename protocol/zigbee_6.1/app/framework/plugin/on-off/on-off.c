// *******************************************************************
// * on-off.c
// *
// *
// * Copyright 2014 Silicon Laboratories, Inc.                              *80*
// *******************************************************************

#include "af.h"
#include "on-off.h"

#ifdef EMBER_AF_PLUGIN_REPORTING
  #include "app/framework/plugin/reporting/reporting.h"
#endif

#ifdef EMBER_AF_PLUGIN_SCENES
  #include "../scenes/scenes.h"
#endif //EMBER_AF_PLUGIN_SCENES

#ifdef EMBER_AF_PLUGIN_ZLL_ON_OFF_SERVER
  #include "../zll-on-off-server/zll-on-off-server.h"
#endif

#ifdef EMBER_AF_PLUGIN_ZLL_LEVEL_CONTROL_SERVER
  #include "../zll-level-control-server/zll-level-control-server.h"
#endif

EmberAfStatus emberAfOnOffClusterSetValueCallback(uint8_t endpoint,
                                                  uint8_t command,
                                                  bool initiatedByLevelChange)
{
  EmberAfStatus status;
  bool currentValue, newValue;

  emberAfOnOffClusterPrintln("On/Off set value: %x %x", endpoint, command);

  // read current on/off value
  status = emberAfReadAttribute(endpoint,
                                ZCL_ON_OFF_CLUSTER_ID,
                                ZCL_ON_OFF_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                (uint8_t *)&currentValue,
                                sizeof(currentValue),
                                NULL); // data type
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfOnOffClusterPrintln("ERR: reading on/off %x", status);
    return status;
  }

  // if the value is already what we want to set it to then do nothing
  if ((!currentValue && command == ZCL_OFF_COMMAND_ID)
      || (currentValue && command == ZCL_ON_COMMAND_ID)) {
    emberAfOnOffClusterPrintln("On/off already set to new value");
    return EMBER_ZCL_STATUS_SUCCESS;
  }

  // we either got a toggle, or an on when off, or an off when on,
  // so we need to swap the value
  newValue = !currentValue;
  emberAfOnOffClusterPrintln("Toggle on/off from %x to %x", currentValue, newValue);

  // write the new on/off value
  status = emberAfWriteAttribute(endpoint,
                                 ZCL_ON_OFF_CLUSTER_ID,
                                 ZCL_ON_OFF_ATTRIBUTE_ID,
                                 CLUSTER_MASK_SERVER,
                                 (uint8_t *)&newValue,
                                 ZCL_BOOLEAN_ATTRIBUTE_TYPE);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    emberAfOnOffClusterPrintln("ERR: writing on/off %x", status);
    return status;
  }

  // If initiatedByLevelChange is true, then we assume that the level change
  // ZCL stuff has already happened.
  if (!initiatedByLevelChange
      && emberAfContainsServer(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID)) {
    emberAfOnOffClusterLevelControlEffectCallback(endpoint,
                                                  newValue);
  }

#ifdef EMBER_AF_PLUGIN_ZLL_ON_OFF_SERVER
  if (initiatedByLevelChange) {
    emberAfPluginZllOnOffServerLevelControlZllExtensions(endpoint);
  }
#endif

  // the scene has been changed (the value of on/off has changed) so
  // the current scene as descibed in the attribute table is invalid,
  // so mark it as invalid (just writes the valid/invalid attribute)
  if (emberAfContainsServer(endpoint, ZCL_SCENES_CLUSTER_ID)) {
    emberAfScenesClusterMakeInvalidCallback(endpoint);
  }

  // The returned status is based solely on the On/Off cluster.  Errors in the
  // Level Control and/or Scenes cluster are ignored.
  return EMBER_ZCL_STATUS_SUCCESS;
}

bool emberAfOnOffClusterOffCallback(void)
{
  EmberAfStatus status = emberAfOnOffClusterSetValueCallback(emberAfCurrentEndpoint(),
                                                             ZCL_OFF_COMMAND_ID,
                                                             false);
#ifdef EMBER_AF_PLUGIN_ZLL_ON_OFF_SERVER
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    emberAfPluginZllOnOffServerOffZllExtensions(emberAfCurrentCommand());
  }
#endif
  emberAfSendImmediateDefaultResponse(status);
  return true;
}

bool emberAfOnOffClusterOnCallback(void)
{
  EmberAfStatus status = emberAfOnOffClusterSetValueCallback(emberAfCurrentEndpoint(),
                                                             ZCL_ON_COMMAND_ID,
                                                             false);
#ifdef EMBER_AF_PLUGIN_ZLL_ON_OFF_SERVER
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    emberAfPluginZllOnOffServerOnZllExtensions(emberAfCurrentCommand());
  }
#endif
  emberAfSendImmediateDefaultResponse(status);
  return true;
}

bool emberAfOnOffClusterToggleCallback(void)
{
  EmberAfStatus status = emberAfOnOffClusterSetValueCallback(emberAfCurrentEndpoint(),
                                                             ZCL_TOGGLE_COMMAND_ID,
                                                             false);
#ifdef EMBER_AF_PLUGIN_ZLL_ON_OFF_SERVER
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    emberAfPluginZllOnOffServerToggleZllExtensions(emberAfCurrentCommand());
  }
#endif
  emberAfSendImmediateDefaultResponse(status);
  return true;
}
