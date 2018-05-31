// Copyright 2017 Silicon Labs, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_CONNECTION_MANAGER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_COMMAND_INTERPRETER2
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

// connection-manager start-connect
void connectionManagerStartConnectCommand(void)
{
  emberConnectionManagerStartConnect();
}

// connection-manager stop-connect
void connectionManagerStopConnectCommand(void)
{
  emberConnectionManagerStopConnect();
}

// connection-manager leave-network
void connectionManagerLeaveNetworkCommand(void)
{
  emberConnectionManagerLeaveNetwork();
}

// connection-manager search-parent
void connectionManagerSearchParentCommand(void)
{
  emberConnectionManagerSearchForParent();
}
