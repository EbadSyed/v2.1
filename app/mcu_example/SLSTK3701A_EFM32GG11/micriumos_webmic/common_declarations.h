/**************************************************************************//**
 * @file common_declarations.h
 * @brief Common values shared in this application
 * @version 5.3.5
 *******************************************************************************
 * # License
 * <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#ifndef COMMON_DECLARATION_H
#define COMMON_DECLARATION_H

#include <stdint.h>
#include <rtos/kernel/include/os.h>
#include "ogg/ogg.h"

// -----------------------------------------------------------------------------
// Define constants

#define  NR_CLIENT_MAX     5u // Increased number MAY introduce latency

#define  READ_BUF_ONE      (OS_FLAGS)0x0002
#define  READ_BUF_TWO      (OS_FLAGS)0x0004

#define  TIME_MS           40u // Max 60ms. See IETF rfc6716
#define  EX_SAMPLE_FREQ    48000u // Max 48kHz. See IETF rfc6716
#define  SAMPLE_BUFFER_LEN (EX_SAMPLE_FREQ * TIME_MS / 1000)
#define  MIC_CH            MIC_CH1 // Select right or left mic
#define  CHANNELS          MONO // NB! Stereo is not implemented
#define  BIT_RATE          72000

#define  SERIALNO          1234 // Can be anything you want
#define  DC_CORRECTION     1900u // DC offset in mic samples

// -----------------------------------------------------------------------------
// Macros

#if defined (__ICCARM__)
   #define AEM_PACKED_STRUCT __packed struct
#elif defined (__clang__)
   #define AEM_PACKED_STRUCT struct __attribute__ ((__packed__))
#elif defined (__GNUC__)
   #define AEM_PACKED_STRUCT struct __attribute__ ((__packed__))
#endif

// -----------------------------------------------------------------------------
// Data types

enum mic_channels {
  MIC_CH1 = 0,
  MIC_CH2 = 1
};

enum sound_channels {
  MONO = 1,
  STEREO = 2
};

typedef struct MIC_Data {
  ogg_page *oggPage;
  uint16_t *pcmBuf;
  // Possibility for other fields to be shared
} MIC_Data_Typedef;

// NB! Opus fields is little endian
typedef AEM_PACKED_STRUCT {
  uint8_t magic_num[4];
  uint8_t version;
  uint8_t type;
  uint8_t granule[8];
  uint32_t serial;
  uint32_t sequence;
  uint32_t checksum;
  uint8_t segments;
  uint8_t size; // WARNING! This assumes just 1 segment
} OGG_PageHeader_TypeDef;

typedef AEM_PACKED_STRUCT {
  char signature[8];
  uint8_t version;
  uint8_t channels;
  uint16_t preskip;
  uint32_t samplerate;
  uint16_t gain;
  uint8_t chmap;
} OPUS_IdHeader_TypeDef;

typedef AEM_PACKED_STRUCT {
  char signature[8];
  uint32_t vendor_len; // WARNING! Must be 0 (see IETF rfc7845 section 5.)
  //char vendor_string[]; don't need this
  uint32_t list_len; // WARNING! Must be 0 (see IETF rfc7845 section 5.)
  //char user_comment[]; don't need this
} OPUS_CommentHeader_TypeDef;

// -----------------------------------------------------------------------------
// Global variables

extern OS_MUTEX    audioMutex;
extern OS_FLAG_GRP micFlags;
extern OS_FLAG_GRP bufferFlags;

extern MIC_Data_Typedef micData;

#endif // COMMON_DECLARATION_H