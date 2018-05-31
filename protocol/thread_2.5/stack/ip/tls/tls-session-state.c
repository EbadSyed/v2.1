/*
 * File: tls-session-state.c
 * Description: saving TLS sessions
 * Author(s): Richard Kelsey
 *
 * Copyright 2011 by Ember Corporation. All rights reserved.                *80*
 */

#include "core/ember-stack.h"
#include "hal/hal.h"
#include "ip/udp.h"
#include "ip/tcp.h"
#include "tls.h"
#include "tls-handshake.h"
#include "tls-session-state.h"

// We currently only save one session.

Buffer emSavedTlsSessions = NULL_BUFFER;

// If this is true we pretend that the client and server already have
// a shared session.  This is not secure.

bool emUsePresharedTlsSessionState = false;

// If sessionId is NULL, this matches on remoteIpAddress only.
// Session can be NULL, in which case no data is copied (but the
// return value is still useful).

static bool findTlsSession(TlsSessionState *session,
                           EmberIpv6Address *remoteIpAddress,
                           uint8_t *sessionId,
                           uint16_t sessionIdLength)
{
  if (emSavedTlsSessions != NULL_BUFFER) {
    EmberIpv6Address *savedAddress =
      (EmberIpv6Address *) emGetBufferPointer(emSavedTlsSessions);
    TlsSessionState *savedState =
      (TlsSessionState *) (((uint8_t *) savedAddress) + sizeof(EmberIpv6Address));
    if (MEMCOMPARE(savedAddress, remoteIpAddress, sizeof(EmberIpv6Address)) == 0
        && (sessionId == NULL
            || (savedState->idLength != 0
                && savedState->idLength == sessionIdLength
                && (MEMCOMPARE(sessionId, savedState->id, sessionIdLength)
                    == 0)))) {
      if (session != NULL) {
        MEMCOPY(session, savedState, sizeof(TlsSessionState));
      }
      return true;
    }
  }
  return false;
}

void emSaveTlsState(EmberIpv6Address *remoteIpAddress,
                    TlsSessionState *session)
{
  if (emSavedTlsSessions == NULL_BUFFER) {
    emSavedTlsSessions = emAllocateBuffer(sizeof(EmberIpv6Address)
                                          + sizeof(TlsSessionState));
    if (emSavedTlsSessions == NULL_BUFFER) {
      return;
    }
  }
  {
    uint8_t *contents = emGetBufferPointer(emSavedTlsSessions);
    MEMCOPY(contents,
            remoteIpAddress,
            sizeof(EmberIpv6Address));
    MEMCOPY(contents + sizeof(EmberIpv6Address),
            session,
            sizeof(TlsSessionState));
  }
}

// If sessionId is NULL this is being used on the client to find the session
// to restore.  If sessionId is non-NULL this is is being used on the server
// to find a session that matches that of the client.

bool emRestoreTlsSession(TlsSessionState *session,
                         EmberIpv6Address *remoteIpAddress,
                         uint8_t *sessionId,
                         uint16_t sessionIdLength)
{
  if (emSavedTlsSessions != NULL_BUFFER) {
    EmberIpv6Address *savedAddress =
      (EmberIpv6Address *) emGetBufferPointer(emSavedTlsSessions);
    TlsSessionState *savedState =
      (TlsSessionState *) (((uint8_t *) savedAddress) + sizeof(EmberIpv6Address));
    if (MEMCOMPARE(savedAddress, remoteIpAddress, sizeof(EmberIpv6Address)) == 0
        && (sessionId == NULL
            || (savedState->idLength != 0
                && savedState->idLength == sessionIdLength
                && (MEMCOMPARE(sessionId, savedState->id, sessionIdLength)
                    == 0)))) {
      MEMCOPY(session, savedState, sizeof(TlsSessionState));
      return true;
    }
  }
  if (emUsePresharedTlsSessionState) {
    uint16_t i;
    if (sessionId == NULL) {
      session->idLength = 16;
      for (i = 0; i < 16 / 2; i++) {
        session->id[i] = halCommonGetRandom();
      }
    } else {
      session->idLength = sessionIdLength;
      MEMMOVE(session->id, sessionId, sessionIdLength);
    }
    for (i = 0; i < TLS_MASTER_SECRET_SIZE; i += 16) {
      MEMMOVE(session->master + i, session->id, 16);
    }
    return true;
  } else {
    return false;
  }
}

// Application interface.  These are declard in tcp.h.

EmberStatus emberTcpSaveTlsState(uint8_t fd)
{
  TcpConnection *connection = emGetFdConnection(fd);

  if (connection == NULL
      || connection->tlsState == NULL_BUFFER) {
    return EMBER_BAD_ARGUMENT;
  }

  TlsState *state = connectionTlsState(connection);
  if (state != NULL) {
    emSaveTlsState((EmberIpv6Address *) connection->udpData.remoteAddress,
                   &state->session);
    return EMBER_SUCCESS;
  }
  return EMBER_BAD_ARGUMENT;
}

void emberDiscardTlsSession(EmberIpv6Address *remoteIpAddress)
{
  if (findTlsSession(NULL, remoteIpAddress, NULL, 0)) {
    emSavedTlsSessions = NULL_BUFFER;
  }
}
