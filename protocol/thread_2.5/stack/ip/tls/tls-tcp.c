/*
 * File: tls-tcp.c
 * Description: TLS handshake
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#if EMBER_STACK_COBRA
#define jmp_buf uint8
#define setjmp(buf) 0
#define longjmp(buf, error) assert(0)
#else
#include <setjmp.h>
#endif

#include "core/ember-stack.h"
#include "ip/udp.h"
#include "ip/tcp.h"
#include "tls.h"
#include "tls-record.h"
#include "tls-handshake.h"
#include "tls-session-state.h"
#include "debug.h"

// This should have a throw for errors and closing.

static jmp_buf *tlsError;

// The return value cannot be zero, and zero is a legal code, so we
// invert the bits before returning.

void emTlsError(uint8_t code)
{
  assert(tlsError != NULL);
  longjmp(*tlsError, ~code);
}

// Called from the TCP code when a TCP event has occured.

void emStartTlsResume(TcpConnection *connection)
{
  TlsState *tls = connectionTlsState(connection);
  if (emRestoreTlsSession(&tls->session,
                          ((EmberIpv6Address *) connection->udpData.remoteAddress),
                          NULL,
                          0)) {
    tls->connection.flags |= TLS_IS_RESUME;
  }
}

// Returns true if the TCP code should continue processing as if
// there were no TLS connection (because the TLS connection has
// shut down).

bool emTlsStatusHandler(TcpConnection *connection)
{
  TlsState *tls = connectionTlsState(connection);
  jmp_buf tlsErrorBuffer;
  bool result = false;
  int errorCode;

  tlsError = &tlsErrorBuffer;
  errorCode = setjmp(tlsErrorBuffer);
  if (errorCode != 0) {
    errorCode = ~errorCode;
    emTlsSendAlarm(tls,
                   (errorCode == TLS_ALERT_CLOSE_NOTIFY
                    ? TLS_ALERT_LEVEL_WARNING
                    : TLS_ALERT_LEVEL_FATAL),
                   errorCode);
    tls->connection.state = TLS_CLOSED;
    emberTcpClose(connection->udpData.connection);
    tlsError = NULL;
    goto kickout;
  }

  if (tls->connection.state == TLS_CLOSED) {
    connection->incomingTlsData = NULL_BUFFER;     // drop any incoming data
//    if (connection->outgoingData == NULL_BUFFER) {
//      emberTcpClose(connection->udpData.connection);
//    }
    result = true;
    goto kickout;
  }

  if (connection->udpData.events & EMBER_TCP_OPENED) {
    emTlsSetState(tls, TLS_CLIENT_SEND_HELLO);
    tlsSetFlag(tls, TLS_AM_CLIENT);
    tlsSetFlag(tls, TLS_VERIFY_REQUIRED);
    connection->udpData.events &= ~EMBER_TCP_OPENED;
  } else if (connection->udpData.events & EMBER_TCP_ACCEPTED) {
    emTlsSetState(tls, TLS_SERVER_EXPECT_HELLO);
    tlsSetFlag(tls, TLS_VERIFY_REQUIRED);
    connection->udpData.events &= ~EMBER_TCP_ACCEPTED;
  } else if (connection->udpData.events & EMBER_TCP_OPEN_FAILED) {
    emberTcpStatusHandler(connection->udpData.connection, EMBER_TCP_OPEN_FAILED);
    connection->udpData.events &= ~EMBER_TCP_OPEN_FAILED;
  }

  // Don't do anything if the TCP connection hasn't completed.
  if (tls->connection.state == TLS_UNINITIALIZED) {
    goto kickout;
  }

  {
    bool done = tls->connection.state == TLS_HANDSHAKE_DONE;
    EmberStatus status = EMBER_SUCCESS;

    // It would be better to stop dropping incoming packets once the handshake
    // is complete.
    while (status == EMBER_SUCCESS
           && connection->incomingTlsData != NULL_BUFFER) {
      DROP_INCOMING_PACKETS(
        status = emReadAndProcessRecord(tls, &connection->incomingTlsData); );
    }
    while (status == EMBER_SUCCESS
           && (emPayloadQueueByteLength(&connection->outgoingData)
               <= TCP_MAX_SEGMENT)
           && !(tlsWaitingForInput(tls)
                || tls->connection.state == TLS_HANDSHAKE_DONE)) {
      DROP_INCOMING_PACKETS(status = emRunHandshake(tls, NULL, 0); );
    }
    if (!done
        && tls->connection.state == TLS_HANDSHAKE_DONE) {
      emTruncateTlsStateBuffer(connection->tlsState);
      if (tlsAmClient(tls)) {
        emberTcpStatusHandler(connection->udpData.connection, EMBER_TCP_OPENED);
      } else {
        emberTcpAcceptHandler(connection->udpData.localPort,
                              connection->udpData.connection);
      }
    }
  }

  if (connection->udpData.events & EMBER_TCP_TX_COMPLETE) {
    if (tls->connection.state == TLS_HANDSHAKE_DONE) {
      emberTcpStatusHandler(connection->udpData.connection, EMBER_TCP_TX_COMPLETE);
    } else if (tls->connection.state == TLS_CLOSING) {
      tls->connection.state = TLS_CLOSED;
      emberTcpClose(connection->udpData.connection);
    }
    connection->udpData.events &= ~EMBER_TCP_TX_COMPLETE;
  }

  emSendTcpDataToApp(connection);

  if (connection->udpData.events
      & (EMBER_TCP_REMOTE_CLOSE | EMBER_TCP_REMOTE_RESET)) {
    //    simPrint("remote TLS close");
    if (tls->connection.state != TLS_CLOSING) {
      tls->connection.state = TLS_CLOSED;
      emberTcpClose(connection->udpData.connection);
    }

    // shutdown TLS here, these are all not open/closed

    // If we are already closing then we have already told the application
    // that the connection is closed.
    // Do we bother with half-open connections?  It seems messy. -- Richard
    // No, RFC 5246, section 7.2.1, states that if we receive a close_notify
    // then the connection must be closed immediately -- Neonate
    // Yes, but that is refering to the TLS connection, not the underlying
    // transport connection.  RFC 5246 explicitly says that the application
    // can do whatever it likes with the transport connection:
    //   "No part of this standard should be taken to dictate the manner
    //    in which a usage profile for TLS manages its data transport,
    //    including when connections are opened or closed."  -- Richard
    // Should closing be a handshake state? -- Richard

    if (tls->connection.state != TLS_CLOSING) {
      emberTcpStatusHandler(connection->udpData.connection,
                            EMBER_TCP_REMOTE_CLOSE | EMBER_TCP_REMOTE_RESET);
    }
  }

  kickout:
  tlsError = NULL;
  return result;
}
