/*
 * File: tls-session-state.h
 * Description: saving TLS sessions
 * Author(s): Richard Kelsey
 *
 * Copyright 2011 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef TLS_SESSION_STATE_H
#define TLS_SESSION_STATE_H

extern bool emUsePresharedTlsSessionState;

void emSaveTlsState(EmberIpv6Address *remoteIpAddress,
                    TlsSessionState *session);

bool emRestoreTlsSession(TlsSessionState *session,
                         EmberIpv6Address *remoteIpAddress,
                         uint8_t *sessionId,
                         uint16_t sessionIdLength);

#endif // TLS_SESSION_STATE_H
