/*
 * File: rsa.h
 * Description: RSA cryptography.
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef RSA_H
#define RSA_H

int emCheckRsaPublicKey(const RsaPublicKey *key);

int emRsaSign(RsaPrivateKey *key,
              uint16_t hashLength,
              const uint8_t *hash,
              uint8_t *signature);

int emRsaVerify(RsaPrivateKey *key,
                bool isSha256,
                uint16_t hashLength,
                const uint8_t *hash,
                const uint8_t *signature);

#define emRsaVerifyRaw(key, hashLength, hash, signature) \
  (emRsaVerify((key), false, (hashLength), (hash), (signature)))

#define emRsaVerifySha256(key, hash, signature) \
  (emRsaVerify((key), true, 0, (hash), (signature)))

// These release the BigInt values for immediate reuse.
void emFreeRsaPrivateKey(RsaPrivateKey *key);
void emFreeRsaPublicKey(RsaPublicKey *key);

#endif // RSA_H
