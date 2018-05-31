// File: coap-stack.h
//
// Description: CoAP stack functionality
//
// Copyright 2015 Silicon Laboratories, Inc.                                *80*

#ifndef COAP_STACK_H
#define COAP_STACK_H

void emCoapStackIncomingMessageHandler(PacketHeader header,
                                       Ipv6Header *ipHeader,
                                       EmberCoapTransmitHandler transmitHandler,
                                       void *transmitHandlerData,
                                       CoapRequestHandler handler);

bool emCoapDtlsTransmitHandler(const uint8_t *payload,
                               uint16_t payloadLength,
                               const EmberIpv6Address *localAddress,
                               uint16_t localPort,
                               const EmberIpv6Address *remoteAddress,
                               uint16_t remotePort,
                               void *transmitHandlerData);

// Special version for messages secured with a commissioning key.
void emHandleIncomingCoapCommission(PacketHeader header, Ipv6Header *ipHeader);

void emCoapCommissionRequestHandler(EmberCoapCode code,
                                    uint8_t *uri,
                                    EmberCoapReadOptions *options,
                                    const uint8_t *payload,
                                    uint16_t payloadLength,
                                    const EmberCoapRequestInfo *info);

#endif // COAP_STACK_H
