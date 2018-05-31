/*
 *  mbedtls networking functions
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 *  This file has been modified per application needs.
 */

#include "core/ember-stack.h"
#include "hal/hal.h"
#include "phy/phy.h"

#include "stack/framework/buffer-management.h"
#include "stack/framework/ip-packet-header.h"
#include "stack/ip/commission.h"
#include "stack/ip/commission-dataset.h"
#include "stack/ip/dispatch.h"
#include "stack/ip/tls/dtls-join.h"
#include "stack/ip/udp.h"
#include "stack/ip/tls/dtls.h"
#include "stack/ip/tls/tls.h"
#include "stack/ip/ip-address.h"
#include "stack/ip/ip-header.h"
#include "stack/ip/zigbee/join.h"

#include MBEDTLS_CONFIG_FILE
#include "mbedtls/entropy.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_cookie.h"
#include "mbedtls/net.h"
#include "mbedtls/timing.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"
#include "mbedtls/error.h"
#include "stack/ip/tls/mbedtls/mbedtls.h"
#include "stack/ip/tls/mbedtls/mbedtls-stack.h"

static int readIndex = 0;
static int writeIndex = 0;
Buffer emDtlsIncomingPayload = NULL_BUFFER;

void mbedtlsMarkBuffers(void)
{
  emMarkBuffer(&emDtlsIncomingPayload);
}

void mbedtlsIncomingHandler(PacketHeader header,
                            Ipv6Header *ipHeader,
                            bool relayed,
                            bool isAppDtls)
{
  uint8_t flags = (isAppDtls
                   ? (DTLS_MODE_CERT | DTLS_MODE_PSK)
                   : (relayed
                      ? (DTLS_JOIN | DTLS_RELAYED_JOIN)
                      : DTLS_JOIN));

  MbedtlsConnection *connection = emAddDtlsConnection(flags,
                                                      ipHeader->source,
                                                      ipHeader->destinationPort,
                                                      ipHeader->sourcePort);

  if (connection == NULL) {
    emLogDrop();
    return;
  } else if (!connection->dtls.amClient && !connection->dtls.open) {
    emLogLine(SECURITY, "new incoming dtls packet. init server just in case...");
    emInitMbedtlsServer(connection);
  }

  emLogLine(SECURITY, "dtls state: %s", emDtlsHandshakeStateString(connection->mbedtls.context.state));

  // server needs the right key for this exchange.
  if (!connection->dtls.amClient) {
    if (connection->mbedtls.context.state != MBEDTLS_SSL_HANDSHAKE_OVER) {
      if (connection->dtls.flags & DTLS_JOIN) {
        uint8_t key[16];
        uint8_t keyLength = 0;

        if (connection->dtls.localPort == emUdpJoinPort) {
          EmberEui64 eui64;
          emInterfaceIdToLongId(connection->dtls.remoteAddress + 8, eui64.bytes);
          keyLength = emGetJoinKey(key, &eui64);
        } else if (connection->dtls.remotePort == emUdpCommissionPort) {
          MEMCOPY(key, emGetActiveDataset()->pskc, THREAD_PSKC_SIZE);
          keyLength = THREAD_PSKC_SIZE;
        }

        mbedtlsSetJpakeKey(connection->dtls.sessionId, key, keyLength);
      }
    }
  } else if (connection->mbedtls.context.state == MBEDTLS_SSL_HELLO_REQUEST) {
    // if we're a client this shouldn't happen.
    emLogDrop();
    return;
  }

  if (0 == readIndex) {
    readIndex = ipHeader->transportPayloadLength;
    emDtlsIncomingPayload = emAllocateBuffer(ipHeader->transportPayloadLength);
    if (emDtlsIncomingPayload != NULL_BUFFER) {
      uint8_t *finger = emGetBufferPointer(emDtlsIncomingPayload);
      MEMCOPY(finger,
              ipHeader->transportPayload,
              ipHeader->transportPayloadLength);
    }
    if (connection->mbedtls.context.state != MBEDTLS_SSL_HANDSHAKE_OVER) {
      emDtlsHandshakeTimerMs(0);
    } else {
      emDtlsReadTimerMs(0);
    }
  }
}

bool emberDtlsTransmitHandler(const uint8_t *payload,
                              uint16_t payloadLength,
                              const EmberIpv6Address *localAddress,
                              uint16_t localPort,
                              const EmberIpv6Address *remoteAddress,
                              uint16_t remotePort,
                              void *transmitHandlerData)
{
  int ret = 0;
  MbedtlsConnection *connection =
    emLookupDtlsConnection((unsigned long) transmitHandlerData);

  if (connection != NULL && connection->dtls.open) {
    ret = mbedtls_ssl_write(&(connection->mbedtls.context), payload, payloadLength);
    if (ret > 0) {
      if (connection->dtls.flags & DTLS_RELAYED_JOIN) {
        Buffer payloadBuffer = emFillBuffer(payload, payloadLength);
        mbedtlsSubmitRelayedPayload(connection->dtls.flags,
                                    connection->dtls.remoteAddress,
                                    connection->dtls.remotePort,
                                    connection->dtls.localPort,
                                    payloadBuffer);
      } else if (connection->dtls.flags & DTLS_SEND_ENTRUST) {
        mbedtlsSendJoinerEntrust(connection->dtls.remoteAddress,
                                 emGetCommissioningMacKey(NULL));
      }
    }
  }
  return (ret > 0);
}

void emDtlsSend(MbedtlsConnection *connection, Buffer payload, const unsigned char *buf, size_t len)
{
  if (writeIndex > 0 || payload != NULL_BUFFER) {
    Buffer payloadBuffer;
    if (payload != NULL_BUFFER) {
      payloadBuffer = payload;
    } else {
      payloadBuffer = emFillBuffer(buf, len);
    }

    if (connection->dtls.flags & DTLS_RELAYED_JOIN) {
      mbedtlsSubmitRelayedPayload(connection->dtls.flags,
                                  connection->dtls.remoteAddress,
                                  connection->dtls.remotePort,
                                  connection->dtls.localPort,
                                  payloadBuffer);
      writeIndex = 0;
    } else {
#ifdef UNIX_HOST
      if (emberSendUdp(connection->dtls.remoteAddress,
                       connection->dtls.localPort,
                       connection->dtls.remotePort,
                       (uint8_t *)buf,
                       len) == EMBER_SUCCESS) {
        writeIndex = 0;
      }
#else // UNIX_HOST
      Ipv6Header ipHeader;
      uint8_t options = 0;
      if (connection->dtls.flags & DTLS_JOIN) {
        options = IP_HEADER_LL64_SOURCE;
      }
      PacketHeader header = emMakeUdpHeader(&ipHeader,
                                            options,
                                            connection->dtls.remoteAddress,
                                            IPV6_DEFAULT_HOP_LIMIT,
                                            connection->dtls.localPort,
                                            connection->dtls.remotePort,
                                            NULL,
                                            0,
                                            len);

      assert(header != NULL_BUFFER);

      if (connection->dtls.localPort == emUdpJoinPort
          || connection->dtls.localPort == emUdpCommissionPort) {
        emSetMacFrameControl(header,
                             (emGetMacFrameControl(header)
                              & ~(MAC_FRAME_FLAG_SECURITY_ENABLED
                                  | MAC_FRAME_VERSION_2006)));
      }

      emSetPayloadLink(header, payloadBuffer);
      if (emSubmitIpHeader(header, &ipHeader)) {
        writeIndex = 0;
      }
#endif // UNIX_HOST
    }
  }
}

// copy the data so it can be sent via our transport layer

int mbedtls_net_send(void *ctx, const unsigned char *buf, size_t len)
{
  int ret = 0;
  int fd = ((mbedtls_net_context *) ctx)->fd;
  MbedtlsConnection *connection = emFindRemoteDtlsConnection(fd);

  if (0 == writeIndex) {
    writeIndex = len;
    emDtlsSend(connection, NULL_BUFFER, buf, len);
    ret = len;
  }

  if ( ret < 0 ) {
    /* Write is non blocking hence an error is an error */
    return(MBEDTLS_ERR_NET_SEND_FAILED);
  } else if (ret == 0) {
    return(MBEDTLS_ERR_SSL_WANT_WRITE);
  }
  return(ret);
}

// receive the data from our transport layer

int mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len)
{
  int ret = 0;

  if (readIndex) {
    uint8_t *finger = emGetBufferPointer(emDtlsIncomingPayload);
    if (len <= readIndex) {
      MEMCOPY(buf, finger, len);
      ret = len;
    } else {
      MEMCOPY(buf, finger, readIndex);
      ret = readIndex;
    }
  }

  if ( ret < 0 ) {
    return(MBEDTLS_ERR_NET_RECV_FAILED);
  } else if (ret == 0) {
    return(MBEDTLS_ERR_SSL_WANT_READ);
  } else {
    readIndex = 0;
    emDtlsIncomingPayload = NULL_BUFFER;
  }
  return(ret);
}

int mbedtls_net_recv_timeout(void *ctx,
                             unsigned char *buf,
                             size_t len,
                             uint32_t timeout)
{
  return(mbedtls_net_recv(ctx, buf, len));
}

// stubs
int mbedtls_net_bind(mbedtls_net_context *ctx,
                     const char *bind_ip,
                     const char *port,
                     int proto)
{
  int ret = 0;
  return(ret);
}

int mbedtls_net_accept(mbedtls_net_context *bind_ctx,
                       mbedtls_net_context *client_ctx,
                       void *client_ip,
                       size_t buf_size,
                       size_t *ip_len)
{
  // We assume we're getting a valid client_ip here.  Is that okay?
  *ip_len = 16;
  return(0);
}

void mbedtls_net_init(mbedtls_net_context *ctx)
{
  ctx->fd = -1;
}

int mbedtls_net_connect(mbedtls_net_context *ctx,
                        const char *host,
                        const char *port,
                        int proto)
{
  return 0;
}

void mbedtls_net_free(mbedtls_net_context *ctx)
{
}
