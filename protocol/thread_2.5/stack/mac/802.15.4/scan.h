// File: scan.h
// Description: Scanning for beacons.
//
// Culprit(s): DC Seward <dc@ember.com>
//
// Copyright 2004 by Ember Corporation. All rights reserved.                *80*

#ifndef SCAN_H
#define SCAN_H

extern uint8_t emMacScanType;

void emScanTimerIsr(void);

#define SCAN_PENDING() (emMacScanType != EM_SCAN_IDLE)
#define SYMBOLS_PER_SUPERFRAME      960
#define SYMBOLS_PER_ENERGY_READING    8 // we take a reading every 8 symbols
#define KICKSTART_DWELL_DURATION      0

void emScanReturn(EmberStatus status, uint8_t scanType, uint32_t txFailureMask);

void emEnergyScanHandler(uint8_t channel, int8_t maxRssiValue);

// Internal version of emberStopScan() that does not send a debug trace.
void emStopScan(void);
void emReallyStopScan(void);
void emStartScan(void);

// Returns the current RSSI value now on the current channel, or
// EMBER_PHY_INVALID_RSSI if the radio is not listening or receiving.
int8_t emGetEnergyDetection(void);

#endif
