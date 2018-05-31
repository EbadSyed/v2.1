// File: mac.h
// Description: Medium Access Control Sublayer.
// Handles transmit, backoff, and carrier detect.
// Modelled on the 802.15.4 unslotted CSMA/CA spec.
//
// Culprit(s): Lee Taylor <lee@ember.com>
//             Jeff Mathews <jeff@ember.com>
//             Matteo Neale Paris <matteo@ember.com>
//             Richard Kelsey <richard@ember.com>
//
// Copyright 2001-2012 by Ember Corporation.  All rights reserved.          *80*

#ifndef MAC_H
#define MAC_H

#include "mac/mac-header.h"

// The maximum number of times the mac will attempt retransmission of an
// unacknowledged packet.
#define MAC_MAX_ACKD_RETRIES_DEFAULT  (3)

#define MAC_ACK_WAIT_DURATION 54 // 54 symbols = 864 microseconds

// Mac states see halStackSymbolDelayAIsr in mac.c for descriptions
enum {
  // The MAC is idle with nothing to do.  The radio may also be
  //  idle if the node is a sleepy type.
  MAC_STATE_IDLE,
  // The phy-specific portion of the mac is dealing with the process of
  //  transmitting a packet (may involve hardware-csma, etc) pass-through
  //  to it for additional handling.
  MAC_STATE_TRANSMIT,
  // After transmitting a data poll, an ack with frame-pending set was rx'd
  //   If this state times out, no data was received within the timeout
  MAC_STATE_EXPECTING_DATA,
  // waiting for deferred foreground processing in emMacTimerEventHandler()
  MAC_STATE_EVENT_PENDING,
  // The mac is dwelling on a channel for the purpose of a scan
  MAC_STATE_SCAN_DWELL,
  // The mac is in between channel dwells, waiting for the next thing to do
  //  In many ways similar to IDLE but the radio is not actually idled
  MAC_STATE_SCAN_PEND
};

extern uint8_t emMacState;
extern uint8_t emMacMaxAckRetries;

// The PHY payload is 127 bytes, including the two-byte CRC (which is
// technically the MAC's responsibility) and not including the initial
// length byte (which is PHY overhead).  The transmit buffer contains
// the length byte but not the CRC, which is appended by the hardware
// during transmission.
//    127 (PHY payload)
//    + 1 (length byte)
//    - 2 (no CRC)
//  = 126
#define TRANSMIT_BUFFER_SIZE 126

#define MAC_CRC_LENGTH         2

extern Event emMacTransmitEvent;
void emMacTransmitEventHandler(void);

void emMacKickStart(void);
// Returns true if CCA should be used when transmitting this packet. It also set
// the flag prepareSuccess to true if encryption and compression succeeded,
// false otherwise.
bool emMacPrepareTransmitBuffer(PacketHeader header,
                                bool *prepareSuccess,
                                uint8_t *newChannel);
void emMacSetFrameCounter(EmberMessageBuffer buffer, uint8_t offset);
void emMacCacheCurrentFrameCounter(void);
void emMacScanEnd(void);
void emMacScanDwell(uint16_t duration);
// Called from emberTick().
// Calls emberRadioNeedsCalibratingHandler() if radio needs calibrating.
void emMacCheckRadio(void);
extern EmberEventControl emCheckRadioEvent;
void emCheckRadioEventHandler(void);
bool emRadioReceiveMacHeaderCallback(Buffer header);
void emReceiveRawPacket(uint8_t *packet, uint8_t length, uint8_t padding);
bool emProcessNetworkHeaderIsr(PacketHeader macHeaderOnly,
                               uint8_t* networkHeader);
EmberStatus emMacSetIdleMode(RadioPowerMode mode); // EMIPSTACK-336

#define emMacIncomingQueueIsEmpty emPhyToMacQueueIsEmpty

#endif // MAC_H
