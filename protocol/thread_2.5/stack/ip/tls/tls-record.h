/*
 * File: tls-record.h
 * Description: TLS record layer
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef TLS_RECORD_H
#define TLS_RECORD_H

#define TLS_RECORD_HEADER_LENGTH 5

// DTLS includes an eight-byte sequence number in the record header.
// The first two bytes are the "epoch", indicating the number of
// cipher state changes.  The rest are a counter that is reset when
// the epoch changes.

#define DTLS_SEQUENCE_NUMBER_LENGTH 8
#define DTLS_SEQUENCE_NUMBER_OFFSET 3

#define DTLS_RECORD_HEADER_LENGTH \
  (TLS_RECORD_HEADER_LENGTH + DTLS_SEQUENCE_NUMBER_LENGTH)

#define TLS_HANDSHAKE_HEADER_LENGTH 4

// DTLS adds:
//  16-bit message_seq
//  24-bit fragment_offset
//  24-bit fragment_length
// These come after the normal TLS handshake header.

#define DTLS_HANDSHAKE_HEADER_LENGTH 12

Buffer emAllocateTlsBuffer(TlsState *tls,
                           uint8_t messageType,
                           uint16_t contentLength,
                           uint8_t **contentLoc);
Buffer emAllocateTlsHeaderBuffer(TlsState *tls,
                                 uint8_t messageType,
                                 uint16_t contentLength,
                                 Buffer contents,
                                 uint8_t **contentLoc);
void emTruncateTlsStateBuffer(Buffer buffer);
Buffer emAllocateTlsHandshakeBuffer(TlsState *tls,
                                    uint8_t messageType,
                                    uint16_t contentLength,
                                    uint8_t **contentLoc);
EmberStatus emTlsSendBuffer(TlsState *tls, Buffer buffer);
EmberStatus emTlsSendBufferAndPayload(TlsState *tls,
                                      Buffer buffer,
                                      uint8_t *payload);
void emUpdateOutgoingFlight(DtlsState *dtls);
void emHashTlsHandshake(Sha256State *hashState,
                        Buffer buffer,
                        uint16_t headerLength,
                        uint16_t payloadLength);

#define emTlsSendCloseNotify(tls) \
  (emTlsSendAlarm((tls), TLS_ALERT_LEVEL_WARNING, TLS_ALERT_CLOSE_NOTIFY))

EmberStatus emTlsSendAlarm(TlsState *tls, uint8_t level, uint8_t alarm);

extern bool emRequestEapIdentity;

// Increment an eight-byte big-endian counter.
#define emIncrementCounter(counter) (emAddToCounter((counter), 1))
void emAddToCounter(uint8_t *counter, uint8_t delta);

// Returns x - y, considered as big-endian six-byte unsigned integers.
// The results are rounded to the interval [-1, 256].
int16_t emSubtractInt48uCounters(const uint8_t *x, const uint8_t *y);

#ifdef EMBER_TEST
void emSubtractInt48uCountersTest(void);
#endif

TlsStatus emTlsProcessIncoming(TlsState *tls,
                               uint8_t *record,
                               uint16_t recordLength,
                               uint16_t *finalRecordLengthLoc);

#endif // TLS_RECORD_H
