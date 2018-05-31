/*
 * File: tls-record-util.c
 * Description: Common code for the CBC and CCM record variants.
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#include "core/ember-stack.h"
#include "mac/802.15.4/802-15-4-ccm.h"
#include "ip/udp.h"
#include "ip/tcp.h"

#include "tls.h"
#include "tls-sha256.h"
#include "debug.h"

#include "tls-record.h"

//----------------------------------------------------------------

EmberStatus emTlsSendBufferedApplicationData(TcpConnection *connection,
                                             const uint8_t *data,
                                             uint16_t length,
                                             Buffer moreData,
                                             uint16_t moreLength)
{
  TlsState *tls = connectionTlsState(connection);
  uint8_t *finger = NULL;
  Buffer buffer = NULL_BUFFER;
  if (tls != NULL) {
    buffer = emAllocateTlsBuffer(tls,
                                 TLS_CONTENT_APPLICATION_DATA,
                                 length + moreLength,
                                 &finger);

    if (buffer == NULL_BUFFER) {
      lose(SECURITY, EMBER_NO_BUFFERS);
    }

    assert(tls->connection.state == TLS_HANDSHAKE_DONE);

    MEMCOPY(finger, data, length);
    emCopyFromLinkedBuffers(finger + length,
                            moreData,
                            moreLength);

    return emTlsSendBuffer(tls, buffer);
  }
  return EMBER_ERR_FATAL;
}

EmberStatus emTlsSendAlarm(TlsState *tls, uint8_t level, uint8_t alarm)
{
  uint8_t *finger;
  Buffer buffer = emAllocateTlsBuffer(tls, TLS_CONTENT_ALERT, 2, &finger);
  EmberStatus status;

  if (buffer == NULL_BUFFER) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  *finger++ = level;
  *finger++ = alarm;

  status = emTlsSendBuffer(tls, buffer);
  tls->connection.state = TLS_CLOSING;
  // Tell application EMBER_TCP_CLOSED - tricky, because we aren't
  // actually closing it at this point.
  return status;
}

// Incrementing 0xFF -> 0x00 causes a carry into the next byte.  Stop as
// soon as there is no carry.  I didn't come up with this myself.
void emAddToCounter(uint8_t *counter, uint8_t delta)
{
  uint16_t x = counter[7] + delta;
  counter[7] = x;       // store low byte

  if (0xFF < x) {       // if carry out, propagate carry upwards
    int8_t i;
    for (i = 6; 0 <= i; i--) {
      if (++counter[i] != 0) {
        break;
      }
    }
  }
}

// Returns x - y, considered as big-endian six-byte unsigned integers.
// The results are rounded to the interval [-1, 256].  The difference
// is used to index into a bitmask of already-seen messages, so the
// result doesn't need to be larger than the bitmask size.
//
// Simpler would be to use uint64_ts, but they are only available on
// Unix at the moment.

int16_t emSubtractInt48uCounters(const uint8_t *x, const uint8_t *y)
{
  uint16_t i;

  for (i = 0; i < 6; i++) {
    int16_t delta = x[i] - y[i];
    if (delta < 0) {
      return -1;                // x < y
    } else if (0 < delta) {
      return (i == 5
              ? delta           // x > y in least-significant byte
              : 256);           // x > y in more significant byte
    }
  }
  return 0;                     // x == y
}

#ifdef EMBER_TEST

// There is no unit test that runs this.

void emSubtractInt48uCountersTest(void)
{
  uint8_t x[6] = { 3, 7, 2, 8, 1, 0 };
  uint8_t y[6] = { 3, 7, 2, 8, 1, 0 };
  uint16_t i;
  uint16_t j;

  for (i = 0; i < 256; i++) {
    for (j = 0; j <= i; j++) {
      x[5] = i;
      y[5] = j;
      assert(emSubtractInt48uCounters(x, y) == i - j);
      assert(emSubtractInt48uCounters(y, x) == (i == j
                                                ? 0
                                                : -1));
      x[i % 5] = 10;
      assert(emSubtractInt48uCounters(x, y) == 256);
      assert(emSubtractInt48uCounters(y, x) == -1);
      x[i % 5] = y[i % 5];
    }
  }
}

#endif

Buffer emAllocateTlsBuffer(TlsState *tls,
                           uint8_t messageType,
                           uint16_t contentLength,
                           uint8_t **contentLoc)
{
  return emAllocateTlsHeaderBuffer(tls,
                                   messageType,
                                   contentLength,
                                   contentLength,
                                   contentLoc);
}

Buffer emAllocateTlsHandshakeBuffer(TlsState *tls,
                                    uint8_t messageType,
                                    uint16_t contentLength,
                                    uint8_t **contentLoc)
{
  uint8_t *content;
  uint16_t headerLength = (tlsUsingDtls(tls)
                           ? DTLS_HANDSHAKE_HEADER_LENGTH
                           : TLS_HANDSHAKE_HEADER_LENGTH);
  Buffer buffer = emAllocateTlsHeaderBuffer(tls,
                                            TLS_CONTENT_HANDSHAKE,
                                            headerLength + contentLength,
                                            (headerLength
                                             + (contentLoc == NULL
                                                ? 0
                                                : contentLength)),
                                            &content);

  if (buffer == NULL_BUFFER) {
    return NULL_BUFFER;
  }

  MEMSET(content, 0, headerLength);

  *content++ = messageType;
  content += 1;                 // unused
  *content++ = HIGH_BYTE(contentLength);
  *content++ = LOW_BYTE(contentLength);

  if (tlsUsingDtls(tls)) {
    DtlsState *dtls = (DtlsState *) tls;
    *content++ = 0;             // high byte of sequence
    *content++ = dtls->dtlsHandshake.outHandshakeCounter++;
    content += 3;               // fragment offset
    content += 1;               // highest byte of fragment length
    *content++ = HIGH_BYTE(contentLength);
    *content++ = LOW_BYTE(contentLength);
  }

  if (contentLoc != NULL) {
    *contentLoc = content;
  }

  emLogLine(SECURITY, "sending %s", tlsHandshakeName(messageType));
  return buffer;
}

void emHashTlsHandshake(Sha256State *hashState,
                        Buffer buffer,
                        uint16_t headerLength,
                        uint16_t payloadLength)
{
  uint16_t i = headerLength;
  while (0 < payloadLength) {
    uint16_t count = emGetBufferLength(buffer) - i;
    if (payloadLength < count) {
      count = payloadLength;
    }
    emSha256HashBytes(hashState, emGetBufferPointer(buffer) + i, count);
    buffer = emGetPayloadLink(buffer);
    payloadLength -= count;
    i = 0;
  }
}

Buffer emAllocateTlsState(uint8_t fd, uint16_t flags)
{
  TlsState *tls;
  uint16_t size = ((flags & TLS_USING_DTLS)
                   ? sizeof(DtlsState)
                   : sizeof(TlsState));
  Buffer buffer = emAllocateBuffer(size);

  if (buffer == NULL_BUFFER) {
    return NULL_BUFFER;
  }

  tls = (TlsState *) emGetBufferPointer(buffer);

  //uint32_t available = TLS_AVAILABLE_SUITES;

  if (tls != NULL) {
    MEMSET(tls, 0, size);
    tls->connection.state = TLS_UNINITIALIZED;
    tls->connection.fd = fd;
    tls->connection.flags =
      // Remove undefined flags and unavailable suites
      ((flags & (TLS_USING_DTLS
                 | TLS_IS_DTLS_JOIN
                 | TLS_NATIVE_COMMISSION
                 | TLS_AVAILABLE_SUITES))
       | TLS_HAS_HANDSHAKE);
    if (flags & TLS_USING_DTLS) {
      tls->connection.flags |= TLS_IN_DTLS_HANDSHAKE;
    }
    emSha256Start(&(tls->handshake.messageHash));
  }

  return buffer;
}

// Remove the handshake data once we are done with it.

void emTruncateTlsStateBuffer(Buffer buffer)
{
  TlsState *tls = (TlsState *) emGetBufferPointer(buffer);
  if (tls != NULL) {
    assert(tls->connection.state == TLS_HANDSHAKE_DONE);
    tlsClearFlag(tls, TLS_HAS_HANDSHAKE);
    emSetBufferLength(buffer,
                      sizeof(TlsConnectionState) + sizeof(TlsSessionState));
  }
}

#define markPointer(pointer) emMarkBufferPointer((void **) &(pointer))

void emMarkTlsState(Buffer *tlsBufferLoc)
{
  TlsState *tls;

  if (*tlsBufferLoc == NULL_BUFFER) {
    return;
  }

  bufferUse("TLS");

  tls = (TlsState *) emGetBufferPointer(*tlsBufferLoc);

  if (tls != NULL) {
    if (tlsUsingDtls(tls)
        && tlsHasHandshake(tls)) {
      emMarkBuffer(&(((DtlsState *) tls)->dtlsHandshake.outgoingFlight));
      emMarkBuffer(&(((DtlsState *) tls)->dtlsHandshake.finishedMessage));
    }
  }

#ifdef HAVE_TLS_DHE_RSA

  markPointer(tls->handshake.peerPublicKey.modulus.digits);
  markPointer(tls->handshake.peerPublicKey.publicExponent.digits);
  markPointer(tls->handshake.peerPublicKey.rRModN.digits);

  markPointer(tls->handshake.dhState.primeModulus.digits);
  markPointer(tls->handshake.dhState.generator.digits);
  markPointer(tls->handshake.dhState.localRandom.digits);
  markPointer(tls->handshake.dhState.remoteRandom.digits);
  markPointer(tls->handshake.dhState.rRModP.digits);

#endif

  emMarkBuffer(tlsBufferLoc);           // marking must come after use
  endBufferUse();
}
