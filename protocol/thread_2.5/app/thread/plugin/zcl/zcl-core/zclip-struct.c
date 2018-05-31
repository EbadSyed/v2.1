// Copyright 2015 Silicon Laboratories, Inc.                                *80*

#include PLATFORM_HEADER
#include "core/ember-stack.h"
#include "zcl-core.h"

// Functions for extracting information about structs and fields.

bool emExpandZclipStructData(const ZclipStructSpec *spec,
                             ZclipStructData *structData)
{
  uint32_t objectData = spec[0];

  if (LOW_BYTE(objectData) != EMBER_ZCLIP_START_MARKER) {
    return false;
  }

  structData->spec = spec;
  structData->size = objectData << 16;
  structData->fieldCount = HIGH_BYTE(objectData);
  emResetZclipFieldData(structData);

  return true;
}

void emResetZclipFieldData(ZclipStructData *structData)
{
  structData->fieldIndex = 0;
  structData->next = structData->spec + 2;      // skip header and names
}

bool emZclipFieldDataFinished(ZclipStructData *structData)
{
  return structData->fieldIndex == structData->fieldCount;
}

void emGetNextZclipFieldData(ZclipStructData *structData,
                             ZclipFieldData *fieldData)
{
  uint32_t data = structData->next[0];

  fieldData->valueType = LOW_BYTE(data);
  fieldData->valueMandatory = fieldData->valueType & EMBER_ZCLIP_TYPE_MODIFIER_MANDATORY; // Field is mandatory if valueType top bit is set.
  fieldData->valueType &= ~(EMBER_ZCLIP_TYPE_MODIFIER_MANDATORY);                         // Always clear valueType top bit.

  fieldData->valueSize = HIGH_BYTE(data);
  fieldData->valueOffset = data >> 16;

  structData->fieldIndex += 1;
  structData->next += 1;

  fieldData->name = (char *)structData->next[0];
  structData->next += 1;
}

//----------------------------------------------------------------
// Utilities for reading and writing integer fields.  These should probably
// go somewhere else.

uint32_t emFetchInt32uValue(const uint8_t *valueLoc, uint16_t valueSize)
{
  switch (valueSize) {
    case 1:
      return *((const uint8_t *) valueLoc);
      break;
    case 2:
      return *((const uint16_t *) ((const void *) valueLoc));
      break;
    case 4:
      return *((const uint32_t *) ((const void *) valueLoc));
      break;
    default:
      assert(false);
      return 0;
  }
}

int32_t emFetchInt32sValue(const uint8_t *valueLoc, uint16_t valueSize)
{
  switch (valueSize) {
    case 1:
      return *((const int8_t *) valueLoc);
    case 2:
      return *((const int16_t *) ((const void *) valueLoc));
    case 4:
      return *((const int32_t *) ((const void *) valueLoc));
    default:
      assert(false);
      return 0;
  }
}

void emStoreInt32sValue(uint8_t* valueLoc, int32_t value, uint8_t valueSize)
{
  switch (valueSize) {
    case 1:
      *((int8_t *) valueLoc) = value;
      break;
    case 2:
      *((int16_t *) ((void *) valueLoc)) = value;
      break;
    case 4:
      *((int32_t *) ((void *) valueLoc)) = value;
      break;
    default:
      assert(false);
  }
}

void emStoreInt32uValue(uint8_t* valueLoc, uint32_t value, uint8_t valueSize)
{
  switch (valueSize) {
    case 1:
      *((uint8_t *) valueLoc) = value;
      break;
    case 2:
      *((uint16_t *) ((void *) valueLoc)) = value;
      break;
    case 4:
      *((uint32_t *) ((void *) valueLoc)) = value;
      break;
    default:
      assert(false);
  }
}

uint8_t emberZclStringLength(const uint8_t *buffer)
{
  // The first byte specifies the length of the string.  If the length is set
  // to the invalid value, there is no ocet or character data.
  return (buffer[0] == EMBER_ZCL_STRING_LENGTH_INVALID ? 0 : buffer[0]);
}

uint8_t emberZclStringSize(const uint8_t *buffer)
{
  return EMBER_ZCL_STRING_OVERHEAD + emberZclStringLength(buffer);
}

uint16_t emberZclLongStringLength(const uint8_t *buffer)
{
  // The first two bytes specify the length of the long string.  If the length
  // is set to the invalid value, there is no octet or character data.
  uint16_t length = emberFetchLowHighInt16u(buffer);
  return (length == EMBER_ZCL_LONG_STRING_LENGTH_INVALID ? 0 : length);
}

uint16_t emberZclLongStringSize(const uint8_t *buffer)
{
  return EMBER_ZCL_LONG_STRING_OVERHEAD + emberZclLongStringLength(buffer);
}

uint8_t emZclDirectBufferedZclipType(uint8_t zclipType)
{
  // The switch cases represent "public" string types that "indirectly" encode/decode from/to
  // an EmberZclStringType_t structure (which contains length and pointer to string value, not
  // the string value itself). This is the format used for automated encoding/decoding using
  // ZclipStructs.
  //
  // This function converts the public types to corresponding internal-only types, which are used
  // to "directly" encode/decode from/to a length-prefixed string value contained in a byte buffer.
  // This is the internal format used for some manual encoding/decoding cases (attributes) and is
  // the format in which ZCL string values are stored locally or conveyed to external handlers.
  //
  // All other types need no special handling and are returned unchanged.
  switch (zclipType) {
    case EMBER_ZCLIP_TYPE_BINARY:
      return EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY;

    case EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING:
      return EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING;

    default:
      return zclipType;
  }
}