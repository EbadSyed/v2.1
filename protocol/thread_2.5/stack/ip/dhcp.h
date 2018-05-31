/*
 * File: dhcp.h
 * Description: DHCPv6
 *
 * Copyright 2013 Silicon Laboratories, Inc.                                *80*
 */

#ifndef DHCP_H
#define DHCP_H

// Incoming messages.  Returns true if the message was process successfully.
void emDhcpClientIncomingMessageHandler(Ipv6Header *ipHeader);
bool emSendDhcpAddressRelease(uint16_t shortId, const uint8_t *longId);

// Server calls - no em... because the DHCP server is only for testing
// and is not released.

void addDhcpServer(const uint8_t *prefix,
                   uint32_t validLifetime,
                   uint32_t preferredLifetime);
void dhcpServerIncomingMessageHandler(const uint8_t *source,
                                      const uint8_t *bytes,
                                      uint16_t bytesLength);
void acceptDhcpSolicit(bool yesno);

//----------------------------------------------------------------

#define DHCP_CLIENT_PORT 546
#define DHCP_SERVER_PORT 547

//----------------------------------------------------------------
// These are really internal to DHCP but are needed by test code.

// DHCP messages.
#define DHCP_SOLICIT               1
#define DHCP_ADVERTISE             2
#define DHCP_REQUEST               3
#define DHCP_CONFIRM               4
#define DHCP_RENEW                 5
#define DHCP_REBIND                6
#define DHCP_REPLY                 7
#define DHCP_RELEASE               8
#define DHCP_DECLINE               9
#define DHCP_RECONFIGURE          10
#define DHCP_INFORMATION_REQUEST  11
#define DHCP_RELAY_FORW           12
#define DHCP_RELAY_REPL           13
#define DHCP_LEASEQUERY           14
#define DHCP_LEASEQUERY_REPLY     15

#define DHCP_MAX_COMMAND DHCP_LEASEQUERY_REPLY

// DHCP options.
// The names are from RFC 3315, so don't blame me for them.
#define DHCP_CLIENTID_OPTION       1
#define DHCP_SERVERID_OPTION       2
#define DHCP_IA_NA_OPTION          3    // IA_NA = Identity Association
//    for Nontemporary Addresses
#define DHCP_IA_TA_OPTION          4    // Ditto, without the Non-
#define DHCP_IAADDR_OPTION         5
#define DHCP_ORO_OPTION            6    // ORO = Option Request Option
#define DHCP_PREFERENCE_OPTION     7
#define DHCP_ELAPSED_TIME_OPTION   8
#define DHCP_RELAY_MSG_OPTION      9
// There is no 10.
#define DHCP_AUTH_OPTION          11
#define DHCP_UNICAST_OPTION       12
#define DHCP_STATUS_CODE_OPTION   13
#define DHCP_RAPID_COMMIT_OPTION  14
#define DHCP_USER_CLASS_OPTION    15
#define DHCP_VENDOR_CLASS_OPTION  16
#define DHCP_VENDOR_OPTS_OPTION   17
#define DHCP_INTERFACE_ID_OPTION  18
#define DHCP_RECONF_MSG_OPTION    19
#define DHCP_RECONF_ACCEPT_OPTION 20

#define DHCP_MAX_OPTION_CODE DHCP_RECONF_ACCEPT_OPTION

// Bitmask indexes for the options we actually use.  This can be an
// enum because the values are only used internally.
//
// This must be synchronized with the optionIds[] array in dhcp.c.

enum {
  DHCP_CLIENTID_OPTION_NUMBER,
  DHCP_SERVERID_OPTION_NUMBER,
  DHCP_IA_NA_OPTION_NUMBER,
  DHCP_IAADDR_OPTION_NUMBER,
  DHCP_ORO_OPTION_NUMBER,
  DHCP_ELAPSED_TIME_OPTION_NUMBER,
  DHCP_STATUS_CODE_OPTION_NUMBER,
  DHCP_RAPID_COMMIT_OPTION_NUMBER,
  DHCP_VENDOR_OPTS_OPTION_NUMBER,
  DHCP_CLIENT_LAST_TIME_OPTION_NUMBER,

  // These indicate the presence of options nested inside other options.
  DHCP_IA_NA_STATUS_OPTION_NUMBER, // alias for STATUS_CODE inside IA_NA
  DHCP_IAADDR_STATUS_OPTION_NUMBER, // alias for STATUS_CODE inside IAADDR
};

// But do count them for indexes.
#define DHCP_OPTION_INDEX_COUNT (DHCP_IAADDR_STATUS_OPTION_NUMBER + 1)

#define DHCP_STATUS_SUCCESS         0 // Success.
#define DHCP_STATUS_UNSPEC_FAIL     1 // Failure, reason unspecified;
#define DHCP_STATUS_NO_ADDRS_AVAIL  2 // Server has no addresses available.
#define DHCP_STATUS_NO_BINDING      3 // Client record (binding) unavailable.
#define DHCP_STATUS_NOT_ON_LINK     4 // The prefix is not appropriate.
#define DHCP_STATUS_USE_MULTICAST   5 // Force client to use multicast.

// From RFC 5007 DHCPv6 Leasequery
#define DHCP_UNKNOWN_QUERY_TYPE     7 // Query-type is unknown or not supported.
#define DHCP_MALFORMED_QUERY        8 // Query is not valid.
#define DHCP_NOT_CONFIGURED         9 // Server doesn't have the target address.
#define DHCP_NOT_ALLOWED           10 // Server does not allow the request.

// Two bytes of ID and two bytes of length.
#define DHCP_OPTION_HEADER_LENGTH 4

// DHCP Unique Identifier (DUID)

#define DUID_TYPE_LINK_LAYER_TIME 1  // link-layer address plus timeout
#define DUID_TYPE_ENTERPRISE      2  // enteprise number plus vendor-assigned ID
#define DUID_TYPE_LINK_LAYER      3  // static link-layer address

// See Hardware Types in
// http://www.iana.org/assignments/arp-parameters/arp-parameters.xhtml
#define DUID_HARDWARE_TYPE_EUI64 27

//----------------------------------------------------------------

typedef struct {
  uint16_t command;
  uint32_t transactionId;
  uint32_t optionMask;            // which options are present

  const uint8_t *optionsStart;    // beginning of options
  const uint8_t *optionsEnd;      // pointer to first byte after options

  // Option data
  // There are two client IDs in LQ Responses.
  uint8_t clientLongId[8];        // client ID option
  uint8_t serverLongId[8];        // server ID option
  uint8_t targetLongId[8];        // client ID option inside LQ query and
                                  //  client data options

  uint16_t status;                // status option

  uint32_t iaid;                  // IA_NA option
  uint32_t t1;                    // IA_NA option
  uint32_t t2;                    // IA_NA option
  uint16_t iaNaStatus;            // status option inside IA_NA option

  const uint8_t *address;         // IAADDR option
  uint32_t preferredLifetime;     // IAADDR option
  uint32_t validLifetime;         // IAADDR option
  uint16_t iaaddrStatus;          // status option inside IAADDR option

  uint32_t lastTransactionSeconds;// client last transaction time
                                  // (DHCP_CLIENT_LAST_TIME_OPTION)
} DhcpMessage;

void emLogDhcp(bool incoming, const uint8_t *ipAddr, uint8_t command);

uint8_t *emAddDhcpHeader(uint8_t *finger,
                         uint8_t command,
                         DhcpMessage *request,
                         uint32_t transactionId);

bool emParseDhcpOptions(DhcpMessage *message,
                        const uint8_t *end,
                        uint32_t allowedOptions,
                        uint32_t requiredOptions);

#endif // DHCP_H
