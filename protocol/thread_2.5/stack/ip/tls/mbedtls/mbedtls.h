/*
 * File: mbedtls.h
 * Description: mbedtls utility functions
 * Author(s): Suvesh Pratapa
 *
 * Copyright 2017 by Silicon Labs. All rights reserved.                *80*
 */

#ifndef MBEDTLS_H
#define MBEDTLS_H

// Internal DTLS flags

#define DTLS_MODE_CERT     EMBER_DTLS_MODE_CERT
#define DTLS_MODE_PSK      EMBER_DTLS_MODE_PSK
#define DTLS_MODE_PKEY     EMBER_DTLS_MODE_PKEY

#define DTLS_JOIN          0x08
#define DTLS_RELAYED_JOIN  0x10
#define DTLS_SEND_ENTRUST  0x20

// Data stored for each connection.
typedef struct {
  uint8_t sessionId;
  uint8_t flags;
  bool amClient;
  bool open;
  uint8_t localAddress[16];
  uint8_t remoteAddress[16];
  uint16_t localPort;
  uint16_t remotePort;
} DtlsConnectionData;

typedef struct {
  mbedtls_ssl_context context;
  mbedtls_net_context remoteFd;
  mbedtls_net_context listenFd;
  mbedtls_ssl_cookie_ctx cookie_ctx;
#ifdef MBEDTLS_CTR_DRBG_C
  mbedtls_ctr_drbg_context ctr_drbg;
#endif
  mbedtls_ssl_config conf;
#ifdef MBEDTLS_X509_CRT_PARSE_C
  mbedtls_x509_crt cacert;
  mbedtls_x509_crt cert;
#endif
  mbedtls_pk_context pkey;
  mbedtls_timing_delay_context timer;
} MbedtlsData;

// typedef void (*DtlsConnectionReadHandler)
//   (DtlsConnectionData *connection,
//    uint8_t *packet,
//    uint16_t length);

typedef struct {
  DtlsConnectionData dtls;          // dtls connection details
  MbedtlsData mbedtls;              // mbedtls data
} MbedtlsConnection;

#define MAX_DTLS_CONNECTIONS 1 // Should be based on platform?
// alloc. heap size is assigned in
// ember-configuration-defaults.h

extern MbedtlsConnection *emDtlsConnections;
extern Buffer emDtlsIncomingPayload;
extern mbedtls_entropy_context emDtlsEntropy;

MbedtlsConnection *emAddDtlsConnection(uint8_t flags,
                                       const uint8_t *remoteAddress,
                                       uint16_t localPort,
                                       uint16_t remotePort);

MbedtlsConnection *emLookupDtlsConnection(uint8_t sessionId);
void emRemoveDtlsConnection(MbedtlsConnection *connection);
MbedtlsConnection *emFindDtlsConnection(const uint8_t *remoteAddress,
                                        uint16_t localPort,
                                        uint16_t remotePort);
MbedtlsConnection *emFindRemoteDtlsConnection(int remoteFd);

void emDtlsFree(MbedtlsConnection *connection);

uint32_t emInitMbedtlsServer(MbedtlsConnection *connection);

uint32_t emOpenMbedtlsConnection(uint8_t flags,
                                 const uint8_t *remoteAddress,
                                 uint16_t localPort,
                                 uint16_t remotePort);
void emCloseMbedtlsConnection(MbedtlsConnection *connection);

void emDtlsHandshakeTimerMs(uint32_t delayMs);
void emDtlsReadTimerMs(uint32_t delayMs);

void emDtlsSend(MbedtlsConnection *connection,
                Buffer payload,
                const unsigned char *buf,
                size_t len);

#ifdef MBEDTLS_DEBUG_C
void emDtlsDebug(void *ctx, int level, const char *file, int line, const char *str);
#endif

void mbedtls_timing_set_delay(void *data, uint32_t int_ms, uint32_t fin_ms);
int mbedtls_timing_get_delay(void *data);

#if !defined(CORTEXM3_EFM32_MICRO)
int mbedtls_platform_entropy_poll(void *data,
                                  unsigned char *output,
                                  size_t len,
                                  size_t *olen);
#endif

const uint8_t *emDtlsHandshakeStateString(uint8_t handshakeState);

#endif // MBEDTLS_H
