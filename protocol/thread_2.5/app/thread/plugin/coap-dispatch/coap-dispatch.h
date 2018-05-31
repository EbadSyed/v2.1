// Copyright 2015 Silicon Laboratories, Inc.

#ifndef __COAP_DISPATCH_H__
#define __COAP_DISPATCH_H__

#include PLATFORM_HEADER
#include EMBER_AF_API_STACK

/** @brief Signature for a function that is called to handle a request.
 *
 * @param request The request information
 */
typedef void EmberAfCoapDispatchHandler (EmberCoapCode code,
                                         uint8_t *uri,
                                         EmberCoapReadOptions *options,
                                         const uint8_t *payload,
                                         uint16_t payloadLength,
                                         const EmberCoapRequestInfo *info);

/** @brief Defines the possible request methods. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberAfCoapDispatchMethod
#else
typedef uint8_t EmberAfCoapDispatchMethod;
enum
#endif
{
  /** Wildcard matching all methods. */
  EMBER_AF_COAP_DISPATCH_METHOD_ANY    = EMBER_COAP_CODE_EMPTY,
  /** The GET method retrieves a representation for the information that
   *  currently corresponds to the resource identified by the request URI. */
  EMBER_AF_COAP_DISPATCH_METHOD_GET    = EMBER_COAP_CODE_GET,
  /** The POST method requests that the representation enclosed in the request
   *  be processed. */
  EMBER_AF_COAP_DISPATCH_METHOD_POST   = EMBER_COAP_CODE_POST,
  /** The PUT method requests that the resource identified by the request
   *  URI be updated or created with the enclosed representation. */
  EMBER_AF_COAP_DISPATCH_METHOD_PUT    = EMBER_COAP_CODE_PUT,
  /** The DELETE method requests that the resource identified by the request
   *  URI be deleted. */
  EMBER_AF_COAP_DISPATCH_METHOD_DELETE = EMBER_COAP_CODE_DELETE,
};

/** @brief A mapping between a request (method + URI) and a handler for that
 * request
 */
typedef struct {
  EmberAfCoapDispatchMethod method;
  const char *uri;
  uint16_t uriLength;
  EmberAfCoapDispatchHandler *handler;
} EmberAfCoapDispatchEntry;

/** @brief The mapping table. It must contain all mappings that are to be
 * handled by this plugin, and it must be terminated with an all-zero element.
 *
 * @appusage Must be provided by the application.
 */
extern const EmberAfCoapDispatchEntry emberAfCoapDispatchTable[];

#endif
