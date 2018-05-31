/*
 * File: multicast.h
 * Description: Multicast forwarding from draft-hui-6man-trickle-mcast.
 * Author(s): Matteo Paris
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef MULTICAST_H
#define MULTICAST_H

// Returns true if the packet has a hop-by-hop multicast option and an
// entry in the multicast table.  Also returns true if the packet should
// have a multicast option but doesn't, or if there is no space left
// in the multicast table for it.
bool emIsDuplicateMulticast(Ipv6Header *ipHeader);

void emStoreMulticastSequence(uint8_t *target);

void emMulticastTableInit(void);

#define MULTICAST_RETRY_COUNT 2

// "Transmission time t is randomly chosen within full Trickle interval,
// rather than half the Trickle interval as specified in [RFC 6206]."
// Trickle doesn't map well to our retry table so we fake it by adding I/2
// between each transmission.  This gives roughly the right distribution.
#define TRICKLE_MULTICAST_INTERVAL 6
extern uint8_t emTrickleMulticastInterval;

#define DWELL_TIME_QS 40

//------------------------------------------------------------------------------

// The windowBitmask records which of the previous 8 sequence numbers since
// lastSequence have been received; the low bit is the highest sequence number.
// Entries are deleted when dwellQs reaches 0.
typedef struct {
  uint8_t lastSequence;
  uint8_t windowBitmask;
  uint8_t dwellQs;
  uint8_t seedLength;
  uint8_t seed[0];
} MulticastTableEntry;

#define MULTICAST_TABLE_SIZE 8

// Exported so someone can trace it.
extern Buffer emMulticastTable;

MulticastTableEntry *emGetMulticastTableEntry(uint16_t i);
void emRestoreMulticastSequence(void);

#endif // MULTICAST_H
