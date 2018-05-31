/*
 * File: mbedtls.c
 * Description: mbedtls utilities
 * Author(s): Suvesh Pratapa
 *
 * Copyright 2017 by Silicon Labs. All rights reserved.                *80*
 */

#include "stack/core/ember-stack.h"

#include "stack/ip/ip-address.h"
#include "stack/ip/zigbee/join.h"
#include "stack/ip/tls/dtls-join.h"
#include "stack/ip/udp.h"
#include "framework/event-queue.h"

#include MBEDTLS_CONFIG_FILE
#include "mbedtls/entropy.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_cookie.h"
#include "mbedtls/net.h"
#include "mbedtls/timing.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"
#include "mbedtls/error.h"
#include "mbedtls/platform.h"
#include "stack/ip/commission.h"
#include "stack/ip/commission-dataset.h"
#include "stack/ip/tls/mbedtls/mbedtls.h"
#include "stack/ip/tls/mbedtls/mbedtls-stack.h"
#include "stack/ip/tls/psk-table.h"
#include "stack/ip/tls/mbedtls/malloc/src/umm_malloc.h"

bool emEccFastSign = false;
bool emEccFastVerify = false;

// handshake event
static void emDtlsHandshakeEventHandler(Event *event);

static EventActions emDtlsHandshakeEventActions = {
  &emStackEventQueue,
  emDtlsHandshakeEventHandler,
  NULL,
  "DTLS handshake"
};

static Event emDtlsHandshakeEvent = { &emDtlsHandshakeEventActions, NULL };

#if defined(PHY_EM3XX)
  #define RETRY_HANDSHAKE_MS 1024
#else
  #define RETRY_HANDSHAKE_MS 0
#endif

static void emDtlsHandshakeEventHandler(Event *event)
{
  emberEventSetInactive(&emDtlsHandshakeEvent);

  uint8_t i;
  for (i = 0; i < MAX_DTLS_CONNECTIONS; i++) {
    MbedtlsConnection *connection = &emDtlsConnections[i];
    if (!connection->dtls.open) {
      return;
    }

    int status = mbedtls_ssl_handshake(&(connection->mbedtls.context));
    if (status == 0) {
      if (connection->dtls.flags
          & (DTLS_MODE_CERT | DTLS_MODE_PSK | DTLS_MODE_PKEY)) {
        emApiDtlsSecureSessionEstablished(!connection->dtls.amClient,
                                          connection->dtls.sessionId,
                                          (const EmberIpv6Address *) connection->dtls.localAddress,
                                          (const EmberIpv6Address *) connection->dtls.remoteAddress,
                                          connection->dtls.localPort,
                                          connection->dtls.remotePort);
      } else if (connection->dtls.amClient) {
        // Joining handshake.
        emCommissioningHandshakeComplete();
      }
    } else if (status == MBEDTLS_ERR_SSL_WANT_READ
               || status == MBEDTLS_ERR_SSL_WANT_WRITE) {
      emDtlsHandshakeTimerMs(RETRY_HANDSHAKE_MS);
    } else if (!connection->dtls.amClient
               && status == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED) {
      emLogLine(SECURITY, "hello verification requested");
      mbedtls_ssl_session_reset(&(connection->mbedtls.context));

      uint32_t ret;

      if ( (ret = mbedtls_ssl_set_client_transport_id(&(connection->mbedtls.context),
                                                      connection->dtls.remoteAddress,
                                                      16)) != 0 ) {
        emLogLine(SECURITY, "failed! mbedtls_ssl_set_client_transport_id returned -0x%x", -ret);
        emDtlsFree(connection);
        return;
      }

      if (connection->dtls.flags & DTLS_JOIN) {
        uint8_t key[16];
        uint8_t keyLength = 0;

        if (connection->dtls.localPort == emUdpJoinPort) {
          EmberEui64 eui64;
          emInterfaceIdToLongId(connection->dtls.remoteAddress + 8, eui64.bytes);
          keyLength = emGetJoinKey(key, &eui64);
        } else if (connection->dtls.localPort == emUdpCommissionPort) {
          MEMCOPY(key, emGetActiveDataset()->pskc, THREAD_PSKC_SIZE);
          keyLength = THREAD_PSKC_SIZE;
        }

        mbedtlsSetJpakeKey(connection->dtls.sessionId, key, keyLength);
        mbedtls_ssl_set_bio(&(connection->mbedtls.context),
                            &(connection->mbedtls.remoteFd),
                            mbedtls_net_send,
                            mbedtls_net_recv,
                            mbedtls_net_recv_timeout);
      }
    } else if (status < 0) {
#ifdef MBEDTLS_ERROR_C
      char error_buf[100];
#endif
      switch (status) {
        case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
          emLogLine(SECURITY, "dtls handshake: closing connection due to peer close notify.");
          emCloseMbedtlsConnection(connection);
          break;
        case MBEDTLS_ERR_SSL_BAD_HS_CLIENT_HELLO:
          // We can get here if we've just closed a connection and received
          // the close reply.  No way to know if it isn't another client-hello.
          // emLogLine(SECURITY, "dtls handshake: not a client hello. closing...");
          emRemoveDtlsConnection(connection);
          break;
        default:
#ifdef MBEDTLS_ERROR_C
          mbedtls_strerror(status, error_buf, 100);
          emLogLine(SECURITY,
                    "mbedtls: handshake error: %s (state: %s)",
                    error_buf,
                    emDtlsHandshakeStateString(connection->mbedtls.context.state));
#else
          emLogLine(SECURITY,
                    "mbedtls: handshake error: %d (state: %s)",
                    status,
                    emDtlsHandshakeStateString(connection->mbedtls.context.state));
#endif
          if (connection->mbedtls.context.state != MBEDTLS_SSL_HANDSHAKE_OVER) {
            if (!connection->dtls.amClient) {
              mbedtls_ssl_send_alert_message(&(connection->mbedtls.context),
                                             MBEDTLS_SSL_ALERT_LEVEL_FATAL,
                                             MBEDTLS_SSL_ALERT_MSG_HANDSHAKE_FAILURE);
            } else if (connection->dtls.amClient
                       && (connection->dtls.flags & DTLS_JOIN)) {
#ifndef UNIX_HOST
              emJoinSecurityFailed();
#endif
            }

            if (connection->dtls.flags
                & (DTLS_MODE_CERT | DTLS_MODE_PSK | DTLS_MODE_PKEY)) {
              emApiOpenDtlsConnectionReturn(status,
                                            (const EmberIpv6Address *) connection->dtls.remoteAddress,
                                            connection->dtls.localPort,
                                            connection->dtls.remotePort);
            }

            // Clearing this too early could mean problems
            // if the other side continues to attempt a connection
            // emRemoveDtlsConnection(connection);
          }
          break;
      }
    }
  }
}

void emDtlsHandshakeTimerMs(uint32_t delayMs)
{
  emberEventSetDelayMs(&emDtlsHandshakeEvent, delayMs);
}

// read event
static void emDtlsReadEventHandler(Event *event);

static EventActions emDtlsReadEventActions = {
  &emStackEventQueue,
  emDtlsReadEventHandler,
  NULL,
  "DTLS Read"
};

static Event emDtlsReadEvent = { &emDtlsReadEventActions, NULL };

static void emDtlsReadEventHandler(Event *event)
{
  emberEventSetInactive(&emDtlsReadEvent);

  uint8_t i;
  for (i = 0; i < MAX_DTLS_CONNECTIONS; i++) {
    MbedtlsConnection *connection = &emDtlsConnections[i];
    if (!connection->dtls.open) {
      return;
    }

    uint8_t buf[MBEDTLS_SSL_MAX_CONTENT_LEN] = { 0 };
    int ret = mbedtls_ssl_read(&(connection->mbedtls.context),
                               buf,
                               MBEDTLS_SSL_MAX_CONTENT_LEN - 1);

    if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
      emLogLine(SECURITY, "dtls read: closing connection due to peer close notify.");
      emCloseMbedtlsConnection(connection);
      return;
    } else if (ret <= 0) {
      emLogLine(SECURITY, "mbedtls: secure read error: %d", ret);
      return;
    }

    emLogLine(SECURITY, "mbedtls: secure data available.");

    if (connection->dtls.flags
        & (DTLS_MODE_CERT | DTLS_MODE_PSK | DTLS_MODE_PKEY)) {
      EmberCoapRequestInfo info;
      MEMSET(&info, 0, sizeof(EmberCoapSendInfo));
      emApiGetLocalIpAddress(0, &info.localAddress);
      MEMCOPY(info.remoteAddress.bytes, connection->dtls.remoteAddress, 16);
      info.localPort = connection->dtls.localPort;
      info.remotePort = connection->dtls.remotePort;
      info.transmitHandler = &emberDtlsTransmitHandler;
      info.transmitHandlerData = (void *)(unsigned long) connection->dtls.sessionId;

      emberProcessCoap(buf, ret, &info);
    } else {
      emHandleSecureDtlsMessage(connection->dtls.localAddress,
                                connection->dtls.remoteAddress,
                                connection->dtls.localPort,
                                connection->dtls.remotePort,
                                connection->dtls.sessionId,
                                buf,
                                ret,
                                NULL_BUFFER);
    }
  }
}

void emDtlsReadTimerMs(uint32_t delayMs)
{
  emberEventSetDelayMs(&emDtlsReadEvent, delayMs);
}

void emApiGetSecureDtlsSessionId(const EmberIpv6Address *remoteAddress,
                                 uint16_t localPort,
                                 uint16_t remotePort)
{
  MbedtlsConnection *connection = emFindDtlsConnection(remoteAddress->bytes,
                                                       localPort,
                                                       remotePort);
  emApiGetSecureDtlsSessionIdReturn(connection == NULL ? 0xFF : connection->dtls.sessionId,
                                    remoteAddress,
                                    localPort,
                                    remotePort);
}

void emCloseMbedtlsConnection(MbedtlsConnection *connection)
{
  mbedtls_ssl_close_notify(&(connection->mbedtls.context));
  if (connection->dtls.flags
      & (DTLS_MODE_CERT | DTLS_MODE_PSK | DTLS_MODE_PKEY)) {
    emApiCloseDtlsConnectionReturn(connection->dtls.sessionId, EMBER_SUCCESS);
  } else if (connection->dtls.amClient) {
    emParentConnectionHandle = NULL_UDP_HANDLE;
  }
  emRemoveDtlsConnection(connection);
}

void emApiCloseDtlsConnection(uint8_t sessionId)
{
  MbedtlsConnection *connection = emLookupDtlsConnection(sessionId);
  if (connection != NULL) {
    emCloseMbedtlsConnection(connection);
  }
}

void emApiSetDtlsDeviceCertificate(const CertificateAuthority **certAuthority,
                                   const DeviceCertificate *deviceCert)
{
  // Using compiled certificates for this release.
  emApiSetDtlsDeviceCertificateReturn(EMBER_SUCCESS);
}

void emApiSetDtlsPresharedKey(const uint8_t *key,
                              uint8_t keyLength,
                              const EmberIpv6Address *remoteAddress)
{
  EmberStatus status = emAddPsk(key, keyLength, remoteAddress->bytes);
  emApiSetDtlsPresharedKeyReturn(status);
}

#ifdef MBEDTLS_DEBUG_C
void emDtlsDebug(void *ctx, int level, const char *file, int line, const char *str)
{
  emLogLine(SECURITY, "\r%s:%04d: %s", file, line, str);
}
#endif

void mbedtls_timing_set_delay(void *data, uint32_t int_ms, uint32_t fin_ms)
{
}
int mbedtls_timing_get_delay(void *data)
{
  return 0;
}

#if !defined(UNIX_HOST) && !defined(CORTEXM3_EFM32_MICRO)
#include "phy/phy.h"

int mbedtls_platform_entropy_poll(void *data,
                                  unsigned char *output,
                                  size_t len,
                                  size_t *olen)
{
  uint16_t extra;
  ((void) data);
  *olen = 0;

  assert(emRadioGetRandomNumbers((uint16_t *)output, len >> 1));
  if (len & 0x01) {
    assert(emRadioGetRandomNumbers(&extra, 1));
    output[len - 1] = LOW_BYTE(extra);
  }
  *olen = len;
  return 0;
}
#endif

//-------------------------------------------------------------------

MbedtlsConnection *emDtlsConnections = NULL;
mbedtls_entropy_context emDtlsEntropy = { { { 0 } } };

void mbedtlsInitializeConnections(void)
{
  emPskTable = NULL_BUFFER;

  if (emDtlsConnections == NULL) {
    emDtlsConnections = mbedtls_calloc(MAX_DTLS_CONNECTIONS,
                                       sizeof(MbedtlsConnection));
  } else {
    mbedtls_free(emDtlsConnections);
  }

  MEMSET(emDtlsConnections,
         0,
         MAX_DTLS_CONNECTIONS * sizeof(MbedtlsConnection));

  if (emDtlsEntropy.source_count != 0) { // init
    emLogLine(SECURITY, "freeing entropy context");
    mbedtls_entropy_free(&emDtlsEntropy);
  }
  emLogLine(SECURITY, "initializing entropy context");
  mbedtls_entropy_init(&emDtlsEntropy);
#if !defined(CORTEXM3_EFM32_MICRO)
  mbedtls_entropy_add_source(&emDtlsEntropy,
                             mbedtls_platform_entropy_poll,
                             NULL,
                             32,  // MBEDTLS_ENTROPY_MIN_PLATFORM
                             MBEDTLS_ENTROPY_SOURCE_STRONG);
#endif
}

static uint8_t previousSessionId;

static uint8_t getNextSessionId(void)
{
  do {
    previousSessionId += 1;
    if (previousSessionId == 0) {
      previousSessionId += 1;
    }
  } while (emLookupDtlsConnection(previousSessionId) != NULL);
  return previousSessionId;
}

MbedtlsConnection *emLookupDtlsConnection(uint8_t sessionId)
{
  uint8_t i;
  for (i = 0; i < MAX_DTLS_CONNECTIONS; i++) {
    MbedtlsConnection *connection = &emDtlsConnections[i];
    if (connection->dtls.sessionId == sessionId) {
      return connection;
    }
  }
  return NULL;
}

void emRemoveDtlsConnection(MbedtlsConnection *connection)
{
  emLogLine(SECURITY,
            "%s: removing connection.",
            (connection->dtls.amClient ? "client" : "server"));

  emDtlsFree(connection);
  MEMSET(connection, 0, sizeof(MbedtlsConnection));
}

MbedtlsConnection *emFindDtlsConnection(const uint8_t *remoteAddress,
                                        uint16_t localPort,
                                        uint16_t remotePort)
{
  uint8_t i;
  for (i = 0; i < MAX_DTLS_CONNECTIONS; i++) {
    MbedtlsConnection *connection = &emDtlsConnections[i];
    if ((MEMCOMPARE(connection->dtls.remoteAddress,
                    remoteAddress,
                    16) == 0)
        && connection->dtls.localPort == localPort
        && connection->dtls.remotePort == remotePort) {
      return connection;
    }
  }
  return NULL;
}

MbedtlsConnection *emFindRemoteDtlsConnection(int remoteFd)
{
  uint8_t i;
  for (i = 0; i < MAX_DTLS_CONNECTIONS; i++) {
    MbedtlsConnection *connection = &emDtlsConnections[i];
    if (connection->dtls.open) {
      return connection;
    }
  }
  return NULL;
}

MbedtlsConnection *emAddDtlsConnection(uint8_t flags,
                                       const uint8_t *remoteAddress,
                                       uint16_t localPort,
                                       uint16_t remotePort)
{
  MbedtlsConnection *connection = emFindDtlsConnection(remoteAddress,
                                                       localPort,
                                                       remotePort);
  if (connection != NULL && connection->dtls.open) {
    return connection;
  } else if (connection == NULL) {
    uint8_t i;
    for (i = 0; i < MAX_DTLS_CONNECTIONS; i++) {
      MbedtlsConnection *conn = &emDtlsConnections[i];
      if (!conn->dtls.open) {
        connection = conn;
      }
    }
    if (connection == NULL) {
      return NULL;
    }
  }

  MEMSET(connection, 0, sizeof(MbedtlsConnection));
  connection->dtls.sessionId = getNextSessionId();
  if (connection->dtls.sessionId == 0) {
    return NULL;
  }

  if (flags & (DTLS_MODE_CERT | DTLS_MODE_PSK | DTLS_MODE_PKEY)) {
    EmberIpv6Address local;
    emApiGetLocalIpAddress(0, &local);
    MEMCOPY(connection->dtls.localAddress, local.bytes, 16);
  } else {
    emStoreLongFe8Address(emMacExtendedId, connection->dtls.localAddress);
  }

  MEMCOPY(connection->dtls.remoteAddress, remoteAddress, 16);
  // the only legit flag arguments
  connection->dtls.flags = flags;
  connection->dtls.localPort = localPort;
  connection->dtls.remotePort = remotePort;

  return connection;
}

void emDtlsFree(MbedtlsConnection *connection)
{
  emLogLine(SECURITY, "dtls free context");
  mbedtls_ssl_free(&(connection->mbedtls.context));
  mbedtls_net_free(&(connection->mbedtls.remoteFd));
  mbedtls_net_free(&(connection->mbedtls.listenFd));
  mbedtls_ssl_cookie_free(&(connection->mbedtls.cookie_ctx));
#ifdef MBEDTLS_CTR_DRBG_C
  mbedtls_ctr_drbg_free(&(connection->mbedtls.ctr_drbg));
#endif
  mbedtls_ssl_config_free(&(connection->mbedtls.conf));
#ifdef MBEDTLS_X509_CRT_PARSE_C
  mbedtls_x509_crt_free(&(connection->mbedtls.cacert));
  mbedtls_x509_crt_free(&(connection->mbedtls.cert));
#endif
#ifdef MBEDTLS_PK_PARSE_C
  mbedtls_pk_free(&(connection->mbedtls.pkey));
#endif
}

//-------------------------------------------------------------------

static const char * const handshakeStates[] = {
  "HELLO_REQUEST",
  "CLIENT_HELLO",
  "SERVER_HELLO",
  "SERVER_CERTIFICATE",
  "SERVER_KEY_EXCHANGE",
  "CERTIFICATE_REQUEST",
  "SERVER_HELLO_DONE",
  "CLIENT_CERTIFICATE",
  "CLIENT_KEY_EXCHANGE",
  "CERTIFICATE_VERIFY",
  "CLIENT_CHANGE_CIPHER_SPEC",
  "CLIENT_FINISHED",
  "SERVER_CHANGE_CIPHER_SPEC",
  "SERVER_FINISHED",
  "FLUSH_BUFFERS",
  "HANDSHAKE_WRAPUP",
  "HANDSHAKE_OVER",
  "SERVER_NEW_SESSION_TICKET",
  "SERVER_HELLO_VERIFY_REQUEST_SENT",
};

const uint8_t *emDtlsHandshakeStateString(uint8_t handshakeState)
{
  if (handshakeState >= sizeof (handshakeStates) / sizeof (handshakeStates[0])) {
    return (const uint8_t *) "fatal";
  }
  return (const uint8_t *)handshakeStates[handshakeState];
}
