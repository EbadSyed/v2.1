/**
 * File: neighbor.h
 * Description: Maintain neighbor link state information.
 * Author(s): Matteo Neale Paris, matteo@ember.com
 *
 * Copyright 2001 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef NEIGHBOR_H
#define NEIGHBOR_H

//------------------------------------------------------------------------------
// MLE

uint8_t emNeighborWriteMleRecords(uint8_t *target, const uint8_t *ipDest);

void emSetIncomingMleState(uint8_t index, bool accept);

bool emLinkIsEstablished(uint8_t index);

//------------------------------------------------------------------------------
// Utilities

// Updates the rolling average link quality for the neighbor.
void emNoteLinkQuality(uint8_t neighborIndex);

uint8_t *emNeighborLookupLongId(EmberNodeId shortId);

// Returns the index of the neighbor entry in the neighbor table,
// or 0xFF if not found.
uint8_t emFindNeighborIndexByShortId(EmberNodeId neighbor);

// Returns the short id of a neighbor with the given long id,
// or NULL_NODE_ID if it is not found.
EmberNodeId emFindNeighborByLongId(const uint8_t *longId);

// Returns the index of a neighbor with the given long id,
// or 0xFF if it is not found.
uint8_t emFindNeighborIndexByLongId(const uint8_t *longId);

// Returns the index of the neighbor that has this ipAddress, or 0xFF
// if there isn't one.
uint8_t emFindNeighborIndexByIpAddress(const uint8_t *ipAddress);

// Finds the given neighbor in the table, or adds it if it is not there
// and there are empty entries, or stale entries that are not in use.
// Returns the index, or 0xFF if the entry was not added.
uint8_t emFindOrAddNeighbor(EmberNodeId id);

// Finds neighbor index for header with long or short mac source.
uint8_t emFindNeighborIndex(PacketHeader header);

// Used to add neighbors to the lurker portion of the neighbor table.
// Only used in the RIP+wakeup stack.
uint8_t emAddLurkerNeighbor(const uint8_t *longId);

// Used during association to kickstart routing communication between
// a router child and its parent.
void emAssumeSymmetricLink(uint8_t neighborIndex);

// Deletes the entry at the given index and creates a new entry for
// the given neighbor id.  Returns the index of the new neighbor entry.
uint8_t emReplaceNeighbor(uint8_t index, EmberNodeId id);

// Added for MLE.
uint8_t emAddNeighbor(EmberNodeId shortId, const uint8_t *longId);

bool emNeighborMakeRoom(void);

uint8_t emNeighborSetShortId(uint8_t index, EmberNodeId id);

bool emNeighborHaveShortId(uint8_t index);

uint16_t emNeighborShortId(uint8_t index);

void emNeighborSetCapabilities(uint8_t index, uint8_t capabilities);

void emRemoveNeighbor(uint8_t index);

// Remove any entries matching the longId and NOT matching avoidShortId.
// This is for removing entries whose short id has changed.  It can
// also be used to remove entries by longId simply by passing NULL_NODE_ID
// for the avoidShortid.
void emRemoveNeighborLongId(const uint8_t *longId, EmberNodeId avoidShortId);

// Fill in the long id given a new long/short pair.
// This is called *after* conflicts have been detected
// so it assumes there are no conflicts.
void emUpdateNeighborTable(const uint8_t *longId, EmberNodeId shortId);

void emSetNeighborLongId(uint8_t index, const uint8_t *longId);

void emNeighborInit(void);

bool emShortIdInUse(uint16_t id);

bool emInUseAsNextHop(uint16_t id);

bool emNeighborInterfaceId(EmberNodeId id, uint8_t *target);

void emNeighborSetFrameCounter(uint8_t index,
                               uint32_t macFrameCounter,
                               uint32_t mleFrameCounter,
                               uint32_t keySequence,
                               bool isLegacy);

// It checks whether a frame counter for a particular neighbor, identified
// by its EUI64 is a valid frame counter. If the frame counter is a valid one,
// the frameCounter field in the neighbor entry is updated.
bool emNeighborIsFrameCounterValid(uint32_t frameCounter,
                                   const uint8_t *longId,
                                   uint32_t keySequence,
                                   MessageKeyType keyType);

void emNeighborResetFrameCounter(void);
void emUpdateNeighborSequenceDeltas(uint8_t skipForward);

void emPrintNeighborTable(uint8_t port);

void emSyncNeighbor(const uint8_t *destination);

#endif // NEIGHBOR_H
