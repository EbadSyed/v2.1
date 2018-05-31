// Copyright 2017 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_BUFFER_MANAGEMENT
#include EMBER_AF_API_EVENT_QUEUE
#include EMBER_AF_API_HAL
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include "zcl-core.h"
#include "zcl-core-well-known.h"

// TODO: Use an appropriate timeout.
#define DISCOVERY_TIMEOUT_MS   1500

// When memory for this structure is allocated, enough space is reserved at the
// end to also hold the URI (with NUL terminator), payload, and application
// data.
typedef struct {
  Event event;
  EmberZclCoapEndpoint_t destination;
  EmberIpv6Address remoteAddress;
  EmberCoapCode code;
  EmberCoapResponseHandler handler;
  size_t uriPathLength; // includes NUL terminator
  uint16_t payloadLength;
  uint16_t applicationDataLength;
  //uint8_t uriPath[uriPathLength];
  //uint8_t payload[payloadLength];
  //uint8_t applicationData[applicationDataLength];
} MessageEvent;

static bool discoverAddress(const EmberZclCoapEndpoint_t *destination);
static void uidDiscoveryResponseHandler(EmberCoapStatus status,
                                        EmberCoapCode code,
                                        EmberCoapReadOptions *options,
                                        uint8_t *payload,
                                        uint16_t payloadLength,
                                        EmberCoapResponseInfo *info);
static bool validateUidDiscoveryResponse(const uint8_t *payload,
                                         uint16_t payloadLength,
                                         EmberZclUid_t *uid);
static void defaultCoapResponseHandler(EmberCoapStatus status,
                                       EmberCoapCode code,
                                       EmberCoapReadOptions *options,
                                       uint8_t *payload,
                                       uint16_t payloadLength,
                                       EmberCoapResponseInfo *info);
static void eventHandler(MessageEvent *event);
static void eventMarker(MessageEvent *event);
static bool destinationPredicate(MessageEvent *event,
                                 const EmberZclUid_t *uid);

extern EventQueue emAppEventQueue;
static EventActions actions = {
  .queue = &emAppEventQueue,
  .handler = (void (*)(struct Event_s *))eventHandler,
  .marker = (void (*)(struct Event_s *))eventMarker,
  .name = "zcl core messaging"
};

#ifdef EMBER_AF_PLUGIN_DTLS_CLI
extern bool secureDtlsSession;
extern uint8_t secureDtlsSessionId;
#endif

static const EmberCoapOption cborOptions[] = {
  { EMBER_COAP_OPTION_CONTENT_FORMAT, NULL, 1, EMBER_COAP_CONTENT_FORMAT_CBOR, },
};

EmberStatus emZclSend(const EmberZclCoapEndpoint_t *destination,
                      EmberCoapCode code,
                      const uint8_t *uriPath,
                      const uint8_t *payload,
                      uint16_t payloadLength,
                      EmberCoapResponseHandler handler,
                      void *applicationData,
                      uint16_t applicationDataLength)
{
  EmberIpv6Address remoteAddressBuf;
  const EmberIpv6Address *remoteAddress;

  if (destination->flags & EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG) {
    remoteAddress = &destination->address;
  } else if (emZclCacheGet(&destination->uid, &remoteAddressBuf)) {
    remoteAddress = &remoteAddressBuf;
  } else if (discoverAddress(destination)) {
    size_t uriPathLength = strlen((const char *)uriPath) + 1; // include the NUL
    Buffer buffer = emAllocateBuffer(sizeof(MessageEvent)
                                     + uriPathLength
                                     + payloadLength
                                     + applicationDataLength);
    if (buffer == NULL_BUFFER) {
      return EMBER_NO_BUFFERS;
    }
    uint8_t *finger = emGetBufferPointer(buffer);
    MessageEvent *event = (MessageEvent *)(void *)finger;
    event->event.actions = &actions;
    event->event.next = NULL;
    event->event.timeToExecute = 0;
    event->destination = *destination;
    MEMSET(&event->remoteAddress, 0, sizeof(EmberIpv6Address)); // filled in later
    event->code = code;
    event->handler = handler;
    event->uriPathLength = uriPathLength;
    event->payloadLength = payloadLength;
    event->applicationDataLength = applicationDataLength;
    finger += sizeof(MessageEvent);
    MEMCOPY(finger, uriPath, event->uriPathLength);
    finger += event->uriPathLength;
    MEMCOPY(finger, payload, event->payloadLength);
    finger += event->payloadLength;
    MEMCOPY(finger, applicationData, event->applicationDataLength);
    emberEventSetDelayMs((Event *)event, DISCOVERY_TIMEOUT_MS); // Set address discovery timeout.
    return EMBER_SUCCESS;
  } else {
    return EMBER_ERR_FATAL;
  }

  EmberCoapSendInfo info = {
    .nonConfirmed = false,

    .localAddress = { { 0 } }, // use default
    .localPort = 0,        // use default
    .remotePort = destination->port,

    .options = cborOptions,
    .numberOfOptions = COUNTOF(cborOptions),

    .responseTimeoutMs = 0, // use default

    .responseAppData = applicationData,
    .responseAppDataLength = applicationDataLength,

    .transmitHandler = NULL // unused
  };

  #ifdef EMBER_AF_PLUGIN_DTLS_CLI
  if (secureDtlsSession) {
    info.transmitHandlerData = (void *) (uint32_t) secureDtlsSessionId;
    info.transmitHandler = &emberDtlsTransmitHandler;
    info.localPort = EMBER_COAP_SECURE_PORT;
    info.remotePort = EMBER_COAP_SECURE_PORT;
  }
  #endif

  return emberCoapSend(remoteAddress,
                       code,
                       uriPath,
                       payload,
                       payloadLength,
                       (handler == NULL
                        ? defaultCoapResponseHandler
                        : handler),
                       &info);
}

static bool discoverAddress(const EmberZclCoapEndpoint_t *destination)
{
  if (destination->flags & EMBER_ZCL_HAVE_UID_FLAG) {
    return (emberZclDiscByUid(&destination->uid,
                              EMBER_ZCL_UID_BITS, // discovery matches all uid bits.
                              uidDiscoveryResponseHandler));
  }
  //TODO Other discovery types here?
  return false;
}

static void uidDiscoveryResponseHandler(EmberCoapStatus status,
                                        EmberCoapCode code,
                                        EmberCoapReadOptions *options,
                                        uint8_t *payload,
                                        uint16_t payloadLength,
                                        EmberCoapResponseInfo *info)
{
  if (status == EMBER_COAP_MESSAGE_RESPONSE) {
    EmberZclUid_t uid;
    if (validateUidDiscoveryResponse(payload,
                                     payloadLength,
                                     &uid)) {
      emZclCacheAdd(&uid, &info->remoteAddress, NULL);
      // Find and send any buffered messages with a matching uid destination.
      MessageEvent *event
        = (MessageEvent *)emberFindAllEvents(&emAppEventQueue,
                                             &actions,
                                             (EventPredicate)destinationPredicate,
                                             (void *)&uid);
      while (event != NULL) {
        uint8_t *finger = (uint8_t *) event;
        finger += sizeof(MessageEvent);
        const uint8_t *uriPath = finger;
        finger += event->uriPathLength;
        const uint8_t *payload = finger;
        finger += event->payloadLength;
        uint8_t *applicationData = finger;
        //finger += event->applicationDataLength;

        EmberZclCoapEndpoint_t destination;
        destination.flags = EMBER_ZCL_HAVE_IPV6_ADDRESS_FLAG;
        MEMCOPY(&destination.address,
                &info->remoteAddress,
                sizeof(EmberIpv6Address));
        destination.port = event->destination.port;

        emZclSend(&destination,
                  event->code,
                  uriPath,
                  payload,
                  event->payloadLength,
                  event->handler,
                  applicationData,
                  event->applicationDataLength);

        MessageEvent *next = (MessageEvent *)event->event.next;
        event->event.next = NULL;
        event = next;
      }
    }
  }
}

static bool validateUidDiscoveryResponse(const uint8_t *payload,
                                         uint16_t payloadLength,
                                         EmberZclUid_t *uid)
{
  // Checks that uid discovery response payload contains a valid UID uri.

  uint16_t uidPrefixLen = strlen(EM_ZCL_URI_QUERY_UID_SHA_256);

  if (payloadLength >= uidPrefixLen + EMBER_ZCL_UID_BASE64URL_LENGTH) {
    for (uint16_t i = 0; i < (payloadLength - uidPrefixLen); ++i) {
      if (MEMCOMPARE(&payload[i], EM_ZCL_URI_QUERY_UID_SHA_256, uidPrefixLen) == 0) {
        // Found the uid prefix, set ptr to start of the uidB64u string.
        uint8_t *uidB64u = (uint8_t *)&payload[i] + uidPrefixLen;

        uint16_t uidBits;
        if (!emZclBase64UrlToUid(uidB64u,
                                 EMBER_ZCL_UID_BASE64URL_LENGTH,
                                 uid,
                                 &uidBits)
            || (uidBits != EMBER_ZCL_UID_BITS)) {
          return false;
        }
        return true;
      }
    }
  }
  return false;
}

// When a message times out, we assume the remote address has changed and
// needs to be rediscovered, so we remove any key pointing to that address in
// our cache.  The next attempt to send to one of those keys will result in a
// rediscovery.
void emZclCoapStatusHandler(EmberCoapStatus status, EmberCoapResponseInfo *info)
{
  if (status == EMBER_COAP_MESSAGE_TIMED_OUT) {
    emZclCacheRemoveAllByIpv6Prefix(&info->remoteAddress, EMBER_IPV6_BITS);
  }
}

static void defaultCoapResponseHandler(EmberCoapStatus status,
                                       EmberCoapCode code,
                                       EmberCoapReadOptions *options,
                                       uint8_t *payload,
                                       uint16_t payloadLength,
                                       EmberCoapResponseInfo *info)
{
  emZclCoapStatusHandler(status, info);
}

static void eventHandler(MessageEvent *event)
{
  // Called on address discovery response timeout.

  if (event->handler != NULL) {
    // Inform response handler that the uid discovery timed out.
    const uint8_t *finger = (const uint8_t *)event;
    finger += sizeof(MessageEvent);
    //const uint8_t *uriPath = finger;
    finger += event->uriPathLength;
    //const uint8_t *payload = (event->payloadLength == 0 ? NULL : finger);
    finger += event->payloadLength;
    //finger += event->applicationDataLength;
    EmberCoapResponseInfo info;
    info.applicationData = (uint8_t *) finger;
    info.applicationDataLength = event->applicationDataLength;
    (*event->handler)(EMBER_ZCL_MESSAGE_STATUS_DISCOVERY_TIMEOUT,
                      EMBER_COAP_CODE_EMPTY,
                      NULL, // EmberCoapReadOptions
                      NULL, // payload
                      0,    // payload length
                      &info);
  }
}

static void eventMarker(MessageEvent *event)
{
}

static bool destinationPredicate(MessageEvent *event,
                                 const EmberZclUid_t *uid)
{
  if (event->destination.flags & EMBER_ZCL_HAVE_UID_FLAG) {
    return (MEMCOMPARE(uid,
                       &event->destination.uid,
                       sizeof(EmberZclUid_t)) == 0);
  }
  return false;
}

typedef struct {
  EmberZclMessageStatus_t status;
  const uint8_t * const name;
} ZclMessageStatusEntry;
static const ZclMessageStatusEntry statusTable[] = {
  { EMBER_ZCL_MESSAGE_STATUS_DISCOVERY_TIMEOUT, (const uint8_t *)"DISCOVERY TIMEOUT", },
  { EMBER_ZCL_MESSAGE_STATUS_COAP_TIMEOUT, (const uint8_t *)"COAP TIMEOUT", },
  { EMBER_ZCL_MESSAGE_STATUS_COAP_ACK, (const uint8_t *)"COAP ACK", },
  { EMBER_ZCL_MESSAGE_STATUS_COAP_RESET, (const uint8_t *)"COAP RESET", },
  { EMBER_ZCL_MESSAGE_STATUS_COAP_RESPONSE, (const uint8_t *)"COAP RESPONSE", },
};
const uint8_t *emZclGetMessageStatusName(EmberZclMessageStatus_t status)
{
  for (size_t i = 0; i < COUNTOF(statusTable); i++) {
    if (status == statusTable[i].status) {
      return statusTable[i].name;
    }
  }
  return (const uint8_t *)"?????";
}
