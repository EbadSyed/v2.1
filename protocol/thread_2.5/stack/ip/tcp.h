/*
 * File: tcp.h
 * Description: TCP
 * Author(s): Richard Kelsey, kelsey@ember.com
 *
 * Copyright 2010 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef SLTHREAD_TCP_H
#define SLTHREAD_TCP_H

// Internal interface

void emTcpInit(void);

void emHandleIncomingTcp(PacketHeader header, Ipv6Header *ipHeader);

extern EmberEventControl emConnectionEvent;
void emConnectionEventHandler(void);

extern EmberEventControl emTcpAckEvent;
void emTcpAckEventHandler(void);

#define TCP_LISTEN_PORT_TABLE_SIZE 4
#define TCP_MAX_CONNECTIONS_DEFAULT 10

// 1024 is apparently the norm for max segment.  The maximum usable size
// would be 1220 (1280 MTU - IPv6(40) - TCP(20)).
// We use 512 because large TCP/IP packets have to be fragmented over the
// air and there is no retry mechanism for fragments.  Sending large TCP/IP
// packets risks losing the whole packet because a fragment gets dropped.
#define TCP_MAX_SEGMENT         512

//----------------------------------------------------------------
// Internal declarations used by TLS.

typedef struct {
  UdpConnectionData udpData;

  Buffer tlsState;

  Buffer outgoingData;          // chain of outgoing data buffers
  Buffer incomingData;          // chain of incoming application packets
  Buffer incomingTlsData;       // chain of incoming TLS packets

  uint8_t resendCount;            // number of retransmissions

  uint32_t nextIncomingSequence;
  uint32_t nextOutgoingSequence;
  uint16_t unackedBytes;

  uint16_t outgoingMaxSegment;    // from incoming SYN
  uint16_t outgoingWindowSize;    // most recently received window size

  uint8_t state;

  uint8_t averageX8;              // average round trip in TCP ticks, * 8
  uint8_t deviationX4;            // mean deviation in TCP ticks, * 4
  uint8_t resendTcpTicks;         // current resend delay, based on estimated
                                  // round trip time
  uint8_t ackMsRemaining;         // timeout for sending an ACK
} TcpConnection;

TcpConnection *emGetFdConnection(uint8_t fd);

#define connectionTlsState(connection) \
  ((TlsState *) emGetBufferPointer((connection)->tlsState))

#define isTcpConnection(connection) \
  (((connection)->udpData.flags & UDP_USING_TCP) != 0)

void emTcpSendTlsRecord(uint8_t fd, Buffer tlsRecord);
EmberStatus emTcpWriteBuffer(uint8_t fd, Buffer payload);
bool emberTcpConnectionIsSecure(uint8_t fd);
EmberStatus emberTcpSaveTlsState(uint8_t fd);
void emSendTcpDataToApp(TcpConnection *connection);

// The following are implemented by the TLS code.
Buffer emAllocateTlsState(uint8_t fd, uint16_t flags);
void emStartTlsResume(TcpConnection *connection);
bool emTlsStatusHandler(TcpConnection *connection);
#define emTlsSendApplicationData(c, d, l) \
  (emTlsSendBufferedApplicationData((c), (d), (l), NULL_BUFFER, 0))
EmberStatus emTlsSendBufferedApplicationData(TcpConnection *connection,
                                             const uint8_t *data,
                                             uint16_t length,
                                             Buffer moreData,
                                             uint16_t moreLength);
EmberStatus emTlsSendApplicationBuffer(TcpConnection *connection,
                                       Buffer content);
bool emTlsClose(TcpConnection *connection);
void emSendDtlsRecord(TcpConnection *connection, Buffer tlsRecord);
void emTcpConnectionTimeout(TcpConnection *connection);
void emDtlsConnectionTimeout(TcpConnection *connection);
void emCheckForIdleDtlsConnection(TcpConnection *connection);
void emRemoveConnection(UdpConnectionData *connection);
void emRemoveUnusedConnections(void);

// One minute.  TCP connections that take longer than this just aren't
// going to work.  Making this larger would requires increasing the size
// of the ticksRemaining field in TcpConnections.
#define MAX_CONNECTION_TIMEOUT_MS 60000
// Returns the number of MS ticks remaining.
uint32_t emStartConnectionTimerMs(uint16_t delayMs);
void emStartConnectionTimerQs(TcpConnection *connection, uint16_t delayQs);
uint16_t emConnectionQsTicksGoneBy(void);

// Returns true if the connection is still needed.
bool emReportTcpStatus(TcpConnection *connection);

// TCP states, straight from the standard TCP finite-state machine.
// We do not have CLOSED or LISTEN states.  Instead we have a list of
// ports that we are listening at.  A TCP struct is assigned only when
// the a connection is opened.

// We need to send a RST in response to any unexpected TCP packet:
//   - SYN on a not-listening port (is this an attack vector?)
//   - Anything on a non-open port, other than a SYN on a listening port.
//   - Invalid sequence or ack number.

typedef enum {
  TCP_UNUSED,           // This TCP struct is not in use.

  // Opening.
  TCP_SYN_RCVD,         // Waiting for an ACK to our SYN+ACK.
  TCP_SYN_SENT_RCVD,    // Ditto, but we sent a SYN before receiving one.
  TCP_SYN_SENT,         // Waiting for a SYN+ACK in response to our SYN.

  TCP_ESTABLISHED,

  // Passive close: initiated by the other end
  TCP_CLOSE_WAIT,       // FIN received, waiting for app to close on this end.

  // The states from here on down are after we have sent a FIN.
  TCP_LAST_ACK,         // Waiting for an ACK for our FIN sent in response
                        // to their FIN.

  // Active close: initiated by this end
  TCP_FIN_WAIT_1,       // FIN sent, wait for ACK or FIN
  TCP_FIN_WAIT_2,       // ACK for FIN received, waiting for FIN (can get
                        // stuck here if no FIN arrives; needs to have a
                        // timeout)
  TCP_CLOSING,          // FIN received, waiting for ACK for FIN
  TCP_TIME_WAIT         // All done, wait for 2 * max segment lifetime
                        // to allow packets to drain.
} TcpState;

// The high bits of the TCP state are used for various flags.  This
// masks off only the state.
#define TCP_STATE_MASK  0x0F

#define MAX_SEGMENT_OPTIONS_SIZE 4

#ifdef EMBER_TEST
HIDDEN TcpConnection *findUnusedConnection(const uint8_t *remoteAddress,
                                           uint16_t localPort,
                                           uint16_t remotePort,
                                           bool useTls,
                                           bool resumeTls);
#endif

#endif // SLTHREAD_TCP_H
