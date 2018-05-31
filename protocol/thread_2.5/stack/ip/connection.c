/*
 * File: connection.c
 * Description: generic connection handling for TCP and DTLS
 * Author(s): Richard Kelsey
 *
 * Copyright 2013 by Ember Corporation. All rights reserved.                *80*
 */

#include "core/ember-stack.h"
#include "hal/hal.h"
#include "framework/buffer-management.h"
#include "framework/event-queue.h"
#include "udp.h"
#include "tcp.h"
#include "tls/tls.h"
#include "tls/dtls.h"

static uint32_t eventStartTimeMs;
HIDDEN void connectionEventHandler(Event *event);

static EventActions connectionEventActions = {
  &emStackEventQueue,
  connectionEventHandler,
  NULL,          // no marking function is needed
  "connection timeout"
};

static Event connectionEvent =
{ &connectionEventActions, NULL };

const char * const tcpStatusNames[] = {
  "opened",
  "open failed",
  "accepted",
  "tx complete",
  "closed",
  "remote close",
  "remote reset",
  "done",
  "remote close + reset",
  "???",
};

const char *emberTcpStatusName(uint8_t status)
{
  switch (status) {
    case EMBER_TCP_OPENED:
      return tcpStatusNames[0];
    case EMBER_TCP_OPEN_FAILED:
      return tcpStatusNames[1];
    case EMBER_TCP_ACCEPTED:
      return tcpStatusNames[2];
    case EMBER_TCP_TX_COMPLETE:
      return tcpStatusNames[3];
    case EMBER_TCP_CLOSED:
      return tcpStatusNames[4];
    case EMBER_TCP_REMOTE_CLOSE:
      return tcpStatusNames[5];
    case EMBER_TCP_REMOTE_RESET:
      return tcpStatusNames[6];
    case EMBER_TCP_DONE:
      return tcpStatusNames[7];
    case (EMBER_TCP_REMOTE_CLOSE | EMBER_TCP_REMOTE_RESET):
      return tcpStatusNames[8];
    default:
      return tcpStatusNames[9];
  }
}

// Quarter-second ticks.

uint16_t emConnectionQsTicksGoneBy(void)
{
  if (emberEventIsScheduled(&connectionEvent)) {
    return (elapsedTimeInt32u(eventStartTimeMs,
                              halCommonGetInt32uMillisecondTick())
            >> 8);
  } else {
    return 0;
  }
}

void emStartConnectionTimerQs(TcpConnection *connection, uint16_t delayQs)
{
  if ((MAX_CONNECTION_TIMEOUT_MS >> 8) < delayQs) {
    delayQs = (MAX_CONNECTION_TIMEOUT_MS >> 8);
  }

  uint32_t msTicksRemaining = emStartConnectionTimerMs(delayQs << 8);
  if (connection != NULL) {
    connection->udpData.ticksRemaining = msTicksRemaining >> 8;
  }
}

uint32_t emStartConnectionTimerMs(uint16_t delayMs)
{
  uint32_t nowMs = halCommonGetInt32uMillisecondTick();
  if (!emberEventIsScheduled(&connectionEvent)) {
    eventStartTimeMs = nowMs;
    emberEventSetDelayMs(&connectionEvent, delayMs);
  } else if (delayMs < emberEventGetRemainingMs(&connectionEvent)) {
    emberEventSetDelayMs(&connectionEvent, delayMs);
  }
  return delayMs + elapsedTimeInt32u(eventStartTimeMs, nowMs);
}

static void markConnectionUnused(UdpConnectionData *connection)
{
  connection->flags |= UDP_DELETE;
}

// We save up the status changes and incoming data to be reported from the
// event handler.  It is possible that several things have to get reported
// at the same time, in which case we have to report them in the order in
// which they must have occured.

static const uint8_t statusReports[] = {
  // Reported before incoming data
  EMBER_TCP_OPENED,             // also EMBER_UDP_CONNECTED for DTLS
  EMBER_TCP_ACCEPTED,
  EMBER_TCP_OPEN_FAILED,
  EMBER_TCP_TX_COMPLETE,

  // Reported after incoming data
  EMBER_TCP_REMOTE_CLOSE,
  EMBER_TCP_REMOTE_RESET
};

// Returns true if the connection is still needed.

bool emReportTcpStatus(TcpConnection *connection)
{
  if ((connection->tlsState == NULL_BUFFER
       || isDtlsConnection(connection)
       || emTlsStatusHandler(connection))
      // This check has to come second because emTlsStatusHandler()
      // must be called even if there are no events.
      && !(connection->udpData.events == 0
           && connection->incomingData == NULL_BUFFER)) {
    uint8_t i;
    for (i = 0; i < sizeof(statusReports); i++) {
      uint8_t status = statusReports[i];

      if (connection->udpData.events & status) {
        if (status == EMBER_TCP_ACCEPTED) {
#ifdef EMBER_SCRIPTED_TEST // TODO: Thread doesn't use TCP, but some tests need this.
          emberTcpAcceptHandler(connection->udpData.localPort,
                                connection->udpData.connection);
#endif
        } else {
          emLogLine(JOIN, "%d: report status %s for fd %d tls %d",
                    indexOf(connection),
                    emberTcpStatusName(status),
                    connection->udpData.connection,
                    connection->tlsState != NULL_BUFFER);
          if (isDtlsConnection(connection)) {
            EmberUdpConnectionData *temp =
              (EmberUdpConnectionData *) &connection->udpData;
            connection->udpData.statusHandler(temp, (UdpStatus) status);
          } else {
#ifdef EMBER_SCRIPTED_TEST // TODO: Thread doesn't use TCP, but some tests need this.
            emberTcpStatusHandler(connection->udpData.connection, status);
#endif
          }
        }
        connection->udpData.events &= ~status;
      }

      // EMBER_TCP_TX_COMPLETE is the last of the pre-input-processing
      // status codes.
      if (status == EMBER_TCP_TX_COMPLETE) {
        emSendTcpDataToApp(connection);
      }
    }

    if (connection->udpData.events & EMBER_TCP_DONE) {
      if (isDtlsConnection(connection)) {
        EmberUdpConnectionData *temp =
          (EmberUdpConnectionData *) &connection->udpData;
        if (connection->udpData.flags & UDP_CONNECTED) {
          connection->udpData.statusHandler(temp, EMBER_UDP_DISCONNECTED);
        }
      } else {
        emLogLine(TCP_TX, "%d: done", indexOf(connection));
#ifdef EMBER_SCRIPTED_TEST // TODO: Thread doesn't use TCP, but some tests need this.
        emberTcpStatusHandler(connection->udpData.connection, EMBER_TCP_DONE);
#endif
      }
      markConnectionUnused(&connection->udpData);
      return false;
    }
  }
  return true;
}

HIDDEN void connectionEventHandler(Event *event)
{
  uint32_t nowMs = halCommonGetInt32uMillisecondTick();
  uint32_t ticks = elapsedTimeInt32u(eventStartTimeMs, nowMs);
  bool allInUse = true;

  Buffer buffer = emBufferQueueHead(&emIpConnections);
  while (buffer != NULL_BUFFER) {
    TcpConnection *connection = (TcpConnection *) emGetBufferPointer(buffer);

    if (connection != NULL) {
      if (isTcpConnection(connection)
          || isDtlsConnection(connection)) {
        uint32_t ticksRemaining = connection->udpData.ticksRemaining << 8;

        if (ticksRemaining != 0) {
          uint16_t newTicksRemaining = (ticksRemaining - ticks) >> 8;
          if (0 < newTicksRemaining) {
            connection->udpData.ticksRemaining = newTicksRemaining;
          } else {
            connection->udpData.ticksRemaining = 0;
            if (isTcpConnection(connection)) {
              emTcpConnectionTimeout(connection);
            } else {
              emDtlsConnectionTimeout(connection);
            }
          }
        } else if (isDtlsConnection(connection)) {
          emCheckForIdleDtlsConnection(connection);
        }
        allInUse = emReportTcpStatus(connection) && allInUse;
      }
    }
    buffer = emBufferQueueNext(&emIpConnections, buffer);
  }

  if (!allInUse) {
    emRemoveUnusedConnections();
  }

  ticks = 0xFFFFFFFF;

  buffer = emBufferQueueHead(&emIpConnections);
  while (buffer != NULL_BUFFER) {
    TcpConnection *connection = (TcpConnection *) emGetBufferPointer(buffer);
    if (connection != NULL) {
      if (isTcpConnection(connection)
          || isDtlsConnection(connection)) {
        uint32_t ticksRemaining = connection->udpData.ticksRemaining << 8;

        if (ticksRemaining != 0
            && ticksRemaining < ticks) {
          ticks = ticksRemaining;
        }
      }
    }
    buffer = emBufferQueueNext(&emIpConnections, buffer);
  }

  if (ticks != 0xFFFFFFFF) {
    emLogLine(TCP_TX, "wake in %d ticks", ticks >> 8);
    eventStartTimeMs = nowMs;
    emberEventSetDelayMs(&connectionEvent, ticks);
  }

  // The dtls-join.c code needs to process DTLS packets that arrive
  // embedded in other DTLS packets.  To keep the call stack smaller
  // they queue up the messages and then schedule this event so that
  // the inner packets get processed with a short call stack.
  //
  // This may potentially take a long time, so we run it after all of
  // the above clock-interested stuff happens.
  buffer = emBufferQueueHead(&emIpConnections);
  while (buffer != NULL_BUFFER) {
    TcpConnection *connection = (TcpConnection *) emGetBufferPointer(buffer);
    if (connection != NULL) {
      if (isDtlsConnection(connection)) {
        DtlsConnection *dtlsConnection = (DtlsConnection *) connection;
        if (!emBufferQueueIsEmpty(&dtlsConnection->incomingTlsData)) {
          emDtlsStatusHandler(dtlsConnection);
        }
      }
    }
    buffer = emBufferQueueNext(&emIpConnections, buffer);
  }
}

void emRemoveConnection(UdpConnectionData *connection)
{
  markConnectionUnused(connection);
  emRemoveUnusedConnections();
}

void emRemoveUnusedConnections(void)
{
  Buffer temp = emIpConnections;
  emIpConnections = NULL_BUFFER;
  while (!emBufferQueueIsEmpty(&temp)) {
    Buffer next = emBufferQueueRemoveHead(&temp);
    UdpConnectionData *connection =
      (UdpConnectionData *) emGetBufferPointer(next);
    if (connection != NULL) {
      if (!(connection->flags & UDP_DELETE)) {
        emBufferQueueAdd(&emIpConnections, next);
      }
    }
  }
}

void emSendTcpDataToApp(TcpConnection *connection)
{
  while (connection->incomingData != NULL_BUFFER) {
#ifdef EMBER_SCRIPTED_TEST // TODO: Thread doesn't use TCP, but some tests need this.
    Buffer payload =
      emBufferQueueRemoveHead(&(connection->incomingData));
    emberTcpReadHandler(connection->udpData.connection,
                        emGetBufferPointer(payload),
                        emGetBufferLength(payload));
#endif
  }
}
