/*
 * File: join.h
 * Description: Declarations used in the join process.
 *
 * Copyright 2011 by Ember Corporation. All rights reserved.                *80*
 */

/*

   Joining as a Router
   -------------------

   Key:
   -> means sent by the joiner
   <- means received by the joiner

   1. 15.4 Active Scan
   -------------------

   -> 15.4 Beacon Request
   <- 15.4 Beacon(s)

   Choose a parent based on beacon info:
   - rip protocol id
   - extended pan id (optional)
   - best rssi
   - allowing join
   - child capacity

   2. DTLS
   -------

   Perform DTLS via parent to obtain master key.
   Derive MAC and MLE key from the master key.

   If DTLS completes successfully, the device is joined to the network but
   does not yet have a parent.  Before reporting the join to the application,
   we establish a parent.

   3. MLE Parent link establishment
   --------------------------------

   In this step we establish a link with a parent,
   obtain a child id, and obtain leader and network data.
   All messages are sent with MLE security.

   -> MLE Request with TLVs:
   - tlv request: short address, global prefix
   - challenge
   <- MLE Accept & Request with TLVs:
   - source
   - short address
   - replay counter
   - response
   - challenge
   - network data
   - global prefix
   - tlv request: rip tlv
   -> MLE Accept with TLVs:
   - source
   - rip tlv
   - replay counter
   - response

   For security, the parent should not add the child to its table based on the
   first message as currently done above.  Instead it should only use the first
   message as a way of initiating a 3-way handshake. The table only changes upon
   receipt of a message from the child that contains a response to a challenge.

   The device now has a parent and a child id, and we report a successful join
   to the application.  If this step was not successful, we currently
   report a complete join failure, but this should be changed to take into
   account the fact that DTLS succeeded and we just need to find a parent.

   Obtaining a router id is separate from the join process.

   Joining as a non-router
   -----------------------

   Joining as an end device is identical to joining as a router, except
   that an end device does not need to request a rip id mask from the parent.

   Joining as a sleepy end device requires requires the following unsecured
   exchange prior to the DTLS step:

   -> MLE Join Request
   - mode
   <- MLE Join Response

   This informs the parent that the joining node will be using indirect
   messaging during the join process.  The joiner continues to include the
   mode TLV in its MLE messages so that the parent can store this information
   in its child table.

   Joining as a lurker is similar to joining as an end device, except that
   the mode is set to indicate the joiner is a lurker, and no short id is
   requested.

   MLE TLV Rules
   -------------

   Source TLV is always included if you have a short id.

   A challenge TLV implicitly requests a response TLV, a replay counter TLV,
   and a leader data TLV.

   A leader data TLV must be sent with a short id assignment.

 */

#ifndef __JOIN_H__
#define __JOIN_H__

// Values for emJoinState.
typedef enum {
  JOIN_IDLE,
  JOIN_SCANNING,
  JOIN_SECURITY,
  JOIN_APP_COMMISSION,  // app-specific commissioning
  JOIN_ENTRUST,         // get entrust message from join router
  JOIN_PAUSE,           // wait for DTLS session closing
  JOIN_LURKER,          // 1.1 compatibility - lurkers get the ULA prefix now
  JOIN_COMPLETE,
  // restart the join process to try again
  JOIN_RESTART,
  JOIN_COMMISSION_FAILED,
  LAST_JOIN_STATE
} JoinState;

typedef enum {
  // Attach states
  ATTACH_IDLE,
  ATTACH_SCANNING_ROUTERS,
  ATTACH_SCANNING_ALL,
  ATTACH_SEND_CHILD_ID_REQUEST,
  ATTACH_RESTART,
  ATTACH_COMPLETE,

  // for while a router is waiting to reclaim its ID after rebooting
  ATTACH_ROUTER_REBOOT,
  // for while an end device is waiting to re-establish connection with parent
  ATTACH_END_DEVICE_REBOOT,
  ATTACH_ORPHAN_SCAN,
  LAST_ATTACH_STATE
} AttachState;

extern uint8_t emJoinState;
extern uint8_t emAttachState;

extern bool emUseMleDiscovery;
extern bool emRandomizeMacExtendedId;
extern bool emEnableMacEncryption;
extern uint16_t emForceParentId;
extern uint8_t emForceParentLongId[8];
extern uint32_t emChannelMaskCache;
extern uint8_t emParentCostToLeader;

void emJoinCallback(bool success);

// Thread beacon payload as defined by
// draft-turon-mesh-commissioning-04.txt
//
//     0:   Protocol ID.      Thread = 3
//     1:   Control Field.    Bit 0:   Allow Join
//                            Bit 1-2: Reserved
//                            Bit 3:   Allow Native Commissioner
//                            Bit 4-7: Protocol version
//                                     Thread = 1
//                                     Legacy = 0
//
//  2-17:   Network ID.      16 ASCII characters.
// 18-25:   Extended PAN ID.  8 bytes.
// 26-42:   Optional steering data TLV (max. 17 bytes)
// 42-43:   Optional joiner port TLV (4 bytes)
// 44-45:   Optional commissioner port TLV (4 bytes)

#define THREAD_MINIMUM_BEACON_PAYLOAD_SIZE \
  (1 + 1                                   \
   + EMBER_NETWORK_ID_SIZE                 \
   + EXTENDED_PAN_ID_SIZE)

#define THREAD_MAXIMUM_STEERING_DATA_SIZE 17

#define THREAD_MAXIMUM_BEACON_PAYLOAD_SIZE \
  (THREAD_MINIMUM_BEACON_PAYLOAD_SIZE      \
   + THREAD_MAXIMUM_STEERING_DATA_SIZE     \
   + 4 + 4)

#define THREAD_PROTOCOL_ID             3

#define THREAD_BEACON_ALLOW_JOIN_BIT              BIT(0)
#define THREAD_BEACON_ALLOW_NATIVE_COMMISSION_BIT BIT(3)

#define THREAD_BEACON_PROTOCOL_VERSION_MASK 0xF0
#define THREAD_BEACON_PROTOCOL_VERSION_SHIFT 4
#define THREAD_1_1_PROTOCOL_VERSION 2
#define THREAD_1_0_PROTOCOL_VERSION 1
#define LEGACY_PROTOCOL_VERSION 0

#define emNetworkId (emBeaconPayloadBuffer + 2)
#define EXTENDED_PAN_ID_INDEX (1 + 1 + EMBER_NETWORK_ID_SIZE)
#define emExtendedPanId (emBeaconPayloadBuffer + EXTENDED_PAN_ID_INDEX)
#define BEACON_STEERING_DATA_INDEX \
  (EXTENDED_PAN_ID_INDEX + EXTENDED_PAN_ID_SIZE)

#ifdef EMBER_WAKEUP_STACK
  #define emSetThreadPermitJoin(val)
  #define emSetLegacyPermitJoin(val) emSetPermitJoin((val))
#else
  #define emSetThreadPermitJoin(val) emSetPermitJoin((val))
  #define emSetLegacyPermitJoin(val)
#endif

// MLE discovery request timeout
#define ATTACH_DISCOVERY_TIMEOUT_MS 300
// Timeout for scan requests (Routers)
#define ATTACH_SCAN_TIMEOUT_ROUTERS_MS 750
// Timeout for scan requests (Routers + REED)
#define ATTACH_SCAN_TIMEOUT_ALL_MS 1250
// Used by sleepy end devices to poll during attaching.
#define ATTACH_POLL_DELAY_MS 150
// Time to wait before declaring failed resumption.  This is just a guess.
#define JOIN_REBOOT_TIMER_DELAY_MS 1500
// Lurker join timeout
#define LURKER_TIMEOUT_MS 2000

void emSetPermitJoin(bool value);
bool emGetThreadJoin(void);
void emSetThreadNativeCommission(bool on);
bool emGetThreadNativeCommission(void);
bool emAcceptUnsecuredFragment(PacketHeader firstFragment);

void emSetExtendedPanId(const uint8_t* extendedPanId);

typedef void (*StartHostJoinClient)(uint8_t *address);
void emSetStartHostJoinClient(StartHostJoinClient handler);

void emStartBeacons(void);
void emSetNetworkId(const uint8_t *networkId, uint8_t length);
void emSetJoinComplete(void);
void emJoinInit(void);
void emStartAttach(AttachReason reason);
void emMaybeAttach(AttachReason reason);
void emOrphanStartAttach(void);
bool emGetParentIpv6Address(EmberIpv6Address *address);
void emStartRouterRebootTimer(void);
void emRouterRebootSuccess(void);
void emStartEndDeviceRebootTimer(void);
void emEndDeviceRebootSuccess(void);
void emEndDeviceRebootFailed(void);
void emJoinSecurityFailed(void);
bool emJoinIsFrameCounterValid(uint32_t macFrameCounter,
                               const uint8_t *longId,
                               uint32_t keySequence,
                               MessageKeyType keyType);
const uint8_t *emAttachParentLongId(void);
// in-band commissioning
void emSetCommissioningMacKey(uint8_t *key);
uint8_t *emGetCommissioningMacKey(const uint8_t *senderEui64);
void emProcessCommissioningMessage(PacketHeader header);
void emCommissioningHandshakeComplete(void);
void emCancelCommissioningKey(void);
void emJoinCommissionCompleteHandler(bool success);
void emAppCommissionCompleteHandler(bool success);
void emProcessCommissioningData(PacketHeader header, Ipv6Header *ipHeader);
void emProcessIncomingJoinBeacon(MacBeaconData *macBeaconData);
void emSetBeaconSteeringData(const uint8_t *steeringData, uint8_t steeringDataLength);

void emComputeEui64Hash(const EmberEui64 *input, EmberEui64 *output);
void emCacheParameters(const EmberNetworkParameters *parameters,
                       uint16_t options);
void emSetupNextJoinState(void);
void emSetOldChannelForAttach(void);
#endif
