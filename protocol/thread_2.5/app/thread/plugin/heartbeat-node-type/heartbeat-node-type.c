// Copyright 2015 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_HAL

void emberAfPluginHeartbeatNodeType(HalBoardLed ledPin);

void emberAfPluginHeartbeatNodeTypeInit(void)
{
  emberAfPluginHeartbeatNodeType(BOARD_HEARTBEAT_LED);
}

void emberAfPluginHeartbeatNodeTypeEventHandler(void)
{
  emberAfPluginHeartbeatNodeType(BOARD_HEARTBEAT_LED);
}
