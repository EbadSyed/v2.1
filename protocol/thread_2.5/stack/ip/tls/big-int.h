/*
 * File: big-int.h
 * Description: Big integer operations used by TLS.
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef BIG_INT_H
#define BIG_INT_H

void emBigIntInitialize(bool free, BigInt *x, ...);
#define emInitBigInts(x, ...) (emBigIntInitialize(false, (x), ##__VA_ARGS__))
#define emFreeBigInts(x, ...) (emBigIntInitialize(true, (x), ##__VA_ARGS__))

int emBigIntSize(const BigInt *x);
int emReadBigIntBytes(BigInt *x, const uint8_t *bytes, uint16_t length);
int emWriteBigIntBytes(const BigInt *x, uint8_t *bytes, uint16_t length);

// Macro weirdness to make it easier to trace who is allocating BigInts.
#define emCopyBigInt(x, y) emCopyBigIntx(x, y, __FILE__, __LINE__)
int emCopyBigIntx(BigInt *x, const BigInt *y, char *filename, int line);

int emSetBigIntFromInt(BigInt *x, int z);
int emBigIntCompare(const BigInt *x, const BigInt *y);
int emBigIntShiftRight(BigInt *x, int count);

// x = a^e mod n via sliding-window exponentiation.
// rRModNCache is used as a cache to share r*r mod n between multiple
// computations, where r = 1 << (emBigIntSize(n) * 8)
// It is either:
//    NULL                            // compute r*r mod n but don't cache it
//    a BigInt with NULL digits       // compute r*r mod n and cache it
//    a BitInt with non-null digits   // used cached r*r mod n

int emBigIntExpMod(BigInt *x,
                   const BigInt *a,
                   const BigInt *e,
                   const BigInt *n,
                   BigInt *rRModNCache);

#endif // BIG_INT_H
