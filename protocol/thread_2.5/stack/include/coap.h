/**
 * @file coap.h
 * @brief Constrained Application Protocol (CoAP) API.
 *
 * <!--Copyright 2015 Silicon Laboratories, Inc.                         *80*-->
 */

#ifndef COAP_H
#define COAP_H

/**
 * @addtogroup coap
 *
 * The CoAP API was updated for the Silicon Labs Thread 2.2 release.  The new
 * API is simpler and more flexible.  These notes provide a brief overview of
 * the key parts of the API to help customers migrate from
 * the old API to the new API.
 *
 * Sending messages
 *
 * The old API had two functions for sending messages:
 *
 * @code
 * EmberStatus emberCoapSend(EmberCoapMessage *message,
 *                           const uint8_t *uri,
 *                           uint32_t responseTimeoutMs,
 *                           EmberCoapResponseHandler responseHandler,
 *                           void *applicationData,
 *                           uint16_t applicationDataLength);
 *
 * EmberStatus emberCoapSendUri(EmberCoapCode code,
 *                              const EmberIpv6Address *destination,
 *                              const uint8_t *uri,
 *                              const uint8_t *body,
 *                              uint16_t bodyLength,
 *                              EmberCoapResponseHandler responseHandler);
 * @endcode
 *
 * If the responseHandler was NULL, the message was sent as a non-confirmable
 * message (NON).  Otherwise, the message was sent as a confirmable message
 * (CON) and responseHandler was called when a response was received or a
 * timeout occurred.
 *
 * Now a single function is used for sending:
 *
 * @code
 * EmberStatus emberCoapSend(const EmberIpv6Address *destination,
 *                           EmberCoapCode code,
 *                           const uint8_t *path,
 *                           const uint8_t *payload,
 *                           uint16_t payloadLength,
 *                           EmberCoapResponseHandler responseHandler,
 *                           const EmberCoapSendInfo *info);
 * @endcode
 *
 * Common settings are passed directly as arguments to the function with other
 * settings passed through a structure.  CON or NON is determined by a field in the
 * structure.  Default settings, which include sending a CON to unicast
 * addresses or NON to multicast addresses, can be obtained by zeroing the
 * structure:
 *
 * @code
 * // Clear the structure to use default settings.
 * EmberCoapSendInfo info;
 * MEMSET(&info, 0, sizeof(EmberCoapSendInfo));
 * EmberStatus status = emberCoapSend(destination,
 *                                    EMBER_COAP_CODE_GET,
 *                                    "an/example/path",
 *                                    payload,
 *                                    payloadLength,
 *                                    myResponseHandler,
 *                                    &info);
 * @endcode
 *
 * The old API had a number of utility functions for sending specific types of
 * requests:
 *
 * @code
 * EmberStatus emberCoapGet(const EmberIpv6Address *destination,
 *                          const uint8_t *uri,
 *                          EmberCoapResponseHandler responseHandler);
 *
 * EmberStatus emberCoapPost(const EmberIpv6Address *destination,
 *                           const uint8_t *uri,
 *                           const uint8_t *body,
 *                           uint16_t bodyLength,
 *                           EmberCoapResponseHandler responseHandler);
 *
 * EmberStatus emberCoapPostNonconfirmable(const EmberIpv6Address *destination,
 *                                         const uint8_t *uri,
 *                                         const uint8_t *body,
 *                                         uint16_t bodyLength);
 * @endcode
 *
 * Similar utility functions exist in the new API.
 *
 * @code
 * EmberStatus emberCoapGet(const EmberIpv6Address *destination,
 *                          const uint8_t *path,
 *                          EmberCoapResponseHandler responseHandler,
 *                          const EmberCoapSendInfo *info);
 *
 * EmberStatus emberCoapPut(const EmberIpv6Address *destination,
 *                          const uint8_t *path,
 *                          const uint8_t *payload,
 *                          uint16_t payloadLength,
 *                          EmberCoapResponseHandler responseHandler,
 *                          const EmberCoapSendInfo *info);
 *
 * EmberStatus emberCoapPost(const EmberIpv6Address *destination,
 *                           const uint8_t *path,
 *                           const uint8_t *payload,
 *                           uint16_t payloadLength,
 *                           EmberCoapResponseHandler responseHandler,
 *                           const EmberCoapSendInfo *info);
 *
 * EmberStatus emberCoapDelete(const EmberIpv6Address *destination,
 *                             const uint8_t *path,
 *                             EmberCoapResponseHandler responseHandler,
 *                             const EmberCoapSendInfo *info);
 * @endcode
 *
 * Receiving messages
 *
 * The old API had a single handler for receiving messages:
 *
 * @code
 * void emberCoapMessageHandler(EmberCoapMessage *message);
 * @endcode
 *
 * All details of the message were contained in a single structure.  Responses
 * had to be sent from within the context of the handler.
 *
 * The new API also has a single handler for receiving messages:
 *
 * @code
 * void emberCoapRequestHandler(EmberCoapCode code,
 *                              uint8_t *uri,
 *                              EmberCoapReadOptions *options,
 *                              const uint8_t *payload,
 *                              uint16_t payloadLength,
 *                              const EmberCoapRequestInfo *info);
 * @endcode
 *
 * Common settings are passed directly as arguments to the handler, with other
 * settings passed via structures.  The EmberCoapRequestInfo structure is used
 * to send a response to the request.  By saving the contents of the structure,
 * the application can send a delayed response to the message.  If a response
 * is sent within the context of the handler, the stack will send a piggybacked
 * response, which has the acknowledgement and the response in a single
 * message.  If a response is not sent within the context of the handler, the
 * stack will send an acknowledgement only.  The application can send a
 * separate response later.
 *
 * Responding to messages
 *
 * The old API had a number of functions for sending responses:
 *
 * @code
 * EmberStatus emberCoapRespond(EmberCoapCode responseCode,
 *                              const uint8_t *payload,
 *                              uint16_t payloadLength);
 *
 * EmberStatus emberCoapRespondFormat(EmberCoapCode responseCode,
 *                                    EmberCoapContentFormatType format,
 *                                    const uint8_t *payload,
 *                                    uint16_t payloadLength);
 *
 * EmberStatus emberCoapRespondCreated(const uint8_t *locationPath);
 * @endcode
 *
 * Each of these were required to be called from within the context of
 * emberCoapMessageHandler.
 *
 * The new API has a single function for sending responses:
 *
 * @code
 * EmberStatus emberCoapRespondWithPath(const EmberCoapRequestInfo *requestInfo,
 *                                      EmberCoapCode code,
 *                                      const uint8_t *path,
 *                                      const EmberCoapOption *options,
 *                                      uint8_t numberOfOptions,
 *                                      const uint8_t *payload,
 *                                      uint16_t payloadLength);
 * @endcode
 *
 * The new API also has utility functions for sending specific types of responses.
 *
 * @code
 * EmberStatus emberCoapRespond(const EmberCoapRequestInfo *requestInfo,
 *                              EmberCoapCode code,
 *                              const EmberCoapOption *options,
 *                              uint8_t numberOfOptions,
 *                              const uint8_t *payload,
 *                              uint16_t payloadLength);
 *
 * EmberStatus emberCoapRespondWithCode(const EmberCoapRequestInfo *requestInfo,
 *                                      EmberCoapCode code);
 *
 * EmberStatus emberCoapRespondWithPayload(const EmberCoapRequestInfo *requestInfo,
 *                                         EmberCoapCode code,
 *                                         const uint8_t *payload,
 *                                         uint16_t payloadLength);
 * @endcode
 *
 * The EmberCoapRequestInfo structure from the request is passed in when
 * sending a response.  To send a piggybacked response, call one of the
 * response APIs from within the handler:
 *
 * @code
 * void emberCoapRequestHandler(EmberCoapCode code,
 *                              uint8_t *uri,
 *                              EmberCoapReadOptions *options,
 *                              const uint8_t *payload,
 *                              uint16_t payloadLength,
 *                              const EmberCoapRequestInfo *info)
 * {
 *   emberCoapRespondWithCode(info, EMBER_COAP_CODE_204_CHANGED);
 * }
 * @endcode
 *
 * To send a separate response, save the structure and use it to reply later.
 * A utility function can be used to save the structure:
 *
 * @code
 * void emberSaveRequestInfo(const EmberCoapRequestInfo *from,
 *                           EmberCoapRequestInfo *to);
 *
 * static EmberCoapRequestInfo myInfo;
 * void emberCoapRequestHandler(EmberCoapCode code,
 *                              uint8_t *uri,
 *                              EmberCoapReadOptions *options,
 *                              const uint8_t *payload,
 *                              uint16_t payloadLength,
 *                              const EmberCoapRequestInfo *info)
 * {
 *   emberSaveRequestInfo(info, &myInfo);
 * }
 *
 * static void sendResponse(void)
 * {
 *   emberCoapRespondWithCode(&myInfo, EMBER_COAP_CODE_204_CHANGED);
 * }
 * @endcode
 *
 * @{
 */

//----------------------------------------------------------------
// Values from from RFC 7252.

#define MAKE_COAP_CODE(class, detail) ((class << 5) | detail)
#define GET_COAP_CLASS(code) (((code) & 0xE0) >> 5)
#define GET_COAP_DETAIL(code) ((code) & 0x1F)

typedef enum {
  EMBER_COAP_CLASS_REQUEST               = 0,
  EMBER_COAP_CLASS_SUCCESS_RESPONSE      = 2,
  EMBER_COAP_CLASS_CLIENT_ERROR_RESPONSE = 4,
  EMBER_COAP_CLASS_SERVER_ERROR_RESPONSE = 5,
} EmberCoapClass;

typedef enum {
  // empty messages
  EMBER_COAP_CODE_EMPTY                          = MAKE_COAP_CODE(0, 0),

  // requests
  EMBER_COAP_CODE_GET                            = MAKE_COAP_CODE(0, 1),
  EMBER_COAP_CODE_POST                           = MAKE_COAP_CODE(0, 2),
  EMBER_COAP_CODE_PUT                            = MAKE_COAP_CODE(0, 3),
  EMBER_COAP_CODE_DELETE                         = MAKE_COAP_CODE(0, 4),

  // responses
  EMBER_COAP_CODE_201_CREATED                    = MAKE_COAP_CODE(2, 1),
  EMBER_COAP_CODE_202_DELETED                    = MAKE_COAP_CODE(2, 2),
  EMBER_COAP_CODE_203_VALID                      = MAKE_COAP_CODE(2, 3),
  EMBER_COAP_CODE_204_CHANGED                    = MAKE_COAP_CODE(2, 4),
  EMBER_COAP_CODE_205_CONTENT                    = MAKE_COAP_CODE(2, 5),
  EMBER_COAP_CODE_400_BAD_REQUEST                = MAKE_COAP_CODE(4, 0),
  EMBER_COAP_CODE_401_UNAUTHORIZED               = MAKE_COAP_CODE(4, 1),
  EMBER_COAP_CODE_402_BAD_OPTION                 = MAKE_COAP_CODE(4, 2),
  EMBER_COAP_CODE_403_FORBIDDEN                  = MAKE_COAP_CODE(4, 3),
  EMBER_COAP_CODE_404_NOT_FOUND                  = MAKE_COAP_CODE(4, 4),
  EMBER_COAP_CODE_405_METHOD_NOT_ALLOWED         = MAKE_COAP_CODE(4, 5),
  EMBER_COAP_CODE_406_NOT_ACCEPTABLE             = MAKE_COAP_CODE(4, 6),
  EMBER_COAP_CODE_412_PRECONDITION_FAILED        = MAKE_COAP_CODE(4, 12),
  EMBER_COAP_CODE_413_REQUEST_ENTITY_TOO_LARGE   = MAKE_COAP_CODE(4, 13),
  EMBER_COAP_CODE_415_UNSUPPORTED_CONTENT_FORMAT = MAKE_COAP_CODE(4, 15),
  EMBER_COAP_CODE_500_INTERNAL_SERVER_ERROR      = MAKE_COAP_CODE(5, 0),
  EMBER_COAP_CODE_501_NOT_IMPLEMENTED            = MAKE_COAP_CODE(5, 1),
  EMBER_COAP_CODE_502_BAD_GATEWAY                = MAKE_COAP_CODE(5, 2),
  EMBER_COAP_CODE_503_SERVICE_UNAVAILABLE        = MAKE_COAP_CODE(5, 3),
  EMBER_COAP_CODE_504_GATEWAY_TIMEOUT            = MAKE_COAP_CODE(5, 4),
  EMBER_COAP_CODE_505_PROXYING_NOT_SUPPORTED     = MAKE_COAP_CODE(5, 5),
} EmberCoapCode;

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief This function indicates whether the code represents a successful response. */
bool emberCoapIsSuccessResponse(EmberCoapCode code);
/** @brief This function indicates whether the code represents a client error response. */
bool emberCoapIsClientErrorResponse(EmberCoapCode code);
/** @brief This function indicates whether the code represents a server error response. */
bool emberCoapIsServerErrorResponse(EmberCoapCode code);
#else
  #define emberCoapIsSuccessResponse(code) \
  (GET_COAP_CLASS(code) == EMBER_COAP_CLASS_SUCCESS_RESPONSE)
  #define emberCoapIsClientErrorResponse(code) \
  (GET_COAP_CLASS(code) == EMBER_COAP_CLASS_CLIENT_ERROR_RESPONSE)
  #define emberCoapIsServerErrorResponse(code) \
  (GET_COAP_CLASS(code) == EMBER_COAP_CLASS_SERVER_ERROR_RESPONSE)
#endif

/** @brief This function indicates whether the code represents a request. */
bool emberCoapIsRequest(EmberCoapCode code);

/** @brief This function indicates whether the code represents a response. */
bool emberCoapIsResponse(EmberCoapCode code);

typedef enum {
  EMBER_COAP_NO_OPTION             = 0,
  EMBER_COAP_OPTION_IF_MATCH       = 1,
  EMBER_COAP_OPTION_URI_HOST       = 3,
  EMBER_COAP_OPTION_ETAG           = 4,
  EMBER_COAP_OPTION_IF_NONE_MATCH  = 5,
  EMBER_COAP_OPTION_OBSERVE        = 6,
  EMBER_COAP_OPTION_URI_PORT       = 7,
  EMBER_COAP_OPTION_LOCATION_PATH  = 8,
  EMBER_COAP_OPTION_URI_PATH       = 11,
  EMBER_COAP_OPTION_CONTENT_FORMAT = 12,
  EMBER_COAP_OPTION_MAX_AGE        = 14,
  EMBER_COAP_OPTION_URI_QUERY      = 15,
  EMBER_COAP_OPTION_ACCEPT         = 17,
  EMBER_COAP_OPTION_LOCATION_QUERY = 20,
  EMBER_COAP_OPTION_BLOCK2         = 23,
  EMBER_COAP_OPTION_BLOCK1         = 27,
  EMBER_COAP_OPTION_SIZE2          = 28,
  EMBER_COAP_OPTION_PROXY_URI      = 35,
  EMBER_COAP_OPTION_PROXY_SCHEME   = 39,
  EMBER_COAP_OPTION_SIZE1          = 60
} EmberCoapOptionType;

typedef enum {
  EMBER_COAP_CONTENT_FORMAT_TEXT_PLAIN   =  0,
  EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT  = 40,
  EMBER_COAP_CONTENT_FORMAT_XML          = 41,
  EMBER_COAP_CONTENT_FORMAT_OCTET_STREAM = 42,
  EMBER_COAP_CONTENT_FORMAT_EXI          = 47,
  EMBER_COAP_CONTENT_FORMAT_JSON         = 50,
  EMBER_COAP_CONTENT_FORMAT_CBOR         = 60,
  EMBER_COAP_CONTENT_FORMAT_LINK_FORMAT_PLUS_CBOR = 65064, // Pending IANA assignment
  EMBER_COAP_CONTENT_FORMAT_NONE         = -1   // no content format option
} EmberCoapContentFormatType;

#define EMBER_COAP_PORT        5683
#define EMBER_COAP_SECURE_PORT 5684

#define EMBER_COAP_MAX_TOKEN_LENGTH 8

#define EMBER_COAP_DEFAULT_TIMEOUT_MS 90000

//----------------
// Receiving options

/** @brief This function encapsulates incoming CoAP options.
 *
 * The EmberCoapReadOptions type encapsulates the incoming options
 * from a message.  It is opaque to the application and contains an
 * interal pointer that can be used to walk down the list of options
 * received.  Options can be read sequentially, using
 * emberReadNextOption(), or individually, using
 * emberReadIntegerOption() or emberReadBytesOption().
 */

typedef struct EmberCoapReadOptions_s EmberCoapReadOptions;

/** @brief This function reads the next option from an incoming message.
 *
 * Reads the next option from an incoming message, advancing the
 * internal pointer to the following option.  Returns
 * EMBER_COAP_NO_OPTION if there are no more options to read.
 */

EmberCoapOptionType emberReadNextOption(EmberCoapReadOptions *options,
                                        const uint8_t **valuePointerLoc,
                                        uint16_t *valueLengthLoc);

/** @brief This function resets the internal pointer back to the first option.
 */

void emberResetReadOptionPointer(EmberCoapReadOptions *options);

/** @brief This function decodes the value of an integer option.
 *
 * This function should be passed the value and length
 * returned by emberReadNextOption().
 */

uint32_t emberReadOptionValue(const uint8_t *value, uint16_t valuelength);

/** @brief This function reads the value of an integer option.
 *
 * This function searches from the beginning of the options and leaves `option`s
 * internal pointer pointing to the option following the one returned.
 *
 * @return true if the option was found
 */

bool emberReadIntegerOption(EmberCoapReadOptions *options,
                            EmberCoapOptionType type,
                            uint32_t *valueLoc);

/** @brief This function reads the value of an option.
 *
 * This function searches from the beginning of the options and leaves `option`s
 * internal pointer pointing to the option following the one returned.
 * @return true if the option was found
 */

bool emberReadBytesOption(EmberCoapReadOptions *options,
                          EmberCoapOptionType type,
                          const uint8_t **valueLoc,
                          uint16_t *valueLengthLoc);

/** @brief This function converts path options to a string.
 *
 * Any path options are copied to `pathBuffer` with a '/' between each
 * pair of path elements.
 *
 * If the path is longer than `pathBufferLength` only the first
 * `pathBufferLength` bytes are stored in `pathBuffer`.
 *
 * @return the length of the complete path
 */

int16_t emberReadLocationPath(EmberCoapReadOptions *options,
                              uint8_t *pathBuffer,
                              uint16_t pathBufferLength);

/** @brief Structure that includes options in outgoing requests and responses.
 *
 * Calls that send messages can be passed a pointer to any array of
 * `EmberCoapOption`s.
 */

typedef struct {
  EmberCoapOptionType type;  /*!< The type of the option. */
  const uint8_t *value;      /*!< A pointer to the option's value. */
  uint16_t valueLength;      /*!< Number of bytes in the option's value. */
  uint32_t intValue; /*!< The value of the option interpreted as a uint32_t. */
} EmberCoapOption;

//----------------
// Sending requests

/** @brief Function type for alternative transports.
 *
 * CoAP messages can be sent via transports other than UDP, such as DTLS,
 * by providing a function of this type.  The CoAP code calls the provided
 * function whenever a message needs to be sent.
 *
 * The addresses, ports, and `transmitHandlerData` are the values that were
 * passed to the original call to `emberCoapSend()` (if the message is a
 * request) or to `emberProcessCoap()` (if the message is a response).
 * Their values are specific to the underlying transport and need not
 * be actual IPv6 addresses or ports.
 *
 * @return true if transmission was initiated and false otherwise
 */

typedef bool (*EmberCoapTransmitHandler)(const uint8_t *payload,
                                         uint16_t payloadLength,
                                         const EmberIpv6Address *localAddress,
                                         uint16_t localPort,
                                         const EmberIpv6Address *remoteAddress,
                                         uint16_t remotePort,
                                         void *transmitHandlerData);

/** @brief Additional information about an incoming response.
 */

typedef struct {
  EmberIpv6Address localAddress;
  EmberIpv6Address remoteAddress;
  uint16_t localPort;
  uint16_t remotePort;
  void *applicationData;         /*!< The value passed to `emberCoapSend()`. */
  uint16_t applicationDataLength; /*!< The value passed to `emberCoapSend()`. */
} EmberCoapResponseInfo;

/** @brief Status values passed to response handlers.
 *
 * For unicast requests `EmberCoapResponseHandler()` will usually be
 * called exactly once, with one of the following three status values:
 *
 * `EMBER_COAP_MESSAGE_RESPONSE`
 * `EMBER_COAP_MESSAGE_TIMED_OUT`
 * `EMBER_COAP_MESSAGE_RESET`
 *
 * If the server sends an ACK before any other response,
 * `EmberCoapResponseHandler()` will be called twice, the first time
 * with status `EMBER_COAP_MESSAGE_ACKED` and the second time with
 * either status `EMBER_COAP_MESSAGE_RESPONSE` (if a response arrives
 * after the ACK and before the timeout) or status
 * `EMBER_COAP_MESSAGE_TIMED_OUT` (if no response arrives after the ACK
 * and before the timeout).
 *
 * For multicast requests, `EmberCoapResponseHandler()` will be called
 * with status `EMBER_COAP_MESSAGE_RESPONSE` for each response that
 * arrives and a final time with status `EMBER_COAP_MESSAGE_TIMED_OUT`.
 */

typedef enum {
  EMBER_COAP_MESSAGE_TIMED_OUT,
  EMBER_COAP_MESSAGE_ACKED,
  EMBER_COAP_MESSAGE_RESET,
  EMBER_COAP_MESSAGE_RESPONSE
} EmberCoapStatus;

/** @brief Type definition for callback handlers for a response.
 *
 * The arguments`options`, `payload`, and `payloadLength` are
 * meaningful only when `status` is `EMBER_COAP_MESSAGE_RESPONSE`.
 */

typedef void (*EmberCoapResponseHandler)(EmberCoapStatus status,
                                         EmberCoapCode code,
                                         EmberCoapReadOptions *options,
                                         uint8_t *payload,
                                         uint16_t payloadLength,
                                         EmberCoapResponseInfo *info);

/** @brief Optional information when sending a message.
 *
 * For all fields a value of 0 or NULL means that the
 * default will be used.
 *
 * Multicast are always sent as unconfirmed.
 */

typedef struct {
  bool nonConfirmed : 1;            /*!< Defaults to confirmed */
  bool multicastLoopback : 1;       /*!< Defaults to not looping back */

  EmberIpv6Address localAddress;    /*!< Default is to let the IP stack choose */
  uint16_t localPort;               /*!< Defaults to the CoAP port (5683) */
  uint16_t remotePort;              /*!< Defaults to the CoAP port (5683) */

  const EmberCoapOption *options;   /*!< Defaults to NULL */
  uint8_t numberOfOptions;          /*!< Defaults to zero */

  uint32_t responseTimeoutMs; /*!< Default is 'EMBER_COAP_DEFAULT_TIMEOUT_MS' */

  const uint8_t *responseAppData;   /*!< Defaults to NULL */
  uint16_t responseAppDataLength;   /*!< Defaults to zero */

  EmberCoapTransmitHandler transmitHandler;  /*!< Defaults to using UDP */
  void *transmitHandlerData;        /*!< Defaults to NULL */
} EmberCoapSendInfo;

/** @brief This function sends a request.
 *
 * Any response is passed to `responseHandler`.
 * For unicast requests, at most one response is processed.
 * For multicast requests, the response handler is called once for each
 * response that arrives before the response timeout.
 *
 * @return `EMBER_SUCCESS` if no errors occured.
 */

EmberStatus emberCoapSend(const EmberIpv6Address *destination,
                          EmberCoapCode code,
                          const uint8_t *path,
                          const uint8_t *payload,
                          uint16_t payloadLength,
                          EmberCoapResponseHandler responseHandler,
                          const EmberCoapSendInfo *info);

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief This function sends a GET request.
 *
 * See `emberCoapSend()` for more information.
 */

EmberStatus emberCoapGet(const EmberIpv6Address *destination,
                         const uint8_t *path,
                         EmberCoapResponseHandler responseHandler,
                         const EmberCoapSendInfo *info);

/** @brief This function sends a PUT request.
 *
 * See `emberCoapSend()` for more information.
 */

EmberStatus emberCoapPut(const EmberIpv6Address *destination,
                         const uint8_t *path,
                         const uint8_t *payload,
                         uint16_t payloadLength,
                         EmberCoapResponseHandler responseHandler,
                         const EmberCoapSendInfo *info);

/** @brief This function sends a POST request.
 *
 * See `emberCoapSend()` for more information.
 */

EmberStatus emberCoapPost(const EmberIpv6Address *destination,
                          const uint8_t *path,
                          const uint8_t *payload,
                          uint16_t payloadLength,
                          EmberCoapResponseHandler responseHandler,
                          const EmberCoapSendInfo *info);

/** @brief This function sends a DELETE request.
 *
 * See `emberCoapSend()` for more information.
 */

EmberStatus emberCoapDelete(const EmberIpv6Address *destination,
                            const uint8_t *path,
                            EmberCoapResponseHandler responseHandler,
                            const EmberCoapSendInfo *info);
#else
// Cosmetic wrappers for emberCoapSend() for the four types of requests.
// GET and DELETE don't have payloads.

#define emberCoapGet(dest, path, handler, info) \
  (emberCoapSend((dest), EMBER_COAP_CODE_GET,   \
                 (path), NULL, 0, (handler), (info)))

#define emberCoapPut(dest, path, pay, len, handler, info) \
  (emberCoapSend((dest), EMBER_COAP_CODE_PUT,             \
                 (path), (pay), (len), (handler), (info)))

#define emberCoapPost(dest, path, pay, len, handler, info) \
  (emberCoapSend((dest), EMBER_COAP_CODE_POST,             \
                 (path), (pay), (len), (handler), (info)))

#define emberCoapDelete(dest, path, handler, info) \
  (emberCoapSend((dest), EMBER_COAP_CODE_DELETE,   \
                 (path), NULL, 0, (handler), (info)))
#endif

//----------------
// Receiving a request

/** @brief Additional information about an incoming request.
 *
 * `transmitHandler` is non-NULL if the request was passed to
 * `emberProcessCoap()`, and will be called to deliver any
 * response.  If it is NULL, the request was an ordinary UDP message
 * and any response is sent using UDP.
 */

typedef struct {
  EmberIpv6Address localAddress;
  EmberIpv6Address remoteAddress;
  uint16_t localPort;
  uint16_t remotePort;

  EmberCoapTransmitHandler transmitHandler;
  void *transmitHandlerData;

  uint8_t token[EMBER_COAP_MAX_TOKEN_LENGTH];
  uint8_t tokenLength;

  void *ackData; /*!< must be NULL when sending a delayed response */
} EmberCoapRequestInfo;

/** @brief Callback for incoming requests.
 *
 * `info` can be passed as-is when sending an immediate response from within
 * the call to `emberCoapRequestHandler()`.  To send
 * a delayed response, `info` data must be copied to a more permanent
 * location using `emberSaveRequestInfo()`.
 */

void emberCoapRequestHandler(EmberCoapCode code,
                             uint8_t *uri,
                             EmberCoapReadOptions *options,
                             const uint8_t *payload,
                             uint16_t payloadLength,
                             const EmberCoapRequestInfo *info);

/** @brief Sending a response.
 */

#ifdef DOXYGEN_SHOULD_SKIP_THIS
EmberStatus emberCoapRespond(const EmberCoapRequestInfo *requestInfo,
                             EmberCoapCode code,
                             const EmberCoapOption *options,
                             uint8_t numberOfOptions,
                             const uint8_t *payload,
                             uint16_t payloadLength);
#else
#define emberCoapRespond(info, code, opts, count, pay, len) \
  (emberCoapRespondWithPath((info), (code), NULL, (opts), (count), (pay), (len)))
#endif

/** @brief Sending a response that includes a location path.
 */

EmberStatus emberCoapRespondWithPath(const EmberCoapRequestInfo *requestInfo,
                                     EmberCoapCode code,
                                     const uint8_t *path,
                                     const EmberCoapOption *options,
                                     uint8_t numberOfOptions,
                                     const uint8_t *payload,
                                     uint16_t payloadLength);

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief Sending a response that consists of just a code.
 */
EmberStatus emberCoapRespondWithCode(const EmberCoapRequestInfo *requestInfo,
                                     EmberCoapCode code);
/** @brief Sending a response that consists of a code and a payload.
 */
EmberStatus emberCoapRespondWithPayload(const EmberCoapRequestInfo *requestInfo,
                                        EmberCoapCode code,
                                        const uint8_t *payload,
                                        uint16_t payloadLength);
#else
#define emberCoapRespondWithCode(info, code) \
  (emberCoapRespond((info), (code), NULL, 0, NULL, 0))

#define emberCoapRespondWithPayload(info, code, payload, payloadLength) \
  (emberCoapRespond((info), (code), NULL, 0, (payload), (payloadLength)))
#endif

/** @brief This function saves a `EmberCoapRequestInfo` for later use.
 *
 * This saves necesary fields from `from` to `to` so that `to` may later
 * be used to send a response.
 */
void emberSaveRequestInfo(const EmberCoapRequestInfo *from,
                          EmberCoapRequestInfo *to);

/** @brief This function processes a CoAP message received over an alternate transport.
 *
 * Called to process a CoAP message that arrives via DTLS or other
 * alternative transport.  Only the address, port, and transmit handler
 * fields of `info` are used.  The token and ackData fields are
 * ignored.
 */
void emberProcessCoap(const uint8_t *message,
                      uint16_t messageLength,
                      EmberCoapRequestInfo *info);

typedef struct {
  bool more;
  uint8_t logSize;
  uint32_t number;
} EmberCoapBlockOption;

#define emberBlockOptionSize(option) (1 << (option)->logSize)

uint32_t emberBlockOptionOffset(EmberCoapBlockOption *option);

bool emberReadBlockOption(EmberCoapReadOptions *options,
                          EmberCoapOptionType type,
                          EmberCoapBlockOption *option);

void emberParseBlockOptionValue(uint32_t value,
                                EmberCoapBlockOption *option);

uint32_t emberBlockOptionValue(bool more, uint8_t logSize, uint32_t number);

void emberInitCoapOption(EmberCoapOption *option,
                         EmberCoapOptionType type,
                         uint32_t value);

bool emberVerifyBlockOption(const EmberCoapBlockOption *blockOption,
                            uint16_t payloadLength,
                            uint8_t expectedLogSize);

EmberStatus emberCoapRequestNextBlock(EmberCoapCode code,
                                      const uint8_t *path,
                                      EmberCoapBlockOption *block2Option,
                                      EmberCoapResponseHandler responseHandler,
                                      EmberCoapResponseInfo *responseInfo);

/** @}*/

#endif // COAP_H
