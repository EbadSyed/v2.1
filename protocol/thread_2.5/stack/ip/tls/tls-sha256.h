/*
 * File: tls-sha256.h
 * Description: Legacy SHA 256 implementation.
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef TLS_SHA256_H
#define TLS_SHA256_H

#define SHA256_BLOCK_SIZE 32

void emSha256Hash(const uint8_t *input, int count, uint8_t *output);

// Same as emSha256Hash(), but broken into three parts to allow the
// bytes to be hashed incrementally.
void emSha256Start(Sha256State *hashState);
void emSha256HashBytes(Sha256State *hashState,
                       const uint8_t *bytes,
                       uint16_t count);
void emSha256Finish(Sha256State *hashState, uint8_t *output);

void emSha256HmacHash(const uint8_t *key,
                      uint8_t keyLength,
                      const uint8_t *bytes,
                      uint16_t count,
                      uint8_t *output);

// Same as emSha256HmacHash(), but broken into three parts to allow the
// bytes to be hashed incrementally.  The middle part is emSha256HashBytes().
#define emSha256HmacStart(hashState, key, keyLength) \
  (emAddHmacPadding((hashState), (key), (keyLength), 0x36))
void emAddHmacPadding(Sha256State *hashState,
                      const uint8_t *key,
                      uint8_t keyLength,
                      uint8_t padByte);
void emSha256HmacFinish(Sha256State *hashState,
                        const uint8_t *key,
                        uint8_t keyLength,
                        uint8_t *output);

#endif // TLS_SHA256_H
