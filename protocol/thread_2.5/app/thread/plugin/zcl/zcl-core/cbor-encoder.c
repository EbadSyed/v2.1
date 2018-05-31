// File: cbor-encoder.c
// Description: Writing out structs as CBOR (RFC 7049).
//
// Copyright 2015 Silicon Laboratories, Inc.                                *80*

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include "zcl-core.h"

// When true, the map/array "...Indefinite..." API functions will instead
// produce encodings with definite lengths. This is the default mode.
static bool forceDefiniteLengthEncoding = true;

//----------------------------------------------------------------
// Encoding

static uint32_t addCborHeader(CborState *state, uint8_t type, uint32_t length)
{
  uint8_t temp[5];
  uint8_t *finger = temp;

  if (state->finger == NULL || state->end <= state->finger) {
    return false;
  }

  if (length <= CBOR_MAX_LENGTH) {
    *finger++ = type | length;
  } else if (length < 1 << 8) {
    *finger++ = type | CBOR_1_BYTE_LENGTH;
    *finger++ = length;
  } else if (length < 1 << 16) {
    *finger++ = type | CBOR_2_BYTE_LENGTH;
    *finger++ = HIGH_BYTE(length);
    *finger++ = LOW_BYTE(length);
  } else {
    *finger++ = type | CBOR_4_BYTE_LENGTH;
    emberStoreHighLowInt32u(finger, length);
    finger += 4;
  }

  uint8_t bytes = finger - temp;
  if (state->finger + bytes <= state->end) {
    MEMCOPY(state->finger, temp, bytes);
    state->finger += bytes;
    return bytes;
  }

  return 0;
}

static uint32_t appendBytes(CborState *state,
                            uint8_t type,
                            const uint8_t *bytes,
                            uint16_t length)
{
  uint32_t len = addCborHeader(state, type, length);

  if (len && (state->finger + length < state->end)) {
    MEMCOPY(state->finger, bytes, length);
    state->finger += length;
    len += length;
  }
  return len;
}

static void incrementCount(CborState *state)
{
  if (state->nestDepth > 0) {
    if (state->nestStack[state->nestDepth - 1].phead != NULL) {
      // Indefinite - count up from zero until break.
      state->nestStack[state->nestDepth - 1].count++;
    } else {
      // Definite - count down from known starting count until zero.
      state->nestStack[state->nestDepth - 1].count--;
      if (state->nestStack[state->nestDepth - 1].count == 0) {
        // Done with this definite layer, pop stack
        state->nestDepth--;
        incrementCount(state);
      }
    }
  }
}

bool emCborGetForceDefiniteLengthEncoding()
{
  return forceDefiniteLengthEncoding;
}

void emCborSetForceDefiniteLengthEncoding(bool force)
{
  forceDefiniteLengthEncoding = force;
}

bool emCborEncodeKey(CborState *state, uint16_t key)
{
  if (addCborHeader(state, CBOR_UNSIGNED, key) != 0) {
    incrementCount(state);
    return true;
  }
  return false;
}

bool emCborEncodeValue(CborState *state,
                       uint8_t type,
                       uint16_t valueLength,
                       const uint8_t *valueLoc)
{
  uint32_t appendedLen = 0;

  if (state->finger == NULL
      || state->end <= state->finger) {
    return false;
  }

  switch (type) {
    case EMBER_ZCLIP_TYPE_BOOLEAN:
      if (state->finger < state->end) {
        *state->finger++ = (*((uint8_t *) valueLoc) != 0
                            ? CBOR_TRUE
                            : CBOR_FALSE);
        appendedLen = -1;
      }
      break;

    case EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER:
      appendedLen = addCborHeader(state,
                                  CBOR_UNSIGNED,
                                  emFetchInt32uValue(valueLoc, valueLength));
      break;

    case EMBER_ZCLIP_TYPE_INTEGER: {
      int32_t n = emFetchInt32sValue(valueLoc, valueLength);
      if (n < 0) {
        appendedLen = addCborHeader(state, CBOR_NEGATIVE, -1 - n);
      } else {
        appendedLen = addCborHeader(state, CBOR_UNSIGNED, n);
      }
      break;
    }

    case EMBER_ZCLIP_TYPE_BINARY: {
      EmberZclStringType_t *ezst = (EmberZclStringType_t *)((void *)valueLoc);
      appendedLen = appendBytes(state, CBOR_BYTES, ezst->ptr, ezst->length);
      break;
    }

    case EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY:
      appendedLen = appendBytes(state, CBOR_BYTES, valueLoc, valueLength);
      break;

    case EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY: {
      uint8_t *buffer = (uint8_t *)((void *)valueLoc);
      appendedLen = appendBytes(state, CBOR_BYTES, &buffer[1], emberZclStringLength(buffer));
      break;
    }

    case EMBER_ZCLIP_TYPE_STRING:
    case EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING: {
      appendedLen = appendBytes(state, CBOR_TEXT, valueLoc, strlen((const char *) valueLoc));
      break;
    }

    case EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING: {
      EmberZclStringType_t *ezst = (EmberZclStringType_t *)((void *)valueLoc);
      appendedLen = appendBytes(state, CBOR_TEXT, ezst->ptr, ezst->length);
      break;
    }

    case EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING: {
      uint8_t *buffer = (uint8_t *)((void *)valueLoc);
      appendedLen = appendBytes(state, CBOR_TEXT, &buffer[1], emberZclStringLength(buffer));
      break;
    }

    case EMBER_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_BINARY:
    case EMBER_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_STRING:
    case EMBER_ZCLIP_TYPE_UINT16_LENGTH_STRING: {
      // TODO: Handle long ZigBee strings.
      appendedLen = 0;
      break;
    }

    default:
      appendedLen = 0;
      break;
  }
  if (appendedLen != 0) {
    incrementCount(state);
    return true;
  }
  return false;
}

void emCborEncodeStart(CborState *state, uint8_t *output, uint16_t outputSize)
{
  MEMSET(state, 0, sizeof(CborState));
  state->start = output;
  state->finger = output;
  state->end = output + outputSize;
}

uint32_t emCborEncodeSize(const CborState *state)
{
  return state->finger - state->start;
}

bool emCborEncodeStruct(CborState *state,
                        const ZclipStructSpec *structSpec,
                        const void *theStruct)
{
  ZclipStructData structData;
  uint16_t i;

  if (state->finger == NULL
      || state->end <= state->finger) {
    return false;
  }

  if (!emExpandZclipStructData(structSpec, &structData)) {
    return false;
  }

  emCborEncodeIndefiniteMap(state);

  for (i = 0; i < structData.fieldCount; i++) {
    ZclipFieldData fieldData;
    emGetNextZclipFieldData(&structData, &fieldData);
    const uint8_t *valueLoc = (uint8_t *) theStruct + fieldData.valueOffset;
    if (fieldData.name == NULL) {
      emCborEncodeKey(state, i);
    } else {
      emCborEncodeValue(state,
                        EMBER_ZCLIP_TYPE_STRING,
                        0, // value length - unused
                        (const uint8_t *)fieldData.name);
    }
    emCborEncodeValue(state,
                      fieldData.valueType,
                      fieldData.valueSize,
                      valueLoc);
  }

  emCborEncodeBreak(state);
  return true;
}

uint16_t emCborEncodeOneStruct(uint8_t *output,
                               uint16_t outputSize,
                               const ZclipStructSpec *structSpec,
                               const void *theStruct)
{
  CborState state;
  emCborEncodeStart(&state, output, outputSize);
  emCborEncodeStruct(&state, structSpec, theStruct);
  return emCborEncodeSize(&state);
}

// Maps

bool emCborEncodeMapStart(CborState *state,
                          uint8_t *output,
                          uint16_t outputSize,
                          uint16_t count)
{
  emCborEncodeStart(state, output, outputSize);
  return emCborEncodeMap(state, count);
}

bool emCborEncodeIndefiniteMapStart(CborState *state,
                                    uint8_t *output,
                                    uint16_t outputSize)
{
  emCborEncodeStart(state, output, outputSize);
  return emCborEncodeIndefiniteMap(state);
}

bool emCborEncodeDefinite(CborState *state, uint8_t valueType, uint16_t count)
{
  if (state->finger == NULL
      || state->end <= state->finger
      || state->nestDepth >= MAX_MAP_ARRAY_NESTING) {
    return false;
  } else if (addCborHeader(state, valueType, count) != 0) {
    state->nestDepth++;
    state->nestStack[state->nestDepth - 1].count
      = (valueType == CBOR_MAP ? count * 2 : count);
    state->nestStack[state->nestDepth - 1].phead = NULL;
    return true;
  } else {
    return false;
  }
}

bool emCborEncodeIndefinite(CborState *state, uint8_t valueType)
{
  if (state->finger == NULL
      || state->end <= state->finger
      || state->nestDepth >= MAX_MAP_ARRAY_NESTING) {
    return false;
  } else if (2 <= state->end - state->finger) {
    state->nestDepth++;
    state->nestStack[state->nestDepth - 1].count = 0;
    state->nestStack[state->nestDepth - 1].phead = state->finger;
    *state->finger++ = valueType | CBOR_INDEFINITE_LENGTH;
    return true;
  } else {
    return false;
  }
}

bool emCborEncodeBreak(CborState *state)
{
  if (state->finger == NULL
      || state->end <= state->finger) {
    return false;
  }

  if (1 <= state->end - state->finger) {
    if (forceDefiniteLengthEncoding) {
      if (state->nestDepth > 0
          && state->nestStack[state->nestDepth - 1].phead != NULL) {
        // update array/map header with count
        uint8_t *phead = state->nestStack[state->nestDepth - 1].phead;
        uint8_t type = (*phead & CBOR_TYPE_MASK);
        uint32_t count = state->nestStack[state->nestDepth - 1].count;
        if (type == CBOR_MAP) {
          // TODO: What if error produced odd-valued count?
          count >>= 1; // counted each map key and value; half that for entries
        }
        if (count <= CBOR_MAX_LENGTH) {
          // Just patch the additionalInfo bits of the map/array header
          *phead = type | (count & CBOR_LENGTH_MASK);
        } else {
          // Shift map/array content forward to create a length field
          // that is sufficiently large (either 1 or 2 bytes) to hold the count.
          if (count < 1 << 8) {
            MEMMOVE(phead + 2, phead + 1, (state->finger - phead + 1));
            state->finger += 1;
            *phead = (type | CBOR_1_BYTE_LENGTH);
            *(phead + 1) = count;
          } else if (2 <= state->end - state->finger) {
            // Assume count of 256+ is within representation of 16 bits.
            // Any count that requires more than that would have exhausted
            // the buffer while encoding the content.
            MEMMOVE(phead + 3, phead + 1, (state->finger - phead + 1));
            state->finger += 2;
            *phead = (type | CBOR_2_BYTE_LENGTH);
            *(phead + 1) = HIGH_BYTE(count);
            *(phead + 2) = LOW_BYTE(count);
          } else {
            // Insufficient space for 16-bit count
            return false;
          }
        }
      }
    } else {
      *state->finger++ = CBOR_BREAK;
    }
    // pop stack, and increment count for next layer up, if any
    state->nestDepth--;
    incrementCount(state);
    return true;
  } else {
    return false;
  }
}

bool emCborEncodeMapEntry(CborState *state,
                          uint16_t key,
                          uint8_t valueType,
                          uint16_t valueSize,
                          const uint8_t *valueLoc)
{
  return (emCborEncodeKey(state, key)
          && emCborEncodeValue(state, valueType, valueSize, valueLoc));
}

// Arrays

bool emCborEncodeArrayStart(CborState *state,
                            uint8_t *output,
                            uint16_t outputSize,
                            uint16_t count)
{
  emCborEncodeStart(state, output, outputSize);
  return emCborEncodeArray(state, count);
}

bool emCborEncodeIndefiniteArrayStart(CborState *state,
                                      uint8_t *output,
                                      uint16_t outputSize)
{
  emCborEncodeStart(state, output, outputSize);
  return emCborEncodeIndefiniteArray(state);
}

//----------------------------------------------------------------
// Decoding

void emCborDecodeStart(CborState *state,
                       const uint8_t *input,
                       uint16_t inputSize)
{
  MEMSET(state, 0, sizeof(CborState));
  state->start = (uint8_t *) input;
  state->finger = (uint8_t *) input;
  state->end = (uint8_t *) input + inputSize;
}

static bool peekOrReadCborHeaderLength(CborState *state,
                                       uint8_t b0,
                                       uint32_t *result,
                                       bool read)
{
  uint8_t length = b0 & CBOR_LENGTH_MASK;
  *result = 0;

  uint8_t *finger = state->finger;
  if (finger == NULL) {
    return false;
  }

  if (!read) {
    finger += 1;        // skip over b0
  }

  if (length == CBOR_INDEFINITE_LENGTH) {
    *result = -1;
  } else if (length <= CBOR_MAX_LENGTH) {
    *result = length;
  } else if (length == CBOR_1_BYTE_LENGTH) {
    *result = *finger++;
  } else if (length == CBOR_2_BYTE_LENGTH) {
    *result = HIGH_LOW_TO_INT(finger[0], finger[1]);
    finger += 2;
  } else {
    *result = emberFetchHighLowInt32u(finger);
    finger += 4;
  }

  if (read) {
    state->finger = finger;
  }

  return (finger <= state->end);
}

static bool readCborHeaderLength(CborState *state,
                                 uint8_t b0,
                                 uint32_t *result)
{
  return peekOrReadCborHeaderLength(state, b0, result, true);
}

static const uint32_t uintMasks[] = {
  0x000000FF,
  0x0000FFFF,
  0,
  0xFFFFFFFF
};

static uint8_t zclipTypeToCborType[] = {
  0,             // BOOLEAN - entry for booleans is not used
  CBOR_UNSIGNED, // INTEGER - can be CBOR_NEGATIVE as well
  CBOR_UNSIGNED, // UNSIGNED_INTEGER
  CBOR_BYTES,    // BINARY
  CBOR_BYTES,    // FIXED_LENGTH_BINARY
  CBOR_TEXT,     // STRING
  CBOR_TEXT,     // MAX_LENGTH_STRING
  CBOR_TEXT,     // UINT8_LENGTH_STRING
  CBOR_TEXT,     // UINT16_LENGTH_STRING
  CBOR_BYTES,    // UINT8_LENGTH_PREFIXED_BINARY
  CBOR_BYTES,    // UINT16_LENGTH_PREFIXED_BINARY
  CBOR_TEXT,     // UINT8_LENGTH_PREFIXED_STRING
  CBOR_TEXT      // UINT16_LENGTH_PREFIXED_STRING
};

// returns whether read is successful.
static EmZclCoreCborValueReadStatus_t
realReadCborValue(CborState *state,
                  ZclipFieldData *fieldData,
                  uint8_t *valueLocation)
{
  if (state->finger == NULL
      || state->end <= state->finger) {
    return EM_ZCL_CORE_CBOR_VALUE_READ_ERROR;
  }

  uint8_t b0 = *state->finger++;
  uint32_t length = 0;

  // The maximum type we can handle.
  if (EMBER_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_STRING < fieldData->valueType) {
    return EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE;
  }

  uint8_t cborType = zclipTypeToCborType[fieldData->valueType];

  if (fieldData->valueType == EMBER_ZCLIP_TYPE_BOOLEAN) {
    if (b0 == CBOR_TRUE || b0 == CBOR_FALSE) {
      *valueLocation = (b0 == CBOR_TRUE);
      return EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS;
    } else {
      return EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE;
    }
  } else if (!readCborHeaderLength(state, b0, &length)) {
    return EM_ZCL_CORE_CBOR_VALUE_READ_ERROR;
    return state->readStatus;
  } else if (!(((b0 & CBOR_TYPE_MASK) == cborType)
               || (fieldData->valueType == EMBER_ZCLIP_TYPE_INTEGER
                   && (b0 & CBOR_TYPE_MASK) == CBOR_NEGATIVE))) {
    return EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE;
    return state->readStatus;
  } else if ((cborType == CBOR_BYTES
              || cborType == CBOR_TEXT)
             && (state->end - state->finger) < length) {
    return EM_ZCL_CORE_CBOR_VALUE_READ_ERROR;
    return state->readStatus;
  } else {
    switch (fieldData->valueType) {
      case EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER:
        if ((length & uintMasks[fieldData->valueSize - 1]) == length) {
          emStoreInt32uValue(valueLocation, length, fieldData->valueSize);
          return EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS;
        } else {
          return EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE;
        }

      case EMBER_ZCLIP_TYPE_INTEGER:
        if ((length & (uintMasks[fieldData->valueSize - 1] >> 1)) == length) {
          emStoreInt32uValue(valueLocation,
                             ((b0 & CBOR_TYPE_MASK) == CBOR_UNSIGNED
                              ? length
                              : -1 - (int32_t) length),
                             fieldData->valueSize);
          return EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS;
        } else {
          return EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE;
        }

      case EMBER_ZCLIP_TYPE_BINARY:     // Same max length as strings.
      case EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING:
        if (length <= EMBER_ZCL_STRING_LENGTH_MAX) {
          EmberZclStringType_t *ezst =
            (EmberZclStringType_t *) ((void *) valueLocation);
          ezst->ptr = state->finger;
          ezst->length = length;
          state->finger += length;
          return EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS;
        } else {
          return EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE;
        }

      case EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY:
        if (length == fieldData->valueSize) {
          MEMCOPY(valueLocation, state->finger, length);
          state->finger += length;
          return EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS;
        } else if (length < fieldData->valueSize) {
          return EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_SMALL;
        } else {
          return EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE;
        }

      case EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY:
      case EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING:
        if (length <= EMBER_ZCL_STRING_LENGTH_MAX) {
          uint8_t *buffer = (uint8_t *) ((void *) valueLocation);
          buffer[0] = length;
          MEMCOPY(&buffer[1], state->finger, length);
          state->finger += length;
          return EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS;
        } else {
          return EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE;
        }

      case EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING:
        if (length + 1 <= fieldData->valueSize) {
          MEMCOPY(valueLocation, state->finger, length);
          valueLocation[length] = 0;
          state->finger += length;
          return EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS;
        } else {
          return EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE;
        }

      case EMBER_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_BINARY:
      case EMBER_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_STRING:
      case EMBER_ZCLIP_TYPE_UINT16_LENGTH_STRING:
        // TODO: Handle long ZigBee strings.
        return EM_ZCL_CORE_CBOR_VALUE_READ_NOT_SUPPORTED;

      default:
        return EM_ZCL_CORE_CBOR_VALUE_READ_ERROR;
    }
  }
}

static EmZclCoreCborValueReadStatus_t readCborValue(CborState *state,
                                                    ZclipFieldData *fieldData,
                                                    uint8_t *valueLocation)
{
  state->readStatus = realReadCborValue(state, fieldData, valueLocation);
  return state->readStatus;
}

static bool cborDecodeStruct(CborState *state,
                             const ZclipStructSpec *structSpec,
                             void *theStruct)
{
  if (state->finger == NULL || state->end <= state->finger) {
    return false;
  }

  ZclipStructData structData;
  if (!emExpandZclipStructData(structSpec, &structData)) {
    return false;
  }

  uint8_t b0 = *state->finger++;

  if ((b0 & CBOR_TYPE_MASK) != CBOR_MAP) {
    return false;
  }

  uint32_t fieldCount = 0;
  if (!readCborHeaderLength(state, b0, &fieldCount)) {
    return false;
  }

  ZclipFieldData fieldData;

  // Track (by field index) the number of mandatory fields we expect in the structure.
  // Only mandatory fields among the first 32 fields are tracked.
  uint32_t mandatoryFieldBitmap = 0;
  emResetZclipFieldData(&structData);
  for (uint16_t i = 0; i < structData.fieldCount; i++) {
    emGetNextZclipFieldData(&structData, &fieldData);
    if (fieldData.valueMandatory && i < sizeof(mandatoryFieldBitmap) * 8) {
      mandatoryFieldBitmap |= (1 << i); // Set bit for each mandatory field
    }
  }

  for (uint16_t i = 0; i < fieldCount || fieldCount == ((uint32_t) -1); i++) {
    uint8_t type = emCborDecodePeek(state, NULL);
    if (type == CBOR_BREAK) {
      return (fieldCount == ((uint32_t) -1));
    }

    uint16_t keyIndex = 0;
    uint8_t keyName[8];
    if (type == CBOR_UNSIGNED) {
      emCborDecodeValue(state,
                        EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                        sizeof(keyIndex),
                        (uint8_t *)&keyIndex);
    } else if (type == CBOR_TEXT) {
      emCborDecodeValue(state,
                        EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING,
                        sizeof(keyName),
                        keyName);
    } else {
      // Skip the key and value.
      emCborDecodeSkipValue(state);
      emCborDecodeSkipValue(state);
      continue;
    }

    uint16_t j;
    emResetZclipFieldData(&structData);
    for (j = 0; j < structData.fieldCount; j++) {
      emGetNextZclipFieldData(&structData, &fieldData);
      if ((type == CBOR_UNSIGNED && fieldData.name == NULL && keyIndex == j)
          || (type == CBOR_TEXT
              && fieldData.name != NULL
              && strcmp((const char *)keyName, fieldData.name) == 0)) {
        break;
      }
    }
    if (j == structData.fieldCount) {
      emCborDecodeSkipValue(state);
    } else {
      uint8_t *valueLoc = (uint8_t *) theStruct + fieldData.valueOffset;
      if (readCborValue(state, &fieldData, valueLoc)
          != EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS) {
        return false;
      }
      if (fieldData.valueMandatory && j < sizeof(mandatoryFieldBitmap) * 8) {
        mandatoryFieldBitmap &= ~(1 << j); // Clear bit for decoded mandatory field
      }
    }
  }

  // Check that all expected mandatory fields are present.
  if (mandatoryFieldBitmap != 0) { // All bits set for mandatory fields should have been cleared
    return false;
  }

  return true;
}

bool emCborDecodeStruct(CborState *state,
                        const ZclipStructSpec *structSpec,
                        void *theStruct)
{
  return cborDecodeStruct(state, structSpec, theStruct);
}

bool emCborDecodeOneStruct(const uint8_t *input,
                           uint16_t inputSize,
                           const ZclipStructSpec *structSpec,
                           void *theStruct)
{
  CborState state;
  emCborDecodeStart(&state, input, inputSize);
  return cborDecodeStruct(&state, structSpec, theStruct);
}

// Decoding arrays and maps

// Decrement the number of array or map entries.  If we know the number of
// elements we decrement the count.  If it reaches zero we pop off the
// innermost count.  If there are an indefinite number of elements we
// peek to see if the next thing is a break, in which case we pop off the
// innermost count.

static bool decrementCount(CborState *state)
{
  if (state->finger == NULL
      || state->end <= state->finger) {
    return false;
  }

  if (state->nestDepth != 0) {
    uint32_t count = state->nestStack[state->nestDepth - 1].count;
    // fprintf(stderr, "[pop depth %d count %d]\n",
    //         state->nestDepth,
    //         count);
    if (count == (uint32_t) -1) {          // ends with a break
      if (*state->finger == CBOR_BREAK) {
        state->nestDepth -= 1;
        state->finger += 1;
        // fprintf(stderr, "[break]\n");
        return false;
      }
    } else if (count == 0) {
      state->nestDepth -= 1;
      return false;
    } else {
      state->nestStack[state->nestDepth - 1].count -= 1;
    }
  }
  return true;
}

bool emCborDecodeSequence(CborState *state, uint8_t valueType)
{
  if (state->finger == NULL
      || state->end <= state->finger
      || !decrementCount(state)) {
    return false;
  }

  if (state->nestDepth == MAX_MAP_ARRAY_NESTING) {
    return false;
  }

  uint8_t b0 = *state->finger++;

  if ((b0 & CBOR_TYPE_MASK) != valueType) {
    return false;
  }

  uint32_t count = 0;
  bool status = readCborHeaderLength(state, b0, &count);
  if (!status) {
    return false;
  }

  state->nestStack[state->nestDepth].count =
    ((count != (uint32_t) -1
      && valueType == CBOR_MAP)
     ? count * 2        // maps have two values (key+value) for each item
     : count);
  state->nestDepth += 1;
  return true;
}

uint16_t emCborDecodeKey(CborState *state)
{
  uint16_t key;
  if (emCborDecodeValue(state,
                        EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                        2,
                        (uint8_t *) &key)) {
    return key;
  } else {
    return -1;
  }
}

bool emCborDecodeValue(CborState *state,
                       uint8_t valueType,
                       uint16_t valueSize,
                       uint8_t *valueLoc)
{
  if (!decrementCount(state)) {
    state->readStatus = EM_ZCL_CORE_CBOR_VALUE_READ_ERROR;
    return false;
  } else {
    ZclipFieldData fieldData;
    fieldData.valueType = valueType;
    fieldData.valueSize = valueSize;

    if (readCborValue(state, &fieldData, valueLoc) == EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS) {
      return true;
    } else {
      return false;
    }
  }
}

// Returns the type of the next value.

uint8_t emCborDecodePeek(CborState *state, uint32_t *length)
{
  // TODO: There might be a better value to return here / at least we are not
  // crashing.
  if (state->finger == NULL
      || state->end <= state->finger) {
    return CBOR_BREAK;
  }

  uint8_t b0 = *state->finger;

  if (state->nestDepth != 0) {
    uint32_t count = state->nestStack[state->nestDepth - 1].count;
    if ((count == (uint32_t) -1
         && b0 == CBOR_BREAK)
        || count == 0) {
      return CBOR_BREAK;
    }
  }

  if (length != NULL && !peekOrReadCborHeaderLength(state, b0, length, false)) {
    return CBOR_BREAK;
  }

  if ((b0 & CBOR_TYPE_MASK) == CBOR_MISC) {
    return b0;
  } else {
    return b0 & CBOR_TYPE_MASK;
  }
}

bool emCborDecodeSkipValue(CborState *state)
{
  if (state->finger == NULL
      || state->end <= state->finger
      || !decrementCount(state)) {
    return false;
  }

  uint32_t needed = 1;
  uint32_t saved[16];
  uint8_t depth = 0;

  while (true) {
    uint8_t b0 = *state->finger++;
    if (needed != (uint32_t) -1) {
      needed -= 1;
      if (needed == 0 && depth > 0) {
        depth -= 1;
        needed = saved[depth];
      }
    }
    if ((b0 & CBOR_TYPE_MASK) == CBOR_MISC) {
      switch (b0) {
        case CBOR_EXTENDED: state->finger += 1; break;
        case CBOR_FLOAT16:  state->finger += 2; break;
        case CBOR_FLOAT32:  state->finger += 4; break;
        case CBOR_FLOAT64:  state->finger += 8; break;
        case CBOR_BREAK:
          if (needed == (uint32_t) -1) {
            depth -= 1;
            needed = saved[depth];
          }
          break;
      }
    } else {
      uint32_t length = 0;
      bool status = readCborHeaderLength(state, b0, &length);

      if (!status) {
        return false;
      }

      switch (b0 & CBOR_TYPE_MASK) {
        case CBOR_UNSIGNED:
        case CBOR_NEGATIVE:
          // nothing to do
          break;
        case CBOR_BYTES:
        case CBOR_TEXT:
          state->finger += length;
          break;
        case CBOR_MAP:
          if (length != (uint32_t) -1) {
            length <<= 1;
          }
        // fall through
        case CBOR_ARRAY:
          if (depth >= 16) {
            return false;
          }
          saved[depth] = needed;
          depth += 1;
          needed = length;
          break;
        case CBOR_TAG:
          if (needed != (uint32_t) -1) {
            needed += 1;
          }
          break;
      }
    }
    if (needed == 0) {
      return true;
    }
  }
}
