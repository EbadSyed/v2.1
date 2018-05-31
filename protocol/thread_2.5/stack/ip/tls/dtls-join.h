/*
 * File: dtls-join.h
 * Description: joining using DTLS
 *
 * Copyright 2015 Silicon Laboratories, Inc.                                *80*
 */

#ifndef DTLS_JOIN_H
#define DTLS_JOIN_H

enum {
  DLTS_JOIN_THREAD_COMMISSIONING,       // the default
  DLTS_JOIN_JPAKE_TEST
};

extern uint8_t emDtlsJoinMode;
extern uint16_t emUdpJoinSrcPort; // random src port used when joining.
extern uint16_t emUdpJoinPort;
extern uint16_t emUdpCommissionPort;

#define DTLS_COMMISSION_PORT 49191
#define DTLS_JOIN_PORT 19786
// "Magic" port used by the driver for passing messages between NCP and
// commission-proxy-app.  Used to be same as DTLS_JOIN_PORT.
// TODO: Investigate changing this number.
#define COMMISSION_PROXY_PORT 19786

#define THREAD_COMMISSIONER_KEY_SIZE 255
#define THREAD_PSKC_SIZE 16

// Thread wants use to require a cookie for DTLS join handshakes.
extern bool emDtlsJoinRequireCookie;

void emDtlsJoinInit(void);

void emSetJoinSecuritySuites(uint16_t suites);

void emCloseDtlsJoinConnection(void);

// Process a message that arrives from a joiner, either directly over the
// radio (relayed == false) or via a commission-proxy-app (relayed == true).
// The value of 'relayed' is used only to determine how to send any
// response.
void emIncomingJoinMessageHandler(PacketHeader header,
                                  Ipv6Header *ipHeader,
                                  bool relayed);

void emHandleSecureDtlsMessage(const uint8_t *localAddress,
                               const uint8_t *remoteAddress,
                               uint16_t localPort,
                               uint16_t remotePort,
                               EmberUdpConnectionHandle connectionHandle,
                               uint8_t *packet,
                               uint16_t length,
                               Buffer buffer);

void emHandleJoinDtlsMessage(EmberUdpConnectionData *connection,
                             uint8_t *packet,
                             uint16_t length,
                             Buffer buffer);

bool emStartJoinClient(const uint8_t *address,
                       uint16_t remotePort,
                       const uint8_t *key,
                       uint8_t keyLength);

void emSetCommissionKey(const uint8_t *key, uint8_t keyLength);
uint8_t emGetCommissionKey(uint8_t *key);

extern EmberUdpConnectionHandle emParentConnectionHandle;

void emMarkDtlsJoinBuffers(void);

void emDerivePskc(const uint8_t *passphrase,
                  int16_t passphraseLen,
                  const uint8_t *extendedPanId,
                  const uint8_t *networkName,
                  uint8_t *result);

uint8_t emGetJoinKey(uint8_t *joinKey, const EmberEui64 *eui64);

#endif // DTLS_JOIN_H
