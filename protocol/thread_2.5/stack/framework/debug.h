/*
 * File: debug.h
 * Development Kit debugging utilities
 *
 * Author(s): James Parziale, jim@ember.com
 *
 * Copyright 2003 by Ember Corporation. All rights reserved.                *80*
 *
 * Debug Protocol Description:
 *  Binary debug message format:
 *   Byte  Description
 *   ----- ---------------------------------------------------------------------
 *    0    initial framing: '['
 *    1    length of data, doesn't count itself, framing, msg type, timestamp
 *    2    timestamp low byte
 *    3    timestamp high byte
 *    4    Debug Message Type
 *    ...  Data dependant on the Message Type
 *    Nth byte, end framing: ']'
 *
 *  If a debug packet as described above is "n" bytes long,
 *   the length byte will be equal to n-6.
 *
 *  Message Types and format:
 *  DebugPrintf
 *    ... ascii debug string
 *
 *  API Trace
 *    Byte 5, API subtype
 *    ... concatenated API parameters, the parser of the protocol needs
 *        to decipher based on API subtype; pointer parameters are not
 *        included in message.
 *
 *  Time sync
 *    No data, length is zero.  All data needed is in timestamp.
 *
 *  Assert
 *    Byte 5, line number low byte
 *    Byte 6, line number high byte
 *    ... ascii filename
 *
 *  Core Dump
 *    Byte 5, address low byte
 *    Byte 6, address high byte
 *    ... memory dump from given address
 *        (Note: if 5th and 6th bytes are 0xFFFF then data is processor state)
 *
 *  Reset & init info
 *    Byte 5, reset type
 *    Byte 6, reserved
 *    Bytes 7-14, EUI64
 *    Byte 15, stack configuration
 *    Byte 16, phy type
 *    Byte 17, stack version (high nibble major, low nibble minor)
 *    Byte 18, build number
 *    Byte 19, Perforce change number low byte
 *    Byte 20, Perforce change number high byte
 *    Byte 21, initial channel number
 *    Byte 22, initial radio power
 *
 *  Packet Trace
 *    Byte 5, packet trace subtype
 *    ... format depends on subtype.  Details below.
 *
 *  Error
 *    Byte 5: error code.
 */
#ifndef __DEBUG_H__
#define __DEBUG_H__
//******************************************************************************

// Include generated debug message types
#include "debug-message-type-gen.h"

// Include generate apitrace calls.
// This include includes the enum for the api trace types
// as well as macros for all apitrace calls.
#include "debug-gen.h"

// Subtypes for use with EM_DEBUG_STATS type.
enum {
  EM_STATS_MAC_CSMA_FAILURE,
  EM_STATS_MAC_SEND_SUCCESS,
  EM_STATS_MAC_SEND_FAILURE,
  EM_STATS_MAC_RECEIVE,        // in mac-arbiter.c:
  EM_STATS_DROP_DEBUG_MESSAGE, // in debug.c
  EM_STATS_OUT_OF_BUFFERS,     // in packet-buffer.c
  EM_STATS_ARRAY_SIZE          // this entry must be last
};

//******************************************************************************

#if DEBUG_LEVEL >= BASIC_DEBUG
void emberDebugHandler(EmberMessageBuffer message);
void emDebugProcessIncoming(uint16_t debugType, uint8_t *data, uint8_t length);
void emDebugResetInfo(void);
void emDebugBinaryFormat(uint16_t debugType, PGM_P formatString, ...);
void emDebugReceiveTick(void);
void emDebugTime(const uint8_t *name, bool start);
  #define emDebugStartTime(name) emDebugTime(name, true)
  #define emDebugEndTime(name) emDebugTime(name, false)
#else   // DEBUG_LEVEL >= BASIC_DEBUG
  #define emDebugProcessIncoming(debugType, message) do {} while (false)
  #define emDebugResetInfo() do {} while (false)
  #define emDebugBinaryFormat(debugType, formatString, ...) do {} while (false)
  #define emDebugReceiveTick() do {} while (false)
  #define emDebugStartTime(name) do {} while (false)
  #define emDebugEndTime(name) do {} while (false)
#endif  // DEBUG_LEVEL >= BASIC_DEBUG

#if DEBUG_LEVEL == FULL_DEBUG
void emDebugClearStats(void);
void emDebugPrintStats(void);
void emDebugStats(uint8_t index);
  #define IF_DEBUG(foo) do { foo; } while (false)
  #define EXEC_AND_ASSERT_IF_DEBUG(foo) assert(foo)
#else   // DEBUG_LEVEL == FULL_DEBUG
  #define emDebugClearStats() do {} while (false)
  #define emDebugPrintStats() do {} while (false)
  #define emDebugStats(index) do {} while (false)
  #define IF_DEBUG(foo) do {} while (false)
  #define EXEC_AND_ASSERT_IF_DEBUG(foo) do { foo; } while (false)
#endif  // DEBUG_LEVEL == FULL_DEBUG

#endif //__DEBUG_H__
