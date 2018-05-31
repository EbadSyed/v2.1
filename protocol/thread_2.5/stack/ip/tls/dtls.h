/*
 * File: dtls.h
 * Description: DTLS
 * Author(s): Richard Kelsey
 *
 * Copyright 2012 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef SLTHREAD_DTLS_H
#define SLTHREAD_DTLS_H

// Data stored for each connection.
typedef struct {
  UdpConnectionData udpData;

  Buffer tlsState;
  Buffer outgoingData;          // chain of outgoing data buffers
  Buffer incomingData;          // chain of incoming application packets
  Buffer incomingTlsData;       // chain of incoming TLS packets

  uint8_t resendCount;            // number of retransmissions
} DtlsConnection;

// Allocate a buffer to hold a DTLS payload.

Buffer emAllocateDtlsBuffer(uint16_t payloadLength, uint8_t **payloadLoc);

// Encrypt an outgoing DTLS paylaod.

bool emDtlsEncryptOutput(Buffer payload);

// Remove the DTLS record fields from the payload and decrypt it.
// Returns the payload's new length.

uint16_t emDtlsDecryptInput(uint8_t *payload, uint16_t payloadLength);

void emOpenDtlsConnection(DtlsConnection *connection);
void emOpenDtlsServer(DtlsConnection *connection);
void emCloseDtlsConnection(DtlsConnection *connection);
void emDtlsStatusHandler(DtlsConnection *connection);
void emUdpSendDtlsRecord(EmberUdpConnectionHandle handle, Buffer tlsRecord);
void emSubmitDtlsPayload(DtlsConnection *connection, Buffer payload);

// Callback into the DTLS join code.
void emSubmitRelayedJoinPayload(DtlsConnection *connection, Buffer payload);

// Useful macro
#define connectionDtlsState(connection) \
  ((TlsState *) emGetBufferPointer((connection)->tlsState))

void emSetJpakeKey(DtlsConnection *connection,
                   const uint8_t *key,
                   uint8_t keyLength);

#endif // SLTHREAD_DTLS_H
