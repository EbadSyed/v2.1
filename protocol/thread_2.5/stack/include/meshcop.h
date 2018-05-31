// File: meshcop.h
//
// Description: Header with common APIs and defines for Thread
// Mesh Commisioning Protocol (MeshCop)
//
// Copyright 2016 by Silicon Laboratories. All rights reserved.             *80*
//----------------------------------------------------------------
#ifndef __MESHCOP_H__
#define __MESHCOP_H__
// Thread Version currently used for advertising commissioning protocol
#define MESHCOP_THREAD_VERSION_STRING "1.1.0"

// Pseudo-radio messages start with a byte that both distinguishes them from
// non-radio DTLS messages, which start with a DTLS command byte, and say
// which key was intended.  No MAC security is actually used.
#define MAC_KEY_BYTE            0xFF
#define JOINER_ENTRUST_KEY_BYTE 0xFE
#define MIN_KEY_BYTE            JOINER_ENTRUST_KEY_BYTE

//State 'Bitmap,' see Thread 1.1 specification, table 8-5
#define SB_CONNECTION_MODE_OFFSET 0
#define SB_CONNECTION_MODE_BITS 3
#define SB_CONNECTION_MODE_MSK 0x00000007
#define SB_CONNECTION_MODE_NO_DTLS 0
#define SB_CONNECTION_MODE_USER_CREDENTIAL 1
#define SB_CONNECTION_MODE_BORDER_AGENT_DEVICE_CREDENTIAL 2
#define SB_CONNECTION_MODE_VENDOR_CREDENTIAL 3

#define SB_THREAD_IF_STATUS_OFFSET (SB_CONNECTION_MODE_BITS)
#define SB_THREAD_IF_STATUS_BITS 2
#define SB_THREAD_IF_STATUS_MSK 0x00000018
#define SB_THREAD_IF_STATUS_INACTIVE 0
#define SB_THREAD_IF_STATUS_ACTIVE_WITHOUT_THREAD_PARTITION 1
#define SB_THREAD_IF_STATUS_ACTIVE_WITH_THREAD_PARTITION 2

#define SB_AVAILABILITY_OFFSET (SB_CONNECTION_MODE_BITS + SB_THREAD_IF_STATUS_BITS)
#define SB_AVAILABILITY_BITS 2
#define SB_AVAILABILITY_MSK 0x00000060
#define SB_AVAILABILITY_INFREQUENT 0
#define SB_AVAILABILITY_HIGH 1

typedef union {
  struct {
    uint32_t connectionMode : 3;
    uint32_t threadIfStatus : 2;
    uint32_t availability : 2;
    uint32_t reserved : 25;
  };
  uint8_t bytes[4];
  uint32_t u32;
} MeshCopStateBitmap;

#endif //__MESHCOP_H__
