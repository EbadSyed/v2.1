/*
 * File: thread-key-stretch.c
 * Description: Key stretching for the Thread commissioning key (PSKc).
 *
 * Copyright 2015 Silicon Laboratories, Inc.                                *80*
 */

// For strnlen(3) in glibc.
#define _GNU_SOURCE

#include "core/ember-stack.h"
#include "platform/micro/aes.h"
#include "stack/ip/tls/dtls-join.h"

// A commissioner authenticates itself to a border router by establishing
// a J-PAKE/DTLS connection using a key called the PSKc.  The PSKc is
// derived from a user passphrase using the PBKDF2 algorithm combined
// with the AES-CMAC-PRF-128 keyed hash function.  The different parts of
// this are defined in various RFCs:
//   PBKDF2:           RFC 2898
//   AES-CMAC-PRF-128: RFC 4615
//   AES-CMAC:         RFC 4493

// Byte Ordering
//
// All of the algorithms use big-endian byte ordering on strings of
// bytes.  In order to avoid complications, especially when using test
// vectors, the bytes are stored with the high order byte at index
// zero: 'x[0]' is the high-order byte of byte sequence 'x'.  That way,
// when a test value "2b7e1516 28aed2a6 abf71588 09cf4f3c" it can be
// stored as "{ 0x2B, 0x7E, 0x15, 0x16, 0x28, ... }".

// Variables names are taken from the algorithm descriptions in the RFCs.
// Don't blame the messenger.

//----------------------------------------------------------------
// This uses AES128 to generate two new keys from one old one.

HIDDEN void aesCmacKeyGen(const uint8_t *k, uint8_t *k1, uint8_t *k2)
{
  uint8_t l[16];
  MEMSET(l, 0, 16);
  int i;
  const uint8_t *from = l;
  uint8_t *to = k1;

  emLoadKeyIntoCore(k);
  emStandAloneEncryptBlock(l);

  for (i = 0; i < 2; i++) {
    int j;
    uint8_t x = (from[0] << 1) & 0xFE;
    for (j = 1; j < 16; j++) {
      to[j - 1] = x | (from[j] >> 7);
      x = (from[j] << 1) & 0xFE;
    }
    to[15] = ((from[0] & 0x80) == 0
              ? x
              : x ^ 0x87);
    from = k1;
    to = k2;
  }
}

// Utility to xor one 16-byte array with another.
static void xor16(uint8_t *x, const uint8_t *y)
{
  uint8_t i;
  for (i = 0; i < 16; i++) {
    x[i] ^= y[i];
  }
}

// 'k' is a 16-byte key that is used with AES128 to hash the 'len'
// bytes of 'm' into 't'.

HIDDEN void aesCmac(const uint8_t *k,
                    const uint8_t *m,
                    int16u len,
                    uint8_t *t)
{
  uint8_t k1[16];
  uint8_t k2[16];
  uint8_t mLast[16];
  int i;
  aesCmacKeyGen(k, k1, k2);
  uint16_t n = (len + 15) >> 4;

  bool flag;
  if (n == 0) {
    n = 1;
    flag = false;
  } else {
    flag = (len & 0x0F) == 0;
  }

  if (flag) {
    MEMCOPY(mLast, m + (len - 16), 16);
    xor16(mLast, k1);
  } else {
    MEMSET(mLast, 0, 16);
    MEMCOPY(mLast, m + ((n - 1) << 4), len & 0x0F);
    mLast[len & 0x0F] = 0x80;
    xor16(mLast, k2);
  }

  MEMSET(t, 0, 16);
  for (i = 0; i < n - 1; i++) {
    xor16(t, m + (i << 4));
    emStandAloneEncryptBlock(t);
  }

  xor16(t, mLast);
  emStandAloneEncryptBlock(t);
}

// Iteratively hash a password and a salt together to get a key.  The
// idea is to make the iteration count high enough to discourage
// people from brute force attempts to derive the password from the
// key.
//
// Thread uses this for the user's commissioning password.  Thread
// devices store the resulting key, called the PSKc.  A commissioning
// device gets the password from the user, uses PBKDF2 to get the key,
// and then uses the key as a J-PAKE passphrase to connect to a border
// router.  The rationale for the PBKDF2 hashing is that it protects
// users who use the same password in multiple places.  Extracting the
// PSKc from a Thread device does not get you in to the user's bank
// or facebook account.

// aesCmacPrf128() is inlined so that we don't have to repeat the key
// derivation thousands of times.

static void pbkdf2(const uint8_t *password,
                   int16_t passwordLen,
                   uint8_t *salt,       // must have at least four extra bytes
                   int16_t saltLen,
                   uint16_t count,
                   uint8_t *result)
{
  // dump("password", password, passwordLen);

  uint8_t key[16];
  if (passwordLen == 16) {
    MEMCOPY(key, password, 16);
  } else {
    uint8_t zero[16];
    MEMSET(zero, 0, 16);
    aesCmac(zero, password, passwordLen, key);
  }

  // dump("key", key, 16);

  salt[saltLen] = 0;
  salt[saltLen + 1] = 0;
  salt[saltLen + 2] = 0;
  salt[saltLen + 3] = 1;

  // dump("salt", salt, saltLen + 4);

  aesCmac(key, salt, saltLen + 4, result);

  // dump("u0", result, 16);

  uint8_t lastU[16];
  uint16_t i;
  MEMCOPY(lastU, result, 16);
  for (i = 1; i < count; i++) {
    uint8_t nextU[16];
    aesCmac(key, lastU, 16, nextU);
    xor16(result, nextU);
    MEMCOPY(lastU, nextU, 16);
  }
}

// salt = ”Thread” || <Extended PAN ID> || <Network Name>

void emDerivePskc(const uint8_t *passphrase,
                  int16_t passphraseLen,
                  const uint8_t *extendedPanId,
                  const uint8_t *networkName,
                  uint8_t *result)
{
  uint8_t salt[6 + 8 + 16 + 4];         // 4 is for uint32_t added by pbkdf2
  uint16_t nameLength = strnlen((const char *)networkName, 16);
  MEMCOPY(salt, "Thread", 6);
  MEMCOPY(salt + 6, extendedPanId, 8);
  MEMCOPY(salt + 14, networkName, nameLength);
  pbkdf2(passphrase,
         passphraseLen,
         salt,
         nameLength + 14,
         16384,         // iteration count
         result);
}
