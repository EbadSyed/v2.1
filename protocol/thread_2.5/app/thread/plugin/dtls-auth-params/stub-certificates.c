/*
 * File: stub-certificates.c
 * Author(s): Suvesh Pratapa
 *
 * Copyright 2017 by Ember Corporation. All rights reserved.                *80*
 */

#include "stack/core/ember-stack.h"
#include "app/thread/plugin/dtls-auth-params/certificate-parameters.h"

//----------------------------------------------------------------
#ifdef EMBER_MBEDTLS_STACK

const char mbedtls_x509_ca_cert_der[] = { 0 };

const size_t mbedtls_x509_ca_cert_der_len = sizeof(mbedtls_x509_ca_cert_der);

//-------------------------------------------------------------------------

const char mbedtls_x509_server_cert_der[] = { 0 };

const size_t mbedtls_x509_server_cert_der_len = sizeof(mbedtls_x509_server_cert_der);

const char mbedtls_x509_server_key_der[] = { 0 };

const size_t mbedtls_x509_server_key_der_len = sizeof(mbedtls_x509_server_key_der);

//-------------------------------------------------------------------------

const char mbedtls_x509_client_cert_der[] = { 0 };

const size_t mbedtls_x509_client_cert_der_len = sizeof(mbedtls_x509_client_cert_der);

const char mbedtls_x509_client_key_der[] = { 0 };

const size_t mbedtls_x509_client_key_der_len = sizeof(mbedtls_x509_client_key_der);

//----------------------------------------------------------------

#else // Below is ! EMBER_MBEDTLS_STACK

#include "stack/ip/tls/tls.h"
#include "stack/ip/tls/certificate.h"

static const uint8_t newCaName[] = { 0 };

static const uint8_t newCaKey[] = { 0 };

const CertificateAuthority newAuthority = {
  newCaName,
  sizeof(newCaName),
  (uint8_t *)newCaKey,
  0xFF
};

const uint8_t newCertificate[] = { 0 };

const uint16_t newCertificateSize = sizeof(newCertificate);

const uint8_t newPrivate[] = { 0 };

//----------------------------------------------------------------
const CertificateAuthority *emMyAuthorities[] = { 0 };

const uint8_t *emMyPrivateKey;
const uint8_t *emMyRawCertificate;
uint16_t emMyRawCertificateLength;

uint8_t *emMyHostname = NULL;
uint16_t emMyHostnameLength = 0;

// No pre-shared key implementation on the application currently.
const SharedKey *emMySharedKey = NULL;
//----------------------------------------------------------------

#endif // EMBER_MBEDTLS_STACK
