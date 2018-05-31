// File: 802.15.4-ccm.c
//
// Description: CCM* as used by 802.15.4 security.
//
// Copyright 2009 by Ember Corporation. All rights reserved.                *80*

#include "core/ember-stack.h"
#include "hal/hal.h"
#include "stack/framework/ip-packet-header.h"
#include "phy/security.h"
#include "platform/micro/aes.h"
#include "802-15-4-ccm.h"
#include "stack/ip/zigbee/key-management.h"
#include "stack/ip/zigbee/join.h" // for emLocalIslandId()
#include "stack/ip/ip-address.h"

#if !(defined(UNIX_HOST) || defined(UNIX_HOST_SIM))
#include "mac/mac-header.h"
#endif

#if defined EMBER_TEST
  #include "phy/simulation/security.h"
#endif

// This implements the CCM* encryption algorithm, using AES-128 as the
// underlying block cipher.  AES-128 is essentially a hash function
// controlled by 128-bit (16 byte) keys.  The AES-128 algorithm takes a key
// and a 16-byte block of data and produces a 16-byte block of noise.
//
// The em250 implements AES-128 in hardware.  The hardware can also handle
// some of the other CCM* calculations, but at this point we only use the
// basic AES-128 functionality.
//
// CCM* uses AES-128 for authentication and encryption.  In authentication,
// AES-128 is used create a hash number from a message, called a MIC.
// The MIC is appended to the message before transmission.  The recipient
// hashes the message and verifies that it obtains the same MIC.  Changes
// to either the body of the message or the appended MIC will result in
// the recipient's MIC differing from the appended MIC.
//
// For encryption, AES-128 is used to create a series of pseudo-random
// blocks which are xor-ed with the message to produce the encrypted
// version.  The recipient performs the same operation, which results in
// the original message.  It is important that each message be encrypted
// with a different psuedo-random sequence.  If two messages used the
// same sequence, xor-ing the encrypted messages would produce the xor
// of the unencrypted messages.
//
// Both authentication and encryption begin by encrypting a special block
// of data called a 'nonce' (as in 'for the nonce').  The goal is that each
// use of a particular key will use a different nonce.  The nonce is:
//
// Offset Size
//   0     1    flags
//   1     8    source EUI64
//   9     4    frame counter
//  13     1    control byte
//  14     2    block count
//
// The frame counter is incremented for each message and the block count is
// incremented for each encryption block.  The flags and control byte ensure
// that different operations on the same message, such as MIC generation and
// encryption, do not use the same nonce.

// 802.15.4 and ZigBee use different byte orders for the source EUI64 and
// the frame counter in the nonce.  ZigBee uses the over-the-air order for
// both and 802.15.4 reverses them.  Hence the need for the following.

// Handy macro.  This is unsafe because it duplicates its arguments.
#define min(x, y) ((x) < (y) ? (x) : (y))

#if !(defined(UNIX_HOST) || defined(UNIX_HOST_SIM))
static void reverseMemCopy(uint8_t* dest, uint8_t* src, uint8_t length)
{
  uint8_t i;
  uint8_t j;

  for (i = 0, j = length - 1; i < length; i++, j--) {
    dest[i] = src[j];
  }
}

//----------------------------------------------------------------
// Maintaining our outgoing NWK frame counter.
//
// We backup each frame counters every 2^12 (4096) messages.  The
// tokens are counter tokens, which can be incremented with a minimum
// number of flash writes.  The token holds the actual value divided
// by 4096.  The tokens are initialized to 0 and incremented at the
// first use and every 4096 uses thereafter.
//
// Keeping the next outgoing frame counter, instead of the previous one,
// makes it easier to start from 0.

uint32_t emNextNwkFrameCounter = 0;

#define FRAME_COUNTER_UPDATE_INTERVAL_LOG 12
#define FRAME_COUNTER_UPDATE_INTERVAL  (1 << FRAME_COUNTER_UPDATE_INTERVAL_LOG)
#define FRAME_COUNTER_UPDATE_MASK      (FRAME_COUNTER_UPDATE_INTERVAL - 1)

void emSecurityReadFrameCounterToken(void)
{
  tokTypeStackNonceCounter tok;

  halCommonGetToken(&tok, TOKEN_STACK_NONCE_COUNTER);
  emNextNwkFrameCounter = tok << FRAME_COUNTER_UPDATE_INTERVAL_LOG;
}

//----------------
// NWK frame counter

uint32_t emSecurityIncrementOutgoingFrameCounter(void)
{
  if ((emNextNwkFrameCounter & FRAME_COUNTER_UPDATE_MASK) == 0) {
    halCommonIncrementCounterToken(TOKEN_STACK_NONCE_COUNTER);
  }

  return emNextNwkFrameCounter++;
}

uint32_t emGetSecurityFrameCounter(void)
{
  return emNextNwkFrameCounter;
}

void emResetNwkFrameCounter(void)
{
  tokTypeStackNonceCounter tok = 0;
  emNextNwkFrameCounter = 0;
  halCommonSetToken(TOKEN_STACK_NONCE_COUNTER, &tok);
}

void emSetNwkFrameCounter(uint32_t newFrameCounter)
{
  emNextNwkFrameCounter = newFrameCounter;
  tokTypeStackNonceCounter tok = (emNextNwkFrameCounter
                                  >> FRAME_COUNTER_UPDATE_INTERVAL_LOG);
  halCommonSetToken(TOKEN_STACK_NONCE_COUNTER, &tok);
}

void emSecurityInit(void)
{
  emSecurityReadFrameCounterToken();
  assert(emZigbeeNetworkSecurityLevel == 0
         || emZigbeeNetworkSecurityLevel == 5);
}

#endif

//----------------------------------------------------------------
// MIC encryption flags

#define COMMON_MIC_FLAGS \
  (STANDALONE_FLAGS_ADATA_FIELD_NONZERO | STANDALONE_FLAGS_L_FIELD_2_BYTES)

#ifdef EMBER_TEST

// The 802.15.4 test vectors use a variety of security levels, so for
// testing we need to be able to change the MIC length and whether or
// not we do encryption.  In actual stack builds we only need the code
// for security level 5 (use encryption and a 4-byte MIC).

static uint8_t PGM micLengthValues[] = { 0, 4, 8, 16 };
static uint8_t PGM micFlagValues[] =
{
  COMMON_MIC_FLAGS,
  COMMON_MIC_FLAGS | STANDALONE_FLAGS_M_FIELD_4_BYTES,
  COMMON_MIC_FLAGS | STANDALONE_FLAGS_M_FIELD_8_BYTES,
  COMMON_MIC_FLAGS | STANDALONE_FLAGS_M_FIELD_16_BYTES,
};

#define macMicLength micLengthValues[emZigbeeNetworkSecurityLevel & 0x03]
#define micNonceFlags micFlagValues[emZigbeeNetworkSecurityLevel & 0x03]
#define useEncryption (4 <= emZigbeeNetworkSecurityLevel)

uint8_t emMacMicLength(void)
{
  return macMicLength;
}

#else
// Hardwire security level 5.
#define macMicLength 4
#define micNonceFlags (COMMON_MIC_FLAGS | STANDALONE_FLAGS_M_FIELD_4_BYTES)
#define useEncryption true
#endif

//----------------------------------------------------------------
// Performs an actual nonce encryption, after first setting the fields
// specific to this block.  We do a copy to avoid clobbering the (shared)
// nonce.

#define encryptMicBlock0(nonce, variableField, result) \
  (encryptNonce((nonce), 0xFF, (variableField), (result)))

#define encryptBlock0(nonce, variableField, result) \
  (encryptNonce((nonce), 0x03, (variableField), (result)))

static void encryptNonce(const uint8_t nonce[SECURITY_BLOCK_SIZE],
                         uint8_t flagsMask,
                         uint16_t variableField,
                         uint8_t block[SECURITY_BLOCK_SIZE])
{
  MEMCOPY(block, nonce, SECURITY_BLOCK_SIZE);

  block[STANDALONE_FLAGS_INDEX] &= flagsMask;
  block[STANDALONE_VARIABLE_FIELD_INDEX_HIGH] = HIGH_BYTE(variableField);
  block[STANDALONE_VARIABLE_FIELD_INDEX_LOW] = LOW_BYTE(variableField);

//  simPrint("nonce: %02X %02X %02X]\n", flags, HIGH_BYTE(variableField),
//           LOW_BYTE(variableField));

  emStandAloneEncryptBlock(block);
}

//----------------------------------------------------------------
// This performs the core of the MIC calculation.  'Count' bytes from
// 'bytes' are xor-ed into 'block' and then encrypted.  We start at
// 'blockIndex' in the block.
//
// The final blockIndex is returned.

static uint8_t xorBytesIntoBlock(uint8_t *block,
                                 uint8_t blockIndex,
                                 const uint8_t *bytes,
                                 uint16_t count)
{
  uint16_t i;

//  {
//    fprintf(stderr, "[xor %d [%02X", count, bytes[0]);
//    for (i = 1; i < count; i++)
//      fprintf(stderr, " %02X", bytes[i]);
//    fprintf(stderr, "]]\n");
//  }

  for (i = 0; i < count; ) {
    uint16_t needed = SECURITY_BLOCK_SIZE - blockIndex;
    uint16_t todo = count - i;
    uint16_t copied = min(todo, needed);
    uint16_t j;

    for (j = 0; j < copied; j++, blockIndex++) {
//      fprintf(stderr, "[%02x ^ %02X =", block[blockIndex], *bytes);
      block[blockIndex] ^= *bytes++;
//      fprintf(stderr, " %02x]\n", block[blockIndex]);
    }
    i += copied;

    if (blockIndex == SECURITY_BLOCK_SIZE) {
      emStandAloneEncryptBlock(block);
      blockIndex = 0;
    }
  }
  return blockIndex;
}

// Calculate the MIC by hashing first the authenticated portion of the
// packet and then the encrypted portion (which hasn't been encrypted yet).
//
// The encrypted bytes are processed on a block boundary, so we finish off
// the block at the end of the authenticated bytes.
//
// The 'for' loop goes around two times (authenticated bytes, encrypted bytes).

static void calculateMic(const uint8_t *authenticate,
                         uint16_t authenticateLength,
                         const uint8_t *encrypt,
                         uint16_t encryptLength,
                         const uint8_t nonce[SECURITY_BLOCK_SIZE],
                         uint8_t *micResult,
                         uint16_t micResultLength)
{
  uint8_t encryptionBlock[SECURITY_BLOCK_SIZE];
  uint8_t blockIndex = 2;      // skip over length
  const uint8_t *chunk = authenticate;
  uint16_t chunkLength;
  uint8_t phase;

  chunkLength = authenticateLength;
  encryptMicBlock0(nonce, encryptLength, encryptionBlock);

  // First two bytes are the 16-bit representation of the frame length,
  // high byte first.
  encryptionBlock[0] ^= HIGH_BYTE(authenticateLength);
  encryptionBlock[1] ^= LOW_BYTE(authenticateLength);

  // phase 0: authenticated bytes
  // phase 1: encrypted bytes

  for (phase = 0; phase < 2; phase++) {
    blockIndex = xorBytesIntoBlock(encryptionBlock,
                                   blockIndex,
                                   chunk,
                                   chunkLength);
    chunk = encrypt;
    chunkLength = encryptLength;

    // Finish off authentication if not on a block boundary.
    if (0 < blockIndex) {
//      simPrint("finish %d", blockIndex);
      emStandAloneEncryptBlock(encryptionBlock);
      blockIndex = 0;
    }
  }

  MEMCOPY(micResult, encryptionBlock, micResultLength);
}

// Encrypt the payload by xor-ing it with a series of AES-encrypted nonces.

static void encryptBytes(uint8_t* bytes,
                         uint16_t length,
                         uint16_t blockCount,
                         const uint8_t nonce[SECURITY_BLOCK_SIZE])
{
  uint8_t encryptionBlock[SECURITY_BLOCK_SIZE];

  for (; 0 < length; ) {
    uint8_t todo = min(length, SECURITY_BLOCK_SIZE);
    uint8_t i;

    encryptBlock0(nonce, blockCount, encryptionBlock);
    blockCount += 1;

    for (i = 0; i < todo; i++) {
      *bytes++ ^= encryptionBlock[i];
    }

    length -= todo;
  }
}

// The MIC gets encrypted as block zero of the message.

#define encryptMic(mic, nonce) \
  (encryptBytes((mic), macMicLength, 0, (nonce)))

// The payload gets encrypted starting from block 1.

#define encryptPayload(payload, length, nonce) \
  (encryptBytes((payload), (length), 1, (nonce)))

#if !(defined(UNIX_HOST) || defined(UNIX_HOST_SIM))

//----------------------------------------------------------------
// Initialize the nonce from an aux frame.  This returns the size of
// the aux frame.
//
// In 802.15.4 security the EUI64 and frame counter have to be reversed
// when being copied from the message to the nonce.
//
// The frame looks like:
//  <frame control:1> <frame counter:4>
//  <key source:[0 4 8]> <key identifier:[0 1]>
// There may be a key identifier with no key source, but not vice versa.

static uint8_t const macAuxFrameSize[] = { 5, 6, 10, 14 };

uint8_t emGetAuxFrameSize(const uint8_t *auxFrame)
{
  return macAuxFrameSize[(auxFrame[0] >> 3) & 0x03];
}

// Returns the key identifier mode
uint8_t emGetSequenceFromAuxFrame(const uint8_t *auxFrame, uint32_t *sequence)
{
  uint8_t auxFrameSize = emGetAuxFrameSize(auxFrame);
  if (auxFrameSize == 5) {
    return MAC_SECURITY_CONTROL_NO_KEY_IDENTIFIER;
  } else {
    uint8_t keyIdentifierMode = (auxFrame[0] & 0xFA);
    if (keyIdentifierMode == MAC_SECURITY_CONTROL_NO_KEY_SOURCE) {
      *sequence = auxFrame[5];
    } else if (keyIdentifierMode == MAC_SECURITY_CONTROL_FOUR_BYTE_KEY_SOURCE) {
      *sequence = emberFetchHighLowInt32u(auxFrame + 5);
    }

    return keyIdentifierMode;
  }
}

static uint8_t initializeNonceFromAuxFrame(uint8_t *nonce,
                                           uint8_t *auxFrame,
                                           uint8_t *sourceAddress)
{
  uint8_t frameControl = auxFrame[0];
  uint8_t auxFrameSize = emGetAuxFrameSize(auxFrame);

  nonce[STANDALONE_FLAGS_INDEX] = micNonceFlags;

  reverseMemCopy(nonce + STANDALONE_NONCE_SOURCE_ADDR_INDEX,
                 sourceAddress,
                 EUI64_SIZE);

  reverseMemCopy(nonce + STANDALONE_NONCE_FRAME_COUNTER_INDEX,
                 auxFrame + 1,
                 SECURITY_FRAME_COUNTER_SIZE);

  // Mask out all but the security level.
  nonce[STANDALONE_NONCE_SECURITY_CONTROL_INDEX] = frameControl & 0x07;

/*
   simPrint("nonce [%02X %02X %02X %02X %02X %02X %02X %02X] [%02X %02X %02X %02X] %02X",
           nonce[STANDALONE_NONCE_SOURCE_ADDR_INDEX],
           nonce[STANDALONE_NONCE_SOURCE_ADDR_INDEX + 1],
           nonce[STANDALONE_NONCE_SOURCE_ADDR_INDEX + 2],
           nonce[STANDALONE_NONCE_SOURCE_ADDR_INDEX + 3],
           nonce[STANDALONE_NONCE_SOURCE_ADDR_INDEX + 4],
           nonce[STANDALONE_NONCE_SOURCE_ADDR_INDEX + 5],
           nonce[STANDALONE_NONCE_SOURCE_ADDR_INDEX + 6],
           nonce[STANDALONE_NONCE_SOURCE_ADDR_INDEX + 7],
           nonce[STANDALONE_NONCE_FRAME_COUNTER_INDEX],
           nonce[STANDALONE_NONCE_FRAME_COUNTER_INDEX + 1],
           nonce[STANDALONE_NONCE_FRAME_COUNTER_INDEX + 2],
           nonce[STANDALONE_NONCE_FRAME_COUNTER_INDEX + 3],
           nonce[STANDALONE_NONCE_SECURITY_CONTROL_INDEX]);
 */
  return auxFrameSize;
}

// 802.15.4 counts the start of the MAC payload as header and doesn't
// encrypt it.  The size of the header data depends on the type of packet.
static uint8_t PGM macHeaderDataSizes[] = {
  2,            // beacon  - GTS and pending address fields (for ZigBee these
                //           are both one byte, in general they can be longer).
  0,            // data    - no header data
  0,            // ack     - never encrypted
  1             // command - the command is not encrypted
};

#define macHeaderDataSize(packet) \
  (macHeaderDataSizes[(packet)[0] & 0x03])

#endif

//----------------------------------------------------------------
// The core encryption function.

void emCcmEncrypt(const uint8_t *nonce,
                  uint8_t *authenticate,
                  uint16_t authenticateLength,
                  uint8_t *encrypt,
                  uint16_t encryptLength,
                  uint8_t *mic,
                  uint8_t packetMicLength)
{
  if (0 < packetMicLength) {
    calculateMic(authenticate,
                 authenticateLength,
                 encrypt,
                 encryptLength,
                 nonce,
                 mic,
                 packetMicLength);
    encryptBytes(mic, packetMicLength, 0, nonce);
  }

  if (0 < encryptLength) {
    encryptPayload(encrypt, encryptLength, nonce);
  }
}

void emCcmEncryptPacket(const uint8_t *nonce,
                        uint8_t *packet,
                        uint16_t authenticateLength,
                        uint16_t encryptLength,
                        uint8_t packetMicLength)
{
  emCcmEncrypt(nonce,
               packet,
               authenticateLength,
               packet + authenticateLength,
               encryptLength,
               packet + authenticateLength + encryptLength,
               packetMicLength);
}

#if !(defined(UNIX_HOST) || defined(UNIX_HOST_SIM))

// Used as the source long ID when encrypting and decrypting wakeup messages.
const uint8_t emDefaultSourceLongId[8] = {
  0x42, 0x8A, 0x2F, 0x98, 0x71, 0x37, 0x44, 0x91
};

// See section 7.2.2.3.1 of the thread spec for key id mode 2 long id.
// Stored in little endian format since that's how we store long ids.
const uint8_t emKeyIdMode2LongId[8] = {
  0x12, 0x87, 0xd4, 0x23, 0xb8, 0xfe, 0x06, 0x35
};

// See section 7.2.2.2.1.
const uint8_t emKeyIdMode2Key[16] = {
  0x78, 0x58, 0x16, 0x86, 0xfd, 0xb4, 0x58, 0x0f,
  0xb0, 0x92, 0x54, 0x6a, 0xec, 0xbd, 0x15, 0x66
};

// The encryption is done in the transmit buffer just before transmission.
// Returns the number of added bytes.

uint8_t emMacEncrypt(uint8_t *packet,
                     uint8_t packetLength,
                     const uint8_t *key,
                     uint8_t keyIdMode,
                     const uint8_t *keySource,
                     uint8_t keySequenceNumber,
                     uint32_t frameCounter)
{
  uint8_t auxFrameIndex = emMacHeaderLength(packet);
  uint8_t *auxHeader = packet + auxFrameIndex;
  uint8_t *finger = auxHeader;
  uint8_t *sourceLongId = (uint8_t *) emMacExtendedId;
  uint16_t frameControl = emberFetchLowHighInt16u(packet);

  if ((frameControl & MAC_FRAME_SOURCE_MODE_MASK)
      == MAC_FRAME_SOURCE_MODE_NONE) {
    sourceLongId = (uint8_t *) emDefaultSourceLongId;
  }

  // See Thread spec 7.2.2 for key id mode 2 special values.
  if (keyIdMode == MAC_SECURITY_CONTROL_FOUR_BYTE_KEY_SOURCE) {
    sourceLongId = (uint8_t *) emKeyIdMode2LongId;
    frameCounter = 0;
    key = emKeyIdMode2Key;
  }

  uint8_t auxHeaderLength = macAuxFrameSize[keyIdMode >> 3];
  MEMMOVE(auxHeader + auxHeaderLength,
          auxHeader,
          packetLength - auxFrameIndex);
  packetLength += auxHeaderLength;

  *finger++ = ((emZigbeeNetworkSecurityLevel & 0x07)
               | keyIdMode);

  emberStoreLowHighInt32u(finger, frameCounter);
  finger += 4;

  switch (keyIdMode) {
    case MAC_SECURITY_CONTROL_FOUR_BYTE_KEY_SOURCE: {
      emberStoreHighLowInt32u(finger, 0xFFFFFFFF);
      finger += 4;
      *finger++ = 0xFF;
      break;
    }
    case MAC_SECURITY_CONTROL_EIGHT_BYTE_KEY_SOURCE:
      MEMCOPY(finger, sourceLongId, 8);
      finger += 8;
    // fall through
    case MAC_SECURITY_CONTROL_NO_KEY_SOURCE:
      *finger++ = keySequenceNumber;
      break;
  }

  emLoadKeyIntoCore(key);

  {
    uint8_t nonce[SECURITY_BLOCK_SIZE];
    uint8_t encryptionStartIndex
      = auxFrameIndex
        + auxHeaderLength
        + macHeaderDataSize(packet);

    uint8_t encryptLength = ((useEncryption
                              && (packetLength > encryptionStartIndex))
                             ? packetLength - encryptionStartIndex
                             : 0);

    initializeNonceFromAuxFrame(nonce, packet + auxFrameIndex, sourceLongId);

    //emLogBytesLine(SECURITY,
    //               "encrypting packet with nonce",
    //               nonce,
    //               SECURITY_BLOCK_SIZE);

    emCcmEncryptPacket(nonce,
                       packet,
                       encryptionStartIndex,
                       encryptLength,
                       macMicLength);
    return auxHeaderLength + macMicLength;
  }
}

// Test function that allows the use of arbitrary aux frames.

#ifdef EMBER_TEST
HIDDEN uint8_t macEncrypt(uint8_t* packet,
                          uint8_t packetLength,
                          uint8_t auxFrameIndex)
{
  uint8_t nonce[SECURITY_BLOCK_SIZE];
  uint8_t auxHeaderLength =
    initializeNonceFromAuxFrame(nonce,
                                packet + auxFrameIndex,
                                (uint8_t *) emMacExtendedId);
  uint8_t encryptionStartIndex =
    auxFrameIndex
    + auxHeaderLength
    + macHeaderDataSize(packet);

  emLoadKeyIntoCore(emGetActiveMacKeyAndSequence(NULL));
  if (useEncryption) {
    emCcmEncryptPacket(nonce,
                       packet,
                       encryptionStartIndex,
                       packetLength - encryptionStartIndex,
                       macMicLength);
  } else {
    emCcmEncryptPacket(nonce,
                       packet,
                       packetLength,
                       0,
                       macMicLength);
  }
  return auxHeaderLength + macMicLength;
}
#endif

#endif

//----------------------------------------------------------------
// Decryption.

// packetLength does not include the MIC.

bool emCcmDecryptPacket(const uint8_t *nonce,
                        uint8_t *packet,
                        uint16_t authenticateLength,
                        uint8_t *encrypt,
                        uint16_t encryptLength,
                        uint8_t packetMicLength)
{
  encryptPayload(encrypt, encryptLength, nonce);

  if (0 < packetMicLength) {
    uint8_t rxMic[MIC_SIZE_MAX];
    uint8_t calcMic[MIC_SIZE_MAX];

    MEMCOPY(rxMic,
            encrypt + encryptLength,
            packetMicLength);
    // encryption is self-reversing
    encryptBytes(rxMic, packetMicLength, 0, nonce);
    calculateMic(packet,
                 authenticateLength,
                 encrypt,
                 encryptLength,
                 nonce,
                 calcMic,
                 packetMicLength);
    return MEMCOMPARE(rxMic, calcMic, packetMicLength) == 0;
  } else {
    return true;
  }
}

#if !(defined(UNIX_HOST) || defined(UNIX_HOST_SIM))

uint8_t *emGetLongIdFromAuxFrame(uint8_t *packet)
{
  uint8_t auxFrameIndex = emMacHeaderLength(packet);
  uint8_t *auxFrame = packet + auxFrameIndex;
  if ((auxFrame[0] & MAC_SECURITY_CONTROL_KEY_IDENTIFIER_MASK)
      == MAC_SECURITY_CONTROL_EIGHT_BYTE_KEY_SOURCE) {
    return auxFrame + 5;
  } else {
    return NULL;
  }
}

// Decrypts the packet in place, removing the security subframe in
// the process and reducing the length.
//
// Returns true if successful and false otherwise.

bool emMacDecrypt(uint8_t *packet,
                  uint8_t *lengthLoc,
                  uint8_t *sourceLongId,
                  const uint8_t *key)
{
  uint8_t nonce[SECURITY_BLOCK_SIZE];
  if (*lengthLoc < macMicLength) {
    return false;
  }
  uint8_t packetLength = *lengthLoc - macMicLength;
  uint8_t auxFrameIndex = emMacHeaderLength(packet);
  uint8_t auxFrameLength = initializeNonceFromAuxFrame(nonce,
                                                       packet + auxFrameIndex,
                                                       sourceLongId);
  uint8_t payloadIndex = auxFrameIndex + auxFrameLength;
  uint8_t encryptionStartIndex = payloadIndex + macHeaderDataSize(packet);
  if (packetLength < encryptionStartIndex) {
    return false;
  }

  emLoadKeyIntoCore(key);
  if (!useEncryption) {
    encryptionStartIndex = packetLength;
  }

  uint8_t encryptLength = packetLength - encryptionStartIndex;

  //emLogBytesLine(SECURITY,
  //               "decrypting packet with nonce",
  //               nonce,
  //               SECURITY_BLOCK_SIZE);

  if (emCcmDecryptPacket(nonce,
                         packet,
                         encryptionStartIndex,
                         packet + encryptionStartIndex,
                         encryptLength,
                         macMicLength)) {
    MEMMOVE(packet + auxFrameIndex,
            packet + payloadIndex,
            packetLength - payloadIndex);
    *lengthLoc = packetLength - auxFrameLength;
    return true;
  }
  return false;
}

#endif
