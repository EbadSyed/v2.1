// File: management-client.h
//
// Description: Functions for sending management commands between the
// host and the NCP.
//
// Copyright 2012 by Silicon Laboratories. All rights reserved.             *80*

#ifndef __MANAGEMENT_H__
#define __MANAGEMENT_H__

// Parses unix command line arguments passed to the host app.
// It is implemented in stack/ip/host/management-client.c.
void parseManagementArguments(int argc, char *argv[]);

int emConnectManagementSocket(uint16_t port);

// This function should be called in the main loop of the host app in
// order to process management commands coming from the NCP.
// It is implemented in stack/ip/host/management-client.c.
#if (defined(UNIX_HOST) || defined(UNIX_HOST_SIM) || defined(RTOS))
void processManagementInputStream(void);
void managementCommandTick(void);
#else
  #define processManagementInputStream()
  #define managementCommandTick()
#endif

#endif // #define __MANAGEMENT_H__
