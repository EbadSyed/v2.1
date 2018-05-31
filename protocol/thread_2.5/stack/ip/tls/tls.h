/*
 * File: tls.h
 * Description: TLS implementation
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef SLTHREAD_TLS_H
#define SLTHREAD_TLS_H

// Using these makes the code a bit more readable.

#ifdef HAVE_TLS_DHE_RSA
  #define haveRsa true
#else
  #define haveRsa false
#endif

#ifdef HAVE_TLS_ECDHE_ECDSA
  #define haveEcc true
#else
  #define haveEcc false
#endif

#ifdef HAVE_TLS_JPAKE
  #define haveJpake true
#else
  #define haveJpake false
#endif

#ifdef HAVE_TLS_PSK
  #define havePsk true
#else
  #define havePsk false
#endif

#if defined(HAVE_TLS_ECDHE_ECDSA) || defined(HAVE_TLS_DHE_RSA)
#define HAVE_TLS_PUBLIC_KEY
#endif

#define TLS_AVAILABLE_SUITES                \
  ((haveRsa   ? TLS_HAVE_DHE_RSA     : 0)   \
   | (haveEcc   ? TLS_HAVE_ECDHE_ECDSA : 0) \
   | (haveJpake ? TLS_HAVE_JPAKE       : 0) \
   | (havePsk   ? TLS_HAVE_PSK         : 0))

//----------------------------------------------------------------
// Security suite APIs.

/* @brief Switch between ECC and PSK.  */
void emUseEccJoin(bool useEcc);

//----------------------------------------------------------------
// TLS constants from RFC 5246.

// For historical reasons, TLS 1.2 is version 3.3.
#define TLS_1P2_MAJOR_VERSION   3
#define TLS_1P2_MINOR_VERSION   3

// DTLS - ones complement of 1.2
#define DTLS_1P2_MAJOR_VERSION   254
#define DTLS_1P2_MINOR_VERSION   253

#define TLS_MAX_RECORD_CONTENT_LENGTH   (1 << 14)

// Record content types
enum {
  TLS_CONTENT_CHANGE_CIPHER_SPEC = 20,
  TLS_CONTENT_ALERT              = 21,
  TLS_CONTENT_HANDSHAKE          = 22,
  TLS_CONTENT_APPLICATION_DATA   = 23
};

// Alerts types
#define TLS_ALERT_LEVEL_WARNING          1
#define TLS_ALERT_LEVEL_FATAL            2

// The actual alerts.  Compliance requires that we send a lot more
// of these than we do.
enum {
  TLS_ALERT_CLOSE_NOTIFY                = 0,
  TLS_ALERT_UNEXPECTED_MESSAGE          = 10,
  TLS_ALERT_BAD_RECORD_MAC              = 20,
  TLS_ALERT_DECRYPTION_FAILED_RESERVED  = 21,
  TLS_ALERT_RECORD_OVERFLOW             = 22,
  TLS_ALERT_DECOMPRESSION_FAILURE       = 30,
  TLS_ALERT_HANDSHAKE_FAILURE           = 40,
  TLS_ALERT_NO_CERTIFICATE_RESERVED     = 41,
  TLS_ALERT_BAD_CERTIFICATE             = 42,
  TLS_ALERT_UNSUPPORTED_CERTIFICATE     = 43,
  TLS_ALERT_CERTIFICATE_REVOKED         = 44,
  TLS_ALERT_CERTIFICATE_EXPIRED         = 45,
  TLS_ALERT_CERTIFICATE_UNKNOWN         = 46,
  TLS_ALERT_ILLEGAL_PARAMETER           = 47,
  TLS_ALERT_UNKNOWN_CA                  = 48,
  TLS_ALERT_ACCESS_DENIED               = 49,
  TLS_ALERT_DECODE_ERROR                = 50,
  TLS_ALERT_DECRYPT_ERROR               = 51,
  TLS_ALERT_EXPORT_RESTRICTION_RESERVED = 60,
  TLS_ALERT_PROTOCOL_VERSION            = 70,
  TLS_ALERT_INSUFFICIENT_SECURITY       = 71,
  TLS_ALERT_INTERNAL_ERROR              = 80,
  TLS_ALERT_USER_CANCELED               = 90,
  TLS_ALERT_NO_RENEGOTIATION            = 100,
  TLS_ALERT_UNSUPPORTED_EXTENSION       = 110
};

// Handshake messages:

enum {
  TLS_HANDSHAKE_HELLO_REQUEST           = 0,
  TLS_HANDSHAKE_CLIENT_HELLO            = 1,
  TLS_HANDSHAKE_SERVER_HELLO            = 2,
  TLS_HANDSHAKE_CLIENT_HELLO_VERIFY     = 3,    // DTLS only
  TLS_HANDSHAKE_CERTIFICATE             = 11,
  TLS_HANDSHAKE_SERVER_KEY_EXCHANGE     = 12,
  TLS_HANDSHAKE_CERTIFICATE_REQUEST     = 13,
  TLS_HANDSHAKE_SERVER_HELLO_DONE       = 14,
  TLS_HANDSHAKE_CERTIFICATE_VERIFY      = 15,
  TLS_HANDSHAKE_CLIENT_KEY_EXCHANGE     = 16,
  TLS_HANDSHAKE_FINISHED                = 20,

  // Not really handshake messages, but using these values internally
  // helps make things more uniform.
  TLS_HANDSHAKE_CHANGE_CIPHER_SPEC      = 0xF0,
  TLS_HANDSHAKE_APPLICATION_DATA        = 0xF1,

  // This is used in the state descriptions to indicate that more than
  // one type of message will be accepted by a particular state.
  TLS_HANDSHAKE_MULTIPLE                = 0xFE,

  // This is used in the state descriptions to indicate that no message
  // will be read by a state.
  TLS_HANDSHAKE_NO_MESSAGE              = 0xFF
};

// TLS extensions
#define TLS_EXTENSION_SIGNATURE_ALGORITHMS    0x0D
#define TLS_EXTENSION_ELLIPTIC_CURVES         0x0A
#define TLS_EXTENSION_EC_POINT_FORMATS        0x0B
#define ECJPAKE_KEY_KP_PAIR                   0x0100

//----------------------------------------------------------------
// Crypto

// We have two different triples of crypto suites, depending on whether
// we are using CBC or CCM.

#define SUITE_COUNT 4

extern const uint16_t pskSuiteIdentifier;
extern const uint16_t dheRsaSuiteIdentifier;
extern const uint16_t rsaSuiteIdentifier;
extern const uint16_t ecdheEcdsaSuiteIdentifier;
extern const uint16_t jpakeSuiteIdentifier;

#define TLS_DHE_RSA_WITH_AES_128_CBC_SHA256     0x0067
#define TLS_PSK_WITH_AES_128_CBC_SHA256         0x00AE
#define TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA1   0xC009    // RFC5289
#define TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 0xC023    // RFC5289
#define TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8      0xC0AE

// These are made up.  IANA has not allocated actual values yet.
#define TLS_DHE_RSA_WITH_AES_128_CCM            0x00FE    // unofficial
#define TLS_PSK_WITH_AES_128_CCM                0xC0A8    // IANA
#define TLS_ECJPAKE_WITH_AES_128_CCM_8          0xC0FF    // unofficial

#define TLS_COMPRESS_ALGORITHM_NULL             0x00
#define TLS_HASH_ALGORITHM_SHA1                 0x02
#define TLS_HASH_ALGORITHM_SHA256               0x04
#define TLS_SIGNATURE_ALGORITHM_RSA             0x01
#define TLS_SIGNATURE_ALGORITHM_ECDSA           0x03

#define TLS_CERT_TYPE_ECDSA_SIGN 64

#define TLS_ELLIPTIC_CURVE_SECP256R1            0x17

// Values for the TLS_EXTENSION_EC_POINT_FORMATS option
#define TLS_ELLIPTIC_UNCOMPRESSED               0x00
#define TLS_ELLIPTIC_ANSIX962_COMPRESSED_PRIME  0x01

#define AES_128_KEY_LENGTH      16
#define AES_128_BLOCK_LENGTH    16
#define AES_128_CBC_IV_LENGTH   16
#define AES_128_CCM_IV_LENGTH    4     // from GCM suite, according to mail
// from Robert Cragie
#define SHA256_HASH_LENGTH      32
#define SHA256_MAC_LENGTH SHA256_HASH_LENGTH // old name

#define TLS_MIN_ENCRYPT_LENGTH  32
#define TLS_CCM_MIN_ENCRYPT_LENGTH 16   // explicit nonce (8) + mic (8)
#define TLS_FINISHED_HASH_SIZE  12
#define SALT_SIZE               32      // Must be even, because we declare
// the salt fields as int16_t[] to
// match the alignment needed by
// emRadioGetRandomNumbers().

// The key exchange hash is preceded by 19 bytes of ASN.1 gubbish.
#define DIGITAL_SIGNATURE_OVERHEAD 19
#define KEY_EXCHANGE_HASH_SIZE (DIGITAL_SIGNATURE_OVERHEAD + SHA256_BLOCK_SIZE)

typedef uint32_t BigIntDigit;
typedef uint16_t BigIntIndex;

typedef struct {
  int8_t       sign;          // -1 or 1
  BigIntIndex length;        // number of digits
  BigIntDigit *digits;
} BigInt;

typedef struct {
  uint16_t modulusLength;         // size of modulus in bytes
  BigInt modulus;
  BigInt publicExponent;
  BigInt rRModN;                // cached R*R mod publicExponent, where
                                // R = 1 << (modulusLength * 8)

  BigInt privateExponent;       // aka d
  BigInt factor0;               // aka p
  BigInt factor1;               // aka q
  BigInt dModPMinus1;           // d mod (p - 1)
  BigInt dModQMinus1;           // d mod (q - 1)
  BigInt inverseQModP;          // 1 / (Q % P)

  BigInt rRModP;                // cached value
  BigInt rRModQ;                // cached value
} RsaPrivateKey;

typedef struct {
  uint16_t modulusLength;         // size of modulus in bytes
  BigInt modulus;
  BigInt publicExponent;
  BigInt rRModN;                // cached r*r mod publicExponent, where
                                // r = 1 << (modulusLength * 8)
} RsaPublicKey;

typedef struct DhStateS {
  BigInt primeModulus;          // aka p
  BigInt generator;             // aka g
  BigInt localRandom;           // aka x
  BigInt remoteRandom;          // g^y mod p, where y is the peer's random value
  BigInt rRModP;                // the usual cached value
} DhState;

// We use p256r1, so coordinates are 256 bits (= 32 bytes) long.
#define ECC_COORDINATE_LENGTH 32

// The join key (PSKd) can be up to 32 characters, so jpake must handle that.
#define JPAKE_MAX_KEY_LENGTH 32

// A public EC keys is a two-dimensional point on an elliptic curve.
// For p256r1 the coordinates are 32 bytes each.  There are three
// possible encodings for points:
//
// 0x00 - this is the origin
//
// 0x02 or 0x03 followed by 32 bytes
//   The last 32 bytes are the X coordinate.  For each X there are two
//   possible Y values, with 0x02 indicating one and 0x03 the other.
//
// 0x04 followed by 64 bytes
//   The 64 bytes are the X and Y coordinates in that order.
//
// So far the Certicom library has always used the 33 byte encoding.
// We have to assume that we may receive one of the others.
//
// One option would be to convert incoming 0x04 keys into the 0x03
// format.  It isn't clear that doing this, which itself requires some
// temporary heap space, is a win over just allocating enough space
// for the 0x04 format in the first place.

#ifdef EMBER_SCRIPTED_TEST
  #define ECDH_OUR_PUBLIC_KEY_LENGTH 33
#else
  #define ECDH_OUR_PUBLIC_KEY_LENGTH 65
#endif

#define ECDH_MAX_PUBLIC_KEY_LENGTH 65

typedef struct {
  uint8_t keyLength;
  uint8_t key[ECDH_MAX_PUBLIC_KEY_LENGTH];
} EccPublicKey;

typedef struct {
  uint8_t secret[ECC_COORDINATE_LENGTH];
} EccPrivateKey;

typedef struct {
  uint32_t state[8];
  uint32_t byteCount;
  uint8_t buffer[64];
} Sha256State;

//----------------------------------------------------------------
// Keys and certificates

extern const uint8_t *emMyPrivateKey;
extern const uint8_t *emMyRawCertificate;
extern uint16_t emMyRawCertificateLength;

//----------------------------------------------------------------
// RSA key, certificates, and hostname

extern const RsaPrivateKey *emMyRsaKey;
extern uint8_t *emMyHostname;
extern uint16_t emMyHostnameLength;

//----------------------------------------------------------------
// Pre-Shared Keys

typedef struct {
  const uint8_t *identity;
  uint16_t identityLength;
  const uint8_t *key;
  uint16_t keyLength;
} SharedKey;

extern const SharedKey *emMySharedKey;

//----------------------------------------------------------------
// Writes the number of seconds since 00:00:00 UTC, January 1, 1970
// as a four-byte big-endian value.  This needs to be defined somewhere.

void emWriteCurrentTime(uint8_t *);

//----------------------------------------------------------------
// Message processing status

typedef enum {
  TLS_SUCCESS,
  TLS_WRONG_HANDSHAKE_MESSAGE,
  TLS_INSUFFICIENT_DATA,
  TLS_DECRYPTION_FAILURE,
  TLS_DTLS_DUPLICATE,
  TLS_PREMATURE_APP_MESSAGE,
  TLS_BAD_RECORD,
  TLS_NO_BUFFERS,
  TLS_MESSAGE_PROCESSING_FAILURE,
} TlsStatus;

//----------------------------------------------------------------
// Handshake states.  These are in the order in which they occur.
// The SERVER_... states happen on the server, the CLIENT_... on
// the client.

enum {
  TLS_SERVER_SEND_HELLO_REQUEST,    // 0
  TLS_CLIENT_EXPECT_HELLO_REQUEST,  // 1

  TLS_CLIENT_SEND_HELLO,            // 2
  TLS_SERVER_EXPECT_HELLO,          // 3

  TLS_SERVER_SEND_HELLO,                 // 4
  TLS_SERVER_SEND_CERTIFICATE,           // 5
  TLS_SERVER_SEND_KEY_EXCHANGE,          // 6
  TLS_SERVER_SEND_CERTIFICATE_REQUEST,   // 7
  TLS_SERVER_SEND_HELLO_DONE,            // 8

  TLS_CLIENT_EXPECT_HELLO,               // 9
  TLS_CLIENT_EXPECT_CERTIFICATE,         // 10
  TLS_CLIENT_EXPECT_KEY_EXCHANGE,        // 11
  TLS_CLIENT_EXPECT_CERTIFICATE_REQUEST, // 12
  TLS_CLIENT_EXPECT_HELLO_DONE,          // 13

  TLS_CLIENT_SEND_CERTIFICATE,           // 14
  TLS_CLIENT_SEND_KEY_EXCHANGE,          // 15
  TLS_CLIENT_SEND_CERTIFICATE_VERIFY,    // 16
  TLS_CLIENT_SEND_CHANGE_CIPHER_SPEC,    // 17
  TLS_CLIENT_SEND_FINISHED,              // 18

  TLS_SERVER_EXPECT_CERTIFICATE,         // 19
  TLS_SERVER_EXPECT_KEY_EXCHANGE,        // 20
  TLS_SERVER_EXPECT_CERTIFICATE_VERIFY,  // 21
  TLS_SERVER_EXPECT_CHANGE_CIPHER_SPEC,  // 22
  TLS_SERVER_EXPECT_FINISHED,            // 23

  TLS_SERVER_SEND_CHANGE_CIPHER_SPEC,    // 24
  TLS_SERVER_SEND_FINISHED,              // 25

  TLS_CLIENT_EXPECT_CHANGE_CIPHER_SPEC,  // 26
  TLS_CLIENT_EXPECT_FINISHED,            // 27

  TLS_HANDSHAKE_DONE,                    // 28

  TLS_CLOSING,
  TLS_CLOSED,
  TLS_UNINITIALIZED
};

// The sequence for resuming a session is:
//
//  TLS_CLIENT_SEND_HELLO
//
//  TLS_SERVER_EXPECT_HELLO
//
//  TLS_SERVER_SEND_HELLO
//  TLS_SERVER_SEND_CHANGE_CIPHER_SPEC
//  TLS_SERVER_SEND_FINISHED
//
//  TLS_CLIENT_EXPECT_HELLO
//  TLS_CLIENT_EXPECT_CHANGE_CIPHER_SPEC
//  TLS_CLIENT_EXPECT_FINISHED
//
//  TLS_CLIENT_SEND_CHANGE_CIPHER_SPEC
//  TLS_CLIENT_SEND_FINISHED
//
//  TLS_SERVER_EXPECT_CHANGE_CIPHER_SPEC
//  TLS_SERVER_EXPECT_FINISHED

//----------------------------------------------------------------
// Flags

enum {
  TLS_AM_CLIENT                 = 0x0001,
  // These may no longer be needed
  TLS_VERIFY_REQUIRED           = 0x0002,
  TLS_CERTIFICATE_REQUESTED     = 0x0004,
  TLS_IS_RESUME                 = 0x0008,

  TLS_ENCRYPT                   = 0x0010,
  TLS_DECRYPT                   = 0x0020,

  TLS_WAITING_FOR_INPUT         = 0x0040,

  TLS_HAVE_PSK                  = 0x0080,
  TLS_HAVE_DHE_RSA              = 0x0100,
  TLS_HAVE_ECDHE_ECDSA          = 0x0200,
  TLS_HAVE_JPAKE                = 0x0400,

  TLS_NATIVE_COMMISSION         = 0x0800,
  TLS_USING_DTLS                = 0x1000,
  // During the DTLS handshake we need to group outgoing messages for
  // transmission in a single UDP message.
  TLS_IN_DTLS_HANDSHAKE         = 0x2000,
  TLS_IS_DTLS_JOIN              = 0x4000,
  // Cleared after joining when we truncate the TLS struct to remove the
  // handhshake data.
  TLS_HAS_HANDSHAKE             = 0x8000
};

#define tlsAmClient(state)        ((state)->connection.flags & TLS_AM_CLIENT)
#define tlsVerifyRequired(state)  ((state)->connection.flags & TLS_VERIFY_REQUIRED)
#define tlsVerifySucceeded(state) ((state)->connection.flags & TLS_VERIFY_SUCCEEDED)
#define tlsCertificateRequested(state) \
  ((state)->connection.flags & TLS_CERTIFICATE_REQUESTED)
#define tlsIsResume(state)        ((state)->connection.flags & TLS_IS_RESUME)
#define tlsEncrypt(state)         ((state)->connection.flags & TLS_ENCRYPT)
#define tlsDecrypt(state)         ((state)->connection.flags & TLS_DECRYPT)
#define tlsWaitingForInput(state) ((state)->connection.flags & TLS_WAITING_FOR_INPUT)
#define tlsUsingDtls(state)       ((state)->connection.flags & TLS_USING_DTLS)
#define tlsInDtlsHandshake(state) ((state)->connection.flags & TLS_IN_DTLS_HANDSHAKE)
#define tlsIsDtlsJoin(state)      ((state)->connection.flags & TLS_IS_DTLS_JOIN)
#define tlsIsNativeCommission(state) ((state)->connection.flags & TLS_NATIVE_COMMISSION)
#define tlsHasHandshake(state)    ((state)->connection.flags & TLS_HAS_HANDSHAKE)

#define tlsSetFlag(state, flag)   ((state)->connection.flags |= (flag))
#define tlsClearFlag(state, flag) ((state)->connection.flags &= ~(flag))

#define TLS_CRYPTO_SUITE_FLAGS \
  (TLS_HAVE_PSK | TLS_HAVE_DHE_RSA | TLS_HAVE_ECDHE_ECDSA | TLS_HAVE_JPAKE)

//----------------------------------------------------------------
// The actual TLS state.

// There are three parts of the state information which have different
// lifetimes.
//   session    - saved master secret used to resume a session
//   handshake  - used while negotiating or renegotiating a session
//   connection - used for exchanging messages
//
// Session state.  This is what needs to be preserved if
// we want to allow a session to be resumed.
// The id is declared as uint16_t in order to get the
// alignment right for calling the phy random number generator.

// Definition TlsSessionState of moved to core/ember-stack.h because tcp.h
// needs it.

// Handshake state.  Used when setting up or resuming a session.

typedef struct {
  Sha256State messageHash;      // Running hash of all incoming and
                                // outgoing handshake messages.

  // "Salt" is random bits used as input to the key derivation.  Both
  // the client and the server contribute 32 bytes of random bits.
  // These two arrays are treated as a single 64-byte array by the
  // crypto code.  They are declared as uint16_t in order to get the
  // alignment right for calling the PHY random number generator.
  uint16_t clientSalt[SALT_SIZE / 2];
  uint16_t serverSalt[SALT_SIZE / 2];

#ifdef HAVE_TLS_DHE_RSA
  DhState dhState;
  RsaPublicKey peerPublicKey;
#endif

#ifdef HAVE_TLS_ECDHE_ECDSA
  uint8_t ecdhePrivateKey[ECC_COORDINATE_LENGTH];
  EccPublicKey ecdhePublicKey;
  EccPublicKey peerPublicKey;
#endif

//#ifdef HAVE_TLS_JPAKE
  uint8_t jpakeKey[JPAKE_MAX_KEY_LENGTH];
  uint16_t jpakeKeyLength;
//#endif
} TlsHandshakeState;

// Additional values needed for the DTLS handshake.

typedef struct {
  // The message field has 16 bits, but
  // our handshakes max out at about ten messages, so one byte is enough.
  uint8_t inHandshakeCounter;
  uint8_t outHandshakeCounter;
  uint8_t epoch0OutRecordCounter[8];
  Buffer outgoingFlight;
  Buffer finishedMessage; // stashed unencrypted copy
} DtlsHandshakeState;

// When running on the PC we include the extra fields for running
// DHE_RSK and CBC.

// This needs a queue to handle tls->app traffic.  It might be simpler
// if it had its own copy of the status bits as well.

typedef struct {
  uint16_t flags;                 // see above
  uint8_t fd;                     // file descriptor
  uint8_t state;

  uint8_t inRecordCounter[8];     // Count of incoming records; used in nonce
  uint8_t decryptKey[AES_128_KEY_LENGTH];
  uint8_t inIv[AES_128_CCM_IV_LENGTH];

  uint8_t outRecordCounter[8];    // Count of outgoing records, used in nonce
  uint8_t encryptKey[AES_128_KEY_LENGTH];
  uint8_t outIv[AES_128_CCM_IV_LENGTH];

  uint8_t emptyRecordCount;       // Apparently, sending empty records is
                                  // a known denial-of-service attack.  We
                                  // close the connection if too many are
                                  // received in a row.
  // for DTLS only; tracks which messages have already been seen
  uint32_t inCounterMask;

#ifdef HAVE_TLS_CBC
  uint8_t inHashKey[32];
  uint8_t outHashKey[32];
#endif
} TlsConnectionState;

// At some point we will free up the handshake data once the handshake
// is complete.

typedef struct {
  TlsConnectionState connection;
  TlsSessionState session;
  TlsHandshakeState handshake;
} TlsState;

typedef struct {
  TlsConnectionState connection;
  TlsSessionState session;
  TlsHandshakeState handshake;
  DtlsHandshakeState dtlsHandshake;
} DtlsState;

void emMarkTlsState(Buffer *tlsBufferLoc);
// Defined by the PANA EAP code.
void emEapWriteBuffer(TlsState *tls, Buffer buffer);

extern bool emMacDropIncomingPackets;

#if ((defined(HAVE_TLS_DHE_RSA) || defined(HAVE_TLS_ECDHE_ECDSA)) \
  && !(defined(UNIX_HOST) || defined(UNIX_HOST_SIM)))
  #define DROP_INCOMING_PACKETS(x)       \
  do { emMacDropIncomingPackets = true;  \
       { x }                             \
       emMacDropIncomingPackets = false; \
  } while (0)
#else
  #define DROP_INCOMING_PACKETS(x) do { x } while (0)
#endif

void dtlsIncomingHandler(PacketHeader header, Ipv6Header *ipHeader);
void emHandleSecureAppDtlsMessage(EmberUdpConnectionData *connection,
                                  uint8_t *packet,
                                  uint16_t length);

#endif // SLTHREAD_TLS_H
