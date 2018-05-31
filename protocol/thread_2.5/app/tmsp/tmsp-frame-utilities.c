// File: tmsp-frame-utilities.c
//
// Description: Functions for reading and writing command and response frames.
//
// Copyright 2015 Silicon Laboratories, Inc.                                *80*

#include PLATFORM_HEADER
#include "include/ember.h"
#include "app/ip-ncp/binary-management.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/tmsp/tmsp-enum.h"
#include "app/tmsp/tmsp-frame-utilities.h"

uint8_t emberIpv6AddressCommandArgument(uint8_t argNum, EmberIpv6Address *address)
{
  return emberGetStringArgument(argNum, address->bytes, 16, false);
}

uint8_t emberKeyDataCommandArgument(uint8_t argNum, EmberKeyData *key)
{
  uint8_t keyLength = emberGetStringArgument(argNum, key->contents, EMBER_ENCRYPTION_KEY_SIZE, false);
  assert(keyLength == EMBER_ENCRYPTION_KEY_SIZE);
  return keyLength;
}
