/*
 * File: ecc-mbedtls-util.h
 * Description: Utility functions for ECC calls into mbed TLS.
 *
 * Copyright 2017 by Silicon Laboratories. All rights reserved.             *80*
 */

#ifndef ECC_MBEDTLS_UTIL_H
#define ECC_MBEDTLS_UTIL_H

int emGetRandomBytes(void *token, unsigned char *result, size_t resultLength);
void emSetWatchdog(bool on);

#endif // ECC_MBEDTLS_UTIL_H
