/*
 * File: jpake-ecc.h
 * Description: J-PAKE header file.
 *
 * Author(s): Maurizio Nanni
 *
 * Copyright 2014 by Silicon Laboratories. All rights reserved.             *80*
 */

#ifndef JPAKE_ECC_H
#define JPAKE_ECC_H

#define ECC_PUBLIC_KEY_MAX_LENGTH        65
#define ECC_ZKP_SIGNATURE_MAX_LENGTH     SHA256_BLOCK_SIZE

#define ECC_ZKP_MAX_DATA_LENGTH          (1 + ECC_PUBLIC_KEY_MAX_LENGTH + 1 + ECC_ZKP_SIGNATURE_MAX_LENGTH)
#define ECC_JPAKE_MAX_DATA_LENGTH        (1 + ECC_PUBLIC_KEY_MAX_LENGTH + ECC_ZKP_MAX_DATA_LENGTH)
#define ECC_JPAKE_CURVE_PARAMS_LENGTH    3

// jPakeData bitmask definitions
#define JPAKE_DATA_INITIALIZED           0x01
#define JPAKE_DATA_HX1_HX2_VALID         0x02
#define JPAKE_DATA_HX3_HX4_VALID         0x04
#define JPAKE_DATA_XCS_VALID             0x08

// This API initializes the ECC JPAKE internal state.
void emJpakeEccStart(bool isClient,
                     const uint8_t *sharedSecret,
                     const uint8_t sharedSecretLength);

// This API concludes the JPAKE process and computes the resulting
// pre-master secret. The pms array must be at least SHA256_BLOCK_SIZE
// bytes in size. If the  pre-master secret was computed successfully,
// it returns true, false otherwise.
bool emJpakeEccFinish(uint8_t *ms, uint16_t len, uint16_t *olen);

// This API writes the (HX1,HX2) ClientHello JPAKE extensions in the passed
// buf array if the node is a client. It writes the (HX3,HX4) ServerHello
// extensions in the passed buf array if the node is a server.
// The buf array must be at least 2 * ECC_JPAKE_MAX_DATA_LENGTH bytes in size.
// It returns true if the requested data was computed successfully , false
// otherwise.
bool emJpakeEccGetHxaHxbData(uint8_t *buf, uint16_t len, uint16_t *olen);

// This API takes as parameters the (HX1,HX2) ClientHello extensions if the
// originator node is a client, while it takes as parameters the (HX3,HX4)
// ServerHello extensions if the originator node is a server.
// It returns true if the passed extensions verify, false otherwise.
bool emJpakeEccVerifyHxaHxbData(uint8_t *buf, uint16_t len);

// This API writes the CKXA (if the node is a client) or the SKXB (if the node
// is a server) JPAKE extension in the passed array.
// The buf array must be at least ECC_JPAKE_MAX_DATA_LENGTH bytes in size.
bool emJpakeEccGetCkxaOrSkxbData(uint8_t *buf, uint16_t len, uint16_t *olen);

// This API takes as parameter the CKXA (if the originator node is a client) or
// the SKXB (if the originator node is a server) JPAKE extension.
// It returns true if the passed extension verifies, false otherwise.
bool emJpakeEccVerifyCkxaOrSkxbData(uint8_t *buf, uint16_t len);

// Mark the buffers allocated by JPAKE
void emJpakeEccMarkData(void);

//------------------------------------------------------------------------------
// Test stuff

#if defined(EMBER_SCRIPTED_TEST)
void setRandomDataType(bool isClient);
#else
#define setRandomDataType(isClient)
#endif // EMBER_SCRIPTED_TEST

#endif // JPAKE_ECC_H
