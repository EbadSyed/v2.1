// File: child-data.h
//
// Description: ZigBee IP child table
//
// Author(s): Richard Kelsey <kelsey@ember.com>
//
// Copyright 2010 by Ember Corporation.  All rights reserved.               *80*

#ifndef CHILD_DATA_H_INCLUDED
#define CHILD_DATA_H_INCLUDED

#include "stack/include/ember-types.h"
#include "stack/include/network-management.h"

// Child Entry Node Data: 2 bytes
// 3 reserved bits
// 1 bit: CHILD_IS_PRESENT
// 1 bit: CHILD_IS_SLEEPY
// 1 bit: CHILD_SECURE_DATA_REQUESTS
// Low 10 bits of Child ID

// Flags against the top byte of node data, as defined above
#define CHILD_ENTRY_CHILD_IS_PRESENT           0x10
#define CHILD_ENTRY_CHILD_IS_SLEEPY            0x08
#define CHILD_ENTRY_CHILD_SECURE_DATA_REQUESTS 0x04

#define CHILD_ENTRY_NODE_DATA_OFFSET              0
#define CHILD_ENTRY_LONG_ID_OFFSET                2
#define CHILD_ENTRY_TIMEOUT_OFFSET               10
#define CHILD_ENTRY_SIZE 14

#define MIN_CHILD_TIMEOUT_SECONDS 30

// Moved to include/child.h to give the application access.
//extern uint8_t emMaxEndDeviceChildren;

// Various per-child timers are allocated by the application via
// ember-ip-configuration.c.
extern uint8_t emberChildTableSize;

// These should be combined into a struct.  The short ID may have to be
// separate because of the time requirements on emberChildIndex()
// (in phy/child-table.c).
extern EmberNodeId *emChildIdTable;     // short ID
extern uint8_t *emChildLongIdTable;       // long ID
extern ChildStatusFlags *emChildStatus; // status flags
extern uint32_t *emChildTimers;
extern uint32_t *emChildTimeouts;
extern uint32_t *emChildMacFrameCounters;
extern uint32_t *emChildMleFrameCounters;
extern int8_t   *emChildSequenceDeltas;
extern uint16_t *emChildLastTransactionTimesSec;
extern Buffer emChildAddressTable;

// The one-byte timer for each end-device child gives the
// elapsed time since the last poll.  The units depend on the current
// flag values.  If CHILD_EXPECTING_JIT is set, the time is in
// milliseconds and is used to clear CHILD_EXPECTING_JIT if we
// fail to send a message in time.  Otherwise, it is in seconds if
// CHILD_IS_MOBILE is set and minutes if not.

//----------------------------------------------------------------
// Child status information

// We have 16 bits bytes of per-child information in RAM.
#define CHILD_IS_PRESENT                 0x0001
#define CHILD_LINK_ESTABLISHED           0x0002
#define CHILD_IS_SLEEPY                  0x0004
#define CHILD_PENDING_MAC_SHORT_INDIRECT 0x0008
#define CHILD_PENDING_MAC_LONG_INDIRECT  0x0010
#define CHILD_USING_OLD_KEY              0x0020
#define CHILD_HAS_OLD_NETWORK_DATA       0x0040
#define CHILD_STABLE_DATA_ONLY           0x0080

#define CHILD_PENDING_APPLICATION_JIT    0x0100
#define CHILD_PENDING_SLEEPY_BROADCAST   0x0200
#define CHILD_EXPECTING_JIT              0x0400
#define CHILD_NEEDS_ACTIVE_DATASET       0x0800
#define CHILD_NEEDS_PENDING_DATASET      0x1000

#define CHILD_SECURE_DATA_REQUESTS       0x2000
#define CHILD_IS_FULL_THREAD_DEVICE      0x4000

// Pick out all of the flags that indicate that a MAC indirect message is
// pending.
#define CHILD_HAS_PENDING_MESSAGE    \
  (CHILD_PENDING_MAC_SHORT_INDIRECT  \
   | CHILD_PENDING_MAC_LONG_INDIRECT \
   | CHILD_PENDING_APPLICATION_JIT   \
   | CHILD_PENDING_SLEEPY_BROADCAST  \
   | CHILD_HAS_OLD_NETWORK_DATA)

#define emHaveChild(i)          (emChildStatusFlag((i), CHILD_IS_PRESENT))
#define emIsSleepyChildIndex(i) (emChildStatusFlag((i), CHILD_IS_SLEEPY))
#define emIsFullThreadDeviceChildIndex(i) \
  (emChildStatusFlag((i), CHILD_IS_FULL_THREAD_DEVICE))

#define emChildHasPendingMessage(childIndex) \
  ((emChildStatus[(childIndex)] & CHILD_HAS_PENDING_MESSAGE) != 0)

#define emChildStatusFlag(childIndex, mask) \
  (emChildStatus[(childIndex)] & (mask))

#define emSetChildStatusFlag(childIndex, mask) \
  (emChildStatus[(childIndex)] |= (mask))

#define emClearChildStatusFlag(childIndex, mask) \
  (emChildStatus[(childIndex)] &= ~(mask))

// Set or clear 'mask' bits for all sleepy children.
bool emSetAllSleepyChildFlags(ChildStatusFlags mask, bool set);

// True if any child as all of the given flags set.
bool emAnyChildHasFlagsSet(ChildStatusFlags mask);

// True if any sleepy child has this flag set.
#define emIsSleepyChildFlagSet(mask) \
  (emAnyChildHasFlagsSet((mask) | CHILD_IS_SLEEPY))

bool emChildHasEnhancedFramePendingMessage(uint8_t childIndex);

//----------------------------------------------------------------
// Adding and removing entries from the child table.

uint8_t emMaybeAddChild(EmberNodeId id,
                        const uint8_t *longId,
                        uint8_t mode,
                        uint32_t timeoutSeconds,
                        uint8_t *macFrameCounter,
                        uint8_t *mleFrameCounter);

bool emInitializeChild(uint8_t index,
                       EmberNodeId id,
                       const uint8_t *longId,
                       uint8_t mode,
                       uint32_t timeoutSeconds);

void emSetChildId(uint8_t childIndex, EmberNodeId newId);
void emResetChildTokens(void);
void emResetChildToken(uint8_t index);
void emEraseChild(uint8_t childIndex);
void emEraseChildTable(void);

void emNoteChildTransaction(uint8_t index);
uint16_t emChildSecondsSinceLastTransaction(uint8_t index);

Buffer emSearchChildAddressTable(uint8_t index, const EmberIpv6Address *address);

#define CHILD_CHANGE_LEAVING     0x00
#define CHILD_CHANGE_JOINING     0x01
#define CHILD_CHANGE_NO_CALLBACK 0x02   // do not call emberChildJoinHandler()

void emNoteChildChange(uint8_t childIndex, uint8_t options);

// Looking up a child.
EmberNodeId emFindChild(const uint8_t *longId);
uint8_t emFindChildIndex(uint8_t startIndex, const uint8_t *longId);
uint8_t emFindChildIndexByAddress(const EmberIpv6Address *address);
#define emFindFreeChildIndex(startIndex) (emFindChildIndex((startIndex), NULL))

uint8_t *emSetChildAddresses(uint8_t index,
                             const uint8_t* data,
                             uint8_t dataLength,
                             uint8_t *finger);

// Used by the routing code, which doesn't care about indexes.
bool emIsMobileChild(EmberNodeId id);
uint8_t *emLookupChildEui64ById(EmberNodeId id);
bool emChildIsFrameCounterValid(uint32_t frameCounter,
                                const uint8_t *eui64,
                                uint32_t sequenceNumber,
                                MessageKeyType keyType);
void emChildSetFrameCounter(uint8_t index,
                            uint32_t macFrameCounter,
                            uint32_t mleFrameCounter,
                            uint32_t keySequence);
void emChildResetFrameCounter(void);
void emUpdateChildSequenceDeltas(uint8_t skipForward);

bool emIsSleepyChild(EmberNodeId id);
bool emIsFullThreadDeviceChild(EmberNodeId id);
bool emIsEndDeviceChild(EmberNodeId id);

// Restart the timeout clock for end devices.
void emNoteSuccessfulPoll(void);

// Just what it says.
uint32_t emMsSinceLastSuccessfulPoll(void);

// Handy utility.
#define copyEui64(to, from)      MEMCOPY((to), (from), EUI64_SIZE)

void emChildInit(void);
void emRestoreChildTable(void);

// Reset the child's timeout.
void emResetChildsTimer(uint8_t index);

// start polling
void emStartPolling(void);

// get the poll timeout
uint32_t emGetPollTimeout(void);

uint8_t emChildCount(void);

#ifdef EMBER_TEST
EmberStatus emRemoveChild(const uint8_t *longId);
#endif

extern uint32_t emKeepaliveFlurryIntervalS;

#endif // include guard
