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

void ALIAS(emberSlaacServerChangeHandler)(const uint8_t * prefix,
                                          uint8_t prefixLengthInBits,
                                          bool available)
{
  if (available) {
    emberAfCorePrint("Requesting SLAAC address in ");
    emberAfCoreDebugExec(emberAfPrintIpv6Prefix((const EmberIpv6Address *)prefix,
                                                prefixLengthInBits));
    emberAfCorePrintln(" prefix");
    emberRequestSlaacAddress(prefix, prefixLengthInBits);
  }
}

void ALIAS(emberRequestSlaacAddressReturn)(EmberStatus status,
                                           const uint8_t * prefix,
                                           uint8_t prefixLengthInBits)
{
  if (status != EMBER_SUCCESS) {
    emberAfCorePrint("ERR: Requesting SLAAC address in ");
    emberAfCoreDebugExec(emberAfPrintIpv6Prefix((const EmberIpv6Address *)prefix,
                                                prefixLengthInBits));
    emberAfCorePrintln(" prefix failed: 0x%x", status);
  }
}
