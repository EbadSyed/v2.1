// File: 802.15.4-ccm.h
//
// Description: CCM* as used by 802.15.4 security.
//
// Copyright 2009 by Ember Corporation. All rights reserved.                *80*

#ifndef MAC_802_15_4_CCM_H
#define MAC_802_15_4_CCM_H

// Offsets into the initial block.  The code calls this block the
// nonce.  Strictly speaking, the nonce is the source address, frame
// counter, and security control.  It does not include the flags and
// the variable field (which is used to encode the length of the
// message.

#define STANDALONE_FLAGS_INDEX                   0
#define STANDALONE_NONCE_SOURCE_ADDR_INDEX       1
#define STANDALONE_NONCE_FRAME_COUNTER_INDEX     9
#define STANDALONE_NONCE_SECURITY_CONTROL_INDEX 13
#define STANDALONE_VARIABLE_FIELD_INDEX_HIGH    14
#define STANDALONE_VARIABLE_FIELD_INDEX_LOW     15

// For authentication, the flags byte has the following form:
//  <reserved:1> <adata:1> <M:3> <L:3>
// Where
//  <reserved:1> = 0   Reserved for future expansion.
//  <adata:1>          0 for zero-length messages, 1 for non-zero-length
//  <M:3>              MIC length, encoded as (micLength - 2)/2
//  <L:3>              Number of bytes used to represent the length
//                     of the message, - 1.

#define STANDALONE_FLAGS_ADATA_FIELD_NONZERO 0x40
#define STANDALONE_FLAGS_M_FIELD_4_BYTES     0x08
#define STANDALONE_FLAGS_M_FIELD_8_BYTES     0x18
#define STANDALONE_FLAGS_M_FIELD_16_BYTES    0x38
#define STANDALONE_FLAGS_L_FIELD_2_BYTES     0x01
#define STANDALONE_FLAGS_L_FIELD_3_BYTES     0x02
#define STANDALONE_FLAGS_ENCRYPTION_FLAGS    0x02

void emSecurityInit(void);

bool emGetNetworkKeySequence(uint32_t *sequence);
uint8_t *emGetMacKey(uint32_t sequence, uint8_t *storage);
uint8_t *emGetLegacyNetworkKey(void);
void emSwitchToNetworkKey(uint32_t sequence);

#ifdef EMBER_TEST
uint8_t macEncrypt(uint8_t* packet,
                   uint8_t packetLength,
                   uint8_t auxFrameIndex);

uint8_t emMacMicLength(void);
#else
// Hardwire security level 5 to save flash.
#define emMacMicLength() 4
#endif

uint8_t *emGetLongIdFromAuxFrame(uint8_t *packet);

uint8_t emGetAuxFrameSize(const uint8_t *auxFrame);

// Returns the key identifier mode
uint8_t emGetSequenceFromAuxFrame(const uint8_t *auxFrame, uint32_t *sequence);

// Made visible so that test apps can set it.
extern uint32_t emNextNwkFrameCounter;

void emResetNwkFrameCounter(void);

// The core encryption function.
void emCcmEncrypt(const uint8_t *nonce,
                  uint8_t *authenticate,
                  uint16_t authenticateLength,
                  uint8_t *encrypt,
                  uint16_t encryptLength,
                  uint8_t *mic,
                  uint8_t packetMicLength);

// Wrapper used when the authenticated, encrypted, and mic sections are
// adjacent.
void emCcmEncryptPacket(const uint8_t *nonce,
                        uint8_t *packet,
                        uint16_t authenticateLength,
                        uint16_t encryptLength,
                        uint8_t micLength);

// Decrypts the packet in place.  The security subframe is removed.
//
// Returns true if successful and false otherwise.

bool emCcmDecryptPacket(const uint8_t *nonce,
                        uint8_t *packet,
                        uint16_t authenticateLength,
                        uint8_t *encrypt,
                        uint16_t encryptLength,
                        uint8_t packetMicLength);

uint8_t emMacEncrypt(uint8_t *packet,
                     uint8_t packetLength,
                     const uint8_t *key,
                     uint8_t keyIdMode,
                     const uint8_t *keySource,
                     uint8_t keySequenceNumber,
                     uint32_t frameCounter);

bool emMacDecrypt(uint8_t *packet,
                  uint8_t *lengthLoc,
                  uint8_t *sourceEui64,
                  const uint8_t *key);

#endif // MAC_802_15_4_CCM_H
