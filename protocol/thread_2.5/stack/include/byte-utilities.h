/**
 * @file byte-utilities.h
 * @brief Data store and fetch routines.
 *
 * <!--Copyright 2013 by Silicon Labs. All rights reserved.              *80*-->
 */
#ifndef __BYTE_UTILITIES_H_
#define __BYTE_UTILITIES_H_

/** @brief This function converts from a number of bits to the equivalent number of bytes.
 *
 * @param bits The number of bits.
 */
#define EMBER_BITS_TO_BYTES(bits) (((bits) + 7) >> 3)

/** @brief This function copies an array of bytes and reverses the order
 *  before writing the data to the destination.
 *
 * @param dest A pointer to the location where the data will be copied to.
 * @param src A pointer to the location where the data will be copied from.
 * @param length The length (in bytes) of the data to be copied.
 */
void emberReverseMemCopy(uint8_t* dest, const uint8_t* src, uint8_t length);

/** @brief This function returns the value built from the two \c uint8_t values
 *  \c contents[0] and \c contents[1]. \c contents[0] is the low byte.
 */
uint16_t emberFetchLowHighInt16u(const uint8_t *contents);

/** @brief This function returns the value built from the two \c uint8_t values
 *  \c contents[0] and \c contents[1]. \c contents[1] is the low byte.
 */
uint16_t emberFetchHighLowInt16u(const uint8_t *contents);

/** @brief This function stores \c value in \c contents[0] and \c contents[1]. \c
 *  contents[0] is the low byte.
 */
void emberStoreLowHighInt16u(uint8_t *contents, uint16_t value);

/** @brief This function stores \c value in \c contents[0] and \c contents[1]. \c
 *  contents[1] is the low byte.
 */
void emberStoreHighLowInt16u(uint8_t *contents, uint16_t value);

#if !defined DOXYGEN_SHOULD_SKIP_THIS
uint32_t emFetchInt32u(bool lowHigh, const uint8_t* contents);
#endif

/** @brief This function returns the value built from the four \c uint8_t values
 *  \c contents[0], \c contents[1], \c contents[2] and \c contents[3]. \c
 *  contents[0] is the low byte.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
uint32_t emberFetchLowHighInt32u(uint8_t *contents);
#else
#define emberFetchLowHighInt32u(contents) \
  (emFetchInt32u(true, contents))
#endif

/** @description This function returns the value built from the four \c uint8_t values
 *  \c contents[0], \c contents[1], \c contents[2] and \c contents[3]. \c
 *  contents[3] is the low byte.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
uint32_t emberFetchHighLowInt32u(uint8_t *contents);
#else
#define emberFetchHighLowInt32u(contents) \
  (emFetchInt32u(false, contents))
#endif

#if !defined DOXYGEN_SHOULD_SKIP_THIS
void emStoreInt32u(bool lowHigh, uint8_t* contents, uint32_t value);
#endif

/** @brief This function stores \c value in \c contents[0], \c contents[1], \c
 *  contents[2] and \c contents[3]. \c contents[0] is the low byte.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
void emberStoreLowHighInt32u(uint8_t *contents, uint32_t value);
#else
#define emberStoreLowHighInt32u(contents, value) \
  (emStoreInt32u(true, contents, value))
#endif

#if !defined DOXYGEN_SHOULD_SKIP_THIS
void emStoreInt48u(bool lowHigh, uint8_t* contents, uint64_t value);
#endif

/** @description This function stores \c value in \c contents[0], \c contents[1], \c
 *  contents[2] and \c contents[3]. \c contents[3] is the low byte.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
void emberStoreHighLowInt32u(uint8_t *contents, uint32_t value);
#else
#define emberStoreHighLowInt32u(contents, value) \
  (emStoreInt32u(false, contents, value))
#endif

#if !defined DOXYGEN_SHOULD_SKIP_THIS
uint64_t emFetchInt48u(bool lowHigh, const uint8_t* contents);
#endif

/** @description This function stores \c value in \c contents[0] thru \c contents[5]. \c
 * contents[5] is the low byte.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
void emberStoreHighLowInt48u(uint8_t *contents, uint32_t value);
#else
#define emberStoreHighLowInt48u(contents, value) \
  (emStoreInt48u(false, contents, value))
#endif

/** @description This function returns the value built from the six \c uint8_t values
 *  \c contents[0] thru \c contents[5]. \c contents[5] is the low bytes.
 */
#if defined DOXYGEN_SHOULD_SKIP_THIS
uint64_t emberFetchHighLowInt48u(uint8_t *contents);
#else
#define emberFetchHighLowInt48u(contents) \
  (emFetchInt48u(false, contents))
#endif

uint16_t emStrlen(const uint8_t * const string);

int8_t emStrcmp(const uint8_t *s1, const uint8_t *s2);

/** @brief This function returns the length in bits of the matching prefix
 * of \c x and \c y.
 */
uint16_t emMatchingPrefixBitLength(const uint8_t *x, uint16_t xLength,
                                   const uint8_t *y, uint16_t yLength);

/** @brief This function copies \c count bits from \c from to \c to.
 */
void emBitCopy(uint8_t *to, const uint8_t *from, uint16_t count);

/** @brief This function returns the number of set bits in \c num.
 */
uint8_t emBitCountInt32u(uint32_t num);

/** @brief This function returns the value of hexadecimal digit ch (0 - 15).
 * This function returns a value > 15 if ch is not a hexadecimal digit.
 */
uint8_t emberHexToInt(uint8_t ch);

#endif
