/*
 * File: coap-diagnostic.c
 * Description: Thread Diagnostic Functionality
 *
 * Copyright 2016 by Silicon Laboratories. All rights reserved.             *80*
 */

#include "core/ember-stack.h"
#include "hal/hal.h"
#include "framework/ip-packet-header.h"
#include "mac/mac-header.h"
#include "app/coap/coap.h"
#include "zigbee/child-data.h"
#include "app/util/counters/counters.h"
#include "stack/ip/coap-diagnostic.h"
#include "stack/ip/ip-address.h"
#include "stack/ip/tls/debug.h"

// for size_t
#include <stddef.h>
// for strncmp() and strlen()
#include <string.h>

typedef enum {
  POINTER,
  INT8,
  INT16,
  INT32,
  TLV
} TlvType_e;

typedef uint8_t TlvType;

typedef const struct {
  uint16_t offset;
  uint8_t tlv;
  uint8_t type;
  uint8_t length;
} DiagnosticTlvData;

// A macro to get the offset of a field within an AddressManagementMessage
#define OFFSET(f) \
  ((uint16_t) (long) &(((EmberDiagnosticData *) NULL)->f))

#define FIELD(tlv, f, type, length) \
  { OFFSET(f), tlv, (type), (length) }

#define DUMMY_FIELD() { 0, 0, 0, 0 }

// the order of these fields must match the TlvFlag enumeration above!
static const DiagnosticTlvData tlvDataArray[] = {
  FIELD(DIAGNOSTIC_MAC_EXTENDED_ADDRESS, macExtendedAddress, POINTER, 8),
  FIELD(DIAGNOSTIC_ADDRESS_16, address16, INT16, 0),
  FIELD(DIAGNOSTIC_MODE, mode, INT8, 0),
  FIELD(DIAGNOSTIC_TIMEOUT, timeout, INT16, 0),
  FIELD(DIAGNOSTIC_CONNECTIVITY, connectivity, TLV, 0),
  FIELD(DIAGNOSTIC_ROUTING_TABLE, routingTable, TLV, 0),
  FIELD(DIAGNOSTIC_LEADER_DATA, leaderData, TLV, 0),
  FIELD(DIAGNOSTIC_NETWORK_DATA, networkData, TLV, 0),
  FIELD(DIAGNOSTIC_IPV6_ADDRESS_LIST, ipv6AddressList, TLV, 0),
  FIELD(DIAGNOSTIC_MAC_COUNTERS, macCounters, TLV, 0),
  DUMMY_FIELD(),
  DUMMY_FIELD(),
  DUMMY_FIELD(),
  DUMMY_FIELD(),
  FIELD(DIAGNOSTIC_BATTERY_LEVEL, batteryLevel, INT8, 0),
  FIELD(DIAGNOSTIC_VOLTAGE, voltage, INT16, 0),
  FIELD(DIAGNOSTIC_CHILD_TABLE, childTable, TLV, 0),
  FIELD(DIAGNOSTIC_CHANNEL_PAGES, channelPages, TLV, 0),
};

// TODO: EMIPSTACK-1226, then enable this for host
#ifndef EMBER_HOST
uint8_t *addTypeListTlv(uint8_t *finger,
                        const uint8_t *tlvs,
                        uint8_t tlvsLength)
{
  *finger++ = DIAGNOSTIC_TYPE_LIST;
  *finger++ = tlvsLength;
  MEMCOPY(finger, tlvs, tlvsLength);
  finger += tlvsLength;
  return finger;
}
#endif

bool emParseDiagnosticData(EmberDiagnosticData *data,
                           const uint8_t *payload,
                           uint16_t length)
{
  MEMSET(data, 0, sizeof(EmberDiagnosticData));
  const uint8_t *finger;

  for (finger = payload;
       finger + 2 <= payload + length;
       finger += (finger[1] + 2)) {
    uint8_t tlvId = finger[0];
    uint16_t length = finger[1];
    const uint8_t *value = finger + 2;

    if (tlvId > LAST_DIAGNOSTIC_VALUE) {
      // we don't know what this is
      continue;
    }

    const DiagnosticTlvData *tlvData = &tlvDataArray[tlvId];
    data->tlvMask |= BIT(tlvId);
    void *field = ((uint8_t *)data) + tlvData->offset;

    switch (tlvData->type) {
      case INT8:
        if (length != 1) {
          lose(COAP, false);
        }
        *(uint8_t *)field = *value;
        break;

      case INT16:
        if (length != 2) {
          lose(COAP, false);
        }
        *((uint16_t *) field) = emberFetchHighLowInt16u(value);
        break;

      case INT32:
        if (length != 4) {
          lose(COAP, false);
        }
        *((uint32_t *) field) = emberFetchHighLowInt32u(value);
        break;

      case POINTER:
        if (length != tlvData->length) {
          lose(COAP, false);
        }
        *((const uint8_t **) field) = value;
        break;

      case TLV:
        *((const uint8_t **) field) = value - 1;
        break;
    }
  }

  return true;
}

// TODO: EMIPSTACK-1226, then enable this for host
#ifndef EMBER_HOST
static void responseHandler(EmberCoapStatus status,
                            EmberCoapCode code,
                            EmberCoapReadOptions *options,
                            uint8_t *payload,
                            uint16_t payloadLength,
                            EmberCoapResponseInfo *info)
{
  if (status == EMBER_COAP_MESSAGE_RESPONSE) {
    emberDiagnosticAnswerHandler(EMBER_SUCCESS,
                                 &info->remoteAddress,
                                 payload,
                                 payloadLength);
  }
}
#endif

void emApiSendDiagnostic(const EmberIpv6Address *destination,
                         const uint8_t *requestedTlvs,
                         uint8_t length,
                         const uint8_t *uri)
{
// TODO: EMIPSTACK-1226, then enable this for host
#ifndef EMBER_HOST
  uint8_t payload[TYPE_LIST_TLV_LENGTH];
  uint8_t *finger = payload;

  if (length == 0
      || length > TYPE_LIST_TLV_LENGTH - 2) {
    emApiDiagnosticAnswerHandler(EMBER_ERR_FATAL, NULL, NULL, 0);
  } else {
    finger = addTypeListTlv(finger, requestedTlvs, length);
  }

  assert(finger - payload <= sizeof(payload));
  CoapMessage message;
  emInitStackCoapMessage(&message, destination, payload, (finger - payload));
  if (emStrcmp(uri, DIAGNOSTIC_GET_URI) == 0) {
    message.responseHandler = responseHandler;
  }
  emStoreDefaultIpAddress(message.localAddress.bytes);
  if (emSubmitCoapMessage(&message, (const uint8_t *)uri, NULL_BUFFER)
      != EMBER_SUCCESS) {
    emApiDiagnosticAnswerHandler(EMBER_ERR_FATAL, NULL, NULL, 0);
  }
#endif
}

void emberSendDiagnosticGet(const EmberIpv6Address *destination,
                            const uint8_t *requestedTlvs,
                            uint8_t length)
{
  emApiSendDiagnostic(destination,
                      requestedTlvs,
                      length,
                      (const uint8_t *)DIAGNOSTIC_GET_URI);
}

void emberSendDiagnosticQuery(const EmberIpv6Address *destination,
                              const uint8_t *requestedTlvs,
                              uint8_t length)
{
  emApiSendDiagnostic(destination,
                      requestedTlvs,
                      length,
                      (const uint8_t *)DIAGNOSTIC_QUERY_URI);
}

void emberSendDiagnosticReset(const EmberIpv6Address *destination,
                              const uint8_t *requestedTlvs,
                              uint8_t length)
{
  emApiSendDiagnostic(destination,
                      requestedTlvs,
                      length,
                      (const uint8_t *)DIAGNOSTIC_RESET_URI);
}
