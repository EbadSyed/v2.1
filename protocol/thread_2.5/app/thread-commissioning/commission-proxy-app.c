// File: commission-proxy-app.c
//
// Description: Thread commissioning proxy for the host (needs ip-driver-app)
//
// -> Maintains a connection over wifi with a commissioner app
// (currently supports the Android release from Intrepid)
//
// -> Performs DTLS to establish the commissioner connection, and also
// when new joiners are added.
//
// -> Routes commissioner management messages between the NCP stack
// and the commissioner.
//
// Copyright 2015 by Silicon Laboratories. All rights reserved.             *80*
//------------------------------------------------------------------------------

#include <stdlib.h>             // malloc() and free()
#include <unistd.h>             // close()
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <poll.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "core/ember-stack.h"
#include "platform/micro/aes.h"
#include "framework/buffer-malloc.h"
#include "framework/event-queue.h"
#include "framework/ip-packet-header.h"
#include "ip/udp.h"
#include "ip/tcp.h"
#include "ip/ip-address.h"
#include "ip/ip-header.h"
#include "app/coap/coap.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/tmsp/tmsp-enum.h"

#include "stack/include/meshcop.h"
#include "stack/ip/host/host-address-table.h"
#include "stack/ip/host/host-listener-table.h"
#include "stack/ip/host/management-client.h"
#include "stack/ip/host/unix-interface.h"
#include "stack/ip/tls/debug.h"
#include "stack/ip/tls/tls.h"
#include "stack/ip/tls/dtls.h"
#include "stack/ip/tls/dtls-join.h"
#include "stack/ip/tls/small-aes/aes.h"
#include "stack/ip/tls/rsa.h"
#include "stack/ip/tls/tls-handshake-crypto.h"
#include "stack/ip/tls/tls-record.h"
#include "stack/ip/tls/tls-handshake.h"
#include "stack/ip/tls/tls-public-key.h"
#include "stack/ip/tls/tls-session-state.h"
#include "stack/ip/tls/certificate.h"
#include "stack/ip/tls/tls-sha256.h"
#include "native-test-util.h"
#include "stack/ip/tls/credentials/test-certificate.h"
#include "stack/ip/tls/jpake-ecc.h"
#include "app/ip-ncp/binary-management.h"
#include "app/ip-ncp/uart-link-protocol.h"

extern void emBorderRouterMessageHandler(CoapMessage *coap);
extern void connectionEventHandler(Event *event);
extern bool emBuffersUseMalloc;
extern uint32_t emMallocCount;
extern uint32_t emMallocSize;

static FILE *devUrandom;
static void randomInit(void);

static bool useBuffers = false;
bool emMacDropIncomingPackets = false;

static int managementFd = -1;

void *tlsMalloc(uint16_t size)
{
  if (useBuffers) {
    return emBufferMalloc(size);
  } else {
    return malloc(size);
  }
}

// Don't call free() because we don't know which malloc produced
// the pointer.

void tlsFree(void *pointer)
{
  if (useBuffers) {
    emBufferFree(pointer);
  }
}

const CertificateAuthority *emMyAuthorities[] = {
  NULL
};

//----------------------------------------------------------------

static Buffer markTestBuffer;   // to make sure we trace TLS structs properly

static void markConnectionBuffer(void)
{
  emMarkBuffer(&markTestBuffer);
  emMarkUdpBuffers();
  emJpakeEccMarkData();
  emberMarkEventQueue(&emStackEventQueue);
  emCoapMarkBuffers();
  emMarkDtlsJoinBuffers();
}

static BufferMarker markers[] = {
  markConnectionBuffer,
  NULL
};

static uint16_t heapSize = 0;
static uint16_t count = 0;
static uint16_t reclaimCount = 10000;

static void reclaimBuffers(void)
{
//  uint16_t before = heapSize - emBufferBytesRemaining();
  if (count == reclaimCount) {
    markTestBuffer = NULL_BUFFER;
  }
  count += 1;
  emMallocFreeList = NULL_BUFFER;
  emReclaimUnusedBuffers(markers);
//  fprintf(stderr, "[heap %d (%d) -> %d]\n", before, emMallocSize,
//        heapSize - emBufferBytesRemaining());
//  debug("heap %d (%d) -> %d", before, emMallocSize,
//        heapSize - emBufferBytesRemaining());
//emMallocSize = 0; // why?
}

static void udpStatusHandler(EmberUdpConnectionData *connection,
                             UdpStatus status)
{
  if (connection->flags & UDP_USING_DTLS) {
    if (status & EMBER_UDP_CONNECTED) {
      fprintf(stdout, "Connected with commissioner.\n");
      emLogLine(COMMISSION, "Informing NCP about active commissioner handle %d", connection->connection);
      emNoteExternalCommissioner(connection->connection, true);
    } else if (status & EMBER_UDP_OPEN_FAILED) {
      fprintf(stdout, "Open failed with commissioner.\n");
    } else if (status & EMBER_UDP_DISCONNECTED) {
      fprintf(stdout, "Disconnected from commissioner.\n");
    }
    fflush(stdout);
  }
}

static bool dtlsTransmitHandler(const uint8_t *payload,
                                uint16_t payloadLength,
                                uint16_t buffer,
                                const EmberIpv6Address *localAddress,
                                uint16_t localPort,
                                const EmberIpv6Address *remoteAddress,
                                uint16_t remotePort,
                                void *transmitHandlerData)
{
  EmberStatus status;
  uint8_t handle = (uint8_t) ((unsigned long) transmitHandlerData);
  UdpConnectionData *data = emFindConnectionFromHandle(handle);
  if (data == NULL) {
    return false;
  }
  emLogLine(COMMISSION, "TX application data %d", handle);
  status = emTlsSendBufferedApplicationData((TcpConnection *) data,
                                            payload,
                                            payloadLength,
                                            (Buffer) buffer,
                                            emTotalPayloadLength((Buffer) buffer));
  return (status == EMBER_SUCCESS);
}

static void udpMessageHandler(EmberUdpConnectionData *connection,
                              uint8_t *packet,
                              uint16_t length)
{
  emLogLine(COMMISSION, "RX application data %d", connection->connection);

  // EMIPSTACK-1801: packet received from commissioner, we must proxy
  // it down to the commissioning code on the NCP.
  uint8_t temp[EMBER_IPV6_MTU + UART_LINK_HEADER_SIZE + 1];
  temp[0] = '[';
  temp[1] = UART_LINK_TYPE_COMMISSIONER_DATA;
  emberStoreHighLowInt16u(temp + 2, length + 1);
  temp[4] = connection->connection;  // prepend the handle
  MEMCOPY(temp + 5, packet, length);
  if (write(managementFd, temp, length + 5) != length + 5) {
    emLogLine(COMMISSION, "Problem with com data write on fd %d", managementFd);
  }
}

static uint16_t tlsFlags = TLS_USING_DTLS;

static int commissionerPort = DTLS_COMMISSION_PORT;
static int ncpCommDataPort = COMMISSION_PROXY_PORT;
static int commissionerFd = -1;
static int ncpCommDataFd = -1;

bool sendHelloVerifyRequest(struct sockaddr_in *destination,
                            socklen_t destLength,
                            uint8_t *packet,
                            uint16_t packetLength)
{
  Buffer helloVerifyRequest = NULL_BUFFER;
  switch (emParseInitialDtlsPacket(packet,
                                   packetLength,
                                   DTLS_REQUIRE_COOKIE_OPTION,
                                   true,
                                   &helloVerifyRequest)) {
    case DTLS_PROCESS:
      break;
    case DTLS_DROP:
      emLogLine(SECURITY, "Dropped hello verify request...");
      return true;
    case DTLS_SEND_VERIFY_REQUEST:
      nativeWrite(commissionerFd,
                  emGetBufferPointer(helloVerifyRequest),
                  emGetBufferLength(helloVerifyRequest),
                  (struct sockaddr *) destination,
                  destLength);
      return true;
  }
  return false;
}

//----------------------------------------------------------------
// We need to translate back and forth between the IPv4 address and port
// that we actually use to communicate with remote devices and pseudo-IPv6
// address that the stack uses.  We have a DTLS connection between the border
// router and the commissioner.
//
// DTLS connections get translated use the ULA prefix with an IID of
//  xxxx:xxxx:0000:0000
// where xxxx:xxxx is the IPv4 address.  We use the ULA prefix to avoid
// confusing any of the IP header code about what source address to use.
//
static void storeRemoteAddress(struct sockaddr_in *ipv4, uint8_t *ipv6)
{
  MEMSET(ipv6, 0, 16);
  memcpy(ipv6 + 8, &ipv4->sin_addr, 4);
  emStoreDefaultGlobalPrefix(ipv6);
}

// Copied from stack/ip/zigbee/join.c to avoid lots of stubbing.
void emComputeEui64Hash(const EmberEui64 *input, EmberEui64 *output)
{
  // create a big-endian version of the eui
  uint8_t bigEndianEui64[8];
  emberReverseMemCopy(bigEndianEui64, input->bytes, 8);

  // create a hash of the big-endian eui
  uint8_t tempOutput[32];
  MEMSET(tempOutput, 0, sizeof(tempOutput));
  emSha256Hash(bigEndianEui64, 8, tempOutput);

  // copy the most-significant bytes of the output to the new eui
  emberReverseMemCopy(output->bytes, tempOutput, 8);

  // set the U/L bit to not unique
  output->bytes[7] |= 0x02;

  emLogBytes(JOIN, "hashed eui:", input->bytes, 8);
  emLogBytes(JOIN, " to:", output->bytes, 8);
  emLog(JOIN, "\n");
}

static uint8_t localIpAddress[16] = { 0 };

bool emberGetLocalIpAddress(uint8_t index, EmberIpv6Address *address)
{
  if (index == 0 && !emIsMemoryZero(localIpAddress, 16)) {
    MEMCOPY(address->bytes, localIpAddress, 16);
    return true;
  }
  return false;
}

static uint8_t ulaPrefix[8] = { 0 };

void emSetDefaultGlobalPrefix(const uint8_t *prefix)
{
  MEMCOPY(ulaPrefix, prefix, 8);
}

bool emStoreDefaultGlobalPrefix(uint8_t *target)
{
  if (!emIsMemoryZero(ulaPrefix, 8)) {
    MEMCOPY(target, ulaPrefix, 8);
    return true;
  } else {
    return false;
  }
}

bool emIsDefaultGlobalPrefix(const uint8_t *prefix)
{
  return (!emIsMemoryZero(ulaPrefix, 8)
          && MEMCOMPARE(prefix, ulaPrefix, 8) == 0);
}

bool emStoreIpSourceAddress(uint8_t *source, const uint8_t *destination)
{
  if (emIsFe8Address(destination)) {
    emStoreLongFe8Address(emMacExtendedId, source);
  } else if (emIsDefaultGlobalPrefix(destination)) {
    emStoreLocalMl64(source);
  } else if (destination[0] == 0xFF) {    // multicast
    if (destination[1] < 0x03               // less than subnet-local scope
        || destination[1] == 0x32) {        // link local all thread nodes
      emStoreLongFe8Address(emMacExtendedId, source);
    } else if (destination[1] <= 0x05       // less than site-local scope
               || destination[1] == 0x33) { // realm local all thread nodes
      return emStoreLocalMl64(source);
    } else {
      return false;                       // too wide a multicast scope
    }
  }
  return true;
}

bool emIsLocalIpAddress(const uint8_t *address)
{
  return (memcmp(localIpAddress, address, 16) == 0);
}

uint8_t emCompressContextPrefix(uint8_t *address,
                                uint8_t matchLength,
                                uint8_t *contextIdLoc)
{
  if (MEMCOMPARE(address, ulaPrefix, 8) == 0) {
    *contextIdLoc = 0;
    return 64;
  }

  return 0;
}

//----------------------------------------------------------------
// Thread commissioning proxy management

static CommissioningDataset localDataset;

const CommissioningDataset *emGetActiveDataset(void)
{
  // Stub with native commissioning bit set, as that is all we care about.
  MEMSET(&localDataset, 0, sizeof(CommissioningDataset));
  localDataset.securityPolicy[2] = 0xF8;
  return &localDataset;
}

typedef struct {
  EmberNetworkParameters networkParams;
  uint32_t networkSequenceNumber;
  uint8_t pskc[THREAD_PSKC_SIZE];
  uint8_t joinKey[EMBER_JOIN_KEY_MAX_SIZE];
  uint8_t joinKeyLength;
  EmberNetworkStatus networkStatus;
  EmberEui64 localEui64;
} CommProxyAppParameters;

static CommProxyAppParameters paramsCache = { { { 0 } } };

#define COMMAND_LINE_MAX_LENGTH 255
bool avahiServiceStarted = false;

static void killDnsService(void)
{
  emStackConfiguration &= ~STACK_CONFIG_NETWORK_IS_UP;
  if (avahiServiceStarted) {
    avahiServiceStarted = false;
    pid_t killPid, pid;
    int status;
    char avahiKill[COMMAND_LINE_MAX_LENGTH] = "";
    switch ((killPid = fork())) {
      case -1:
        perror("fork");
        break;
      case 0:
        sprintf((char *) avahiKill, "sudo /etc/init.d/avahi-daemon restart &");
        fprintf(stderr, "command: %s\n", avahiKill);
        system(avahiKill);
        exit(1);
      default:
        do {
          pid = wait(&status);
        } while (pid != killPid);
        break;
    }
  }
}

static void commProxyAppParametersCallback(void)
{
  // Set the network parameters.
  emberGetStringArgument(0, paramsCache.networkParams.extendedPanId, 8, false);
  uint8_t networkIdLength;
  const uint8_t *networkId = emberStringCommandArgument(1, &networkIdLength);
  if (EMBER_NETWORK_ID_SIZE < networkIdLength) {
    networkIdLength = EMBER_NETWORK_ID_SIZE;
  }
  MEMCOPY(paramsCache.networkParams.networkId, networkId, networkIdLength);

  emberGetStringArgument(2, paramsCache.networkParams.ulaPrefix.bytes, sizeof(paramsCache.networkParams.ulaPrefix.bytes), false);
  emSetDefaultGlobalPrefix(paramsCache.networkParams.ulaPrefix.bytes);

  paramsCache.networkParams.panId = emberUnsignedCommandArgument(3);
  paramsCache.networkParams.channel = emberUnsignedCommandArgument(4);
  emberGetStringArgument(5, paramsCache.localEui64.bytes, 8, false);
  MEMCOPY(emLocalEui64.bytes, paramsCache.localEui64.bytes, 8);
  emberGetStringArgument(6, emMacExtendedId, 8, false);
  paramsCache.networkStatus = emberUnsignedCommandArgument(7);

  // Setup the DNS service with the correct TXT record
  if (paramsCache.networkStatus != EMBER_JOINED_NETWORK_ATTACHED
      && paramsCache.networkStatus != EMBER_JOINED_NETWORK_NO_PARENT) {
    killDnsService();
  } else {
    if (!avahiServiceStarted) {
      killDnsService();
      emSetStackConfig(STACK_CONFIG_NETWORK_IS_UP); // Mark the network as up.
      char recordVersionString[8] = "1";
      char threadVersionString[8] = { 0 };
      MeshCopStateBitmap meshcop = { { 0 } };
      pid_t avahiPid, pid;
      int status;
      switch ((avahiPid = fork())) {
        case -1:
          // Fork() has failed
          perror("fork");
          break;
        case 0:

          sprintf(threadVersionString, "%s", MESHCOP_THREAD_VERSION_STRING);

          // Assume the border-router reference design provides 'user' credentials
          meshcop.connectionMode = SB_CONNECTION_MODE_USER_CREDENTIAL;

          // Thread Interface is active if the stack has attached a network
          if (paramsCache.networkStatus == EMBER_JOINED_NETWORK_ATTACHED) {
            // If the Thread Interface is active, and the node is a Router
            // then there is an active thread partition
            meshcop.threadIfStatus =
              ((paramsCache.networkParams.nodeType == EMBER_ROUTER)
               ? SB_THREAD_IF_STATUS_ACTIVE_WITH_THREAD_PARTITION
               : SB_THREAD_IF_STATUS_ACTIVE_WITHOUT_THREAD_PARTITION);
          } else {
            // If emNetworkIsUp is false, the Thread Interface is not active
            meshcop.threadIfStatus = SB_THREAD_IF_STATUS_INACTIVE;
          }

          // assume the border-router reference design provides "INFREQUENT" availability
          meshcop.availability = SB_AVAILABILITY_INFREQUENT;

          // This is processed by the child
          if (!avahiServiceStarted) {
            if (!emIsMemoryZero(paramsCache.networkParams.networkId, 16)
                && !emIsMemoryZero(paramsCache.networkParams.extendedPanId, 8)) {
              uint8_t *xp = paramsCache.networkParams.extendedPanId;
              uint8_t *sb = meshcop.bytes;

              //NOTE: this does not currently comply with the Thread 1.1 specification
              //      because of missing the 'nn' options in the TXT record field
              //      Additionally, some payloads, like SB and XP should be in binary format,
              //      however, the current commissioning app does comprehend hex data.
              //      Rather than making a call to 'avahi-publish' in the near future the C-API
              //      will need to be used so that binary payloads (which aren't supported by avahi-daemon)
              //      can be included in the TXT mDNS field, as the 1.1 Thread spec requires.
              char avahiPublish[COMMAND_LINE_MAX_LENGTH] = { 0 };
              sprintf(avahiPublish,
                      "avahi-publish -s ThreadCommissioner "
                      "_meshcop._udp %d "
                      "nn=%.*s "
                      "rv=%s "
                      "tv=%s "
                      "sb=%02X%02X%02X%02X "
                      "xp=%02X%02X%02X%02X%02X%02X%02X%02X &",
                      DTLS_COMMISSION_PORT,
                      EMBER_NETWORK_ID_SIZE,
                      paramsCache.networkParams.networkId,
                      recordVersionString,
                      threadVersionString,
                      sb[0], sb[1], sb[2], sb[3],
                      xp[0], xp[1], xp[2], xp[3], xp[4], xp[5], xp[6], xp[7]);
              avahiServiceStarted = true;
              fprintf(stdout, "command: %s\n", avahiPublish);
              system(avahiPublish);
              exit(1);
            }
          }
          break;
        default:
          avahiServiceStarted = true;
          do {
            pid = wait(&status);
          } while (pid != avahiPid);
          break;
      }
    }
  }
}

static void commProxyAppSecurityCallback(void)
{
  emberGetStringArgument(0, paramsCache.networkParams.masterKey.contents, 16, false);
  paramsCache.networkSequenceNumber = emberUnsignedCommandArgument(1);
}

static void commProxyAppAddressCallback(void)
{
  uint8_t address[16];
  emberGetStringArgument(0, address, 16, false);
  if (emIsDefaultGlobalPrefix(address)) {
    MEMCOPY(localIpAddress, address, 16);
    uint8_t meshLocalIdentifier[8];
    emberReverseMemCopy(meshLocalIdentifier,
                        address + 8,
                        8);
    meshLocalIdentifier[7] ^= 0x02;
    emSetMeshLocalIdentifierFromLongId(meshLocalIdentifier);

    if (commissionerFd == -1) {
      if (nativeOpenUdp(&commissionerPort,
                        &commissionerFd)
          != EMBER_SUCCESS) {
        fprintf(stderr, "Failed to open commissioner port %d\n", commissionerPort);
      }
    } else {
      fprintf(stderr, "Commissioner port %d fd %d already open\n", commissionerPort, commissionerFd);
    }
  }
}

bool emGeneratePskc(const uint8_t *extendedPanId,
                    const uint8_t *networkId,
                    uint8_t *pskc)
{
  if (!emIsMemoryZero(paramsCache.pskc, THREAD_PSKC_SIZE)) {
    MEMCOPY(pskc, paramsCache.pskc, THREAD_PSKC_SIZE);
    return true;
  }
  return false;
}

static void setPskcCommand(void)
{
  uint8_t *pskc;
  uint8_t length;
  pskc = emberStringCommandArgument(0, &length);
  assert(length == THREAD_PSKC_SIZE);
  MEMCOPY(paramsCache.pskc, pskc, THREAD_PSKC_SIZE);
}

static void setJoinKeyCommand(void)
{
  EmberEui64 eui64;
  MEMSET(&eui64, 0, sizeof(EmberEui64));
  emberGetStringArgument(0, eui64.bytes, EUI64_SIZE, false);
  uint8_t *key;
  uint8_t length;
  key = emberStringCommandArgument(1, &length);
  if (EMBER_JOIN_KEY_MAX_SIZE < length) {
    length = EMBER_JOIN_KEY_MAX_SIZE;
  }
  MEMCOPY(paramsCache.joinKey, key, length);
  paramsCache.joinKeyLength = length;
  emberSetJoinKey(&eui64,
                  paramsCache.joinKey,
                  paramsCache.joinKeyLength);
}

const EmberCommandEntry managementCallbackCommandTable[] = {
  emberBinaryCommandEntryAction(CB_SET_COMM_PROXY_APP_PARAMETERS_COMMAND_IDENTIFIER,
                                commProxyAppParametersCallback,
                                "bbbvubbu",
                                NULL),
  emberBinaryCommandEntryAction(CB_SET_COMM_PROXY_APP_SECURITY_COMMAND_IDENTIFIER,
                                commProxyAppSecurityCallback,
                                "bu",
                                NULL),
  emberBinaryCommandEntryAction(CB_SET_COMM_PROXY_APP_ADDRESS_COMMAND_IDENTIFIER,
                                commProxyAppAddressCallback,
                                "b",
                                NULL),
  emberBinaryCommandEntryAction(CB_SET_COMM_PROXY_APP_PSKC_COMMAND_IDENTIFIER,
                                setPskcCommand,
                                "b",
                                NULL),
  emberBinaryCommandEntryAction(EMBER_SET_JOIN_KEY_COMMAND_IDENTIFIER,
                                setJoinKeyCommand,
                                "bb",
                                NULL),
  emberCommandEntryTerminator(),
};

//----------------------------------------------------------------

static UdpConnectionData *getConnection(struct sockaddr_in* address)
{
  uint8_t remoteAddress[16];
  storeRemoteAddress(address, remoteAddress);
  UdpConnectionData *connection = emFindConnection(remoteAddress,
                                                   commissionerPort,
                                                   ntohs(address->sin_port));
  if (connection != NULL) {
    return connection;
  } else {
    connection = emAddUdpConnection(remoteAddress,
                                    commissionerPort,
                                    ntohs(address->sin_port),
                                    (UDP_USING_DTLS
                                     | (tlsFlags & TLS_CRYPTO_SUITE_FLAGS)),
                                    sizeof(DtlsConnection),
                                    udpStatusHandler,
                                    udpMessageHandler);
    if (connection != NULL) {
      emLogLine(COMMISSION, "got new connection handle: %d", connection->connection);
      if (tlsFlags & TLS_HAVE_JPAKE) {
        emSetJpakeKey((DtlsConnection *) connection,
                      paramsCache.pskc,
                      THREAD_PSKC_SIZE);
      }
    }
    return connection;
  }
}

//----------------------------------------------------------------
// network status

EmberEui64 emLocalEui64 = { { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7 } };
int8u emMacExtendedId[8] = { 0 };

static uint8_t extendedPanId[8] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t emDefaultSourceLongId[8] = {
  0x42, 0x8A, 0x2F, 0x98, 0x71, 0x37, 0x44, 0x91
};

EmberNodeType emNodeType = EMBER_ROUTER;

#define THE_CHANNEL 0x77
#define THE_PAN_ID  0x89AB

void emberGetNetworkParameters(EmberNetworkParameters *networkParams)
{
  MEMCOPY(networkParams->extendedPanId,
          paramsCache.networkParams.extendedPanId,
          8);
  networkParams->panId   = paramsCache.networkParams.panId;
  networkParams->channel = paramsCache.networkParams.channel;
}

uint8_t emberGetRadioChannel(void)
{
  return paramsCache.networkParams.channel;
}

EmberPanId emberGetPanId(void)
{
  return paramsCache.networkParams.panId;
}

static uint8_t defaultKey[] = {
  0xbf, 0xbe, 0xbd, 0xbc, 0xbb, 0xba, 0xb9, 0xb8,
  0xb7, 0xb6, 0xb5, 0xb4, 0xb3, 0xb2, 0xb1, 0xb0
};

bool emGetNetworkKeySequence(uint32_t *sequenceLoc)
{
  if (sequenceLoc != NULL) {
    *sequenceLoc = paramsCache.networkSequenceNumber;
  }
  return true;
}

uint8_t *emGetNetworkMasterKey(uint8_t *storage)
{
  return paramsCache.networkParams.masterKey.contents;
}

void emberCoapMessageHandler(CoapMessage *message)
{
  assert(false);
}

//----------------------------------------------------------------

int main(int argc, char *argv[])
{
  int mgmtPort = 0;

  int errors = 0;
  argv++; argc--;   // Skip program name.

  for (; 0 < argc; argc--, argv++) {
    char *arg = argv[0];
    if (strcmp(arg, "--debug") == 0) {
#if defined(DEBUG)
      debugLevel = 100;
#endif
      continue;
    } else if (strcmp(arg, "--log") == 0) {
      uint8_t logType;

      if (0 < argc) {
        logType = emLogTypeFromName((uint8_t *)argv[1], strlen(argv[1]));

        if (logType == 0xFF) {
          fprintf(stderr, "Invalid argument: unable to parse %s %s\n",
                  argv[0],
                  argv[1]);
          errors++;
          break;
        }
        emLogConfig(logType, EM_LOG_PORT_STDERR, true);
        argc--;
        argv++;
        continue;
      }
    } else if (strcmp(arg, "--port") == 0) {
      if (0 < argc) {
        commissionerPort = atoi(argv[1]);
        if (0 <= commissionerPort) {
          argc--;
          argv++;
          continue;
        }
      }
    } else if (strcmp(arg, "--mgmt_port") == 0) {
      if (0 < argc) {
        mgmtPort = atoi(argv[1]);
        if (0 <= mgmtPort) {
          argc--;
          argv++;
          continue;
        }
      }
    }
    errors++;
  }

  tlsFlags |= TLS_HAVE_JPAKE;
  MEMCOPY(paramsCache.networkParams.extendedPanId, extendedPanId, 8);
  paramsCache.networkParams.panId = THE_PAN_ID;
  paramsCache.networkParams.channel = THE_CHANNEL;
  MEMCOPY(paramsCache.networkParams.masterKey.contents,
          defaultKey,
          16);
  paramsCache.networkSequenceNumber = 12;

  // emBuffersUseMalloc = true;                 // needed if using valgrind
  emInitializeBuffers();
  emInitializeEventQueue(&emStackEventQueue);
  emberCommandReaderInit();
  markTestBuffer = emAllocateBuffer(50);        // make sure this is first
  useBuffers = true;
  heapSize = emBufferBytesRemaining();
  emTcpInit();

  emMallocCount = 0;
  emMallocSize = 0;

  randomInit();
  emCoapInitialize();
  emDtlsJoinInit();
  emSetJoinSecuritySuites(tlsFlags);

  if (mgmtPort != 0) {
    managementFd = emConnectManagementSocket(mgmtPort);
  }

  if (ncpCommDataFd == -1) {
    ncpCommDataFd = socket(AF_INET6, SOCK_STREAM, 0);

    if (ncpCommDataFd < 0) {
      perror("ncp comm-data socket creation failed");
      return -1;
    }

    struct sockaddr_in6 address;
    memset(&address, 0, sizeof(struct sockaddr_in6));
    address.sin6_family = AF_INET6;
    address.sin6_addr.s6_addr[15] = 1;
    address.sin6_port = htons(ncpCommDataPort);

    if (connect(ncpCommDataFd, (struct sockaddr *) &address, sizeof(address))
        != 0) {
      fprintf(stderr, "Failed to connect to ncp comm-data port %d\n", ncpCommDataPort);
      return -1;
    }
  } else {
    fprintf(stderr, "ncp comm-data port %d fd %d already open\n", ncpCommDataPort, ncpCommDataFd);
  }

  int flags = fcntl(ncpCommDataFd, F_GETFL);
  assert(fcntl(ncpCommDataFd, F_SETFL, flags | O_NONBLOCK) != -1);

  while (true) {
    reclaimBuffers();

    uint8_t readBuffer[1 << 15];

    fd_set input;
    FD_ZERO(&input);
    FD_SET(managementFd, &input);
    FD_SET(commissionerFd, &input);
    FD_SET(ncpCommDataFd, &input);

    int maxFd = managementFd;
    if (maxFd < commissionerFd) {
      maxFd = commissionerFd;
    }
    if (maxFd < ncpCommDataFd) {
      maxFd = ncpCommDataFd;
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    int n = select(maxFd + 1, &input, NULL, NULL, &timeout);
    if (n < 0) {
      perror("select failed");
    } else if (n == 0) {
      // Timeout
    } else {
      if (FD_ISSET(managementFd, &input)) {
        processManagementInputStream();
      }
      if (FD_ISSET(commissionerFd, &input)) {
        struct sockaddr_storage sender;
        socklen_t length = sizeof(sender);
        uint8_t remoteAddress[16];
        uint16_t got =
          nativeReadWithSender(commissionerFd,
                               readBuffer,
                               sizeof(readBuffer),
                               (struct sockaddr*) &sender,
                               &length);
        struct sockaddr_in6 *senderV6Address =
          (struct sockaddr_in6 *)&sender;
        MEMCOPY(remoteAddress, senderV6Address->sin6_addr.s6_addr, 16);

        storeRemoteAddress((struct sockaddr_in *) &sender, remoteAddress);

        UdpConnectionData *connection =
          emFindConnection(remoteAddress,
                           commissionerPort,
                           ntohs(senderV6Address->sin6_port));
        if (connection != NULL) {
          // OK
          if ((connection->flags & UDP_USING_DTLS)) {
            if (tlsFlags & TLS_HAVE_JPAKE) {
              emSetJpakeKey((DtlsConnection *) connection,
                            paramsCache.pskc,
                            THREAD_PSKC_SIZE);
            }
          }
        } else {
          connection = getConnection((struct sockaddr_in *) &sender);
          emOpenDtlsServer((DtlsConnection *) connection);
        }

        if (got > 0) {
          if ((connection->flags & UDP_USING_DTLS)
              && (((TlsState *)
                   emGetBufferPointer(((DtlsConnection *)
                                       connection)->tlsState))->connection.state
                  == TLS_SERVER_EXPECT_HELLO)
              && sendHelloVerifyRequest((struct sockaddr_in *) &sender, length, readBuffer, got)) {
            debug("asking for cookie");
            // do nothing
          } else {
            uint8_t *contents = readBuffer;
            PacketHeader header = emFillBuffer(contents, got);
            if (header == NULL_BUFFER) {
              fprintf(stderr, "out of buffers");
              goto exit;
            }
            emBufferQueueAdd(&((DtlsConnection *)connection)->incomingTlsData,
                             header);
            emDtlsStatusHandler((DtlsConnection *)connection);
          }
        }
      }
      if (FD_ISSET(ncpCommDataFd, &input)) {
        struct sockaddr_storage sender;
        socklen_t length = sizeof(sender);
        uint16_t got =
          nativeReadWithSender(ncpCommDataFd,
                               readBuffer,
                               sizeof(readBuffer),
                               (struct sockaddr*) &sender,
                               &length);
        if (got > 0) {
          uint8_t *contents = readBuffer;
          if (contents[1] == UART_LINK_TYPE_COMMISSIONER_DATA) {
            dtlsTransmitHandler(contents + 5,  // uart link header (4) plus handle (1)
                                got - 5,
                                NULL_BUFFER,
                                NULL, 0, NULL, 0,
                                (void *) (unsigned long) contents[4]);
          }
        }
      }
    }

    if (emberProcessCommandInput(APP_SERIAL)) {
      // No prompt because no customer commands so far.
      // fprintf(stdout, "> ");
      // fflush(stdout);
    }

    while (emberMsToNextQueueEvent(&emStackEventQueue) == 0) {
      emberRunEventQueue(&emStackEventQueue);
    }
  }

  exit:

  useBuffers = false;
  finishRunningTrace();
  killDnsService();
  debug("closing connection");
  fclose(devUrandom);
  close(commissionerFd);
  close(ncpCommDataFd);
  return 0;
}

bool emReallySubmitIpHeader(PacketHeader header,
                            Ipv6Header *ipHeader,
                            bool allowLoopback,
                            uint8_t retries,
                            uint16_t delayMs)
{
  uint8_t toSendBuffer[1 << 16];
  uint8_t *toSend = toSendBuffer;
  bool commissionerMessage = (ipHeader->sourcePort == DTLS_COMMISSION_PORT);

  if (commissionerMessage) {
    uint16_t length = ipHeader->transportPayloadLength;
    uint16_t toDo = length;
    Buffer finger = header;
    uint8_t *from = ipHeader->transportPayload;
    while (0 < toDo) {
      uint16_t have = ((emGetBufferPointer(finger) + emGetBufferLength(finger))
                       - from);
      uint16_t copy = (have < toDo
                       ? have
                       : toDo);
      MEMCOPY(toSend, from, copy);
      toSend += copy;
      toDo -= copy;
      finger = emGetPayloadLink(finger);
      from = emGetBufferPointer(finger);
    }

    struct sockaddr_in destination;
    destination.sin_family = AF_INET;
    memcpy(&destination.sin_addr,
           ipHeader->destination + 8,
           sizeof(destination.sin_addr));
    destination.sin_port = htons(ipHeader->destinationPort);
    nativeWrite(commissionerFd,
                toSendBuffer,
                length,
                (struct sockaddr*) &destination,
                sizeof(destination));
  } else {
    uint16_t length = 0;
    Buffer temp;
    uint16_t start;

    if (ipHeader->nextHeader == IPV6_NEXT_HEADER_UDP) {
      uint8_t checksumIndex = UDP_CHECKSUM_INDEX;
      emberStoreHighLowInt16u(ipHeader->ipPayload + checksumIndex, 0);
      emberStoreHighLowInt16u(ipHeader->ipPayload + checksumIndex,
                              emTransportChecksum(header, ipHeader));
    } else {
      emLogLine(SECURITY, "Expecting only UDP packets on ncp comm-data fd %d", ncpCommDataFd);
      return false;
    }

    for (temp = header, start = emMacPayloadIndex(header) + 6; // INTERNAL_IP_OVERHEAD
         temp != NULL_BUFFER;
         temp = emGetPayloadLink(temp), start = 0) {
      uint8_t *data = emGetBufferPointer(temp) + start;
      uint16_t count = emGetBufferLength(temp) - start;
      MEMCOPY(toSend + length, data, count);
      length += count;
    }

    if (write(ncpCommDataFd, toSendBuffer, length) != length) {
      emLogLine(SECURITY, "Problem with write on ncp comm-data fd %d", ncpCommDataFd);
    }
  }
  return true;
}

//----------------------------------------------------------------
// management commands

void emNoteExternalCommissioner(uint8_t commissionerId,
                                bool available)
{
  emSendBinaryManagementCommand(EMBER_NOTE_EXTERNAL_COMMISSIONER_COMMAND_IDENTIFIER,
                                "uu",
                                commissionerId,
                                available);
}

void emberSetJoinKeyReturn(EmberStatus status)
{
  emSendBinaryManagementCommand(CB_SET_JOIN_KEY_COMMAND_IDENTIFIER,
                                "u",
                                status);
}

EmberNetworkStatus emberNetworkStatus(void)
{
  return paramsCache.networkStatus;
}

static void statusCommand(void)
{
  if (paramsCache.networkStatus != EMBER_NO_NETWORK) {
    fprintf(stderr, "ext pan: {");
    uint8_t i;
    for (i = 0; i < 8; i++) {
      fprintf(stderr, "%02X", paramsCache.networkParams.extendedPanId[i]);
    }
    fprintf(stderr, "}\nnetwork id: %s", paramsCache.networkParams.networkId);
    fprintf(stderr,
            "\npan id: 0x%2X channel: %d",
            paramsCache.networkParams.panId,
            paramsCache.networkParams.channel);
    fprintf(stderr, "\neui: {");
    for (i = 0; i < 8; i++) {
      fprintf(stderr, "%02X", emLocalEui64.bytes[7 - i]);
    }
    fprintf(stderr, "}\nmac ext id: {");
    for (i = 0; i < 8; i++) {
      fprintf(stderr, "%02X", emMacExtendedId[7 - i]);
    }
    fprintf(stderr, "}\nmaster key: {");
    for (i = 0; i < 16; i++) {
      fprintf(stderr, "%02X", paramsCache.networkParams.masterKey.contents[i]);
    }
    fprintf(stderr, "} sequenceNumber: %d", paramsCache.networkSequenceNumber);
    fprintf(stderr, "}\njoin key: %s", paramsCache.joinKey);
    fprintf(stderr, "\ncomm pskc: {");
    for (i = 0; i < THREAD_PSKC_SIZE; i++) {
      fprintf(stderr, "%02X", paramsCache.pskc[i]);
    }
    fprintf(stderr, "}\n");
  }
}

const EmberCommandEntry emberCommandTable[] = {
  emberCommand("status", statusCommand, "", NULL),
  emberCommandEntryTerminator(),
};

//----------------------------------------------------------------
// Randomness.

static void randomInit(void)
{
  devUrandom = fopen("/dev/urandom", "r");
  assert(devUrandom != NULL);
}

static uint8_t nextRandom = 0;

void randomize(uint8_t *blob, uint16_t length)
{
  uint16_t i;

  for (i = 0; i < length; i++) {
    int result = (usingTraceFile()
                  ? nextRandom++
                  : fgetc(devUrandom));
    assert(result != EOF);
    blob[i] = result & 0xFF;
  }
}

bool emRadioGetRandomNumbers(uint16_t *rn, uint8_t theCount)
{
  randomize((uint8_t *) rn, theCount * 2);
  return true;
}

uint16_t halCommonGetRandomTraced(char *file, int line)
{
  uint16_t x;
  randomize((uint8_t *) &x, 2);
  return x;
}

//----------------------------------------------------------------
// Time.

uint32_t halCommonGetInt32uMillisecondTick(void)
{
  struct timeval tv;
  assert(gettimeofday(&tv, NULL) == 0);
  return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

uint16_t halCommonGetInt16uMillisecondTick(void)
{
  return halCommonGetInt32uMillisecondTick();
}

//----------------------------------------------------------------
// Sim print

void vSimPrint(char *format, va_list argPointer)
{
  fprintf(stderr, "[");
  vfprintf(stderr, format, argPointer);
  putc(']', stderr);
  putc('\n', stderr);
}

void simPrint(char* format, ...)
{
  va_list argPointer;
  va_start(argPointer, format);
  vSimPrint(format, argPointer);
  va_end(argPointer);
}

uint32_t emExpandSequenceNumber(uint8_t sequenceNumber)
{
  return sequenceNumber;
}

void printBytes(const uint8_t *bytes, uint16_t length)
{
  uint16_t i;

  for (i = 0; i < length; i++) {
    printf("%X ", bytes[i]);
  }

  printf("\n");
}

//----------------------------------------------------------------
// stubs

#ifdef EMBER_TEST
  #define USE_STUB_makeMessage
  #define USE_STUB_emLoseHelper
#endif

#ifdef EMBER_SCRIPTED_TEST
  #define USE_STUB_setRandomDataType
#endif

#define USE_STUB_emberSetPskcHandler
#define USE_STUB_emCommissionerSetPskcTlv
#define USE_STUB_emberSetCommissionerKeyReturn
#define USE_STUB_emberSetCommProxyAppKeyHandler
#define USE_STUB_emberTcpAcceptHandler
#define USE_STUB_emberTcpStatusHandler
#define USE_STUB_emberAddressConfigurationChangeHandler
#define USE_STUB_AddressData
#define USE_STUB_emAddressCache
#define USE_STUB_emAmThreadCommissioner
#define USE_STUB_emBeaconPayloadBuffer
#define USE_STUB_emCoapRequestHandler
#define USE_STUB_emCoapStackIncomingMessageHandler
#define USE_STUB_emCommissioningHandshakeComplete
#define USE_STUB_emDhcpClientIncomingMessageHandler
#define USE_STUB_emForwardToCommissioner
#define USE_STUB_emForwardToJoiner
#define USE_STUB_emGetThreadJoin
#define USE_STUB_emGetThreadNativeCommission
#define USE_STUB_emHandleJoinDtlsMessage
#define USE_STUB_emJoinSecurityFailed
#define USE_STUB_emLookupLongId
#define USE_STUB_emMacHeaderLength
#define USE_STUB_emParentId
#define USE_STUB_emSecurityToUart
#define USE_STUB_emSendJoinerEntrust
#define USE_STUB_emSetCommissioningMacKey
#define USE_STUB_emStackConfiguration
#define USE_STUB_emStackEventQueue
#define USE_STUB_emStoreLegacyUla
#define USE_STUB_emStoreMulticastSequence
#define USE_STUB_emUncompressContextPrefix
#define USE_STUB_emberGetNodeId
#define USE_STUB_emberCoapRequestHandler
#define USE_STUB_emDnsClientIncomingMessageHandler
#include "stack/ip/stubs.c"
