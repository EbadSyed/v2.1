/*
 * File: dtls.c
 * Description: DTLS
 * Author(s): Richard Kelsey
 *
 * Copyright 2012 by Ember Corporation. All rights reserved.                *80*
 */

// This is TLS 1.2 as specified in RFC 6347.

#include "core/ember-stack.h"
#include "hal/hal.h"
#include "phy/phy.h"
#include "mac/mac-header.h"
#include "framework/ip-packet-header.h"
#include "framework/buffer-management.h"
#include "ip/ip-header.h"
#include "ip/dispatch.h"
#include "ip/udp.h"
#include "ip/tcp.h"
#include "ip/commission.h"
#include "tls.h"
#include "tls-record.h"
#include "tls-handshake.h"
#include "debug.h"
#include "dtls.h"

extern void emSendJoinerEntrust(DtlsConnection *connection, const uint8_t *key);

// How long we wait after sending the final handshake message before discarding
// the handshake data.
#define MAX_SEGMENT_LIFETIME_SECONDS 30

#define DTLS_JOIN_CLOSE_TIMEOUT_QS  120

// The handshake looks a lot like TLS, and a lot like any UDP resend mechanism.
// Should we have some central doohicky for these?  PANA would be the other
// immediate user.  Except that DTLS handshake retries are not exact duplicates.
// The record layer counter goes up.
//
// Gathering flights
//  Each TLS record must fit in a single datagram.  This means that
//  some handshake messages are split over multiple records/datagrams.
//  We could also collect several small handshake messages into a
//  single record/datagram.
//  The fragmentation is a requirement:
//   - Pick a maximum fragment size < 1280 (could use the same value
//     as EAP (940))
//   - If emAllocateTlsHandshakeBuffer() gets a request for something
//     larger than this, it allocates a buffer that has room for an
//     extra TLS record header and enough slack to split the buffer.
//   - At send time, the buffer is split in two and the new record
//     header is added.
//  It's actually messier because the large record is the certificate
//  chain, which is prepackaged and sent using an indirect buffer.
//  But SE2 is the one that is using the huge chains, and they don't
//  want DTLS.  So no fragmentation is needed yet.
//
//  Multiple messages could be merged by removing the later headers,
//  adjusting the length of the first header, and then linking the
//  buffers using payload links.

//----------------------------------------------------------------
// DTLS retry engine - not yet implemented
//
// A. Prepare next flight
//     send next flight        -> B
//     send last flight        -> C
// B. Set timer
//     receive next flight     -> A
//     receive last flight     -> C
//     timeout                 -> resend, B
//     receive previous flight -> resend, B
// C. done
//     receive previous flight -> resend, C
//
// How hard is it to tell if a full flight has arrived?  We don't want to
// process part of a flight.  It's not too bad, you walk down checking the
// handshake type.  We do need to reorder messages and reorder and reassemble
// fragments.  Need an incoming queue as well as an outgoing one.
//
// The TLS code already handles a queue of partial messages.  We need to
// collect the flight and then convert it to a queue that doesn't have
// the internal headers.

// These are the flights - simplest is to have the message handlers release
// the previous flight.  It would be good to make the system as bullet proof
// as possible, so that once we start processing a message we know that, if
// the message is correct, we will complete the processing.
// 1     ---ClientHello-->
//      client waits for state higher than TLS_CLIENT_EXPECT_HELLO_DONE
// 2     <--ServerHello, ..., ServerHelloDone--
//      server waits for state higher than TLS_SERVER_EXPECT_FINISHED
// 3     ---[Certificate], ClientKeyExchange, ..., Finished-->
//      client waits for TLS_HANDSHAKE_DONE
// 4     <--ChangeCipherSpec, Finished---
//      server waits for timeout

// From RFC6347: Implementations SHOULD use an initial timer value of
//   1 second (the minimum defined in RFC 6298 [RFC6298]) and double the
//   value at each retransmission, up to no less than the RFC 6298
//   maximum of 60 seconds.
//
// We wait a bit longer if any public key calculations are required.
// We really need two PSK timeouts, one for joining, where there is no
// significant travel delay, and one for normal DTLS, where there is.
#define PSK_FLIGHT_TIMEOUT_QS 4         // 1 second
// This is from the Thread spec.
#define ECC_FLIGHT_TIMEOUT_QS 32        // 8 seconds
uint16_t emDtlsRetryTimeoutQs = ECC_FLIGHT_TIMEOUT_QS;

// This needs to depend on the initial timeout.  PSK can get more resends.
#define MAX_RESENDS 3

// This needs to use the resend count in order to increase the timeout.
void startMessageTimeout(DtlsConnection *connection)
{
  DtlsState *dtls = (DtlsState *) connectionDtlsState(connection);
  uint16_t baseTime = 0;
  if (dtls != NULL) {
    baseTime = (dtls->connection.flags & TLS_HAVE_JPAKE
                ? emDtlsRetryTimeoutQs
                : (dtls->connection.flags & TLS_HAVE_PSK
                   || (dtls->connection.state
                       == TLS_CLIENT_EXPECT_CHANGE_CIPHER_SPEC)
                   ? PSK_FLIGHT_TIMEOUT_QS
                   : ECC_FLIGHT_TIMEOUT_QS));
  }
//  simPrint("starting DTLS timer %d for %d", connection->udpData.connection,
//           (uint16_t) baseTime << connection->resendCount);
//  debug("starting DTLS timer");
// All but the last client state have to allow for public key computations.
  emStartConnectionTimerQs((TcpConnection *) connection,
                           baseTime << connection->resendCount);
}

static void sendDtlsRecords(DtlsConnection *connection,
                            bool isHandshake)
{
  DtlsState *dtls = (DtlsState *) connectionDtlsState(connection);
  Buffer payload = connection->outgoingData;

  emPayloadQueueToLinkedPayload(&payload);
  emSubmitDtlsPayload(connection, payload);
  emLogLine(SECURITY, "DTLS send new flight");

  if (dtls != NULL) {
    if (isHandshake) {
      connection->resendCount = 0;
      // Resending requires changing the sequence numbers, which is OK except
      // for the finished message, which is encrypted.  Need to save the
      // finished hash somewhere so that we can regenerate the message.
      // Would it be possible to regenerate all of the messages?  Our state
      // mechanism saves everything from the incoming messages before sending
      // anything.  No, it would mean redoing the public key signing.
      dtls->dtlsHandshake.outgoingFlight = payload;
    }

    if (tlsIsDtlsJoin(dtls) && !tlsHasHandshake(dtls)) {
      emStartConnectionTimerQs((TcpConnection *) connection,
                               DTLS_JOIN_CLOSE_TIMEOUT_QS);
    }

    connection->outgoingData = NULL_BUFFER;
  }
}

void emSubmitDtlsPayload(DtlsConnection *connection, Buffer payload)
{
  const uint8_t *destination = connection->udpData.remoteAddress;

  emLogBytesLine(SECURITY,
                 "sending DTLS packet to port %u from port %u to: ",
                 destination,
                 16,
                 connection->udpData.remotePort,
                 connection->udpData.localPort);

  if (connection->udpData.flags & UDP_DTLS_RELAYED_JOIN) {
    emSubmitRelayedJoinPayload(connection, payload);
    return;
  } else if (connection->udpData.flags & UDP_DTLS_JOIN_KEK) {
    emSendJoinerEntrust(connection, NULL);
  }

  uint8_t options = ((connection->udpData.flags & UDP_DTLS_JOIN)
                     ? IP_HEADER_LL64_SOURCE
                     : 0);
  Ipv6Header ipHeader;
  PacketHeader header = emMakeUdpHeader(&ipHeader,
                                        options,
                                        destination,
                                        IPV6_DEFAULT_HOP_LIMIT,
                                        connection->udpData.localPort,
                                        connection->udpData.remotePort,
                                        NULL,
                                        0,
                                        emTotalPayloadLength(payload));

  // BUG: what to do?
  assert(header != NULL_BUFFER);

  if (connection->udpData.flags & UDP_DTLS_JOIN) {
    emSetMacFrameControl(header,
                         (emGetMacFrameControl(header)
                          & ~(MAC_FRAME_FLAG_SECURITY_ENABLED
                              | MAC_FRAME_VERSION_2006)));
  }
  emSetPayloadLink(header, payload);
  emSubmitIpHeader(header, &ipHeader);
}

static void resendFlight(DtlsConnection *connection)
{
  DtlsState *dtls = (DtlsState *) connectionDtlsState(connection);
  emLogLine(SECURITY, "DTLS resend number %d", connection->resendCount);
  connection->resendCount += 1;
  emUpdateOutgoingFlight(dtls);
  if (dtls != NULL) {
    emSubmitDtlsPayload(connection, dtls->dtlsHandshake.outgoingFlight);
  }
}

// From RFC6347: In addition, for at least twice the default MSL
//    defined for [TCP], when in the FINISHED state, the node
//    that transmits the last flight (the server in an ordinary
//    handshake or the client in a resumed handshake) MUST
//    respond to a retransmit of the peer's last flight with a
//    retransmit of the last flight.
// This is a long time for us to hang on to all that data.  We
// need to discard everything but what is needed to
// the last flight.

// After completion, server resends at most once a second.
#define SERVER_FINAL_TIMEOUT 4
#define MAX_FINAL_SERVER_RESENDS \
  ((MAX_SEGMENT_LIFETIME_SECONDS * 4 * 2) / SERVER_FINAL_TIMEOUT)

void emDtlsConnectionTimeout(TcpConnection *connection)
{
  DtlsConnection *dtlsConnection = (DtlsConnection *) connection;
  TlsState *tls = connectionDtlsState(connection);
//  simPrint("DTLS timeout %d", connection->udpData.connection);
  if (tls != NULL) {
    if (TLS_HANDSHAKE_DONE <= tls->connection.state) {
      debug("DTLS timeout with handshake complete %d", tlsAmClient(tls));
      if (tlsIsDtlsJoin(tls)
          && !tlsIsNativeCommission(tls)
          && !tlsHasHandshake(tls)) {
        // Cleaning up unused joining connections.
        connection->udpData.events |= EMBER_TCP_DONE;
      } else if (tlsAmClient(tls)) {
        // nothing to do
      } else if (MAX_FINAL_SERVER_RESENDS <= dtlsConnection->resendCount) {
        debug("DTLS state truncate");
        emTruncateTlsStateBuffer(connection->tlsState);
      } else {
        if (connection->udpData.flags & UDP_DTLS_NEED_RESEND) {
          connection->udpData.flags &= ~UDP_DTLS_NEED_RESEND;
          resendFlight((DtlsConnection *) connection);
        } else {
          connection->resendCount += 1;
        }
        emStartConnectionTimerQs(connection, SERVER_FINAL_TIMEOUT);
      }
    } else if (dtlsConnection->resendCount < MAX_RESENDS) {
      debug("DTLS resend");
      resendFlight(dtlsConnection);
      startMessageTimeout(dtlsConnection);
    } else {
      debug("DTLS timeout");
      connection->udpData.events |= (EMBER_TCP_OPEN_FAILED | EMBER_TCP_DONE);
    }
  }
}

// This is used to remove idle DTLS join sessions.

void emCheckForIdleDtlsConnection(TcpConnection *connection)
{
  TlsState *tls = connectionDtlsState(connection);
  if (tls != NULL) {
    if (tlsIsDtlsJoin(tls) && !tlsHasHandshake(tls)) {
      connection->udpData.ticksRemaining = DTLS_JOIN_CLOSE_TIMEOUT_QS;
    }
  }
}

void emUdpSendDtlsRecord(EmberUdpConnectionHandle handle, Buffer tlsRecord)
{
  DtlsConnection *connection =
    (DtlsConnection *) emFindConnectionFromHandle(handle);
  TlsState *tls = connectionDtlsState(connection);
  uint8_t *bufferPointer = emGetBufferPointer(tlsRecord);
  uint8_t recordType = 0;
  if (bufferPointer != NULL) {
    recordType = bufferPointer[0];
  }

  if (tls != NULL) {
    if (recordType == TLS_CONTENT_APPLICATION_DATA
        && tls->connection.state != TLS_HANDSHAKE_DONE) {
      // sent too early, drop it
    } else {
      // This is duplicated from tcp.c, so it should be in a function.
      do {
        Buffer next = emGetPayloadLink(tlsRecord);
        emPayloadQueueAdd(&connection->outgoingData, tlsRecord);
        tlsRecord = next;
      } while (tlsRecord != NULL_BUFFER);

      if (!tlsInDtlsHandshake(tls)) {
        sendDtlsRecords(connection, false);
      }
    }
  }
}

void emOpenDtlsConnection(DtlsConnection *connection)
{
  TlsState *tls = connectionDtlsState(connection);
  if (tls != NULL) {
    emTlsSetState(tls, TLS_CLIENT_SEND_HELLO);
    tlsSetFlag(tls, TLS_AM_CLIENT);
    tlsSetFlag(tls, TLS_VERIFY_REQUIRED);
    emStartConnectionTimerQs((TcpConnection *) connection,
                             (tls->connection.flags & TLS_HAVE_PSK
                              ? PSK_FLIGHT_TIMEOUT_QS
                              : (tls->connection.flags & TLS_HAVE_JPAKE
                                 ? emDtlsRetryTimeoutQs
                                 : ECC_FLIGHT_TIMEOUT_QS)));
    emDtlsStatusHandler(connection);
  }
}

void emOpenDtlsServer(DtlsConnection *connection)
{
  TlsState *tls = connectionDtlsState(connection);
  if (tls != NULL) {
    emTlsSetState(tls, TLS_SERVER_EXPECT_HELLO);
    tlsSetFlag(tls, TLS_VERIFY_REQUIRED);
  }
}

void emCloseDtlsConnection(DtlsConnection *connection)
{
  TlsState *tls = connectionDtlsState(connection);

  if (tls != NULL) {
    if (tls->connection.state == TLS_HANDSHAKE_DONE) {
      emTlsSendCloseNotify(tls);
      tls->connection.state = TLS_CLOSING;
    }
    connection->udpData.ticksRemaining = 0;
    if (!(connection->udpData.flags & UDP_CONNECTED)) {
      connection->udpData.events |= EMBER_TCP_OPEN_FAILED;
    }
    connection->udpData.events |= EMBER_TCP_DONE;
    emStartConnectionTimerMs(0);
  }
}

extern EmberStatus emFinishDtlsServerJoin(DtlsConnection *connection);

// Fow now we assume that each flight fits in a single UDP message.

void emDtlsStatusHandler(DtlsConnection *connection)
{
  TlsState *tls = connectionDtlsState(connection);
  if (tls != NULL) {
    bool done = tls->connection.state == TLS_HANDSHAKE_DONE;
    TlsStatus status = TLS_SUCCESS;
    Buffer *incoming = &connection->incomingTlsData;

    if (tls->connection.state == TLS_CLOSED) {
      *incoming = NULL_BUFFER;     // drop any incoming data
      goto kickout;
    }

    while (status == TLS_SUCCESS
           && *incoming != NULL_BUFFER) {
      uint8_t initialState = tls->connection.state;
      Buffer initialBuffer = emBufferQueueHead(incoming);
      bool messageStart = true;

      if (tls->connection.state == TLS_HANDSHAKE_DONE) {
        status = emReadAndProcessRecord(tls, incoming);
        // Once we have successfully received an encrypted message we know
        // that no more handshake retries are needed.
        if (status == TLS_SUCCESS
            && tls->connection.state == TLS_HANDSHAKE_DONE
            && connection->udpData.ticksRemaining != 0) {
          emTruncateTlsStateBuffer(connection->tlsState);
          connection->udpData.ticksRemaining = 0;
        }
      } else {
        // Temporary hack until DTLS retries are working.  We can't afford
        // to drop messages while processing the final handshake message.
        // We can't afford to drop them at other times either, but this
        // is when it tends to happen.
        // TODO: DTLS retries are working.  Do we still need this?
        if (tls->connection.state == TLS_CLIENT_EXPECT_FINISHED) {
          status = emReadAndProcessRecord(tls, incoming);
        } else {
          DROP_INCOMING_PACKETS(
            status = emReadAndProcessRecord(tls, incoming);
            );
        }

        if (messageStart
            && tls->connection.state == initialState
            && emBufferQueueHead(incoming) == initialBuffer) {
          // We received some message other than the expected next flight,
          // so drop it.
          emBufferQueueRemoveHead(incoming);
        }
        messageStart = (*incoming != NULL_BUFFER
                        && emBufferQueueHead(incoming) != initialBuffer);
      }
    }

    if (tlsHasHandshake(tls)) {
      if (status == TLS_DECRYPTION_FAILURE
          && (tls->connection.state == TLS_SERVER_EXPECT_FINISHED
              || tls->connection.state == TLS_CLIENT_EXPECT_FINISHED)) {
        emTlsSendAlarm(tls, TLS_ALERT_LEVEL_FATAL, TLS_ALERT_BAD_RECORD_MAC);
      } else if (status != TLS_SUCCESS
                 && ((DtlsState *) tls)->dtlsHandshake.outgoingFlight != NULL_BUFFER
                 && tls->connection.state == TLS_HANDSHAKE_DONE) {
        // Respond to all other failures by resending the last flight.
        connection->udpData.flags |= UDP_DTLS_NEED_RESEND;
      }
    }

    while (status == TLS_SUCCESS
           && (emPayloadQueueByteLength(&connection->outgoingData)
               <= 900)           // don't get too big
           && !(tlsWaitingForInput(tls)
                || tls->connection.state == TLS_HANDSHAKE_DONE)) {
      DROP_INCOMING_PACKETS(status = emRunHandshake(tls, NULL, 0); );
    }

    if (!emBufferQueueIsEmpty(&connection->outgoingData)) {
      if (!done
          && tls->connection.state == TLS_HANDSHAKE_DONE
          && !tlsAmClient(tls)
          && tlsIsDtlsJoin(tls)) {
        if (emFinishDtlsServerJoin(connection) != EMBER_SUCCESS) {
          // what to do?
        }
      }
      sendDtlsRecords(connection, true);

      // These are the waiting-for-a-flight states.
      if (tls->connection.state == TLS_SERVER_EXPECT_KEY_EXCHANGE
          || tls->connection.state == TLS_SERVER_EXPECT_CERTIFICATE
          || tls->connection.state == TLS_CLIENT_EXPECT_HELLO
          || tls->connection.state == TLS_CLIENT_EXPECT_CHANGE_CIPHER_SPEC) {
        startMessageTimeout(connection);
      }
    }

    if (!done
        && tls->connection.state == TLS_HANDSHAKE_DONE) {
      connection->udpData.events |= EMBER_TCP_OPENED;
      connection->udpData.flags |= UDP_CONNECTED;
      tls->connection.flags &= ~TLS_IN_DTLS_HANDSHAKE;

      // Let the application know that the handshake is complete.
      // (in case we go ahead and process application data below)
      emReportTcpStatus((TcpConnection *) connection);

      // The last handshake message is server->client, so when the client
      // receives its half the handshake is complete.  The server doesn't
      // know if the client received that last message, so it has to wait
      // before discarding the handshake data.
      if (tlsAmClient(tls)) {
        emStartConnectionTimerMs(0);
        emTruncateTlsStateBuffer(connection->tlsState);
      } else {
        emStartConnectionTimerQs((TcpConnection *) connection,
                                 SERVER_FINAL_TIMEOUT);
      }
    }

    // This handles the case where the DTLS connection was closed remotely.
    if (tls->connection.state == TLS_CLOSING) {
      emCloseDtlsConnection(connection);
    }

    if (tls->connection.state == TLS_CLOSED) {
      connection->incomingTlsData = NULL_BUFFER;     // drop any incoming data
      connection->udpData.events |= EMBER_TCP_DONE;  // shut down the connection
      emStartConnectionTimerMs(0);
    } else if (tls->connection.state == TLS_HANDSHAKE_DONE) {
      while (connection->incomingData != NULL_BUFFER) {
        Buffer payload =
          emBufferQueueRemoveHead(&(connection->incomingData));
        ((UdpConnectionReadHandler)
         // Not all handlers want the last argument, but passing to those that
         // don't doesn't affect anything.
         connection->udpData.readHandler)((EmberUdpConnectionData *) connection,
                                          emGetBufferPointer(payload),
                                          emGetBufferLength(payload),
                                          payload);
      }
    }

    kickout:

    emLogLine(SECURITY, "DTLS exit %u, state %u",
              connection->udpData.connection,
              tls->connection.state);
    emMicroBusy(false);
    return;
  }
}

void emSetJpakeKey(DtlsConnection *connection,
                   const uint8_t *key,
                   uint8_t keyLength)
{
  assert(keyLength <= JPAKE_MAX_KEY_LENGTH);
  DtlsState *dtls = (DtlsState *) connectionDtlsState(connection);
  TlsHandshakeState *handshake = &dtls->handshake;
  MEMCOPY(handshake->jpakeKey, key, keyLength);
  handshake->jpakeKeyLength = keyLength;
}
