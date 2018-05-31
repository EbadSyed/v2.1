/*
 * File: buffer-malloc.h
 * Description: malloc() and free() implemented on top of buffers.
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef BUFFER_MALLOC_H
#define BUFFER_MALLOC_H

// An implementation of malloc() and free() that uses buffers.  malloc()
// returns a pointer to the contents of a buffer.  Freed buffers are kept
// on a list and reused; a new buffer is allocated only if no buffer on
// the freelist is large enough to be used.
//
// The buffers are not marked by this code.  emMallocFreeList must be
// set to NULL_BUFFER before calling emReclaimUnusedBuffers().

extern Buffer emMallocFreeList;

void *emBufferMalloc(uint16_t size);

void emBufferFree(void *pointer);

#endif // BUFFER_MALLOC_H
