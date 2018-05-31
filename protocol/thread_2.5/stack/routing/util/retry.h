// File: retry.h
//
// Description: A queue for retrying and/or delaying outgoing packets.
// This is independent of MAC layer retries.  A typical use is to retry
// MAC layer broadcasts, or to add initial jitter to any outgoing packet.
//
// There are really two queues: a "submit" queue for entries that have been
// submitted to the mac, and a "delay" queue for entries with nonzero delay.
//
// This code was originally in grad/retry.c and has been made generic.
//
// Author(s): Matteo Neale Paris <matteo@ember.com>
//            Jeff Mathews <jeff.mathews@ember.com>
//
// Copyright 2001-2004 by Ember Corporation.  All rights reserved.          *80*

#ifndef RETRY_H
#define RETRY_H

extern uint8_t emberRetryQueueSize;

// There are two queues, one for messages that we have to wait to send
// (the delay queue) and one for messages ready to go out (the submit
// queue).  They are kept in the emRetryEntries[] array in queue order,
// first the submit queue and then the delay queue.  Moving the head
// of the delay queue to the tail of the submit queue can be done by
// decrementing delayCount and incrementing submitCount.  This is not
// a time-critical operation, but it helps to understand how the queues
// are represented.

extern RetryEntry *emRetryEntries;

#define EXTENDED_RETRY_COUNT 2
#define EXTENDED_RETRY_MIN_DELAY_MS 16
#define EXTENDED_RETRY_LOG_JITTER 5
#define EXTENDED_RETRY_TIMEOUT_MS 250

#define emRetrySuccessfulAttempts(entry) ((entry)->attempts >> 4)
#define emRetryRemainingAttempts(entry) ((entry)->attempts & 0x0F)
#define emRetryIncrementSuccessfulAttempts(entry) ((entry)->attempts += 0x10)
#define emRetryIncrementRemainingAttempts(entry) ((entry)->attempts += 0x01)
#define emRetryClearSuccessfulAttempts(entry) ((entry)->attempts &= 0x0F)
#define emRetrySetRemainingAttempts(entry, remaining) \
  ((entry)->attempts = ((entry)->attempts & 0xF0) + ((remaining) & 0x0F))

// Returns false if the queues were full.
bool emRetrySubmit(PacketHeader header,
                   uint8_t retries,
                   uint16_t delayMs);

// Called after a transmission succeeds to see if further sends are
// necessary.
bool emNeedsFurtherTransmissions(RetryEntry *entry);

// Called when the radio is available.  Returns the next packet that
// needs to go out, or EMBER_NULL_MESSAGE_BUFFER if there is none.
PacketHeader emRetryTransmit(void);

// This call informs the retry module that the packet has been transmitted.
// Typically called from within the emMacTransmitCompleteCallback, which
// has different implementations depending on the stack.
// Returns true if the header is found on the retry table.
bool emRetryTransmitComplete(PacketHeader header, EmberStatus status);

// Callback to higher layer immediately before submission to the mac.
// The higher layer code must set the value of the RetryEntry timer
// for the next delay.  If it returns not success, the entry is not
// submitted to the mac, and is placed back on the retry queue if
// there are any remaining attempts.
EmberStatus emPrepareRetryEntryForSubmission(RetryEntry *entry);

bool emRetryIsEmpty(void);
bool emHavePendingRetryMessages(void);
void emRetryInit(void);

// Called by the retry code when it is time to resend a packet.  This
// needs to be provided by whatever routing layer is present.  Returns
// true if successful and false otherwise.
//
// When using ZigBee stand-alone security on the EM2420 this has to copy
// the message and then encrypt it before sending it to the MAC.  Everyone
// else just calls emMacSubmit().  Because of the freshness check done
// using the frame counter messages must be encrypted in the same order
// as they are transmitted, which means that we have to re-encrypt it for
// each retry.

bool emRetryRetransmit(PacketHeader header);

// Called when transmission of a network message is complete.
void emRetryTransmitCompleteCallback(PacketHeader header,
                                     EmberStatus status);

void emMarkRetryBuffers(void);

#endif // RETRY_H
