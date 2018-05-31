/*
 * File: debug.h
 * Description: debug code for TLS
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef SLTHREAD_DEBUG_H
#define SLTHREAD_DEBUG_H

// emLoseHelper() is a debug hook so that a concerned programmer can set a
// breakpoint for lose() statements
#ifdef EMBER_TEST
// see stack/core/ip-stack.c for implementation
extern void emLoseHelper(void);
#else
  #define emLoseHelper()
#endif

#define lose(type, ...)              \
  do { emLogCodePoint(type, "lost"); \
       emLoseHelper();               \
       return __VA_ARGS__; } while (0)

#define loseVoid(type)               \
  do { emLogCodePoint(type, "lost"); \
       emLoseHelper();               \
       return; } while (0)

const char *tlsHandshakeName(uint8_t id);
const char *tlsStateName(uint8_t state);

#if defined(DEBUG)
  #define debugLose(...) emberDebugPrintf(__VA_ARGS__)
  #define debug(...) emberDebugPrintf(__VA_ARGS__)
  #define dump(name, data, length) \
  do { emberDebugPrintf(name);     \
       emberDebugBinaryPrintf("lp", length, data); } while (0)
extern int debugLevel;
#elif defined(EMBER_TEST)
  #define debugLose(...) debug(__VA_ARGS__)
void debug(char *format, ...);
void dump(char *name, const uint8_t *data, int length);
extern int debugLevel;
#else
  #define debugLose(...)
  #ifndef debug
    #define debug(...)
  #endif
  #define dump(name, data, length)
#endif

#endif // SLTHREAD_DEBUG_H
