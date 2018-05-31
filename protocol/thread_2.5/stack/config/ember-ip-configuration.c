/** @file ember-ip-configuration.c
 * @brief User-configurable stack memory allocation and convenience stubs
 * for little-used callbacks.
 *
 * \b Note: Application developers should \b not modify any portion
 * of this file. Doing so may lead to mysterious bugs. Allocations should be
 * adjusted only with macros in a custom CONFIGURATION_HEADER.
 *
 * <!--Copyright 2009 by Ember Corporation. All rights reserved.         *80*-->
 */
#include PLATFORM_HEADER
#include "stack/core/ember-stack.h"
#include "hal/hal.h"
#include "config/config.h"
#include "stack/config/ember-configuration-defaults.h"

// On RTOS, the appliction thread is in charge of memory allocation.
#ifndef IP_MODEM_LIBRARY

// Declare heap memory contents based on EMBER_HEAP_SIZE
#if defined(CORTEXM3)
VAR_AT_SEGMENT(NO_STRIPPING uint16_t heapMemory[(EMBER_HEAP_SIZE) / 2], __EMHEAP__);
#elif defined(EMBER_STACK_COBRA)
// on Cobra, the heap is allocated the entire unused portion of memory by the linker
#elif defined(EMBER_MBEDTLS_STACK)
uint16_t heapMemory[(EMBER_HEAP_SIZE + EMBER_MALLOC_HEAP_SIZE_BYTES) / 2];
const uint32_t heapMemorySize = EMBER_HEAP_SIZE + EMBER_MALLOC_HEAP_SIZE_BYTES;
#else
uint16_t heapMemory[(EMBER_HEAP_SIZE) / 2];
const uint32_t heapMemorySize = EMBER_HEAP_SIZE;
#endif

uint8_t emberChildTableSize = EMBER_CHILD_TABLE_SIZE;
static ChildStatusFlags childStatus[EMBER_CHILD_TABLE_SIZE];
ChildStatusFlags *emChildStatus = childStatus;
static EmberNodeId childIdTable[EMBER_CHILD_TABLE_SIZE + 1];
EmberNodeId *emChildIdTable = childIdTable;
static uint8_t childLongIdTable[(EMBER_CHILD_TABLE_SIZE + 1) * 8];
uint8_t *emChildLongIdTable = childLongIdTable;
static uint32_t childTimers[EMBER_CHILD_TABLE_SIZE];
uint32_t *emChildTimers = childTimers;
static uint32_t childTimeouts[EMBER_CHILD_TABLE_SIZE];
uint32_t *emChildTimeouts = childTimeouts;
static uint32_t childMacFrameCounters[EMBER_CHILD_TABLE_SIZE];
uint32_t *emChildMacFrameCounters = childMacFrameCounters;
static uint32_t childMleFrameCounters[EMBER_CHILD_TABLE_SIZE];
uint32_t *emChildMleFrameCounters = childMleFrameCounters;
static int8_t childSequenceDeltas[EMBER_CHILD_TABLE_SIZE];
int8_t *emChildSequenceDeltas = childSequenceDeltas;
static uint16_t childTransactionTimers[EMBER_CHILD_TABLE_SIZE];
uint16_t *emChildLastTransactionTimesSec = childTransactionTimers;
uint8_t emMaxEndDeviceChildren = EMBER_CHILD_TABLE_SIZE;
uint16_t emberMacIndirectTimeout = EMBER_INDIRECT_TRANSMISSION_TIMEOUT;
uint32_t emberSleepyChildPollTimeout = EMBER_SLEEPY_CHILD_POLL_TIMEOUT;
uint8_t emberEndDevicePollTimeout = EMBER_END_DEVICE_POLL_TIMEOUT;
uint8_t emZigbeeNetworkSecurityLevel = EMBER_SECURITY_LEVEL;
// Needed by some phy files
uint8_t emberReservedMobileChildEntries = 0;

uint8_t emberRetryQueueSize = EMBER_RETRY_QUEUE_SIZE;
static RetryEntry retryEntries[EMBER_RETRY_QUEUE_SIZE];
RetryEntry *emRetryEntries = retryEntries;

uint8_t emRipTableSize = RIP_MAX_ROUTERS + RIP_MAX_LURKERS;
static RipEntry ripTable[RIP_MAX_ROUTERS + RIP_MAX_LURKERS];
RipEntry *emRipTable = ripTable;

PGM uint8_t emTaskCount = EMBER_TASK_COUNT;
EmberTaskControl emTasks[EMBER_TASK_COUNT];

#endif // ifndef IP_MODEM_LIBRARY

// On RTOS, the stack thread must implement stack callbacks, not the app thread.
#if (!defined(RTOS) || defined(IP_MODEM_LIBRARY))

const EmberVersion emberVersion = {
  EMBER_MAJOR_VERSION,
  EMBER_MINOR_VERSION,
  EMBER_PATCH_VERSION,
  EMBER_VERSION_TYPE,
  EMBER_BUILD_NUMBER,
  EMBER_CHANGE_NUMBER,
};

#ifndef EMBER_APPLICATION_HAS_GET_VERSIONS
void emApiGetVersions(void)
{
  static const uint8_t emBuildTimestamp[] = __DATE__ " "__TIME__;
  emApiGetVersionsReturn((const uint8_t *)EMBER_VERSION_NAME,
                         EMBER_MANAGEMENT_VERSION,
                         EMBER_FULL_VERSION,
                         EMBER_BUILD_NUMBER,
                         EMBER_VERSION_TYPE,
                         emBuildTimestamp);
}
#endif

#ifndef EMBER_APPLICATION_HAS_APP_NAME
const char * const emAppName = "";
#endif

#ifndef EMBER_APPLICATION_HAS_SIM_NOTIFY_SERIAL_RX
bool simNotifySerialRx(const uint8_t *data, uint16_t length, Buffer rxBufferList)
{
  return true;
}
#endif

#ifndef EMBER_APPLICATION_HAS_SIM_NOTIFY_HEADER_RX
#include "app/ip-ncp/uart-link-protocol.h"
#include "app/ip-ncp/ip-modem-link.h" // for ManagementType

bool simNotifyHeaderRx(ManagementType managementType,
                       const uint8_t *data,
                       uint8_t length)
{
  return true;
}
#endif

// Framework applications, which are distinguished by the SILABS_THREAD_CONFIG
// define, have generated stubs for any unimplemented handlers and returns,
// making these stubs unnecessary.
#if !defined(SILABS_THREAD_CONFIG) && !defined(__THREAD_CONFIG__)

#ifndef EMBER_APPLICATION_HAS_EXTERNAL_MEMORY_ALLOCATION
void *emberAllocateMemoryForPacketHandler(uint32_t size, void **objectRef)
{
  return NULL;
}
void emberFreeMemoryForPacketHandler(void *objectRef)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_COUNTER_HANDLER
void emApiCounterHandler(EmberCounterType type, uint16_t increment)
{
}
uint16_t emApiCounterValueHandler(EmberCounterType type)
{
  return 0;
}
  #if (!defined(UNIX_HOST) && !defined(UNIX_HOST_SIM))
void emApiGetCounter(EmberCounterType type)
{
}
void emApiClearCounters(void)
{
}
  #endif
#endif

#if (defined (EMBER_TEST)        \
  || defined(QA_THREAD_TEST)     \
  || defined(EMBER_WAKEUP_STACK) \
  || defined(RTOS))
#ifndef EMBER_APPLICATION_HAS_WAKEUP_HANDLER
void emApiWakeupHandler(EmberWakeupReason reason,
                        EmberWakeupState state,
                        uint16_t remainingMs,
                        uint8_t dataByte,
                        uint16_t otaSequence)
{
}
#endif
#endif // EMBER_WAKEUP_STACK

#ifndef EMBER_APPLICATION_HAS_NETWORK_STATUS_CHANGED_HANDLER
void emApiNetworkStatusChangedHandler(EmberNetworkStatus oldStatus,
                                      EmberNetworkStatus newStatus)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_UDP_HANDLER
void emApiUdpHandler(const uint8_t *destination,
                     const uint8_t *source,
                     uint16_t localPort,
                     uint16_t remotePort,
                     const uint8_t *payload,
                     uint16_t payloadLength)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_UDP_MULTICAST_HANDLER
void emApiUdpMulticastHandler(const uint8_t *destinationIpv6Address,
                              const uint8_t *sourceIpv6Address,
                              uint16_t localPort,
                              uint16_t remotePort,
                              const uint8_t *packet,
                              uint16_t length)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_INCOMING_ICMP_HANDLER
void emApiIncomingIcmpHandler(Ipv6Header *ipHeader)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_MAC_PASSTHROUGH_FILTER_HANDLER
bool emApiMacPassthroughFilterHandler(uint8_t *macHeader)
{
  return false;
}
#endif

#ifndef EMBER_APPLICATION_HAS_MAC_PASSTHROUGH_MESSAGE_HANDLER
void emApiMacPassthroughMessageHandler(PacketHeader header)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_RSSI_FILTER_HANDLER
bool emApiMacRssiFilterHandler(uint8_t *macHeader)
{
  return false;
}

void emApiMacRssiHandler(int8_t currentRssi)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_BUTTON_TICK
void emButtonTick(void)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_GET_VERSIONS_RETURN
void emApiGetVersionsReturn(const uint8_t *versionName,
                            uint16_t managementVersionNumber,
                            uint16_t stackVersionNumber,
                            uint16_t stackBuildNumber,
                            EmberVersionType versionType,
                            const uint8_t *buildTimestamp)
{
  assert(false);
}
#endif

#ifndef EMBER_APPLICATION_HAS_SWITCH_TO_NEXT_NETWORK_KEY_HANDLER
void emApiSwitchToNextNetworkKeyHandler(EmberStatus status)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_ASH_STATUS_HANDLER
#include "hal/micro/generic/ash-v3.h" // for AshState

void emberAshStatusHandler(AshState state)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_NETWORK_STATE_CHANGED_HANDLER
void emNetworkStateChangedHandler(void)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_EVENT_DELAY_UPDATED_FROM_ISR_HANDLER
void emApiEventDelayUpdatedFromIsrHandler(Event *event)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_DIAGNOSTIC_ANSWER_HANDLER
void emApiDiagnosticAnswerHandler(EmberStatus status,
                                  const EmberIpv6Address *remoteAddress,
                                  const uint8_t *payload,
                                  uint8_t payloadLength)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_THREAD_TEST_LOG_STACK_HANDLER
void emApiThreadTestLogStackMessageHandler(bool tx,
                                           const uint8_t *type,
                                           const uint8_t *code,
                                           const uint8_t *uri,
                                           const uint8_t *payload,
                                           uint16_t payloadLength,
                                           EmberIpv6Address remoteAddress,
                                           uint16_t remotePort,
                                           uint8_t const *token,
                                           uint8_t tokenLength)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_THREAD_TEST_LOG_TX_HANDLER
void emApiThreadTestLogTxMessageHandler(uint16_t messageId,
                                        EmberIpv6Address remoteAddress,
                                        uint16_t remotePort,
                                        const uint8_t *payload,
                                        uint16_t payloadLength,
                                        uint8_t convertedUri[],
                                        const uint8_t *type,
                                        const uint8_t *code,
                                        uint8_t *payloadBuffer,
                                        uint16_t payloadBufferLength)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_THREAD_TEST_LOG_ACK_HANDLER
void emApiThreadTestLogAckUriMessageHandler(uint16_t messageId,
                                            const uint8_t *payload,
                                            uint16_t payloadLength,
                                            uint8_t *uri)
{
}
#endif

#ifndef EMBER_APPLICATION_HAS_MICRO_BUSY_HANDLER
void emberMicroBusyHandler(bool busy)
{
}
#endif

// This mechanism does not work for the RTOS build, because these
// stubs get compiled into the ip-modem-library and the application
// can't override them.  We need a different mechanism.

// #ifndef EMBER_APPLICATION_HAS_START_HOST_JOIN_CLIENT_HANDLER
// void emberStartHostJoinClientHandler(const uint8_t *parentAddress) {}
// #endif

// #ifndef EMBER_APPLICATION_HAS_IP_PACKET_RECEIVED_HANDLER
// #include "app/ip-ncp/uart-link-protocol.h" // for SerialLinkMessageType
// struct pbuf;
// void emberProcessIpPacketReceived(struct pbuf *packet,
//                                   SerialLinkMessageType type)
// {
// }
// #endif

// #ifndef EMBER_APPLICATION_HAS_MANAGEMENT_COMMAND_ENQUEUED_HANDLER
// void emberManagementCommandEnqueuedHandler(void) {}
// #endif

#endif // !SILABS_THREAD_CONFIG && !__THREAD_CONFIG__

#endif // (! defined(RTOS) || defined(IP_MODEM_LIBRARY))
