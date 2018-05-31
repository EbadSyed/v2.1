/**
 * @file aes.h
 * @brief AES crypto routines.
 *
 * <!--Copyright 2015 by Silicon Labs.                                   *80*-->
 */
#ifndef __AES_H__
#define __AES_H__

/**
 * @addtogroup Aes
 *
 * See aes.h for source code.
 * @{
 */

//-----------------------------------------------------------------------------
// New Public API

/** @brief The number of bytes in a 128-bit AES block.
 */
#define EMBER_AES_BLOCK_SIZE_BYTES 16 // 128 bits

/** @brief This function performs a standalone-mode "electronic code book"
 *  (ECB) AES-128 encryption of the 16-byte plaintext \c block using the
 *  128-bit (16-byte) \c key.  The resulting 16 byte ciphertext overwrites the
 *  plaintext \c block.
 *
 * @param block A pointer to the 128-bit data in RAM to be encrypted in place.
 * @param key A pointer to the 128-bit key to be used for the encryption.
 * @param sameKey If true, indicates that the 128-bit \c key value is the same
 * as it was in a prior call to this routine and serves as a hint that the
 * key needn't be reloaded into the AES hardware engine.  Otherwise, the \c key
 * value is considered new and will always be loaded.
 */
void emberAesEcbEncryptBlock(uint8_t* block, const uint8_t* key, bool sameKey);

/** @brief This function performs a counter-mode (CTR) AES-128 encrypt/decrypt
 *  of the \c data for \c dataLen bytes, using the 128-bit (16-byte) \c key
 *  and 128-bit (16-byte) \c nonce.  The resulting encrypted/decrypted data
 *  overwrites the \c data passed in.
 *
 * @param nonce The big-endian nonce (MSB is nonce[0] and LSB is nonce[15])
 * serves as a 128-bit block counter for every 16-byte block of \c data.
 * It is incremented by the number of blocks processed ((dataLen+15)/16).
 * @param key A pointer to the 128-bit key to be used for the nonce encryption.
 * @param data A pointer to the plain- or cypher-text to be encrypted/decrypted
 * in place.
 * @param dataLen Indicates the number of bytes of data.  It need not be a
 * multiple of 16 bytes.
 * @param dataDid This parameter allows splitting a CTR operation across
 * multiple calls.  The first call passes in \c dataDid of 0 to start a fresh
 * CTR. Then subsequent calls pass in \c dataDid of the sum of the previous
 * calls' \c dataLen values (with \c data and \c dataLen representing the new
 * portion to encrypt/decrypt).  A non-zero \c dataDid indicates a
 * continuation of the prior CTR operation which will pick up where the
 * earlier one left off.
 *
 * @note If your \c nonce is divided into a fixed and counter portion, ensure
 * that the counter value passed in is such that when incremented by the
 * number of blocks ((dataLen+15)/16) it won't overflow the counter portion
 * into the fixed portion of the nonce.  It may be necessary to split the
 * operation across multiple calls to emberAesCtrCryptData() to satisfy
 * this criteria.
 */
void emberAesCtrCryptData(uint8_t* nonce, const uint8_t* key, uint8_t* data,
                          uint32_t dataLen, uint32_t dataDid);

//-----------------------------------------------------------------------------
// Old Internal API

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// This function loads the 16 byte key into the AES hardware accelerator.
void emLoadKeyIntoCore(const uint8_t* key);

// This function retrieves the 16 byte key from the AES hardware accelerator.
void emGetKeyFromCore(uint8_t* key);

// This function encrypts the 16 byte plaintext block with the previously-loaded
// 16 byte key using the AES hardware accelerator.
// The resulting 16 byte ciphertext is written to the block parameter,
// overwriting the plaintext.
void emStandAloneEncryptBlock(uint8_t* block);

// emAesEncrypt performs AES encryption in ECB mode on the plaintext pointed to
// by the block parameter, using the key pointed to by the key parameter, and
// places the resulting ciphertext into the 16 bytes of memory pointed to by the
// block parameter (overwriting the supplied plaintext).  Any existing key is
// destroyed.
void emAesEncrypt(uint8_t* block, const uint8_t* key);

#endif//DOXYGEN_SHOULD_SKIP_THIS

/** @} // END addtogroup
 */

#endif //__AES_H__
