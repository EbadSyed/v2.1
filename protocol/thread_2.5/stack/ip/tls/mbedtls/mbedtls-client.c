/*
 * File: mbedtls-client.c
 * Description: mbedtls client
 * Author(s): Suvesh Pratapa
 *
 * Copyright 2017 by Silicon Labs. All rights reserved.                *80*
 */

#include "stack/core/ember-stack.h"

#include "app/thread/plugin/dtls-auth-params/certificate-parameters.h"
#include "stack/ip/tls/dtls-join.h"
#include "stack/ip/udp.h"
#include "stack/ip/tls/dtls.h"
#include "stack/ip/tls/tls.h"
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
#include "stack/ip/commission.h"
#include "stack/ip/tls/mbedtls/mbedtls.h"
#include "stack/ip/tls/mbedtls/mbedtls-stack.h"
#include "stack/ip/tls/psk-table.h"

uint32_t emOpenMbedtlsConnection(uint8_t flags,
                                 const uint8_t *remoteAddress,
                                 uint16_t localPort,
                                 uint16_t remotePort)
{
  MbedtlsConnection *connection = emAddDtlsConnection(flags,
                                                      remoteAddress,
                                                      localPort,
                                                      remotePort);

  if (connection == NULL) {
    return UINT32_MAX;
  }

  connection->dtls.amClient = true;
  connection->dtls.open = true;

  if (connection->dtls.flags & DTLS_JOIN) {
    emParentConnectionHandle = connection->dtls.sessionId;
  }

  uint32_t ret;

  mbedtls_ssl_init(&(connection->mbedtls.context));
  mbedtls_ssl_config_init(&(connection->mbedtls.conf));

  if (connection->dtls.flags & DTLS_MODE_CERT) {
#ifdef MBEDTLS_X509_CRT_PARSE_C
    mbedtls_x509_crt_init(&(connection->mbedtls.cert));
#endif
  }

#ifdef MBEDTLS_CTR_DRBG_C
  mbedtls_ctr_drbg_init(&(connection->mbedtls.ctr_drbg));
#endif
#ifdef MBEDTLS_DEBUG_C
  mbedtls_debug_set_threshold(0);
#endif

#ifdef MBEDTLS_CTR_DRBG_C
  const char *pers = "dtls_client";
  if ( (ret = mbedtls_ctr_drbg_seed(&(connection->mbedtls.ctr_drbg),
                                    mbedtls_entropy_func,
                                    &emDtlsEntropy,
                                    (const unsigned char *) pers,
                                    strlen(pers))) != 0 ) {
    emLogLine(SECURITY, "failed! mbedtls_ctr_drbg_seed returned %d", ret);
    emDtlsFree(connection);
    return ret;
  }
#endif

#ifdef MBEDTLS_X509_CRT_PARSE_C
  emLogLine(SECURITY, "Setting up the CA...");
  if ( (ret = mbedtls_x509_crt_parse(&(connection->mbedtls.cacert),
                                     (const unsigned char*) mbedtls_x509_ca_cert_der,
                                     mbedtls_x509_ca_cert_der_len)) != 0 ) {
    emLogLine(SECURITY, "failed! mbedtls_x509_crt_parse returned %d", ret);
    emDtlsFree(connection);
    return ret;
  }
#endif

  if (connection->dtls.flags & DTLS_MODE_CERT) {
#ifdef MBEDTLS_X509_CRT_PARSE_C
    emLogLine(SECURITY, "Setting up the device certificate...");
    if ( (ret = mbedtls_x509_crt_parse(&(connection->mbedtls.cert),
                                       (const unsigned char*) mbedtls_x509_client_cert_der,
                                       mbedtls_x509_client_cert_der_len)) != 0 ) {
      emLogLine(SECURITY, "failed! mbedtls_x509_crt_parse returned %d", ret);
      emDtlsFree(connection);
      return ret;
    }
#endif

#ifdef MBEDTLS_PK_PARSE_C
    emLogLine(SECURITY, "Setting up the device public key...");
    if ( (ret = mbedtls_pk_parse_key(&(connection->mbedtls.pkey),
                                     (const unsigned char *) mbedtls_x509_client_key_der,
                                     mbedtls_x509_client_key_der_len,
                                     NULL,
                                     0)) != 0) {
      emLogLine(SECURITY, "failed! mbedtls_pk_parse_key returned %d", ret);
      emDtlsFree(connection);
      return ret;
    }
#endif
  }

  emLogLine(SECURITY, "Setting up the DTLS structure...");

  if ( (ret = mbedtls_ssl_config_defaults(&(connection->mbedtls.conf),
                                          MBEDTLS_SSL_IS_CLIENT,
                                          MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                                          MBEDTLS_SSL_PRESET_DEFAULT)) != 0 ) {
    emLogLine(SECURITY, "failed! mbedtls_ssl_config_defaults returned %d", ret);
    emDtlsFree(connection);
    return ret;
  }

#ifdef MBEDTLS_CTR_DRBG_C
  mbedtls_ssl_conf_rng(&(connection->mbedtls.conf),
                       mbedtls_ctr_drbg_random,
                       &(connection->mbedtls.ctr_drbg));
#endif
#ifdef MBEDTLS_DEBUG_C
  mbedtls_ssl_conf_dbg(&(connection->mbedtls.conf), emDtlsDebug, NULL);
#endif
  mbedtls_ssl_conf_read_timeout(&(connection->mbedtls.conf), 60000);

  mbedtls_ssl_conf_min_version(&(connection->mbedtls.conf),
                               MBEDTLS_SSL_MAJOR_VERSION_3,
                               MBEDTLS_SSL_MINOR_VERSION_3);
  mbedtls_ssl_conf_max_version(&(connection->mbedtls.conf),
                               MBEDTLS_SSL_MAJOR_VERSION_3,
                               MBEDTLS_SSL_MINOR_VERSION_3);

  if (connection->dtls.flags & DTLS_JOIN) {
    static const int ciphersuites[2] = { 0xC0FF, 0 }; // EC-JPAKE cipher suite
    mbedtls_ssl_conf_ciphersuites(&(connection->mbedtls.conf), ciphersuites);

    mbedtls_ssl_conf_export_keys_cb(&(connection->mbedtls.conf),
                                    (mbedtls_ssl_export_keys_t *) mbedtlsDeriveCommissioningMacKey,
                                    NULL);
    mbedtls_ssl_conf_handshake_timeout(&(connection->mbedtls.conf), 8000, 60000);
  } else {
    if (connection->dtls.flags & DTLS_MODE_CERT) {
      static const int ciphersuites[2] = { 0xC0AE, 0 }; // TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8
      mbedtls_ssl_conf_ciphersuites(&(connection->mbedtls.conf), ciphersuites);
    } else if (connection->dtls.flags & DTLS_MODE_PSK) {
      static const int ciphersuites[2] = { 0xC0A8, 0 }; // TLS_PSK_WITH_AES_128_CCM
      mbedtls_ssl_conf_ciphersuites(&(connection->mbedtls.conf), ciphersuites);
    }
    mbedtls_ssl_conf_handshake_timeout(&(connection->mbedtls.conf), 60000, 60000);
  }

#if defined(MBEDTLS_X509_CRT_PARSE_C) && defined (MBEDTLS_PK_PARSE_C)
  if (connection->dtls.flags & DTLS_MODE_CERT) {
    mbedtls_ssl_conf_authmode(&(connection->mbedtls.conf), MBEDTLS_SSL_VERIFY_REQUIRED);

    emLogLine(SECURITY, "Configuring the CA chain...");
    mbedtls_ssl_conf_ca_chain(&(connection->mbedtls.conf),
                              &(connection->mbedtls.cacert),
                              NULL);

    if ( (ret = mbedtls_ssl_conf_own_cert(&(connection->mbedtls.conf),
                                          &(connection->mbedtls.cert),
                                          &(connection->mbedtls.pkey))) != 0 ) {
      emLogLine(SECURITY, "failed! mbedtls_ssl_conf_own_cert returned %d", ret);
      emDtlsFree(connection);
      return ret;
    }
  }
#endif

#ifdef MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
  if (connection->dtls.flags & DTLS_MODE_PSK) {
    uint8_t psk[32];
    uint8_t pskLength = emGetPsk(psk, remoteAddress);

    if (pskLength != 0
        && (ret = mbedtls_ssl_conf_psk(&(connection->mbedtls.conf),
                                       psk,
                                       pskLength,
                                       (const unsigned char *) "mbedtls", // Dummy identity
                                       7)) != 0) {
      emLogLine(SECURITY, "failed! mbedtls_ssl_conf_psk returned %d", ret);
      emDtlsFree(connection);
      return ret;
    }
  }
#endif

  if ( (ret = mbedtls_ssl_setup(&(connection->mbedtls.context),
                                &(connection->mbedtls.conf))) != 0 ) {
    emLogLine(SECURITY, "failed! mbedtls_ssl_setup returned %d", ret);
    emDtlsFree(connection);
    return ret;
  }

  mbedtls_ssl_set_bio(&(connection->mbedtls.context),
                      &(connection->mbedtls.remoteFd),
                      mbedtls_net_send,
                      mbedtls_net_recv,
                      mbedtls_net_recv_timeout);

  mbedtls_ssl_set_timer_cb(&(connection->mbedtls.context),
                           &(connection->mbedtls.timer),
                           mbedtls_timing_set_delay,
                           mbedtls_timing_get_delay);

  emDtlsHandshakeTimerMs(0);

  return 0;
}

void emApiOpenDtlsConnection(EmberDtlsMode dtlsMode,
                             const EmberIpv6Address *remoteAddress,
                             uint16_t localPort,
                             uint16_t remotePort)
{
  uint8_t mode = (dtlsMode & (DTLS_MODE_CERT
                              | DTLS_MODE_PSK
                              | DTLS_MODE_PKEY));

  uint32_t ret = emOpenMbedtlsConnection(mode, remoteAddress->bytes, localPort, remotePort);
  emApiOpenDtlsConnectionReturn(ret, remoteAddress, localPort, remotePort);
}
