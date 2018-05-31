/*
 * File: mle.h
 * Description: Mesh Link Exchange
 * Author(s): Matteo Paris, matteo@ember.com
 *
 * Copyright 2011 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef MLE_H
#define MLE_H

void emMleInit(void);
bool emSendMleRequest(uint8_t *ipDest, bool jitter, bool isLegacy);
bool emSendMleLurkerJoinRequest(uint8_t *ipDest);
bool emSendMleRouterIdVerifyRequest(void);
bool emSendMlePseudoBeaconRequest(uint8_t scanMask);
bool emSendMleParentRequest(bool everything);
bool emSendMleChildIdRequest(uint8_t *ipDest,
                             const uint8_t *response,
                             uint8_t responseLength,
                             bool amSleepy,
                             bool darkRouterActivationOnly);
bool emSendMleAnnounce(uint8_t txChannel,
                       const uint8_t *timestamp,
                       uint8_t channelPage,
                       uint16_t channel,
                       uint16_t panId);

// Incoming messages.  Returns true if the message was process successfully.
bool emMleIncomingMessageHandler(PacketHeader header,
                                 Ipv6Header *ipHeader,
                                 PacketHeader headerForReuse);

// For jittered responses to multicast requrests.
PacketHeader emPrepareRetriedMleMessage(PacketHeader header);

void emMleLurkerAcceptHandler(void);
void emMleAttachCompleteHandler(bool success);

uint8_t *emGetMleKey(uint32_t sequence, uint8_t *storage);
uint8_t *emGetLegacyMleKey(void);

void emSendMleAdvertisement(void);

void emSetMleFrameCounter(uint32_t newFrameCounter);
bool emSendNetworkDataRequest(const uint8_t *ipDest,
                              bool includeIdMask,
                              uint16_t jitter);

// options for emSendNetworkData
typedef enum {
  MLE_INCLUDE_RIP_TLV         = 0x01,
  MLE_STABLE_DATA_ONLY        = 0x02,
  MLE_RETURN_HEADER           = 0x04,
  MLE_INCLUDE_ACTIVE_DATASET  = 0x08,
  MLE_INCLUDE_PENDING_DATASET = 0x10
} SendNetworkDataOption_e;

typedef uint8_t SendNetworkDataOption;

PacketHeader emSendNetworkData(const uint8_t *ipDest,
                               SendNetworkDataOption options);

bool emSendMleDataRequestTest(const uint8_t *ipDest, uint16_t requestedTlvs);

//----------------------------------------------------------------
// These are really internal to MLE but are needed by test code.

// MLE commands.
typedef enum {
  MLE_REQUEST               = 0,
  MLE_ACCEPT                = 1,
  MLE_ACCEPT_AND_REQUEST    = 2,
  MLE_REJECT                = 3,
  MLE_ADVERTISEMENT         = 4,
  MLE_UPDATE                = 5,
  MLE_UPDATE_REQUEST        = 6,
  MLE_DATA_REQUEST          = 7,
  MLE_DATA_RESPONSE         = 8,
  MLE_PARENT_REQUEST        = 9,
  MLE_PARENT_RESPONSE       = 10,
  MLE_CHILD_ID_REQUEST      = 11,
  MLE_CHILD_ID_RESPONSE     = 12,
  MLE_CHILD_UPDATE          = 13,
  MLE_CHILD_UPDATE_RESPONSE = 14,
  MLE_ANNOUNCE              = 15,
  MLE_DISCOVERY_REQUEST     = 16,
  MLE_DISCOVERY_RESPONSE    = 17,

  // this must come last
  MLE_COMMAND_COUNT
} MleCommand;

// MLE TLVs
#define MLE_SOURCE_ADDRESS_TLV 0
#define MLE_MODE_TLV 1
#define MLE_TIMEOUT_TLV 2
#define MLE_CHALLENGE_TLV 3
#define MLE_RESPONSE_TLV 4
#define MAC_FRAME_COUNTER_TLV 5
#define MLE_NEIGHBOR_TLV 6
#define MLE_PARAMETER_TLV 7
#define MLE_FRAME_COUNTER_TLV 8
#define MLE_ROUTE_64_TLV 9
#define MLE_ADDRESS_16_TLV 10
// Contains the contents of the NWK_DATA_LEADER TLV from the network data.
#define MLE_LEADER_DATA_TLV   11
// Contains the full network data.
#define MLE_NETWORK_DATA_TLV  12
#define MLE_TLV_REQUEST_TLV   13
#define MLE_SCAN_MASK_TLV     14
#define MLE_CONNECTIVITY_TLV  15
#define MLE_LINK_MARGIN_TLV   16
#define MLE_STATUS_TLV        17
#define MLE_VERSION_TLV       18
#define MLE_ADDRESS_REGISTRATION_TLV 19
#define MLE_CHANNEL_TLV              20
#define MLE_PAN_ID_TLV               21
#define MLE_ACTIVE_TIMESTAMP_TLV     22
#define MLE_PENDING_TIMESTAMP_TLV    23
#define MLE_ACTIVE_OPERATIONAL_DATASET_TLV  24
#define MLE_PENDING_OPERATIONAL_DATASET_TLV 25
#define MLE_THREAD_DISCOVERY_TLV            26
#define MLE_MAX_TLV_CODE MLE_THREAD_DISCOVERY_TLV

#define MLE_PARAMETER_CHANNEL 0
#define MLE_PARAMETER_PAN_ID 1
#define MLE_PARAMETER_ALLOW_JOIN 2

typedef enum {
  MLE_SCAN_FULL_ROUTERS = 0x80,
  MLE_SCAN_DARK_ROUTERS = 0x40
} MleScanType;

// Connectivity data has ten bytes
#define MLE_CONNECTIVITY_PARENT_PRIORITY 0
#define MLE_CONNECTIVITY_LINK_QUALITY_3_INDEX  1
#define MLE_CONNECTIVITY_LINK_QUALITY_2_INDEX  2
#define MLE_CONNECTIVITY_LINK_QUALITY_1_INDEX  3
#define MLE_CONNECTIVITY_COST_TO_LEADER_INDEX  4
#define MLE_CONNECTIVITY_RIP_ID_SEQUENCE_INDEX 5
#define MLE_CONNECTIVITY_RIP_ACTIVE_ROUTERS 6
#define MLE_CONNECTIVITY_RIP_SED_BUFFER_SIZE 7  // 2 bytes
#define MLE_CONNECTIVITY_RIP_SED_DATAGRAM_COUNT 9
#define MLE_CONNECTIVITY_LENGTH 10

#define SED_DATAGRAM_COUNT 1 // Currently set to a minimum value as per the Thread Conformance Doc.
#define SED_BUFFER_SIZE 0x0046  // Size of one IPv6 datagram (in octets) for each attached SED

// Thread Mode bits
// Thread used to use the 802.15.4 capability bits in the mode TLV,
// but then it diverged; therefore we define the mode bits separately here.
//
// In 1.1 we had everyone but lurkers set the "allocate address" bit.
// We can't do that any more, because Thread has that bit as reserved.
// In the hybrid stack 1.1 and 2.0 messages don't get mixed, so we are
// OK there, but there are still tests that run with 2.0 lurkers.  For
// those we flip the meaning of the bit and have lurkers set the
// "allocate address" bit and everyone else leave it clear.

#define MODE_COMPLETE_NETWORK_DATA  0x01
#define MODE_DEVICE_TYPE            0x02
#define MODE_SECURE_DATA_REQUESTS   0x04
#define MODE_RECEIVER_ON_WHEN_IDLE  0x08

// We need some limit in order to size buffer space.  There is no
// point in making the challenge longer than the key we are
// synchronizing on.
#define MLE_MAX_CHALLENGE_LENGTH 16

// MLE Route TLV (formerly known as RIP TLV):
//
// index  0: rip id sequence number
//        1: assigned rip id mask
//        9: one byte entry per assigned id
//
// Total length is variable.
//
// A route entry has the following format:
//
// Bits 0-3  RIP metric
// Bits 4-5  Incoming link quality
// Bits 6-7  Outgoing link qulity
//
// The index of the entry int the rip id mask is the id of the destination.
// A RIP metric value of 0 means no route (also known as infinity).
// An incoming or outgoing link quality value of 0 means no link.
//
// We used to include the next hops at the end of the TLV for loop avoidance,
// but we no longer do for space reasons.
//
// RIP_MAX_ROUTERS inlined to avoid .h file dependencies.
#define RIP_TLV_MAX_LENGTH (1 + 8 + 32 /* RIP_MAX_ROUTERS */)

//----------------------------------------------------------------

typedef struct {
  uint16_t version;
  MleCommand command;
  uint32_t keySequence;           // key sequence number used for decryption
  uint8_t *ipSource;
  bool isMulticast;

  // TLV data (grouped by size to minimize padding)
  uint8_t mode;
  uint8_t challengeLength;
  uint8_t responseLength;
  uint8_t routeDataLength;
  uint8_t scanMask;
  bool isLegacy;

  // about the source
  uint8_t senderLongId[8];
  EmberNodeId sourceAddress; // taken from the MLE_SOURCE_ADDRESS_TLV
  uint8_t neighborIndex;
  uint8_t childIndex;
  bool fromParent;
  bool haveLink;

  // assigned to us from the source, taken from MLE_ADDRESS_16_TLV
  EmberNodeId assignedShortAddress;

  uint32_t tlvMask;               // which TLVs are present
  uint32_t holdTimeMs;            // taken from MLE_HOLD_TIME_TLV
  uint32_t requestedTlvMask;      // which TLVs are requested from the destination
  uint32_t timeout;      // child timeout, from MLE_TIMEOUT_TLV
  uint8_t *challenge;    // points to the challenge in MLE_CHALLENGE_TLV
  uint8_t *response;     // points to the response in MLE_RESPONSE_TLV
  uint8_t routeData[RIP_TLV_MAX_LENGTH];
  // contents of the MLE_ROUTE64_TLV
  uint8_t *macFrameCounter; // points to the frame counter in MAC_FRAME_COUNTER_TLV
  uint8_t *mleFrameCounter; // points to the frame counter in MLE_FRAME_COUNTER_TLV
  uint8_t *globalPrefix; // points to the global prefix in MLE_GLOBAL_PREFIX_TLV
  uint8_t *leaderData;            // points to start of data
  uint8_t *networkData;           // points to length byte at start of data
  uint8_t *assignedIdMask;        // points to start of data
  // New style address registration
  uint8_t *childAddressTlv;       // points to length byte at start of data

  // active and pending timestamps
  uint8_t *activeTimestamp;  // points to the value in MLE_ACTIVE_TIMESTAMP_TLV
  uint8_t *pendingTimestamp; // points to the value in MLE_PENDING_TIMESTAMP_TLV
  uint8_t *activeDatasetTlv;    // points to the length byte of MLE_ACTIVE_OPERATIONAL_DATASET_TLV
  uint8_t *pendingDatasetTlv;   // points to the length byte of MLE_PENDING_OPERATIONAL_DATASET_TLV

  uint8_t *threadDiscoveryTlv; // points to the length byte of MLE_THREAD_DISCOVERY_TLV

  // Storage for making a new-style address registration out of old ones
  uint8_t childAddressTlvStorage[1 + (MAX_CHILD_ADDRESS_COUNT * (1 + 8))];
  uint8_t *connectivity;
  uint8_t linkMargin;
  uint8_t channel; // the channel in MLE_CHANNEL_TLV (byte 4)
  uint16_t panId; // the pan ID from MLE_PAN_ID_TLV

  EmberStatus status;
} MleMessage;

// Called by the MLE code at the start of processing an MLE message.
// Returns a value indicating whether the message should be processed
// or dropped, and if dropped, whether a request should be sent or not.
enum {
  MLE_PROCESS,
  MLE_SEND_REQUEST,
  MLE_DROP,
  MLE_SEND_REQUEST_AND_PROCESS
};

uint8_t emNoteMleSender(MleMessage *message);

// Called by the MLE code when adding a new non-router neighbor.
// Returns true if the joiner has been added.
bool emNoteMleJoiner(MleMessage *message, bool isLurker);
#define emNoteMleChild(message)          (emNoteMleJoiner((message), false))
#define emNoteMleLurkerNeighbor(message) (emNoteMleJoiner((message), true))

bool emMleTlvPresent(const MleMessage *message, uint8_t tlv);
bool emNoteMleAdvertisement(MleMessage *message, uint8_t neighborIndex);
void emProcessRipNeighborTlv(MleMessage *message);
bool emProcessRplAdvertisement(MleMessage *message);

// Implemented in join.c
void emProcessMleParentResponse(PacketHeader header, MleMessage *message);
bool emAddParentLink(MleMessage *message,
                     EmberNodeId parentId,
                     int32u mleFrameCounter);

// Implemented in network-fragmentation.c
void emFragmentRepairScanHandler(PacketHeader header, MleMessage *message);

//----------------------------------------------------------------

#define MLE_MODE_ROUTER         \
  (MODE_DEVICE_TYPE             \
   | MODE_RECEIVER_ON_WHEN_IDLE \
   | MODE_COMPLETE_NETWORK_DATA)

#define MLE_MODE_END_DEVICE     \
  (MODE_DEVICE_TYPE             \
   | MODE_RECEIVER_ON_WHEN_IDLE \
   | MODE_COMPLETE_NETWORK_DATA)

#define MLE_MODE_SLEEPY_END_DEVICE MODE_SECURE_DATA_REQUESTS

#define MLE_MODE_MINIMAL_END_DEVICE \
  (MODE_RECEIVER_ON_WHEN_IDLE       \
   | MODE_COMPLETE_NETWORK_DATA)

#define MLE_MODE_LURKER        CAPABILITY_ALLOCATE_ADDRESS
#define MLE_MODE_LURKER_LEGACY 0

// Default 802.15.4 hold time
#define MLE_HOLD_TIME_MS 7680

// Use this to check whether the message has a lurker mode.
bool emMleModeIsLurker(MleMessage *message);

// Thread nodes all attachs as if they were alway-on end devices.
#define ATTACH_CAPABILITIES MLE_MODE_END_DEVICE

void emAcceptParentRequest(bool yesno);
void emFilterParentRequest(const uint8_t *longId);

#ifdef EMBER_TEST
// HIDDEN items

extern const uint8_t capabilityBytes[];
void fillBlock0(uint8_t *block0,
                const uint8_t *eui64,
                const uint8_t *auxFrame);
extern uint16_t challenges16u[];
#define challenges ((uint8_t *) challenges16u)
void getChallenge(uint8_t *loc);
bool isOurChallenge(uint8_t *loc);

#endif

void emStartLurkerAdvertisements(void);
void emStopLurkerAdvertisements(void);

void emStartReedAdvertisements(void);
void emStopReedAdvertisements(void);

typedef enum {
  CHILD_KEEP_ALIVE,
  CHILD_UPDATE,
  PARENT_REBOOT
} ChildUpdateState;

bool emSendMleChildUpdate(const EmberIpv6Address *destination,
                          ChildUpdateState isKeepAlive);
bool emDisassociateChild(const EmberIpv6Address *destination);
bool emSendMleChildReboot(void);

uint8_t *emAddAddressRegistrationTlv(uint8_t *finger);

typedef enum {
  MLE_STATUS_ERROR                      = 1,
  MLE_STATUS_DUPLICATE_ADDRESS_DETECTED = 2
} MleStatus;

void emSendMleReject(uint8_t *ipDest, MleStatus status, bool useKeySource);
bool emSendMleDiscoveryRequest(const EmberEui64 *extendedPanIds,
                               uint8_t count,
                               bool joinerFlag);
void emProcessIncomingDiscoveryResponse(const uint8_t *mleTlv,
                                        const uint8_t *senderLongId,
                                        PacketHeader header);

// do we need stable only (false) or all (true) network data?
void emNeedAllNetworkData(bool yesno);
bool emGetCompleteNetworkData(void);

uint8_t *emAddConnectivityTlv(uint8_t *finger);
uint8_t *emAddRoute64Tlv(uint8_t *finger, const uint8_t *ipDest);
uint8_t *emAddLeaderTlv(uint8_t *finger);
uint8_t *emAddNetworkData(uint8_t *finger, bool stableDataOnly);
uint8_t *emAddActiveTimestampTlv(uint8_t *finger);

#endif // MLE_H
