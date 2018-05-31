/*
 * File: tls-handshake.h
 * Description: TLS handshake
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef TLS_HANDSHAKE_H
#define TLS_HANDSHAKE_H

void emTlsSetState(TlsState *tls, uint8_t state);
EmberStatus emTlsProcessInput(TlsState *tls, uint8_t **inputLoc, uint16_t length);
TlsStatus emReadAndProcessRecord(TlsState *tls, Buffer *incomingQueue);
TlsStatus emRunHandshake(TlsState *tls, uint8_t *incoming, uint16_t length);

enum {
  DTLS_IS_NEXT_FLIGHT,
  DTLS_MIGHT_BE_FINISHED_MESSAGE,
  DTLS_IS_ALERT,
  DTLS_IS_OTHER
};

uint8_t emIsNextDtlsFlight(DtlsState *tls, Buffer buffer);

enum {
  DTLS_PROCESS,
  DTLS_DROP,
  DTLS_SEND_VERIFY_REQUEST
};

enum {
  DTLS_NO_OPTIONS            = 0x00,
  DTLS_ALLOW_RESUME_OPTION   = 0x01,
  DTLS_REQUIRE_COOKIE_OPTION = 0x02
};

uint8_t emParseInitialDtlsPacket(uint8_t *packet,
                                 uint16_t packetLength,
                                 uint8_t options,
                                 bool expectHello,
                                 Buffer *requestResult);

#endif // TLS_HANDSHAKE_H
