// File: mac-arbiter.c
//
// Description: Manages the mac transmit queue and isolates the mac (and mac
// interrupt thread) from higher layers. The mac arbiter is the only code that
// is allowed to call the mac directly.
//
// General principle: if you're reading or writing to the transmit queue in
// non-interrupt context, you need to disable interrupts.
//
// Author(s): Lee Taylor <lee@ember.com>
//            Matteo Neale Paris <matteo@ember.com>
//            Jeff Mathews <jeff.mathews@ember.com>
//
// Copyright 2005 by Ember Corporation.  All rights reserved.               *80*

#ifndef MAC_ARBITER_H
#define MAC_ARBITER_H

extern uint8_t emOldChannel;

// Submit a buffer to be transmitted using CSMA. The buffer will added to the
// front or the back of the transmit queue depending on the priority.
bool emMacSubmit(PacketHeader header, TransmitPriority priority);

// Dequeue the next packet from the transmit queue.
void emMacDequeue(void);

// Suspend transmission until further notice.
enum {
  MAC_SUSPEND_REASON_NONE,
  MAC_SUSPEND_REASON_START_SCAN,
  MAC_SUSPEND_REASON_STOP_SCAN,
  MAC_SUSPEND_REASON_RESET_NETWORK,
  MAC_SUSPEND_REASON_SET_CHANNEL
};
typedef uint8_t MacSuspendReason;
// Return values
enum {
  MAC_SUSPEND_FAILURE,          // suspend already in progress
  MAC_SUSPEND_SUCCESS,          // done already
  MAC_SUSPEND_DELAYED           // will call callback when done
};
uint8_t emMacSuspendTransmission(MacSuspendReason suspendReason);

// Sets the channel once the mac is idle.  Returns false if the channel
// is invalid.  Higher layers should call this function rather than
// emSetPhyRadioChannel(), which can fail if the phy is busy.
bool emMacSetChannel(uint8_t channel);

// Gets the current channel.
uint8_t emMacGetChannel(void);

// Return true if the queue is empty and the MAC state is idle.
bool emMacIsEmpty(void);

// Return true if the MAC is currently dwelling during an active scan.
bool emMacIsDwelling(void);

// Initialize the MAC.
void emMacInit(void);

// Remove all unsent messages from the transmit queue.
void emMacPurgeTransmitQueue(void);

// Called in ISR context when the lower level mac completes
// transmission of a packet.
void emMacTransmitCompleteIsr(EmberStatus status);

// Called outside of ISR context when the lower level mac has
// completed transmission.
void emMacTransmitCompleteHandler(PacketHeader header);

// Called when the lower level mac completes transmission of a scan
// related packet (either MAC_BEACON_REQUEST or MAC_ORPHAN_NOTIFICATION).
//  returns true if the callback handles the message and mac state
//  transition, false if not.
bool emScanCommandTransmitCompleteIsr(uint8_t command, EmberStatus status);

// Notify the next higher layer of a completed transmission.
void emMacTransmitCompleteCallback(PacketHeader header, EmberStatus status);

// Request that a poll be sent after the next transmission completes.
void emMacRequestPoll(void);

// Cancel any requested poll.
void emMacCancelPoll(void);

// Low-level consistency checking and processing of MAC commands.  Returns
// true if the packet does not require further processing.
bool emProcessMacCommand(EmberMessageBuffer header);

// For buffer management.
void emMarkMacArbiterBuffers(void);

#endif//MAC_ARBITER_H
