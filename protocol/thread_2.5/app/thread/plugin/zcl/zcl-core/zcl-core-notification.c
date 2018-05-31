// Copyright 2016 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include "thread-bookkeeping.h"
#include "zcl-core.h"

typedef struct {
  EmberZclReportingConfigurationId_t reportingConfigurationId;
  uint8_t uri[EMBER_ZCL_URI_MAX_LENGTH];
  uint32_t timestamp;
  uint8_t bindingId;
} Notification_t;
#define EMBER_ZCLIP_STRUCT Notification_t
static const ZclipStructSpec notificationSpec[] = {
  EMBER_ZCLIP_OBJECT(sizeof(EMBER_ZCLIP_STRUCT),
                     4,     // fieldCount
                     NULL), // names
  EMBER_ZCLIP_FIELD_NAMED_MANDATORY(EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, uri, "u"),
  EMBER_ZCLIP_FIELD_NAMED_MANDATORY(EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, reportingConfigurationId, "r"),
  EMBER_ZCLIP_FIELD_NAMED_MANDATORY(EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, bindingId, "b"),
  EMBER_ZCLIP_FIELD_NAMED(EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, timestamp, "t"),
};
#undef EMBER_ZCLIP_STRUCT

static bool findAttributeMap(CborState *state);
static bool getNextAttributeKeyValue(CborState *state,
                                     const EmberZclClusterSpec_t *clusterSpec,
                                     const EmZclAttributeEntry_t **attribute,
                                     uint8_t *buffer);
static EmberStatus notify(const EmZclContext_t *context, CborState *state, void *data);

// zcl/e/XX/<cluster>/n:
// zcl/g/XXXX/<cluster>/n:
//   POST: report notification.
//   OTHER: not allowed.
void emZclUriClusterNotificationHandler(EmZclContext_t *context)
{
  Notification_t notification = {
    .reportingConfigurationId = EMBER_ZCL_REPORTING_CONFIGURATION_NULL,
    .uri = { 0 },
    .timestamp = 0,
    .bindingId = 0,
  };
  if (!emCborDecodeOneStruct(context->payload,
                             context->payloadLength,
                             notificationSpec,
                             &notification)) {
    emZclRespond400BadRequest();
    return;
  }

  EmberZclClusterSpec_t clusterSpec;
  emberZclReverseClusterSpec(&context->clusterSpec, &clusterSpec);
  uint8_t buffer[EMBER_ZCL_ATTRIBUTE_MAX_SIZE];
  EmberZclNotificationContext_t notificationContext = {
    .remoteAddress = context->info->remoteAddress,
    .sourceEndpointId = EMBER_ZCL_ENDPOINT_NULL, // filled in later
    .sourceReportingConfigurationId = notification.reportingConfigurationId,
    .sourceTimestamp = notification.timestamp,
    .groupId = context->groupId,
    .endpointId = context->endpoint->endpointId,
    .clusterSpec = &clusterSpec,
    .attributeId = EMBER_ZCL_ATTRIBUTE_NULL, // filled in later
    .buffer = buffer,
    .bufferLength = 0, // filled in later
  };

  // TODO: This verifies the URI up to the cluster.  It does not verify that
  // the URI path ends in /a.
  EmberZclBindingEntry_t source;
  if (emZclUriToBindingEntry(notification.uri, &source, true)
      && (source.destination.application.type
          == EMBER_ZCL_APPLICATION_DESTINATION_TYPE_ENDPOINT)
      && emberZclAreClusterSpecsEqual(&clusterSpec, &source.clusterSpec)) {
    notificationContext.sourceEndpointId
      = source.destination.application.data.endpointId;
  } else {
    emZclRespond400BadRequest();
    return;
  }

  CborState state;
  emCborDecodeStart(&state, context->payload, context->payloadLength);
  if (findAttributeMap(&state)) {
    const EmZclAttributeEntry_t *attribute;
    notificationContext.buffer = buffer;
    while (getNextAttributeKeyValue(&state,
                                    notificationContext.clusterSpec,
                                    &attribute,
                                    buffer)) {
      notificationContext.attributeId = attribute->attributeId;
      notificationContext.bufferLength = attribute->size;
      emZclMultiEndpointDispatch(context,
                                 notify,
                                 &state,
                                 &notificationContext);
    }
  }
}

static bool findAttributeMap(CborState *state)
{
  if (emCborDecodeMap(state)) {
    while (true) {
      uint8_t type = emCborDecodePeek(state, NULL);
      if (type == CBOR_TEXT) {
        uint8_t key[2]; // "a" plus a NUL
        if (emCborDecodeValue(state,
                              EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING,
                              sizeof(key),
                              key)
            && strcmp((const char *)key, "a") == 0) {
          if (emCborDecodeMap(state)) {
            return true;
          } else {
            break;
          }
        } else if (!emCborDecodeSkipValue(state)) {
          break;
        }
      } else if (type == CBOR_BREAK) {
        break;
      } else if (!emCborDecodeSkipValue(state)
                 || !emCborDecodeSkipValue(state)) {
        break;
      }
    }
  }
  emZclRespond400BadRequest();
  return false;
}

// TODO: This could probably be used in the over-the-air write handler.
static bool getNextAttributeKeyValue(CborState *state,
                                     const EmberZclClusterSpec_t *clusterSpec,
                                     const EmZclAttributeEntry_t **attribute,
                                     uint8_t *buffer)
{
  while (true) {
    uint8_t type = emCborDecodePeek(state, NULL);
    if (type == CBOR_UNSIGNED) {
      EmberZclAttributeId_t attributeId;
      if (!emCborDecodeValue(state,
                             EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                             sizeof(attributeId),
                             (uint8_t *)&attributeId)) {
        break;
      }

      *attribute = emZclFindAttribute(clusterSpec,
                                      attributeId,
                                      true); // include remote
      if (*attribute != NULL
          && emCborDecodeValue(state,
                               (*attribute)->type,
                               (*attribute)->size,
                               buffer)) {
        return true;
      } else if (!emCborDecodeSkipValue(state)) {
        break;
      }
    } else if (type == CBOR_BREAK) {
      emZclRespond204Changed();
      return false;
    } else if (!emCborDecodeSkipValue(state)
               || !emCborDecodeSkipValue(state)) {
      break;
    }
  }

  emZclRespond400BadRequest();
  return false;
}

static EmberStatus notify(const EmZclContext_t *context, CborState *state, void *data)
{
  EmberZclNotificationContext_t *notificationContext = data;
  notificationContext->endpointId = context->endpoint->endpointId;
  emZclNotification(notificationContext,
                    notificationContext->clusterSpec,
                    notificationContext->attributeId,
                    notificationContext->buffer,
                    notificationContext->bufferLength);
  return EMBER_ZCL_STATUS_SUCCESS;
}
