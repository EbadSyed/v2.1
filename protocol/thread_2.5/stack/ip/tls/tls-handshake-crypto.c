/*
 * File: tls-handshake-crypto.c
 * Description: cryptographic utilites for the TLS handshake
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#include "core/ember-stack.h"

#include "phy/phy.h"
#include "debug.h"
#include "ip/udp.h"
#include "ip/tcp.h"
#include "tls.h"
#include "dtls.h"

#include "psk-table.h"
#include "tls-sha256.h"
#include "tls-public-key.h"
#include "tls-handshake-crypto.h"
#include "jpake-ecc.h"

// PRF = PseudoRandom Function

static void prfHash(uint8_t *secret, uint16_t secretLength,
                    char *label,
                    uint8_t *random0, uint16_t randomLength0,
                    uint8_t *random1, uint16_t randomLength1,
                    uint8_t *result, uint16_t resultLength)
{
  uint16_t labelLength = emStrlen((uint8_t *)label);
  uint8_t buffer[SHA256_BLOCK_SIZE];
  uint8_t hash[SHA256_BLOCK_SIZE];        // This could be in the hashState
                                          // buffer, the hash in put and output
                                          // don't overlap.
  Sha256State hashState;

  int i;

  MEMSET(&hashState, 0, sizeof(Sha256State));

  for (i = 0; i < resultLength; i += SHA256_BLOCK_SIZE) {
    uint8_t j;
    uint8_t *hashResult = buffer;
    for (j = 0; j < 2; j++) {
      // j == 0: A(0) == seed, A(i) = HMAC_hash(secret, A(i-1))
      // j == 1: P_hash = ... ++ HMAC_hash(secret, A(i) ++ seed) ++ ...
      emSha256HmacStart(&hashState, secret, secretLength);
      if (0 < i || j == 1) {
        emSha256HashBytes(&hashState, buffer, SHA256_BLOCK_SIZE);
      }
      if (i == 0 || j == 1) {
        // seed = label ++ random
        emSha256HashBytes(&hashState, (uint8_t *)label, labelLength);
        emSha256HashBytes(&hashState, random0, randomLength0);
        emSha256HashBytes(&hashState, random1, randomLength1);
      }
      emSha256HmacFinish(&hashState, secret, secretLength, hashResult);
      hashResult = hash;
    }

    MEMCOPY(result + i,
            hash,
            (resultLength - i < SHA256_BLOCK_SIZE
             ? resultLength - i
             : SHA256_BLOCK_SIZE));
  }

  MEMSET(buffer, 0, sizeof(buffer));
  MEMSET(hash, 0, sizeof(hash));
  MEMSET(&hashState, 0, sizeof(hashState));
}

void emComputeDerivedKeys(TlsState *tls,
                          char *label,
                          bool clientSaltFirst,
                          uint8_t *keyBlock,
                          uint16_t keyBlockSize)
{
  uint8_t *firstSalt;
  uint8_t *secondSalt;

  if (clientSaltFirst) {
    firstSalt = (uint8_t *) tls->handshake.clientSalt;
    secondSalt = (uint8_t *) tls->handshake.serverSalt;
  } else {
    firstSalt = (uint8_t *) tls->handshake.serverSalt;
    secondSalt = (uint8_t *) tls->handshake.clientSalt;
  }

  prfHash(tls->session.master, TLS_MASTER_SECRET_SIZE,
          label,
          firstSalt, SALT_SIZE,
          secondSalt, SALT_SIZE,
          keyBlock,
          keyBlockSize);
}

EmberStatus emDeriveKeys(TlsState *tls)
{
  TlsHandshakeState *handshake = &tls->handshake;

  if (!tlsIsResume(tls)) {
    uint8_t premasterSecret[256];           // 2048 bits -> 256 bytes
    uint16_t premasterSecretLength = 0;

    if ((haveEcc
         && tls->connection.flags & TLS_HAVE_ECDHE_ECDSA)
        || (haveRsa
            && tls->connection.flags & TLS_HAVE_DHE_RSA)) {
      premasterSecretLength =
        emDeriveDhePremasterSecret(handshake, premasterSecret);
      if (premasterSecretLength == 0) {
        lose(SECURITY, EMBER_ERR_FATAL);
      }
    } else if (tls->connection.flags & TLS_HAVE_PSK) {
      // The premaster secret is:
      //  <keylen:2> <0:keylen> <keylen:2> <key:keylen>
      // For DHE_PSK or RSA_PSK the block of zeros is replaced by a
      // block of bits from the DHE or RSA calculation.

      uint8_t *psk = NULL;
      uint8_t key[32];
      uint16_t pskLength = 0;
      if (emMySharedKey != NULL) {
        pskLength = emMySharedKey->keyLength;
        psk = (uint8_t *) emMySharedKey->key;
      } else if (tlsUsingDtls(tls)) {
        DtlsConnection *connection = ((DtlsConnection *)
                                      emFindConnectionFromHandle(tls->connection.fd));
        pskLength = emGetPsk(key, connection->udpData.remoteAddress);
        psk = key;
      }

      uint8_t *finger = premasterSecret;
      uint8_t i;

      for (i = 0; i < 2; i++) {
        *finger++ = HIGH_BYTE(pskLength);
        *finger++ = LOW_BYTE(pskLength);
        if (i == 0) {
          MEMSET(finger, 0, pskLength);
        } else {
          MEMCOPY(finger, psk, pskLength);
        }
        finger += pskLength;
      }

      premasterSecretLength = finger - premasterSecret;
#ifdef HAVE_TLS_JPAKE
    } else {
      if (!emJpakeEccFinish(premasterSecret,
                            sizeof(premasterSecret),
                            &premasterSecretLength)) {
        lose(SECURITY, EMBER_ERR_FATAL);
      }
#endif
    }

    // TODO: disable these
    emLogBytesLine(SECURITY2,
                   "pre master secret",
                   premasterSecret,
                   premasterSecretLength);
    emLogBytesLine(SECURITY2,
                   "client salt",
                   (uint8_t *)handshake->clientSalt,
                   2 * SALT_SIZE);
    prfHash(premasterSecret, premasterSecretLength,
            "master secret",
            (uint8_t *) handshake->clientSalt, 2 * SALT_SIZE,
            NULL, 0,
            tls->session.master, TLS_MASTER_SECRET_SIZE);
    emLogBytesLine(SECURITY,
                   "master secret",
                   tls->session.master,
                   TLS_MASTER_SECRET_SIZE);

    MEMSET(premasterSecret, 0, sizeof(premasterSecret));
  }

  {
    uint8_t keyBlockBuffer[2 * (SHA256_MAC_LENGTH
                                + AES_128_KEY_LENGTH
                                + AES_128_CCM_IV_LENGTH)];
    uint8_t *keyBlock = keyBlockBuffer;
    uint16_t encryptKey;
    uint16_t decryptKey;
    uint16_t outIv;
    uint16_t inIv;

    emComputeDerivedKeys(tls,
                         "key expansion",
                         false,
                         keyBlock,
                         sizeof(keyBlockBuffer));
    dump("key expansion", keyBlock, sizeof(keyBlockBuffer));
    MEMSET(handshake->clientSalt, 0, 2 * SALT_SIZE);

#ifdef HAVE_TLS_CBC
    // A MAC key is used with CBC and not with CCM.  On a PC we run CBC.
    if (pskSuiteIdentifier == TLS_PSK_WITH_AES_128_CBC_SHA256) {
      uint16_t outHashKey;
      uint16_t inHashKey;

      if (tlsAmClient(tls)) {
        outHashKey = 0;
        inHashKey  = SHA256_MAC_LENGTH;
      } else {
        inHashKey  = 0;
        outHashKey = SHA256_MAC_LENGTH;
      }

      MEMCOPY(tls->connection.outHashKey, keyBlock + outHashKey, SHA256_MAC_LENGTH);
      MEMCOPY(tls->connection.inHashKey, keyBlock + inHashKey, SHA256_MAC_LENGTH);

      keyBlock += 2 * SHA256_MAC_LENGTH;
    }
#endif
    if (tlsAmClient(tls)) {
      encryptKey = 0;
      decryptKey = AES_128_KEY_LENGTH;
      outIv      = AES_128_KEY_LENGTH * 2;
      inIv       = AES_128_KEY_LENGTH * 2 + AES_128_CCM_IV_LENGTH;
    } else {
      decryptKey = 0;
      encryptKey = AES_128_KEY_LENGTH;
      inIv       = AES_128_KEY_LENGTH * 2;
      outIv      = AES_128_KEY_LENGTH * 2 + AES_128_CCM_IV_LENGTH;
    }

    MEMCOPY(tls->connection.encryptKey, keyBlock + encryptKey, AES_128_KEY_LENGTH);
    MEMCOPY(tls->connection.decryptKey, keyBlock + decryptKey, AES_128_KEY_LENGTH);
    MEMCOPY(tls->connection.inIv, keyBlock + inIv, AES_128_CCM_IV_LENGTH);
    MEMCOPY(tls->connection.outIv, keyBlock + outIv, AES_128_CCM_IV_LENGTH);

    MEMSET(keyBlockBuffer, 0, sizeof(keyBlockBuffer));
  }

  return EMBER_SUCCESS;
}

// Compute a hash over the exchange so far.

void emPartialHandshakeHash(TlsState *tls, uint8_t *result)
{
  Sha256State hashState;

  MEMCOPY(&hashState, &tls->handshake.messageHash, sizeof(Sha256State));
  emSha256Finish(&hashState, result);
  MEMSET(&hashState, 0, sizeof(hashState));
}

void emHandshakeFinishedHash(TlsState *tls, uint8_t *result, bool fromClient)
{
  uint8_t hash[SHA256_MAC_LENGTH];

  emPartialHandshakeHash(tls, hash);
  dump("tls sha256 finish", hash, SHA256_MAC_LENGTH);

  prfHash(tls->session.master, TLS_MASTER_SECRET_SIZE,
          (fromClient
           ? "client finished"
           : "server finished"),
          hash, SHA256_MAC_LENGTH,
          NULL, 0,
          result, TLS_FINISHED_HASH_SIZE);
  dump("tls finish verify", result, TLS_FINISHED_HASH_SIZE);

  MEMSET(hash, 0, sizeof(hash));
}
