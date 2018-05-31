/*
 * File: tls-test-credentials.c
 * Description: credentials for the TLS test app
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#include "core/ember-stack.h"
#include "tls.h"
#include "certificate.h"
#include "credentials/test-certificate.h"

#ifdef HAVE_TLS_DHE_RSA
#include "rsa.h"
#endif

//----------------------------------------------------------------

const uint8_t sharedKeyName[] = { '0' };

const uint8_t sharedKey[] = {
  0xCF, 0xCE, 0xCD, 0xCC, 0xCB, 0xCA, 0xC9, 0xC8,
  0xC7, 0xC6, 0xC5, 0xC4, 0xC3, 0xC2, 0xC1, 0xC0
};

// Key used by the test server at idvm-infk-mattern04.inf.ethz.ch:5685
// const uint8_t sharedKeyName[] = "PSK_Identity";
// const uint8_t sharedKey[] = {
//   0x01, 0x02, 0x03, 0x04, 0x05, 0x06
// };

static const SharedKey theSharedKey = {
  sharedKeyName,
  sizeof(sharedKeyName),
  sharedKey,
  sizeof(sharedKey)
};

const SharedKey *emMySharedKey = &theSharedKey;

//----------------------------------------------------------------

const uint8_t *emMyPrivateKey;
const uint8_t *emMyRawCertificate;
uint16_t emMyRawCertificateLength;
uint8_t *emMyHostname = NULL;
uint16_t emMyHostnameLength = 0;

#ifdef HAVE_TLS_DHE_RSA

// Diffie-Hellman prime.  Copied from PolarSSL.
static const uint32_t dhP[] = {
  0xA979DABF, 0xFA8E77B0, 0x0B7F1200, 0xE8A700D6,
  0xB64BDD48, 0x0A40B1EA, 0xA60BB7F0, 0x01F76949,
  0x2C4B248C, 0xF489AA51, 0x01EB2800, 0xFFA1D0B6,
  0x64965300, 0xE1F96B05, 0xA6BCC3B4, 0xF6AC8E1D,
  0x8B1D0C1C, 0xFD16D2C4, 0x001CF5A0, 0xF2C6B176,
  0x1BCA7538, 0xF43DFB90, 0x4C26A400, 0x11CCEF78,
  0x321CFD00, 0x002C20D0, 0x83301270, 0x2CE4B44A,
  0x448B3F80, 0x103D883A, 0x94182000, 0xE4004C1F
};

static const uint32_t dhG[] = { 0x00000004 };

const RsaPrivateKey *emMyRsaKey;

const CertificateAuthority *emMyAuthorities[] = { &testAuthority, NULL };

void emInitializeTestDhState(DhState *dhState)
{
  dhState->primeModulus.sign = 1;
  dhState->primeModulus.length = sizeof(dhP) / 4;
  dhState->primeModulus.digits = (BigIntDigit *) dhP;
  dhState->generator.sign = 1;
  dhState->generator.length = sizeof(dhG) / 4;
  dhState->generator.digits = (BigIntDigit *) dhG;
}

#endif // HAVE_TLS_DHE_RSA
