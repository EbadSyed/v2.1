/*
 * File: certificate.h
 * Description: X509 certificate parsing and verification
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef CERTIFICATE_H
#define CERTIFICATE_H

extern const CertificateAuthority *emMyAuthorities[];

bool emParseCertificate(const uint8_t *data,
                        uint16_t length,
                        const uint8_t *authorityKey,
                        CertificateAuthority *result);

#ifdef HAVE_TLS_ECDHE_ECDSA
bool emVerifyEccSignature(EccPublicKey *eccKey,
                          const uint8_t *signature,
                          uint16_t signatureLength,
                          const uint8_t *hashResult,
                          bool returnStatus);
#endif

#ifdef EMBER_TEST
// Utilities for converting keys from base64-encoded ASN.1 to
// C structures.

bool parseRsaPrivateKey(const uint8_t *data, uint16_t length, char *prefix);
bool parseEccPrivateKey(const uint8_t *data, uint16_t length, char *prefix, bool public);
#endif

#endif // CERTIFICATE_H
