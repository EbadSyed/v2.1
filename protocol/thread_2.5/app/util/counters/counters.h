/**
 * @file: counters.h
 *
 * A library to tally up Ember stack counter events.
 *
 * The Ember stack tracks a number of events defined by ::EmberCountersType
 * and reports them to the application via the ::emberCounterHandler() callback.
 * This library keeps a tally of the number of times each type of a
 * counter event occurs.  The application must define
 * ::EMBER_APPLICATION_HAS_COUNTER_HANDLER in its CONFIGURATION_HEADER
 * to use this library.
 *
 * Copyright 2007 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef __EMBER_COUNTERS_H__
#define __EMBER_COUNTERS_H__

/**
 * The ith entry in this array is the count of events of EmberCounterType i.
 */
extern uint16_t emberCounters[EMBER_COUNTER_TYPE_COUNT];

#endif
