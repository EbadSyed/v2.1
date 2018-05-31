/*
 * File: buffer-malloc.c
 * Description: malloc() and free() implemented on top of buffers.
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

// An implementation of malloc() and free() that uses buffers.  malloc()
// returns a pointer to the contents of a buffer.  Freed buffers are kept
// on a list and reused; a new buffer is allocated only if no buffer on
// the freelist is large enough to be used.
//
// The buffers are not marked by this code.  emMallocFreeList must be
// set to NULL_BUFFER before calling emReclaimUnusedBuffers().

#include "core/ember-stack.h"
#include "buffer-malloc.h"

// Keep statistics.
uint32_t emMallocCount = 0;
uint32_t emMallocSize = 0;

#define getFreeLink(buffer)      (emGetBufferLink((buffer), 0))
#define setFreeLink(buffer, new) (emSetBufferLink((buffer), 0, (new)))

#ifdef EMBER_TEST
static UNUSED void printFreeList(char *tag)
{
  Buffer buffer = emMallocFreeList;
  fprintf(stderr, "[%s:", tag);
  for (; buffer != NULL_BUFFER; buffer = getFreeLink(buffer)) {
    fprintf(stderr, " %04X", buffer);
  }
  fprintf(stderr, "]\n");
}

static UNUSED void printFreeSizes(char *tag)
{
  Buffer buffer = emMallocFreeList;
  fprintf(stderr, "[%s:", tag);
  for (; buffer != NULL_BUFFER; buffer = getFreeLink(buffer)) {
    fprintf(stderr, " %d", emGetBufferLength(buffer));
  }
  fprintf(stderr, "]\n");
}
#endif

// Make 'front' precede 'back', merging them if they are adjacent.
// Either or both may be NULL_BUFFER.

static void connect(Buffer front, Buffer back)
{
  if (front == NULL_BUFFER) {
    emMallocFreeList = back;
  } else if (emFollowingBuffer(front) == back) {
    setFreeLink(front, getFreeLink(back));
    emMergeBuffers(front, back);
  } else {
    setFreeLink(front, back);
  }
}

void emBufferFree(void *pointer)
{
  Buffer buffer = emBufferPointerToBuffer(pointer);
  if (buffer != NULL_BUFFER) {
    Buffer next = emMallocFreeList;
    Buffer previous = NULL_BUFFER;

    for (;
         next != NULL_BUFFER && next < buffer;
         previous = next, next = getFreeLink(next)) {
      ;
    }

    connect(buffer, next);
    connect(previous, buffer);
  }
}

// This uses 'best fit': the smallest free buffer that is 'size' or
// larger is returned.  Any extra is split off into a separate buffer
// that is left on the free list.

void *emBufferMalloc(uint16_t size)
{
  Buffer next = emMallocFreeList;
  Buffer previous = NULL_BUFFER;
  Buffer result = NULL_BUFFER;
  Buffer resultPrevious = NULL_BUFFER;

  size += size & 1;             // round up to a word boundary

  for (;
       next != NULL_BUFFER;
       previous = next, next = getFreeLink(next)) {
    uint16_t nextSize = emGetBufferLength(next);

    if (size <= nextSize
        && (result == NULL_BUFFER
            || nextSize < emGetBufferLength(result))) {
      result = next;
      resultPrevious = previous;
      if (size == nextSize) {
        break;
      }
    }
  }

  if (result == NULL_BUFFER) {
    result = emAllocateBuffer(size);
    if (result == NULL_BUFFER) {
      return NULL;
    }
    emMallocCount += 1;
    emMallocSize += size;
  } else {
    Buffer leftover = emSplitBuffer(result, size);
    Buffer resultNext = getFreeLink(result);
    if (leftover == NULL_BUFFER) {
      connect(resultPrevious, resultNext);
    } else {
      connect(leftover, resultNext);
      connect(resultPrevious, leftover);
    }
    connect(result, NULL_BUFFER);
  }

  return emGetBufferPointer(result);
}
