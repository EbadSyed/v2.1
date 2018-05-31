/*
 * File: dtls-join.c
 * Description: joining using DTLS
 *
 * Copyright 2015 Silicon Laboratories, Inc.                                *80*
 */

#include "core/ember-stack.h"
#include "hal/hal.h"
#include "phy/phy.h"
#include "mac/mac-header.h"
#include "mac/802.15.4/802-15-4-ccm.h"

#include "routing/util/retry.h"           // needed for dispatch.h
#include "ip/dispatch.h"
#include "ip/ip-header.h"
#include "ip/ip-address.h"
#include "framework/ip-packet-header.h"
#include "ip/zigbee/key-management.h"
#include "ip/zigbee/join.h"
#include "ip/commission.h"
#include "ip/commission-dataset.h"
#include "ip/association.h"

#include "ip/udp.h"
#include "ip/tcp.h"                       // for emAllocateTlsState()

#include "tls.h"
#include "dtls.h"
#include "tls-record.h"
#include "tls-handshake.h"
#include "tls-sha256.h"
#include "tls-handshake-crypto.h"
#include "debug.h"
#include "dtls-join.h"

#ifdef EMBER_MBEDTLS_STACK
#include "stack/ip/tls/mbedtls/mbedtls-stack.h"
#endif

void emGenerateKek(TlsState *tls, uint8_t *key);

uint8_t emDtlsJoinMode = DLTS_JOIN_THREAD_COMMISSIONING;

// Join procedure
// 1. Unsecure MLE to establish link with neigbhor.
// 2. DTLS exchange to get super master key.
//   --- rejoin starts here
// 3. MLE challenge and response.  Get Node ID here?
// 4. Get network data - contexts, fragment owner's address, prefixes
// 5. request router ID if necessary

// Child IDs need to be passed out using MLE, as that is all rejoiners have
// access to.  Could use a PARAMETER TLV with a new MAC_ID parameter.
// Need a PARAMETER_REQUEST in order to get a router ID.  Use 0xFFFF or
// something to retract values or as a rejection?

// Use a simple format for all of the data.  MLE is probably simplest.

// 1. DTLS
//    Four IP datagrams, two of them large because of certificates.
//    Should we go to raw public keys?  Self-signed certs (or raw
//    keys) do not protect from man-in-the-middle attacks.
// 2. Send uber key and joiner's node ID.  This might fit in the last
//    packet of the handshake, sent from server to client.
// 3. MLE handshake (three messages).  Could save one message by
//    including the server's frame counter with the uber key (making
//    that packet even larger).  DTLS handshake guarentees freshness.
// 4. Server sends network data:
//    1. Controller's router ID and EUI64
//    2. Prefix, router ID, context, and timeout for each border router
//    3. For Nest, alarm counter
//    4. Some kind of data version number?  How is new data promulgated?

// Pick a port.  This might be tricky.  It might need to be put in the
// beacon.  Or use the MLE port if it is possible to tell the difference
// between the packets.
//
// Do we only use this to distribute the one key that rules them all?
// If so, no complicated protocol is needed.
//
// Use 6LoWPAN fragmentation for retries, once we have it implemented.

uint16_t emUdpJoinSrcPort = DTLS_JOIN_PORT;
uint16_t emUdpJoinPort = DTLS_JOIN_PORT;
#define UDP_JOIN_PORT emUdpJoinPort
uint16_t emUdpCommissionPort = DTLS_COMMISSION_PORT;

// Here is the normal DTLS exchange:
//
//    Authenticating Peer     Authenticator
//    -------------------     -------------
//
//    (TLS client_hello)->
//                            <- (TLS server_hello,
//                              TLS certificate,
//                     [TLS server_key_exchange,]
//                      TLS certificate_request,
//                         TLS server_hello_done)
//    (TLS certificate,
//     TLS client_key_exchange,
//     TLS certificate_verify,
//     TLS change_cipher_spec,
//     TLS finished) ->
//                            <- (TLS change_cipher_spec,
//                             TLS finished)

EmberUdpConnectionHandle emParentConnectionHandle = NULL_UDP_HANDLE;
static uint8_t commissionKey[THREAD_COMMISSIONER_KEY_SIZE];
static uint8_t commissionKeyLength;

#ifdef HAVE_TLS_JPAKE
static uint16_t joinTlsFlags = TLS_HAVE_JPAKE;
#elif HAVE_TLS_ECDHE_ECDSA
static uint16_t joinTlsFlags = TLS_HAVE_PSK;   // Use PSK suite as default
                                               // when ECC is enabled.
#else
static uint16_t joinTlsFlags = TLS_AVAILABLE_SUITES;
#endif

typedef struct {
  EmberEui64 eui64;
  uint8_t joinKey[EMBER_JOIN_KEY_MAX_SIZE];
  uint8_t joinKeyLength;
} JoinKeyPair;

static Buffer joinKeyPairs;

void emDtlsJoinInit(void)
{
  joinKeyPairs = NULL_BUFFER;
  MEMSET(commissionKey, 0, THREAD_COMMISSIONER_KEY_SIZE);
  commissionKeyLength = 0;
}

void emMarkDtlsJoinBuffers(void)
{
  emMarkBuffer(&joinKeyPairs);
}

#ifdef HAVE_TLS_JPAKE
static const JoinKeyPair *getJoinKeyPair(const EmberEui64 *eui64)
{
  Buffer that;

  for (that = emBufferQueueHead(&joinKeyPairs);
       that != NULL_BUFFER;
       that = emBufferQueueNext(&joinKeyPairs, that)) {
    JoinKeyPair *pair = (JoinKeyPair *)(void *)emGetBufferPointer(that);

    if (pair != NULL) {
      if (isNullEui64(pair->eui64.bytes)
          || emIsMemoryZero(pair->eui64.bytes, 8)) {
        assert(emBufferQueueLength(&joinKeyPairs) == 1);
        return pair;
      } else if (eui64 != NULL
                 && MEMCOMPARE(&pair->eui64, eui64, sizeof(EmberEui64)) == 0) {
        return pair;
      }
    }
  }

  return NULL;
}
#endif

uint8_t emGetJoinKey(uint8_t *joinKey, const EmberEui64 *eui64)
{
#ifdef HAVE_TLS_JPAKE
  const JoinKeyPair *pair = getJoinKeyPair(eui64);
  if (pair != NULL && pair->joinKeyLength != 0) {
    MEMCOPY(joinKey, pair->joinKey, pair->joinKeyLength);
    return pair->joinKeyLength;
  }
#endif
  return 0;
}

static void deleteJoinKeyPair(const EmberEui64 *eui64)
{
  Buffer that;

  for (that = emBufferQueueHead(&joinKeyPairs);
       that != NULL_BUFFER;
       that = emBufferQueueNext(&joinKeyPairs, that)) {
    JoinKeyPair *pair = (JoinKeyPair *)(void *)emGetBufferPointer(that);

    if (pair != NULL) {
      if ((eui64 == NULL
           && isNullEui64(pair->eui64.bytes))
          || (eui64 != NULL
              && MEMCOMPARE(&pair->eui64, eui64, sizeof(EmberEui64)) == 0)) {
        emBufferQueueRemove(&joinKeyPairs, that);
        return;
      }
    }
  }

  //assert(false);
}

void emSetJoinSecuritySuites(uint16_t suites)
{
  joinTlsFlags = suites & TLS_AVAILABLE_SUITES;
  //joinTlsFlags = TLS_HAVE_JPAKE;
}

// As the commissioner, there are three possibilities:
//  - We are also the joiner router, and send the joiner entrust ourselves.
//    Only need to save the IID of the joiner.
//  - We are the border router and forward via the joiner router.
//    Need to save both the IID the joiner and the short address of
//    the joiner router. -> can go in the remote address slot
//  - We are a commissioner, and forward via the border router and
//    joiner router.
// The last is not possible for a node.  At some point we will need an
// implementation of the commissioner that runs on an external, if only
// to test the border router.
//
// CoAP processing needs to pass through a connection somehow.
//
// At a minimum, it would be good to have a marker saying that there was
// something to look up.  The IP packet isn't real, in that the payload
// is not the actual payload to send.

EmberStatus emFinishDtlsServerJoin(DtlsConnection *connection)
{
  debug("parent connected");

  if ((connection->udpData.localPort == emUdpCommissionPort)
      && (emGetActiveDataset()->securityPolicy[2] & NATIVE_COMMISSIONING) != 0) {
    emStackNoteExternalCommissioner(connection->udpData.connection, true);
  }

  return EMBER_SUCCESS;
}

static void udpStatusHandler(EmberUdpConnectionData *connection,
                             UdpStatus status)
{
  switch (status) {
    case EMBER_UDP_CONNECTED: {
      if (connection->connection == emParentConnectionHandle) {
        debug("child connected");
        emLogLine(SECURITY, "dtls generate kek");
        uint8_t kek[AES_128_KEY_LENGTH];
        emGenerateKek(connectionDtlsState((DtlsConnection *) connection), kek);
        emSetCommissioningMacKey(kek);
        emCommissioningHandshakeComplete();
      }
      break;
    }

    case EMBER_UDP_OPEN_FAILED:
      emJoinSecurityFailed();
      break;

    case EMBER_UDP_DISCONNECTED:
      if (connection->connection == emParentConnectionHandle) {
        debug("disconnected on child");
      } else {
        debug("disconnected on parent");
      }
      break;
  }
}

void emCloseDtlsJoinConnection(void)
{
#ifdef EMBER_MBEDTLS_STACK
  // TODO: Also delete existing join keys as below
  mbedtlsCloseJoinConnection();
#else
  if (emParentConnectionHandle != NULL_UDP_HANDLE) {
    DtlsConnection *data = (DtlsConnection *)
                           emFindConnectionFromHandle(emParentConnectionHandle);
    if (data != NULL) {
      EmberEui64 eui64;
      emInterfaceIdToLongId(data->udpData.remoteAddress + 8, eui64.bytes);
      deleteJoinKeyPair(&eui64);

      emCloseDtlsConnection(data);
    }
  }
#endif
}

// Thread wants use to require a cookie for DTLS join handshakes.
bool emDtlsJoinRequireCookie = true;

// We need to check the IP header, especially if macSecured is false.
// Should we use the hop limit = 255 trick?  What else to check?

// Need to look at the incoming packet and connection.
//  If no connection, packet must be TLS_HANDSHAKE_CLIENT_HELLO.
//  If connection and packet is TLS_HANDSHAKE_CLIENT_HELLO, then
//  check against the seed to see if it is a new packet or a
//  resend.
// These or similar checks are needed for all incoming packets, not
// just joining.  Do joining for now.

// At the commissioner we have three possibilities:
//  - We are the joiner router
//       ports are correct, source is a link-local address
//  - We are on the mesh
//       we get: encapulated DTLS message (says payload only)
//               joiner UDP Port (for remote port?  what is local? do we know?)
//               joiner Address (LL IID)
//               Joiner Router Locator (its node ID)
//       this needs to get combined into a matchable address
//       could use multicast prefix and joiner address as local port
//  - We are off the mesh.  No change, we just have to remember which
//    tunnel goes to the border router.

// The packet header is only there to be reused if a buffer is needed.

void emIncomingJoinMessageHandler(PacketHeader header,
                                  Ipv6Header *ipHeader,
                                  bool relayed)
{
  bool isCommission = ipHeader->destinationPort == emUdpCommissionPort;

  if (emNetworkIsUp()) {
    if (isCommission) {
      if ((emGetActiveDataset()->securityPolicy[2] & NATIVE_COMMISSIONING) == 0
          && emNodeType != EMBER_COMMISSIONER) {
        emLogLine(SECURITY, "no thread native commissioning");
        return;
      }
    } else if (emDtlsJoinMode == DLTS_JOIN_THREAD_COMMISSIONING
               && !emGetThreadJoin()) {
      // simPrint("no joining");
      emLogLine(SECURITY, "dtls join mode mismatch no thread join");
      return;
    } else if (emDtlsJoinMode == DLTS_JOIN_THREAD_COMMISSIONING
               && (!emAmThreadCommissioner()
                   || emSecurityToUart)) {
      emLogLine(SECURITY, "dtls join fwd to commissioner");
      emForwardToCommissioner(header, ipHeader);
      return;
    }
  }

#ifdef EMBER_MBEDTLS_STACK
  mbedtlsIncomingHandler(header, ipHeader, relayed, false);
#else
  UdpConnectionData *connection =
    emFindConnection(ipHeader->source,
                     ipHeader->destinationPort,
                     ipHeader->sourcePort);

  emLogBytesLine(SECURITY,
                 "dtls incoming port pair %u/%u and source",
                 ipHeader->source,
                 16,
                 ipHeader->sourcePort,
                 ipHeader->destinationPort);

  if (connection == NULL) {
    emLogLine(SECURITY, "dtls incoming: unknown source");
  } else {
    TlsState *state = connectionDtlsState((DtlsConnection *) connection);
    if (state != NULL) {
      emLogLine(SECURITY, "DTLS enter %u, state %u",
                connection->connection,
                state->connection.state);
    }
  }

  if (!emNetworkIsUp()) {
    if (emParentConnectionHandle == NULL_UDP_HANDLE) {
      if (connection != NULL) {
        emLogLine(SECURITY, "dtls very strange");
        return;           // something very odd is going on, so ignore the packet
      }
    } else if (connection != NULL
               && connection->connection != emParentConnectionHandle) {
      emLogLine(SECURITY, "dtls don't know sender");
      return;             // this is not from our parent
    }
  }

  if (connection != NULL) {
    TlsState *state = connectionDtlsState((DtlsConnection *) connection);
    if (state != NULL) {
      if (!(isDtlsConnection((TcpConnection *) connection)
            && tlsIsDtlsJoin(state))) {
        emLogLine(SECURITY, "dtls wrong connection");
        // simPrint("wrong kind of connection");
        return;     // wrong kind of connection
      }
    }
  }

  Buffer sendVerifyRequest = NULL_BUFFER;
  switch (emParseInitialDtlsPacket(ipHeader->transportPayload,
                                   ipHeader->transportPayloadLength,
                                   ((emDtlsJoinRequireCookie
                                     && joinTlsFlags == TLS_HAVE_JPAKE)
                                    ? DTLS_REQUIRE_COOKIE_OPTION
                                    : DTLS_NO_OPTIONS),
                                   connection == NULL,
                                   &sendVerifyRequest)) {
    case DTLS_DROP:
      if (connection == NULL) {
        emLogLine(SECURITY, "dtls parsed: no connection");
        // simPrint("DTLS_DROP - no connection");
        return; // no connection and not a proper CLIENT_HELLO packet
      }
      break;

    case DTLS_SEND_VERIFY_REQUEST: {
      emLogBytesLine(SECURITY,
                     "dtls parsed: send verify request to ",
                     ipHeader->source + 8,
                     8);
      if (relayed) {
        emLogLine(SECURITY, "dtls parsed: relaying...");
        emForwardToJoiner(ipHeader->source + 8,
                          ipHeader->sourcePort,
                          ipHeader->destinationPort,
                          NULL,
                          sendVerifyRequest);
      } else {
        Ipv6Header outIpHeader;
        PacketHeader outHeader =
          emMakeUdpHeader(&outIpHeader,
                          IP_HEADER_LL64_SOURCE,
                          ipHeader->source,
                          IPV6_DEFAULT_HOP_LIMIT,
                          ipHeader->destinationPort,
                          ipHeader->sourcePort,
                          NULL,
                          0,
                          emGetBufferLength(sendVerifyRequest));
        if (outHeader != NULL_BUFFER) {
          emSetMacFrameControl(outHeader,
                               (emGetMacFrameControl(outHeader)
                                & ~(MAC_FRAME_FLAG_SECURITY_ENABLED
                                    | MAC_FRAME_VERSION_2006)));
          emSetPayloadLink(outHeader, sendVerifyRequest);
          emSubmitIpHeader(outHeader, &outIpHeader);
        }
      }
      // simPrint("want verify");
      return;   // we've sent a verify, ignore the rest
    }

    case DTLS_PROCESS:
      // This is a valid HELLO - either an old one or a new one.
      // We used to remove the old connection, if any, and make a new one,
      // but that fails if the second HELLO is a retry.  This happens if
      // we are taking a long time to respond, as happens with JPAKE on a
      // chip with no ECC engine.
      //
      // There may be situations where the old behavior is better, such
      // as if the other device has abandoned the earlier attempt.  Is
      // there any good way to tell these situations apart?
      if (connection == NULL) {
        debug("adding DTLS join connection");
        emLogBytesLine(SECURITY,
                       "dtls parsed: adding connection on %u/%u for address: ",
                       ipHeader->source,
                       16,
                       ipHeader->destinationPort,
                       ipHeader->sourcePort);

        connection =
          emAddUdpConnection(ipHeader->source,
                             ipHeader->destinationPort,
                             ipHeader->sourcePort,
                             (UDP_USING_DTLS
                              | UDP_DTLS_JOIN
                              | joinTlsFlags
                              | (isCommission ? TLS_NATIVE_COMMISSION : 0)
                              | (relayed ? UDP_DTLS_RELAYED_JOIN : 0)),
                             sizeof(DtlsConnection),
                             udpStatusHandler,
                             ((EmberUdpConnectionReadHandler)
                              emHandleJoinDtlsMessage));
        if (connection == NULL) {
          return;
        }
        MEMCOPY(connection->localAddress, ipHeader->destination, 16);
#ifdef HAVE_TLS_JPAKE
        emLogLine(SECURITY, "dtls parsed: jpake activate");
        TlsState *tls = connectionDtlsState((DtlsConnection *) connection);
        if (tls != NULL) {
          if (isCommission) {
            uint8_t *pskc = (uint8_t *) emGetActiveDataset()->pskc;
            if (pskc == NULL || emIsMemoryZero(pskc, THREAD_PSKC_SIZE)) {
              // commission-proxy-app use case (since it uses dtls-join.c)
              if (!emGeneratePskc(emExtendedPanId,
                                  emNetworkId,
                                  pskc)) {
                loseVoid(SECURITY);
              }
            }
            MEMCOPY(tls->handshake.jpakeKey, pskc, THREAD_PSKC_SIZE);
            tls->handshake.jpakeKeyLength = THREAD_PSKC_SIZE;
          } else {
            EmberEui64 eui64;
            emInterfaceIdToLongId(ipHeader->source + 8, eui64.bytes);
            const JoinKeyPair *pair = getJoinKeyPair(&eui64);

            if (pair == NULL) {
              loseVoid(SECURITY);
            }

            MEMCOPY(tls->handshake.jpakeKey, pair->joinKey, pair->joinKeyLength);
            tls->handshake.jpakeKeyLength = pair->joinKeyLength;
          }
        }
#endif
        break;
      }
  }

  if (header == NULL_BUFFER) {
    header = emAllocateBuffer(ipHeader->transportPayloadLength);
    if (header == NULL_BUFFER) {
      return;
    }
  }
  MEMMOVE(emGetBufferPointer(header),
          ipHeader->transportPayload,
          ipHeader->transportPayloadLength);
  emSetBufferLength(header, ipHeader->transportPayloadLength);
  emBufferQueueAdd(&(((DtlsConnection *) connection)->incomingTlsData),
                   header);
  // The connection event will process the packet we just added to the queue.
  // We are too deep in the call stack to process it here.
  emStartConnectionTimerMs(0);
#endif
}

// Called by the TLS record layer to send DTLS records back to a joiner
// by relaying them through the commissioning protocol.  Include the
// KEK key if it is called for.

void emSubmitRelayedJoinPayload(DtlsConnection *connection, Buffer payload)
{
  if (connection != NULL) {
    TlsState *tls = connectionDtlsState(connection);
    uint8_t kekBlock[AES_128_KEY_LENGTH];
    uint8_t *kek = NULL;

    if (tls != NULL) {
      if (connection->udpData.flags & UDP_DTLS_JOIN_KEK) {
        kek = kekBlock;
        emGenerateKek(tls, kek);
      }
    }

    emForwardToJoiner(connection->udpData.remoteAddress + 8,    // joiner IID
                      connection->udpData.remotePort,           // joiner port
                      connection->udpData.localPort,            // joinerRouterId
                      kek,
                      payload);
  }
}

void emGenerateKek(TlsState *tls, uint8_t *key)
{
  // Step one is to reconstruct the original key block
  uint8_t keyBlock[2 * (AES_128_KEY_LENGTH + AES_128_CCM_IV_LENGTH)];
  uint16_t encryptKey;
  uint16_t decryptKey;
  uint16_t outIv;
  uint16_t inIv;

  if (tlsAmClient(tls)) {
    encryptKey = 0;
    decryptKey = AES_128_KEY_LENGTH;
    outIv      = AES_128_KEY_LENGTH * 2;
    inIv       = AES_128_KEY_LENGTH * 2 + AES_128_CCM_IV_LENGTH;
  } else {
    decryptKey = 0;
    encryptKey = AES_128_KEY_LENGTH;
    inIv       = AES_128_KEY_LENGTH * 2;
    outIv      = AES_128_KEY_LENGTH * 2 + AES_128_CCM_IV_LENGTH;
  }

  MEMCOPY(keyBlock + encryptKey, tls->connection.encryptKey, AES_128_KEY_LENGTH);
  MEMCOPY(keyBlock + decryptKey, tls->connection.decryptKey, AES_128_KEY_LENGTH);
  MEMCOPY(keyBlock + inIv, tls->connection.inIv, AES_128_CCM_IV_LENGTH);
  MEMCOPY(keyBlock + outIv, tls->connection.outIv, AES_128_CCM_IV_LENGTH);

  // Step 2 is to hash it to get the key.
  uint8_t hashOutput[SHA256_BLOCK_SIZE];
  emSha256Hash(keyBlock, sizeof(keyBlock), hashOutput);
  MEMCOPY(key, hashOutput, AES_128_KEY_LENGTH);
  emLogBytesLine(SECURITY, "kek", key, AES_128_KEY_LENGTH);
}

void emApiSetJoinKey(const EmberEui64 *eui64,
                     const uint8_t *key,
                     uint8_t keyLength)
{
  if (keyLength <= EMBER_JOIN_KEY_MAX_SIZE) {
    if (eui64 == NULL || emIsMemoryZero(eui64->bytes, 8)) {
      // setting a general join key, clear the join key pairs
      joinKeyPairs = NULL_BUFFER;
    } else {
      // delete the general join key, if it exists
      deleteJoinKeyPair(NULL);
    }

    assert(key != NULL);
    Buffer pairBuffer = emAllocateBuffer(sizeof(JoinKeyPair));
    MEMSET(emGetBufferPointer(pairBuffer), 0, sizeof(JoinKeyPair));
    JoinKeyPair *pair = (JoinKeyPair *)(void *)emGetBufferPointer(pairBuffer);
    if (pair != NULL) {
      MEMCOPY(pair->joinKey, key, keyLength);
      pair->joinKeyLength = keyLength;
    }
    emBufferQueueAdd(&joinKeyPairs, pairBuffer);

    if (eui64 != NULL && !emIsMemoryZero(eui64->bytes, 8)) {
      emComputeEui64Hash(eui64, &pair->eui64);
    }
    emApiSetJoinKeyReturn(EMBER_SUCCESS);
  } else {
    emApiSetJoinKeyReturn(EMBER_BAD_ARGUMENT);
  }
}

void emApiSetCommissionerKey(const int8u *commissionerKey,
                             int8u commissionerKeyLength)
{
  if (!emAmThreadCommissioner()
      && emberNetworkStatus() != EMBER_NO_NETWORK) {
    emApiSetCommissionerKeyReturn(EMBER_INVALID_CALL);
    return;
  }

  if (commissionerKeyLength < 6) {
    emApiSetCommissionerKeyReturn(EMBER_BAD_ARGUMENT);
  } else {
    emSetCommissionKey(commissionerKey, commissionerKeyLength);
    emApiSetCommissionerKeyReturn(EMBER_SUCCESS);
  }
}

void emSetCommissionKey(const uint8_t *key, uint8_t keyLength)
{
  MEMCOPY(commissionKey, key, keyLength);
  commissionKeyLength = keyLength;

  if (emAmThreadCommissioner()) {
    // If we're the commissioner, set the PSKc in the active dataset.
    emCommissionerSetPskcTlv();
  }
}

uint8_t emGetCommissionKey(uint8_t *key)
{
  if (commissionKeyLength != 0) {
    MEMCOPY(key, commissionKey, commissionKeyLength);
  }
  return commissionKeyLength;
}

static uint16_t getEphemeralPort(void)
{
  uint16_t port = halCommonGetRandom() & 0xFFFF;
  // RFC 6056: The dynamic port range defined by IANA consists of
  // the 49152 (0xC000) - 65535 (0xFFFF) range, and is meant for
  // the selection of ephemeral ports.
  while (port < 0xC000
         || port == DTLS_COMMISSION_PORT // 49191
         || port == 0xFFFF) {
    port = halCommonGetRandom() & 0xFFFF;
  }
  return port;
}

bool emStartJoinClient(const uint8_t *address,
                       uint16_t remotePort,
                       const uint8_t *key,
                       uint8_t keyLength)
{
  if (emParentConnectionHandle != NULL_UDP_HANDLE) {
    UdpConnectionData *connectionData =
      emFindConnectionFromHandle(emParentConnectionHandle);
    if (connectionData != NULL) {
      emRemoveConnection(connectionData);
    }
    emParentConnectionHandle = NULL_UDP_HANDLE;
  }
  bool isCommission = emNodeType == EMBER_COMMISSIONER;

  uint16_t localPort;
  if (isCommission) {
    if (remotePort == 0) {
      remotePort = emUdpCommissionPort;
    }
    localPort = emUdpCommissionPort;
  } else {
    if (remotePort == 0) {
      remotePort = emUdpJoinPort;
    }
    // Use a random src port for Thread DTLS joining.
    localPort = getEphemeralPort();
    emUdpJoinSrcPort = localPort;
  }

  DtlsConnection *data =
    (DtlsConnection *)
    emAddUdpConnection(address,
                       localPort,
                       remotePort,
                       (UDP_USING_DTLS
                        | UDP_DTLS_JOIN
                        | (isCommission ? TLS_NATIVE_COMMISSION : 0)
                        | joinTlsFlags),
                       sizeof(DtlsConnection),
                       udpStatusHandler,
                       ((EmberUdpConnectionReadHandler)
                        emHandleJoinDtlsMessage));
  if (data == NULL) {
    return false;
  }
  emStoreLongFe8Address(emLocalEui64.bytes, data->udpData.localAddress);
  emParentConnectionHandle = data->udpData.connection;

#ifdef HAVE_TLS_JPAKE
  TlsState *tls = connectionDtlsState(data);
  if (tls != NULL) {
    if (isCommission) {
      uint8_t stretchedKey[THREAD_PSKC_SIZE];
      emDerivePskc(key,
                   keyLength,
                   emExtendedPanId,
                   emNetworkId,
                   stretchedKey);
      MEMCOPY(tls->handshake.jpakeKey, stretchedKey, THREAD_PSKC_SIZE);
      tls->handshake.jpakeKeyLength = THREAD_PSKC_SIZE;
    } else {
      MEMCOPY(tls->handshake.jpakeKey, key, keyLength);
      tls->handshake.jpakeKeyLength = keyLength;
    }
    emLogBytesLine(COMMISSION,
                   "client key",
                   tls->handshake.jpakeKey,
                   tls->handshake.jpakeKeyLength);
  }
#endif
  emOpenDtlsConnection(data);
  return true;
}
