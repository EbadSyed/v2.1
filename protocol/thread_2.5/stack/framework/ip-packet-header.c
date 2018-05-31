// File: ip-packet-header.c
//
// Description: The in-core representation of IP packet headers.
//
// Copyright 2009 by Ember Corporation. All rights reserved.                *80*

#include "core/ember-stack.h"
#include "mac/mac-header.h"
#include "ip-packet-header.h"
#include "mac/802.15.4/802-15-4-ccm.h"

#ifdef EMBER_TEST
#include "core/parcel.h"
#endif

//------------------------------------------------------------------------------

uint8_t emMacPayloadIndex(PacketHeader header)
{
  uint8_t *contents = emberMessageBufferContents(header);
  if (contents != NULL) {
    if (!(contents[MAC_INFO_INDEX] & MAC_INFO_COMPRESSED)) {
      return (MAC_HEADER_OFFSET
              + emMacHeaderLength(contents + MAC_HEADER_OFFSET));
    } else if (contents[MAC_INFO_INDEX] & MAC_INFO_LONG_ADDRESS) {
      return MAC_HEADER_OFFSET + 2 + 8;    // frame control and long address
    } else {
      return MAC_HEADER_OFFSET + 2 + 2;    // frame control and short address
    }
  }
  return MAC_HEADER_OFFSET + 2 + 2;    // frame control and short address
}

uint8_t *emMacPayloadPointer(PacketHeader header)
{
  return emberMessageBufferContents(header)
         + emMacPayloadIndex(header);
}

//------------------------------------------------------------------------------
// Mac info field

uint8_t emHeaderMacInfoField(PacketHeader header)
{
  return emHeaderGetInt8u(header, MAC_INFO_INDEX);
}

void emHeaderSetMacInfoField(PacketHeader header, uint8_t info)
{
  emHeaderSetInt8u(header, MAC_INFO_INDEX, info);
}

void emHeaderSetStackPrivate(PacketHeader header)
{
  uint8_t *contents = emberLinkedBufferContents(header);
  if (contents != NULL) {
    contents[MAC_INFO_INDEX] |= MAC_INFO_STACK_PRIVATE_MASK;
  }
}

//------------------------------------------------------------------------------
// Header Tag field

uint8_t emHeaderTag(PacketHeader header)
{
  return emHeaderGetInt8u(header, HEADER_TAG_INDEX);
}

void emHeaderSetTag(PacketHeader header, uint8_t tag)
{
  emHeaderSetInt8u(header, HEADER_TAG_INDEX, tag);
}

void emHeaderClearTag(PacketHeader header)
{
  emHeaderSetInt8u(header, HEADER_TAG_INDEX, HEADER_TAG_NONE);
}

//------------------------------------------------------------------------------

uint16_t emMacDestinationMode(PacketHeader header)
{
  return emGetMacFrameControl(header) & MAC_FRAME_DESTINATION_MODE_MASK;
}

uint8_t *emMacDestinationPointer(PacketHeader header)
{
  uint8_t *contents = emGetBufferPointer(header);
  if (emHeaderMacInfoField(header) & MAC_INFO_COMPRESSED) {
    return contents + MAC_ADDRESS_OFFSET;
  } else {
    return contents + MAC_ADDRESS_OFFSET + 1 + 2; // skip sequence and PAN ID
  }
}

EmberNodeId emMacShortDestination(PacketHeader header)
{
  return emberFetchLowHighInt16u(emMacDestinationPointer(header));
}

void emSetMacShortDestination(PacketHeader header, EmberNodeId id)
{
  emberStoreLowHighInt16u(emMacDestinationPointer(header), id);
  uint16_t frameControl = emGetMacFrameControl(header);

  if (id == BROADCAST_ADDRESS) {
    emSetMacFrameControl(header, frameControl & ~MAC_FRAME_FLAG_ACK_REQUIRED);
  } else {
    emSetMacFrameControl(header, frameControl | MAC_FRAME_FLAG_ACK_REQUIRED);
  }
}

void emSetMacLongDestination(PacketHeader header, const uint8_t *longId)
{
  MEMMOVE(emMacDestinationPointer(header), longId, 8);
}

static void setDestinationMode(PacketHeader header, uint16_t mode)
{
  if (emMacDestinationMode(header) != mode) {
    emSetMacFrameControl(header,
                         ((emGetMacFrameControl(header)
                           & ~MAC_FRAME_DESTINATION_MODE_MASK)
                          | mode));
  }
}

// Forward an incoming packet back out again.
//
// TODO: for the Cambridge stack interop on 2/22/2012, modified this
// function to make sure the frame control dest mode matches the mac
// info byte, to handle the case of relaying a packet that came in
// with mixed source/dest address modes.  We need to handle this at
// a more fundamental level.

bool emSetMacForwardingDestination(PacketHeader header, uint16_t shortId)
{
  uint16_t infoField = emHeaderMacInfoField(header);
  assert(infoField & MAC_INFO_COMPRESSED);

  if (infoField & MAC_INFO_LONG_ADDRESS) {
    // Ugh, we need to change the size of the MAC header.
    uint8_t *contents = emGetBufferPointer(header);
    uint16_t length = emGetBufferLength(header);
    MEMMOVE(contents + MAC_ADDRESS_OFFSET + 2,
            contents + MAC_ADDRESS_OFFSET + 8,
            length - (MAC_ADDRESS_OFFSET + 8));
    emSetBufferLength(header, length - 6);
    emHeaderSetMacInfoField(header, infoField & ~MAC_INFO_LONG_ADDRESS);
  }
  setDestinationMode(header, MAC_FRAME_DESTINATION_MODE_SHORT);

  if (shortId == 0xFFFE) {
    emHeaderSetMacInfoField(header,
                            (emHeaderMacInfoField(header)
                             | MAC_INFO_RELAYING_PACKET));
  } else {
    emHeaderSetMacInfoField(header,
                            (emHeaderMacInfoField(header)
                             & ~MAC_INFO_RELAYING_PACKET));
    emSetMacShortDestination(header, shortId);
  }
  return true;
}

bool emSetMacForwardingLongDestination(PacketHeader header,
                                       const uint8_t *longId)
{
  uint16_t infoField = emHeaderMacInfoField(header);
  assert(infoField & MAC_INFO_COMPRESSED);

  if (infoField & MAC_INFO_LONG_ADDRESS) {
    setDestinationMode(header, MAC_FRAME_DESTINATION_MODE_LONG);
    emSetMacLongDestination(header, longId);
    return true;
  } else {
    return false;
  }
}

bool emHeaderIsBroadcast(PacketHeader header)
{
  uint16_t mode = emMacDestinationMode(header);
  // No destination counts as broadcast (eg, beacons).
  return (mode == MAC_FRAME_DESTINATION_MODE_NONE
          || (mode == MAC_FRAME_DESTINATION_MODE_SHORT
              && emMacShortDestination(header) == BROADCAST_ADDRESS));
}

uint16_t emMacSourceMode(PacketHeader header)
{
  return emGetMacFrameControl(header) & MAC_FRAME_SOURCE_MODE_MASK;
}

uint8_t *emMacSourcePointer(PacketHeader header)
{
  uint8_t *contents = emGetBufferPointer(header);
  if (emHeaderMacInfoField(header) & MAC_INFO_COMPRESSED) {
    return contents + MAC_ADDRESS_OFFSET;
  } else {
    return (emMacPayloadPointer(header)
            - (emMacSourceMode(header) == MAC_FRAME_SOURCE_MODE_LONG
               ? 8
               : 2));
  }
}

EmberNodeId emMacShortSource(PacketHeader header)
{
  return emberFetchLowHighInt16u(emMacSourcePointer(header));
}

uint16_t emMacSourcePanId(PacketHeader header)
{
  if (emHeaderMacInfoField(header) & MAC_INFO_COMPRESSED) {
    return emApiGetPanId();
  }

  uint16_t frameControl = emGetMacFrameControl(header);
  // skip over frame control and counter
  uint8_t *loc = emGetBufferPointer(header) + MAC_HEADER_OFFSET + 3;
  if (frameControl & MAC_FRAME_FLAG_INTRA_PAN) {
    // do nothing
  } else if ((frameControl &  MAC_FRAME_SOURCE_MODE_MASK)
             == MAC_FRAME_SOURCE_MODE_NONE) {
    return 0xFFFF;          // have to return something
  } else if ((frameControl &  MAC_FRAME_DESTINATION_MODE_MASK)
             == MAC_FRAME_DESTINATION_MODE_SHORT) {
    loc += 4;   // PAN ID + short destination
  } else if ((frameControl &  MAC_FRAME_DESTINATION_MODE_MASK)
             == MAC_FRAME_DESTINATION_MODE_LONG) {
    loc += 10;  // PAN ID + long destination
  }
  return emberFetchLowHighInt16u(loc);
}

EmberNodeId emGetMeshSource(PacketHeader header)
{
  if (emHeaderMacInfoField(header) & MAC_INFO_MESH_ROUTED) {
    return emMacShortSource(header);
  } else {
    return 0xFFFE;
  }
}

// The caller is responsible for filling in the destination
// address.  There is a helpful function emSetShortOrLongDestination()
// that can be used for this purpose.

PacketHeader emMakeDataHeader(bool isBroadcast,
                              bool longSource,
                              bool longDest,
                              uint8_t *payload,
                              uint16_t payloadLength)
{
  uint8_t infoField = MAC_INFO_COMPRESSED;
  uint16_t frameControl = MAC_FRAME_TYPE_DATA;
  PacketHeader header;
  uint8_t *contents;

  if (isBroadcast) {
    frameControl |= MAC_FRAME_DESTINATION_MODE_SHORT;
  } else {
    frameControl |= MAC_FRAME_FLAG_ACK_REQUIRED;
    if (longDest) {
      infoField |= MAC_INFO_LONG_ADDRESS;
      frameControl |= MAC_FRAME_DESTINATION_MODE_LONG;
    } else {
      frameControl |= MAC_FRAME_DESTINATION_MODE_SHORT;
    }
  }
  frameControl |= (longSource
                   ? MAC_FRAME_SOURCE_MODE_LONG
                   : MAC_FRAME_SOURCE_MODE_SHORT);
  if (emGetNetworkKeySequence(NULL)) {
    frameControl |= (MAC_FRAME_FLAG_SECURITY_ENABLED
                     | MAC_FRAME_VERSION_2006);
  }
  header = emAllocateBuffer(MAC_ADDRESS_OFFSET
                            + ((infoField & MAC_INFO_LONG_ADDRESS) ? 8 : 2)
                            + payloadLength);
  if (header == NULL_BUFFER) {
    return NULL_BUFFER;
  }
  // Zero it out for safety.  We do not want to transmit anything
  // unintentionally.
  MEMSET(emGetBufferPointer(header), 0, emGetBufferLength(header));
  emHeaderSetMacInfoField(header, infoField);
  contents = emGetBufferPointer(header) + MAC_HEADER_OFFSET;
  emberStoreLowHighInt16u(contents, frameControl);
  contents += 2;
  contents += (infoField & MAC_INFO_LONG_ADDRESS ? 8 : 2);
  if (payload != NULL) {
    MEMCOPY(contents, payload, payloadLength);
  }
  return header;
}

PacketHeader emMakeRawPacketHeader(uint8_t *contents, uint8_t length)
{
  PacketHeader header;

  header = emAllocateBuffer(MAC_HEADER_OFFSET + length);
  if (header == NULL_BUFFER) {
    return NULL_BUFFER;
  }
  // Zero it out for safety.  We do not want to transmit anything
  // unintentionally.
  MEMSET(emGetBufferPointer(header), 0, emGetBufferLength(header));
  emHeaderSetMacInfoField(header, MAC_INFO_PASSTHROUGH);
  if (contents != NULL) {
    MEMCOPY(emGetBufferPointer(header) + MAC_HEADER_OFFSET, contents, length);
  }
  return header;
}

//------------------------------------------------------------------------------
#ifdef EMBER_TEST

// Creating packet headers for testing.

PacketHeader makeDataHeader(EmberNodeId address,
                            bool longSource,
                            bool longDest,
                            uint8_t *payload,
                            uint16_t payloadLength)
{
  PacketHeader header = emMakeDataHeader(address == BROADCAST_ADDRESS,
                                         longSource,
                                         longDest,
                                         payload,
                                         payloadLength);
  uint16_t frameControl;
  if (header == NULL_BUFFER) {
    return NULL_BUFFER;
  }
  frameControl = emGetMacFrameControl(header);
  frameControl |= (MAC_FRAME_FLAG_INTRA_PAN
                   | MAC_FRAME_DESTINATION_MODE_SHORT
                   | MAC_FRAME_SOURCE_MODE_SHORT);
  emHeaderSetInt16u(header, MAC_HEADER_OFFSET, frameControl);
  emSetMacShortDestination(header, address);
  return header;
}

PacketHeader makeMacHeader(uint16_t macFrameControl,
                           uint8_t *destination,
                           uint16_t destinationPanId,
                           uint8_t *source,
                           uint16_t sourcePanId,
                           uint8_t *frame,
                           uint8_t frameLength)
{
  uint16_t srcAddressMode = macFrameControl & MAC_FRAME_SOURCE_MODE_MASK;
  uint16_t destAddressMode = macFrameControl & MAC_FRAME_DESTINATION_MODE_MASK;
  PacketHeader header;
  uint8_t contents[2 + 1 + 10 + 10];
  uint8_t *finger = contents;
  uint8_t macLength;

  *finger++ = LOW_BYTE(macFrameControl);
  *finger++ = HIGH_BYTE(macFrameControl);
  finger++;     // sequence number, set when transmitted

  if (destAddressMode != MAC_FRAME_DESTINATION_MODE_NONE) {
    *finger++ = LOW_BYTE(destinationPanId);
    *finger++ = HIGH_BYTE(destinationPanId);
  }

  switch (destAddressMode) {
    case MAC_FRAME_DESTINATION_MODE_SHORT: {
      *finger++ = LOW_BYTE((uintptr_t) destination);
      *finger++ = HIGH_BYTE((uintptr_t) destination);
      break;
    }
    case MAC_FRAME_DESTINATION_MODE_LONG: {
      MEMCOPY(finger, destination, 8);
      finger += 8;
      break;
    }
  }

  if (srcAddressMode != MAC_FRAME_SOURCE_MODE_NONE
      && !(macFrameControl & MAC_FRAME_FLAG_INTRA_PAN)) {
    uint16_t panId = sourcePanId;
    *finger++ = LOW_BYTE(panId);
    *finger++ = HIGH_BYTE(panId);
  }

  switch (srcAddressMode) {
    case MAC_FRAME_SOURCE_MODE_SHORT: {
      *finger++ = LOW_BYTE((uintptr_t) source);
      *finger++ = HIGH_BYTE((uintptr_t) source);
      break;
    }
    case MAC_FRAME_SOURCE_MODE_LONG: {
      MEMCOPY(finger, source, 8);
      finger += 8;
      break;
    }
  }

  macLength = finger - contents;
  header = emAllocateBuffer(MAC_HEADER_OFFSET + macLength + frameLength);

  if (header == NULL_BUFFER) {
    return NULL_BUFFER;
  }

  emHeaderSetMacInfoField(header, 0);
  finger = emGetBufferPointer(header) + MAC_HEADER_OFFSET;
  MEMCOPY(finger, contents, macLength);
  finger += macLength;
  MEMCOPY(finger, frame, frameLength);

  return header;
}

//----------------------------------------------------------------
// Encode a MAC address as a parcel, dispatching on the address mode.
// This works for both source and destination addresses.  For ease of use
// a short address of NULL is encoded as 0xFFFF.

static Parcel *makeMacAddress(uint16_t mode, void *address)
{
  switch (mode) {
    case MAC_FRAME_DESTINATION_MODE_NONE:
//  case MAC_FRAME_SOURCE_MODE_NONE:  // Both _MODE_NONE values are zero.
      return makeMessage("");
    case MAC_FRAME_DESTINATION_MODE_SHORT:
    case MAC_FRAME_SOURCE_MODE_SHORT:
      return makeMessage("<2",
                         (address == NULL
                          ? 0xFFFF
                          : *((uint16_t *) address)));
    case MAC_FRAME_DESTINATION_MODE_LONG:
    case MAC_FRAME_SOURCE_MODE_LONG:
      return makeMessage("s", (uint8_t *) address, 8);
    default:
      assert(!"bad MAC address mode");
      return NULL;
  }
}

Parcel *makeMacFrame(uint16_t frameControl,
                     uint8_t packetNumber,
                     void *sourceId,
                     uint16_t sourcePanId,
                     void *destinationId,
                     uint16_t destinationPanId)
{
  uint16_t destMode = frameControl & MAC_FRAME_DESTINATION_MODE_MASK;
  if ((frameControl & MAC_FRAME_TYPE_MASK) == MAC_FRAME_TYPE_ACK) {
    return makeMessage("<21",
                       frameControl,
                       packetNumber);
  } else if ((frameControl & MAC_FRAME_SOURCE_MODE_MASK)
             == MAC_FRAME_SOURCE_MODE_NONE) {
    return makeMessage("<21<2p",
                       frameControl,
                       packetNumber,
                       destinationPanId,
                       makeMacAddress(destMode, destinationId));
  } else {
    bool intraPan = (((sourcePanId == destinationPanId)
                      && (sourcePanId != 0xffff)
                      && (destMode != MAC_FRAME_DESTINATION_MODE_NONE))
                     || frameControl & MAC_FRAME_FLAG_INTRA_PAN);

    return makeMessage("<21pppp",
                       frameControl | (intraPan ? MAC_FRAME_FLAG_INTRA_PAN : 0),
                       packetNumber,
                       (destMode == MAC_FRAME_DESTINATION_MODE_NONE
                        ? makeMessage("")
                        : makeMessage("<2", destinationPanId)),
                       makeMacAddress(destMode, destinationId),
                       (intraPan
                        ? makeMessage("")
                        : makeMessage("<2", sourcePanId)),
                       makeMacAddress((frameControl
                                       & MAC_FRAME_SOURCE_MODE_MASK),
                                      sourceId));
  }
}

#endif // EMBER_TEST
