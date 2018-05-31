/*
 * File: debug.c
 * Description: debug code for TLS
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#include "core/ember-stack.h"

#ifdef EMBER_TEST

int debugLevel = 0;

void debug(char *format, ...)
{
  if (0 < debugLevel) {
    va_list argPointer;

    fprintf(stderr, "[");
    va_start(argPointer, format);
    vfprintf(stderr, format, argPointer);
    va_end(argPointer);
    putc(']', stderr);
    putc('\n', stderr);
  }
}

void dump(char *name, const uint8_t *data, int length)
{
  if (0 < debugLevel) {
    int i;
    fprintf(stderr, "[%s %d", name, length);
    for (i = 0; i < length; i++) {
      fprintf(stderr, " %02X", data[i]);
    }
    fprintf(stderr, "]\n");
  }
}

#endif
//----------------------------------------------------------------

static const char * const tlsHandshakeNames[] =
{
  "HELLO_REQUEST",
  "CLIENT_HELLO",
  "SERVER_HELLO",
  "CLIENT_HELLO_VERIFY",
  "???", "???", "???", "???", "???", "???", "???",
  "CERTIFICATE",
  "SERVER_KEY_EXCHANGE",
  "CERTIFICATE_REQUEST",
  "SERVER_HELLO_DONE",
  "CERTIFICATE_VERIFY",
  "CLIENT_KEY_EXCHANGE",
  "???", "???", "???",
  "FINISHED"
};

const char *tlsHandshakeName(uint8_t id)
{
  if ((sizeof(tlsHandshakeNames) / sizeof(char *)) <= id) {
    return "???";
  } else {
    return tlsHandshakeNames[id];
  }
}

static const char * const tlsStateNames[] =
{
  "TLS_SERVER_SEND_HELLO_REQUEST",
  "TLS_CLIENT_EXPECT_HELLO_REQUEST",

  "TLS_CLIENT_SEND_HELLO",
  "TLS_SERVER_EXPECT_HELLO",

  "TLS_SERVER_SEND_HELLO",
  "TLS_SERVER_SEND_CERTIFICATE",
  "TLS_SERVER_SEND_KEY_EXCHANGE",
  "TLS_SERVER_SEND_CERTIFICATE_REQUEST",
  "TLS_SERVER_SEND_HELLO_DONE",

  "TLS_CLIENT_EXPECT_HELLO",
  "TLS_CLIENT_EXPECT_CERTIFICATE",
  "TLS_CLIENT_EXPECT_KEY_EXCHANGE",
  "TLS_CLIENT_EXPECT_CERTIFICATE_REQUEST",
  "TLS_CLIENT_EXPECT_HELLO_DONE",

  "TLS_CLIENT_SEND_CERTIFICATE",
  "TLS_CLIENT_SEND_KEY_EXCHANGE",
  "TLS_CLIENT_SEND_CERTIFICATE_VERIFY",
  "TLS_CLIENT_SEND_CHANGE_CIPHER_SPEC",
  "TLS_CLIENT_SEND_FINISHED",

  "TLS_SERVER_EXPECT_CERTIFICATE",
  "TLS_SERVER_EXPECT_KEY_EXCHANGE",
  "TLS_SERVER_EXPECT_CERTIFICATE_VERIFY",
  "TLS_SERVER_EXPECT_CHANGE_CIPHER_SPEC",
  "TLS_SERVER_EXPECT_FINISHED",

  "TLS_SERVER_SEND_CHANGE_CIPHER_SPEC",
  "TLS_SERVER_SEND_FINISHED",

  "TLS_CLIENT_EXPECT_CHANGE_CIPHER_SPEC",
  "TLS_CLIENT_EXPECT_FINISHED",

  "TLS_HANDSHAKE_DONE",

  "TLS_CLOSING",
  "TLS_CLOSED",
  "TLS_UNINITIALIZED"
};

const char *tlsStateName(uint8_t state)
{
  return tlsStateNames[state];
}
