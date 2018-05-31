/*
 * File: mbedtls-host-stubs.c
 * Description: stubs needed for running mbedtls code on host.
 * Author(s): Matteo Paris
 *
 * Copyright 2017 by Silicon Labs. All rights reserved.                *80*
 */

#ifdef EMBER_MBEDTLS_STACK

#include "stack/core/ember-stack.h"
#include "stack/ip/commission.h"
#include "stack/ip/commission-dataset.h"

#include MBEDTLS_CONFIG_FILE
#include "mbedtls/entropy.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_cookie.h"
#include "mbedtls/net.h"
#include "mbedtls/timing.h"

#ifdef MBEDTLS_CTR_DRBG_C
  #include "mbedtls/ctr_drbg.h"
#endif
#ifdef MBEDTLS_DEBUG_C
  #include "mbedtls/debug.h"
#endif
#ifdef MBEDTLS_ERROR_C
  #include "mbedtls/error.h"
#endif
#include "stack/ip/tls/mbedtls/mbedtls.h"
#include "stack/ip/tls/mbedtls/mbedtls-stack.h"

void emCommissioningHandshakeComplete(void)
{
  assert(false);
}

void emForwardToJoiner(const uint8_t *joinerIid,
                       uint16_t joinerPort,
                       EmberNodeId joinerRouterNodeId,
                       const uint8_t *kek,
                       Buffer payload)
{
  assert(false);
}

void emGetCommissionKey(uint8_t *key)
{
  assert(false);
}

uint8_t *emGetCommissioningMacKey(const uint8_t *senderEui64)
{
  assert(false);
  return NULL;
}

uint8_t emGetJoinKey(uint8_t *joinKey, const EmberEui64 *eui64)
{
  assert(false);
  return 0;
}

void emHandleSecureDtlsMessage(const uint8_t *remoteAddress,
                               const uint8_t *localAddress,
                               uint16_t remotePort,
                               uint16_t localPort,
                               EmberUdpConnectionHandle connectionHandle,
                               uint8_t *packet,
                               uint16_t length,
                               Buffer buffer)
{
  assert(false);
}

uint8_t emParentConnectionHandle;

void emSetCommissioningMacKey(uint8_t *key)
{
  assert(false);
}

uint16_t emUdpCommissionPort;
uint16_t emUdpJoinSrcPort;
uint16_t emUdpJoinPort;

void mbedtlsSendJoinerEntrust(const uint8_t *remoteAddress, const uint8_t *key)
{
  assert(false);
}

void mbedtlsSubmitRelayedPayload(uint8_t flags,
                                 const uint8_t* joinerIid,
                                 uint16_t joinerPort,
                                 uint16_t joinerRouterId,
                                 Buffer payload)
{
  assert(false);
}

void mbedtlsSetJpakeKey(uint8_t sessionId,
                        const uint8_t *key,
                        uint8_t keyLength)
{
  assert(false);
}

void mbedtlsDeriveCommissioningMacKey(void *context,
                                      const uint8_t *masterSecret,
                                      const uint8_t *keyBlock,
                                      size_t macLength,
                                      size_t keyLength,
                                      size_t ivLength)
{
  assert(false);
}

const CommissioningDataset *emGetActiveDataset(void)
{
  return NULL;
}

#endif // EMBER_MBEDTLS_STACK
