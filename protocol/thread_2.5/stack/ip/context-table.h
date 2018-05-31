/*
 * File: context-table.h
 * Description: Table of prefixes for use in 6lowpan context compression.
 * Author(s): Matteo Paris, matteo@ember.com
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef CONTEXT_TABLE_H
#define CONTEXT_TABLE_H

// Contexts are stored in the over-the-air format used for network data.
// See network-data.h for a description.

#define PREFIX_6CO_ID_MASK       0x0F
#define PREFIX_6CO_COMPRESS_FLAG 0x10

#define MAX_6LOWPAN_HC_CONTEXT_ID 15

// Copies the context's prefix into 'address', returning the
// number of bits copied.
uint8_t emUncompressContextPrefix(uint8_t context, uint8_t *address);

// Returns the number of bits that matched a context's prefix,
// and the ID of the context.
uint8_t emCompressContextPrefix(const uint8_t *address,
                                uint8_t matchLength,
                                uint8_t *context);

// This is intended for debugging only.  The prefix pointer will
// become invalid the next time the network data changes, so it should
// be used immediately.
typedef struct {
  uint8_t contextId;
  uint8_t flags;
  uint8_t *prefix;
  uint8_t prefixLengthInBits;
  uint16_t lifetime;
} LowpanContext;

bool emFindContext(uint8_t contextId, LowpanContext *result);

#endif // CONTEXT_TABLE_H
