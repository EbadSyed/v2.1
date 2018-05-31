// File: log.c
//
// Description: Utilities to log by type to a serial port.
//
// Copyright 2011 by Ember Corporation. All rights reserved.                *80*

#include PLATFORM_HEADER
#include "stack/include/ember-types.h"
#include "stack/core/ember-stack.h"
#include "hal/hal.h"
#include "plugin/serial/serial.h"
#include "app/util/serial/command-interpreter2.h"

static uint32_t portMasks[EM_LOG_PORT_COUNT] = { 0 };

static const char * const emLogTypeNames[] = {
  EM_LOG_TYPE_NAMES
};

static const uint32_t logEnabledMask = EM_LOG_ENABLED_MASK;

#ifdef EMBER_TEST

#ifdef EMBER_SCRIPTED_TEST

#define logPrintfVarArg(port, format, ap)  \
  do {                                     \
    logPrintfStderrVarArg((format), (ap)); \
  } while (0)

#define logPrintf(port, ...)      \
  do {                            \
    logPrintfStderr(__VA_ARGS__); \
  } while (0)

#else

// Todo: this silliness should be taken care of by the ember serial library.
#define logPrintfVarArg(port, format, ap)              \
  do {                                                 \
    if (port == EM_LOG_PORT_STDERR) {                  \
      logPrintfStderrVarArg((format), (ap));           \
    } else {                                           \
      emberSerialPrintfVarArg((port), (format), (ap)); \
    }                                                  \
  } while (0)

#define logPrintf(port, ...)                \
  do {                                      \
    if (port == EM_LOG_PORT_STDERR) {       \
      logPrintfStderr(__VA_ARGS__);         \
    } else {                                \
      emberSerialPrintf(port, __VA_ARGS__); \
    }                                       \
  } while (0)

#endif

// Todo: this silliness is required because the ember serial library uses
// non-standard format specifiers.  When we change serial.c to use the
// IAR standard library for formatting, this will go away.
// Note: this code was pasted from simple-linux-serial.c.
static void logPrintfStderrVarArg(PGM_P string, va_list args)
{
  for (; *string != 0; string++) {
    uint8_t next = *string;
    if (next != '%') {
      fprintf(stderr, "%c", next);
    } else {
      string += 1;
      switch (*string) {
        case '%':
          fprintf(stderr, "%%");
          break;
        case 'c':
          fprintf(stderr, "%c", va_arg(args, unsigned int) & 0xFF);
          break;
        case 'p':
        case 's':
          fputs(va_arg(args, uint8_t *), stderr);
          break;
        case 'l':
          if (string[1] == 'u') {
            fprintf(stderr, "%lu", va_arg(args, unsigned long int));
            string++;
          }
          break;
        case 'u':
          fprintf(stderr, "%u", va_arg(args, int));
          break;
        case 'd':
          fprintf(stderr, "%d", va_arg(args, int) & 0xFFFF);
          break;
        case 'x':
        case 'X':
          fprintf(stderr, "%02X", va_arg(args, int));
          break;
        case '2':
        case '4':
          if (string[1] == 'x' || string[1] == 'X') {
            if (*string == '2') {
              fprintf(stderr, "%04X", va_arg(args, int));
            } else {
              fprintf(stderr, "%08X", va_arg(args, uint32_t));
            }
            string++;
          }
          break;
      }
    }
  }
  fflush(stderr);
}

static void logPrintfStderr(PGM_P formatString, ...)
{
  va_list ap;
  va_start(ap, formatString);
  logPrintfStderrVarArg(formatString, ap);
  va_end(ap);
}

#else

#define logPrintfVarArg(port, format, ap) \
  emberSerialPrintfVarArg(port, (format), (ap));

#define logPrintf(port, ...) \
  emberSerialPrintf(port, __VA_ARGS__);

#endif

uint8_t emLogTypeFromName(uint8_t *name, uint8_t nameLength)
{
  uint8_t i;
  for (i = 0; i < COUNTOF(emLogTypeNames); i++) {
    if (MEMCOMPARE(emLogTypeNames[i], name, nameLength) == 0
        && emLogTypeNames[i][nameLength] == '\0') {
      return i;
    }
  }
  return 0xFF;
}

void emLogConfig(uint8_t logType, uint8_t port, bool on)
{
  if (logType < EM_LOG_TYPE_COUNT && port < EM_LOG_PORT_COUNT) {
    if (on) {
      portMasks[port] |= (BIT32(logType) & logEnabledMask);
    } else {
      portMasks[port] &= ~BIT32(logType);
    }
  }
}

static bool typeAndPortEnabled(uint8_t logType, uint8_t port)
{
  return ((portMasks[port] & BIT32(logType)) != 0);
}

bool emReallyLogIsActive(uint8_t logType)
{
  uint8_t port;
  for (port = 0; port < EM_LOG_PORT_COUNT; port++) {
    if (typeAndPortEnabled(logType, port)) {
      return true;
    }
  }
  return false;
}

void emReallyLog(uint8_t logType, PGM_P formatString, ...)
{
  va_list ap;
  uint8_t port;
  for (port = 0; port < EM_LOG_PORT_COUNT; port++) {
    if (typeAndPortEnabled(logType, port)) {
      va_start(ap, formatString);
#ifdef UNIX_SIMULATION
      if (port == EM_LOG_PORT_STDERR
          && strcmp(formatString, "\r\n") == 0) {
        logPrintfVarArg(port, "\n", ap);
      } else
#endif
      logPrintfVarArg(port, formatString, ap);
      va_end(ap);
    }
  }
}

void emReallyStartLogLine(uint8_t logType)
{
#ifdef UNIX_SIMULATION
  if (typeAndPortEnabled(logType, EM_LOG_PORT_STDERR)) {
    simPrintStartLine();
  }
#endif
  emReallyLog(logType, "{%s} ", emLogTypeNames[logType]);
}

void emReallyEndLogLine(uint8_t logType)
{
#ifdef UNIX_SIMULATION
  if (typeAndPortEnabled(logType, EM_LOG_PORT_STDERR)) {
    putc(']', stderr);
  }
#endif
  emReallyLog(logType, "\r\n");
}

void emReallyLogLine(uint8_t logType, PGM_P formatString, ...)
{
  uint8_t port;
  emReallyStartLogLine(logType);
  for (port = 0; port < EM_LOG_PORT_COUNT; port++) {
    if (typeAndPortEnabled(logType, port)) {
      va_list ap;
      va_start(ap, formatString);
      logPrintfVarArg(port, formatString, ap);
      va_end(ap);
    }
  }
  emReallyEndLogLine(logType);
}

void emReallyLogBytes(uint8_t logType,
                      bool lineStartAndEnd,
                      bool useHex,
                      uint8_t const *bytes,
                      uint16_t length,
                      PGM_P formatString,
                      ...)
{
  uint8_t port;
  if (lineStartAndEnd) {
    emReallyStartLogLine(logType);
  }
  for (port = 0; port < EM_LOG_PORT_COUNT; port++) {
    if (typeAndPortEnabled(logType, port)) {
      va_list ap;
      uint16_t i;
      va_start(ap, formatString);
      logPrintfVarArg(port, formatString, ap);
      va_end(ap);
      if (!useHex) {
        logPrintf(port, "'");
      }
      for (i = 0; i < length; i++) {
        logPrintf(port, useHex ? " %x" : "%c", bytes[i]);
      }
      if (!useHex) {
        logPrintf(port, "'");
      }
    }
  }
  if (lineStartAndEnd) {
    emReallyEndLogLine(logType);
  }
}

void emReallyLogCodePoint(uint8_t logType,
                          PGM_P file,
                          uint16_t line,
                          PGM_P formatString,
                          ...)
{
  uint8_t port;
  emReallyStartLogLine(logType);
  for (port = 0; port < EM_LOG_PORT_COUNT; port++) {
    if (typeAndPortEnabled(logType, port)) {
      logPrintf(port, "%s:%u ", file, line);
      if (formatString != NULL) {
        va_list ap;
        va_start(ap, formatString);
        logPrintfVarArg(port, formatString, ap);
        va_end(ap);
      }
    }
  }
  emReallyEndLogLine(logType);
}

void emReallyLogIpAddress(uint8_t logType, const uint8_t *address)
{
  uint8_t i;
  for (i = 0; i < 8; i++) {
    uint16_t word = emberFetchHighLowInt16u(address + (i << 1));
    emReallyLog(logType, "%p%2x", (i == 0) ? "" : ":", word);
  }
}

#ifndef EMBER_SCRIPTED_TEST

static void printLogConfig(uint8_t port)
{
  uint32_t off = logEnabledMask;
  uint8_t p;
  for (p = 0; p < EM_LOG_PORT_COUNT; p++) {
    if (portMasks[p] != 0) {
      off &= ~portMasks[p];
      logPrintf(port, "port %u: ", p);
      uint8_t t;
      for (t = 0; t < COUNTOF(emLogTypeNames); t++) {
        if (typeAndPortEnabled(t, p)) {
          logPrintf(port, "%s, ", emLogTypeNames[t]);
        }
      }
      logPrintf(port, "\r\n");
    }
  }
  if (off != 0) {
    logPrintf(port, "off: ");
    uint8_t t;
    for (t = 0; t < COUNTOF(emLogTypeNames); t++) {
      if (off & BIT32(t)) {
        logPrintf(port, "%s, ", emLogTypeNames[t]);
      }
    }
    logPrintf(port, "\r\n");
  }
}

bool emLogConfigFromName(const char *typeName,
                         uint8_t typeNameLength,
                         bool on,
                         uint8_t port)
{
  // TODO: fixme
  uint8_t type = emLogTypeFromName((uint8_t *)typeName, typeNameLength);

  if (type == 0xFF) {
    logPrintf(port,
              "Error: %s is not a valid log type name\r\n",
              typeName);
    return false;
  } else if (!(logEnabledMask & BIT32(type))) {
    logPrintf(port,
              "Error: %s log is not enabled\r\n",
              typeName);
    return false;
  } else {
    emLogConfig(type, port, on);
    logPrintf(port,
              "%s log port %u: %s\r\n",
              typeName,
              port,
              on ? "on" : "off");
  }

  return true;
}

void emLogCommand(void)
{
  bool on;
  uint8_t i, length;
  uint8_t count = emberCommandArgumentCount();
  uint8_t port = emberUnsignedCommandArgument(0);
  emberStringCommandArgument(-1, &length);
  if (length > 7) {
    printLogConfig(port);
    return;
  }
  on = (length < 4);  // The commands are 'log' and 'log_off'.
  bool all = (emStrcmp(emberStringCommandArgument(1, NULL),
                       (uint8_t *)"all")
              == 0);

  if (!on
      && count == 2
      && all) {
    portMasks[port] = 0;
  } else if (on && all) {
    portMasks[port] = 0xFFFFFFFF;
  } else {
    for (i = 1; i < count; i++) {
      uint8_t typeNameLength;
      uint8_t *typeName = emberStringCommandArgument(i, &typeNameLength);
      emLogConfigFromName((const char *)typeName, typeNameLength, on, port);
    }
  }
}

#endif
