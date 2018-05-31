// Copyright 2015 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

#ifndef ALIAS
  #define ALIAS(x) x
#endif

const uint8_t *emGetCoapCodeName(EmberCoapCode type);
const uint8_t *emGetCoapContentFormatTypeName(EmberCoapContentFormatType type);

void ALIAS(emberCoapRequestHandler)(EmberCoapCode code,
                                    uint8_t * uri,
                                    EmberCoapReadOptions * options,
                                    const uint8_t * payload,
                                    uint16_t payloadLength,
                                    const EmberCoapRequestInfo * info)
{
  emberAfCorePrint("CoAP RX:");

  emberAfCorePrint(" s=");
  emberAfCoreDebugExec(emberAfPrintIpv6Address(&info->remoteAddress));

  emberAfCorePrint(" %s", emGetCoapCodeName(code));

  emberAfCorePrint(" u=%s", uri);

  if (info->tokenLength != 0) {
    emberAfCorePrint(" t=");
    emberAfCorePrintBuffer(info->token, info->tokenLength, false);
  }

  if (payloadLength != 0) {
    uint32_t valueLoc;
    EmberCoapContentFormatType contentFormat
      = (emberReadIntegerOption(options,
                                EMBER_COAP_OPTION_CONTENT_FORMAT,
                                &valueLoc)
         ? (EmberCoapContentFormatType)valueLoc
         : EMBER_COAP_CONTENT_FORMAT_NONE);
    emberAfCorePrint(" f=%s p=", emGetCoapContentFormatTypeName(contentFormat));
    emberAfCorePrintBuffer(payload, payloadLength, false);
  }

  emberAfCorePrintln("");
}
