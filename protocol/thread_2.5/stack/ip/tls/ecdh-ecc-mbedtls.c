/*
 * File: ecdh-ecc-mbedtls.c
 * Description: Wrapper on top of the mbedtls implementation of the
 * key agreement protocol ECDH and the digital signature protocol ECDSA.
 * Author(s): Suvesh Pratapa
 *
 * Copyright 2017 by Silicon Laboratories. All rights reserved.             *80*
 */

#include "stack/core/ember-stack.h"
#include "stack/framework/buffer-malloc.h"
#include "tls.h"
#include "ecc.h"
#include "ecc-mbedtls-util.h"

#include "mbedtls/ecdh.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/platform.h"

//------------------------------------------------------------------------------
// These are not relevant to us.

bool emEccFastSign = false;
bool emEccFastVerify = false;

//------------------------------------------------------------------------------
// APIs.

// Compute ECDSA signature of a previously hashed message.
bool emEccSign(const uint8_t *hash,
               const EccPrivateKey *key,
               uint8_t *r,
               uint8_t *s)
{
  mbedtls_ecdsa_context ecdsaCtx;
  mbedtls_mpi rM, sM;

  mbedtls_ecdsa_init(&ecdsaCtx);
  mbedtls_mpi_init(&rM);
  mbedtls_mpi_lset(&rM, 0);
  mbedtls_mpi_init(&sM);
  mbedtls_mpi_lset(&sM, 0);

  mbedtls_ecp_group_load(&ecdsaCtx.grp, MBEDTLS_ECP_DP_SECP256R1);
  mbedtls_mpi_read_binary(&ecdsaCtx.d,
                          (const unsigned char *) key->secret,
                          ECC_COORDINATE_LENGTH);

  emSetWatchdog(false);
  int ret = mbedtls_ecdsa_sign(&ecdsaCtx.grp,     // ECP group
                               &rM,               // First output integer
                               &sM,               // Second output integer
                               &ecdsaCtx.d,       // Private signing key
                               hash,              // Message hash
                               SHA256_MAC_LENGTH, // Message hash length
                               emGetRandomBytes,  // RNG function
                               NULL);             // RNG parameter
  emSetWatchdog(true);

  if (ret == 0) {
    // Write output if successful.
    mbedtls_mpi_write_binary(&rM, r, mbedtls_mpi_size(&rM));
    mbedtls_mpi_write_binary(&sM, s, mbedtls_mpi_size(&sM));
  }

  mbedtls_mpi_free(&rM);
  mbedtls_mpi_free(&sM);
  mbedtls_ecdsa_free(&ecdsaCtx);

  return (ret == 0);
}

// Verify ECDSA signature of a previously hashed message.
bool emEccVerify(const uint8_t *hash,
                 EccPublicKey *key,
                 const uint8_t *r,
                 uint16_t rLength,
                 const uint8_t *s,
                 uint16_t sLength)
{
  mbedtls_ecdsa_context ecdsaCtx;
  mbedtls_mpi rM, sM;

  mbedtls_ecdsa_init(&ecdsaCtx);
  mbedtls_mpi_init(&rM);
  mbedtls_mpi_init(&sM);

  mbedtls_ecp_group_load(&ecdsaCtx.grp, MBEDTLS_ECP_DP_SECP256R1);
  mbedtls_ecp_point_read_binary(&ecdsaCtx.grp,
                                &ecdsaCtx.Q,
                                (const unsigned char *) key->key,
                                key->keyLength);
  mbedtls_mpi_read_binary(&rM, (const unsigned char *) r, rLength);
  mbedtls_mpi_read_binary(&sM, (const unsigned char *) s, sLength);

  emSetWatchdog(false);
  int ret = mbedtls_ecdsa_verify(&ecdsaCtx.grp,     // ECP group
                                 hash,              // Message hash
                                 SHA256_MAC_LENGTH, // Message hash length
                                 &ecdsaCtx.Q,       // Public key
                                 &rM,               // First input integer
                                 &sM);              // Second input integer
  emSetWatchdog(true);

  mbedtls_mpi_free(&rM);
  mbedtls_mpi_free(&sM);
  mbedtls_ecdsa_free(&ecdsaCtx);

  return (ret == 0);
}

// Generate a public / private key pair.
bool emGenerateEcdh(uint8_t **publicKeyLoc, uint8_t *secret)
{
  mbedtls_ecdh_context ecdhCtx;

  mbedtls_ecdh_init(&ecdhCtx);

  mbedtls_ecp_group_load(&ecdhCtx.grp, MBEDTLS_ECP_DP_SECP256R1);

  emSetWatchdog(false);
  int ret = mbedtls_ecdh_gen_public(&ecdhCtx.grp,     // ECP group
                                    &ecdhCtx.d,       // Private key
                                    &ecdhCtx.Q,       // Public key
                                    emGetRandomBytes, // RNG function
                                    NULL);            // RNG parameter
  emSetWatchdog(true);

  if (ret == 0) {
    size_t len;
    mbedtls_ecp_point_write_binary(&ecdhCtx.grp,
                                   &ecdhCtx.Q,
                                   MBEDTLS_ECP_PF_UNCOMPRESSED,
                                   &len,
                                   *publicKeyLoc,
                                   ECDH_OUR_PUBLIC_KEY_LENGTH);
    assert(len == ECDH_OUR_PUBLIC_KEY_LENGTH);
    *publicKeyLoc += len;
    mbedtls_mpi_write_binary(&ecdhCtx.d,
                             secret,
                             mbedtls_mpi_size(&ecdhCtx.d));
  }

  mbedtls_ecdh_free(&ecdhCtx);

  return (ret == 0);
}

// Compute shared secret.
bool emEcdhSharedSecret(const EccPublicKey *remotePublicKey,
                        const uint8_t *localSecret,
                        uint8_t *sharedSecret)
{
  mbedtls_ecdh_context ecdhCtx;

  mbedtls_ecdh_init(&ecdhCtx);

  mbedtls_ecp_group_load(&ecdhCtx.grp, MBEDTLS_ECP_DP_SECP256R1);
  mbedtls_ecp_point_read_binary(&ecdhCtx.grp,
                                &ecdhCtx.Qp,
                                (const unsigned char *) remotePublicKey->key,
                                remotePublicKey->keyLength);

  mbedtls_mpi_read_binary(&ecdhCtx.d,
                          (const unsigned char *) localSecret,
                          ECC_COORDINATE_LENGTH);

  emSetWatchdog(false);
  int ret = mbedtls_ecdh_compute_shared(&ecdhCtx.grp,     // ECP group
                                        &ecdhCtx.z,       // Dest. shared secret
                                        &ecdhCtx.Qp,      // Remote public key
                                        &ecdhCtx.d,       // Our private key
                                        emGetRandomBytes, // RNG function
                                        NULL);            // RNG parameter
  emSetWatchdog(true);

  if (ret == 0) {
    mbedtls_mpi_write_binary(&ecdhCtx.z,
                             sharedSecret,
                             mbedtls_mpi_size(&ecdhCtx.z));
  }

  mbedtls_ecdh_free(&ecdhCtx);

  return (ret == 0);
}
