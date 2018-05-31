/*
 * File: mbedtls-stack.h
 * Description: mbedtls stack utility functions
 * Author(s): Suvesh Pratapa
 *
 * Copyright 2017 by Silicon Labs. All rights reserved.                *80*
 */

#ifndef MBEDTLS_STACK_H
#define MBEDTLS_STACK_H

void mbedtlsMarkBuffers(void);
void mbedtlsInitializeConnections(void);
void mbedtlsIncomingHandler(PacketHeader header,
                            Ipv6Header *ipHeader,
                            bool relayed,
                            bool isAppDtls);

void mbedtlsSubmitRelayedPayload(uint8_t flags,
                                 const uint8_t* joinerIid,
                                 uint16_t joinerPort,
                                 uint16_t joinerRouterId,
                                 Buffer payload);
void mbedtlsRelayTxHandler(CommissionMessage *message,
                           const uint8_t *payload,
                           uint16_t payloadLength,
                           Buffer header,
                           const EmberCoapRequestInfo *info);
void mbedtlsJoinFinalHandler(CommissionMessage *message,
                             const uint8_t *payload,
                             uint16_t payloadLength,
                             Buffer header,
                             const EmberCoapRequestInfo *info);
void mbedtlsSendJoinerEntrust(const uint8_t *remoteAddress, const uint8_t *key);

void mbedtlsCloseJoinConnection(void);
bool mbedtlsStartJoinClient(const uint8_t *address,
                            uint16_t remotePort,
                            const uint8_t *key,
                            uint8_t keyLength);

void mbedtlsSetJpakeKey(uint8_t sessionId,
                        const uint8_t *key,
                        uint8_t keyLength);

void mbedtlsDeriveCommissioningMacKey(void *context,
                                      const uint8_t *masterSecret,
                                      const uint8_t *keyBlock,
                                      size_t macLength,
                                      size_t keyLength,
                                      size_t ivLength);

#endif // MBEDTLS_STACK_H
