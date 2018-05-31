/*
 * File: tls-jpake.c
 * Description: JPAKE-specific parts of TLS.
 *
 * Copyright 2015 Silicon Laboratories, Inc.                                *80*
 */

#include "core/ember-stack.h"
#include "phy/phy.h"

#include "tls.h"
#include "tls-handshake-crypto.h"
#include "tls-record.h"
#include "tls-sha256.h"
#include "debug.h"

#ifdef EMBER_TEST
#include <netinet/in.h>
#include "native-test-util.h"
#endif

#include "tls-handshake.h"
#include "tls-public-key.h"
#include "jpake-ecc.h"

EmberStatus emSendJpakeServerKeyExchange(TlsState *tls)
{
  Buffer buffer;
  uint8_t *params;
  uint8_t skxb[ECC_JPAKE_CURVE_PARAMS_LENGTH + ECC_JPAKE_MAX_DATA_LENGTH];
  uint16_t length = 0;

  if (!emJpakeEccGetCkxaOrSkxbData(skxb, sizeof(skxb), &length)) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  buffer = emAllocateTlsHandshakeBuffer(tls,
                                        TLS_HANDSHAKE_SERVER_KEY_EXCHANGE,
                                        length,
                                        &params);

  if (buffer == NULL_BUFFER) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }

  MEMCOPY(params, skxb, length);
  tls->connection.state = TLS_SERVER_SEND_HELLO_DONE;
  return emTlsSendBuffer(tls, buffer);
}

EmberStatus emProcessJpakeServerKeyExchange(TlsState *tls,
                                            uint8_t *incoming,
                                            uint16_t length)
{
  if (!emJpakeEccVerifyCkxaOrSkxbData(incoming, length)) {
    lose(SECURITY, EMBER_ERR_FATAL);
  }
  tls->connection.state = TLS_CLIENT_EXPECT_HELLO_DONE;
  return EMBER_SUCCESS;
}
