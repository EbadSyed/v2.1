/*
 * File: fragment.h
 * Description: 6LowPAN header fragmentation and reassembly.
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef FRAGMENT_H
#define FRAGMENT_H

// The first fragment is one byte shorter over the air because it doesn't
// include the offset.  Internally we always use the larger size.
#define FIRST_FRAGMENT_HEADER_LENGTH 4
#define FRAGMENT_HEADER_LENGTH 5

#define FRAGMENT_HEADER_TAG_INDEX       2
#define FRAGMENT_HEADER_OFFSET_INDEX    4

// The number of payload bytes in transmitted fragments.  The length of the
// payload in non-final fragments must be a multiple of eight.
#define MAX_FRAGMENT          80
#define MAX_FINAL_FRAGMENT    80
// The maximum time we will wait for the next fragment.
#define FRAGMENT_TIMEOUT_MS 2000

enum {
  FRAGMENT_FAILED = 0xFF,
  FRAGMENT_NEEDED = 0xFE
};

// For flexibility, the actual number is kept in a variable.
extern uint16_t emFragmentTimeoutMs;

bool emIsFragment(PacketHeader header);

PacketHeader emProcessFragment(PacketHeader fragment);

uint8_t emMakeNextFragment(PacketHeader packet,
                           uint8_t *txBuffer,
                           uint8_t maxLength,
                           uint8_t *destinationLoc,
                           bool okayToFragment);

bool emMoreFragments(PacketHeader packet);

bool emAdvanceFragmentPointer(PacketHeader packet);

void emResetFragmentPointer(PacketHeader packet);

void emMarkFragmentBuffers(void);

#endif // FRAGMENT_H
