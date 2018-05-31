/*
 * File: public-key-stubs.c
 * Description: stubs for building without public key
 *
 * Copyright 2015 Silicon Laboratories, Inc.                                *80*
 */

#include "core/ember-stack.h"
#include "tls.h"
#include "big-int.h"
#include "certificate.h"

EmberStatus emSendEcdhServerKeyExchange(TlsState *tls)
{
  return EMBER_ERR_FATAL;
}

EmberStatus emSendCertificateRequest(TlsState *tls)
{
  return EMBER_ERR_FATAL;
}

EmberStatus emProcessCertificateRequest(TlsState *tls,
                                        uint8_t *commandLoc,
                                        uint8_t *incoming,
                                        uint16_t length)
{
  return EMBER_ERR_FATAL;
}

EmberStatus emSendCertificate(TlsState *tls)
{
  return EMBER_ERR_FATAL;
}

EmberStatus emProcessCertificate(TlsState *tls,
                                 uint8_t *incoming,
                                 uint16_t length)
{
  return EMBER_ERR_FATAL;
}

EmberStatus emProcessEcdhServerKeyExchange(TlsState *tls,
                                           uint8_t *incoming,
                                           uint16_t length)
{
  return EMBER_ERR_FATAL;
}

EmberStatus emSendCertificateVerify(TlsState *tls)
{
  return EMBER_ERR_FATAL;
}

EmberStatus emProcessCertificateVerify(TlsState *tls,
                                       uint8_t *incoming,
                                       uint16_t length)
{
  return EMBER_ERR_FATAL;
}

uint16_t emDheRandomSize(TlsState *tls)
{
  return 0;
}

EmberStatus emWriteDheRandom(TlsState *tls, uint8_t **resultLoc)
{
  return EMBER_ERR_FATAL;
}

bool emReadDheRandom(TlsState *tls, uint8_t **fingerLoc)
{
  return false;
}

uint16_t emDeriveDhePremasterSecret(TlsHandshakeState *handshake,
                                    uint8_t *premasterSecret)
{
  return 0;
}

bool emReadBignum(BigInt *n, uint8_t **fingerLoc, uint8_t *end)
{
  return false;
}
