// File: counters.c
//
// Description: implements emberCounterHandler() and keeps a tally
// of the events reported by the stack.  The application must define
// EMBER_APPLICATION_HAS_COUNTER_HANDLER in its configuration header
// to use this file.
//
// Author(s): Matteo Paris, matteo@ember.com
//
// Copyright 2012 by Ember Corporation. All rights reserved.                *80*

#include PLATFORM_HEADER
#include "stack/core/ember-stack.h"

uint16_t emberCounters[EMBER_COUNTER_TYPE_COUNT] = { 0 };

// Implement the stack callback by simply tallying up the counts.
void emberCounterHandler(EmberCounterType type, uint16_t increment)
{
  if (type < EMBER_COUNTER_TYPE_COUNT) {
    uint32_t value = emberCounters[type];
    value += increment;
#ifndef EMBER_COUNTER_WRAP_AROUND
    if (value > 0xFFFF) {
      value = 0xFFFF;
    }
#endif
    emberCounters[type] = (uint16_t) value;
  }
}

uint16_t emberCounterValueHandler(EmberCounterType type)
{
  // it's a bug to call this with an invalid type
  assert(type < EMBER_COUNTER_TYPE_COUNT);
  return emberCounters[type];
}

#if (!defined(UNIX_HOST) && !defined(UNIX_HOST_SIM) && !defined(RTOS))
void emberGetCounter(EmberCounterType type)
{
  if (type == EMBER_COUNTER_ALL) {
    int i;
    for (i = 0; i < EMBER_COUNTER_TYPE_COUNT; i++) {
      emApiGetCounterReturn(i, emberCounters[i]);
    }
  } else {
    emberGetCounterReturn(type, emberCounters[type]);
  }
}

void emberClearCounters(void)
{
  MEMSET(emberCounters, 0, sizeof(emberCounters));
}
#endif
