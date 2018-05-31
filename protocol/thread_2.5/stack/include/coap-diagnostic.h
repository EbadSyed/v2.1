/**
 * @file coap-diagnostic.h
 * @brief Diagnostic Functionality Over CoAP API
 *
 * <!--Copyright 2016 Silicon Laboratories, Inc.                         *80*-->
 */

#ifndef COAP_DIAGNOSTIC_H
#define COAP_DIAGNOSTIC_H

typedef enum {
  DIAGNOSTIC_MAC_EXTENDED_ADDRESS        = 0, // EUI64, 8 bytes
  DIAGNOSTIC_ADDRESS_16                  = 1, // 2-byte address
  DIAGNOSTIC_MODE                        = 2, // capability information
  DIAGNOSTIC_TIMEOUT                     = 3, // 2-byte value, sleepy polling rate
  DIAGNOSTIC_CONNECTIVITY                = 4, // same as connectivity TLV
  DIAGNOSTIC_ROUTING_TABLE               = 5, // same as Route64 TLV (MLE type 9)
  DIAGNOSTIC_LEADER_DATA                 = 6, // same as TLV type 11
  DIAGNOSTIC_NETWORK_DATA                = 7, // Same as TLV Type 12
  DIAGNOSTIC_IPV6_ADDRESS_LIST           = 8,  // List of all IPv6 addresses
                                               // registered by the device
  DIAGNOSTIC_MAC_COUNTERS                = 9,  // packet/event counters for 802.15.4 interface.
  DIAGNOSTIC_BATTERY_LEVEL               = 14,
  DIAGNOSTIC_VOLTAGE                     = 15, // 2-btye value
  DIAGNOSTIC_CHILD_TABLE                 = 16, // Structure containing information
                                               // on all children
  DIAGNOSTIC_CHANNEL_PAGES               = 17, // one or more 8 bit integers
  DIAGNOSTIC_TYPE_LIST                   = 18, // TLV to hold other diagnostic TLVs
  LAST_DIAGNOSTIC_VALUE = DIAGNOSTIC_CHANNEL_PAGES
} EmberDiagnosticValue;

// The following TLVs have been separated from Diagnostic TLVs
// as they belong to the Mac Counters TLV list
typedef enum {
  DIAGNOSTIC_PACKETS_SENT                = 9,  // 2-btye value
  DIAGNOSTIC_PACKETS_RECEIVED            = 10, // 2-btye value
  DIAGNOSTIC_PACKETS_DROPPED_ON_TRANSMIT = 11, // 2-btye value
  DIAGNOSTIC_PACKETS_DROPPED_ON_RECEIVE  = 12, // 2-btye value
  DIAGNOSTIC_SECURITY_ERRORS             = 13, // 2-btye value
  DIAGNOSTIC_NUMBER_OF_RETRIES           = 14, // 2-btye value
} MacCountersValue;

// there are 14 Diagnostic TLVs
#define TYPE_LIST_TLV_LENGTH          16

typedef struct {
  uint32_t tlvMask;

  // EUI64, 8 bytes
  const uint8_t *macExtendedAddress;

  // 2-byte address
  uint16_t address16;

  uint8_t mode;
  uint16_t timeout;

  // connectivity TLV (MLE TLV Type 15)
  // points to the length byte
  const uint8_t *connectivity;

  // Routing Table, Router64 TLV (MLE Type 9)
  // points to the length byte
  const uint8_t *routingTable;

  // Leader Data TLV, same as TLV Type 11
  // points to the length byte
  const uint8_t *leaderData;

  // Network Data TLV, same as TLV Type 12
  // points to the length byte
  const uint8_t *networkData;

  // IPv6 Address List, points to the length byte
  const uint8_t *ipv6AddressList;

  const uint8_t *macCounters;
  uint8_t batteryLevel;
  uint16_t voltage;

  // Child Table, points to the length byte
  const uint8_t *childTable;

  // Channel Pages TLV, point to the length byte
  const uint8_t *channelPages;
} EmberDiagnosticData;

typedef struct {
  uint16_t packetsSent;
  uint16_t packetsReceived;
  uint16_t packetsDroppedOnTransmit;
  uint16_t packetsDroppedOnReceive;
  uint16_t securityErrors;
  uint16_t numberOfRetries;
} MacCountersData;

#define emberDiagnosticDataHasTlv(data, tlv) \
  (((data)->tlvMask & BIT(tlv)) == BIT(tlv))

void emApiSendDiagnostic(const EmberIpv6Address *destination,
                         const uint8_t *requestedTlvs,
                         uint8_t length,
                         const uint8_t *uri);
/**
 * @brief  This function sends a CoAP diagnostic query. See emberDiagnosticAnswerHandler() for callback.
 *
 * @param destination   The destination, which may be unicast or multicast.
 *
 * @param requestedTlvs   An array of requested TLVs.
 *
 * @param length   The length of requestedTlvs.
 */
void emberSendDiagnosticQuery(const EmberIpv6Address *destination,
                              const uint8_t *requestedTlvs,
                              uint8_t length);

/**
 * @brief This function sends a CoAP diagnostic get. See emberDiagnosticAnswerHandler() for callback.
 *
 * @param destination   The destination, which must be unicast.
 *
 * @param requestedTlvs   An array of requested TLVs.
 *
 * @param length   The length of requestedTlvs.
 */
void emberSendDiagnosticGet(const EmberIpv6Address *destination,
                            const uint8_t *requestedTlvs,
                            uint8_t length);

/**
 * @brief This function sends a CoAP diagnostic reset. See emberDiagnosticAnswerHandler() for callback.
 *
 * @param destination   The destination, which may be unicast or multicast.
 *
 * @param requestedTlvs   An array of TLVs marked for reset.
 *
 * @param length   The length of requestedTlvs.
 */
void emberSendDiagnosticReset(const EmberIpv6Address *destination,
                              const uint8_t *requestedTlvs,
                              uint8_t length);

/**
 * @brief Application callback for emberSendDiagnosticQuery() and emberSendDiagnosticGet().
 *
 * @param status   Status of the query result.
 *
 * @param remoteAddress   The remote address that sent the answer.
 *
 * @param payload   The returned payload.
 *
 * @param payloadLength The returned payload length.
 */
void emberDiagnosticAnswerHandler(EmberStatus status,
                                  const EmberIpv6Address *remoteAddress,
                                  const uint8_t *payload,
                                  uint8_t payloadLength);

#endif // COAP_DIAGNOSTIC_H
