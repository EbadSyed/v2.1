/*
 * File: tls-public-key.h
 * Description: TLS message handlers for doing public key cryptography.
 * Author(s): Richard Kelsey
 *
 * Copyright 2011 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef TLS_PUBLIC_KEY_H
#define TLS_PUBLIC_KEY_H

EmberStatus emSendEcdhServerKeyExchange(TlsState *tls);
EmberStatus emSendCertificateRequest(TlsState *tls);
EmberStatus emProcessCertificateRequest(TlsState *tls,
                                        uint8_t *commandLoc,
                                        uint8_t *incoming,
                                        uint16_t length);
EmberStatus emSendCertificate(TlsState *tls);
EmberStatus emProcessCertificate(TlsState *tls,
                                 uint8_t *commandLoc,
                                 uint8_t *incoming,
                                 uint16_t length);
EmberStatus emProcessEcdhServerKeyExchange(TlsState *tls,
                                           uint8_t *incoming,
                                           uint16_t length);
EmberStatus emSendCertificateVerify(TlsState *tls);
EmberStatus emProcessCertificateVerify(TlsState *tls,
                                       uint8_t *commandLoc,
                                       uint8_t *incoming,
                                       uint16_t length);

uint16_t emSignatureLength(TlsState *tls);
uint8_t *emAddSignature(TlsState *tls,
                        uint8_t *finger,
                        const uint8_t *hash,
                        uint16_t hashLength);
uint16_t emDheRandomSize(TlsState *tls);
EmberStatus emWriteDheRandom(TlsState *tls, uint8_t **resultLoc);
bool emReadDheRandom(TlsState *tls, uint8_t **fingerLoc);

uint16_t emDeriveDhePremasterSecret(TlsHandshakeState *handshake,
                                    uint8_t *premasterSecret);
bool emReadBignum(BigInt *n, uint8_t **fingerLoc, uint8_t *end);
bool emWriteBignum(uint8_t **fingerLoc, BigInt *n);

// This must be defined by the application, which gets to choose its
// own Diffie-Hellman constants.
void emInitializeTestDhState(DhState *dhState);

EmberStatus emSendJpakeServerKeyExchange(TlsState *tls);
EmberStatus emProcessJpakeServerKeyExchange(TlsState *tls,
                                            uint8_t *incoming,
                                            uint16_t length);

#endif // TLS_PUBLIC_KEY_H
