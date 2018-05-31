/*
 * File: tls-handshake.c
 * Description: TLS handshake
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

// TLS 1.2 is described in RFC 5246.

// This does not support renegotiation (sending a CLIENT_HELLO in an
// existing connection).  Besides the added complexity, renegotiation
// has a vulnerability, the fix for which (RFC 5746) would consume RAM
// and add yet more complexity.

#include "core/ember-stack.h"
#include "phy/phy.h"
#include "ip/udp.h"
#include "ip/tcp.h"

#include "tls.h"
#include "dtls.h"
#include "big-int.h"

#include "tls-handshake-crypto.h"
#include "tls-public-key.h"
#include "tls-record.h"
#include "tls-session-state.h"
#include "certificate.h"
#include "tls-sha256.h"
#include "debug.h"

#ifdef EMBER_TEST
  #include <netinet/in.h>
  #include "native-test-util.h"
#endif

#include "tls-handshake.h"
#include "jpake-ecc.h"

#if 0
// For testing the full DTLS handshake against the Thread test doc.

static const uint8_t threadTestClientHelloRandom[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
};

static const uint8_t threadTestServerHelloRandom[] = {
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
  0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

static const uint8_t threadTestServerHelloSessionId[] = {
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
  0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
  0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F
};
#endif

// It would be more efficient to put multiple messages into single records
// and/or multiple records into a single buffer.
// The multiple handshake messages are smaller and could easily be put in
// one record.
//
//  TLS_SERVER_SEND_CERTIFICATE_REQUEST,
//  TLS_SERVER_SEND_HELLO_DONE,
//
//  TLS_CLIENT_SEND_CERTIFICATE_VERIFY,
//  TLS_CLIENT_SEND_CHANGE_CIPHER_SPEC,
//
// The other messages are larger, I think, and it might not make
// sense to double them up.

//----------------------------------------------------------------
// Forward declarations.

static EmberStatus sendClientKeyExchange(TlsState *tls);
static EmberStatus processHelloDone(TlsState *tls,
                                    uint8_t *commandLoc,
                                    uint8_t *incoming,
                                    uint16_t length);
static TlsStatus processRecord(TlsState *tls, uint8_t *incoming, uint16_t length);

//----------------------------------------------------------------
// Alerts and shutting down.
//
// A non-error closing is done using a CLOSE_NOTIFY alert.  No data
// may be sent after sending a CLOSE_NOTIFY.  When a CLOSE_NOTIFY
// is received, any pending writes must be discarded (i.e. data not
// yet handed to TCP) and a CLOSE_NOTIFY sent in return.
//
// There are various situations in which we are required to respond
// with a fatal error alert and then close down the connection.
// When a fatal error alert is received the connection is shut down
// immediately.  No alert is sent back.  As far as the TLS RFC is
// concerned, only the TLS connection needs to be shut down; the TCP
// connection may remain open.  I am wondering if it would be simpler
// if we also kept the two connections separate.  This would require
// that the application mediate somehow.

bool emTlsClose(TcpConnection *connection)
{
  TlsState *tls = connectionTlsState(connection);
  if (tls != NULL) {
    if (tls->connection.state == TLS_HANDSHAKE_DONE) {
      emTlsSendCloseNotify(tls);
      tls->connection.state = TLS_CLOSING;
      return false;
    } else if (tls->connection.state == TLS_CLOSING) {
      return false;
    } else {
      tls->connection.state = TLS_CLOSED;
      return true;                // continue with close
    }
  }
  return false;
}

// Do we need our own event here to handle timeouts?

TlsStatus emReadAndProcessRecord(TlsState *tls, Buffer *incomingQueue)
{
  uint16_t available = emBufferQueueByteLength(incomingQueue);
  Buffer recordBuffer = emBufferQueueHead(incomingQueue);
  uint8_t header[DTLS_RECORD_HEADER_LENGTH];      // the larger of the two
  uint16_t initialRecordLength = 0;
  uint16_t finalRecordLength = 0;
  uint8_t *record;
  uint16_t recordHeaderLength = 0;
  if ( tls != NULL) {
    recordHeaderLength = (tlsUsingDtls(tls)
                          ? DTLS_RECORD_HEADER_LENGTH
                          : TLS_RECORD_HEADER_LENGTH);
  }
  if (available < recordHeaderLength) {
    lose(SECURITY, TLS_INSUFFICIENT_DATA);
  }

  emCopyFromBufferQueue(incomingQueue, recordHeaderLength, header);

  // recordHeaderLength is set to be at least 4 (TLS_RECORD_HEADER_LENGTH)
  initialRecordLength = (recordHeaderLength + HIGH_LOW_TO_INT(header[recordHeaderLength - 2],
                                                              header[recordHeaderLength - 1]));

  if (available < initialRecordLength) {
    lose(SECURITY, TLS_INSUFFICIENT_DATA);
  }

  emLogLine(SECURITY2,
            "in record length %u, buffer length %u",
            initialRecordLength,
            emGetBufferLength(recordBuffer));
  if (emGetBufferLength(recordBuffer) < initialRecordLength) {
    lose(SECURITY, TLS_INSUFFICIENT_DATA);
  }

  record = emGetBufferPointer(recordBuffer);
  dump("incoming", record, initialRecordLength);

  // Initilize the status and then check if tls is null for CSTAT
  TlsStatus status = TLS_MESSAGE_PROCESSING_FAILURE;
  if (tls != NULL && record != NULL) {
    status = emTlsProcessIncoming(tls,
                                  record,
                                  initialRecordLength,
                                  &finalRecordLength);
  }

  // RFC 5246 says that any corrupt packet should cause a fatal alert
  // to be sent and shut down the connection.  This seems odd, because
  // it makes it easy for an attacker to shut down connections.
  if (tls != NULL) {
    if (status != TLS_SUCCESS) {
      // BUG: shutdown    // Check the RFCs.  Shutting down here allows a
      // simple DOS attack.  Send a garbage packet and shut down any connection.
      emBufferQueueRemoveHead(incomingQueue);
      return status;
    } else if (finalRecordLength == recordHeaderLength) {
      // Only allow a few empty messages in a row.
      tls->connection.emptyRecordCount++;
      if (3 < tls->connection.emptyRecordCount) {
        // BUG: shutdown
        assert(false);
        return TLS_WRONG_HANDSHAKE_MESSAGE;
      }
    } else {
      tls->connection.emptyRecordCount = 0;
    }
  }

  dump("record", emGetBufferPointer(recordBuffer), finalRecordLength);

  if (record != NULL && tls != NULL) {
    if (record[0] != TLS_CONTENT_APPLICATION_DATA) {
      TlsStatus status = processRecord(tls, record, finalRecordLength);
      if (recordBuffer == emBufferQueueHead(incomingQueue)) {
        emRemoveBytesFromBufferQueue(incomingQueue, initialRecordLength);
      } else {
        debug("not dropping incoming record");
      }
      return status;
    } else if (tls->connection.state != TLS_HANDSHAKE_DONE) {
      lose(SECURITY, TLS_PREMATURE_APP_MESSAGE);
    } else {
      // Now we need to remove the header and disconnect the record
      // buffer from the queue so that the buffer can be used by higher
      // layers.
      uint8_t *payload = emGetBufferPointer(recordBuffer);
      uint16_t payloadLength = finalRecordLength - recordHeaderLength;
      MEMMOVE(payload, payload + recordHeaderLength, payloadLength);

      if (recordBuffer != emBufferQueueHead(incomingQueue)) {
        // It's our own buffer created above and the app can have it.
      } else {
        uint16_t leftOver = emGetBufferLength(recordBuffer) - initialRecordLength;
        emBufferQueueRemoveHead(incomingQueue);
        if (leftOver != 0) {
          // The tricky case.  There are bytes at the end of the record
          // buffer that need to go back on incomingQueue.  We split the
          // buffer into two and put the second part back on the head of
          // the queue.  The new Buffer header goes into the space freed
          // up when the security data and the header were removed.
          Buffer bufferB = emSplitBuffer(recordBuffer, payloadLength);
          uint8_t *contentsB = emGetBufferPointer(bufferB);
          MEMMOVE(contentsB, payload + initialRecordLength, leftOver);
          emSetBufferLength(bufferB, leftOver);
          emBufferQueueAddToHead(incomingQueue, bufferB);
        }
      }
      emSetBufferLength(recordBuffer, payloadLength);
      emBufferQueueAdd((&((DtlsConnection *)
                          emFindConnectionFromHandle(tls->connection.fd))
                        ->incomingData),
                       recordBuffer);
      return TLS_SUCCESS;
    }
  } else {
    return TLS_MESSAGE_PROCESSING_FAILURE;
  }
}

static uint16_t preferredCryptoSuite(uint16_t *flags)
{
  if (*flags & TLS_HAVE_ECDHE_ECDSA) {
    *flags = TLS_HAVE_ECDHE_ECDSA;
    return ecdheEcdsaSuiteIdentifier;
  } else if (*flags & TLS_HAVE_DHE_RSA) {
    *flags = TLS_HAVE_DHE_RSA;
    return dheRsaSuiteIdentifier;
  } else if (*flags & TLS_HAVE_PSK) {
    *flags = TLS_HAVE_PSK;
    return pskSuiteIdentifier;
  } else if (*flags & TLS_HAVE_JPAKE) {
    *flags = TLS_HAVE_JPAKE;
    return jpakeSuiteIdentifier;
  } else {
    *flags = 0;
    return 0;
  }
}

static uint16_t matchCryptoSuites(uint8_t *finger,
                                  uint16_t cipherListLength,
                                  uint16_t flags)
{
  uint16_t i;
  uint16_t cipherMask = 0;
  for (i = 0; i < cipherListLength; i += 2) {
    uint16_t suite = emberFetchHighLowInt16u(finger + i);
    if (suite == dheRsaSuiteIdentifier) {
      cipherMask |= TLS_HAVE_DHE_RSA;
    } else if (suite == ecdheEcdsaSuiteIdentifier) {
      cipherMask |= TLS_HAVE_ECDHE_ECDSA;
    } else if (suite == pskSuiteIdentifier) {
      cipherMask |= TLS_HAVE_PSK;
    } else if (suite == jpakeSuiteIdentifier) {
      cipherMask |= TLS_HAVE_JPAKE;
    }
  }
  cipherMask &= flags;
  preferredCryptoSuite(&cipherMask);
  if (cipherMask == 0) {
    return 0;
  }
  return ((~TLS_CRYPTO_SUITE_FLAGS) | cipherMask);
}

//----------------------------------------------------------------
// Client sending:
//   ClientHello

static const uint8_t eccClientHelloTail[] =
{
  0x01, TLS_COMPRESS_ALGORITHM_NULL,
  0x00, 0x17,                           // total size of extensions

  0x00, TLS_EXTENSION_SIGNATURE_ALGORITHMS,
  0x00, 0x04,                           // bytes of extension data
  0x00, 0x02,                           // bytes in algorithm array
  TLS_HASH_ALGORITHM_SHA256, TLS_SIGNATURE_ALGORITHM_ECDSA,
// These are needed for tls.woodlawnbank.com
//    TLS_HASH_ALGORITHM_SHA1, TLS_SIGNATURE_ALGORITHM_ECDSA,
//    TLS_HASH_ALGORITHM_SHA1, TLS_SIGNATURE_ALGORITHM_RSA,

// ECC requires an extension indicating which curves are acceptable.
  0x00, TLS_EXTENSION_ELLIPTIC_CURVES,
  0x00, 0x04,                           // bytes of extension data
  0x00, 0x02,                           // bytes in curve array
  0x00, TLS_ELLIPTIC_CURVE_SECP256R1,

// There is also an extension indicating which point formats are
// acceptable.  We only support the uncompressed format.
// Implementations are required to support the uncompressed format
// and there doesn't seem to be any requirement for us to say that
// we do.
  0x00, TLS_EXTENSION_EC_POINT_FORMATS,
  0x00, 0x03,                           // bytes of extension data
  0x02,                                 // bytes in format array
  TLS_ELLIPTIC_UNCOMPRESSED,
  TLS_ELLIPTIC_ANSIX962_COMPRESSED_PRIME,

// Extensions that may be needed, but haven't been required so far.
//    0xFF, 0x01, 0x00, 0x01, 0x00,       // secure renegotiation extension
//    0x00, 0x09, 0x00, 0x01, 0x00,       // cert type x509 (the default)
//    0x00, 0x23, 0x00, 0x00              // session ticket
};

static const uint8_t rsaClientHelloTail[] =
{
  0x01, TLS_COMPRESS_ALGORITHM_NULL,

  0x00, 0x08,                           // total size of extensions

  0x00, TLS_EXTENSION_SIGNATURE_ALGORITHMS,
  0x00, 0x04,                           // bytes of extension data
  0x00, 0x02,                           // bytes in algorithm array
  TLS_HASH_ALGORITHM_SHA256,
  TLS_SIGNATURE_ALGORITHM_RSA,
};

static const uint8_t pskClientHelloTail[] =
{
  0x01, TLS_COMPRESS_ALGORITHM_NULL,

  // no extensions
};

static const uint8_t jpakeClientHelloTailHead[] =
{
  0x01, TLS_COMPRESS_ALGORITHM_NULL,

  0x01, 0x5c,   // length - dynamically updated - max: 0x015c

  // extension 0
  0x00, TLS_EXTENSION_ELLIPTIC_CURVES,
  0x00, 0x04,   // length
  0x00, 0x02,   // elliptic curve list length
  0x00, TLS_ELLIPTIC_CURVE_SECP256R1,

  // extension 1
  0x00, TLS_EXTENSION_EC_POINT_FORMATS,
  0x00, 0x02,   // length
  0x01,         // ec point format length
  TLS_ELLIPTIC_UNCOMPRESSED,

  // extension 2
  0x01, 0x00,   // ecjpake key kp pair list
  0x01, 0x4a,   // length - dynamically updated - max: 0x014a

  // the tail tail is filled in dynamically using jpake
};

static const uint8_t jpakeServerHelloTailHead[] =
{
  0x01, 0x54,   // length - dynamically updated - max: 0x015c

  // extension 0
  0x00, TLS_EXTENSION_EC_POINT_FORMATS,
  0x00, 0x02,   // length
  0x01,         // ec point format length
  TLS_ELLIPTIC_UNCOMPRESSED,

  // extension 2
  0x01, 0x00,   // ecjpake key kp pair list
  0x01, 0x4a,   // length - dynamically updated - max: 0x014a

  // the tail tail is filled in dynamically using jpake
};

#define helloStartLength 34

static uint8_t *writeHelloStart(TlsState *tls, uint8_t *message, uint16_t *randomLoc)
{
  if (tlsUsingDtls(tls)) {
    *message++ = DTLS_1P2_MAJOR_VERSION;
    *message++ = DTLS_1P2_MINOR_VERSION;
  } else {
    *message++ = TLS_1P2_MAJOR_VERSION;
    *message++ = TLS_1P2_MINOR_VERSION;
  }

  emWriteCurrentTime((uint8_t *) randomLoc);
  assert(emRadioGetRandomNumbers(randomLoc + 2, (SALT_SIZE - 4) / 2));
  // MEMCOPY(randomLoc,
  //         (tlsAmClient(tls)
  //          ? threadTestClientHelloRandom
  //          : threadTestServerHelloRandom),
  //         SALT_SIZE);
  MEMMOVE(message, randomLoc, SALT_SIZE);
  dump((tlsAmClient(tls) ? "client salt" : "server salt"),
       message,
       SALT_SIZE);
  return message + SALT_SIZE;
}

static void writeSessionId(TlsSessionState *session, uint8_t **fingerLoc)
{
  uint8_t *finger = *fingerLoc;
  *finger++ = session->idLength;
  MEMMOVE(finger, session->id, session->idLength);
  finger += session->idLength;
  *fingerLoc = finger;
}

static EmberStatus sendHelloRequest(TlsState *tls)
{
  uint8_t *finger;
  Buffer buffer = emAllocateTlsHandshakeBuffer(tls,
                                               TLS_HANDSHAKE_HELLO_REQUEST,
                                               0,
                                               &finger);
  if (buffer == NULL_BUFFER) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  return emTlsSendBuffer(tls, buffer);
}

static EmberStatus processHelloRequest(TlsState *tls,
                                       uint8_t *commandLoc,
                                       uint8_t *incoming,
                                       uint16_t length)
{
  if (length != 0) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }
  return EMBER_SUCCESS;
}

static EmberStatus sendClientHelloHandler(TlsState *tls)
{
  uint8_t suitesLength = 0;
  uint8_t *handshakePayload;
  uint16_t length;
  Buffer buffer;
  const uint8_t *tail = pskClientHelloTail;
  uint16_t tailLength = sizeof(pskClientHelloTail);
  uint8_t suites[2
                 + SUITE_COUNT * 2];
  uint8_t *finger = suites + 2;
  uint8_t hxaHxb[2 * ECC_JPAKE_MAX_DATA_LENGTH];

  MEMSET(suites, 0, sizeof(suites));

  if (haveEcc && tls->connection.flags & TLS_HAVE_ECDHE_ECDSA) {
    finger[0] = HIGH_BYTE(ecdheEcdsaSuiteIdentifier);
    finger[1] = LOW_BYTE(ecdheEcdsaSuiteIdentifier);
    finger += 2;
    tail = eccClientHelloTail;
    tailLength = sizeof(eccClientHelloTail);
  }

  if (haveRsa && tls->connection.flags & TLS_HAVE_DHE_RSA) {
    finger[0] = HIGH_BYTE(dheRsaSuiteIdentifier);
    finger[1] = LOW_BYTE(dheRsaSuiteIdentifier);
    finger += 2;
    tail = rsaClientHelloTail;
    tailLength = sizeof(rsaClientHelloTail);
  }

  if (tls->connection.flags & TLS_HAVE_PSK) {
    finger[0] = HIGH_BYTE(pskSuiteIdentifier);
    finger[1] = LOW_BYTE(pskSuiteIdentifier);
    finger += 2;
  }

  uint16_t jpakeExtraLength = 0;

  if (tls->connection.flags & TLS_HAVE_JPAKE) {
    finger[0] = HIGH_BYTE(jpakeSuiteIdentifier);
    finger[1] = LOW_BYTE(jpakeSuiteIdentifier);
    finger += 2;
    tail = jpakeClientHelloTailHead;
    tailLength = sizeof(jpakeClientHelloTailHead);
    emJpakeEccStart(true, tls->handshake.jpakeKey, tls->handshake.jpakeKeyLength);
    assert(emJpakeEccGetHxaHxbData(hxaHxb, sizeof(hxaHxb), &jpakeExtraLength));
  }

  suites[1] = finger - (suites + 2);
  suitesLength = 2 + suites[1];

  length = (helloStartLength
            + 1         // length of session ID length
            + tls->session.idLength
            + suitesLength
            + tailLength
            + jpakeExtraLength);
  if (tlsUsingDtls(tls)) {
    length += 1;        // for cookie length (= 0)
  }
  buffer = emAllocateTlsHandshakeBuffer(tls,
                                        TLS_HANDSHAKE_CLIENT_HELLO,
                                        length,
                                        &handshakePayload);

  if (buffer == NULL_BUFFER) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  finger = handshakePayload;
  finger = writeHelloStart(tls, finger, tls->handshake.clientSalt);

  writeSessionId(&tls->session, &finger);

  if (tlsUsingDtls(tls)) {
    *finger++ = 0;      // cookieLength
  }

  MEMCOPY(finger, suites, suitesLength);
  finger += suitesLength;
  MEMCOPY(finger, tail, tailLength);
  finger += tailLength;

  if (tls->connection.flags & TLS_HAVE_JPAKE) {
    // update the lengths in the tail head
    uint8_t *tailHead = finger - tailLength;
    tailHead[2] = HIGH_BYTE(jpakeExtraLength + 18);
    tailHead[3] = LOW_BYTE(jpakeExtraLength + 18);
    tailHead[20] = HIGH_BYTE(jpakeExtraLength);
    tailHead[21] = LOW_BYTE(jpakeExtraLength);
    MEMCOPY(finger, hxaHxb, jpakeExtraLength);
    finger += jpakeExtraLength;
  }

  // Could add a hostname extension here.

  assert(finger - handshakePayload == length);
  return emTlsSendBuffer(tls, buffer);
}

// What a mess.  The server rejected our ClientHello message and wants us to
// resend it with a cookie.  With JPAKE we can't regenerate it because of all
// of the randomness in the zero-knowledge proofs.  So we have to take the
// copy we saved for retrying and update it with a cookie in the right place,
// making sure we get all of the lengths updated as well.

static void addToLength(uint8_t *location, uint16_t delta)
{
  emberStoreHighLowInt16u(location,
                          (emberFetchHighLowInt16u(location)
                           + delta));
}

static void copyBlock(uint8_t **toLoc, uint8_t **fromLoc, uint16_t count)
{
  MEMCOPY(*toLoc, *fromLoc, count);
  *toLoc += count;
  *fromLoc += count;
}

static bool addCookie(DtlsState *dtls, uint8_t *cookie, uint16_t cookieLength)
{
  Buffer flight = dtls->dtlsHandshake.outgoingFlight;
  if (flight == NULL_BUFFER
      || emGetPayloadLink(flight) != NULL_BUFFER) {
    lose(SECURITY, false);
  }
  uint8_t *from = emGetBufferPointer(flight);
  uint16_t length = emGetBufferLength(flight);
  uint8_t *end = from + length;

  if (from != NULL) {
    emLogBytesLine(SECURITY2, "before ", from, length);

    if (!(from[0] == TLS_CONTENT_HANDSHAKE
          && from[DTLS_SEQUENCE_NUMBER_OFFSET + 1] == 0       // check epoch == 0
          && from[DTLS_SEQUENCE_NUMBER_OFFSET] == 0
          && from[DTLS_RECORD_HEADER_LENGTH] == TLS_HANDSHAKE_CLIENT_HELLO)) {
      lose(SECURITY, false);
    }
  }

  Buffer newFlight = emAllocateBuffer(length + cookieLength);
  if (newFlight == NULL_BUFFER) {
    lose(SECURITY, false);
  }
  uint8_t *to = emGetBufferPointer(newFlight);

  copyBlock(&to, &from, DTLS_RECORD_HEADER_LENGTH);
  addToLength(to - 2, cookieLength);

  copyBlock(&to, &from, DTLS_HANDSHAKE_HEADER_LENGTH);
  addToLength(to - 2, cookieLength);
  addToLength(to - 10, cookieLength);
  *(to - 7) = dtls->dtlsHandshake.outHandshakeCounter++;

  copyBlock(&to, &from, 2 + SALT_SIZE); // version and salt
  copyBlock(&to, &from, *from + 1);     // session ID

  emLogBytesLine(SECURITY2, "left ", from, end - from);

  if (0 < *from++) {
    lose(SECURITY, false);       // already has a cookie
  }
  *to++ = cookieLength;
  MEMCOPY(to, cookie, cookieLength);
  to += cookieLength;

  MEMCOPY(to, from, end - from);
  dtls->dtlsHandshake.outgoingFlight = newFlight;

  emSha256Start(&(dtls->handshake.messageHash));
  emSha256HashBytes(&(dtls->handshake.messageHash),
                    emGetBufferPointer(newFlight) + DTLS_RECORD_HEADER_LENGTH,
                    emGetBufferLength(newFlight) - DTLS_RECORD_HEADER_LENGTH);
  return true;
}

static bool checkEllipticCurveSupport(uint8_t *contents, uint16_t length)
{
  for (uint8_t i = 0; i < length; i += 2, contents += 2) {
    // Currently, we only support the secp256r1 (23) elliptic curve extension
    if (emberFetchHighLowInt16u(contents) == TLS_ELLIPTIC_CURVE_SECP256R1) {
      return true;
    }
  }
  return false;
}

static bool checkPointFormatSupport(uint8_t *contents, uint16_t length)
{
  for (uint8_t i = 0; i < length; i += 1, contents += 1) {
    // Currently, we only parse an uncompressed point format
    if (*contents == TLS_ELLIPTIC_UNCOMPRESSED) {
      return true;
    }
  }
  return false;
}

static EmberStatus processJpakeHello(uint8_t *finger, uint16_t length, bool amClient)
{
  const uint8_t *end = finger + length;
  uint16_t extension;
  uint16_t extensionLength;
  uint16_t tailLength = 0;
  bool isEllipticCurveSupported = false;
  bool isPointFormatSupported = false;

  while (finger < end) {
    extension = emberFetchHighLowInt16u(finger);
    finger += 2;
    extensionLength = emberFetchHighLowInt16u(finger);
    finger += 2;

    switch (extension) {
      case TLS_EXTENSION_ELLIPTIC_CURVES:
        isEllipticCurveSupported = checkEllipticCurveSupport(finger + 2,
                                                             emberFetchHighLowInt16u(finger));
        finger += extensionLength;
        break;

      case TLS_EXTENSION_EC_POINT_FORMATS:
        isPointFormatSupported = checkPointFormatSupport(finger + 1, *finger);
        finger += extensionLength;
        break;

      case ECJPAKE_KEY_KP_PAIR:
        tailLength = extensionLength;
        break;
    }
    if (extension == ECJPAKE_KEY_KP_PAIR) {
      break;
    }
  }
  if ((amClient && (!isPointFormatSupported))
      || (!amClient && (!isEllipticCurveSupported || !isPointFormatSupported))) {
    // TODO: Commented out for mbedtls interop.
    // lose(SECURITY, EMBER_ERR_FATAL);
  }

  if (!emJpakeEccVerifyHxaHxbData(finger, tailLength)) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }
  finger += tailLength;

  return EMBER_SUCCESS;
}

//----------------------------------------------------------------
// Server processing:
//   ClientHello

//
#define MINIMUM_CLIENT_HELLO \
  (1 + 1 + SALT_SIZE + 1 + 4 + 2)
// messageType length majorVersion minorVersion random sessionLength
// cipherList compressionList

static EmberStatus processClientHello(TlsState *tls,
                                      uint8_t *commandLoc,
                                      uint8_t *incoming,
                                      uint16_t length)
{
  uint8_t *finger = incoming;
  uint8_t *end = incoming + length;

  if (!(MINIMUM_CLIENT_HELLO <= length
        && (tlsUsingDtls(tls)
            ? (*finger++ == DTLS_1P2_MAJOR_VERSION
               && *finger++ == DTLS_1P2_MINOR_VERSION)
            : (*finger++ == TLS_1P2_MAJOR_VERSION
               && *finger++ == TLS_1P2_MINOR_VERSION)))) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  MEMMOVE(tls->handshake.clientSalt, finger, SALT_SIZE);
  dump("got client salt", finger, SALT_SIZE);

  finger += SALT_SIZE;

  {
    uint8_t sessionLength = *finger++;

    if (TLS_SESSION_ID_SIZE < sessionLength) {
      lose(SECURITY, EMBER_ERR_FATAL);
    }

    if (sessionLength != 0
        && !(tls->connection.flags & TLS_IS_DTLS_JOIN)
        && emRestoreTlsSession(&tls->session,
                               emberTcpRemoteIpAddress(tls->connection.fd),
                               finger,
                               sessionLength)) {
      tls->connection.flags |= TLS_IS_RESUME;
    } else {
      uint16_t *sessionId = tls->session.id;
      MEMSET(sessionId, 0, sizeof(tls->session.id));
      MEMMOVE(sessionId, finger, sessionLength);
      tls->session.idLength = sessionLength;
    }
    finger += sessionLength;
  }

  if (tlsUsingDtls(tls)) {
    uint16_t cookieLength = *finger++;
    finger += cookieLength;
    if (0 < cookieLength) {
      // We have to skip over any sequence numbers from RequestHelloVerify
      // messages sent earlier.  We leave the epoch (first two bytes) alone.
      DtlsState *dtls = (DtlsState *) tls;
      MEMCOPY(dtls->dtlsHandshake.epoch0OutRecordCounter + 2,
              commandLoc - 8,     // Skip back over a two-byte length and the
                                  // sequence number itself.
              6);
      emLogBytesLine(SECURITY2, "out count ", dtls->dtlsHandshake.epoch0OutRecordCounter,
                     8);
    }
  }

  // FIX: When resuming, the cipher and compression should match
  // the saved session.

  {
    uint16_t cipherListLength = HIGH_LOW_TO_INT(finger[0], finger[1]);
    uint16_t cipherMask = 0;
    finger += 2;
    if (!(2 <= cipherListLength
          && finger + cipherListLength <= end
          && ((cipherListLength & 0x01) == 0))) {
      debug("cipherListLength: %u\n"
            "finger + cipherLength <= end: %u\n"
            "((cipherListLength & 0x01) == 0): %u\n",
            2 <= cipherListLength,
            finger + cipherListLength <= end,
            ((cipherListLength & 0x01) == 0));
      lose(SECURITY, EMBER_ERR_FATAL);
    }
    cipherMask = matchCryptoSuites(finger,
                                   cipherListLength,
                                   tls->connection.flags);
    if (cipherMask == 0) {
      lose(SECURITY, EMBER_ERR_FATAL);
    }
    tls->connection.flags &= cipherMask;
    finger += cipherListLength;
  }

  {
    uint8_t compressionListLength = *finger++;
    if (!(1 <= compressionListLength    // must have CompressionMethod.null
          && (finger + compressionListLength <= end))) {
      lose(SECURITY, EMBER_ERR_FATAL);
    }
    // We don't use any compression, so we do not care what is in the list.
    finger += compressionListLength;
  }

  // The ServerHello is required to use the same handshake counter as
  // the clientHello.
  if (tlsUsingDtls(tls)) {
    DtlsState *dtls = (DtlsState *) tls;
    dtls->dtlsHandshake.outHandshakeCounter =
      dtls->dtlsHandshake.inHandshakeCounter - 1;
  }

  if (finger == end) {
    if (tls->connection.flags & TLS_HAVE_JPAKE) {
      lose(SECURITY, EMBER_ERR_FATAL);
    } else {
      return EMBER_SUCCESS;           // no extensions
    }
  }

  {
    uint16_t extensionLength = HIGH_LOW_TO_INT(finger[0], finger[1]);
    finger += 2;

    if (finger + extensionLength != end) {
      lose(SECURITY, EMBER_ERR_FATAL);
    }

    // We ignore any extensions (except JPAKE), which is probably not a great
    // idea. For ECC in particular, we should check that our curve is supported.

    if (tls->connection.flags & TLS_HAVE_JPAKE) {
      emJpakeEccStart(false, tls->handshake.jpakeKey, tls->handshake.jpakeKeyLength);

      if (processJpakeHello(finger, extensionLength, false) != EMBER_SUCCESS) {
        lose(SECURITY, EMBER_ERR_FATAL);
      }
    }

    return EMBER_SUCCESS;
  }
}

// RFC 6347:
// In order to avoid sequence number duplication in case of multiple
// HelloVerifyRequests, the server MUST use the record sequence number
// in the ClientHello as the record sequence number in the
// HelloVerifyRequest.

#define SEQUENCE_OFFSET 5       // count it out below

static const uint8_t helloVerifyRequestMessage[] = {
  0x16,            // handshake message
  0xFE, 0xFD,      // version
  0x00, 0x00,      // epoch
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // sequence
  0x00, 0x17,      // length

  0x03,            // HelloVerifyRequest
  0x00, 0x00, 0x0B, // length
  0x00, 0x00,      // handshake message sequence
  0x00, 0x00, 0x00, // fragment offset
  0x00, 0x00, 0x0B, // fragment length
  0xFE, 0xFD,      // server version
  0x08,            // cookie length
};

#define OUR_COOKIE_LENGTH 8

static const uint8_t ourCookie[] = {
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57
};

Buffer emMakeHelloVerifyRequestMessage(uint8_t *sequence, const uint8_t *cookie)
{
  Buffer result =
    emAllocateBuffer(sizeof(helloVerifyRequestMessage) + OUR_COOKIE_LENGTH);
  if (result != NULL_BUFFER) {
    uint8_t *contents = emGetBufferPointer(result);
    MEMCOPY(contents,
            helloVerifyRequestMessage,
            sizeof(helloVerifyRequestMessage));
    MEMCOPY(contents + SEQUENCE_OFFSET,
            sequence,
            6);
    MEMCOPY(contents + sizeof(helloVerifyRequestMessage),
            cookie,
            OUR_COOKIE_LENGTH);
  }
  return result;
}

// This is called on incoming packets for which we do not have a connection.
// It checks to see if this is a CLIENT_HELLO, and, if so, does it have the
// right cookie.

uint8_t emParseInitialDtlsPacket(uint8_t *packet,
                                 uint16_t packetLength,
                                 uint8_t options,
                                 bool expectHello,
                                 Buffer *requestResult)
{
  // Does anyone check transport payload length against the buffer size?
  // This should be checked when we fill in the ipHeader fields.
  // This isn't quite the correct check because there is other stuff in
  // the buffer ahead fo the transport payload.
  // if (emGetBufferLength(recordBuffer) < recordLength) {
  //   lose(SECURITY, DTLS_DROP);
  // }

  if (packetLength < DTLS_RECORD_HEADER_LENGTH) {
    emLogLine(SECURITY,
              "dtls parse packet length have %d want at least %d",
              packetLength,
              DTLS_RECORD_HEADER_LENGTH);
    lose(SECURITY, DTLS_DROP);
  }

  uint8_t *finger = packet;

  if (!(*finger++ == TLS_CONTENT_HANDSHAKE)) {
    if (expectHello) {
      lose(SECURITY, DTLS_DROP);
    } else {
      return DTLS_DROP;
    }
  }

  if (!(*finger++ == DTLS_1P2_MAJOR_VERSION
        && *finger++ == DTLS_1P2_MINOR_VERSION)) {
    emLogLine(SECURITY,
              "dtls parse drop bad packet header want [%x %x %x ...]",
              TLS_CONTENT_HANDSHAKE,
              DTLS_1P2_MAJOR_VERSION,
              DTLS_1P2_MINOR_VERSION);
    lose(SECURITY, DTLS_DROP);
  }

  // Saved in case we are sending a HelloVerifyRequest.  We skip over the
  // epoch.
  uint8_t *sequence = finger + 2;
  finger += DTLS_SEQUENCE_NUMBER_LENGTH;

  uint16_t length = emberFetchHighLowInt16u(finger);
  finger += 2;
  uint8_t *end = finger + length;

  if (packetLength < end - packet) {
    emLogLine(SECURITY,
              "dtls parse fatal for (%d < %d)",
              packetLength,
              end - packet);
    lose(SECURITY, DTLS_DROP);
  }

  if (!(DTLS_HANDSHAKE_HEADER_LENGTH < end - finger)) {
    emLogLine(SECURITY,
              "dtls parse fatal for (end - finger: %u > "
              "DTLS_HANDSHAKE_HEADER_LENGTH: %u, "
              "*finger == TLS_HANDSHAKE_CLIENT_HELLO: %u)",
              end - finger,
              DTLS_HANDSHAKE_HEADER_LENGTH,
              (end - finger
               > DTLS_HANDSHAKE_HEADER_LENGTH));
    lose(SECURITY, DTLS_DROP);
  }

  if (*finger != TLS_HANDSHAKE_CLIENT_HELLO) {
    if (expectHello) {
      lose(SECURITY, DTLS_DROP);
    } else {
      return DTLS_DROP;
    }
  }

  uint16_t handshakeLength =
    HIGH_LOW_TO_INT(finger[2], finger[3])
    + DTLS_HANDSHAKE_HEADER_LENGTH;

  if (!(handshakeLength <= end - finger
        && finger[1] == 0)) {   // we should not see lengths > 0xFFFF.
    emLogLine(SECURITY, "dtls parse bad length");
    lose(SECURITY, DTLS_DROP);
  }
  finger += 4;

  // next two bytes are handshake counter, which we need to check
  // 0 for initial, does second with cookie have 1?
  finger += 2;

  // next are fragment offset and length, three bytes each
  // should we do anything with them?
  finger += 6;

  if (!(*finger++ == DTLS_1P2_MAJOR_VERSION
        && *finger++ == DTLS_1P2_MINOR_VERSION)) {
    emLogLine(SECURITY,
              "dtls parse bad version want [%x %x]",
              DTLS_1P2_MAJOR_VERSION,
              DTLS_1P2_MINOR_VERSION);
    lose(SECURITY, DTLS_DROP);
  }

  finger += SALT_SIZE;

  uint8_t sessionLength = *finger++;

  if (TLS_SESSION_ID_SIZE < sessionLength) {
    emLogLine(SECURITY,
              "dtls parse bad session id size %u",
              sessionLength);
    lose(SECURITY, DTLS_DROP);
  }

  if (end <= finger) {
    emLogLine(SECURITY, "dtls parse bad length");
    lose(SECURITY, DTLS_DROP);
  }

  if (sessionLength != 0) {
    if (options & DTLS_ALLOW_RESUME_OPTION
        // && have_session
        ) {
      emLogLine(SECURITY, "dtls parse process have session");
      return DTLS_PROCESS;
    } else {
      emLogLine(SECURITY, "dtls parse no session");
      lose(SECURITY, DTLS_DROP);
    }
  }

  uint16_t cookieLength = *finger++;
  if (cookieLength != 0) {
    if (finger + cookieLength <= end
        // && verifyCookie()
        ) {
      emLogLine(SECURITY, "dtls parse process verify cookie");
      return DTLS_PROCESS;
    } else {
      emLogLine(SECURITY, "dtls parse no verify cookie");
      lose(SECURITY, DTLS_DROP);
    }
  } else if (options & DTLS_REQUIRE_COOKIE_OPTION) {
    emLogLine(SECURITY, "dtls parse making cookie");
    *requestResult =
      emMakeHelloVerifyRequestMessage(sequence, ourCookie);
    if (*requestResult == NULL_BUFFER) {
      lose(SECURITY, DTLS_DROP);
    } else {
      return DTLS_SEND_VERIFY_REQUEST;
    }
  } else {
    emLogLine(SECURITY, "dtls parse process no cookie needed");
    return DTLS_PROCESS;        // cookies not needed
  }
}

//----------------------------------------------------------------
// Server sending:
//   ServerHello
//   Certificate
//   ServerKeyExchange
//   CertificateRequest
//   ServerHelloDone

#define serverHelloLength (helloStartLength + 1 + 3)

static EmberStatus sendServerHello(TlsState *tls)
{
  uint8_t *handshakePayload;
  uint8_t *finger;
  Buffer buffer;
  uint8_t hxaHxb[2 * ECC_JPAKE_MAX_DATA_LENGTH];
  uint16_t jpakeExtraLength = 0;

  if (!tlsIsResume(tls)) {
    tls->session.idLength = TLS_SESSION_ID_SIZE;
    // MEMCOPY(tls->session.id,
    //         threadTestServerHelloSessionId,
    //         TLS_SESSION_ID_SIZE);
    assert(emRadioGetRandomNumbers(tls->session.id,
                                   (sizeof(tls->session.id) + 1) / 2));
  }

  uint16_t helloLength = serverHelloLength;

  if (tls->connection.flags & TLS_HAVE_JPAKE) {
    assert(emJpakeEccGetHxaHxbData(hxaHxb, sizeof(hxaHxb), &jpakeExtraLength));
    helloLength += (sizeof(jpakeServerHelloTailHead) + jpakeExtraLength);
  }

  buffer = emAllocateTlsHandshakeBuffer(tls,
                                        TLS_HANDSHAKE_SERVER_HELLO,
                                        helloLength + tls->session.idLength,
                                        &handshakePayload);

  if (buffer == NULL_BUFFER) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  finger = writeHelloStart(tls, handshakePayload, tls->handshake.serverSalt);
  writeSessionId(&tls->session, &finger);

  {
    uint16_t flags = tls->connection.flags;
    uint16_t suiteId = preferredCryptoSuite(&flags);
    *finger++ = HIGH_BYTE(suiteId);
    *finger++ = LOW_BYTE(suiteId);
  }
  *finger++ = TLS_COMPRESS_ALGORITHM_NULL;

  if (tls->connection.flags & TLS_HAVE_JPAKE) {
    MEMCOPY(finger, jpakeServerHelloTailHead, sizeof(jpakeServerHelloTailHead));
    // update the lengths in the tail head
    finger[0] = HIGH_BYTE(jpakeExtraLength + 10);
    finger[1] = LOW_BYTE(jpakeExtraLength + 10);
    finger[10] = HIGH_BYTE(jpakeExtraLength);
    finger[11] = LOW_BYTE(jpakeExtraLength);
    finger += sizeof(jpakeServerHelloTailHead);
    MEMCOPY(finger, hxaHxb, jpakeExtraLength);
    finger += jpakeExtraLength;
  }

  assert(finger - handshakePayload
         == helloLength + tls->session.idLength);

  if (tlsIsResume(tls)) {
    emDeriveKeys(tls);
    tls->connection.state = TLS_SERVER_SEND_CHANGE_CIPHER_SPEC;
  } else if (tls->connection.flags & TLS_HAVE_PSK) {
    tls->connection.state = TLS_SERVER_SEND_HELLO_DONE;
  } else if (tls->connection.flags & TLS_HAVE_JPAKE) {
    tls->connection.state = TLS_SERVER_SEND_KEY_EXCHANGE;
  }

  return emTlsSendBuffer(tls, buffer);
}

static EmberStatus sendServerKeyExchange(TlsState *tls)
{
#ifdef HAVE_TLS_ECDHE_ECDSA
  if (haveEcc && tls->connection.flags & TLS_HAVE_ECDHE_ECDSA) {
    return emSendEcdhServerKeyExchange(tls);
  }
#endif
#ifdef HAVE_TLS_JPAKE
  if (tls->connection.flags & TLS_HAVE_JPAKE) {
    return emSendJpakeServerKeyExchange(tls);
  }
#endif
  lose(SECURITY, EMBER_ERR_FATAL);
}

static EmberStatus sendServerHelloDone(TlsState *tls)
{
  uint8_t *finger;
  Buffer buffer =
    emAllocateTlsHandshakeBuffer(tls,
                                 TLS_HANDSHAKE_SERVER_HELLO_DONE,
                                 0,
                                 &finger);

  if (buffer == NULL_BUFFER) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  if ((tls->connection.flags & TLS_HAVE_PSK)
      || (tls->connection.flags & TLS_HAVE_JPAKE)) {
    tls->connection.state = TLS_SERVER_EXPECT_KEY_EXCHANGE;
  }

  return emTlsSendBuffer(tls, buffer);
}

//----------------------------------------------------------------
// Client processing:
//   ServerHello
//   Certificate        (this is shared with the server side)
//   ServerKeyExchange
//   CertificateRequest
//   ServerHelloDone

static EmberStatus processServerHello(TlsState *tls,
                                      uint8_t *commandLoc,
                                      uint8_t *incoming,
                                      uint16_t length)
{
  uint8_t sessionIdLength;
  uint16_t extensionLength;
  uint8_t *finger = incoming;
  uint8_t *end = incoming + length;

  if (!(tlsUsingDtls(tls)
        ? (*finger++ == DTLS_1P2_MAJOR_VERSION
           && *finger++ == DTLS_1P2_MINOR_VERSION)
        : (*finger++ == TLS_1P2_MAJOR_VERSION
           && *finger++ == TLS_1P2_MINOR_VERSION))) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  if (*commandLoc != TLS_HANDSHAKE_SERVER_HELLO) {
    if (tlsUsingDtls(tls)
        && *commandLoc == TLS_HANDSHAKE_CLIENT_HELLO_VERIFY) {
      DtlsState *dtls = (DtlsState *) tls;
      uint16_t cookieLength = *finger++;
      if (addCookie(dtls, finger, cookieLength)) {
        emUpdateOutgoingFlight(dtls);         // updates sequence
        emUdpSendDtlsRecord(tls->connection.fd,
                            dtls->dtlsHandshake.outgoingFlight);
        emTlsSetState(tls, TLS_CLIENT_EXPECT_HELLO);
        return EMBER_SUCCESS;
      } else {
        lose(SECURITY, EMBER_ERR_FATAL);
      }
    } else {
      lose(SECURITY, EMBER_ERR_FATAL);
    }
  }

  if (length < 2 + SALT_SIZE + 1) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  MEMMOVE(tls->handshake.serverSalt, finger, SALT_SIZE);
  dump("got server salt", finger, SALT_SIZE);
  finger += SALT_SIZE;

  sessionIdLength = *finger++;

  if (!(sessionIdLength <= TLS_SESSION_ID_SIZE
        && finger + sessionIdLength + 2 <= end)) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  if (tlsIsResume(tls)
      && tls->session.idLength == sessionIdLength
      && MEMCOMPARE(tls->session.id, finger, sessionIdLength) == 0) {
    emDeriveKeys(tls);
    tls->connection.state = TLS_CLIENT_EXPECT_CHANGE_CIPHER_SPEC;
  } else {
    tls->connection.flags &= ~TLS_IS_RESUME;
    tls->session.idLength = sessionIdLength;
    MEMMOVE(tls->session.id, finger, sessionIdLength);
  }

  finger += sessionIdLength;

  if (finger + 3 > end) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  if (!tlsIsResume(tls)) {
    uint16_t cipherMask = matchCryptoSuites(finger, 2, tls->connection.flags);
    if (cipherMask == 0) {
      lose(SECURITY, EMBER_ERR_FATAL);
    } else if (cipherMask & TLS_HAVE_PSK) {
      tls->connection.state = TLS_CLIENT_EXPECT_HELLO_DONE;
    }
    tls->connection.flags &= cipherMask;
  }
  finger += 2;

  if (!(*finger++ == TLS_COMPRESS_ALGORITHM_NULL)) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  if (finger == end) {
    return EMBER_SUCCESS;           // no extensions, we're done
  }

  extensionLength = HIGH_LOW_TO_INT(finger[0], finger[1]);
  finger += 2;

  if (finger + extensionLength != end) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  // We ignore any extensions (except JPAKE). They can only include those that
  // we send, and the only one we send is not echoed back.

  if (tls->connection.flags & TLS_HAVE_JPAKE) {
    if (processJpakeHello(finger, extensionLength, true) != EMBER_SUCCESS) {
      lose(SECURITY, EMBER_ERR_FATAL);
    }

    tls->connection.state = TLS_CLIENT_EXPECT_KEY_EXCHANGE;
  }

  return EMBER_SUCCESS;
}

static EmberStatus processServerKeyExchange(TlsState *tls,
                                            uint8_t *commandLoc,
                                            uint8_t *incoming,
                                            uint16_t length)
{
#ifdef HAVE_TLS_ECDHE_ECDSA
  if (haveEcc && tls->connection.flags & TLS_HAVE_ECDHE_ECDSA) {
    return emProcessEcdhServerKeyExchange(tls, incoming, length);
  }
#endif
#ifdef HAVE_TLS_JPAKE
  if (tls->connection.flags & TLS_HAVE_JPAKE) {
    return emProcessJpakeServerKeyExchange(tls, incoming, length);
  }
#endif
  lose(SECURITY, EMBER_ERR_FATAL);
}

static EmberStatus processCertificateRequest(TlsState *tls,
                                             uint8_t *commandLoc,
                                             uint8_t *incoming,
                                             uint16_t length)
{
  switch (*commandLoc) {
    case TLS_HANDSHAKE_CERTIFICATE_REQUEST:
#ifdef  HAVE_TLS_ECDHE_ECDSA
      if ((tls->connection.flags & TLS_HAVE_ECDHE_ECDSA)
          || (tls->connection.flags & TLS_HAVE_DHE_RSA)) {
        return emProcessCertificateRequest(tls, commandLoc, incoming, length);
      }
#endif
      lose(SECURITY, EMBER_ERR_FATAL);
    case TLS_HANDSHAKE_SERVER_HELLO_DONE:
      if (tls->connection.flags & TLS_HAVE_PSK) {
        tlsClearFlag(tls, TLS_CERTIFICATE_REQUESTED);
        return processHelloDone(tls, commandLoc, incoming, length);
      }
  }

  lose(SECURITY, EMBER_ERR_FATAL);
}

static EmberStatus processHelloDone(TlsState *tls,
                                    uint8_t *commandLoc,
                                    uint8_t *incoming,
                                    uint16_t length)
{
  if (length != 0) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  if (tlsAmClient(tls)
      && ((tls->connection.flags & TLS_HAVE_PSK)
          || (tls->connection.flags & TLS_HAVE_JPAKE))) {
    tls->connection.state = TLS_CLIENT_SEND_KEY_EXCHANGE;
  }

  return EMBER_SUCCESS;
}

//----------------------------------------------------------------
// Client sending
//   certificate        (shared with the server code)
//   clientKeyExchange
//   certificateVerify
//   changeCipherSpec
//   finished

static EmberStatus sendClientKeyExchange(TlsState *tls)
{
  uint8_t *handshakePayload;
  uint8_t *finger;
  Buffer buffer;
  uint16_t dataSize = 0;
  uint8_t ckxa[ECC_JPAKE_MAX_DATA_LENGTH];

  if ((haveEcc
       && tls->connection.flags & TLS_HAVE_ECDHE_ECDSA)
      || (haveRsa
          && tls->connection.flags & TLS_HAVE_DHE_RSA)) {
    dataSize = emDheRandomSize(tls);
  } else if (tls->connection.flags & TLS_HAVE_PSK) {
    dataSize = 2 + (emMySharedKey != NULL
                    ? emMySharedKey->identityLength
                    : 4);
  } else if (tls->connection.flags & TLS_HAVE_JPAKE) {
    assert(emJpakeEccGetCkxaOrSkxbData(ckxa, sizeof(ckxa), &dataSize));
  }

  buffer = emAllocateTlsHandshakeBuffer(tls,
                                        TLS_HANDSHAKE_CLIENT_KEY_EXCHANGE,
                                        dataSize,
                                        &handshakePayload);

  if (buffer == NULL_BUFFER) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  finger = handshakePayload;

  if ((haveEcc
       && tls->connection.flags & TLS_HAVE_ECDHE_ECDSA)
      || (haveRsa
          && tls->connection.flags & TLS_HAVE_DHE_RSA)) {
    if (emWriteDheRandom(tls, &finger)
        != EMBER_SUCCESS) {
      lose(SECURITY, EMBER_ERR_FATAL);
    }
  } else if (tls->connection.flags & TLS_HAVE_PSK) {
    emberStoreHighLowInt16u(finger,
                            (emMySharedKey != NULL
                             ? emMySharedKey->identityLength
                             : 4));
    if (emMySharedKey != NULL) {
      MEMCOPY(finger + 2, emMySharedKey->identity, emMySharedKey->identityLength);
    } else {
      MEMCOPY(finger + 2, "dtls", 4);
    }
    finger += dataSize;
#ifdef HAVE_TLS_JPAKE
  } else if (tls->connection.flags & TLS_HAVE_JPAKE) {
    MEMCOPY(finger, ckxa, dataSize);
    finger += dataSize;
#endif
  }

  if (emDeriveKeys(tls) != EMBER_SUCCESS) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  tls->connection.state = (tlsCertificateRequested(tls)
                           ? TLS_CLIENT_SEND_CERTIFICATE_VERIFY
                           : TLS_CLIENT_SEND_CHANGE_CIPHER_SPEC);

  assert(finger - handshakePayload == dataSize);

  return emTlsSendBuffer(tls, buffer);
}

static EmberStatus sendChangeCipherSpec(TlsState *tls)
{
  uint8_t *finger;
  Buffer buffer =
    emAllocateTlsBuffer(tls, TLS_CONTENT_CHANGE_CIPHER_SPEC, 1, &finger);

  if (buffer == NULL_BUFFER) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  *finger++ = 1;

  return emTlsSendBuffer(tls, buffer);
}

static EmberStatus sendFinished(TlsState *tls)
{
  uint8_t *finger;
  Buffer buffer =
    emAllocateTlsHandshakeBuffer(tls,
                                 TLS_HANDSHAKE_FINISHED,
                                 TLS_FINISHED_HASH_SIZE,
                                 &finger);

  if (buffer == NULL_BUFFER) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  emHandshakeFinishedHash(tls, finger, tlsAmClient(tls));
  finger += TLS_FINISHED_HASH_SIZE;

  if (tlsIsResume(tls)) {
    tls->connection.state = (tlsAmClient(tls)
                             ? TLS_HANDSHAKE_DONE
                             : TLS_SERVER_EXPECT_CHANGE_CIPHER_SPEC);
  }

  return emTlsSendBuffer(tls, buffer);
}

static EmberStatus processClientKeyExchange(TlsState *tls,
                                            uint8_t *commandLoc,
                                            uint8_t *incoming,
                                            uint16_t length)
{
  uint8_t *finger = incoming;
  uint8_t *end = incoming + length;

  if ((haveEcc
       && tls->connection.flags & TLS_HAVE_ECDHE_ECDSA)
      || (haveRsa
          && tls->connection.flags & TLS_HAVE_DHE_RSA)) {
    if (!emReadDheRandom(tls, &finger)) {
      lose(SECURITY, EMBER_ERR_FATAL);
    }
  } else if (tls->connection.flags & TLS_HAVE_PSK) {
    uint16_t identityLength = HIGH_LOW_TO_INT(finger[0], finger[1]);
    finger += 2;
    finger += identityLength;
  } else if (tls->connection.flags & TLS_HAVE_JPAKE) {
#ifdef HAVE_TLS_JPAKE
    if (!emJpakeEccVerifyCkxaOrSkxbData(finger, length)) {
      lose(SECURITY, EMBER_ERR_FATAL);
    }
    finger += length;
#endif
  }

  if (!(finger == end
        && emDeriveKeys(tls) == EMBER_SUCCESS)) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  if (tls->connection.flags & TLS_HAVE_PSK) {
    tls->connection.state = TLS_SERVER_EXPECT_CHANGE_CIPHER_SPEC;
  } else if (tls->connection.flags & TLS_HAVE_JPAKE) {
    tls->connection.state = TLS_SERVER_EXPECT_CHANGE_CIPHER_SPEC;
  }

  return EMBER_SUCCESS;
}

static EmberStatus processFinished(TlsState *tls,
                                   uint8_t *commandLoc,
                                   uint8_t *incoming,
                                   uint16_t length)
{
  uint8_t hash[TLS_FINISHED_HASH_SIZE];

  if (length != TLS_FINISHED_HASH_SIZE) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  emHandshakeFinishedHash(tls, hash, !tlsAmClient(tls));

  // Has to come after we compute our own hash value.
  dump("tls sha256 in", incoming, length);
  emSha256HashBytes(&tls->handshake.messageHash,
                    commandLoc,
                    (incoming - commandLoc) + length);

  if (MEMCOMPARE(incoming, hash, TLS_FINISHED_HASH_SIZE) == 0) {
    if (tlsIsResume(tls)) {
      tls->connection.state = (tlsAmClient(tls)
                               ? TLS_CLIENT_SEND_CHANGE_CIPHER_SPEC
                               : TLS_HANDSHAKE_DONE);
    }
    return EMBER_SUCCESS;
  } else {
    lose(SECURITY, EMBER_ERR_FATAL);
  }
}

//----------------------------------------------------------------
// Application processing

// We ignore incoming messages when we are closing.
static EmberStatus processClosingData(TlsState *tls,
                                      uint8_t *commandLoc,
                                      uint8_t *incoming,
                                      uint16_t length)
{
  return EMBER_SUCCESS;
}

//----------------------------------------------------------------
// This array encodes the state diagram.  Each state has a handler
// function, along with the default following state and the type of
// expected incoming message, if any.  The handler may switch to
// a state other than the given nextState.

// The handshake dispatch passes all four arguments to every handler,
// although not all handlers use all of them.  In C any additional
// arguments are ignored.
//
// A couple of the handlers need the state of the running message hash
// saved from before the current incoming was added to the hash.

typedef EmberStatus (StateHandler)(TlsState *tlsState,
                                   uint8_t *commandLoc,
                                   uint8_t *incoming,
                                   uint16_t incomingLength);

static UNUSED EmberStatus wrongMessage(TlsState *tlsState,
                                       uint8_t *commandLoc,
                                       uint8_t *incoming,
                                       uint16_t incomingLength)
{
  lose(SECURITY, EMBER_ERR_FATAL);
}

// no ECDHE -> no certificates
#ifndef HAVE_TLS_ECDHE_ECDSA
#define emSendCertificate           wrongMessage
#define emSendCertificateRequest    wrongMessage
#define emSendCertificateVerify     wrongMessage
#define emProcessCertificate        wrongMessage
#define emProcessCertificateRequest wrongMessage
#define emProcessCertificateVerify  wrongMessage
#endif

typedef struct {
  StateHandler *handler;
  uint8_t expectedMessage;
  uint8_t nextState;
  uint16_t rsaAllocated;
  uint16_t pskAllocated;
  uint16_t jpakeAllocated;
} StateAction;

static const StateAction stateActions[] = {
  {
    // TLS_SERVER_SEND_HELLO_REQUEST
    (StateHandler *) sendHelloRequest,
    0xFF,
    TLS_SERVER_EXPECT_HELLO,
    0,
    0,
    0,
  },

  {
    // TLS_CLIENT_EXPECT_HELLO_REQUEST
    processHelloRequest,
    TLS_HANDSHAKE_HELLO_REQUEST,
    TLS_CLIENT_SEND_HELLO,
    0,
    0,
    0,
  },

  {
    // TLS_CLIENT_SEND_HELLO
    (StateHandler *) sendClientHelloHandler,
    0xFF,
    TLS_CLIENT_EXPECT_HELLO,
    200,
    200,
    3200,
  },

  {
    // TLS_SERVER_EXPECT_HELLO
    processClientHello,
    TLS_HANDSHAKE_CLIENT_HELLO,
    TLS_SERVER_SEND_HELLO,
    0,
    0,
    3200,
  },

  //----------------
  // server ->

  {
    // TLS_SERVER_SEND_HELLO
    (StateHandler *) sendServerHello,
    0xFF,
    TLS_SERVER_SEND_CERTIFICATE,
    200,
    200,
    1000,
  },

  {
    // TLS_SERVER_SEND_CERTIFICATE
    (StateHandler *) emSendCertificate,
    0xFF,
    TLS_SERVER_SEND_KEY_EXCHANGE,
    1000,
    0,
    1000,
  },

  {
    // TLS_SERVER_SEND_KEY_EXCHANGE
    (StateHandler *) sendServerKeyExchange,
    0xFF,
    TLS_SERVER_SEND_CERTIFICATE_REQUEST,
    3200,
    0,
    1400,
  },

  {
    // TLS_SERVER_SEND_CERTIFICATE_REQUEST
    (StateHandler *) emSendCertificateRequest,
    0xFF,
    TLS_SERVER_SEND_HELLO_DONE,
    200,
    0,
    200,
  },

  {
    // TLS_SERVER_SEND_HELLO_DONE
    (StateHandler *) sendServerHelloDone,
    0xFF,
    TLS_SERVER_EXPECT_CERTIFICATE,
    120,
    120,
    120,
  },

  //----------------
  // -> client

  {
    // TLS_CLIENT_EXPECT_HELLO
    processServerHello,
    TLS_HANDSHAKE_MULTIPLE,
    TLS_CLIENT_EXPECT_CERTIFICATE,
    0,
    0,
    0,
  },

  {
    // TLS_CLIENT_EXPECT_CERTIFICATE
    emProcessCertificate,
    TLS_HANDSHAKE_CERTIFICATE,
    TLS_CLIENT_EXPECT_KEY_EXCHANGE,
    1450,
    0,
    0,
  },

  {
    // TLS_CLIENT_EXPECT_KEY_EXCHANGE
    processServerKeyExchange,
    TLS_HANDSHAKE_SERVER_KEY_EXCHANGE,
    TLS_CLIENT_EXPECT_CERTIFICATE_REQUEST,
    2500,
    0,
    0,
  },

  {
    // TLS_CLIENT_EXPECT_CERTIFICATE_REQUEST
    processCertificateRequest,
    TLS_HANDSHAKE_MULTIPLE,
    TLS_CLIENT_EXPECT_HELLO_DONE,
    0,
    0,
    0,
  },

  {
    // TLS_CLIENT_EXPECT_HELLO_DONE
    processHelloDone,
    TLS_HANDSHAKE_SERVER_HELLO_DONE,
    TLS_CLIENT_SEND_CERTIFICATE,
    0,
    0,
    0,
  },

  //----------------
  // client ->

  {
    // TLS_CLIENT_SEND_CERTIFICATE,
    (StateHandler *) emSendCertificate,
    0xFF,
    TLS_CLIENT_SEND_KEY_EXCHANGE,
    1000,
    0,
    1000,
  },

  {
    // TLS_CLIENT_SEND_KEY_EXCHANGE
    (StateHandler *) sendClientKeyExchange,
    0xFF,
    TLS_CLIENT_SEND_CERTIFICATE_VERIFY,
    2200,
    0,
    0,
  },

  {
    // TLS_CLIENT_SEND_CERTIFICATE_VERIFY
    (StateHandler *) emSendCertificateVerify,
    0xFF,
    TLS_CLIENT_SEND_CHANGE_CIPHER_SPEC,
    2300,
    0,
    2300,
  },

  {
    // TLS_CLIENT_SEND_CHANGE_CIPHER_SPEC
    (StateHandler *) sendChangeCipherSpec,
    0xFF,
    TLS_CLIENT_SEND_FINISHED,
    120,
    120,
    120,
  },

  {
    // TLS_CLIENT_SEND_FINISHED
    (StateHandler *) sendFinished,
    0xFF,
    TLS_CLIENT_EXPECT_CHANGE_CIPHER_SPEC,
    150,
    150,
    150,
  },

  //----------------
  // -> server

  {
    // TLS_SERVER_EXPECT_CERTIFICATE
    emProcessCertificate,
    TLS_HANDSHAKE_MULTIPLE,       // client certificate or client key exchange
    TLS_SERVER_EXPECT_KEY_EXCHANGE,
    0,
    0,
    0,
  },

  {
    // TLS_SERVER_EXPECT_KEY_EXCHANGE
    processClientKeyExchange,
    TLS_HANDSHAKE_CLIENT_KEY_EXCHANGE,
    TLS_SERVER_EXPECT_CERTIFICATE_VERIFY,
    2500,
    0,
    2500,
  },

  {
    // TLS_SERVER_EXPECT_CERTIFICATE_VERIFY
    emProcessCertificateVerify,
    TLS_HANDSHAKE_MULTIPLE,       // certificate verify or change cipher spec
    TLS_SERVER_EXPECT_CHANGE_CIPHER_SPEC,
    2500,
    0,
    2500,
  },

  {
    // TLS_SERVER_EXPECT_CHANGE_CIPHER_SPEC
    NULL,         // non-handshake messages are handled separately
    TLS_HANDSHAKE_CHANGE_CIPHER_SPEC,
    TLS_SERVER_EXPECT_FINISHED,
    0,
    0,
    0,
  },

  {
    // TLS_SERVER_EXPECT_FINISHED
    processFinished,
    TLS_HANDSHAKE_FINISHED,
    TLS_SERVER_SEND_CHANGE_CIPHER_SPEC,
    0,
    0,
    0,
  },

  //----------------
  // server ->

  {
    // TLS_SERVER_SEND_CHANGE_CIPHER_SPEC
    (StateHandler *) sendChangeCipherSpec,
    0xFF,
    TLS_SERVER_SEND_FINISHED,
    120,
    120,
    120,
  },

  {
    // TLS_SERVER_SEND_FINISHED
    (StateHandler *) sendFinished,
    0xFF,
    TLS_HANDSHAKE_DONE,
    150,
    0,
    150,
  },

  //----------------
  // -> client

  {
    // TLS_CLIENT_EXPECT_CHANGE_CIPHER_SPEC
    NULL,         // non-handshake messages are handled separately
    TLS_HANDSHAKE_CHANGE_CIPHER_SPEC,
    TLS_CLIENT_EXPECT_FINISHED,
    0,
    0,
    0,
  },

  {
    // TLS_CLIENT_EXPECT_FINISHED
    processFinished,
    TLS_HANDSHAKE_FINISHED,
    TLS_HANDSHAKE_DONE,
    0,
    0,
    0,
  },

  //----------------
  // data

  {
    // TLS_HANDSHAKE_DONE
    NULL,
    TLS_HANDSHAKE_APPLICATION_DATA, // could be data, alarm, or hello request
    TLS_HANDSHAKE_DONE,
    0,
    0,
    0,
  },

  {
    // TLS_CLOSING
    processClosingData,
    TLS_HANDSHAKE_APPLICATION_DATA, // could be data, alarm, or hello request
    TLS_CLOSING,
    0,
    0,
    0
  },
};

static TlsStatus processRecord(TlsState *tls, uint8_t *incoming, uint16_t length)
{
  uint16_t recordHeaderLength = (tlsUsingDtls(tls)
                                 ? DTLS_RECORD_HEADER_LENGTH
                                 : TLS_RECORD_HEADER_LENGTH);

  debug("incoming: state %s", tlsStateName(tls->connection.state));

  switch (incoming[0]) {
    case TLS_CONTENT_CHANGE_CIPHER_SPEC: {
      StateAction const *action = stateActions + tls->connection.state;
      if (action->expectedMessage == TLS_HANDSHAKE_CHANGE_CIPHER_SPEC
          && length == recordHeaderLength + 1
          && incoming[recordHeaderLength] == 1) {
        tlsSetFlag(tls, TLS_DECRYPT);
        MEMSET(tls->connection.inRecordCounter, 0, 8);
        if (tlsUsingDtls(tls)) {
          // now in epoch 1
          tls->connection.inRecordCounter[1] = 1;
          tls->connection.inCounterMask = 0;
        }
        emTlsSetState(tls, action->nextState);
        return TLS_SUCCESS;
      } else {
        lose(SECURITY, TLS_WRONG_HANDSHAKE_MESSAGE);
      }
    }

    case TLS_CONTENT_ALERT:
      // If the alert is fatal we need to set the session ID length to
      // zero.
      // If the alert is a close_notify we need to respond with our own
      // close_notify and close the connection.  Once we receive a
      // close_notify we can no longer be sure that the other end is
      // still open for receiving.
//    simPrint("received alert");
      if (tls->connection.state != TLS_CLOSING) {
        emTlsSendCloseNotify(tls);
        tls->connection.state = TLS_CLOSING;
        debug("sending close_notify");
      }
      return TLS_SUCCESS;

    case TLS_CONTENT_HANDSHAKE: {
      uint16_t handshakeHeaderLength = (tlsUsingDtls(tls)
                                        ? DTLS_HANDSHAKE_HEADER_LENGTH
                                        : TLS_HANDSHAKE_HEADER_LENGTH);

      incoming += recordHeaderLength;
      length -= recordHeaderLength;

      while (0 < length) {
        uint16_t handshakeLength =
          HIGH_LOW_TO_INT(incoming[2], incoming[3])
          + handshakeHeaderLength;

        if (length < handshakeLength
            || incoming[1] != 0) { // we should not see lengths > 0xFFFF.
          lose(SECURITY, TLS_BAD_RECORD);
        }

        // We do not support renegotiation, so we ignore handshake messages
        // once the handshake is done.  Per RFC 5246 7.4.1.1. we are allowed
        // to ignore renegotiation requests.
        if (tlsHasHandshake(tls)) {
          if (tlsUsingDtls(tls)) {
            uint16_t gotCounter = HIGH_LOW_TO_INT(incoming[4], incoming[5]);
            uint16_t haveCounter =
              ((DtlsState *) tls)->dtlsHandshake.inHandshakeCounter;
            if (gotCounter == haveCounter
                || tls->connection.state == TLS_SERVER_EXPECT_HELLO) {
              ((DtlsState *) tls)->dtlsHandshake.inHandshakeCounter
                = gotCounter + 1;
            } else {
              debug("handshake count: got %d expected %d",
                    gotCounter,
                    haveCounter);
              lose(SECURITY, TLS_WRONG_HANDSHAKE_MESSAGE);
            }
          }
          TlsStatus status = emRunHandshake(tls, incoming, handshakeLength);
          if (status != TLS_SUCCESS) {
            lose(SECURITY, status);
          }
        }
        length -= handshakeLength;
        incoming += handshakeLength;
        if (length != 0) {
          debug("additional handshake");
        }
      }
      break;
    }
    default:
      assert(false);
  }

  return TLS_SUCCESS;
}

TlsStatus emRunHandshake(TlsState *tls, uint8_t *incoming, uint16_t length)
{
  uint8_t state = tls->connection.state;
  StateAction const *action = stateActions + state;
  uint16_t handshakeHeaderLength = (tlsUsingDtls(tls)
                                    ? DTLS_HANDSHAKE_HEADER_LENGTH
                                    : TLS_HANDSHAKE_HEADER_LENGTH);
  TlsStatus status;

  // determine if there's enough buffer space to run the handshake
  uint16_t allocated = (tls->connection.flags & TLS_HAVE_JPAKE
                        ? action->jpakeAllocated
                        : ((tls->connection.flags & TLS_HAVE_DHE_RSA)
                           ? action->rsaAllocated
                           : action->pskAllocated));

  if (0 < allocated && !emSetReservedBufferSpace(allocated)) {
    lose(SECURITY, TLS_NO_BUFFERS);
  }

  if (incoming != NULL) {
    uint8_t command = incoming[0];
    debug("incoming handshake: state %s message %s",
          tlsStateName(state), tlsHandshakeName(command));

    if (!(action->expectedMessage == TLS_HANDSHAKE_MULTIPLE
          || action->expectedMessage == command)) {
      emLogCodePoint(SECURITY, "lost");
      status = TLS_WRONG_HANDSHAKE_MESSAGE;
      goto kickout;
    }

    // Need to set this aside before adding the incoming message.
    // This could be encoded in the high bit of the expected message.
    if (!(state == TLS_SERVER_EXPECT_CERTIFICATE_VERIFY
          || state == TLS_CLIENT_EXPECT_FINISHED
          || state == TLS_SERVER_EXPECT_FINISHED)) {
      dump("tls sha256 in", incoming, length);
      emSha256HashBytes(&tls->handshake.messageHash, incoming, length);
    }
  }

  tls->connection.state = action->nextState;
  if (action->handler(tls,
                      incoming,
                      incoming + handshakeHeaderLength,
                      length - handshakeHeaderLength)
      != EMBER_SUCCESS) {
    emLogCodePoint(SECURITY, "lost");
    emTlsSetState(tls, TLS_CLOSED);
    status = TLS_MESSAGE_PROCESSING_FAILURE;
    goto kickout;
  }

  if (action->expectedMessage == 0xFF) {
    // This has to happen after the message has been sent.
    // DTLS should increment the epoch as well.
    if (state == TLS_CLIENT_SEND_CHANGE_CIPHER_SPEC
        || state == TLS_SERVER_SEND_CHANGE_CIPHER_SPEC) {
      tlsSetFlag(tls, TLS_ENCRYPT);
      MEMSET(tls->connection.outRecordCounter, 0, 8);
      if (tlsUsingDtls(tls)) {
        // For DTLS this starts the next epoch.
        tls->connection.outRecordCounter[1] = 1;
      }
    }
  }

  emTlsSetState(tls, tls->connection.state);
  status = TLS_SUCCESS;

  kickout:
  emEndBufferSpaceReservation();
  return status;
}

void emTlsSetState(TlsState *tls, uint8_t state)
{
  assert(state == TLS_CLOSED || state < COUNTOF(stateActions));
  tls->connection.state = state;
  emLogLine(SECURITY, "new state %s",
            tlsStateName(tls->connection.state));
  if (stateActions[state].expectedMessage == 0xFF) {
    tlsClearFlag(tls, TLS_WAITING_FOR_INPUT);
  } else {
    tlsSetFlag(tls, TLS_WAITING_FOR_INPUT);
  }
}
