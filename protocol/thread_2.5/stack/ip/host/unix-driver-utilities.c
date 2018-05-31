// File: unix-driver-utilities.c
//
// Description: Utilities and stubs for the ip driver on a Unix host.
//
// Copyright 2012 by Ember Corporation. All rights reserved.                *80*

#include PLATFORM_HEADER
#include "stack/core/ember-stack.h"
#include "hal/hal.h"
#include "phy/phy.h"
#include "plugin/serial/serial.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/ip-ncp/binary-management.h"

void emberInit(void)
{
  emberTaskEnableIdling(true);
  emberCommandReaderInit();
  emSendBinaryManagementCommand(EMBER_INIT_HOST_COMMAND_IDENTIFIER, "");
  emberSerialPrintfLine(APP_SERIAL, "thread-app reset");
}

void emberTick(void)
{
}
