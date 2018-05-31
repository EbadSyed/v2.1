// File: thread-app-stubs.c
//
// Description: Temporary stubs.  These will go away in future builds.
//
// Copyright 2012 by Ember Corporation. All rights reserved.                *80*

#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "hal/hal.h"
#include "hal/micro/bootloader-interface-app.h"

#ifdef EMBER_STACK_COBRA
void halButtonIsr(uint8_t button, uint8_t state)
{
}
void emButtonTick(void)
{
}
#endif

#if (defined(UNIX_HOST) || defined(UNIX_HOST_SIM))
bool emForceParentId = false;
#else
bool emUseLongMacAddress = false;
uint8_t defaultPrefix[8];
#endif

#if (defined(UNIX_HOST) && !defined(UNIX_HOST_SIM))
uint8_t halAppBootloaderWriteRawStorage(uint32_t address,
                                        const uint8_t *data,
                                        uint16_t len)
{
  return EEPROM_SUCCESS;
}
#endif

#ifdef UNIX_HOST_SIM
void halInternalStartSymbolTimer(void)
{
}
#endif

void emberStateReturn(const EmberNetworkParameters *parameters,
                      const EmberEui64 *eui64,
                      const EmberEui64 *extendedMacId,
                      EmberNetworkStatus networkStatus)
{
}

void emberSetPskcHandler(const uint8_t *pskc)
{
}

void emApiSetDtlsDeviceCertificateReturn(uint32_t result)
{
}
void emApiSetDtlsPresharedKeyReturn(EmberStatus status)
{
}
void emApiOpenDtlsConnectionReturn(uint32_t result,
                                   const EmberIpv6Address *remoteAddress,
                                   uint16_t localPort,
                                   uint16_t remotePort)
{
}
void emApiDtlsSecureSessionEstablished(uint8_t flags,
                                       uint8_t sessionId,
                                       const EmberIpv6Address *localAddress,
                                       const EmberIpv6Address *remoteAddress,
                                       uint16_t localPort,
                                       uint16_t remotePort)
{
}
void emApiGetSecureDtlsSessionIdReturn(uint8_t sessionId,
                                       const EmberIpv6Address *remoteAddress,
                                       uint16_t localPort,
                                       uint16_t remotePort)
{
}
void emApiCloseDtlsConnectionReturn(uint8_t sessionId, EmberStatus status)
{
}
