/*
 * File: tls-sha256.c
 * Description: Legacy SHA 256 implementation.
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#include "core/ember-stack.h"
#include "tls.h"
#include "tls-sha256.h"
#include "debug.h"

// Not just any random bits, but the first 32 bits of the fractional
// parts of the square roots of the first 8 primes 2..19.

static const uint32_t initialState[] = {
  0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
  0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19
};

void emSha256Start(Sha256State *hashState)
{
  hashState->byteCount = 0;
  MEMCOPY(hashState->state, initialState, sizeof(initialState));
}

// The first 32 bits of the fractional parts of the cube roots of the
// first 64 primes 2..311.

static const uint32_t officialRandomness[] = {
  0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
  0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
  0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
  0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
  0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
  0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
  0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
  0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
  0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
  0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
  0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
  0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
  0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
  0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
  0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
  0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
};

// 32-bit rotate right and 32-bit shift right
#ifdef EMBER_STACK_COBRA
extern __near_func uint32_t fast_rotr(uint32_t x, uint8_t n);
#define ROTR(x, n) (fast_rotr((x), (n)))
#define SHFR(x, n) (fast_rotr((x), (n)) & (0xFFFFFFFFul >> (n)))
#else
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define SHFR(x, n) ((x) >> (n))
#endif

// As Wikipedia says, "Note the great increase in mixing between bits
// of the w[16..63] words compared to SHA-1".

static void hashBytes(Sha256State *hashState, const uint8_t data[64])
{
  uint32_t w[64];
  uint32_t state[8];
  uint8_t i;

#ifdef EMBER_STACK_COBRA
  halResetWatchdog();
#endif

  for (i = 0; i < 16; i++) {
    w[i] = emberFetchHighLowInt32u(((uint8_t *) data) + (i << 2));
  }

  MEMCOPY(state, hashState->state, sizeof(state));

  //emLogLine(SECURITY, "hashBytes");
  //emLogBytesLine(SECURITY, "data ", data, 64);
  //emLogBytesLine(SECURITY, "pre w", (uint8_t*)w, sizeof(w));
  //emLogBytesLine(SECURITY, "pre state", (uint8_t*)state, sizeof(state));

  for (i = 0; i < 64; i++) {
    uint32_t temp1;
    uint32_t temp2 = ((ROTR(state[0], 2)
                       ^ ROTR(state[0], 13)
                       ^ ROTR(state[0], 22))
                      + ((state[0] & state[1])
                         | (state[2] & (state[0] | state[1]))));

    if (16 <= i) {
      uint32_t wTm2 = w[i - 2];
      uint32_t wTm15 = w[i - 15];
      w[i] = ((ROTR(wTm2, 17) ^ ROTR(wTm2, 19) ^ SHFR(wTm2, 10))
              + w[i - 7]
              + (ROTR(wTm15, 7) ^ ROTR(wTm15, 18) ^ SHFR(wTm15, 3))
              + w[i - 16]);
    }

    temp1 = (state[7]
             + (ROTR(state[4], 6) ^ ROTR(state[4], 11) ^ ROTR(state[4], 25))
             + (state[6] ^ (state[4] & (state[5] ^ state[6])))
             + officialRandomness[i]
             + w[i]);

    // Using a loop for this made the code significantly slower on a PC.
    state[7] = state[6];
    state[6] = state[5];
    state[5] = state[4];
    state[4] = state[3] + temp1;
    state[3] = state[2];
    state[2] = state[1];
    state[1] = state[0];
    state[0] = temp1 + temp2;
  }

  //emLogBytesLine(SECURITY, "post state", (uint8_t*)state, sizeof(state));

  for (i = 0; i < 8; i++) {
    hashState->state[i] += state[i];
  }
}

void emSha256HashBytes(Sha256State *hashState, const uint8_t *bytes, uint16_t count)
{
  //dump("sha256", bytes, count);
  uint8_t inBuffer = hashState->byteCount & 0x3F;
  uint8_t bufferSpace = 64 - inBuffer;

  if (count == 0) {
    return;
  }

  hashState->byteCount += count;

  if (0 < inBuffer && bufferSpace <= count) {
    MEMCOPY(hashState->buffer + inBuffer, bytes, bufferSpace);
    hashBytes(hashState, hashState->buffer);
    bytes += bufferSpace;
    count -= bufferSpace;
    inBuffer = 0;
  }

  while (64 <= count) {
    hashBytes(hashState, bytes);
    bytes += 64;
    count -= 64;
  }

  if (0 < count) {
    MEMCOPY(hashState->buffer + inBuffer, bytes, count);
  }
}

void emSha256Finish(Sha256State *hashState, uint8_t *output)
{
  uint32_t byteCount = hashState->byteCount;
  uint8_t haveBytes = byteCount & 0x3F;
  uint8_t i;

  hashState->buffer[haveBytes] = 0x80;
  haveBytes += 1;
  MEMSET(hashState->buffer + haveBytes,
         0,
         64 - haveBytes);
  // buffer is now   [i0 i1 ... iN 80 00 ... 00]

  if (56 < haveBytes) {
    hashBytes(hashState, hashState->buffer);
    MEMSET(hashState->buffer, 0, haveBytes);
  }

  emberStoreHighLowInt32u(hashState->buffer + 56, SHFR(byteCount, 29));
  emberStoreHighLowInt32u(hashState->buffer + 60, byteCount << 3);
  hashBytes(hashState, hashState->buffer);

  for (i = 0; i < 8; i++) {
    emberStoreHighLowInt32u(output + (i << 2), hashState->state[i]);
  }
}

void emSha256Hash(const uint8_t *input, int count, uint8_t *output)
{
  Sha256State hashState;

  emSha256Start(&hashState);
  emSha256HashBytes(&hashState, input, count);
  emSha256Finish(&hashState, output);

  MEMSET(&hashState, 0, sizeof(Sha256State));
}

void emAddHmacPadding(Sha256State *hashState,
                      const uint8_t *key,
                      uint8_t keyLength,
                      uint8_t padByte)
{
  int i;
  uint8_t sum[32];

  // The TLS keys are all 32 bytes, so this won't be needed (which is just
  // as well because we would end up hashing the key twice, once for the
  // start padding and once for the end padding).
  if (64 < keyLength) {
    emSha256Hash(key, keyLength, sum);
    keyLength = 32;
    key = sum;
  }

  emSha256Start(hashState);
  MEMSET(hashState->buffer, padByte, 64);

  for (i = 0; i < keyLength; i++) {
    hashState->buffer[i] = padByte ^ key[i];
  }
  hashState->byteCount = 64;
  hashBytes(hashState, hashState->buffer);

  MEMSET(sum, 0, sizeof(sum));
}

void emSha256HmacFinish(Sha256State *hashState,
                        const uint8_t *key,
                        uint8_t keyLength,
                        uint8_t *output)
{
  emSha256Finish(hashState, output);
  emAddHmacPadding(hashState, key, keyLength, 0x5C);
  emSha256HashBytes(hashState, output, 32);
  emSha256Finish(hashState, output);
}

void emSha256HmacHash(const uint8_t *key,
                      uint8_t keyLength,
                      const uint8_t *bytes,
                      uint16_t count,
                      uint8_t *output)
{
  Sha256State hashState;

  emSha256HmacStart(&hashState, key, keyLength);
  emSha256HashBytes(&hashState, bytes, count);
  emSha256HmacFinish(&hashState, key, keyLength, output);

  MEMSET(&hashState, 0, sizeof(Sha256State));
}
