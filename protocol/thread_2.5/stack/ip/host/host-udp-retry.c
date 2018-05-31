// File: host-udp-retry.c
//
// Description: UDP retry functionality for UNIX Hosts
//
// Copyright 2014 by Silicon Laboratories. All rights reserved.             *80*

#include "stack/core/ember-stack.h"
#include "hal/hal.h"
#include "stack/framework/event-queue.h"
#include "stack/ip/host/host-udp-retry.h"
#include "plugin/serial/serial.h"

typedef struct {
  Event event;
  uint16_t token;
  uint16_t localPort;
  uint16_t remotePort;
  Buffer data;
  EmberIpv6Address target;
  uint8_t retryCount;
  RetryHandler *retryHandler;
  uint16_t retryDelayMs;
} UdpRetryEvent;

static EventQueue eventQueue = { 0 };

static EmberStatus send(UdpRetryEvent *event)
{
  return emberSendUdp(event->target.bytes,
                      event->localPort,
                      event->remotePort,
                      emGetBufferPointer(event->data),
                      emGetBufferLength(event->data));
}

static void udpRetryEventHandler(Event *preEvent)
{
  UdpRetryEvent *event = (UdpRetryEvent*)preEvent;
  assert(event->retryCount > 0);
  event->retryCount--;
  event->retryHandler(event->token, event->retryCount);

  if (event->retryCount == 0) {
    emberEventSetInactive((Event*)event);
  } else {
    if (!send(event)) {
      // emberSerialPrintfLine(APP_SERIAL, "Failed to send retry");
    }

    // schedule!
    emberEventSetDelayMs((Event*)event, event->retryDelayMs);
  }
}

static void udpEventMarker(UdpRetryEvent *event)
{
  emMarkBuffer(&event->data);
}

static const EventActions udpEventActions = {
  &eventQueue,
  (void (*)(Event *))udpRetryEventHandler,
  (void (*)(Event *))udpEventMarker,
  "udp retry"
};

void emInitializeUdpRetry(void)
{
  emInitializeEventQueue(&eventQueue);
}

void emMarkUdpRetryBuffers(void)
{
  emberMarkEventQueue(&eventQueue);
}

void emRunUdpRetryEvents(void)
{
  emberRunEventQueue(&eventQueue);
}

EmberStatus emAddUdpRetry(const EmberIpv6Address *target,
                          uint16_t localPort,
                          uint16_t remotePort,
                          const uint8_t *payload,
                          uint16_t payloadLength,
                          uint16_t token,
                          uint8_t retryLimit,
                          uint16_t retryDelayMs,
                          RetryHandler *retryHandler)
{
  Buffer eventBuffer = emAllocateBuffer(sizeof(UdpRetryEvent));
  UdpRetryEvent *event = (UdpRetryEvent*)emGetBufferPointer(eventBuffer);
  event->event.actions = &udpEventActions;
  event->event.next = NULL;

  MEMCOPY(&event->target, target, sizeof(EmberIpv6Address));
  event->token = token;
  event->localPort = localPort;
  event->remotePort = remotePort;
  event->retryCount = retryLimit;
  event->retryDelayMs = retryDelayMs;
  event->retryHandler = retryHandler;

  // copy the payload
  event->data = emAllocateBuffer(payloadLength);
  assert(event->data != NULL_BUFFER);
  uint8_t *dataBytes = emGetBufferPointer(event->data);
  MEMCOPY(dataBytes, payload, payloadLength);

  // schedule!
  emberEventSetDelayMs((Event*)event, event->retryDelayMs);
  return send(event);
}

static bool udpEventPredicate(Event *event, void *token)
{
  return (((UdpRetryEvent *)event)->token) == *(uint16_t*)token;
}

void emRemoveUdpRetry(uint16_t token)
{
  // remove the event
  emberFindEvent(&eventQueue,
                 &udpEventActions,
                 udpEventPredicate,
                 (void*)&token);
}
