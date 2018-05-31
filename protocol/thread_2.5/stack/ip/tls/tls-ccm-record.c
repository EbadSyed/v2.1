/*
 * File: tls-ccm-record.c
 * Description: TLS record layer using CCM for encryption and authentication.
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

// TLS is described in RFC 5246.

#include "core/ember-stack.h"
#include "mac/802.15.4/802-15-4-ccm.h"
#include "platform/micro/aes.h"
#include "ip/udp.h"
#include "ip/tcp.h"

#include "tls.h"
#include "dtls.h"
#include "tls-handshake.h"
#include "tls-sha256.h"
#include "debug.h"

#include "tls-record.h"

// CCM needs a counter for use in the nonce.  This is sent as a separate
// 8-byte value after the TLS header.  This counter is the same value as
// the seq_num that sender and receiver already know, but the IETF loonies
// insist that we have to include explicitly.  Eight more bytes of payload
// down the tubes.
//
// DTLS adds its own explicit sequence number, which could be used for the
// nonce as well, but isn't.  The DTLS sequence number has two bytes of
// "epoch" (which counts the number of change-cipher-spec messages) and
// six bytes of counter (which is set to zero by change-cipher-spec messages.
// The six-byte counter has the same value as the CCM counter.

#define CCM_EXPLICIT_NONCE_LENGTH 8

// ZigBee IP uses CCM_8
#define CCM_MIC_LENGTH 8

//----------------------------------------------------------------
// Forward declarations
static uint16_t counterDifference(TlsState *tls, uint8_t *record);
static void markMessageSeen(TlsState *tls, uint8_t *counter, uint16_t diff);

//----------------------------------------------------------------
// The CBC suites.

const uint16_t pskSuiteIdentifier = TLS_PSK_WITH_AES_128_CCM;
const uint16_t dheRsaSuiteIdentifier = TLS_DHE_RSA_WITH_AES_128_CCM;
const uint16_t ecdheEcdsaSuiteIdentifier = TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8;
const uint16_t jpakeSuiteIdentifier = TLS_ECJPAKE_WITH_AES_128_CCM_8;

//----------------------------------------------------------------

static uint8_t *writeTlsRecordHeader(TlsState *tls,
                                     uint8_t *finger,
                                     uint8_t messageType,
                                     uint16_t contentLength)
{
  *finger++ = messageType;
  if (tlsUsingDtls(tls)) {
    *finger++ = DTLS_1P2_MAJOR_VERSION;
    *finger++ = DTLS_1P2_MINOR_VERSION;
    finger += DTLS_SEQUENCE_NUMBER_LENGTH;
  } else {
    *finger++ = TLS_1P2_MAJOR_VERSION;
    *finger++ = TLS_1P2_MINOR_VERSION;
  }

  *finger++ = HIGH_BYTE(contentLength);
  *finger++ = LOW_BYTE(contentLength);

  if (tlsEncrypt(tls)) {
    finger += CCM_EXPLICIT_NONCE_LENGTH;
  }

  return finger;
}

static uint16_t txHeaderLength(TlsState *tls)
{
  uint16_t length = (tlsUsingDtls(tls)
                     ? DTLS_RECORD_HEADER_LENGTH
                     : TLS_RECORD_HEADER_LENGTH);
  if (tlsEncrypt(tls)) {
    return length + CCM_EXPLICIT_NONCE_LENGTH;
  } else {
    return length;
  }
}

static uint16_t rxHeaderLength(TlsState *tls)
{
  uint16_t length = (tlsUsingDtls(tls)
                     ? DTLS_RECORD_HEADER_LENGTH
                     : TLS_RECORD_HEADER_LENGTH);
  if (tlsDecrypt(tls)) {
    return length + CCM_EXPLICIT_NONCE_LENGTH;
  } else {
    return length;
  }
}

Buffer emAllocateTlsHeaderBuffer(TlsState *tls,
                                 uint8_t messageType,
                                 uint16_t contentLength,
                                 Buffer bufferSpace,
                                 uint8_t **contentLoc)
{
  uint16_t overhead = txHeaderLength(tls);
  Buffer buffer;
  uint8_t *finger;

  if (tlsEncrypt(tls)) {
    overhead += CCM_MIC_LENGTH;
  }

  buffer = emAllocateBuffer(overhead + bufferSpace);

  if (buffer == NULL_BUFFER) {
    return NULL_BUFFER;
  }

  finger = emGetBufferPointer(buffer);

  if (finger != NULL) {
    uint8_t *temp =
      writeTlsRecordHeader(tls, finger, messageType, contentLength);
    if (contentLoc != NULL) {
      *contentLoc = temp;
    }
  }

  return buffer;
}

#define NONCE_COUNTER_OFFSET 5

static void initializeNonce(uint8_t *nonce,
                            uint8_t *counter,
                            uint8_t *iv,
                            uint16_t length)
{
  nonce[0] = (STANDALONE_FLAGS_ADATA_FIELD_NONZERO
              | STANDALONE_FLAGS_M_FIELD_8_BYTES
              | STANDALONE_FLAGS_L_FIELD_3_BYTES);
  MEMCOPY(nonce + 1, iv, AES_128_CCM_IV_LENGTH);
  MEMCOPY(nonce + NONCE_COUNTER_OFFSET, counter, 8);
  nonce[13] = 0;
  nonce[14] = HIGH_BYTE(length);
  nonce[15] = LOW_BYTE(length);
}

// Hashing: <nonce:8> <header:3> <length:2>
// Message: <header:3> [<seq:8>] <length:2> <nonce:8> <contents:n> <mac:16>
// <seq:8> is only in DTLS packets.
//
// The payload and MIC may be in separate buffers connected through payload
// links.

static EmberStatus sendRecord(TlsState *tls,
                              Buffer buffer,
                              uint8_t* payload,
                              uint8_t* mic,
                              bool updateOnly)
{
  uint8_t* tlsHeader = emGetBufferPointer(buffer);
  uint16_t tlsHeaderLength = txHeaderLength(tls);
  uint8_t* lengthLoc = tlsHeader + tlsHeaderLength - 2;
  uint8_t* counter =
    (tlsUsingDtls(tls)
     && !tlsEncrypt(tls))
    ? ((DtlsState *) tls)->dtlsHandshake.epoch0OutRecordCounter
    : tls->connection.outRecordCounter;

  if (tlsEncrypt(tls)) {
    lengthLoc -=  CCM_EXPLICIT_NONCE_LENGTH;
  }

  uint16_t payloadLength = HIGH_LOW_TO_INT(lengthLoc[0], lengthLoc[1]);

  if (tls->connection.state == TLS_CLOSING) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  if (tlsHeader != NULL) {
    if (tlsHeader[0] == TLS_CONTENT_HANDSHAKE
        && !updateOnly) {
      emHashTlsHandshake(&tls->handshake.messageHash,
                         buffer,
                         tlsHeaderLength,
                         payloadLength);
    }
  }

  if (tlsEncrypt(tls)) {
    uint8_t nonce[16];
    uint8_t authData[CCM_EXPLICIT_NONCE_LENGTH + TLS_RECORD_HEADER_LENGTH];
    uint8_t *finger = authData;

    MEMCOPY(finger, counter, CCM_EXPLICIT_NONCE_LENGTH);
    finger += CCM_EXPLICIT_NONCE_LENGTH;
    MEMCOPY(finger, tlsHeader, 3);
    finger += 3;
    MEMCOPY(finger, lengthLoc, 2);

    initializeNonce(nonce,
                    counter,
                    tls->connection.outIv,
                    payloadLength);
    emLoadKeyIntoCore(tls->connection.encryptKey);

    emLogLine(SECURITY2, "** emCcmEncrypt");
    emLogBytesLine(SECURITY2, "key ", tls->connection.encryptKey, AES_128_KEY_LENGTH);
    emLogBytesLine(SECURITY2, "nonce ", nonce, SECURITY_BLOCK_SIZE);
    emLogBytesLine(SECURITY2, "authenticate ", authData, sizeof(authData));
    emLogBytesLine(SECURITY2, "payload ", payload, payloadLength);

    emCcmEncrypt(nonce,
                 authData,
                 sizeof(authData),
                 payload,
                 payloadLength,
                 mic,
                 CCM_MIC_LENGTH);

    // Nonce and MIC are now part of the payload.
    payloadLength += CCM_EXPLICIT_NONCE_LENGTH + CCM_MIC_LENGTH;
    lengthLoc[0] = HIGH_BYTE(payloadLength);
    lengthLoc[1] = LOW_BYTE(payloadLength);

    // Put counter after the header as the explicit nonce.
    MEMCOPY(tlsHeader + tlsHeaderLength - CCM_EXPLICIT_NONCE_LENGTH,
            counter,
            CCM_EXPLICIT_NONCE_LENGTH);
  }

  // For DTLS, also put the counter in the header.
  if (tlsUsingDtls(tls)) {
    MEMCOPY(tlsHeader + 3, counter, DTLS_SEQUENCE_NUMBER_LENGTH);
  }

  emIncrementCounter(counter);

  if (updateOnly) {
    // nothing more to do
  } else if (tlsUsingDtls(tls)) {
    emUdpSendDtlsRecord(tls->connection.fd, buffer);
  } else {
    emTcpSendTlsRecord(tls->connection.fd, buffer);
  }

  return EMBER_SUCCESS;
}

EmberStatus emTlsSendBuffer(TlsState *tls, Buffer buffer)
{
  uint8_t *contents = emGetBufferPointer(buffer);

  if (contents != NULL) {
    if (tlsEncrypt(tls)
        && tlsUsingDtls(tls)
        && tlsHasHandshake(tls)
        && (contents[0] == TLS_CONTENT_HANDSHAKE
            || tlsIsDtlsJoin(tls))) {
      Buffer copy = emFillBuffer(contents, emGetBufferLength(buffer));

      if (copy == NULL_BUFFER) {
        // what to do?
        return EMBER_NO_BUFFERS;
      }

      ((DtlsState *) tls)->dtlsHandshake.finishedMessage = copy;
    }
  }

  return sendRecord(tls,
                    buffer,
                    contents + txHeaderLength(tls),
                    contents + emGetBufferLength(buffer) - CCM_MIC_LENGTH,
                    false);
}

// Used for sending certificates, where the header and payload are in
// separate buffers.

EmberStatus emTlsSendBufferAndPayload(TlsState *tls,
                                      Buffer buffer,
                                      uint8_t *payload)
{
  return sendRecord(tls, buffer, payload, NULL, false);
}

EmberStatus emTlsSendApplicationBuffer(TcpConnection *connection,
                                       Buffer content)
{
  TlsState *tls = connectionTlsState(connection);
  uint16_t contentLength = emGetBufferLength(content);
  if (tls != NULL) {
    Buffer headerBuffer = emAllocateBuffer(txHeaderLength(tls));
    Buffer micBuffer = emAllocateBuffer(CCM_MIC_LENGTH);

    assert(tls->connection.state == TLS_HANDSHAKE_DONE);

    if (headerBuffer == NULL_BUFFER
        || micBuffer == NULL_BUFFER) {
      return NULL_BUFFER;
    }

    writeTlsRecordHeader(tls,
                         emGetBufferPointer(headerBuffer),
                         TLS_CONTENT_APPLICATION_DATA,
                         contentLength);
    emSetPayloadLink(headerBuffer, content);
    emSetPayloadLink(content, micBuffer);

    return sendRecord(tls,
                      headerBuffer,
                      emGetBufferPointer(content),
                      emGetBufferPointer(micBuffer),
                      false);
  }
  return NULL_BUFFER;
}

// Input: <header:3> [<seq:8>] <length:2> <counter:8> <contents:n> <mac:16>
// <seq:8> only if using DTLS.

TlsStatus emTlsProcessIncoming(TlsState *tls,
                               uint8_t *record,
                               uint16_t recordLength,
                               uint16_t *finalRecordLengthLoc)
{
  uint16_t tlsHeaderLength = rxHeaderLength(tls);
  uint8_t* lengthLoc = record + tlsHeaderLength - 2;

  if (tlsUsingDtls(tls)
      ? (record[1] != DTLS_1P2_MAJOR_VERSION
         || record[2] != DTLS_1P2_MINOR_VERSION)
      : (record[1] != TLS_1P2_MAJOR_VERSION
         || record[2] != TLS_1P2_MINOR_VERSION)) {
    return TLS_BAD_RECORD;
  }

  if (tlsDecrypt(tls)) {
    lengthLoc -=  CCM_EXPLICIT_NONCE_LENGTH;
  }

  uint16_t payloadLength = HIGH_LOW_TO_INT(lengthLoc[0], lengthLoc[1]);
  uint16_t difference = (tlsUsingDtls(tls)
                         ? counterDifference(tls, record)
                         : 0);

  if (difference == 0xFFFF) {
    // message is old or has already been seen
    lose(SECURITY, TLS_DTLS_DUPLICATE);
  }

  if (!tlsDecrypt(tls)
      // BUG:
      // This code handles the case where an already connected devices tries
      // to reconnect.  Letting this through without more support for this
      // case elsewhere may be a security hole.  Allowed for a Pelican
      // development drop.
      || (record[0] == TLS_CONTENT_HANDSHAKE
          && (record[tlsUsingDtls(tls)
                     ? DTLS_RECORD_HEADER_LENGTH
                     : TLS_RECORD_HEADER_LENGTH]
              == TLS_HANDSHAKE_CLIENT_HELLO))) {
    if (payloadLength == 0
        || TLS_MAX_RECORD_CONTENT_LENGTH < payloadLength) {
      lose(SECURITY, TLS_BAD_RECORD);
    }
  } else {
    uint8_t *explicitNonce = record + tlsHeaderLength - CCM_EXPLICIT_NONCE_LENGTH;
    uint8_t nonce[16];
    uint8_t authData[CCM_EXPLICIT_NONCE_LENGTH + TLS_RECORD_HEADER_LENGTH];
    uint8_t *finger = authData;

    if (payloadLength < TLS_CCM_MIN_ENCRYPT_LENGTH
        || (TLS_CCM_MIN_ENCRYPT_LENGTH + TLS_MAX_RECORD_CONTENT_LENGTH + 256
            < payloadLength)) {
      lose(SECURITY, TLS_BAD_RECORD);
    }

    // Check that the explicitNonce and the sequence number match.  For
    // DTLS the sequence number is a second copy (why?) of the same number
    // in the same packet.
    if (MEMCOMPARE(explicitNonce,
                   (tlsUsingDtls(tls)
                    ? record + 3
                    : tls->connection.inRecordCounter),
                   DTLS_SEQUENCE_NUMBER_LENGTH) != 0) {
      dump("found", explicitNonce, CCM_EXPLICIT_NONCE_LENGTH);
      dump("expected ",
           (tlsUsingDtls(tls)
            ? record + 3
            : tls->connection.inRecordCounter),
           CCM_EXPLICIT_NONCE_LENGTH);
      lose(SECURITY, TLS_BAD_RECORD);
    }

    // Remove the explicit nonce and MIC from the packet length.
    payloadLength -= CCM_EXPLICIT_NONCE_LENGTH + CCM_MIC_LENGTH;
    lengthLoc[0] = HIGH_BYTE(payloadLength);
    lengthLoc[1] = LOW_BYTE(payloadLength);
    recordLength = tlsHeaderLength + payloadLength;

    MEMCOPY(finger, explicitNonce, CCM_EXPLICIT_NONCE_LENGTH);
    finger += CCM_EXPLICIT_NONCE_LENGTH;
    MEMCOPY(finger, record, 3);
    finger += 3;
    MEMCOPY(finger, lengthLoc, 2);

    initializeNonce(nonce, explicitNonce, tls->connection.inIv, payloadLength);
    emLoadKeyIntoCore(tls->connection.decryptKey);

    emLogLine(SECURITY2, "** emCcmDecrypt");
    emLogBytesLine(SECURITY2, "key ", tls->connection.decryptKey, AES_128_KEY_LENGTH);
    emLogBytesLine(SECURITY2, "nonce ", nonce, SECURITY_BLOCK_SIZE);
    emLogBytesLine(SECURITY2, "authenticate ", authData, sizeof(authData));
    emLogBytesLine(SECURITY2, "payload ", record + tlsHeaderLength, payloadLength);

    if (!emCcmDecryptPacket(nonce,
                            authData,
                            sizeof(authData),
                            record + tlsHeaderLength,
                            payloadLength,
                            CCM_MIC_LENGTH)) {
      lose(SECURITY, TLS_DECRYPTION_FAILURE);
    }

    // For incoming packets we remove the explicit nonce to make
    // processing simpler.  It isn't clear this is worth the extra
    // effort.
    MEMMOVE(explicitNonce,
            explicitNonce + CCM_EXPLICIT_NONCE_LENGTH,
            payloadLength);
    recordLength -= CCM_EXPLICIT_NONCE_LENGTH;

    if (!tlsUsingDtls(tls)) {
      // For TLS only secured messages have a counter and messages arrive
      // in order.
      emIncrementCounter(tls->connection.inRecordCounter);
    }
  }

  if (tlsUsingDtls(tls)) {
    // For DTLS all messages have counters and may arrive out of order.
    markMessageSeen(tls,
                    tls->connection.inRecordCounter,
                    difference);
  }

  *finalRecordLengthLoc = recordLength;
  return TLS_SUCCESS;
}

void emUpdateOutgoingFlight(DtlsState *dtls)
{
  Buffer flight = dtls->dtlsHandshake.outgoingFlight;
  for (; flight != NULL_BUFFER; flight = emGetPayloadLink(flight)) {
    uint8_t* tlsHeader = emGetBufferPointer(flight);
    uint16_t remaining = emGetBufferLength(flight);
    if (tlsHeader != NULL) {
      do {                // loop through the records in the buffer
        if (tlsHeader[4] == 0) {
          MEMMOVE(tlsHeader + 3,
                  dtls->dtlsHandshake.epoch0OutRecordCounter,
                  DTLS_SEQUENCE_NUMBER_LENGTH);
          emIncrementCounter(dtls->dtlsHandshake.epoch0OutRecordCounter);
          uint16_t recordLength =
            DTLS_RECORD_HEADER_LENGTH
            + HIGH_LOW_TO_INT(tlsHeader[DTLS_RECORD_HEADER_LENGTH - 2],
                              tlsHeader[DTLS_RECORD_HEADER_LENGTH - 1]);
          // Take care of records that span multiple buffers.
          while (remaining < recordLength) {
            recordLength -= remaining;
            flight = emGetPayloadLink(flight);
            remaining = emGetBufferLength(flight);
          }
          remaining -= recordLength;
        } else {
          // Flight is the finished message, which must be re-encrypted using the
          // unecrypted copy that was saved earlier.
          uint8_t *contents = emGetBufferPointer(flight);
          assert(tlsHeader == contents);
          Buffer savedCopy = dtls->dtlsHandshake.finishedMessage;
          if (savedCopy != NULL_BUFFER) {
            uint16_t length = emGetBufferLength(flight);
            contents = emGetBufferPointer(flight);
            assert(length == emGetBufferLength(savedCopy));
            MEMMOVE(contents, emGetBufferPointer(savedCopy), length);
            sendRecord((TlsState *) dtls,
                       flight,
                       contents + txHeaderLength((TlsState *) dtls),
                       contents + length - CCM_MIC_LENGTH,
                       true);
            flight = emGetPayloadLink(flight);
          }
          return;
        }
      } while (0 < remaining);
    }
  }
}

// With k = incomingCounter - inRecordCounter, this returns
//  0 if the message is a duplicate
//  k if k < 64
//  64 if 64 <= k
// Bit 1<<k of inCounterMask is set if message inRecordCounter+k has been
// received.

static uint16_t counterDifference(TlsState *tls, uint8_t *record)
{
  uint8_t *received = record + DTLS_SEQUENCE_NUMBER_OFFSET;
  uint8_t *have = tls->connection.inRecordCounter;
  int16_t diff = emSubtractInt48uCounters(received + 2, have + 2);

  // dump("received ", received, 8);
  // dump("have ", have, 8);
  // fprintf(stderr, "[diff %d]\n", diff);

  if (received[0] != have[0]
      || received[1] != have[1]) {
    // epoch is wrong, ignore the message
    return 0xFFFF;
  } else if (32 < diff) {
    // message is does not fit in bitmask
    return 32;
  } else if (diff < 0
             || ((1 << diff) & tls->connection.inCounterMask)) {
    // message is too old or has already been seen
    return 0xFFFF;
  } else {
    return diff;
  }
}

// 'diff' is the value returned by counterDifference() for this message.
// If 'diff' is 32 then the inCounterMask is discarded and we set
// inRecordCounter to be the counter from the current message.
// Otherwise we shift inCounterMask enough so that the message's bit
// fits in and set the bit.

static void markMessageSeen(TlsState *tls, uint8_t *counter, uint16_t diff)
{
  uint16_t increment = 0;         // how much to add to inRecordCounter

  // fprintf(stderr, "[diff %d mask %08X]\n",
  //         diff,
  //         tls->connection.inCounterMask);

  if (diff < 32) {
    tls->connection.inCounterMask |= (1 << diff);
    while (tls->connection.inCounterMask & 1) {
      tls->connection.inCounterMask >>= 1;
      increment += 1;
    }
  } else {
    MEMMOVE(tls->connection.inRecordCounter,
            counter,
            DTLS_SEQUENCE_NUMBER_LENGTH);
    tls->connection.inCounterMask = 0;
    increment = 1;
  }

  if (increment != 0) {
    emAddToCounter(tls->connection.inRecordCounter, increment);
  }

  // fprintf(stderr, "[diff %d mask %08X increment %d]\n",
  //         diff,
  //         tls->connection.inCounterMask,
  //         increment);
}
