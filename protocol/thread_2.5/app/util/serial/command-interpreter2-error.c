/**
 * File: command-interpreter2-error.c
 * Description: processes commands incoming over the serial port.
 *
 * Culprit(s): Richard Kelsey, Matteo Paris
 *
 * Copyright 2008 by Ember Corporation.  All rights reserved.               *80*
 */

#include PLATFORM_HEADER

#include "stack/include/ember.h"
#include "hal/hal.h"
#include "plugin/serial/serial.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/util/serial/command-interpreter2-util.h"

#if !defined APP_SERIAL
extern uint8_t serialPort;
  #define APP_SERIAL serialPort
#endif

#ifndef EMBER_COMMAND_INTERPRETER_NO_ERROR_MESSAGE
const char * const emberCommandErrorNames[] =
{
  "",
  "Serial port error",
  "No such command",
  "Wrong number of args",
  "Arg out of range",
  "Arg syntax error",
  "Too long",
  "Bad arg type"
};
#endif // EMBER_COMMAND_INTERPRETER_NO_ERROR_MESSAGE

static void printCommandUsage(EmberCommandEntry *entry)
{
  const char * arg = entry->argumentTypes;
  emberSerialPrintf(APP_SERIAL, "%p", entry->identifier.name);

  if (entry->action == NULL) {
    emberSerialPrintf(APP_SERIAL, "...");
  } else {
    while (*arg) {
      uint8_t c = *arg;
      emberSerialPrintf(APP_SERIAL,
                        (c == 'u' ? " <uint8_t>"
                         : c == 'v' ? " <uint16_t>"
                         : c == 'w' ? " <uint32_t>"
                         : c == 's' ? " <int8_t>"
                         : c == 'b' ? " <string>"
                         : c == 'n' ? " ..."
                         : c == '*' ? " *"
                         : " ?"));
      arg += 1;
    }
  }

#ifdef EMBER_COMMAND_INTERPRETER_HAS_DESCRIPTION_FIELD
  if (entry->description) {
    emberSerialPrintf(APP_SERIAL, " - %p", entry->description);
  }
#endif // EMBER_COMMAND_INTERPRETER_HAS_DESCRIPTION_FIELD

  emberSerialPrintf(APP_SERIAL, "\r\n");
}

void emberPrintCommandUsage(EmberCommandEntry *entry)
{
  EmberCommandEntry *commandFinger;
  printCommandUsage(entry);

  if (emberGetNestedCommand(entry, &commandFinger)) {
    for (; commandFinger->identifier.name != NULL; commandFinger++) {
      emberSerialPrintf(APP_SERIAL, "  ");
      printCommandUsage(commandFinger);
    }
  }
}

void emberPrintCommandUsageNotes(void)
{
  emberSerialPrintf(APP_SERIAL,
                    "Usage notes:\r\n\r\n"
#ifndef EMBER_COMMAND_INTERPRETER_NO_ERROR_MESSAGE
                    "type      description\r\n"
                    "<uint8_t>   8-bit unsigned int, eg: 255, 0xAB\r\n"
                    "<int8_t>   8-bit signed int, eg: -128, 0xA9\r\n"
                    "<uint16_t>  16-bit unsigned int, eg: 3000 0xFFAA\r\n"
                    "<string>  A string, eg: \"foo\" or {0A 1B 2C}\r\n"
                    "*         Zero or more of the previous type\r\n\r\n"
#endif // EMBER_COMMAND_INTERPRETER_NO_ERROR_MESSAGE
                    );
  emberSerialWaitSend(APP_SERIAL);
}

void emberPrintCommandTable(void)
{
  EmberCommandEntry *commandFinger = COMMAND_TABLE;
  emberPrintCommandUsageNotes();
  for (; commandFinger->identifier.name != NULL; commandFinger++) {
    printCommandUsage(commandFinger);
  }
}

void emberCommandErrorHandler(EmberCommandStatus status,
                              EmberCommandEntry *command)
{
#ifdef EMBER_COMMAND_INTERPRETER_NO_ERROR_MESSAGE
  emberSerialPrintf(APP_SERIAL, "Error\r\n");
#else // EMBER_COMMAND_INTERPRETER_NO_ERROR_MESSAGE
  emberSerialPrintf(APP_SERIAL, "Error: %p\r\n", emberCommandErrorNames[status]);
#endif // EMBER_COMMAND_INTERPRETER_NO_ERROR_MESSAGE

  if (command != NULL) {
    emberSerialPrintf(APP_SERIAL, "Usage: ");
    emberPrintCommandUsage(command);
  }
}
