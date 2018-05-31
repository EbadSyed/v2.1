// File: host-udp-retry.h
//
// Description: UDP retry functionality for UNIX Hosts
//
// Copyright 2014 by Silicon Laboratories. All rights reserved.             *80*

#ifndef HOST_UDP_RETRY_H
#define HOST_UDP_RETRY_H

typedef void (RetryHandler)(uint16_t token, uint8_t retriesRemaining);

void emRunUdpRetryEvents(void);

void emMarkUdpRetryBuffers(void);

void emInitializeUdpRetry(void);

EmberStatus emAddUdpRetry(const EmberIpv6Address *target,
                          uint16_t localPort,
                          uint16_t remotePort,
                          const uint8_t *payload,
                          uint16_t payloadLength,
                          uint16_t token,
                          uint8_t retryLimit,
                          uint16_t retryDelayMs,
                          RetryHandler *timeout);

void emRemoveUdpRetry(uint16_t token);

#endif // HOST_UDP_RETRY_H
