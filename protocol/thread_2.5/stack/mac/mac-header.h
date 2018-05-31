// File: mac-header.h
// Description: Declarations and definitions needed for manipulating the
//   over-the-air MAC header.
//
// Culprit(s): Jeff Mathews <jeff@ember.com>
//             Matteo Neale Paris <matteo@ember.com>
//             Richard Kelsey <richard@ember.com>
//
// Copyright 2004 by Ember Corporation.  All rights reserved.               *80*

#ifndef MAC_HEADER_H
#define MAC_HEADER_H

#include "phy/ieee802154mac.h"

//------------------------------------------------------------------
// 0x0100 is actually reserved, but we set it on incoming messages
// and thus need to allow it.
#define MAC_FRAME_RESERVED_BITS          0x0200

// For incoming unicasts, the PHY copies the frame pending bit on the
// outgoing ACK into this bit on the incoming, ACKed, message.
#define MAC_FRAME_FLAG_CC_FRAME_PENDING  0x0100

#define MAC_DATA_HEADER_LENGTH 9
#define MAC_BEACON_HEADER_LENGTH 7

#define MAC_MIN_HEADER_LENGTH 7

// Auxiliary security header
//
// (1)      frame control
// (4)      frame counter
// (0/4/8)  key identifier
// (0/1)    key index (must be present if key identifier is)

#define MAC_SECURITY_CONTROL_KEY_IDENTIFIER_MASK   0x18
#define MAC_SECURITY_CONTROL_NO_KEY_IDENTIFIER     0x00
#define MAC_SECURITY_CONTROL_NO_KEY_SOURCE         0x08
#define MAC_SECURITY_CONTROL_FOUR_BYTE_KEY_SOURCE  0x10
#define MAC_SECURITY_CONTROL_EIGHT_BYTE_KEY_SOURCE 0x18

#endif // MAC_HEADER_H
