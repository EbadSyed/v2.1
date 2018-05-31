// File: ip-driver-log.c
//
// Description: logging IP driver events and I/O
//
// Copyright 2013 by Silicon Laboratories. All rights reserved.             *80*

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include PLATFORM_HEADER
#include "stack/core/ember-stack.h"
#include "app/ip-ncp/uart-link-protocol.h"
#include "ip-driver-log.h"

extern bool logEnabled;

static FILE *logFile = NULL;

static const char * const logEventNames[] = {
  "NCP->driver->IP stack ",
  "IP stack->driver->NCP ",
  "app->driver->NCP ",
  "NCP->driver mgmt ",
  "driver->app ",
  "driver->comm-proxy-app",
  "driver->NCP data ",
  "driver->NCP mgmt ",
  "rx error",
  "init event",
  "rx event",
  "signal ",
};

static const char * const messageTypeNames[] = {
  "?",
  "MGMT",
  "DATA",
  "UNS_DATA",
  "ALT_DATA",
  "COMMISSIONER_DATA"
};

static uint32_t lastLogEventTime;

static uint32_t unixTimeToMilliseconds(struct timeval *tv)
{
  return ((tv->tv_sec * 1000) + (tv->tv_usec / 1000));
}

void ipDriverOpenLogFile(const char *filename)
{
  if (logEnabled) {
    logFile = fopen(filename, "w");
    if (logFile == NULL) {
      perror("failed to open log file");
      exit(1);
    }
  }
}

static void logTimestamp(void)
{
  struct timeval tv;
  struct tm *tm;
  char datetime[64];
  gettimeofday(&tv, NULL);
  tm = localtime(&tv.tv_sec);
  assert(tm != NULL);
  strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", tm);
  uint32_t now = unixTimeToMilliseconds(&tv);
  uint32_t delta = elapsedTimeInt32u(lastLogEventTime, now);
  lastLogEventTime = now;

  if (logFile == NULL) {
    logFile = stderr;
  }

  if (delta == 0) {
    fprintf(logFile, "                                    ");
  } else {
    if (delta > 1000) {
      fprintf(logFile, "\n");
    }
    fprintf(logFile, "[%s.%03d +%4d.%03d] ",
            datetime, (int)(tv.tv_usec / 1000), delta / 1000, delta % 1000);
  }
}

void ipDriverLogEvent(LogEvent type,
                      const uint8_t *data,
                      uint16_t length,
                      SerialLinkMessageType messageType)
{
  if (logEnabled) {
    logTimestamp();
    fprintf(logFile, "[%s] [%s] [",
            logEventNames[type],
            messageTypeNames[messageType]);
    uint16_t i;
    for (i = 0; i < length; i++) {
      fprintf(logFile, "%02X", data[i]);
    }
    fprintf(logFile, "]\n");
    ipDriverLogFlush();
  }
}

void ipDriverLogStatus(char *format, ...)
{
  if (logEnabled) {
    logTimestamp();
    fprintf(logFile, "[status] [");
    va_list ap;
    va_start(ap, format);
    vfprintf(logFile, format, ap);
    va_end(ap);
    fprintf(logFile, "]\n");

    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);
    fprintf(stdout, "\n");
    fflush(stdout);
  }
}

void ipDriverLogFlush(void)
{
  if (logEnabled) {
    fflush(logFile);
  }
}
