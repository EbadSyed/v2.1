// Copyright 2017 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include "thread-bookkeeping.h"
#include "thread-callbacks.h"
#include "zcl-core.h"

typedef struct {
  EmberZclAttributeContext_t context;
  EmberZclReadAttributeResponseHandler readHandler;
  EmberZclWriteAttributeResponseHandler writeHandler;
} Response;

typedef struct {
  uint16_t count;
  uint16_t usedCounts;
} FilterState;

static void *attributeDataLocation(EmberZclEndpointIndex_t endpointIndex,
                                   const EmZclAttributeEntry_t *attribute);
static const void *attributeDefaultMinMaxLocation(const EmZclAttributeEntry_t *attribute,
                                                  EmZclAttributeMask_t dataBit);
static bool isValueLocIndirect(const EmZclAttributeEntry_t *attribute);
static bool isLessThan(const uint8_t *dataA,
                       size_t dataALength,
                       const uint8_t *dataB,
                       size_t dataBLength,
                       const EmZclAttributeEntry_t *attribute);
static bool encodeAttributeResponseMap(CborState *state,
                                       EmberZclEndpointId_t endpointId,
                                       const EmZclAttributeEntry_t *attribute);
static EmberZclStatus_t getAttributeIdsHandler(const EmZclContext_t *context,
                                               CborState *state,
                                               void *data);
static EmberZclStatus_t getAttributeHandler(const EmZclContext_t *context,
                                            CborState *state,
                                            void *data);
static EmberZclStatus_t updateAttributeHandler(const EmZclContext_t *context,
                                               CborState *state,
                                               void *data);
static bool filterAttribute(const EmZclContext_t *context,
                            FilterState *state,
                            const EmZclAttributeEntry_t *attribute);
static void readWriteResponseHandler(EmberCoapStatus status,
                                     EmberCoapCode code,
                                     EmberCoapReadOptions *options,
                                     uint8_t *payload,
                                     uint16_t payloadLength,
                                     EmberCoapResponseInfo *info);
static void handleRead(EmberZclMessageStatus_t status,
                       const Response *response);
static void handleWrite(EmberZclMessageStatus_t status,
                        const Response *response);
static EmberZclStatus_t writeAttribute(EmberZclEndpointIndex_t index,
                                       EmberZclEndpointId_t endpointId,
                                       const EmZclAttributeEntry_t *attribute,
                                       const void *data,
                                       size_t dataLength);
static void callPostAttributeChange(EmberZclEndpointId_t endpointId,
                                    const EmZclAttributeEntry_t *attribute,
                                    const void *data,
                                    size_t dataLength);
static bool callPreAttributeChange(EmberZclEndpointId_t endpointId,
                                   const EmZclAttributeEntry_t *attribute,
                                   const void *data,
                                   size_t dataLength);
static size_t tokenize(const EmZclContext_t *context,
                       void *skipData,
                       uint8_t depth,
                       const uint8_t **tokens,
                       size_t *tokenLengths);
static void convertBufferToTwosComplement(uint8_t *buffer, size_t size);

#define oneBitSet(mask) ((mask) != 0 && (mask) == ((mask) & - (mask)))

#define attributeDefaultLocation(a) \
  attributeDefaultMinMaxLocation((a), EM_ZCL_ATTRIBUTE_DATA_DEFAULT)
#define attributeMinimumLocation(a) \
  attributeDefaultMinMaxLocation((a), EM_ZCL_ATTRIBUTE_DATA_MINIMUM)
#define attributeMaximumLocation(a) \
  attributeDefaultMinMaxLocation((a), EM_ZCL_ATTRIBUTE_DATA_MAXIMUM)

// This limit is copied from MAX_ENCODED_URI in coap.c.
#define MAX_ATTRIBUTE_URI_LENGTH 64

void emberZclResetAttributes(EmberZclEndpointId_t endpointId)
{
  // TODO: Handle tokens.
  for (size_t i = 0; i < EM_ZCL_ATTRIBUTE_COUNT; i++) {
    const EmZclAttributeEntry_t *attribute = &emZclAttributeTable[i];
    EmberZclEndpointIndex_t index
      = emberZclEndpointIdToIndex(endpointId, attribute->clusterSpec);
    if (index != EMBER_ZCL_ENDPOINT_INDEX_NULL
        && emZclIsAttributeLocal(attribute)) {
      const void *dephault = attributeDefaultLocation(attribute);
      writeAttribute(index, endpointId, attribute, dephault, attribute->size);
      callPostAttributeChange(endpointId, attribute, dephault, attribute->size);
    }
  }
}

EmberZclStatus_t emberZclReadAttribute(EmberZclEndpointId_t endpointId,
                                       const EmberZclClusterSpec_t *clusterSpec,
                                       EmberZclAttributeId_t attributeId,
                                       void *buffer,
                                       size_t bufferLength)
{
  return emZclReadAttributeEntry(endpointId,
                                 emZclFindAttribute(clusterSpec,
                                                    attributeId,
                                                    false), // exclude remote
                                 buffer,
                                 bufferLength);
}

EmberZclStatus_t emberZclWriteAttribute(EmberZclEndpointId_t endpointId,
                                        const EmberZclClusterSpec_t *clusterSpec,
                                        EmberZclAttributeId_t attributeId,
                                        const void *buffer,
                                        size_t bufferLength)
{
  return emZclWriteAttributeEntry(endpointId,
                                  emZclFindAttribute(clusterSpec,
                                                     attributeId,
                                                     false), // exclude remote
                                  buffer,
                                  bufferLength,
                                  true);                    // enable update
}

EmberZclStatus_t emberZclExternalAttributeChanged(EmberZclEndpointId_t endpointId,
                                                  const EmberZclClusterSpec_t *clusterSpec,
                                                  EmberZclAttributeId_t attributeId,
                                                  const void *buffer,
                                                  size_t bufferLength)
{
  if (!emZclEndpointHasCluster(endpointId, clusterSpec)) {
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  const EmZclAttributeEntry_t *attribute
    = emZclFindAttribute(clusterSpec, attributeId, false); // exclude remote
  if (attribute == NULL) {
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  if (!emZclIsAttributeExternal(attribute)) {
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  callPostAttributeChange(endpointId, attribute, buffer, bufferLength);

  return EMBER_ZCL_STATUS_SUCCESS;
}

const EmZclAttributeEntry_t *emZclFindAttribute(const EmberZclClusterSpec_t *clusterSpec,
                                                EmberZclAttributeId_t attributeId,
                                                bool includeRemote)
{
  for (size_t i = 0; i < EM_ZCL_ATTRIBUTE_COUNT; i++) {
    const EmZclAttributeEntry_t *attribute = &emZclAttributeTable[i];
    int32_t compare
      = emberZclCompareClusterSpec(attribute->clusterSpec, clusterSpec);
    if (compare > 0) {
      break;
    } else if (compare == 0
               && attributeId == attribute->attributeId
               && (includeRemote
                   || emZclIsAttributeLocal(attribute))) {
      return attribute;
    }
  }
  return NULL;
}

EmberZclStatus_t emZclReadAttributeEntry(EmberZclEndpointId_t endpointId,
                                         const EmZclAttributeEntry_t *attribute,
                                         void *buffer,
                                         size_t bufferLength)
{
  if (attribute == NULL || emZclIsAttributeRemote(attribute)) {
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  EmberZclEndpointIndex_t index
    = emberZclEndpointIdToIndex(endpointId, attribute->clusterSpec);
  if (index == EMBER_ZCL_ENDPOINT_INDEX_NULL) {
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  if (emZclIsAttributeExternal(attribute)) {
    return emberZclReadExternalAttributeCallback(endpointId,
                                                 attribute->clusterSpec,
                                                 attribute->attributeId,
                                                 buffer,
                                                 bufferLength);
  }

  // For variable-length attributes, we are a little flexible for buffer sizes.
  // As long as there is enough space in the buffer to store the current value
  // of the attribute, we permit the read, even if the buffer is smaller than
  // the maximum possible size of the attribute.
  void *data = attributeDataLocation(index, attribute);
  size_t size = emZclAttributeSize(attribute, data);
  if (bufferLength < size) {
    return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  }

  MEMCOPY(buffer, data, size);
  return EMBER_ZCL_STATUS_SUCCESS;
}

EmberZclStatus_t emZclWriteAttributeEntry(EmberZclEndpointId_t endpointId,
                                          const EmZclAttributeEntry_t *attribute,
                                          const void *buffer,
                                          size_t bufferLength,
                                          bool enableUpdate)
{
  if ((attribute == NULL) || (emZclIsAttributeRemote(attribute))) {
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  EmberZclEndpointIndex_t index
    = emberZclEndpointIdToIndex(endpointId, attribute->clusterSpec);
  if (index == EMBER_ZCL_ENDPOINT_INDEX_NULL) {
    return EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
  }

  // For variable-length attributes, we are a little flexible for buffer sizes.
  // As long as there is enough space in the table to store the new value of
  // the attribute, we permit the write, even if the actual length of buffer
  // containing the new value is larger than what we have space for.
  size_t size = emZclAttributeSize(attribute, buffer);

  if (attribute->size < size) {
    return EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
  }

  if ((bufferLength == 0)
      || (bufferLength > EMBER_ZCL_ATTRIBUTE_MAX_SIZE)) {
    return EMBER_ZCL_STATUS_INVALID_VALUE;
  }

  // Check new attribute value against appbuilder bounded min and max values.
  // (Attribute must be an integer numeric type for the bound check to be
  // valid).
  if ((emZclIsAttributeBounded(attribute))
      && ((attribute->type == EMBER_ZCLIP_TYPE_INTEGER)
          || (attribute->type == EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER)
          || (attribute->type == EMBER_ZCLIP_TYPE_BOOLEAN))) {
    if ((isLessThan(buffer,
                    bufferLength,
                    attributeMinimumLocation(attribute),
                    attribute->size,
                    attribute))
        || (isLessThan(attributeMaximumLocation(attribute),
                       attribute->size,
                       buffer,
                       bufferLength,
                       attribute))) {
      return EMBER_ZCL_STATUS_INVALID_VALUE;
    }
  }

  if (!enableUpdate) {
    // If we are pre-write error checking return before the actual write.
    return EMBER_ZCL_STATUS_SUCCESS;
  }

  if (!callPreAttributeChange(endpointId, attribute, buffer, size)) {
    return EMBER_ZCL_STATUS_FAILURE;
  }

  EmberZclStatus_t status = writeAttribute(index,
                                           endpointId,
                                           attribute,
                                           buffer,
                                           size);
  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    callPostAttributeChange(endpointId, attribute, buffer, size);
  }
  return status;
}

bool emZclReadEncodeAttributeKeyValue(CborState *state,
                                      EmberZclEndpointId_t endpointId,
                                      const EmZclAttributeEntry_t *attribute,
                                      void *buffer,
                                      size_t bufferLength)
{
  if (attribute == NULL) {
    return true;
  } else {
    return ((emZclReadAttributeEntry(endpointId,
                                     attribute,
                                     buffer,
                                     attribute->size)
             == EMBER_ZCL_STATUS_SUCCESS)
            && emCborEncodeMapEntry(state,
                                    attribute->attributeId,
                                    emZclDirectBufferedZclipType(attribute->type),
                                    attribute->size,
                                    buffer));
  }
}

size_t emZclAttributeSize(const EmZclAttributeEntry_t *attribute,
                          const void *data)
{
  if (attribute->type == EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING) {
    return emberZclStringSize(data);
  } else if (attribute->type == EMBER_ZCLIP_TYPE_UINT16_LENGTH_STRING) {
    return emberZclLongStringSize(data);
  } else {
    return attribute->size;
  }
}

static void *attributeDataLocation(EmberZclEndpointIndex_t endpointIndex,
                                   const EmZclAttributeEntry_t *attribute)
{
  // AppBuilder generates the maximum size for all of the attribute data, so
  // that the app can create a buffer to hold all of the runtime attribute
  // values. This maximum size factors in attributes that need multiple data
  // instances since they exist on multiple endpoints and are not singleton.
  // When an attribute has multiple data instances, the values are stored
  // sequentially in the buffer. AppBuilder also generates the per-attribute
  // offset into the buffer so that it is easy to go from attribute to value.
  static uint8_t attributeData[EM_ZCL_ATTRIBUTE_DATA_SIZE] = EM_ZCL_ATTRIBUTE_DEFAULTS;
  return (attributeData
          + attribute->dataOffset
          + (attribute->size
             * (emZclIsAttributeSingleton(attribute)
                ? 0
                : endpointIndex)));
}

const void *attributeDefaultMinMaxLocation(const EmZclAttributeEntry_t *attribute,
                                           EmZclAttributeMask_t dataBit)
{
  // AppBuilder generates a table of attribute "constants" that are all possible
  // values of defaults, minimums, and maximums that the user has configured
  // their app to use. We don't generate attribute constants that are zero. We
  // always assume that if an attribute doesn't have a bit set for a
  // default/min/max value in its mask, then the value is all zeros.
  //
  // Each attribute uses an index into a lookup table to figure out where each
  // of their default/min/max constants are located in the constant table. The
  // lookup table stores up to 3 indices per attribute (always in this order):
  // a default value index, a minimum value index, and a maximum value index.
  // However, all of these indices are optional. If an attribute does not have
  // a default/min/max value, or the value is all 0's, or the app was
  // configured not to include that constant through AppBuilder (in the case of
  // min/max values), then an index will not be generated.
  assert(READBITS(dataBit, EM_ZCL_ATTRIBUTE_DATA_MASK) && oneBitSet(dataBit));
  if (!READBITS(attribute->mask, dataBit)) {
    const static uint8_t zeros[EMBER_ZCL_ATTRIBUTE_MAX_SIZE] = { 0 };
    return zeros;
  }

  const size_t *lookupLocation = (emZclAttributeDefaultMinMaxLookupTable
                                  + attribute->defaultMinMaxLookupOffset);
  for (EmZclAttributeMask_t mask = EM_ZCL_ATTRIBUTE_DATA_DEFAULT;
       mask < dataBit;
       mask <<= 1) {
    if (READBITS(attribute->mask, mask)) {
      lookupLocation++;
    }
  }

  return emZclAttributeDefaultMinMaxData + *lookupLocation;
}

static bool isValueLocIndirect(const EmZclAttributeEntry_t *attribute)
{
  // For the CBOR encoder and decoder, in most cases, valueLoc is a pointer to
  // some data.  For strings, valueLoc is a pointer to a pointer to some data.
  // For commands, this is handled automatically by the structs and specs.  For
  // attributes, it must be done manually.
  switch (attribute->type) {
    case EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING:
    case EMBER_ZCLIP_TYPE_UINT16_LENGTH_STRING:
      return true;
    default:
      return false;
  }
}

static bool isLessThan(const uint8_t *dataA,
                       size_t dataALength,
                       const uint8_t *dataB,
                       size_t dataBLength,
                       const EmZclAttributeEntry_t *attribute)
{
  // Compares two integer type attribute values in buffers A and B and
  // returns true if dataB < dataA.
  // This function assumes a few things:-
  // - The data* arrays follow the native machine endianness.
  // - The data* arrays represent the same (ZCL) data types (as defined by *attribute).
  // - The data* arrays represent numeric data types.
  // - The data*Length values are not 0.
  // - The data*Length values are no greater than EMBER_ZCL_ATTRIBUTE_MAX_SIZE.

  // Use the largest size of dataALength, dataBALength or attributeSize.
  size_t size = attribute->size;
  if (dataALength > size) {
    size = dataALength;
  }
  if (dataBLength > size) {
    size = dataBLength;
  }
  if (size > EMBER_ZCL_ATTRIBUTE_MAX_SIZE) {
    return false; // Something has gone badly wrong.
  }

  // Allocate local storage for processing numeric integer attribute types-
  // (Buffer SIZE+1 allows us to add a sign-extension byte which is necessary
  // for absolute difference caculation to return the correct result).
  // TODO-? if input ptrs refer to buffers which can hold EMBER_ZCL_ATTRIBUTE_MAX_SIZE+1
  // we can do without this local storage.
  uint8_t dataABuffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE + 1];
  uint8_t dataBBuffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE + 1];
  uint8_t diffBuffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE + 1];

  ++size; // incr size to include one sign-extension byte.

  emZclSignExtendAttributeBuffer(dataABuffer,
                                 size,
                                 dataA,
                                 dataALength,
                                 attribute->type);
  emZclSignExtendAttributeBuffer(dataBBuffer,
                                 size,
                                 dataB,
                                 dataBLength,
                                 attribute->type);

  return emZclGetAbsDifference(dataABuffer,
                               dataBBuffer,
                               diffBuffer,
                               size);
}

// Encode response map of attributes, where key is "v" and value is attribute's
// value for a successful read, or key is "s" and value is failure status for
// unsuccessful read. [ZCLIP 16-07008-026 spec 3.6.4, 3.7.2]
// e.g. {0: {"v": 1}, 6: {"s": 0x86}}
static bool encodeAttributeResponseMap(CborState *state,
                                       EmberZclEndpointId_t endpointId,
                                       const EmZclAttributeEntry_t *attribute)
{
  uint8_t buffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE];
  EmberZclStatus_t status = emZclReadAttributeEntry(endpointId,
                                                    attribute,
                                                    buffer,
                                                    attribute->size);
  char key[2];
  uint8_t *value;
  uint8_t valueType;
  size_t valueSize;

  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    key[0] = 'v';
    value = buffer;
    valueType = emZclDirectBufferedZclipType(attribute->type);
    valueSize = attribute->size;
  } else {
    key[0] = 's';
    value = &status;
    valueType = EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER;
    valueSize = sizeof(status);
  }
  key[1] = '\0';

  return (emCborEncodeValue(state,
                            EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                            sizeof(attribute->attributeId),
                            (const uint8_t *)&attribute->attributeId)
          && emCborEncodeIndefiniteMap(state)
          && emCborEncodeValue(state,
                               EMBER_ZCLIP_TYPE_STRING,
                               0, // size - ignored
                               (const uint8_t *)&key)
          && emCborEncodeValue(state,
                               valueType,
                               valueSize,
                               (const uint8_t *)value)
          && emCborEncodeBreak(state));
}

static EmberZclStatus_t getAttributeIdsHandler(const EmZclContext_t *context,
                                               CborState *state,
                                               void *data)
{
  // If there are no queries, then we return an array of attribute ids.
  // Otherwise, we return a map from the filtered attribute ids to
  // their values.
  const EmZclAttributeQuery_t *query = &context->attributeQuery;
  bool array = (query->filterCount == 0);
  if (array) {
    emCborEncodeIndefiniteArray(state);
  } else {
    emCborEncodeIndefiniteMap(state);
  }

  FilterState filterState = { 0 };
  for (size_t i = 0; i < EM_ZCL_ATTRIBUTE_COUNT; i++) {
    const EmZclAttributeEntry_t *attribute = emZclAttributeTable + i;
    int32_t compare
      = emberZclCompareClusterSpec(attribute->clusterSpec,
                                   &context->clusterSpec);
    if (compare > 0) {
      break;
    } else if (compare == 0
               && emZclIsAttributeLocal(attribute)
               && filterAttribute(context, &filterState, attribute)
               && !(array
                    ? emCborEncodeValue(state,
                                        EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                                        sizeof(attribute->attributeId),
                                        (const uint8_t *)&attribute->attributeId)
                    : encodeAttributeResponseMap(state,
                                                 context->endpoint->endpointId,
                                                 attribute))) {
      return EMBER_ZCL_STATUS_FAILURE;
    }
  }
  if (emCborEncodeBreak(state)) {
    return EMBER_ZCL_STATUS_SUCCESS;
  }
  return EMBER_ZCL_STATUS_FAILURE;
}

static EmberZclStatus_t getAttributeHandler(const EmZclContext_t *context,
                                            CborState *state,
                                            void *data)
{
  if (emCborEncodeIndefiniteMap(state)
      && encodeAttributeResponseMap(state,
                                    context->endpoint->endpointId,
                                    context->attribute)
      && emCborEncodeBreak(state)) {
    return EMBER_ZCL_STATUS_SUCCESS;
  }
  return EMBER_ZCL_STATUS_FAILURE;
}

static EmberZclStatus_t updateAttributeHandler(const EmZclContext_t *context,
                                               CborState *state,
                                               void *data)
{
  // If Undivided Write mode is selected attributes are only updated if the
  // prepass check (i.e. a check of all attributes for write errors) completes
  // with no errors.

  CborState inState;
  uint8_t numberOfPasses = (context->attributeQuery.undivided) ? 2 : 1;
  bool precheckFailed = false;
  EmberZclStatus_t result = EMBER_ZCL_STATUS_SUCCESS;

  emCborEncodeIndefiniteMap(state);

  for (uint8_t i = 0; i < numberOfPasses && !precheckFailed; ++i) {
    bool prechecking = ((numberOfPasses == 2) && (i == 0));

    emCborDecodeStart(&inState, context->payload, context->payloadLength); // Reset cbor decode state on each loop.
    if (emCborDecodeMap(&inState)) {
      EmberZclAttributeId_t attributeId;
      while (emCborDecodeValue(&inState,
                               EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                               sizeof(attributeId),
                               (uint8_t *)&attributeId)) {
        EmberZclStatus_t status = EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;

        if ((context->attribute == NULL)
            || (context->attribute->attributeId == attributeId)) {
          const EmZclAttributeEntry_t *attribute
            = ((context->attribute == NULL)
               ? emZclFindAttribute(&context->clusterSpec,
                                    attributeId,
                                    false)  // exclude remote
               : context->attribute);

          if (attribute == NULL) {
            status = EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
            if (!emCborDecodeSkipValue(&inState)) {
              return EMBER_ZCL_STATUS_FAILURE;
            }
          } else if (!emZclIsAttributeWritable(attribute)) {
            status = EMBER_ZCL_STATUS_READ_ONLY;
            if (!emCborDecodeSkipValue(&inState)) {
              return EMBER_ZCL_STATUS_FAILURE;
            }
          } else {
            uint8_t buffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE];
            if (!emCborDecodeValue(&inState,
                                   emZclDirectBufferedZclipType(attribute->type),
                                   attribute->size,
                                   buffer)) {
              status = emZclCborValueReadStatusToEmberStatus(inState.readStatus);
            } else {
              status = emZclWriteAttributeEntry(context->endpoint->endpointId,
                                                attribute,
                                                buffer,
                                                attribute->size,
                                                !prechecking); // we don't update the attribute on the precheck.
            }
          }
        }

        // Encode any attribute write fails into the response map.
        if (status != EMBER_ZCL_STATUS_SUCCESS) {
          result = status;
          if (!emCborEncodeMapEntry(state,
                                    attributeId,
                                    EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                                    sizeof(status),
                                    &status)) {
            return EMBER_ZCL_STATUS_FAILURE;
          }
        }

        if ((prechecking) && (status != EMBER_ZCL_STATUS_SUCCESS)) {
          precheckFailed = true;  // Set failed flag but continue looping for other attribute writes.
        }
      } // while
    }
  } // for

  if (!emCborEncodeBreak(state)) {
    return EMBER_ZCL_STATUS_FAILURE;
  }
  if (precheckFailed) {
    return EMBER_ZCL_STATUS_NULL;  // no specific ZCL_STATUS enum for pre check failed so return NULL.
  }
  return result;
}

static bool filterAttribute(const EmZclContext_t *context,
                            FilterState *state,
                            const EmZclAttributeEntry_t *attribute)
{
  EmberZclAttributeId_t attributeId = attribute->attributeId;
  for (size_t i = 0; i < context->attributeQuery.filterCount; i++) {
    const EmZclAttributeQueryFilter_t *filter = &context->attributeQuery.filters[i];
    switch (filter->type) {
      case EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_ID:
        if (attributeId == filter->data.attributeId) {
          return true;
        }
        break;

      case EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_COUNT:
        if (attributeId >= filter->data.countData.start) {
          if (filter->data.countData.count > state->count
              && !READBIT(state->usedCounts, i)) {
            state->count = filter->data.countData.count;
          }
          SETBIT(state->usedCounts, i);
        }
        break;

      case EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_RANGE:
        if (attributeId >= filter->data.rangeData.start
            && attributeId <= filter->data.rangeData.end) {
          return true;
        }
        break;

      case EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_WILDCARD:
        return true;

      default:
        assert(false);
    }
  }

  bool ret = (context->attributeQuery.filterCount == 0 || state->count > 0);
  if (state->count > 0) {
    state->count--;
  }
  return ret;
}

EmberStatus emberZclSendAttributeRead(const EmberZclDestination_t *destination,
                                      const EmberZclClusterSpec_t *clusterSpec,
                                      const EmberZclAttributeId_t *attributeIds,
                                      size_t attributeIdsCount,
                                      const EmberZclReadAttributeResponseHandler handler)
{
  // The size of this array is the maximum number of filter range data
  // structures that could possibly be encoded into a URI string of length 64
  // that has been optimized with range formatting. This helps us protect from
  // writing off the end of the filterRangeData array in the second for loop
  // below.
  EmZclAttributeQueryFilterRangeData_t filterRangeData[19];
  EmZclAttributeQueryFilterRangeData_t *filterRangeDatum = filterRangeData;
  for (size_t i = 0; i < COUNTOF(filterRangeData); i++) {
    filterRangeData[i].start = filterRangeData[i].end = EMBER_ZCL_ATTRIBUTE_NULL;
  }
  for (size_t i = 0; i < attributeIdsCount; i++) {
    EmberZclAttributeId_t attributeId = attributeIds[i];
    if (attributeId == filterRangeDatum->end + 1) {
      filterRangeDatum->end = attributeId;
    } else {
      if (filterRangeDatum->start != EMBER_ZCL_ATTRIBUTE_NULL) {
        filterRangeDatum++;
        if (filterRangeDatum - filterRangeData > sizeof(filterRangeData)) {
          return EMBER_BAD_ARGUMENT;
        }
      }
      filterRangeDatum->start = filterRangeDatum->end = attributeId;
    }
  }

  size_t filterRangeDataCount = filterRangeDatum - filterRangeData + 1;
  uint8_t uri[MAX_ATTRIBUTE_URI_LENGTH];
  uint8_t *uriFinger = uri;
  uriFinger += emZclAttributeToUriPath(&destination->application,
                                       clusterSpec,
                                       uriFinger);
  *uriFinger++ = '?';
  *uriFinger++ = 'f';
  *uriFinger++ = '=';
  for (size_t i = 0; i < filterRangeDataCount; i++) {
    uint8_t buffer[10];
    uint8_t *bufferFinger = buffer;
    if (i != 0) {
      *bufferFinger++ = ',';
    }
    bufferFinger += emZclIntToHexString(filterRangeData[i].start,
                                        sizeof(EmberZclAttributeId_t),
                                        bufferFinger);
    if (filterRangeData[i].start != filterRangeData[i].end) {
      *bufferFinger++ = '-';
      bufferFinger += emZclIntToHexString(filterRangeData[i].end,
                                          sizeof(EmberZclAttributeId_t),
                                          bufferFinger);
    }

    size_t bufferLength = bufferFinger - buffer;
    if ((uriFinger + bufferLength + 1) - uri > sizeof(uri)) { // +1 for the nul
      return EMBER_BAD_ARGUMENT;
    }
    MEMMOVE(uriFinger, buffer, bufferLength);
    uriFinger += bufferLength;
    *uriFinger = '\0';
  }

  Response response = {
    .context = {
      .code = EMBER_COAP_CODE_EMPTY, // filled in when the response arrives
      .groupId
        = ((destination->application.type
            == EMBER_ZCL_APPLICATION_DESTINATION_TYPE_GROUP)
           ? destination->application.data.groupId
           : EMBER_ZCL_GROUP_NULL),
      .endpointId
        = ((destination->application.type
            == EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT)
           ? destination->application.data.endpointId
           : EMBER_ZCL_ENDPOINT_NULL),
      .clusterSpec = clusterSpec,
      .attributeId = EMBER_ZCL_ATTRIBUTE_NULL, // filled in when the response arrives
      .status = EMBER_ZCL_STATUS_NULL, // filled in when the response arrives
      .state = NULL, // filled in when the response arrives
    },
    .readHandler = handler,
    .writeHandler = NULL, // unused
  };

  return emZclSend(&destination->network,
                   EMBER_COAP_CODE_GET,
                   uri,
                   NULL, // payload
                   0,    // payload length
                   (handler == NULL ? NULL : readWriteResponseHandler),
                   &response,
                   sizeof(Response));
}

EmberStatus emberZclSendAttributeWrite(const EmberZclDestination_t *destination,
                                       const EmberZclClusterSpec_t *clusterSpec,
                                       const EmberZclAttributeWriteData_t *attributeWriteData,
                                       size_t attributeWriteDataCount,
                                       const EmberZclWriteAttributeResponseHandler handler)
{
  CborState state;
  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  emCborEncodeIndefiniteMapStart(&state, buffer, sizeof(buffer));
  for (size_t i = 0; i < attributeWriteDataCount; i++) {
    const EmZclAttributeEntry_t *attribute
      = emZclFindAttribute(clusterSpec,
                           attributeWriteData[i].attributeId,
                           true); // include remote
    if (attribute == NULL) {
      return EMBER_BAD_ARGUMENT;
    } else if (!emCborEncodeMapEntry(&state,
                                     attribute->attributeId,
                                     attribute->type,
                                     attribute->size,
                                     (isValueLocIndirect(attribute)
                                      ? (const uint8_t *)&attributeWriteData[i].buffer
                                      : attributeWriteData[i].buffer))) {
      return EMBER_ERR_FATAL;
    }
  }
  emCborEncodeBreak(&state);

  uint8_t uriPath[EMBER_ZCL_URI_PATH_MAX_LENGTH];
  emZclAttributeToUriPath(&destination->application, clusterSpec, uriPath);

  Response response = {
    .context = {
      .code = EMBER_COAP_CODE_EMPTY, // filled in when the response arrives
      .groupId
        = ((destination->application.type
            == EMBER_ZCL_APPLICATION_DESTINATION_TYPE_GROUP)
           ? destination->application.data.groupId
           : EMBER_ZCL_GROUP_NULL),
      .endpointId
        = ((destination->application.type
            == EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT)
           ? destination->application.data.endpointId
           : EMBER_ZCL_ENDPOINT_NULL),
      .clusterSpec = clusterSpec,
      .attributeId = EMBER_ZCL_ATTRIBUTE_NULL, // filled in when the response arrives
      .status = EMBER_ZCL_STATUS_NULL, // filled in when the response arrives
      .state = NULL, // filled in when the response arrives
    },
    .readHandler = NULL, // unused
    .writeHandler = handler,
  };

  return emZclSend(&destination->network,
                   EMBER_COAP_CODE_POST,
                   uriPath,
                   buffer,
                   emCborEncodeSize(&state),
                   (handler == NULL ? NULL : readWriteResponseHandler),
                   &response,
                   sizeof(Response));
}

static void readWriteResponseHandler(EmberCoapStatus coapStatus,
                                     EmberCoapCode code,
                                     EmberCoapReadOptions *options,
                                     uint8_t *payload,
                                     uint16_t payloadLength,
                                     EmberCoapResponseInfo *info)
{
  // We should only be here if the application specified a handler.
  assert(info->applicationDataLength == sizeof(Response));
  const Response *response = info->applicationData;
  bool isRead = (*response->readHandler != NULL);
  bool isWrite = (*response->writeHandler != NULL);
  assert(isRead != isWrite);
  EmberZclMessageStatus_t status = (EmberZclMessageStatus_t) coapStatus;

  emZclCoapStatusHandler(coapStatus, info);
  ((Response *)response)->context.code = code;

  // TODO: What should happen if the overall payload is missing or malformed?
  // Note that this is a separate issue from how missing or malformed responses
  // from the individual endpoints should be handled.
  if (status == EMBER_ZCL_MESSAGE_STATUS_COAP_RESPONSE) {
    // Note- all coap rsp codes are now checked here.
    CborState state;
    ((Response *)response)->context.state = &state;
    emCborDecodeStart(&state, payload, payloadLength);
    if (response->context.groupId == EMBER_ZCL_GROUP_NULL) {
      (isRead ? handleRead : handleWrite)(status, response);
      return;
    } else if (emCborDecodeMap(&state)) {
      while (emCborDecodeValue(&state,
                               EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                               sizeof(response->context.endpointId),
                               (uint8_t *)&response->context.endpointId)) {
        (isRead ? handleRead : handleWrite)(status, response);
      }
      return;
    }
  }

  if (isRead) {
    (*response->readHandler)(status, &response->context, NULL, 0);
  } else {
    (*response->writeHandler)(status, &response->context);
  }
}

static void handleRead(EmberZclMessageStatus_t status,
                       const Response *response)
{
  // TODO: If we expect an attribute but it is missing, or it is present but
  // malformed, would should we do?
  if (emCborDecodeMap(response->context.state)) {
    while (emCborDecodeValue(response->context.state,
                             EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                             sizeof(response->context.attributeId),
                             (uint8_t *)&response->context.attributeId)) {
      const EmZclAttributeEntry_t *attribute
        = emZclFindAttribute(response->context.clusterSpec,
                             response->context.attributeId,
                             true); // include remote
      if (attribute == NULL) {
        emCborDecodeMap(response->context.state);
        emCborDecodeSkipValue(response->context.state); // key
        emCborDecodeSkipValue(response->context.state); // value
        emCborDecodeSkipValue(response->context.state); // break
      } else {
        uint8_t buffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE];
        uint8_t key[2]; // 'v' or 's' plus a NUL
        if (emCborDecodeMap(response->context.state)
            && emCborDecodeValue(response->context.state,
                                 EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING,
                                 sizeof(key),
                                 key)
            && emCborDecodeValue(response->context.state,
                                 emZclDirectBufferedZclipType(attribute->type),
                                 attribute->size,
                                 buffer)) {
          ((Response *)response)->context.status = (key[0] == 'v'
                                                    ? EMBER_ZCL_STATUS_SUCCESS
                                                    : buffer[0]);
          (*response->readHandler)(status,
                                   &response->context,
                                   buffer,
                                   attribute->size);
        }
        emCborDecodeSkipValue(response->context.state); // break
      }
    }
  }
  (*response->readHandler)(status, &response->context, NULL, 0);
}

static void handleWrite(EmberZclMessageStatus_t status,
                        const Response *response)
{
  // Handle attribute write response.

  (*response->writeHandler)(status, &response->context);
}

static EmberZclStatus_t writeAttribute(EmberZclEndpointIndex_t index,
                                       EmberZclEndpointId_t endpointId,
                                       const EmZclAttributeEntry_t *attribute,
                                       const void *data,
                                       size_t dataLength)
{
  EmberZclStatus_t status = EMBER_ZCL_STATUS_SUCCESS;
  EmZclAttributeMask_t storageType
    = READBITS(attribute->mask, EM_ZCL_ATTRIBUTE_STORAGE_TYPE_MASK);

  assert(emZclIsAttributeLocal(attribute));

  switch (storageType) {
    case EM_ZCL_ATTRIBUTE_STORAGE_TYPE_EXTERNAL:
      status = emberZclWriteExternalAttributeCallback(endpointId,
                                                      attribute->clusterSpec,
                                                      attribute->attributeId,
                                                      data,
                                                      dataLength);
      break;

    case EM_ZCL_ATTRIBUTE_STORAGE_TYPE_RAM:
      MEMMOVE(attributeDataLocation(index, attribute), data, dataLength);
      break;

    default:
      assert(false);
  }

  return status;
}

static void callPostAttributeChange(EmberZclEndpointId_t endpointId,
                                    const EmZclAttributeEntry_t *attribute,
                                    const void *data,
                                    size_t dataLength)
{
  if (emZclIsAttributeSingleton(attribute)) {
    for (size_t i = 0; i < emZclEndpointCount; i++) {
      if (emZclEndpointHasCluster(emZclEndpointTable[i].endpointId,
                                  attribute->clusterSpec)) {
        emZclPostAttributeChange(emZclEndpointTable[i].endpointId,
                                 attribute->clusterSpec,
                                 attribute->attributeId,
                                 data,
                                 dataLength);
      }
    }
  } else {
    emZclPostAttributeChange(endpointId,
                             attribute->clusterSpec,
                             attribute->attributeId,
                             data,
                             dataLength);
  }
}

static bool callPreAttributeChange(EmberZclEndpointId_t endpointId,
                                   const EmZclAttributeEntry_t *attribute,
                                   const void *data,
                                   size_t dataLength)
{
  if (emZclIsAttributeSingleton(attribute)) {
    for (size_t i = 0; i < emZclEndpointCount; i++) {
      if (emZclEndpointHasCluster(emZclEndpointTable[i].endpointId,
                                  attribute->clusterSpec)
          && !emZclPreAttributeChange(emZclEndpointTable[i].endpointId,
                                      attribute->clusterSpec,
                                      attribute->attributeId,
                                      data,
                                      dataLength)) {
        return false;
      }
    }
    return true;
  } else {
    return emZclPreAttributeChange(endpointId,
                                   attribute->clusterSpec,
                                   attribute->attributeId,
                                   data,
                                   dataLength);
  }
}

static size_t tokenize(const EmZclContext_t *context,
                       void *skipData,
                       uint8_t depth,
                       const uint8_t **tokens,
                       size_t *tokenLengths)
{
  uint8_t skipLength = strlen((const char *)skipData);
  const uint8_t *bytes = context->uriQuery[depth] + skipLength;
  size_t bytesLength = context->uriQueryLength[depth] - skipLength;
  size_t count = 0;
  const uint8_t *next = NULL, *end = bytes + bytesLength;

  for (; bytes < end; bytes = next + 1) {
    next = memchr(bytes, ',', end - bytes);
    if (next == NULL) {
      next = end;
    } else if (next == bytes || next == end - 1) {
      return 0; // 0 length strings are no good.
    }

    tokens[count] = bytes;
    tokenLengths[count] = next - bytes;
    count++;
  }

  return count;
}

static void convertBufferToTwosComplement(uint8_t *buffer, size_t size)
{
  // Converts the input buffer to 2S complement format.

  // Invert all bytes in buffer.
  for (uint8_t i = 0; i < size; i++) {
    buffer[i] = ~buffer[i];
  }

  // Add +1 to number in buffer.
  for (uint8_t i = 0; i < size; i++) {
    if (buffer[i] < 0xFF) {
      buffer[i] += 1;
      break;
    } else {
      buffer[i] = 0;
    }
  }
}

// .../a?f=
bool emZclAttributeUriQueryFilterParse(EmZclContext_t *context,
                                       void *data,
                                       uint8_t depth)
{
  const uint8_t *tokens[MAX_URI_QUERY_SEGMENTS];
  size_t tokenLengths[MAX_URI_QUERY_SEGMENTS];
  size_t tokenCount = tokenize(context, data, depth, tokens, tokenLengths);
  if (tokenCount == 0) {
    return false;
  }

  for (size_t i = 0; i < tokenCount; i++) {
    EmZclAttributeQueryFilter_t *filter
      = &context->attributeQuery.filters[context->attributeQuery.filterCount++];
    if (tokenLengths[i] == 1 && tokens[i][0] == '*') {
      // f=*
      filter->type = EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_WILDCARD;
    } else {
      const uint8_t *operator = NULL;
      const uint8_t *now = tokens[i];
      const uint8_t *next = now + tokenLengths[i];
      uintmax_t first, second, length;
      if (((operator = memchr(now, '-', tokenLengths[i])) != NULL
           || (operator = memchr(now, '+', tokenLengths[i])) != NULL)
          && (length = operator - now) > 0
          && length <= sizeof(EmberZclAttributeId_t) * 2 // nibbles
          && emZclHexStringToInt(now, length, &first)
          && (length = next - operator - 1) > 0
          && length <= sizeof(EmberZclAttributeId_t) * 2 // nibbles
          && emZclHexStringToInt(operator + 1, length, &second)) {
        // f=1-2
        // f=3+4
        if (*operator == '-') {
          filter->type = EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_RANGE;
          filter->data.rangeData.start = first;
          filter->data.rangeData.end = second;
          if (filter->data.rangeData.end <= filter->data.rangeData.start) {
            return false;
          }
        } else {
          filter->type = EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_COUNT;
          filter->data.countData.start = first;
          filter->data.countData.count = second;
        }
      } else if (tokenLengths[i] <= sizeof(EmberZclAttributeId_t) * 2
                 && emZclHexStringToInt(now, tokenLengths[i], &first)) {
        // f=5
        filter->type = EM_ZCL_ATTRIBUTE_QUERY_FILTER_TYPE_ID;
        filter->data.attributeId = first;
      } else {
        return false;
      }
    }
  }

  return true;
}

// ...a/?u
bool emZclAttributeUriQueryUndividedParse(EmZclContext_t *context,
                                          void *data,
                                          uint8_t depth)
{
  // Only accept 'u'.
  return (context->attributeQuery.undivided
            = (context->uriQueryLength[depth] == 1));
}

// zcl/[eg]/XX/<cluster>/a:
//   GET:
//     w/ query: read multiple attributes.
//     w/o query: list attributes in cluster.
//   POST:
//     w/ query: update attributes undivided.
//     w/o query: write multiple attributes.
//   OTHER: not allowed.
void emZclUriClusterAttributeHandler(EmZclContext_t *context)
{
  CborState state;
  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  EmberZclStatus_t status;
  emCborEncodeStart(&state, buffer, sizeof(buffer));

  switch (context->code) {
    case EMBER_COAP_CODE_GET:
      status = emZclMultiEndpointDispatch(context,
                                          getAttributeIdsHandler,
                                          &state,
                                          NULL);
      switch (status) {
        case EMBER_ZCL_STATUS_SUCCESS:
          emZclRespond205ContentCborState(&state);
          break;
        case EMBER_ZCL_STATUS_FAILURE:
          emZclRespond500InternalServerError();
          break;
        default:
          emZclRespond404NotFound();
          break;
      }
      break;

    case EMBER_COAP_CODE_POST:
      status = emZclMultiEndpointDispatch(context,
                                          updateAttributeHandler,
                                          &state,
                                          NULL);
      switch (status) {
        case EMBER_ZCL_STATUS_SUCCESS:
          emZclRespondCborPayload(EMBER_COAP_CODE_204_CHANGED, NULL, 0); // All attr writes ok (no rsp payload).
          break;
        case EMBER_ZCL_STATUS_FAILURE:
          emZclRespond500InternalServerError();
          break;
        case EMBER_ZCL_STATUS_NULL:
          emZclRespond412PreconditionFailedCborState(&state); // Response payload is a map of all failed write prechecks (attId/status).
          break;
        default:
          emZclRespond204ChangedCborState(&state); // Response payload is a map of all failed writes (attId/status).
          break;
      }
      break;

    default:
      assert(false);
  }
}

// zcl/[eg]/XX/<cluster>/a/XXXX:
//   GET: read one attribute.
//   PUT: write one attribute.
//   OTHER: not allowed.
void emZclUriClusterAttributeIdHandler(EmZclContext_t *context)
{
  CborState state;
  uint8_t buffer[EM_ZCL_MAX_PAYLOAD_SIZE];
  EmberZclStatus_t status;
  emCborEncodeStart(&state, buffer, sizeof(buffer));

  switch (context->code) {
    case EMBER_COAP_CODE_GET:
      status = emZclMultiEndpointDispatch(context,
                                          getAttributeHandler,
                                          &state,
                                          NULL);
      switch (status) {
        case EMBER_ZCL_STATUS_SUCCESS:
          emZclRespond205ContentCborState(&state);
          break;
        default:
          emZclRespond500InternalServerError();
          break;
      }
      break;

    case EMBER_COAP_CODE_PUT:
      status = emZclMultiEndpointDispatch(context,
                                          updateAttributeHandler,
                                          &state,
                                          NULL);
      switch (status) {
        case EMBER_ZCL_STATUS_SUCCESS:
          emZclRespondWithStatus(EMBER_COAP_CODE_204_CHANGED, status);
          break;
        case EMBER_ZCL_STATUS_FAILURE:
          emZclRespond500InternalServerError();
          break;
        default:
          emZclRespondCborState(EMBER_COAP_CODE_400_BAD_REQUEST, &state);
          break;
      }
      break;

    default:
      assert(false);
  }
}

bool emZclGetAbsDifference(uint8_t *bufferA,
                           uint8_t *bufferB,
                           uint8_t *diffBuffer,
                           size_t size)
{
  // -Assumes that bufferA and bufferB represent valid sign-extended numeric
  // integer type attribute values.
  // -The resulting abs (i.e. +ve) difference is placed in the diffBuffer.
  // -Returns TRUE if the difference is negative.

  // Convert the bufferB value to 2S complement (i.e. -bufferB).
  convertBufferToTwosComplement(bufferB, size);

  // Do a bytewise add of bufferA to -bufferB. The result in the
  // diffBuffer is the signed difference of bufferA and bufferB.
  int8u carry = 0;
  for (size_t i = 0; i < size; i++) {
    uint16_t tmp = (bufferA[i] + bufferB[i] + carry);
    if (tmp >= 256) {
      diffBuffer[i] = tmp - 256;
      carry = 1;
    } else {
      diffBuffer[i] = tmp;
      carry = 0;
    }
  }

  // Check if the value in the diffBuffer is negative.
  bool diffIsNegative = (diffBuffer[size - 1] & 0x80);

  if (diffIsNegative) {
    // Convert the diff buffer value to a positive number by running
    // a 2S complement on it.
    convertBufferToTwosComplement(diffBuffer, size);
  }

  return diffIsNegative;
}

void emZclSignExtendAttributeBuffer(uint8_t *outBuffer,
                                    size_t outBufferLength,
                                    const uint8_t *inBuffer,
                                    size_t inBufferLength,
                                    uint8_t attributeType)
{
  if ((attributeType != EMBER_ZCLIP_TYPE_INTEGER)
      && (attributeType != EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER)
      && (attributeType != EMBER_ZCLIP_TYPE_BOOLEAN)) {
    assert(false);
    return;
  }
  if (inBufferLength >= outBufferLength) {
    assert(false); // outBuffer length must be greater.
    return;
  }

  // Copy input buffer bytes to output buffer (convert to LE order).
  #if BIGENDIAN_CPU
  emberReverseMemCopy(outBuffer, inBuffer, inBufferLength);
  #else
  MEMCOPY(outBuffer, inBuffer, inBufferLength);
  #endif

  // Set all the remaining high end bytes of the outputBuffer to
  // the sign-extension byte value.
  uint8_t signExtensionByteValue = 0x00;  // default sign-extension value for positive numbers.
  uint8_t MSbyteOffset = inBufferLength - 1;
  if ((attributeType == EMBER_ZCLIP_TYPE_INTEGER)
      && (outBuffer[MSbyteOffset] & 0x80)) {
    signExtensionByteValue = 0xFF; // Sign bit is set so use sign-extension value for negative number.
  }
  MEMSET(&outBuffer[MSbyteOffset + 1],
         signExtensionByteValue,
         outBufferLength - inBufferLength);
}
