/*
 * File: stack/platform/micro/generic/aes.c
 * Description: Implementation of AES.
 *
 * Author(s): lost in the mists of time
 *
 * Copyright 2015 Silicon Laboratories, Inc.                                *80*
 */

#include "core/ember-stack.h"
#include "hal/hal.h"
#include "stack/platform/micro/aes.h"

#include "hal/micro/generic/aes/rijndael-api-fst.h"
#include "hal/micro/generic/aes/rijndael-alg-fst.h"

// These are used to keep track of data used by the Rijndael (AES) block cipher
// implementation.
keyInstance myKey;
bool cipherInitialized = false;
cipherInstance myCipher;
uint8_t myKeyData[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
bool keyInitialized = false;

static void securityInit(void)
{
  if (!cipherInitialized) {
    // ZigBee defines the IV for AES Encrypt to be zero, per the Spec. section
    // B.1 Block-Cipher-Based Cryptographic Hash Function
    assert(cipherInit(&myCipher,
                      MODE_CBC,  // XXX: Is this correct???
                      NULL));    // Initialization Vector
    cipherInitialized = true;
  }
}

//------------------------------------------------------------------------------
// This routine simulates loading a key into our cryptographic engine by
// creating (and caching) a key suitable for use in our Unit Test
// AES Block Cihper.

void emLoadKeyIntoCore(const uint8_t* keyData)
{
  char keyMaterial[EMBER_ENCRYPTION_KEY_SIZE * 2];
  uint8_t i;
  uint8_t j = 0;

//  if ( keyInitialized &&
//       0 == memcmp(keyData, myKeyData, EMBER_ENCRYPTION_KEY_SIZE) )
//    return true;

  // The Key material we pass is expected to be in ASCII.
  // So we need to convert the raw HEX into ASCII just
  // so it can be converted back by 'makeKey()'.
  for ( i = 0; i < EMBER_ENCRYPTION_KEY_SIZE; i++ ) {
    uint8_t nibble = keyData[i] >> 4;
    if ( nibble <= 0x09 ) {
      keyMaterial[j] = 0x30 + nibble;
    } else { // if ( nibble >= 0xA && nibble <= 0xF )
      keyMaterial[j] = 0x41 + nibble - 0x0A;
    }
    j++;

    nibble = keyData[i] & 0x0F;
    if ( nibble <= 0x9 ) {
      keyMaterial[j] = 0x30 + nibble;
    } else { // if ( nibble >= 0xA && nibble <= 0xF )
      keyMaterial[j] = 0x41 + nibble - 0x0A;
    }
    j++;

    // Cache in our global
    myKeyData[i] = keyData[i];
  }
  assert(0 <= makeKey(&myKey,
                      DIR_ENCRYPT,
                      EMBER_ENCRYPTION_KEY_SIZE * 8,
                      keyMaterial));

  keyInitialized = true;
}

//------------------------------------------------------------------------------
// Retrieve key cached in our global.

void emGetKeyFromCore(uint8_t* keyPointer)
{
  MEMCOPY(keyPointer, myKeyData, EMBER_ENCRYPTION_KEY_SIZE);
}

//------------------------------------------------------------------------------
// This function utilizes the Rijndael AES Block Cipher implementation to
// perform real AES Encryption.  See 'hal/micro/generic/aes/rijndael-api-fst.c'

void emStandAloneEncryptBlock(uint8_t block[SECURITY_BLOCK_SIZE])
{
  uint8_t outBlock[SECURITY_BLOCK_SIZE];

//  simPrint("AES in:  %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
//           block[0], block[1], block[2], block[3], block[4], block[5], block[6], block[7],
//           block[8], block[9], block[10], block[11], block[12], block[13], block[14], block[15]);

  securityInit();
  assert(0 <= blockEncrypt(&myCipher,
                           &myKey,
                           block,
                           SECURITY_BLOCK_SIZE * 8,
                           outBlock));

  MEMCOPY(block, outBlock, SECURITY_BLOCK_SIZE);
//  simPrint("AES out: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
//           block[0], block[1], block[2], block[3], block[4], block[5], block[6], block[7],
//           block[8], block[9], block[10], block[11], block[12], block[13], block[14], block[15]);
}
