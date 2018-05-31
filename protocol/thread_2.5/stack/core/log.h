/*
 * File: log.h
 * Internal logging system for use in debugging.
 *
 * Copyright 2011 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef __EMBER_LOG_H__
#define __EMBER_LOG_H__

enum {
  EM_LOG_PORT_VIRTUAL_UART = 0,
  EM_LOG_PORT_UART = 1,
  EM_LOG_PORT_STDERR = 2,
  EM_LOG_PORT_COUNT = 3
};

#define EM_LOG_PORT_DEFAULT EM_LOG_PORT_UART

// Select the log types that will be enabled on hardware. Logging for types not
// listed here will be stripped when the stack is compiled. These definitions
// must be made before including log-gen.h.

// note: define EMBER_ENABLE_LOGGING to enable logging. Debugging also must be
// enabled, i.e (DEBUG_LEVEL >= BASIC_DEBUG)

#ifndef BASIC_DEBUG
  #error BASIC_DEBUG not defined: ember-debug.h must be included first.
#endif

#if defined(EMBER_ENABLE_LOGGING) && (DEBUG_LEVEL >= BASIC_DEBUG)
  #define EM_LOG_PANA_ENABLED true
  #define EM_LOG_MLE_ENABLED true
  #define EM_LOG_NETWORK_DATA_ENABLED true
  #define EM_LOG_DROP_ENABLED true
  #define EM_LOG_JOIN_ENABLED true
  #define EM_LOG_SECURITY_ENABLED true
  #define EM_LOG_SECURITY2_ENABLED true
  #define EM_LOG_REST_ENABLED true
  #define EM_LOG_EXI_ENABLED true
  #define EM_LOG_HTTP_ENABLED true
  #define EM_LOG_MDNS_ENABLED true
  #define EM_LOG_RIP_ENABLED true
  #define EM_LOG_WAKEUP_ENABLED true
  #define EM_LOG_BOOTLOAD_ENABLED true
  #define EM_LOG_TOPOLOGY_ENABLED true
  #define EM_LOG_SERIAL_ENABLED true
  #define EM_LOG_POLL_ENABLED true
  #define EM_LOG_IP_MODEM_ENABLED true
  #define EM_LOG_COAP_ENABLED true
  #define EM_LOG_LOWPAN_ENABLED true
  #define EM_LOG_DISPATCH_ENABLED true
  #define EM_LOG_COMMISSION_ENABLED true
  #define EM_LOG_DHCP_ENABLED true
#endif

#include "log-gen.h"

//------------------------------------------------------------------------------
// Public interface for logging messages

#define emLog(type, ...)                         \
  do {                                           \
    if (EM_LOG_ ## type ## _MASK) {              \
      emReallyLog(EM_LOG_ ## type, __VA_ARGS__); \
    } } while (0)

#define emStartLogLine(type)                 \
  do {                                       \
    if (EM_LOG_ ## type ## _MASK) {          \
      emReallyStartLogLine(EM_LOG_ ## type); \
    } } while (0)

#define emEndLogLine(type)                 \
  do {                                     \
    if (EM_LOG_ ## type ## _MASK) {        \
      emReallyEndLogLine(EM_LOG_ ## type); \
    } } while (0)

#define emLogLine(type, ...)                         \
  do {                                               \
    if (EM_LOG_ ## type ## _MASK) {                  \
      emReallyLogLine(EM_LOG_ ## type, __VA_ARGS__); \
    } } while (0)

#define emLogBytes(type, format, bytes, count, ...)           \
  do {                                                        \
    if (EM_LOG_ ## type ## _MASK) {                           \
      emReallyLogBytes(EM_LOG_ ## type, false, true, (bytes), \
                       (count), (format), ##__VA_ARGS__);     \
    } } while (0)

#define emLogBytesLine(type, format, bytes, count, ...)      \
  do {                                                       \
    if (EM_LOG_ ## type ## _MASK) {                          \
      emReallyLogBytes(EM_LOG_ ## type, true, true, (bytes), \
                       (count), (format), ##__VA_ARGS__);    \
    } } while (0)

#define emLogChars(type, format, bytes, count, ...)            \
  do {                                                         \
    if (EM_LOG_ ## type ## _MASK) {                            \
      emReallyLogBytes(EM_LOG_ ## type, false, false, (bytes), \
                       (count), (format), ##__VA_ARGS__);      \
    } } while (0)

#define emLogCharsLine(type, format, bytes, count, ...)       \
  do {                                                        \
    if (EM_LOG_ ## type ## _MASK) {                           \
      emReallyLogBytes(EM_LOG_ ## type, true, false, (bytes), \
                       (count), (format), ##__VA_ARGS__);     \
    } } while (0)

#define emLogCodePoint(type, ...)                                             \
  do {                                                                        \
    if (EM_LOG_ ## type ## _MASK) {                                           \
      emReallyLogCodePoint(EM_LOG_ ## type, __FILE__, __LINE__, __VA_ARGS__); \
    } } while (0)

#define emLogIpAddress(type, address)                   \
  do {                                                  \
    if (EM_LOG_ ## type ## _MASK) {                     \
      emReallyLogIpAddress(EM_LOG_ ## type, (address)); \
    } } while (0)

#define emLogNetworkData(type)                 \
  do {                                         \
    if (EM_LOG_ ## type ## _MASK) {            \
      emReallyLogNetworkData(EM_LOG_ ## type); \
    } } while (0)

#define emLogNetworkDataTlvs(type, title, tlvs, end)                 \
  do {                                                               \
    if (EM_LOG_ ## type ## _MASK) {                                  \
      emReallyLogNetworkDataTlvs(EM_LOG_ ## type, title, tlvs, end); \
    } } while (0)

#define emLogIsActive(type)   \
  ((EM_LOG_ ## type ## _MASK) \
   ? emReallyLogIsActive(EM_LOG_ ## type) : false)

// Convenience macro for logging drops without a message string.
#define emLogDrop() emLogCodePoint(DROP, NULL);

//-----------------------------------------------------------------------------
// The actual internal log functions

void emReallyLog(uint8_t logType, PGM_P formatString, ...);
void emReallyStartLogLine(uint8_t logType);
void emReallyEndLogLine(uint8_t logType);
void emReallyLogLine(uint8_t logType, PGM_P formatString, ...);
void emReallyLogBytes(uint8_t logType,
                      bool lineStartAndEnd,
                      bool useHex,
                      uint8_t const *bytes,
                      uint16_t length,
                      PGM_P formatString,
                      ...);
void emReallyLogCodePoint(uint8_t logType,
                          PGM_P file,
                          uint16_t line,
                          PGM_P formatString,
                          ...);
void emReallyLogIpAddress(uint8_t logType, const uint8_t *address);
void emReallyLogNetworkData(uint8_t logType);
void emReallyLogNetworkDataTlvs(uint8_t logType,
                                const char *title,
                                uint8_t *tlvs,
                                const uint8_t *end);

//------------------------------------------------------------------------------
// Configuration funtions

// Enable or disable logging for an individual type and port.
void emLogConfig(uint8_t logType, uint8_t port, bool on);

// Returns true if the given type is enabled on any port.
bool emReallyLogIsActive(uint8_t logType);

uint8_t emLogTypeFromName(uint8_t *name, uint8_t nameLength);

bool emLogConfigFromName(const char *typeName,
                         uint8_t typeNameLength,
                         bool on,
                         uint8_t port);

#endif
