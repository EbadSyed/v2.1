/*
 * File: tls-handshake-crypto.h
 * Description: cryptographic utilites for the TLS handshake
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef TLS_HANDSHAKE_CRYPTO_H
#define TLS_HANDSHAKE_CRYPTO_H

void emComputeDerivedKeys(TlsState *tls,
                          char *label,
                          bool clientSaltFirst,
                          uint8_t *keyBlock,
                          uint16_t keyBlockSize);

EmberStatus emDeriveKeys(TlsState *tls);

void emPartialHandshakeHash(TlsState *tls, uint8_t *result);
void emHandshakeFinishedHash(TlsState *tls, uint8_t *result, bool fromClient);

#endif // TLS_HANDSHAKE_CRYPTO_H
