// File: tmsp-frame-utilities.h
//
// Description: Functions for reading and writing command and response frames.
//
// Copyright 2015 Silicon Laboratories, Inc.                                *80*

#ifndef __TMSP_FRAME_UTILITIES_H__
#define __TMSP_FRAME_UTILITIES_H__

uint8_t emberIpv6AddressCommandArgument(uint8_t argNum, EmberIpv6Address *address);
uint8_t emberKeyDataCommandArgument(uint8_t argNum, EmberKeyData *key);

#endif // __TMSP_FRAME_UTILITIES_H__
