// Copyright 2015 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_COAP_DISPATCH
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

const uint8_t *emGetCoapCodeName(EmberCoapCode type);

void emberCoapRequestHandler(EmberCoapCode code,
                             uint8_t *uri,
                             EmberCoapReadOptions *options,
                             const uint8_t *payload,
                             uint16_t payloadLength,
                             const EmberCoapRequestInfo *info)
{
  emberAfPluginCoapDispatchPrintln("Request: %s %s",
                                   emGetCoapCodeName(code),
                                   uri);

  size_t uriLength = strlen((const char *)uri);
  for (const EmberAfCoapDispatchEntry *entry = emberAfCoapDispatchTable;
       entry->uri != NULL;
       entry++) {
    // If the user specified something like "prefix/" for the URI, then the
    // incoming URIs "prefix" and "prefix/..." will both match.  Otherwise, an
    // exact match is required, so "notaprefix" won't match "notaprefixeither"
    // or "notaprefix/...."  In either case, the method must match or must have
    // been specified as ANY.
    bool prefixMatch = (entry->uri[entry->uriLength - 1] == '/');
    uint16_t entryUriLength = (prefixMatch
                               ? entry->uriLength - 1
                               : entry->uriLength);
    if ((entry->method == EMBER_AF_COAP_DISPATCH_METHOD_ANY
         || entry->method == code)
        && (prefixMatch
            ? (entryUriLength <= uriLength
               && (uri[entryUriLength] == '\0' || uri[entryUriLength] == '/'))
            : entryUriLength == uriLength)
        && strncmp(entry->uri, (const char *)uri, entryUriLength) == 0) {
      emberAfPluginCoapDispatchPrintln("Found handler for request at index %lu",
                                       entry - emberAfCoapDispatchTable);
      (*entry->handler)(code, uri, options, payload, payloadLength, info);
      return;
    }
  }

  emberAfPluginCoapDispatchPrintln("No handler found for request");
  emberCoapRespondWithCode(info, EMBER_COAP_CODE_404_NOT_FOUND);
}
