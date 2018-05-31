/*
 * File: tcp.c
 * Description: TCP
 * Author(s): Richard Kelsey
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#include "core/ember-stack.h"
#include "hal/hal.h"
#include "framework/ip-packet-header.h"
#include "framework/buffer-management.h"
#include "routing/util/retry.h"
#include "ip/tls/tls.h"
#include "ip/tls/tls-session-state.h"

#include "ip-header.h"
#include "6lowpan-header.h"
#include "dispatch.h"
#include "udp.h"
#include "tcp.h"

#define removeConnection(c) (emRemoveConnection(&c->udpData))

//----------------------------------------------------------------
#define simPrint(x, ...)

/*
 #undef emLogLine
 #define emLogLine(foo, format, ...) \
   fprintf(stderr, format "\n", ##__VA_ARGS__)
 */

// To do:
//  * Set the initial sequence number via the clock, as per spec.
//  * Register buffers.
//  * Open connections on command.
//  * Close connections on command.
//  - Send RST in response to unexpected inputs.
//  * Get the event working.
//  * Data reception.
//  * Data transmission.
//  * Resend data if no ack is heard.
//  - Check fragmented messages
//  + Check data restransmission timing (chapter 21 in book).
//  - Check that ACK are sent at appropriate moments.
//  - Manage our incoming window size.
//  - Persist timer in face of zero-sized window (chapter 22 in book).
//  - Keepalive timer?

// TCP uses a 500ms timer.
#define TCP_QS_TICKS 2
// Can wait up to 500ms before sending an ack.

EmberEventControl emTcpAckEvent;

// Start time for the ack event, in milliseconds
static uint16_t ackEventStartTime;

uint8_t emMaxTcpConnections = TCP_MAX_CONNECTIONS_DEFAULT;

// These are invalid values used as "unused" markers.
#define NULL_PORT 0
#define NULL_FD   0

#define TCP_RECEIVE_WINDOW      512

// As per RFC 879.  This may have been updated for IPv6, but I haven't
// seen it.
#define DEFAULT_MAX_SEGMENT     536

// The number of SYN transmissions before an open is considered a failure.
// The value is from Contiki; I do not know if it makes sense.
#define MAX_SYN_TRANSMISSIONS   5

// The number of transmission attempts before closing a connection.
// The value is from Contiki; I do not know if it makes sense.
#define MAX_DATA_TRANSMISSIONS  8

// This is not the format of the actual packet.  The values have
// been converted into the local byte order.

typedef struct {
  uint16_t sourcePort;
  uint16_t destinationPort;
  uint32_t sequenceNumber;
  uint32_t ackNumber;
  uint8_t  headerLength;
  uint8_t  flags;
  uint16_t windowSize;
} TcpHeader;

#define TCP_FIN_FLAG  0x01      // Sender is done sending data
#define TCP_SYN_FLAG  0x02      // Synchronizing to create connection
#define TCP_RST_FLAG  0x04      // Reset the connection
#define TCP_PSH_FLAG  0x08      // PUSH - always set for outgoing, ignored
// for incoming
#define TCP_ACK_FLAG  0x10      // Acknowledgement number is valid
#define TCP_URG_FLAG  0x20      // URGENT - urgent pointer points to last
// byte of urgent data.  We never set this
// in outgoing and ignore it for incoming.
#define TCP_FLAG_MASK 0x3F

#ifdef EMBER_TEST
static UNUSED char *flagNamesVariable[] = {
  "", "F", "S", "F", "R", "FR", "SR", "SFR",
  "P", "FP", "SP", "FP", "RP", "FRP", "SRP", "SFRP",
  "A", "FA", "SA", "FA", "RA", "FRA", "SRA", "SFRA",
  "PA", "FPA", "SPA", "FPA", "RPA", "FRPA", "SRPA", "SFRPA"
};

#define flagNames(x) flagNamesVariable[x]

#else

#define flagNames(x) ""

#endif

// There may be options after the header.  The options must be a
// multiple of four bytes in length.  Their length is included in
// the header's headerLength field.  Options look like:
//  <type:1> <length in bytes:1> <data>
// END and NOOP are one byte each and have no length field.  The total
// options must be padded out with NOOPs to a 4-byte boundary.
//
// The only option we send or pay attention to is MAX_SEGMENT.
// All other incoming options are ignored.

#define TCP_OPTION_END                  0
#define TCP_OPTION_NOOP                 1       // padding
#define TCP_OPTION_MAX_SEGMENT          2
#define TCP_OPTION_MAX_SEGMENT_LENGTH   4
#define TCP_OPTION_WINDOW_SCALE         3       // not supported
#define TCP_OPTION_SELECTIVE_ACK        4       // not supported
#define TCP_OPTION_TIMESTAMP            8       // not supported

// This is application dependent.  We wait twice this long before reusing
// a connection that was closed locally.
#define MAX_SEGMENT_LIFETIME_SECONDS 30U
// Times four because we use a quarter-second clock.
#define TIME_WAIT_TIMEOUT (2U * MAX_SEGMENT_LIFETIME_SECONDS * 4U)

#define finSent(state) (TCP_LAST_ACK <= (state))

const char * const tcpStateNames[] = {
  "unused",
  "syn_rcvd",
  "syn_sent_rcvd",
  "syn_sent",
  "established",
  "close_wait",
  "last_ack",
  "fin_wait_1",
  "fin_wait_2",
  "closing",
  "time_wait"
};

const char * const emTlsModeStrings[] = {
  "",
  " using TLS",
  " resuming TLS",
  " preshared TLS"
};

// A flag bit used to indicate that the application will not accept
// any new data.  When this is set we advertise a window size of zero
// and do not ack incoming data.
//
// I think that this might be better done by having a separate
// TCP_SUSPENDED state that is a near clone of the TCP_ESTABLISHED
// state.  Something to try once the code is working.

#define TCP_SUSPENDED   0x10

// Set if we owe the other end an ack.
#define TCP_SEND_ACK    0x20

// As per RFC 1122.  The initial deviation is chosen so that the
// initial timeout is three seconds (= 6 ticks; we store the deviation
// scaled by 4 and the timeout is A + 4D.
//   A = average, initially zero
//   D = deviation, initially 6/4, stored scaled up to 6
#define INITIAL_DEVIATION 6
#define INITIAL_RESEND_TCP_TICKS 6

//----------------------------------------------------------------
// Forward declarations

static void handleIncomingTcp(TcpConnection *connection,
                              PacketHeader packetHeader,
                              Ipv6Header *ipHeader,
                              TcpHeader *tcpHeader);
static void processSyn(TcpConnection *connection,
                       Ipv6Header *ipheader,
                       TcpHeader *tcpHeader);
static void processOptions(Ipv6Header *ipHeader, TcpConnection *connection);

static bool sendTcp(TcpConnection *connection, uint8_t flags);
static bool sendSyn(TcpConnection *connection, uint8_t flags);
static void sendReset(TcpConnection *connection);
static void startAckTimer(TcpConnection *connection, uint16_t delay);

static const uint8_t maxSegmentOptions[] =
{
  TCP_OPTION_MAX_SEGMENT,
  TCP_OPTION_MAX_SEGMENT_LENGTH,
  HIGH_BYTE(TCP_MAX_SEGMENT),
  LOW_BYTE(TCP_MAX_SEGMENT)
};

//----------------------------------------------------------------
// fd management
//
// We cycle through the full space of file descriptors in order to reduce
// the chance of confusion if the application uses an out-of-date one.
// Unix always gives the lowest unused fd, in part because they use fd's
// as indexes into a table.  Ours would better be called handles.

TcpConnection *emGetFdOrPortConnection(uint8_t fd, uint16_t port)
{
  Buffer buffer = emBufferQueueHead(&emIpConnections);
  while (buffer != NULL_BUFFER) {
    TcpConnection *connection = (TcpConnection *) emGetBufferPointer(buffer);
    if (isTcpConnection(connection)
        && (connection->udpData.connection == fd
            || connection->udpData.localPort == port)) {
      return connection;
    }
    buffer = emBufferQueueNext(&emIpConnections, buffer);
  }
  return NULL;
}

TcpConnection *emGetFdConnection(uint8_t fd)
{
  if (fd == 0xFF) {
    return NULL;
  } else {
    return emGetFdOrPortConnection(fd, NULL_PORT);
  }
}

uint8_t emGetFdForAddress(const EmberIpv6Address *address, uint16_t optionalRemotePort)
{
  Buffer buffer = emBufferQueueHead(&emIpConnections);
  while (buffer != NULL_BUFFER) {
    TcpConnection *connection = (TcpConnection *) emGetBufferPointer(buffer);
    if (isTcpConnection(connection)
        && connection->state != TCP_UNUSED
        && (MEMCOMPARE(connection->udpData.remoteAddress,
                       address,
                       sizeof(EmberIpv6Address))
            == 0)
        && (optionalRemotePort == 0
            || connection->udpData.remotePort == optionalRemotePort)) {
      return connection->udpData.connection;
    }
    buffer = emBufferQueueNext(&emIpConnections, buffer);
  }
  return 0xFF;
}

uint16_t emberTcpGetPort(uint8_t fd)
{
  TcpConnection *connection = emGetFdConnection(fd);
  if (connection == NULL) {
    return 0;
  }
  return connection->udpData.localPort;
}

//----------------------------------------------------------------
// Port management

static uint16_t listeningPorts[TCP_LISTEN_PORT_TABLE_SIZE];
static uint8_t listenPortUsingTls = 0;

static uint16_t previousPort;

bool emberTcpPortInUse(uint16_t port)
{
  uint8_t i;

  if (port == NULL_PORT) {              // used as an 'unused' marker
    return false;
  }

  for (i = 0; i < TCP_LISTEN_PORT_TABLE_SIZE; i++) {
    if (port == listeningPorts[i]) {
      return true;
    }
  }

  return emGetFdOrPortConnection(NULL_FD, port) != NULL;
}

static uint16_t getNextPort(void)
{
  do {
    previousPort += 1;
    if (previousPort == 32000) {
      previousPort = 4096;
    }
  } while (emberTcpPortInUse(previousPort));

  return previousPort;
}

//----------------------------------------------------------------
// Listening

#define cancelListen(port) (flipTcpListen((port), 0))
#define amListening(port)  (flipTcpListen((port), (port)))

static uint8_t flipTcpListen(uint16_t from, uint16_t to)
{
  uint8_t i;
  for (i = 0; i < TCP_LISTEN_PORT_TABLE_SIZE; i++) {
    if (listeningPorts[i] == from) {
      listeningPorts[i] = to;
      return 1 << i;
    }
  }
  return 0;
}

EmberStatus emTcpListen(uint16_t *portLoc, uint8_t tlsMode)
{
  uint16_t port = *portLoc;
  uint8_t mask;

  if (port == 0) {
    port = getNextPort();
  } else if (amListening(port)) {
    return EMBER_SUCCESS;
  } else if (emberTcpPortInUse(port)) {
    return EMBER_BAD_ARGUMENT;
  }

  mask = flipTcpListen(0, port);

  if (mask == 0) {
    return EMBER_TABLE_FULL;
  }

  assert(tlsMode != RESUME_TLS);
  if (tlsMode != NO_TLS) {
    listenPortUsingTls |= mask;
  }
  emUsePresharedTlsSessionState = (tlsMode == PRESHARED_TLS);

  *portLoc = port;
  return EMBER_SUCCESS;
}

bool emberTcpAmListening(uint16_t port)
{
  return amListening(port) != 0;
}

EmberStatus emberTcpStopListening(uint16_t port)
{
  listenPortUsingTls &= ~cancelListen(port);
  return EMBER_SUCCESS;
}

//----------------------------------------------------------------
// The initial sequence number is supposed to be incremented every
// four microseconds.  Our clock only does milliseconds.

static uint32_t lastInitialSequenceNumber;
static uint32_t initialSequenceNumberTime;

static uint32_t initialSequenceNumber(void)
{
  uint32_t now = halCommonGetInt32uMillisecondTick();
  lastInitialSequenceNumber +=
    elapsedTimeInt32u(initialSequenceNumberTime, now);

  initialSequenceNumberTime = now;

  return lastInitialSequenceNumber;
}

//----------------------------------------------------------------

void emTcpInit(void)
{
  MEMSET(listeningPorts, NULL_PORT, sizeof(listeningPorts));

  // This is dangerous, because we don't know what ports were open
  // before we rebooted.  RFC 793 says that we should wait for MSL
  // before opening any connections.
  previousPort = 4095;

  initialSequenceNumber();
}

HIDDEN TcpConnection *findUnusedConnection(const uint8_t *remoteAddress,
                                           uint16_t localPort,
                                           uint16_t remotePort,
                                           bool useTls,
                                           bool resumeTls)
{
  TcpConnection *connection = NULL;
  uint16_t count = 0;

  // Use the closing connection with the least time remaining, if
  // there are any closing connections.

  Buffer buffer = emBufferQueueHead(&emIpConnections);
  while (buffer != NULL_BUFFER) {
    TcpConnection *next = (TcpConnection *) emGetBufferPointer(buffer);
    count += 1;
    if (isTcpConnection(next)) {
      if (next->state == TCP_TIME_WAIT
          && (connection == NULL
              || (next->udpData.ticksRemaining
                  < connection->udpData.ticksRemaining))) {
        connection = next;
      } else if (MEMCOMPARE(next->udpData.remoteAddress, remoteAddress, 16)
                 && next->udpData.localPort == localPort
                 && next->udpData.remotePort == remotePort) {
        // Can't have two identical open connections.
        assert(false);
        return NULL;
      }
    }
    buffer = emBufferQueueNext(&emIpConnections, buffer);
  }

  if (connection == NULL) {
    if (emMaxTcpConnections <= count) {
      return NULL;
    }
  } else {
    removeConnection(connection);
  }

  connection = (TcpConnection *)
               emAddUdpConnection(remoteAddress,
                                  localPort,
                                  remotePort,
                                  (useTls
                                   ? (UDP_USING_TCP | UDP_USING_TLS | TLS_CRYPTO_SUITE_FLAGS)
                                   : (UDP_USING_TCP | TLS_CRYPTO_SUITE_FLAGS)),
                                  sizeof(TcpConnection),
                                  NULL,
                                  NULL);
  if (connection == NULL) {
    return NULL;
  }

  connection->nextOutgoingSequence = initialSequenceNumber();
  connection->deviationX4 = INITIAL_DEVIATION;
  connection->resendTcpTicks = INITIAL_RESEND_TCP_TICKS;

  if (useTls && resumeTls) {
    emStartTlsResume(connection);
  }

  emLogLine(TCP_TX, "%d: new, fd %d",
            indexOf(connection),
            connection->udpData.connection);

  return connection;
}

// assume the upper bound for hops is 8
// space required for 8 hops: 8 + (relayCount << 1) = 8 + (8 << 1) = 8 + 16 = 24
#define HOPS_UPPER_BOUND 24

// Temporarily increase the amount of reserved buffer space to allow
// for inefficient tunneling implementation.

#define TCP_HEADER_UPPER_BOUND     \
  3 * (INTERNAL_IP_HEADER_SIZE     \
       + sizeof(maxSegmentOptions) \
       + TCP_HEADER_SIZE           \
       + HOPS_UPPER_BOUND          \
       + MAC_ADDRESS_OFFSET        \
       + 8 /* OVERHEAD_IN_BYTES */ \
       + 8) // see ip-packet-header.c:emMakeDataHeader()'s buffer allocation

uint8_t emTcpConnect(const EmberIpv6Address *remoteIpAddress,
                     uint16_t remotePort,
                     uint8_t tlsMode)
{
  TcpConnection *connection;
  uint8_t result = 0xFF;
  bool useTls = (tlsMode != NO_TLS);
  bool resumeTls = (tlsMode == RESUME_TLS || tlsMode == PRESHARED_TLS);
  emUsePresharedTlsSessionState = (tlsMode == PRESHARED_TLS);
  connection = findUnusedConnection(remoteIpAddress->bytes,
                                    getNextPort(),
                                    remotePort,
                                    useTls,
                                    resumeTls);

  if (connection != NULL) {
    if (emSetReservedBufferSpace(TCP_HEADER_UPPER_BOUND)) {
      emLogLine(TCP, "%d: connect fd %d useTls %d resume %d",
                indexOf(connection),
                connection->udpData.connection,
                useTls,
                resumeTls);
      if (useTls) {
        simPrint("TLS state %d",
                 connectionTlsState(connection)->connection.state);
      }
      connection->outgoingMaxSegment = DEFAULT_MAX_SEGMENT;

      // Must come after getNextPort() call.
      connection->state = TCP_SYN_SENT;

      if (sendSyn(connection, TCP_SYN_FLAG)) {
        result = connection->udpData.connection;
      } else {
        emLogLine(TCP_TX, "%d: sendSyn failed", indexOf(connection));
        removeConnection(connection);
      }
      emEndBufferSpaceReservation();
    } else {
      emLogLine(TCP, "No buffers (need %u have %u)",
                TCP_HEADER_UPPER_BOUND,
                emBufferBytesRemaining());
      assert(false);
    }
  } else {
    emLogLine(TCP, "conn fail: out of conn");
  }

  return result;
}

static void writeBuffer(TcpConnection *connection, Buffer payload)
{
  simPrint("%d: adding %d bytes (unacked = %d)",
           indexOf(connection),
           emTotalPayloadLength(payload),
           connection->unackedBytes);
  do {
    Buffer next = emGetPayloadLink(payload);
    emPayloadQueueAdd(&connection->outgoingData, payload);
    payload = next;
  } while (payload != NULL_BUFFER);

  connection->udpData.events &= ~EMBER_TCP_TX_COMPLETE;

  emLogLine(TCP_TX, "%d: queueing %d bytes",
            indexOf(connection),
            emGetBufferLength(payload));

  if (connection->unackedBytes == 0) {
    sendTcp(connection, TCP_PSH_FLAG | TCP_ACK_FLAG);
  }
}

EmberStatus emTcpWrite(uint8_t fd,
                       const uint8_t *buffer,
                       uint16_t count,
                       Buffer payload)
{
  TcpConnection *connection = emGetFdConnection(fd);

  if (connection == NULL) {
    emLogLine(TCP, "no conn for %d", fd);
    return EMBER_BAD_ARGUMENT;
  } else if (!((connection->state & TCP_STATE_MASK) == TCP_ESTABLISHED
               || (connection->state & TCP_STATE_MASK) == TCP_CLOSE_WAIT)) {
    emLogLine(TCP, "%d conn not established", fd);
    return EMBER_BAD_ARGUMENT;
  }

  if (connection->tlsState != NULL_BUFFER) {
    if (buffer != NULL) {
      return emTlsSendApplicationData(connection, buffer, count);
    } else {
      return emTlsSendApplicationBuffer(connection, payload);
    }
  } else {
    if (buffer != NULL) {
      payload = emFillBuffer(buffer, count);
      if (payload == NULL_BUFFER) {
        return EMBER_NO_BUFFERS;
      }
    }
    writeBuffer(connection, payload);
    return EMBER_SUCCESS;
  }
}

EmberStatus emberTcpWrite(uint8_t fd, const uint8_t *buffer, uint16_t count)
{
  return emTcpWrite(fd, buffer, count, NULL_BUFFER);
}

EmberStatus emTcpWriteBuffer(uint8_t fd, Buffer payload)
{
  return emTcpWrite(fd, NULL, 0, payload);
}

void emTcpSendTlsRecord(uint8_t fd, Buffer tlsRecord)
{
  writeBuffer(emGetFdConnection(fd), tlsRecord);
}

bool emberTcpFdInUse(uint8_t fd)
{
  TcpConnection *connection = emGetFdConnection(fd);
  return (connection != NULL);
}

bool emberTcpConnectionIsOpen(uint8_t fd)
{
  TcpConnection *connection = emGetFdConnection(fd);

  bool tcpOpen =
    (connection != NULL
     && ((connection->state & TCP_STATE_MASK) == TCP_ESTABLISHED
         || (connection->state & TCP_STATE_MASK) == TCP_CLOSE_WAIT));
  if (tcpOpen && connection->tlsState != NULL_BUFFER) {
    TlsState *tls = connectionTlsState(connection);
    return (tls->connection.state == TLS_HANDSHAKE_DONE);
  } else {
    return tcpOpen;
  }
}

bool emberTcpConnectionIsSecure(uint8_t fd)
{
  TcpConnection *connection = emGetFdConnection(fd);
  return (connection != NULL
          && connection->tlsState != NULL_BUFFER);
}

EmberIpv6Address *emberTcpRemoteIpAddress(uint8_t fd)
{
  TcpConnection *connection = emGetFdConnection(fd);
  if (connection == NULL) {
    return NULL;
  }
  return (EmberIpv6Address *) connection->udpData.remoteAddress;
}

//----------------------------------------------------------------

void emHandleIncomingTcp(PacketHeader header, Ipv6Header *ipHeader)
{
  TcpHeader tcpHeader;
  TcpConnection *connection;
  uint8_t *finger = ipHeader->transportHeader;
  Buffer buffer;

  // emLogLine(TCP_TX, "incoming");

  tcpHeader.sourcePort = emberFetchHighLowInt16u(finger);
  finger += 2;
  tcpHeader.destinationPort = emberFetchHighLowInt16u(finger);
  finger += 2;
  tcpHeader.sequenceNumber = emberFetchHighLowInt32u(finger);
  finger += 4;
  tcpHeader.ackNumber = emberFetchHighLowInt32u(finger);
  finger += 4;
  tcpHeader.headerLength = (*finger++ & 0xF0) >> 2;
  tcpHeader.flags = *finger++ & 0x3F;
  tcpHeader.windowSize = emberFetchHighLowInt16u(finger);

  emSetIncomingTransportHeaderLength(ipHeader, tcpHeader.headerLength);

  buffer = emBufferQueueHead(&emIpConnections);
  while (buffer != NULL_BUFFER) {
    connection = (TcpConnection *) emGetBufferPointer(buffer);
    if (isTcpConnection(connection)
        && tcpHeader.destinationPort == connection->udpData.localPort
        && tcpHeader.sourcePort == connection->udpData.remotePort
        && (MEMCOMPARE(ipHeader->source,
                       connection->udpData.remoteAddress,
                       16)
            == 0)) {
      if (tcpHeader.flags & TCP_RST_FLAG) {
        connection->udpData.events |= (EMBER_TCP_REMOTE_RESET | EMBER_TCP_DONE);
        emStartConnectionTimerMs(0);
        emLogLine(TCP_TX, "%d rx RST", indexOf(connection));
      } else {
        // Should check the incoming sequence number.
        handleIncomingTcp(connection, header, ipHeader, &tcpHeader);
      }
      return;
    }
    buffer = emBufferQueueNext(&emIpConnections, buffer);
  }

  // send a reset if necessary
  if (((tcpHeader.flags & TCP_FLAG_MASK) & TCP_SYN_FLAG) == 0
      && ((tcpHeader.flags & TCP_FLAG_MASK) & TCP_RST_FLAG) == 0) {
    connection
      = findUnusedConnection(ipHeader->source,
                             tcpHeader.destinationPort,
                             tcpHeader.sourcePort,
                             false,
                             false);

    if (connection != NULL) {
      connection->udpData.remotePort = tcpHeader.sourcePort;
      sendReset(connection);
      emLogLine(TCP_TX, "%d: reset", indexOf(connection));
      removeConnection(connection);
    }
    return;
  }

  if ((tcpHeader.flags & TCP_FLAG_MASK) != TCP_SYN_FLAG) {
    emLogLine(TCP_TX, "drop: no conn (dst %d port %d [%02X ... %02X]",
              tcpHeader.destinationPort,
              tcpHeader.sourcePort,
              ipHeader->source[0],
              ipHeader->source[15]);
  } else {
    uint8_t mask = amListening(tcpHeader.destinationPort);
    if (mask == 0) {
      // need to send an RST here
      emLogLine(TCP_TX, "drop: not listening on %d",
                tcpHeader.destinationPort);
    } else {
      connection =
        findUnusedConnection(ipHeader->source,
                             tcpHeader.destinationPort,
                             tcpHeader.sourcePort,
                             ((mask & listenPortUsingTls)
                              ? true
                              : false),
                             false);
      if (connection == NULL) {
        // need to send an RST here
        emLogLine(TCP_TX, "drop: no free conn");
      } else {
        processSyn(connection, ipHeader, &tcpHeader);
      }
    }
  }
}

static void processSyn(TcpConnection *connection,
                       Ipv6Header *ipHeader,
                       TcpHeader *tcpHeader)
{
  connection->udpData.localPort = tcpHeader->destinationPort;
  connection->udpData.remotePort = tcpHeader->sourcePort;
  connection->nextIncomingSequence = tcpHeader->sequenceNumber + 1;
  connection->outgoingWindowSize = tcpHeader->windowSize;

  connection->state = (connection->state == TCP_SYN_SENT
                       ? TCP_SYN_SENT_RCVD
                       : TCP_SYN_RCVD);

  processOptions(ipHeader, connection);

  emLogLine(TCP_TX, "new conn %d", indexOf(connection));

  sendSyn(connection, TCP_SYN_FLAG | TCP_ACK_FLAG);
}

static bool sendSyn(TcpConnection *connection, uint8_t flags)
{
  connection->unackedBytes = 1;    // for the SYN
  connection->resendCount = 0;
  return sendTcp(connection, flags);
}

static void addPacketToIncomingBufferQueue(TcpConnection *connection,
                                           Ipv6Header *ipHeader,
                                           PacketHeader packetHeader,
                                           uint32_t extraPrefix)
{
  uint16_t unwanted = ((ipHeader->transportPayload
                        - emGetBufferPointer(packetHeader))
                       + extraPrefix);
  uint16_t oldLength = emGetBufferLength(packetHeader);
  uint16_t newLength = oldLength - unwanted;
  uint8_t *contents = emGetBufferPointer(packetHeader);

  emLogLine(TCP_TX, "%d: incoming %d data bytes",
            indexOf(connection),
            newLength);

  MEMMOVE(contents, contents + unwanted, newLength);
  emSetBufferLength(packetHeader, newLength);
  emBufferQueueAdd((connection->tlsState == NULL_BUFFER
                    ? &(connection->incomingData)
                    : &(connection->incomingTlsData)),
                   packetHeader);
  emStartConnectionTimerMs(0);
}

// We need a way to handle the following sequence of events:
//  1. incoming TCP that needs an ack
//  2. check with application for new data
//  3. send an ack
// We can't just call the application because of threading
// issues.

static void handleIncomingTcp(TcpConnection *connection,
                              PacketHeader packetHeader,
                              Ipv6Header *ipHeader,
                              TcpHeader *tcpHeader)
{
  uint8_t connectionState = connection->state & TCP_STATE_MASK;
  bool hasAck = false;

  emLogLine(TCP_TX, "%d: %s -%s->",
            indexOf(connection),
            tcpStateNames[connectionState],
            flagNames(tcpHeader->flags & TCP_FLAG_MASK));

  if (tcpHeader->flags & TCP_ACK_FLAG) {
    uint32_t newlyAcked = tcpHeader->ackNumber - connection->nextOutgoingSequence;
    emLogLine(TCP_TX, "%d: %d of %d newly acked",
              indexOf(connection),
              newlyAcked,
              connection->unackedBytes);
    if (0 < newlyAcked
        && newlyAcked <= connection->unackedBytes) {
      hasAck = true;

      if (connection->resendCount == 0) {
        uint16_t elapsed = emConnectionQsTicksGoneBy();
        uint16_t remaining = (elapsed < connection->udpData.ticksRemaining
                              ? connection->udpData.ticksRemaining - elapsed
                              : 0);
        int8_t error = ((connection->resendTcpTicks
                         - (remaining / TCP_QS_TICKS))
                        - (connection->averageX8 >> 3));
        emLogLine(TCP_TX, "%d: remain %d scaled %d resend %d avg %d err %d",
                  indexOf(connection),
                  remaining,
                  remaining / TCP_QS_TICKS,
                  connection->resendTcpTicks,
                  connection->averageX8,
                  error);
        connection->averageX8 += error;
        if (error < 0) {
          error = -error;
        }
        emLogLine(TCP_TX, "%d: err %d devX4 %d -> %d resendTcpTicks %d -> %d",
                  indexOf(connection),
                  error,
                  connection->deviationX4,
                  (connection->deviationX4
                   + (error - (connection->deviationX4 >> 2))),
                  connection->resendTcpTicks,
                  ((connection->averageX8 >> 3)
                   + (connection->deviationX4
                      + (error - (connection->deviationX4 >> 2)))));
        connection->deviationX4 += error - (connection->deviationX4 >> 2);
        connection->resendTcpTicks = ((connection->averageX8 >> 3)
                                      + connection->deviationX4);
      }

      connection->resendCount = 0;
      connection->nextOutgoingSequence += newlyAcked;
      connection->unackedBytes -= newlyAcked;
      if (connection->unackedBytes == 0) {
        connection->udpData.ticksRemaining = 0;
      }

      if (connectionState == TCP_ESTABLISHED
          || connectionState == TCP_CLOSE_WAIT) {
        emRemoveBytesFromPayloadQueue(&connection->outgoingData, newlyAcked);
        if (connection->outgoingData == NULL_BUFFER) {
          connection->udpData.events |= EMBER_TCP_TX_COMPLETE;
          emStartConnectionTimerMs(0);
        }
      }

      // If we get an ACK for previously unacked data we could
      // tell the application so that it can produce more, if it
      // wants.
      //connection->udpData.events |= EMBER_TCP_WINDOW_CHANGE;
    }
  }

  switch (connectionState) {
    case TCP_SYN_RCVD:
    case TCP_SYN_SENT_RCVD:
      if (hasAck) {
        connection->state = TCP_ESTABLISHED;
        connection->udpData.events |= (connectionState == TCP_SYN_RCVD
                                       ? EMBER_TCP_ACCEPTED
                                       : EMBER_TCP_OPENED);
        emStartConnectionTimerMs(0);
        goto TCP_ESTABLISHED; // process any incoming data
      } else if (tcpHeader->flags & TCP_SYN_FLAG) {
        processSyn(connection, ipHeader, tcpHeader);
      }
      break;
    case TCP_SYN_SENT:
      if (hasAck
          && ((tcpHeader->flags & TCP_FLAG_MASK)
              == (TCP_SYN_FLAG | TCP_ACK_FLAG))) {
        processOptions(ipHeader, connection);

        connection->state = TCP_ESTABLISHED;
        connection->udpData.events |= EMBER_TCP_OPENED;

        connection->nextIncomingSequence = tcpHeader->sequenceNumber + 1;
        connection->outgoingWindowSize = tcpHeader->windowSize;
        connection->state |= TCP_SEND_ACK;    // to ack the SYN
        emStartConnectionTimerMs(0);
      } else if ((tcpHeader->flags & TCP_FLAG_MASK)
                 == TCP_SYN_FLAG) {
        // simultaneous open
        processSyn(connection, ipHeader, tcpHeader);
      } else {
        if (!(tcpHeader->flags & TCP_RST_FLAG)) {
          sendReset(connection);
          connection->udpData.events |= EMBER_TCP_OPEN_FAILED;
          emStartConnectionTimerMs(0);
        }
      }
      break;

    case TCP_CLOSE_WAIT:
    // In close_wait we need to continue to send outgoing data. Since the other
    // end has closed, incoming transportPayloadLength should always be 0 and
    // incoming flags should always have FIN set. We don't currently check
    // either of these.
    case TCP_ESTABLISHED:
      TCP_ESTABLISHED: {
        uint8_t flags = 0;

        if (ipHeader->transportPayloadLength != 0
            && !(connection->state & TCP_SUSPENDED)) {
          uint32_t newIncomingSequence =
            tcpHeader->sequenceNumber + ipHeader->transportPayloadLength;
          if (connection->nextIncomingSequence < newIncomingSequence) {
            uint32_t alreadyHave =
              connection->nextIncomingSequence - tcpHeader->sequenceNumber;
            addPacketToIncomingBufferQueue(connection,
                                           ipHeader,
                                           packetHeader,
                                           alreadyHave);
            connection->nextIncomingSequence = newIncomingSequence;
            simPrint("%d: %d new bytes (in now %d)",
                     indexOf(connection),
                     ipHeader->transportPayloadLength - alreadyHave,
                     connection->nextIncomingSequence);
          } else {
            simPrint("%d: not queueing %d bytes, got seq %d, want seq %d",
                     indexOf(connection),
                     ipHeader->transportPayloadLength,
                     tcpHeader->sequenceNumber,
                     connection->nextIncomingSequence);
          }
          connection->state |= TCP_SEND_ACK;
        }

        if ((tcpHeader->flags & TCP_FIN_FLAG)
            && !(connection->state & TCP_SUSPENDED)
            && connectionState == TCP_ESTABLISHED
            && tcpHeader->sequenceNumber == connection->nextIncomingSequence) {
          connection->nextIncomingSequence += 1;
          connection->state = TCP_CLOSE_WAIT;
          connection->udpData.events |= EMBER_TCP_REMOTE_CLOSE;
          connection->state |= TCP_SEND_ACK;
          emStartConnectionTimerMs(0);
          flags = TCP_ACK_FLAG;
        }

        if (tcpHeader->windowSize == 0) {
          // If the window size goes to zero we need to send data at exponentially
          // increasing intervals to test to see if the window has opened.
        } else {
          connection->outgoingWindowSize = tcpHeader->windowSize;
          if (connection->unackedBytes == 0
              && connection->outgoingData != NULL_BUFFER) {
            flags = TCP_PSH_FLAG | TCP_ACK_FLAG;
          }
        }
        if (flags != 0) {
          sendTcp(connection, flags);
        }
        break;
      }

    case TCP_LAST_ACK:
      if (hasAck) {
        connection->udpData.events |= EMBER_TCP_DONE;
        emStartConnectionTimerMs(0);
      }
      break;

    case TCP_FIN_WAIT_1:
      connection->nextIncomingSequence += ipHeader->transportPayloadLength;

      if (0 < ipHeader->transportPayloadLength) {
        addPacketToIncomingBufferQueue(connection, ipHeader, packetHeader, 0);
        connection->state |= TCP_SEND_ACK;
      }

      if (tcpHeader->flags & TCP_FIN_FLAG) {
        if (hasAck) {
          connection->state = TCP_TIME_WAIT;
          emStartConnectionTimerQs(connection, TIME_WAIT_TIMEOUT);
        } else {
          connection->state = TCP_CLOSING;
        }
        connection->udpData.events |= EMBER_TCP_REMOTE_CLOSE;
        connection->nextIncomingSequence += 1;
        connection->state |= TCP_SEND_ACK;
        emStartConnectionTimerMs(0);
      } else if (hasAck) {
        // Can get stuck here.
        // Take care not to lose the TCP_SEND_ACK flag we just set if the incoming
        // transportPayloadLength was > 0.
        connection->state = (TCP_FIN_WAIT_2
                             | (connection->state & TCP_SEND_ACK));
      }
      break;

    case TCP_FIN_WAIT_2:
      connection->nextIncomingSequence += ipHeader->transportPayloadLength;

      if (0 < ipHeader->transportPayloadLength) {
        addPacketToIncomingBufferQueue(connection, ipHeader, packetHeader, 0);
        connection->state |= TCP_SEND_ACK;
      }

      if (tcpHeader->flags & TCP_FIN_FLAG) {
        connection->udpData.events |= EMBER_TCP_REMOTE_CLOSE;
        connection->state = TCP_TIME_WAIT;
        emStartConnectionTimerQs(connection, TIME_WAIT_TIMEOUT);
        connection->nextIncomingSequence += 1;
        connection->state |= TCP_SEND_ACK;
        emStartConnectionTimerMs(0);
      }
      break;

    case TCP_TIME_WAIT:
      connection->state |= TCP_SEND_ACK;
      break;

    case TCP_CLOSING:
      if (hasAck) {
        connection->state = TCP_TIME_WAIT;
        emStartConnectionTimerQs(connection, TIME_WAIT_TIMEOUT);
      }
  }

  if (connection->state & TCP_SEND_ACK) {
    uint8_t state = connection->state & TCP_STATE_MASK;
    // In these two states the app may have data to send.  If there is a lot
    // of data (more than one segment) in the incoming TLS queue, then ack right
    // away to prevent resends.  This matters for HTTPS where we get large
    // chunks that take a while to process.
    if ((state == TCP_ESTABLISHED
         && (connection->tlsState == NULL_BUFFER
             || emBufferQueueByteLength(&(connection->incomingTlsData)) < 600))
        || state == TCP_CLOSE_WAIT) {
      if (connection->ackMsRemaining == 0) {
        startAckTimer(connection, 200);
        emLogLine(TCP_TX, "%d: ack %d in",
                  indexOf(connection),
                  connection->ackMsRemaining);
      }
    } else {
      sendTcp(connection, TCP_ACK_FLAG);
    }
  }

  emLogLine(TCP_TX, "%d:-> %s%s",
            indexOf(connection),
            tcpStateNames[connection->state & TCP_STATE_MASK],
            ((connection->state & TCP_SEND_ACK)
             ? " (ack)"
             : ""));
}

// Aborting and closing
//  if (uip_flags & UIP_ABORT) {
//    // clear outgoing data
//    connection->state = TCP_UNUSED;
//    sendTcp(connection, TCP_RST_FLAG | TCP_ACK_FLAG);
//  } else if (uip_flags & UIP_CLOSE) {
//    // clear outgoing data
//    connection->state = TCP_FIN_WAIT_1;
//    connection->resendCount = 0;
//    sendTcp(connection, TCP_FIN_FLAG | TCP_ACK_FLAG);
//  }

// The only option we care about is max segment, so we ignore all others
// and stop as soon as we find max segment.

static void processOptions(Ipv6Header *ipHeader, TcpConnection *connection)
{
  uint8_t *finger;
  uint8_t optionLength;

  for (finger = ipHeader->transportHeader + TCP_HEADER_SIZE;
       finger < ipHeader->transportPayload;     // hmm
       finger += optionLength) {
    uint8_t optionType = finger[0];
    optionLength = finger[1];

    if (optionType == TCP_OPTION_MAX_SEGMENT
        && optionLength == TCP_OPTION_MAX_SEGMENT_LENGTH) {
      connection->outgoingMaxSegment = HIGH_LOW_TO_INT(finger[2], finger[3]);
      return;
    } else if (optionType == TCP_OPTION_NOOP) {
      optionLength = 1;
    } else if (optionType == TCP_OPTION_END
               || optionLength == 0) {           // bad length field; give up
      return;
    }
  }
}

// Shoot.  We have to send resets in response to random messages
// as well.  I will leave that part out for now. -- Richard
// Richard, I added it -- Nate 1/6/2011
// What about pending data? -- Richard

static void sendReset(TcpConnection *connection)
{
  sendTcp(connection, TCP_RST_FLAG | TCP_ACK_FLAG);
  connection->udpData.events |= EMBER_TCP_DONE;
  emStartConnectionTimerMs(0);
}

void emberTcpReset(uint8_t fd)
{
  TcpConnection *connection = emGetFdConnection(fd);
  if (connection != NULL) {
    sendReset(connection);
  }
}

EmberStatus emberTcpClose(uint8_t fd)
{
  TcpConnection *connection = emGetFdConnection(fd);

  if (connection == NULL) {
    emLogLine(TCP_TX, "no conn for %d", fd);
    return EMBER_SUCCESS;
  }

  if (connection->outgoingData != NULL_BUFFER) {
    emLogLine(TCP_TX, "close err: outgoing data for %d", fd);
    return EMBER_BAD_ARGUMENT;
  }

  switch (connection->state & TCP_STATE_MASK) {
    case TCP_SYN_SENT:
      // never opened, so we don't have to do anything
      emLogLine(TCP_TX, "%d: close b4 open", indexOf(connection));
      removeConnection(connection);
      return EMBER_SUCCESS;

    case TCP_ESTABLISHED:
      if (connection->outgoingData != NULL_BUFFER) {
        return EMBER_BAD_ARGUMENT;
      } else if (connection->tlsState != NULL_BUFFER) {
        if (!emTlsClose(connection)) {
          return EMBER_SUCCESS;
        }
      }
    // fall through

    case TCP_SYN_SENT_RCVD:
      sendTcp(connection, TCP_FIN_FLAG | TCP_ACK_FLAG);
      connection->unackedBytes += 1;
      connection->state = TCP_FIN_WAIT_1;
      return EMBER_SUCCESS;

    case TCP_CLOSE_WAIT:
      sendTcp(connection, TCP_FIN_FLAG | TCP_ACK_FLAG);
      connection->unackedBytes += 1;
      connection->state = TCP_LAST_ACK;
      return EMBER_SUCCESS;

    case TCP_FIN_WAIT_1:
    case TCP_FIN_WAIT_2:
    case TCP_CLOSING:
    case TCP_LAST_ACK:
      // close() here is unnecessary but harmless
      return EMBER_SUCCESS;

    default:
      emLogLine(TCP_TX, "%d conn not estab or started %s(%d)",
                fd,
                tcpStateNames[connection->state & TCP_STATE_MASK],
                connection->state & TCP_STATE_MASK);
      return EMBER_SUCCESS;
  }
}

//----------------------------------------------------------------
// Called when connection->udpData.ticksRemaining goes to zero.
//
// TCP_TIME_WAIT_TIMEOUT is the TCP_WAIT_2/TCP_TIME_WAIT
// -> TCP_UNUSED timeout.

void emTcpConnectionTimeout(TcpConnection *connection)
{
  uint8_t state = connection->state & TCP_STATE_MASK;

  if (state == TCP_TIME_WAIT
      || state == TCP_FIN_WAIT_2) {
    connection->udpData.events |= EMBER_TCP_DONE;
    emStartConnectionTimerMs(0);
  } else if (connection->resendCount
             == ((state == TCP_SYN_SENT
                  || state == TCP_SYN_RCVD
                  || state == TCP_SYN_SENT_RCVD)
                 ? MAX_SYN_TRANSMISSIONS
                 : MAX_DATA_TRANSMISSIONS)) {
    if (state == TCP_SYN_SENT
        || state == TCP_SYN_SENT_RCVD) {
      connection->udpData.events |= EMBER_TCP_OPEN_FAILED;
      emStartConnectionTimerMs(0);
    }
    sendReset(connection);
  } else if (connection->unackedBytes != 0) {
    uint8_t flags = 0;

    switch (state) {
      case TCP_SYN_SENT:
        flags = TCP_SYN_FLAG;
        break;

      case TCP_SYN_RCVD:
      case TCP_SYN_SENT_RCVD:
        flags = TCP_SYN_FLAG | TCP_ACK_FLAG;
        break;

      case TCP_ESTABLISHED:
      case TCP_CLOSE_WAIT:
        if (connection->outgoingData != NULL_BUFFER) {
          flags = TCP_PSH_FLAG | TCP_ACK_FLAG;
        }
        break;

      case TCP_FIN_WAIT_1:
      case TCP_CLOSING:
      case TCP_LAST_ACK:
        flags = TCP_FIN_FLAG | TCP_ACK_FLAG;
        break;
    }
    if (flags != 0) {
      connection->resendCount += 1;
      sendTcp(connection, flags);
    }
  } else if (connection->outgoingData != NULL_BUFFER) {
    // Previous send attempt failed due to lack of buffer space.
    // We should really have some kind of time limit on how long
    // we are going to keep trying.
    sendTcp(connection, TCP_PSH_FLAG | TCP_ACK_FLAG);
  }
}

static void startAckTimer(TcpConnection *connection, uint16_t delay)
{
  uint32_t now = halCommonGetInt32uMillisecondTick();
  uint32_t timeToExecute = now + delay;
  if (!emberEventControlGetActive(emTcpAckEvent)) {
    ackEventStartTime = now;
    emTcpAckEvent.status = EMBER_EVENT_MS_TIME;
    emTcpAckEvent.timeToExecute = timeToExecute;
  } else if (timeGTorEqualInt32u(emTcpAckEvent.timeToExecute, timeToExecute)) {
    emTcpAckEvent.timeToExecute = timeToExecute;
  }
  if (connection != NULL) {
    connection->ackMsRemaining =
      delay + elapsedTimeInt16u(ackEventStartTime, now);
  }
}

void emTcpAckEventHandler(void)
{
  uint32_t now = halCommonGetInt32uMillisecondTick();
  uint16_t ticks = elapsedTimeInt16u(ackEventStartTime, now);
  uint16_t newTicks = 0xFFFF;

  emberEventControlSetInactive(emTcpAckEvent);

  Buffer buffer = emBufferQueueHead(&emIpConnections);
  while (buffer != NULL_BUFFER) {
    TcpConnection *next = (TcpConnection *) emGetBufferPointer(buffer);
    if (isTcpConnection(next)) {
      uint16_t msRemaining = next->ackMsRemaining;

      if (msRemaining != 0) {
        if (msRemaining <= ticks) {
          next->ackMsRemaining = 0;
          simPrint("%d: ACK event handler sending",
                   indexOf(next));
          if (next->state & TCP_SEND_ACK) {
            sendTcp(next, TCP_ACK_FLAG);
          }
        } else {
          msRemaining = msRemaining - ticks;
          next->ackMsRemaining = msRemaining;
          if (msRemaining < newTicks) {
            newTicks = msRemaining;
          }
          simPrint("%d: now has %d ms till ACK, %d for event",
                   indexOf(next),
                   msRemaining,
                   newTicks);
        }
      }
    }
    buffer = emBufferQueueNext(&emIpConnections, buffer);
  }

  if (newTicks != 0xFFFF) {
    ackEventStartTime = now;
    emTcpAckEvent.status = EMBER_EVENT_MS_TIME;
    emTcpAckEvent.timeToExecute = now + newTicks;
  }
}

//----------------------------------------------------------------

// This needs to create the TCP header including payload pointer and
// length and then call PacketHeader emMakeIpHeader(Ipv6Header *ipHeader).
// That in turn needs to change to remove the transport header portions.

static bool sendTcp(TcpConnection *connection, uint8_t flags)
{
  Ipv6Header ipHeader;
  PacketHeader header;
  uint8_t tcpHeader[TCP_HEADER_SIZE + sizeof(maxSegmentOptions)];
  uint8_t optionsLength = ((flags & TCP_SYN_FLAG)
                           ? sizeof(maxSegmentOptions)
                           : 0);
  uint16_t toSend = 0;
  bool result = false;

  MEMSET(&ipHeader, 0, sizeof(ipHeader));
  MEMSET(tcpHeader, 0, sizeof(tcpHeader));

  if (flags & TCP_PSH_FLAG) {
    if (connection->outgoingData != NULL_BUFFER) {
      toSend = emPayloadQueueByteLength(&connection->outgoingData);
      if (connection->outgoingMaxSegment < toSend) {
        toSend = connection->outgoingMaxSegment;
      }
      if (connection->outgoingWindowSize < toSend) {
        toSend = connection->outgoingWindowSize;
      }
    }
  } else {
    flags &= ~TCP_PSH_FLAG;     // no data to send
  }

  // Once we have decided to close, we tell them every chance we get.
  if (finSent(connection->state & TCP_STATE_MASK)) {
    flags |= TCP_FIN_FLAG;
  }

  emLogLine(TCP_TX, "%d: %s(%d) send %s %d out %d in %d",
            indexOf(connection),
            tcpStateNames[connection->state & TCP_STATE_MASK],
            connection->state,
            flagNames(flags),
            toSend,
            connection->nextOutgoingSequence,
            connection->nextIncomingSequence);

  emberStoreHighLowInt16u(tcpHeader, connection->udpData.localPort);
  emberStoreHighLowInt16u(tcpHeader +  2, connection->udpData.remotePort);
  emberStoreHighLowInt32u(tcpHeader +  4, connection->nextOutgoingSequence);
  emberStoreHighLowInt32u(tcpHeader +  8, connection->nextIncomingSequence);
  emberStoreHighLowInt16u(tcpHeader + 12,
                          ((((TCP_HEADER_SIZE + optionsLength) / 4) << 12)
                           | flags));
  emberStoreHighLowInt16u(tcpHeader + 14,
                          ((connection->state & TCP_SUSPENDED)
                           ? 0
                           : TCP_RECEIVE_WINDOW));
  // Last is the checksum and urgent pointer, which we leave zero.
  MEMCOPY(tcpHeader + 20, maxSegmentOptions, sizeof(maxSegmentOptions));

  ipHeader.transportProtocol = IPV6_NEXT_HEADER_TCP;
  ipHeader.transportHeader = tcpHeader;
  ipHeader.transportHeaderLength = TCP_HEADER_SIZE + optionsLength;
  ipHeader.transportPayload = NULL;
  ipHeader.transportPayloadLength = toSend;

  header = emMakeIpHeader(&ipHeader,
                          HEADER_TAG_NONE,
                          IP_HEADER_NO_OPTIONS,
                          connection->udpData.remoteAddress,
                          IPV6_DEFAULT_HOP_LIMIT,
                          toSend);

  if (header == NULL_BUFFER) {
    emLogLine(TCP, "%d: low mem, retry", indexOf(connection));
    emStartConnectionTimerQs(connection, 1);  // try again in a quarter second
  } else {
    if (toSend != 0) {
      emLogLine(TCP_TX, " %d: send %d data bytes", indexOf(connection), toSend);
      connection->unackedBytes = toSend;
      emSetPayloadLink(header, emPayloadQueueHead(&connection->outgoingData));
    }
    result = emSubmitIpHeader(header, &ipHeader);
    if (toSend != 0) {
      simPrint("TCP->stack %d %d", toSend, result);
    }
    // Only open, close, and data get retried.
    if ((flags & (TCP_SYN_FLAG | TCP_FIN_FLAG))
        || 0 < toSend) {
      emLogLine(TCP_TX, "retry timer set");
      emStartConnectionTimerQs(connection,
                               ((connection->resendTcpTicks
                                 << (4 < connection->resendCount
                                     ? 4
                                     : connection->resendCount))
                                * TCP_QS_TICKS));
    }
    emLogLine(TCP_TX, "%d: resend %d tcpTicks %d delay %d",
              indexOf(connection),
              connection->resendCount,
              connection->resendTcpTicks,
              connection->udpData.ticksRemaining);
    if (flags & TCP_ACK_FLAG) {
      connection->state &= ~TCP_SEND_ACK;
    }
  }

  return result;
}
