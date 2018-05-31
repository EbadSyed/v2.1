/*
 * File: address-management.h
 * Description: Address Management Functionality
 *
 * Copyright 2015 by Silicon Laboratories. All rights reserved.             *80*
 */

#ifndef ADDRESS_MANAGEMENT_H
#define ADDRESS_MANAGEMENT_H

// Address Management TLVs
enum AddressManagementTlv_e {
  TARGET_EID_TLV            = 0,
  EXTENDED_MAC_ADDRESS_TLV  = 1,
  LOCATOR_TLV               = 2,
  ML_EID_TLV                = 3,
  STATUS_TLV                = 4,
  // 5 was ATTACHED_TIME_TLV, which was dropped from the Thread spec
  LAST_TRANSACTION_TIME_TLV = 6,
  ROUTER_MASK_TLV           = 7,
  ND_OPTION_TLV             = 8,
  ND_DATA_TLV               = 9,
  NETWORK_DATA_TLV          = 10
};

#define MAX_TLV NETWORK_DATA_TLV

#define TARGET_EID_TLV_LENGTH 18
#define EXTENDED_MAC_ADDRESS_TLV_LENGTH 10
#define LOCATOR_TLV_LENGTH 4
#define ML_EID_TLV_LENGTH 10
#define STATUS_TLV_LENGTH 3
#define LAST_TRANSACTION_TIME_TLV_LENGTH 6
#define ROUTER_MASK_TLV_LENGTH 11

// These are the required TLVs for each message.
#define ADDRESS_QUERY_TLVS BIT(TARGET_EID_TLV)
#define ADDRESS_QUERY_RESPONSE_TLVS (BIT(TARGET_EID_TLV) \
                                     | BIT(LOCATOR_TLV)  \
                                     | BIT(ML_EID_TLV))
#define ADDRESS_SOLICIT_TLVS BIT(EXTENDED_MAC_ADDRESS_TLV)
#define ADDRESS_RELEASE_TLVS (BIT(EXTENDED_MAC_ADDRESS_TLV) | BIT(LOCATOR_TLV))
#define ADDRESS_SOLICIT_REPLY_TLVS   BIT(STATUS_TLV)
#define ADDRESS_ERROR_NOTIFICATION_TLVS (BIT(TARGET_EID_TLV) \
                                         | BIT(ML_EID_TLV))

typedef enum {
  STATUS_SUCCESS                  = 0,
  STATUS_NO_ADDRESS_AVAILABLE     = 1,
  STATUS_TOO_FEW_ROUTERS          = 2,
  STATUS_HAVE_CHILD_ID_REQUEST    = 3,
  STATUS_PARENT_PARTITION_CHANGE  = 4,
  STATUS_UNUSED                   = 0xFF
} AddressManagementStatus;

bool emHandleAddressManagementPost(const uint8_t *uri,
                                   const uint8_t *payload,
                                   uint16_t payloadLength,
                                   const EmberCoapRequestInfo *info);
EmberStatus emSendAddressQuery(const EmberIpv6Address *address);

bool emSendAddressSolicitOrRelease(bool isSolicit, AddressManagementStatus status);
#define emSendAddressSolicit(status) emSendAddressSolicitOrRelease(true, status)
#define emSendAddressRelease() \
  emSendAddressSolicitOrRelease(false, STATUS_UNUSED)

#endif // ADDRESS_MANAGEMENT_H
