// Copyright 2015 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_COMMAND_INTERPRETER2
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

#include "app/coap/coap.h"

bool secureDtlsSession = false;
uint8_t secureDtlsSessionId = 0;

//----------------------------------------------------------------------------//

// dtls certs [no arguments]
void dtlsSetCertsCommand(void)
{
  emberSetDtlsDeviceCertificate(NULL, NULL); // Use compiled certificates
}

// dtls psk <ip> <psk>
void dtlsSetPskCommand(void)
{
  EmberIpv6Address address;
  if (emberGetIpArgument(0, (uint8_t *) &address.bytes)) {
    uint8_t keyLength;
    uint8_t *key = emberStringCommandArgument(1, &keyLength);
    emberSetDtlsPresharedKey((const uint8_t *) key,
                             keyLength,
                             &address);
  }
}

// dtls open <mode> <ip> <port>
void dtlsOpenCommand(void)
{
  uint8_t mode = emberUnsignedCommandArgument(0);
  EmberIpv6Address address;
  if (emberGetIpArgument(1, (uint8_t *) &address.bytes)) {
    uint16_t port = emberUnsignedCommandArgument(2);
    emberOpenDtlsConnection(mode, &address, port, port);
    emberAfAppPrintln("DTLS open.");
  }
}

void emberSetDtlsDeviceCertificateReturn(uint32_t result)
{
  if (result == EMBER_SUCCESS) {
    emberAfAppPrintln("set certificates.");
  } else {
    emberAfAppPrintln("something went wrong in set certificates, mbedtls err: %d",
                      result);
  }
}

void emberOpenDtlsConnectionReturn(uint32_t result,
                                   const EmberIpv6Address *remoteAddress,
                                   uint16_t localPort,
                                   uint16_t remotePort)
{
  if (result != 0) {
    emberAfAppPrintln("something went wrong in creating a dtls session, mbedtls err: %d",
                      result);
  }
}

void emberDtlsSecureSessionEstablished(uint8_t flags,
                                       uint8_t sessionId,
                                       const EmberIpv6Address *localAddress,
                                       const EmberIpv6Address *remoteAddress,
                                       uint16_t localPort,
                                       uint16_t remotePort)
{
  emberAfAppPrintln("secure session available: %d (%s)",
                    sessionId,
                    flags ? "server" : "client");
  secureDtlsSession = true;
  secureDtlsSessionId = sessionId;
}

void emberProcessCoap(const uint8_t *message,
                      uint16_t messageLength,
                      EmberCoapRequestInfo *info)
{
  if (messageLength > 0) {
    if (secureDtlsSession) {
      info->transmitHandlerData = (void *) (uint32_t) secureDtlsSessionId;
      info->transmitHandler = &emberDtlsTransmitHandler;
    }

    emProcessCoapMessage(NULL_BUFFER,
                         message,
                         messageLength,
                         (CoapRequestHandler) & emberCoapRequestHandler,
                         info);
  } else {
    emberAfAppPrintln("failed to parse secure data.");
  }
}

//----------------------------------------------------------------------------//

// dtls session <ip> <port>
void dtlsSessionCommand(void)
{
  EmberIpv6Address address;
  if (emberGetIpArgument(0, (uint8_t *) &address.bytes)) {
    uint16_t port = emberUnsignedCommandArgument(1);
    emberGetSecureDtlsSessionId(&address, port, port);
    emberAfAppPrintln("session id");
  }
}

void emberGetSecureDtlsSessionIdReturn(uint8_t sessionId,
                                       const EmberIpv6Address *remoteAddress,
                                       uint16_t localPort,
                                       uint16_t remotePort)
{
  emberAfAppPrintln("sessionId: %d", sessionId);
}

//----------------------------------------------------------------------------//

// dtls close <session_id>
void dtlsCloseCommand(void)
{
  uint8_t sessionId = emberUnsignedCommandArgument(0);
  emberCloseDtlsConnection(sessionId);
  emberAfAppPrintln("DTLS close.");
}

void emberCloseDtlsConnectionReturn(uint8_t sessionId, EmberStatus status)
{
  if (status == EMBER_SUCCESS) {
    emberAfAppPrintln("closed session %d successfully.", sessionId);
    secureDtlsSession = false;
  } else {
    emberAfAppPrintln("failure to close session status: %d", status);
  }
}
