// File: ip-packet-header.h
//
// Description: The in-core representation of IP packet headers.  The
// MAC translates outgoing packets from this to the over-the-air
// packet format and converts them back again on the way in.
//
// Header layout:
// Offset Size Description
//  0      1    mac info byte
//  1      3    queue storage
//  4      1    header tag
//  5      2    802.15.4 frame control
//
// Compressed MAC header:
//  7      ?    source or destination address
//
// Uncompressed MAC header:
//  7      1    sequence number
//  8      ?    MAC addressing fields
//
// We elide the MAC sequence number as well as the destination address
// and PAN ID on incoming packets and the source address and PAN ID on
// outgoing ones.  This saves a bit of RAM.
//
// Copyright 2009 by Ember Corporation. All rights reserved.                *80*

#ifndef __PACKET_HEADER_H__
#define __PACKET_HEADER_H__

#define MAC_INFO_INDEX       0
#define QUEUE_STORAGE_INDEX  1
#define HEADER_TAG_INDEX     4
#define MAC_HEADER_OFFSET    5

// These were needed when incoming messages were passed from the MAC
// to the network layer using a queue.  They are now passed directly,
// so the LQI, RSSI, and channel could be passed as arguments.
#define QUEUE_STORAGE_LQI_INDEX     QUEUE_STORAGE_INDEX
#define QUEUE_STORAGE_RSSI_INDEX    (QUEUE_STORAGE_INDEX + 1)
#define QUEUE_STORAGE_CHANNEL_INDEX (QUEUE_STORAGE_INDEX + 2)

// When in shortIndirectPool or longIndirectPool (in mac-indirect.c).
// Timeout is used when in emFragmentedMessages (in fragment.c)
#define QUEUE_STORAGE_TIMEOUT_INDEX   QUEUE_STORAGE_INDEX
#define QUEUE_STORAGE_TRIES_INDEX    (QUEUE_STORAGE_INDEX + 2)

#define MAC_ADDRESS_OFFSET (MAC_HEADER_OFFSET + 2)

//------------------------------------------------------------------------------
// Mac info field
//
// These are in-memory extensions to the MAC frame control.  If we run
// short of bits we can double up on incoming-only and outoing-only flags.

// Incoming and outgoing messages ------------------------------

// Set for incoming and outgoing packets that have been compressed.  If
// this is not set the packet contains a a full MAC header.
#define MAC_INFO_COMPRESSED                BIT(0)             // 0x01

// Set for incoming and outgoing compressed packets that have a long
// address.
#define MAC_INFO_LONG_ADDRESS              BIT(1)             // 0x02

// Set for incoming and outgoing uncompressed packets that are passed
// directly between the application and the MAC.
#define MAC_INFO_PASSTHROUGH               BIT(2)             // 0x04

// Packets that either came in with a mesh header or will be sent out
// with one.  The MAC address is the mesh source/destination rather
// than an actual one-hop neighbor.
#define MAC_INFO_MESH_ROUTED               BIT(3)             // 0x08

// Set for incoming and outgoing packets for which the legacy key
// should be used for encryption and decryption.
#define MAC_INFO_LEGACY_KEY                BIT(4)             // 0x10

// Incoming Messages -------------------------------------------

// For incoming packets this is a copy of the frame pending bit on the
// ACK that was sent in response.
#define MAC_INFO_OUTGOING_FRAME_PENDING    BIT(5)             // 0x20

// Marks incoming data packets that were sent to the broadcast node ID.
#define MAC_INFO_BROADCAST_NODE_ID         BIT(6)             // 0x40

// Outgoing Messages -------------------------------------------

// Indicates the packet is being relayed.  The previous sender id is
// left as the mac destination so that the code that fills in the next
// hop can check that it is not being sent back to the sender (2-hop loop).
#define MAC_INFO_RELAYING_PACKET           BIT(5)             // 0x20

// If set on an outgoing unicast to a sleepy child, this causes the
// message to be sent directly instead of waiting for a poll.
#define MAC_INFO_POLL_RESPONSE_MASK        BIT(6)             // 0x40

// Outgoing packets with this set were created by the stack itself and
// are not passed to emberMessageSentHandler().
#define MAC_INFO_STACK_PRIVATE_MASK        BIT(7)             // 0x80

// If more flags are needed, the MAC info field can be combined with the
// header tag into a 16-bit field with the header tag fitting into four bits.

// -------------------------------------------------------------

uint8_t emHeaderMacInfoField(PacketHeader header);
void emHeaderSetMacInfoField(PacketHeader header, uint8_t info);

void emHeaderSetStackPrivate(PacketHeader header);

#define emHeaderIsStackPrivate(header) \
  (emHeaderMacInfoField(header) & MAC_INFO_STACK_PRIVATE_MASK)

#define emHeaderIsPassThrough(header)               \
  ((emHeaderMacInfoField(header)                    \
    & (MAC_INFO_COMPRESSED | MAC_INFO_PASSTHROUGH)) \
   == MAC_INFO_PASSTHROUGH)

#define emHeaderWasBroadcast(header) \
  (emHeaderMacInfoField(header) & MAC_INFO_BROADCAST_NODE_ID)

//------------------------------------------------------------------------------
// Header Tag field

// This field is used internally to mark particular types of messages
// that need special processing just before or after transmission,
// in emPrepareRetryEntryForSubmission(), emRetryTransmitCompleteCallback(),
// and so forth.

enum {
  HEADER_TAG_NONE = 0,  // The header tag byte is initialized to 0.
  HEADER_TAG_MLE_JITTER,
  HEADER_TAG_WAKEUP,
  HEADER_TAG_COMMISSIONING_KEY,
  HEADER_TAG_COMMISSION_DATASET_ANNOUNCE,
  HEADER_TAG_MLE_DISCOVERY_REQUEST,
  HEADER_TAG_MLE_DISCOVERY_RESPONSE,
  HEADER_TAG_RSSI_PEEK
};

uint8_t emHeaderTag(PacketHeader header);
void emHeaderSetTag(PacketHeader header, uint8_t tag);
void emHeaderClearTag(PacketHeader header);

//------------------------------------------------------------------------------
// Utilities for reading and writing from headers (these actually work
// on any buffer).  The values must be in the first 32 bytes.

#define emHeaderGetInt8u emberGetLinkedBuffersByte
#define emHeaderSetInt8u emberSetLinkedBuffersByte
#define emHeaderGetInt16u emberGetLinkedBuffersLowHighInt16u
#define emHeaderSetInt16u emberSetLinkedBuffersLowHighInt16u

// Returns the index of first post-MAC-header byte.
uint8_t emMacPayloadIndex(PacketHeader header);

// Returns a pointer to the first byte of the the NWK frame (for data
// packets) or the 15.4 frame control (for all other packets).
uint8_t *emMacPayloadPointer(PacketHeader header);

//------------------------------------------------------------------------------
// MAC storage

// Get and set the MAC source for incoming data packets and the MAC destination
// for outgoing data packets.
//EmberNodeId emGetMacDataAddress(PacketHeader header);
//void emSetMacDataAddress(PacketHeader header, EmberNodeId id);

#define emGetMacFrameControl(header) \
  (emHeaderGetInt16u((header), MAC_HEADER_OFFSET))
#define emSetMacFrameControl(header, frameControl) \
  (emHeaderSetInt16u((header), MAC_HEADER_OFFSET, (frameControl)))

// MAC_FRAME_TYPE_MASK 0x0007 should be defined here not in phy.h.
#define emMacFrameType(header) \
  (emGetMacFrameControl((header)) & 0x0007)

// MAC_FRAME_TYPE_DATA 0x0001 should be defined here not in phy.h
#define emMacFrameTypeIsData(header) \
  (emMacFrameType((header)) == 0x0001)

// MAC_FRAME_TYPE_CONTROL 0x0003 should be defined here not in phy.h
#define emMacFrameTypeIsControl(header) \
  (emMacFrameType((header)) == 0x0003)

// Returns the length of the MAC header given a pointer to the start
// of the frame.
uint8_t emMacHeaderLength(uint8_t *macFrame);

// Packets that are to be sent via the UART instead of the radio get this
// as a short destination ID.  This is not a valid 802.15.4 short ID.
#define UART_NEXT_HOP_NODE_ID 0xFFFE

// Access to the addresses of all headers, including beacons, MAC commands,
// and passthrough.  These can be applied to data packets as well, but the
// 'source' and 'destination' will both be the one address in the header.
// It is up to the caller to know if the packet is incoming or outgoing.
uint16_t emMacDestinationMode(PacketHeader header);
uint8_t *emMacDestinationPointer(PacketHeader header);
EmberNodeId emMacShortDestination(PacketHeader header);
void emSetMacShortDestination(PacketHeader header, EmberNodeId id);
void emSetMacLongDestination(PacketHeader header, const uint8_t *longId);
uint16_t emMacSourcePanId(PacketHeader header);
// Reflects an incoming packet back out again.  Looks up long from short via
// neighbor table if necessary.
bool emSetMacForwardingDestination(PacketHeader header, uint16_t shortId);
bool emSetMacForwardingLongDestination(PacketHeader header,
                                       const uint8_t *longId);
bool emHeaderIsBroadcast(PacketHeader header);

uint16_t emMacSourceMode(PacketHeader header);
uint8_t *emMacSourcePointer(PacketHeader header);
EmberNodeId emMacShortSource(PacketHeader header);
EmberNodeId emGetMeshSource(PacketHeader header);

//------------------------------------------------------------------------------
// Three dead functions that should be removed.

#define emHoldPacketHeader(header) emberHoldMessageBuffer(header)
#define emReleasePacketHeader(header) emberReleaseMessageBuffer(header)
#define emFreePacketHeaders()   do {} while (0)

//------------------------------------------------------------------------------
// Creating Packet Headers

PacketHeader emMakeDataHeader(bool isBroadcast,
                              bool longSource,
                              bool longDest,
                              uint8_t *payload,           // may be NULL
                              uint16_t payloadLength);

PacketHeader emMakeRawPacketHeader(uint8_t *contents, uint8_t length);

#ifdef EMBER_TEST
// For building test headers.

PacketHeader makeDataHeader(EmberNodeId address,
                            bool longSource,
                            bool longDest,
                            uint8_t *payload,
                            uint16_t payloadLength);

PacketHeader makeMacHeader(uint16_t macFrameControl,
                           uint8_t *destination,
                           uint16_t destinationPanId,
                           uint8_t *source,
                           uint16_t sourcePanId,
                           uint8_t *frame,
                           uint8_t frameLength);

// #define emMakeRawHeaderParcel(macInfoFlags, frameControl)
// (makeMessage("<221<2", (macInfoFlags), 0, 0, (frameControl)))

#define emMakeDataHeaderParcel(sourceMode, address)     \
  (makeMessage("11111<2<2",                             \
               (MAC_INFO_COMPRESSED                     \
                | MAC_INFO_STACK_PRIVATE_MASK),         \
               0, 0, 0, 0,                              \
               (MAC_FRAME_TYPE_DATA                     \
                | sourceMode                            \
                | MAC_FRAME_DESTINATION_MODE_SHORT      \
                | ((address == BROADCAST_ADDRESS)       \
                   ? 0 : MAC_FRAME_FLAG_ACK_REQUIRED)), \
               (address)))

// #define emMakeBeaconHeaderParcel(shortId, panId, superframe)
// (makeMessage("<221<21<2<2<211",
//             MAC_INFO_TYPE_BEACON,
//             0,          /* queue data */
//             0,          /* queue data */
//             MAC_BEACON_FRAME_CONTROL,
//             0,          /* sequence number */
//             (panId),
//             (shortId),
//             ((superframe) | ZIGBEE_BEACON_SUPERFRAME),
//             0,         /* no GTS */
//             0))        /* no pending addresses */

// This needs to be called makeMacFrameParcel() or something.

Parcel *makeMacFrame(uint16_t frameControl,
                     uint8_t packetNumber,
                     void *sourceId,
                     uint16_t sourcePanId,
                     void *destinationId,
                     uint16_t destinationPanId);

#endif // EMBER_TEST

#endif // __PACKET_HEADER_H__
