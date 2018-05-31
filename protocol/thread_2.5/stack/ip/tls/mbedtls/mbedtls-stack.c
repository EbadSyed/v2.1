/*
 * File: mbedtls-stack.c
 * Description: mbedtls stack utilities
 * Author(s): Suvesh Pratapa
 *
 * Copyright 2017 by Silicon Labs. All rights reserved.                *80*
 */

#ifndef UNIX_HOST

#include "stack/core/ember-stack.h"
#include "stack/ip/commission.h"
#include "stack/ip/commission-dataset.h"
#include "stack/ip/ip-address.h"
#include "stack/ip/tls/dtls-join.h"
#include "stack/ip/tls/tls.h"
#include "stack/ip/tls/tls-sha256.h"
#include "stack/ip/zigbee/key-management.h"
#include "stack/ip/zigbee/join.h"
#include "stack/mac/802.15.4/802-15-4-ccm.h"

#include MBEDTLS_CONFIG_FILE
#include "mbedtls/entropy.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_cookie.h"
#include "mbedtls/net.h"
#include "mbedtls/timing.h"

#ifdef MBEDTLS_CTR_DRBG_C
  #include "mbedtls/ctr_drbg.h"
#endif
#ifdef MBEDTLS_DEBUG_C
  #include "mbedtls/debug.h"
#endif
#ifdef MBEDTLS_ERROR_C
  #include "mbedtls/error.h"
#endif
#include "stack/ip/tls/mbedtls/mbedtls.h"
#include "stack/ip/tls/mbedtls/mbedtls-stack.h"

void mbedtlsSubmitRelayedPayload(uint8_t flags,
                                 const uint8_t* remoteAddress,
                                 uint16_t joinerPort,
                                 uint16_t joinerRouterId,
                                 Buffer payload)
{
  uint8_t *kek = NULL;
  if (flags & DTLS_SEND_ENTRUST) {
    kek = emGetCommissioningMacKey(NULL);
  }

  emForwardToJoiner(remoteAddress + 8, // joiner IID
                    joinerPort,
                    joinerRouterId,
                    kek,
                    payload);
}

void mbedtlsSendJoinerEntrust(const uint8_t *remoteAddress, const uint8_t *key)
{
  uint8_t ipDestination[16];
  MEMCOPY(ipDestination, remoteAddress, 16);

  emLogBytesLine(COMMISSION, "entrust sent to", ipDestination, 16);

  emLogBytesLine(COMMISSION, "entrust key", key, 16);

  uint8_t payload[125];
  uint8_t *finger = payload;
  uint32_t sequence;
  CoapMessage coap;

  uint8_t storage[EMBER_ENCRYPTION_KEY_SIZE];
  finger = emAddTlv(finger,
                    COMMISSION_NETWORK_MASTER_KEY_TLV,
                    emGetNetworkMasterKey(storage),
                    EMBER_ENCRYPTION_KEY_SIZE);
  assert(emGetNetworkKeySequence(&sequence));

  const CommissioningDataset *active = emGetActiveDataset();

  finger = emAddInt32uTlv(finger, COMMISSION_NETWORK_KEY_SEQUENCE_TLV, sequence);
  finger = emAddMeshLocalUlaTlv(finger);
  finger = emAddExtendedPanIdTlv(finger);
  finger = emAddNetworkNameTlv(finger);
  finger = emAddTlv(finger,
                    COMMISSION_SECURITY_POLICY_TLV,
                    active->securityPolicy,
                    3);
  finger = emAddTlv(finger, COMMISSION_PSKC_TLV, active->pskc, 16);
  finger = emAddTlv(finger,
                    COMMISSION_CHANNEL_MASK_TLV,
                    active->channelMask,
                    6);
  finger = emAddTlv(finger,
                    COMMISSION_ACTIVE_TIMESTAMP_TLV,
                    active->timestamp,
                    8);
  assert(finger - payload <= sizeof(payload));

  Buffer keyBuffer = emFillBuffer(key, 16);
  if (keyBuffer == NULL_BUFFER) {
    return;
  }

  emInitStackCoapMessage(&coap,
                         (EmberIpv6Address *) ipDestination,
                         payload,
                         finger - payload);
  coap.transmitHandler = &emJoinerEntrustTransmitHandler;
  if (emSubmitCoapMessage(&coap, JOINER_ENTRUST_URI, keyBuffer)
      == EMBER_SUCCESS) {
    emLogLine(COMMISSION, "sent joiner entrust");
  } else {
    emLogLine(COMMISSION, "failed to send joiner entrust");
  }
}

static uint16_t largeTlvLength(const uint8_t *tlv)
{
  if (tlv[1] == 255) {
    return emberFetchHighLowInt16u(tlv + 2);
  } else {
    return tlv[1];
  }
}

static const uint8_t *largeTlvData(const uint8_t *tlv)
{
  return tlv + ((tlv[1] == 255) ? 4 : 2);
}

void mbedtlsRelayTxHandler(CommissionMessage *message,
                           const uint8_t *payload,
                           uint16_t payloadLength,
                           Buffer header,
                           const EmberCoapRequestInfo *info)
{
  uint8_t kek[16];
  MbedtlsConnection connection;

  // Save these because we are about to clobber the buffer holding them.
  if (message->tlvMask & COMM_TLV_BIT(JOINER_ROUTER_KEK_TLV)) {
    MEMCOPY(kek, message->joinerRouterKek, 16);
  }
  MEMCOPY(connection.dtls.remoteAddress, emFe8Prefix.contents, 8);
  MEMCOPY(connection.dtls.remoteAddress + 8, message->joinerAddress, 8);
  connection.dtls.remotePort = message->joinerUdpPort;

  if (message->tlvMask & COMM_TLV_BIT(JOINER_DTLS_ENCAP_TLV)) {
    uint16_t length = largeTlvLength(message->joinerDtlsEncapTlv);
    MEMMOVE(emGetBufferPointer(header),
            largeTlvData(message->joinerDtlsEncapTlv),
            length);
    emSetBufferLength(header, length);
    connection.dtls.flags = DTLS_JOIN;
    connection.dtls.localPort = emUdpJoinPort;
    connection.dtls.remotePort = message->joinerUdpPort;

    emDtlsSend(&connection, header, NULL, length);
  }

  if (message->tlvMask & COMM_TLV_BIT(JOINER_ROUTER_KEK_TLV)) {
    mbedtlsSendJoinerEntrust(connection.dtls.remoteAddress, kek);
  }
}

static const uint8_t successTlv[] =
{ COMMISSION_STATE_TLV, 1, COMMISSION_SUCCESS };

static const uint8_t failureTlv[] =
{ COMMISSION_STATE_TLV, 1, COMMISSION_FAILURE };

void mbedtlsJoinFinalHandler(CommissionMessage *message,
                             const uint8_t *payload,
                             uint16_t payloadLength,
                             Buffer header,
                             const EmberCoapRequestInfo *info)
{
  MbedtlsConnection *connection =
    emLookupDtlsConnection((unsigned long) info->transmitHandlerData);
  if (connection == NULL) {
    emberCoapRespondWithCode(info, EMBER_COAP_CODE_401_UNAUTHORIZED);
    return;
  }

  bool rejectJoinUrl = false;
  const int8u *provisioningUrlTlv = message->provisioningUrlTlv;
  if (emProvisioningUrlLength != 0) { // Want to filter on prov. url
    if (provisioningUrlTlv == NULL
        || (provisioningUrlTlv[0] != COMMISSION_PROVISIONING_URL_TLV)
        || (memcmp(emProvisioningUrl,
                   provisioningUrlTlv + 2,         // URL
                   provisioningUrlTlv[1]) != 0)) { // length
      rejectJoinUrl = true;
    }
  }

  connection->dtls.flags |= DTLS_SEND_ENTRUST;

  if (rejectJoinUrl) {
    emberCoapRespondWithPayload(info,
                                EMBER_COAP_CODE_204_CHANGED,
                                failureTlv,
                                sizeof(failureTlv));
  } else {
    emberCoapRespondWithPayload(info,
                                EMBER_COAP_CODE_204_CHANGED,
                                successTlv,
                                sizeof(successTlv));
  }

  connection->dtls.flags &= ~DTLS_SEND_ENTRUST;
}

void mbedtlsCloseJoinConnection(void)
{
  // TODO: Also delete existing join keys as below
  MbedtlsConnection *connection = emLookupDtlsConnection(emParentConnectionHandle);
  emCloseMbedtlsConnection(connection);
}

bool mbedtlsStartJoinClient(const uint8_t *address,
                            uint16_t remotePort,
                            const uint8_t *key,
                            uint8_t keyLength)
{
  emOpenMbedtlsConnection(DTLS_JOIN,
                          address,
                          remotePort,
                          remotePort);
  MbedtlsConnection *connection = emFindDtlsConnection(address,
                                                       remotePort,
                                                       remotePort);
  assert(connection != NULL);
  mbedtlsSetJpakeKey(connection->dtls.sessionId,
                     key,
                     keyLength);
  return true;
}

void mbedtlsSetJpakeKey(uint8_t sessionId,
                        const uint8_t *key,
                        uint8_t keyLength)
{
  MbedtlsConnection *connection = emLookupDtlsConnection(sessionId);

  if (connection != NULL) {
    uint32_t ret = 0;

    if ( (ret = mbedtls_ssl_set_hs_ecjpake_password(&(connection->mbedtls.context),
                                                    (const unsigned char *) key,
                                                    keyLength)) != 0 ) {
      emLogLine(SECURITY, "failed! mbedtls_ssl_set_hs_ecjpake_password returned %d", ret);
      emDtlsFree(connection);
    }
  }
}

void mbedtlsDeriveCommissioningMacKey(void *context,
                                      const uint8_t *masterSecret,
                                      const uint8_t *keyBlock,
                                      size_t macLength,
                                      size_t keyLength,
                                      size_t ivLength)
{
  emLogLine(SECURITY, "dtls jpake: generating kek...");

  uint16_t bufSize = (2 * (macLength + keyLength + ivLength));
  // emLogBytesLine(SECURITY, "master secret", masterSecret, 48);
  // emLogBytesLine(SECURITY, "key block", keyBlock, bufSize);
  uint8_t hashOutput[SHA256_BLOCK_SIZE];
  mbedtls_sha256(keyBlock, bufSize, hashOutput, 0);

  uint8_t generatedKek[16];
  MEMCOPY(generatedKek, hashOutput, AES_128_KEY_LENGTH);
  emLogBytesLine(SECURITY, "kek", generatedKek, AES_128_KEY_LENGTH);
  emSetCommissioningMacKey(generatedKek);
}

#endif // UNIX_HOST
