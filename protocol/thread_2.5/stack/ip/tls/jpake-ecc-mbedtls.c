/*
 * File: jpake-ecc-mbedtls.c
 * Description: Wrapper on top of the mbedtls implementation of the
 * password-authenticated key exchange protocol J-PAKE.
 *
 * Copyright 2014 by Silicon Laboratories. All rights reserved.             *80*
 */

#include "stack/core/ember-stack.h"
#include "stack/framework/buffer-malloc.h"
#include "jpake-ecc.h"
#include "ecc-mbedtls-util.h"

#include "mbedtls/platform.h"
#include "mbedtls/ecjpake.h"

#ifdef EMBER_TEST
// This lets us set how long the JPAKE crypto operations take in the simulator.
#include "hal/hal.h"
uint32_t emJpakeDelayMs = 0;
void jpakeWait(void)
{
  uint32_t remainingMs = emJpakeDelayMs;
  uint32_t start = halCommonGetInt32uMillisecondTick();
  while (0 < remainingMs) {
    simulatedTimePassesUs(remainingMs * 1000);
    uint32_t end = halCommonGetInt32uMillisecondTick();
    remainingMs -= end - start;
    start = end;
  }
}
#else
#define jpakeWait()
#endif

//------------------------------------------------------------------------------
// Static variables and forward declarations.

static struct {
  uint8_t bitmask;
  mbedtls_ecjpake_context *ctx;
} jPakeData = { 0, NULL };

static void mbedtls_ecp_group_mark(mbedtls_ecp_group *group);
static void mbedtls_ecp_point_mark(mbedtls_ecp_point *point);
static void mbedtls_mpi_mark(mbedtls_mpi *X);

//------------------------------------------------------------------------------
// APIs.

void emJpakeEccStart(bool isClient,
                     const uint8_t *sharedSecret,
                     const uint8_t sharedSecretLength)
{
  if (jPakeData.bitmask & JPAKE_DATA_INITIALIZED) {
    mbedtls_ecjpake_free(jPakeData.ctx);
    jPakeData.bitmask = 0;
  }

  setRandomDataType(isClient);

  jPakeData.bitmask = JPAKE_DATA_INITIALIZED;
  jPakeData.ctx = emBufferMalloc(sizeof(mbedtls_ecjpake_context));
  mbedtls_ecjpake_init(jPakeData.ctx);
  mbedtls_ecjpake_setup(jPakeData.ctx,
                        isClient ? MBEDTLS_ECJPAKE_CLIENT : MBEDTLS_ECJPAKE_SERVER,
                        MBEDTLS_MD_SHA256,
                        MBEDTLS_ECP_DP_SECP256R1,
                        sharedSecret,
                        sharedSecretLength);
}

bool emJpakeEccFinish(uint8_t *pms,
                      uint16_t len,
                      uint16_t *olen)
{
  int ret;
  size_t written;
  bool retVal = false;

  emMicroBusy(true);

  if ((jPakeData.bitmask & JPAKE_DATA_INITIALIZED)
      && (jPakeData.bitmask & JPAKE_DATA_HX1_HX2_VALID)
      && (jPakeData.bitmask & JPAKE_DATA_HX3_HX4_VALID)
      && (jPakeData.bitmask & JPAKE_DATA_XCS_VALID)) {
    emSetWatchdog(false);
    ret = mbedtls_ecjpake_derive_secret(jPakeData.ctx, pms, len, &written, &emGetRandomBytes, NULL);
    jpakeWait();
    emSetWatchdog(true);
    retVal = (ret == 0);
    *olen = written;
  }

  if (jPakeData.bitmask & JPAKE_DATA_INITIALIZED) {
    mbedtls_ecjpake_free(jPakeData.ctx);
    jPakeData.bitmask = 0;
  }

  return retVal;
}

bool emJpakeEccGetHxaHxbData(uint8_t *buf,
                             uint16_t len,
                             uint16_t *olen)
{
  int ret;
  size_t written;

  if (!(jPakeData.bitmask & JPAKE_DATA_INITIALIZED)) {
    return false;
  }

  emMicroBusy(true);
  emSetWatchdog(false);
  ret = mbedtls_ecjpake_write_round_one(jPakeData.ctx, buf, len, &written, &emGetRandomBytes, NULL);
  jpakeWait();
  emSetWatchdog(true);
  *olen = written;

  if (ret == 0) {
    jPakeData.bitmask |= ((jPakeData.ctx->role == MBEDTLS_ECJPAKE_CLIENT)
                          ? JPAKE_DATA_HX1_HX2_VALID
                          : JPAKE_DATA_HX3_HX4_VALID);
  }

  return (ret == 0);
}

bool emJpakeEccVerifyHxaHxbData(uint8_t *buf,
                                uint16_t len)
{
  int ret;

  if (!(jPakeData.bitmask & JPAKE_DATA_INITIALIZED)) {
    return false;
  }

  emMicroBusy(true);
  emSetWatchdog(false);
  ret = mbedtls_ecjpake_read_round_one(jPakeData.ctx, buf, len);
  jpakeWait();
  emSetWatchdog(true);

  if (ret == 0) {
    jPakeData.bitmask |= ((jPakeData.ctx->role == MBEDTLS_ECJPAKE_SERVER)
                          ? JPAKE_DATA_HX1_HX2_VALID
                          : JPAKE_DATA_HX3_HX4_VALID);
  }

  return (ret == 0);
}

bool emJpakeEccGetCkxaOrSkxbData(uint8_t *buf,
                                 uint16_t len,
                                 uint16_t *olen)
{
  int ret;
  size_t written;

  if (!(jPakeData.bitmask & JPAKE_DATA_INITIALIZED)
      || !(jPakeData.bitmask & JPAKE_DATA_HX1_HX2_VALID)
      || !(jPakeData.bitmask & JPAKE_DATA_HX3_HX4_VALID)) {
    return false;
  }

  emMicroBusy(true);
  emSetWatchdog(false);
  ret = mbedtls_ecjpake_write_round_two(jPakeData.ctx, buf, len, &written, &emGetRandomBytes, NULL);
  jpakeWait();
  emSetWatchdog(true);
  *olen = written;

  return (ret == 0);
}

bool emJpakeEccVerifyCkxaOrSkxbData(uint8_t *buf,
                                    uint16_t len)
{
  int ret;

  if (!(jPakeData.bitmask & JPAKE_DATA_INITIALIZED)
      || !(jPakeData.bitmask & JPAKE_DATA_HX1_HX2_VALID)
      || !(jPakeData.bitmask & JPAKE_DATA_HX3_HX4_VALID)) {
    return false;
  }

  emMicroBusy(true);
  emSetWatchdog(false);
  ret = mbedtls_ecjpake_read_round_two(jPakeData.ctx, buf, len);
  jpakeWait();
  emSetWatchdog(true);

  if (ret == 0) {
    jPakeData.bitmask |= JPAKE_DATA_XCS_VALID;
  }

  return (ret == 0);
}

void emJpakeEccMarkData(void)
{
  if (jPakeData.bitmask & JPAKE_DATA_INITIALIZED) {
    mbedtls_ecp_group_mark(&jPakeData.ctx->grp);

    mbedtls_ecp_point_mark(&jPakeData.ctx->Xm1);
    mbedtls_ecp_point_mark(&jPakeData.ctx->Xm2);
    mbedtls_ecp_point_mark(&jPakeData.ctx->Xp1);
    mbedtls_ecp_point_mark(&jPakeData.ctx->Xp2);
    mbedtls_ecp_point_mark(&jPakeData.ctx->Xp);
    mbedtls_mpi_mark(&jPakeData.ctx->xm1);
    mbedtls_mpi_mark(&jPakeData.ctx->xm2);
    mbedtls_mpi_mark(&jPakeData.ctx->s);

    // mark jPakeData last
    emMarkBufferPointer((void **) &jPakeData.ctx);
  }
}

//------------------------------------------------------------------------------
// mbedtls callbacks

// mbedtls calls calloc so we must create a wrapper on top of emBufferMalloc
// that allocates and clears memory before returning the pointer to the caller.
void *emBufferCalloc(size_t count, size_t size)
{
  uint16_t totalSize = count * size;
  void *p = emBufferMalloc(totalSize);
  assert(p != NULL);
  MEMSET(p, 0, totalSize);
  return p;
}

//------------------------------------------------------------------------------
// Static functions

static void mbedtls_ecp_group_mark(mbedtls_ecp_group *group)
{
  size_t i;

  // From ecp.h when describing the h field of mbedtls_ecp_group
  //
  //    unsigned int h; internal: 1 if the constants are static
  //
  // Therefore the marking logic follows the same logic that
  // exists in mbedtls_ecp_group_free() and checks the value of
  // h before marking P, A, B, G and N.

  if ( group->h != 1 ) {
    mbedtls_mpi_mark(&group->P);
    mbedtls_mpi_mark(&group->A);
    mbedtls_mpi_mark(&group->B);
    mbedtls_ecp_point_mark(&group->G);
    mbedtls_mpi_mark(&group->N);
  }

  if ( group->T != NULL ) {
    for ( i = 0; i < group->T_size; i++ ) {
      mbedtls_ecp_point_mark(&group->T[i]);
    }
    emMarkBufferPointer((void **) &group->T);
  }
}

static void mbedtls_ecp_point_mark(mbedtls_ecp_point *point)
{
  mbedtls_mpi_mark(&point->X);
  mbedtls_mpi_mark(&point->Y);
  mbedtls_mpi_mark(&point->Z);
}

static void mbedtls_mpi_mark(mbedtls_mpi *X)
{
  if ( X->p != NULL ) {
    emMarkBufferPointer((void **) &X->p);
  }
}
