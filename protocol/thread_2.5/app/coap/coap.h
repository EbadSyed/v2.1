// File: coap.h
//
// Description: stack-internal declarations for CoAP
//
// Copyright 2015 Silicon Laboratories, Inc.                                *80*

#ifndef __COAP_H__
#define __COAP_H__

// This is RFC 7252's ACK_TIMEOUT * ACK_RANDOM_FACTOR = 2000ms * 1.5
#define COAP_ACK_TIMEOUT_MS    3000
#define COAP_MAX_RETRANSMIT    4
extern uint32_t emCoapAckTimeoutMs;

#define THREAD_COAP_PORT 61631

void emCoapMarkBuffers(void);
void emCoapInitialize(void);

// The Coap Header, from RFC 7252
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |Ver| T |  TKL  |      Code     |        Message ID             |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   Tokens (if any, TKL bytes) ...
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   Options (if any) ...
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |1 1 1 1 1 1 1 1|   Payload (if any) ...
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// The fields in the header are defined as follows:
//
//   Version (Ver):  2-bit unsigned integer.  Indicates the CoAP version
//      number.  Implementations of this specification MUST set this field
//      to 1 (01 binary).  Other values are reserved for future versions.
//      Messages with unknown version numbers MUST be silently ignored.
//
//   Type (T):  2-bit unsigned integer.  Indicates if this message is of
//      type Confirmable (0), Non-confirmable (1), Acknowledgement (2), or
//      Reset (3).  The semantics of these message types are defined in
//      Section 4.
//
//   Token Length (TKL):  4-bit unsigned integer.  Indicates the length of
//      the variable-length Token field (0-8 bytes).  Lengths 9-15 are
//      reserved, MUST NOT be sent, and MUST be processed as a message
//      format error.
//
//   Code:  8-bit unsigned integer, split into a 3-bit class (most
//      significant bits) and a 5-bit detail (least significant bits),
//      documented as "c.dd" where "c" is a digit from 0 to 7 for the
//      3-bit subfield and "dd" are two digits from 00 to 31 for the 5-bit
//      subfield.  The class can indicate a request (0), a success
//      response (2), a client error response (4), or a server error
//      response (5).  (All other class values are reserved.)  As a
//      special case, Code 0.00 indicates an Empty message.  In case of a
//      request, the Code field indicates the Request Method; in case of a
//      response, a Response Code.  Possible values are maintained in the
//      CoAP Code Registries (Section 12.1).  The semantics of requests
//      and responses are defined in Section 5.
//
//   Message ID:  16-bit unsigned integer in network byte order.  Used to
//      detect message duplication and to match messages of type
//      Acknowledgement/Reset to messages of type Confirmable/Non-
//      confirmable.  The rules for generating a Message ID and matching
//      messages are defined in Section 4.

#define COAP_VERSION_MASK      0xC0
#define COAP_VERSION           0x40
#define COAP_TYPE_MASK         0x30
#define COAP_TOKEN_LENGTH_MASK 0x0F
#define COAP_MAX_TOKEN_LENGTH     8

// Minimum packet size - flag byte, code byte, two bytes of message ID.
#define COAP_BASE_SIZE 4
// Token comes after the base.
#define COAP_TOKEN_INDEX 4

//----------------------------------------------------------------
// From here on down is only used by coap-stack.c and coap-host.c and
// needs to be moved back to app/coap/coap.h.

typedef enum {
  COAP_TYPE_CONFIRMABLE     = 0x00,
  COAP_TYPE_NON_CONFIRMABLE = 0x10,
  COAP_TYPE_ACK             = 0x20,
  COAP_TYPE_RESET           = 0x30
} CoapType;

typedef struct CoapMessage_s {
  // IP addressing
  EmberIpv6Address localAddress;
  EmberIpv6Address remoteAddress;
  uint16_t localPort;
  uint16_t remotePort;
  bool multicastLoopback : 1;

  // CoAP header
  EmberCoapCode code;
  uint8_t token[EMBER_COAP_MAX_TOKEN_LENGTH];
  uint8_t tokenLength;
  const uint8_t *encodedOptions;
  uint16_t encodedOptionsLength;
  const uint8_t *encodedUri;
  uint16_t encodedUriLength;

  // Payload
  const uint8_t *payload;
  uint16_t payloadLength;

  CoapType type;
  EmberCoapTransmitHandler transmitHandler;
  void *transmitHandlerData;
  uint16_t transmitDelayMs;
  uint16_t messageId;             // matched against incoming acks
  EmberCoapResponseHandler responseHandler;
  uint32_t responseTimeoutMs;
  const uint8_t *responseAppData;
  uint16_t responseAppDataLength;
  Buffer message;       // incoming message buffer (for potential reuse)
} CoapMessage;

EmberStatus emberFinishCoapMessage(CoapMessage *coapMessage,
                                   uint8_t *coapHeader,
                                   uint16_t coapHeaderLength,
                                   Buffer payloadBuffer,
                                   Buffer *headerLoc);

// The reality of the opaque type in stack/include/coap.h.

struct EmberCoapReadOptions_s {
  const uint8_t *start;
  const uint8_t *end;
  const uint8_t *nextOption;
  EmberCoapOptionType previousType;
};

void emInitCoapReadOptions(EmberCoapReadOptions *options,
                           const uint8_t *encodedOptions,
                           int16_t length);

int16_t emberReadUriPath(EmberCoapReadOptions *options,
                         uint8_t *pathBuffer,
                         uint16_t pathBufferLength);
#ifdef EMBER_TEST
extern bool emCoapHistoryCheck;

uint16_t coapEncodeUri(const uint8_t *uri,
                       uint8_t *buffer,
                       uint16_t bufferLength,
                       EmberCoapOptionType previousOption);
void coapDecodeUri(const uint8_t *optionBytes,
                   uint16_t optionBytesLength,
                   uint8_t *uriBuffer,
                   uint16_t uriBufferLength);
#endif

// If 'uri' is NULL, then message->uriPath is used as the encoded URI.  If
// that is also NULL, then there is no URI.

EmberStatus emSubmitCoapMessage(CoapMessage *message,
                                const uint8_t *uri,
                                Buffer payloadBuffer);

EmberStatus emCoapRespondWithBuffer(const EmberCoapRequestInfo *requestInfo,
                                    EmberCoapCode code,
                                    const uint8_t *payload,
                                    uint16_t payloadLength,
                                    Buffer morePayload);

bool emCoapRequestHandler(EmberCoapCode code,
                          const uint8_t *uri,
                          EmberCoapReadOptions *options,
                          const uint8_t *payload,
                          uint16_t payloadLength,
                          const EmberCoapRequestInfo *info,
                          Buffer header);

const uint8_t *emGetCoapCodeName(EmberCoapCode type);
const uint8_t *emGetCoapTypeName(CoapType type);

typedef struct {
  Event event;
  uint8_t retryCount;           // number of retries so far
  uint16_t messageId;           // matched against incoming acks
  Buffer packetHeader;          // for resending
  uint16_t token;               // for matching requests and responses
  uint32_t responseTimeoutMs;
  EmberCoapResponseHandler handler;
  uint16_t responseAppDataLength;
  EmberCoapTransmitHandler transmitHandler;

  // Stuff passed to the transmitHandler
  EmberIpv6Address localAddress;
  uint16_t localPort;
  EmberIpv6Address remoteAddress;
  uint16_t remotePort;
  void *transmitHandlerData;

  Buffer uri;
} CoapRetryEvent;

EmberStatus emberRetryCoapMessage(CoapRetryEvent *event);

bool emParseCoapMessage(const uint8_t *data,
                        uint16_t dataLength,
                        CoapMessage *coapMessage);

typedef bool (*CoapRequestHandler)(EmberCoapCode code,
                                   const uint8_t *uri,
                                   EmberCoapReadOptions *options,
                                   uint8_t *payload,
                                   uint16_t payloadLength,
                                   const EmberCoapRequestInfo *info,
                                   Buffer header);

void emProcessCoapMessage(Buffer header,
                          const uint8_t *message,
                          uint16_t messageLength,
                          CoapRequestHandler handler,
                          EmberCoapRequestInfo *requestInfo);

bool emJoinerEntrustTransmitHandler(const uint8_t *payload,
                                    uint16_t payloadLength,
                                    const EmberIpv6Address *localAddress,
                                    uint16_t localPort,
                                    const EmberIpv6Address *remoteAddress,
                                    uint16_t remotePort,
                                    void *transmitHandlerData);
#define emIsJoinerEntrustMessage(coapMessage) \
  ((coapMessage)->transmitHandler == &emJoinerEntrustTransmitHandler)

// Unacked Messages

extern Buffer emUnackedCoapMessages;
extern EventActions emCoapRetryEventActions;
void emNoteCoapAck(CoapMessage *message);

extern uint16_t emStackCoapPort;

// Utility for looking up URIs.
typedef void (*Func)(void);
typedef const struct {
  const char *uri;
  uint32_t tlvMask;
  Func handler;
} UriHandler;

UriHandler* emLookupUriHandler(const uint8_t *uri, UriHandler *handlers);

#endif
