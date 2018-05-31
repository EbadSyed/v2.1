/*
 * File: buffer-queue.c
 * Description: queues of buffers
 *
 * Copyright 2014 Silicon Laboratories, Inc.                                *80*
 */

#include "core/ember-stack.h"
#include "hal/hal.h"

// Queues of buffers.

// A queue is an index into the root set, as all queues must
// be handles.  Queues are linked in a circle through link0, with the
// handle pointing to the end of the queue and the tail pointing at
// the head, the head pointing at the next element and so on back
// around to the tail.

#if 0
static void printQueue(Buffer queue, int i)
{
  if (queue == NULL_BUFFER) {
    fprintf(stderr, "[]");
  } else {
    Buffer head = emGetBufferLink(queue, i);
    fprintf(stderr, "[%04X", head);
    queue = emGetBufferLink(head, i);
    while (queue != head) {
      fprintf(stderr, " %04X", queue);
      queue = emGetBufferLink(queue, i);
    }
    fprintf(stderr, "]");
  }
}
#define QDEBUG(x) x
#else
#define QDEBUG(x)
#endif

bool emBufferQueueIsEmpty(Buffer *queue)
{
  return *queue == NULL_BUFFER;
}

uint16_t emBufferQueueLength(Buffer *queue)
{
  uint16_t length = 0;
  if (queue == NULL) {
    return 0;
  }
  Buffer finger = emBufferQueueHead(queue);
  while (finger != NULL_BUFFER) {
    length++;
    finger = emBufferQueueNext(queue, finger);
  }
  return length;
}

Buffer emGenericQueueHead(Buffer *queue, uint16_t i)
{
  Buffer tail = *queue;
  if (tail == NULL_BUFFER) {
    return tail;
  } else {
    return emGetBufferLink(tail, i);
  }
}

void emGenericQueueAdd(Buffer *queue, Buffer newTail, uint16_t i)
{
  Buffer oldTail = *queue;
  Buffer head;

  if (oldTail == NULL_BUFFER) {
    head = newTail;
  } else {
    head = emGetBufferLink(oldTail, i);
    emSetBufferLink(oldTail, i, newTail);
  }

  emSetBufferLink(newTail, i, head);
  *queue = newTail;
  QDEBUG(fprintf(stderr, "[qadd(%08lX, %04X) ", (unsigned long) queue, newTail);
         printQueue(*queue, i);
         fprintf(stderr, "]\n");
         );
}

// Add a buffer, but as the head, not the tail.

void emBufferQueueAddToHead(Buffer *queue, Buffer newHead)
{
  Buffer tail = *queue;
  emBufferQueueAdd(queue, newHead);
  if (tail != NULL_BUFFER) {
    // Switch back to original tail, so newHead ends up as the head.
    *queue = tail;
  }
}

Buffer emBufferQueueNext(Buffer *queue, Buffer finger)
{
  Buffer tail = *queue;
  if (finger == tail) {
    return NULL_BUFFER;
  } else {
    return emGetQueueLink(finger);
  }
}

Buffer emGenericQueueRemoveHead(Buffer *queue, uint16_t i)
{
  Buffer tail = *queue;

  if (tail == NULL_BUFFER) {
    QDEBUG(fprintf(stderr, "[qsub(%08lX) -> zip]\n", (unsigned long) queue); )
    return NULL_BUFFER;
  } else {
    Buffer head = emGetBufferLink(tail, i);
    if (head == tail) {
      *queue = NULL_BUFFER;
    } else {
      emSetBufferLink(tail, i, emGetBufferLink(head, i));
    }
    QDEBUG(fprintf(stderr, "[qsub(%08lX) -> %04X ",
                   (unsigned long) queue, head);
           printQueue(*queue, 0);
           fprintf(stderr, "]\n");
           )
    emSetBufferLink(head, NULL_BUFFER, i);
    return head;
  }
}

// If 'buffer' is NULL_BUFFER this returns the total number of bytes in
// the queue.
uint16_t emGenericQueueRemove(Buffer *queue, Buffer buffer, uint16_t i)
{
  Buffer tail = *queue;

  QDEBUG(fprintf(stderr, "[qremove(%08lX, %04X) ",
                 (unsigned long) queue, buffer);
         printQueue(*queue, i);
         fprintf(stderr, "]\n");
         );

  if (tail == NULL_BUFFER) {
    return 0;
  } else {
    Buffer finger = tail;
    uint16_t length = emGetBufferLength(finger);
    while (true) {
      Buffer next = emGetBufferLink(finger, i);
      if (next == buffer) {
        emSetBufferLink(finger, i, emGetBufferLink(next, i));
        if (tail == buffer) {
          *queue = (finger == tail
                    ? NULL_BUFFER
                    : finger);
        }
        emSetBufferLink(buffer, i, NULL_BUFFER);
        return length;
      } else if (next == tail) {
        return length;
      } else {
        finger = next;
        length += emGetBufferLength(finger);
      }
    }
  }
}

uint16_t emRemoveBytesFromGenericQueue(Buffer *queue, uint16_t count, uint16_t i)
{
  uint16_t todo = count;

  while (0 < todo && *queue != NULL_BUFFER) {
    Buffer head = emGenericQueueHead(queue, i);
    uint16_t length = emGetBufferLength(head);
    if (length <= todo) {
      emGenericQueueRemoveHead(queue, i);
      todo -= length;
    } else {
      emSetBufferLengthFromEnd(head, length - todo);
      todo = 0;
    }
  }

  return count - todo;
}

void emCopyFromGenericQueue(Buffer *queue, uint16_t count, uint8_t *to, uint16_t i)
{
  Buffer tail = *queue;
  Buffer finger = tail;

  if (count == 0) {
    return;
  }

  assert(tail != NULL_BUFFER);
  while (true) {
    Buffer next = emGetBufferLink(finger, i);
    uint16_t have = emGetBufferLength(next);
    uint16_t copy = (have < count
                     ? have
                     : count);
    MEMMOVE(to, emGetBufferPointer(next), copy);
    to += copy;
    count -= copy;
    if (count == 0) {
      break;
    }
    assert(next != tail);
    finger = next;
  }
}

void emLinkedPayloadToPayloadQueue(Buffer *queue)
{
  Buffer head = *queue;
  if (head != NULL_BUFFER) {
    Buffer tail = head;
    while (emGetPayloadLink(tail) != NULL_BUFFER) {
      tail = emGetPayloadLink(tail);
    }
    emSetPayloadLink(tail, head);
    *queue = tail;
  }
}

void emPayloadQueueToLinkedPayload(Buffer *queue)
{
  Buffer tail = *queue;

  if (tail != NULL_BUFFER) {
    Buffer head = emGetPayloadLink(tail);
    emSetPayloadLink(tail, NULL_BUFFER);
    *queue = head;
  }
}

//----------------------------------------------------------------
// Utilities to use a queue of buffers as an extensible vector (a
// one-dimensional array).  The buffers in the queue may contain
// different numbers of elements.  This allows the queue to be
// amalgamated, should anyone want to do so.

// Returns the first element in the vector for which
// predicate(element, target) returns true.  If indexLoc is non-NULL
// the index of the element is stored there.

void *emVectorSearch(Vector *vector,
                     EqualityPredicate predicate,
                     const void *target,
                     uint16_t *indexLoc)
{
  Buffer buffer = emBufferQueueHead(&vector->values);
  uint16_t index = 0;

  while (buffer != NULL_BUFFER) {
    uint8_t *finger = emGetBufferPointer(buffer);
    uint16_t length = emGetBufferLength(buffer);
    uint8_t *end = finger + length;
    for (;
         finger < end && index < vector->valueCount;
         finger += vector->valueSize, index++) {
      if (predicate(finger, target)) {
        if (indexLoc != NULL) {
          *indexLoc = index;
        }
        return finger;
      }
    }
    buffer = emBufferQueueNext(&vector->values, buffer);
  }
  return NULL;
}

uint16_t emVectorMatchCount(Vector *vector,
                            EqualityPredicate predicate,
                            const void *target)
{
  Buffer buffer = emBufferQueueHead(&vector->values);
  uint16_t index = 0;
  uint16_t result = 0;

  while (buffer != NULL_BUFFER) {
    uint8_t *finger = emGetBufferPointer(buffer);
    uint16_t length = emGetBufferLength(buffer);
    uint8_t *end = finger + length;
    for (;
         finger < end && index < vector->valueCount;
         finger += vector->valueSize, index++) {
      if (predicate(finger, target)) {
        result++;
      }
    }
    buffer = emBufferQueueNext(&vector->values, buffer);
  }

  return result;
}

// Return the 'index'th element of the vector.

void *emVectorRef(Vector *vector, uint16_t index)
{
  if (vector->valueCount == 0
      || (vector->valueCount - 1) < index) {
    return NULL;
  }

  Buffer buffer = emBufferQueueHead(&vector->values);

  index *= vector->valueSize;

  while (buffer != NULL_BUFFER) {
    uint16_t length = emGetBufferLength(buffer);
    if (index < length) {
      return emGetBufferPointer(buffer) + index;
    } else {
      index -= length;
      buffer = emBufferQueueNext(&vector->values, buffer);
    }
  }
  return NULL;
}

uint16_t emVectorFindIndex(Vector *vector, const uint8_t *value)
{
  Buffer buffer = emBufferQueueHead(&vector->values);
  uint16_t index = 0;

  while (buffer != NULL_BUFFER) {
    uint8_t *contents = emGetBufferPointer(buffer);
    uint16_t length = emGetBufferLength(buffer);
    if (contents <= value && value < (contents + length)) {
      return (index + (value - contents)) / vector->valueSize;
    } else {
      index += length;
      buffer = emBufferQueueNext(&vector->values, buffer);
    }
  }
  return -1;
}

// Add space for 'quanta' more elements to the vector.

void *emVectorAdd(Vector *vector, uint16_t quanta)
{
  if (0 < vector->emptyCount) {
    vector->valueCount += 1;
    vector->emptyCount -= 1;
    return emVectorRef(vector, vector->valueCount - 1);
  } else {
    Buffer buffer = emAllocateBuffer(vector->valueSize * quanta);
    if (buffer == NULL_BUFFER) {
      return NULL;
    }
    emBufferQueueAdd(&vector->values, buffer);
    vector->valueCount += 1;
    vector->emptyCount = quanta - 1;
    return emGetBufferPointer(buffer);
  }
}
