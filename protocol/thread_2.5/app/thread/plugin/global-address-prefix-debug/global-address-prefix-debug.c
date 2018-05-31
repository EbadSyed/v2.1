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

void ALIAS(emberGetGlobalAddressReturn)(const EmberIpv6Address * address,
                                        uint32_t preferredLifetime,
                                        uint32_t validLifetime,
                                        uint8_t addressFlags)
{
  emberAfCorePrint("global address: ");
  emberAfCoreDebugExec(emberAfPrintIpv6Address(address));
  emberAfCorePrintln("");
}

void ALIAS(emberGetGlobalPrefixReturn)(uint8_t borderRouterFlags,
                                       bool isStable,
                                       const uint8_t * prefix,
                                       uint8_t prefixLengthInBits,
                                       uint8_t domainId,
                                       uint32_t preferredLifetime,
                                       uint32_t validLifetime)
{
  emberAfCorePrint("global prefix: ");
  emberAfCoreDebugExec(emberAfPrintIpv6Prefix((const EmberIpv6Address *)prefix,
                                              prefixLengthInBits));
  emberAfCorePrintln("");
}
