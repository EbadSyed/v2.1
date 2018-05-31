/*
 * File: commission.h
 * Description: Thread commissioning
 *
 * Copyright 2015 Silicon Laboratories, Inc.                                *80*
 */

#ifndef COMMISSION_H
#define COMMISSION_H

// Com = commissioner
// BR = border router
// JR = joiner router
//
// For on-mesh commissioners the commissioner and the border router
// are the same device.

// Commissioner petition and keep alive

#define BORDER_KEEP_ALIVE_URI    "c/ca"  // Com->BR
#define LEADER_KEEP_ALIVE_URI    "c/la"  // BR->leader
#define BORDER_PETITION_URI      "c/cp"  // Com->BR
#define LEADER_PETITION_URI      "c/lp"  // BR->leader

// Joining

#define DTLS_RELAY_TRANSMIT_URI  "c/tx"  // Com->BR->JR
#define DTLS_RELAY_RECEIVE_URI   "c/rx"  // JR->BR->Com
#define JOIN_FINAL_URI           "c/jf"  // joiner->Com via DTLS
#define JOINER_ENTRUST_URI       "c/je"  // JR->joiner using special MAC key
#define JOIN_APP_URI             "c/ja"  // any->Com

// Network management

// These are all Com->BR->leader.
#define COMMISSIONER_GET_URI       "c/cg"
#define COMMISSIONER_SET_URI       "c/cs"
#define MANAGEMENT_ACTIVE_GET_URI  "c/ag" // no handler on BR, how does it work?
#define MANAGEMENT_ACTIVE_SET_URI  "c/as" // no handler on BR, how does it work?
#define MANAGEMENT_PENDING_GET_URI "c/pg" // no handler on BR, how does it work?
#define MANAGEMENT_PENDING_SET_URI "c/ps" // no handler on BR, how does it work?

#define MANAGEMENT_DATASET_CHANGED_URI "c/dc" // leader->Com

#define MANAGEMENT_ANNOUNCE_BEGIN_URI "c/ab"  // Com->any, may be multicast

#define MGMT_PANID_SCAN_URI      "c/pq"  // Com->any, may be multicast
#define MGMT_PANID_CONFLICT_URI  "c/pc"  // any->Com
#define MGMT_ENERGY_SCAN_URI     "c/es"  // Com->any, may be multicast
#define MGMT_ENERGY_REPORT_URI   "c/er"  // any->Com

// SPEC-460 lists these values as obtainable from c/mg:
//   Provisioning URL TLV
//   Vendor Name TLV
//   Vendor Model TLV
//   Vendor SW Version TLV
//   Vendor Data TLV
//   Vendor Stack Version TLV
#define MANAGEMENT_GET_URI       "c/mg"  //             // not implemented

#define UDP_PROXY_TRANSMIT_URI   "c/ut"  // Com->BR     // not implemented
#define UDP_PROXY_RECEIVE_URI    "c/ur"  // BR->Com     // not implemented

// The rest are used within the mesh and not sent to the commissioner.

#define SERVER_DATA_URI          "a/sd"

#define ADDRESS_QUERY_URI        "a/aq"
#define ADDRESS_NOTIFICATION_URI "a/an"
#define ADDRESS_SOLICIT_URI      "a/as"
#define ADDRESS_RELEASE_URI      "a/ar"
#define ADDRESS_ERROR_NOTIFICATION_URI "a/ae"
#define ND_DATA_REQUEST_URI      "a/nd"

//----------------------------------------------------------------

#define COMMISSION_CHANNEL_TLV                  0x00
#define COMMISSION_PAN_ID_TLV                   0x01
#define COMMISSION_EXTENDED_PAN_ID_TLV          0x02
#define COMMISSION_NETWORK_NAME_TLV             0x03
#define COMMISSION_PSKC_TLV                     0x04
#define COMMISSION_NETWORK_MASTER_KEY_TLV       0x05
#define COMMISSION_NETWORK_KEY_SEQUENCE_TLV     0x06
#define COMMISSION_MESH_LOCAL_ULA_TLV           0x07
#define COMMISSION_STEERING_DATA_TLV            0x08
#define COMMISSION_BORDER_ROUTER_LOCATOR_TLV    0x09
#define COMMISSION_COMMISSIONER_ID_TLV          0x0A
#define COMMISSION_SESSION_ID_TLV               0x0B
#define COMMISSION_SECURITY_POLICY_TLV          0x0C
#define COMMISSION_GET_TLV                      0x0D
#define COMMISSION_ACTIVE_TIMESTAMP_TLV         0x0E
#define COMMISSION_COMMISSIONER_UDP_PORT_TLV    0x0F
#define COMMISSION_STATE_TLV                    0x10
#define COMMISSION_JOINER_DTLS_ENCAP_TLV        0x11
#define COMMISSION_JOINER_UDP_PORT_TLV          0x12
#define COMMISSION_JOINER_ADDRESS_TLV           0x13
#define COMMISSION_JOINER_ROUTER_LOCATOR_TLV    0x14
#define COMMISSION_JOINER_ROUTER_KEK_TLV        0x15
// big gap
#define COMMISSION_PROVISIONING_URL_TLV         0x20
#define COMMISSION_VENDOR_NAME_TLV              0x21
#define COMMISSION_VENDOR_MODEL_TLV             0x22
#define COMMISSION_VENDOR_SW_VERSION_TLV        0x23
#define COMMISSION_VENDOR_DATA_TLV              0x24
#define COMMISSION_VENDOR_STACK_VERSION_TLV     0x25
// big gap
#define COMMISSION_UDP_ENCAP_TLV                0x30
#define COMMISSION_IPV6_ADDRESS_TLV             0x31
#define COMMISSION_FORWARDING_PORT_TLV          0x32
#define COMMISSION_PENDING_TIMESTAMP_TLV        0x33
#define COMMISSION_DELAY_TIMER_TLV              0x34
#define COMMISSION_CHANNEL_MASK_TLV             0x35
#define COMMISSION_SCAN_COUNT_TLV               0x36
#define COMMISSION_SCAN_PERIOD_TLV              0x37
#define COMMISSION_SCAN_DURATION_TLV            0x38
#define COMMISSION_SCAN_ENERGY_LIST_TLV         0x39
// enormous gap (what were they thinking?)
#define COMMISSION_DISCOVERY_REQUEST_TLV        0x80
#define COMMISSION_DISCOVERY_RESPONSE_TLV       0x81

#define MAX_TLV_TYPE COMMISSION_DISCOVERY_RESPONSE_TLV

//----------------------------------------------------------------
// These are the TLVs that the code needs to reference directly.  Unlike
// the full list, this list is small enough that we can use an int32u as
// a bitmask for a set of TLVs.

enum {
  // command parameters
  COMMISSION_STATE_TLV_FLAG,
  COMMISSION_SESSION_ID_TLV_FLAG,
  COMMISSION_BORDER_ROUTER_LOCATOR_TLV_FLAG,
  COMMISSION_JOINER_ROUTER_LOCATOR_TLV_FLAG,
  COMMISSION_JOINER_UDP_PORT_TLV_FLAG,
  COMMISSION_COMMISSIONER_ID_TLV_FLAG,
  COMMISSION_SECURITY_POLICY_TLV_FLAG,
  COMMISSION_PSKC_TLV_FLAG,
  COMMISSION_ACTIVE_TIMESTAMP_TLV_FLAG,
  COMMISSION_GET_TLV_FLAG,
  COMMISSION_JOINER_DTLS_ENCAP_TLV_FLAG,
  COMMISSION_JOINER_ADDRESS_TLV_FLAG,
  COMMISSION_JOINER_ROUTER_KEK_TLV_FLAG,

  // The values that a joiner needs to receive in the joiner entrust message.
  COMMISSION_NETWORK_MASTER_KEY_TLV_FLAG,
  COMMISSION_NETWORK_KEY_SEQUENCE_TLV_FLAG,
  COMMISSION_EXTENDED_PAN_ID_TLV_FLAG,
  COMMISSION_NETWORK_NAME_TLV_FLAG,
  COMMISSION_MESH_LOCAL_ULA_TLV_FLAG,

  // Special because it is in the network data
  COMMISSION_STEERING_DATA_TLV_FLAG,

  COMMISSION_CHANNEL_TLV_FLAG,

  // Used in scan requests
  COMMISSION_PAN_ID_TLV_FLAG,

  // Used in join final message
  COMMISSION_PROVISIONING_URL_TLV_FLAG,

  // TLVs used by active and energy scan requests
  COMMISSION_CHANNEL_MASK_TLV_FLAG,
  COMMISSION_SCAN_COUNT_TLV_FLAG,
  COMMISSION_SCAN_PERIOD_TLV_FLAG,
  COMMISSION_SCAN_DURATION_TLV_FLAG,
  COMMISSION_SCAN_ENERGY_LIST_TLV_FLAG,

  // TLVs used in discovery responses
  COMMISSION_DISCOVERY_RESPONSE_TLV_FLAG,
  COMMISSION_COMMISSIONER_UDP_PORT_TLV_FLAG,

  // TLVs used in UDP Proxy notification
  COMMISSION_UDP_ENCAP_TLV_FLAG,
  COMMISSION_IPV6_ADDRESS_TLV_FLAG
};

// Bits in the tlvMask of parsed commissioning messages
#define COMM_TLV_BIT(tlv) BIT32(COMMISSION_##tlv##_FLAG)

typedef struct {
  uint32_t tlvMask;

  // command parameters
  uint8_t state;
  uint16_t sessionId;
  EmberNodeId borderRouterNodeId;
  EmberNodeId joinerRouterNodeId;
  uint16_t joinerUdpPort;
  const uint8_t *commissionerIdTlv;
  const uint8_t *getTlv;
  const uint8_t *joinerDtlsEncapTlv;
  const uint8_t *joinerAddress;
  const uint8_t *joinerRouterKek;

  // the values that a joiner needs to receive in the joiner entrust message
  uint8_t *masterKey;
  uint32_t keySequence;
  const uint8_t *extendedPanId;
  const uint8_t *networkNameTlv;
  const uint8_t *meshLocalUla;
  const uint8_t *securityPolicy;
  const uint8_t *pskcTlv;
  const uint8_t *activeTimestamp;

  // Special because it is in the network data
  const uint8_t *steeringDataTlv;

  const uint8_t *channelTlv;

  // Used in scan requests
  uint16_t panId;

  // Used in join final message
  const uint8_t *provisioningUrlTlv;

  // TLVs used by active and energy scan requests
  uint8_t *channelMask;
  uint8_t scanCount;
  uint16_t scanPeriod;
  uint16_t scanDuration;
  const int8_t *scanEnergyListTlv;

  // TLVs used by network discovery
  uint16_t commissionerUdpPort;
  uint8_t discoveryResponseFlags;
} CommissionMessage;

//----------------------------------------------------------------
// "Required" TLVs for each URI

#define NO_TLVS 0

#define KEEP_ALIVE_TLVS (COMM_TLV_BIT(SESSION_ID_TLV) \
                         | COMM_TLV_BIT(STATE_TLV))
#define PETITION_TLVS COMM_TLV_BIT(COMMISSIONER_ID_TLV)

#define GET_DATA_TLVS 0
#define SET_DATA_TLVS 0

// TODO Adding BIT(...) of the required TLVs seems to fail
// here since they're high-numbered.
#define PANID_SCAN_TLVS (COMM_TLV_BIT(CHANNEL_MASK_TLV) \
                         | COMM_TLV_BIT(PAN_ID_TLV))
#define PANID_CONFLICT_TLVS 0

#define ENERGY_SCAN_TLVS (COMM_TLV_BIT(CHANNEL_MASK_TLV)    \
                          | COMM_TLV_BIT(SCAN_COUNT_TLV)    \
                          | COMM_TLV_BIT(SCAN_PERIOD_TLV)   \
                          | COMM_TLV_BIT(SCAN_DURATION_TLV) \
                          | COMM_TLV_BIT(SESSION_ID_TLV))
#define ENERGY_REPORT_TLVS (COMM_TLV_BIT(CHANNEL_MASK_TLV) \
                            | COMM_TLV_BIT(SCAN_ENERGY_LIST_TLV))

#define RELAY_RX_TLVS (COMM_TLV_BIT(JOINER_UDP_PORT_TLV)         \
                       | COMM_TLV_BIT(JOINER_ADDRESS_TLV)        \
                       | COMM_TLV_BIT(JOINER_ROUTER_LOCATOR_TLV) \
                       | COMM_TLV_BIT(JOINER_DTLS_ENCAP_TLV))

#define RELAY_TX_TLVS (COMM_TLV_BIT(JOINER_UDP_PORT_TLV)         \
                       | COMM_TLV_BIT(JOINER_ADDRESS_TLV)        \
                       | COMM_TLV_BIT(JOINER_ROUTER_LOCATOR_TLV) \
                       | COMM_TLV_BIT(JOINER_DTLS_ENCAP_TLV))

#define PROXY_TX_TLVS (COMM_TLV_BIT(IPV6_ADDRESS_TLV) \
                       | COMM_TLV_BIT(UDP_ENCAP_TLV))

#define PROXY_RX_TLVS 0

#define JOIN_FINAL_TLVS NO_TLVS
#define JOIN_APP_TLVS NO_TLVS

//----------------------------------------------------------------

#define MAX_LEADER_KEEPALIVE_RESPONSE 3
#define COMMISSION_MAX_STEERING_DATA_SIZE 16
#define DISCOVERY_RESPONSE_NATIVE_COMMISSIONER_FLAG 0x08

#define COMMISSION_SUCCESS 0x01
#define COMMISSION_FAILURE 0xFF

#define CHANNEL_MASK_PAGE_INDEX   0
#define CHANNEL_MASK_LENGTH_INDEX 1
#define CHANNEL_MASK_INDEX        2

//----------------------------------------------------------------
void emCommissionInit(void);
void emInitializeCommissioningData(void);
void emSendCommissionerPetition(uint8_t *commissionerId,
                                uint8_t commissionerIdLength);
void emSendJoinSteeringData(uint8_t *steeringData, uint8_t steeringDataLength);
void emRemoveCommissioner(void);
void emSetCommissionState(const uint8_t *networkDataTlv);
void emHandleIncomingCommission(PacketHeader header, Ipv6Header *ipHeader);
bool emStartAppCommissioning(bool joinerSession);
void emForwardToCommissioner(PacketHeader header, Ipv6Header *ipHeader);
void emForwardToJoiner(const uint8_t *joinerIid,
                       uint16_t joinerPort,
                       EmberNodeId joinerRouterNodeId,
                       const uint8_t *kek,
                       Buffer payload);
bool emAmThreadCommissioner(void);
bool emSteeringDataMatch(uint8_t *data, uint8_t length);
void emSetSteeringData(const uint8_t *steeringData, uint8_t steeringDataLength);
extern uint16_t emCommissionerSessionId;

// Messages sent by an external commissioner.
void emBecomeExternalCommissioner(const uint8_t *id, uint8_t idLength);
void emExternalCommissionerKeepAlive(bool accept);

bool emComSendPanIdScanRequest(const int8u *destination,
                               uint32_t channelMask,
                               uint16_t panId);
bool emComSendEnergyScanRequest(const int8u *destination,
                                uint32_t channelMask,
                                uint8_t scanCount,
                                uint16_t scanPeriod,
                                uint16_t scanDuration);
void emMgmtEnergyScanHandler(uint8_t channel, int8_t maxRssiValue);
void emMgmtEnergyScanComplete(void);
void emMgmtActiveScanHandler(const EmberMacBeaconData *beaconData);
void emMgmtActiveScanComplete(void);
uint32_t emFetchChannelMask(const uint8_t *channelMaskTlv);

// The types of values that TLVs can contain.  POINTER is a pointer to the
// contents of a TLV of a known length, TLV is a pointer to the start of
// the TLV so that the length can be read.
enum MessageType_e {
  NOT_USED0,
  INT8,
  INT16,
  INT32,
  POINTER,
  TLV,
  NOT_USED
};

typedef uint8_t MessageType;

typedef const struct {
  uint16_t  offset;
  uint8_t   flagIndex;
  uint8_t   type;
  uint8_t   minLength; // POINTER and TLV types, for POINTER it is the fixed size
  uint16_t  maxLength; // TLV types
} TlvData;

// Returns NULL if the tlvId is not valid.
typedef TlvData *(*TlvDataGetter)(uint8_t tlvId);

bool emParseTlvMessage(void *message,
                       uint32_t *tlvMask,
                       const uint8_t *payload,
                       uint16_t length,
                       TlvDataGetter getTlvData);

uint8_t *emAddInt16uTlv(uint8_t *finger, uint8_t tlvType, uint16_t value);
uint8_t *emAddInt32uTlv(uint8_t *finger, uint8_t tlvType, uint32_t value);
uint8_t *emAddTlv(uint8_t *finger, uint8_t type, const uint8_t *data, uint8_t length);
uint8_t *emAddSessionIdTlv(uint8_t *finger);
uint8_t *emAddDiscoveryResponseTlvs(uint8_t *finger);
uint8_t *emAddNetworkNameTlv(uint8_t *finger);
uint8_t *emAddMeshLocalUlaTlv(uint8_t *finger);
uint8_t *emAddExtendedPanIdTlv(uint8_t *finger);
uint8_t *emAddCommissioningDataTlv(uint8_t *finger);

void emUdpProxyRxHandler(const Ipv6Header* ipHeader);

#ifdef EMBER_TEST
const char *emCommissionTlvName(uint8_t type);
#endif

extern bool emHashEui64;

extern uint8_t emProvisioningUrl[64];
extern uint8_t emProvisioningUrlLength;

extern EmberNodeId emBorderRouterNodeId;

#define DELAY_JOIN_ENTRUST_MS 50

#define SCAN_DELAY_MS 1000

#endif // COMMISSION_H
