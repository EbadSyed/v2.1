/*
 * File: psk-table.c
 * Description: PSK table utilities
 *
 * Copyright 2017 by Silicon Labs. All rights reserved.                *80*
 */

#include PLATFORM_HEADER
#include "stack/core/ember-stack.h"
#include "stack/ip/tls/psk-table.h"

Buffer emPskTable = NULL_BUFFER;

static const PskEntry *getPskEntry(const uint8_t *address)
{
  Buffer that;

  for (that = emBufferQueueHead(&emPskTable);
       that != NULL_BUFFER;
       that = emBufferQueueNext(&emPskTable, that)) {
    PskEntry *entry = (PskEntry *)(void *)emGetBufferPointer(that);

    if (entry != NULL) {
      if (address != NULL
          && MEMCOMPARE(address, entry->address, 16) == 0) {
        return entry;
      }
    }
  }

  return NULL;
}

uint8_t emGetPsk(uint8_t *psk, const uint8_t *address)
{
  const PskEntry *entry = getPskEntry(address);
  if (entry != NULL) {
    if (entry->pskLength != 0) {
      MEMCOPY(psk, entry->psk, entry->pskLength);
      return entry->pskLength;
    }
  }
  return 0;
}

EmberStatus emAddPsk(const uint8_t *psk,
                     uint8_t pskLength,
                     const uint8_t *address)
{
  if (emBufferQueueLength(&emPskTable) > MAX_PSK_TABLE_SIZE) {
    return EMBER_TABLE_FULL;
  }
  Buffer pskBuffer = emAllocateBuffer(sizeof(PskEntry));
  MEMSET(emGetBufferPointer(pskBuffer), 0, sizeof(PskEntry));
  PskEntry *entry = (PskEntry *)(void *)emGetBufferPointer(pskBuffer);
  if (entry != NULL) {
    MEMCOPY(entry->psk, psk, pskLength);
    entry->pskLength = pskLength;
    MEMCOPY(entry->address, address, 16);
    emBufferQueueAdd(&emPskTable, pskBuffer);
    return EMBER_SUCCESS;
  } else {
    return EMBER_ERR_FATAL;
  }
}
