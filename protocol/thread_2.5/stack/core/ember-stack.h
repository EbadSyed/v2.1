// ember-stack.h

#ifndef EMBER_STACK_H
#define EMBER_STACK_H

// Get the stack configuration from the application first
#include PLATFORM_HEADER  //DOLATER: macro-ize this

// Headers wrapped between define-ember-api.h and undefine-ember-api.h
// require RTOS namespace resolution.
#include "include/define-ember-api.h"

// Include the public definitions
#include "include/ember.h"

// TODO: We shouldn't need both packet-buffer.h and buffer-management.h.
#include "include/packet-buffer.h"

// TODO: Put this in its own header.
// commission-proxy-app related definition that requires conflict resolution.
void emNoteExternalCommissioner(EmberUdpConnectionHandle handle, bool available);

// Other stack-specific headers that require RTOS namespace-conflict resolution.
#include "framework/eui64.h"
#include "framework/buffer-management.h"
// I am not sure that this should be here, but I don't know how many files
// would need to include it separately.   -Richard Kelsey
#include "framework/buffer-queue.h"
#include "core/log.h"

#include "app/coap/coap.h"

#include "include/undefine-ember-api.h"

// For back-compatibility with the hal, which is shared with znet.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern uint8_t emMacExtendedId[];
  #define emberGetEui64() ((uint8_t *)&emMacExtendedId)
  #define emberIsLocalEui64(eui64) \
  (MEMCOMPARE((eui64), (uint8_t *)&emMacExtendedId, EUI64_SIZE) == 0)
#endif

// This can be modified by the application in ember-configuration.c
extern uint8_t emZigbeeNetworkSecurityLevel;
extern uint16_t emberMacIndirectTimeout;
extern uint8_t emberEndDevicePollTimeout;
extern uint32_t emberSleepyChildPollTimeout;
extern uint8_t emberReservedMobileChildEntries;

//----------------------------------------------------------------
// Handy macros for exposing things only for testing.
// HIDDEN expands to 'static' in platform builds and disappears in test builds.
// ONLY_IF_EMBER_TEST(x) is x in test builds and disappears in platform builds.

#ifdef EMBER_TEST
#define HIDDEN
#define ONLY_IF_EMBER_TEST(x) x
#else
#define HIDDEN static
#define ONLY_IF_EMBER_TEST(x)
#endif

#ifdef LEAF_STACK
  #define NOT_IN_LEAF_STACK(x)
#else
  #define NOT_IN_LEAF_STACK(x) x
#endif

//**************************************************
// Stack private types

#ifdef EMBER_WAKEUP_STACK
  #define WAKEUP_STACK true
#else
  #define WAKEUP_STACK false
#endif

#define BROADCAST_ADDRESS            0xFFFF
#define EM_BROADCAST_PAN_ID          0xFFFF
#define EM_USE_LONG_ADDRESS          0xFFFE
#define NO_SHORT_ID                  0xFFFE
#define LEADER_ANYCAST_ADDRESS       0xFC00
// The commissioner anycast address is actually of the form
// 0xFC3x [0xFC30 to 0xFC37] where the index is the commissioner
// session ID mod 8.
// TODO: We support only one commissioner session ID.
#define COMMISSIONER_ANYCAST_ADDRESS 0xFC30

#define emIsValidNodeId(id) ((id) < EM_USE_LONG_ADDRESS)

// Power management status
enum {
  AWAKE,
  SLEEPY,
  ASLEEP
};

typedef uint8_t PowerStatus;

#define NO_OPTIONS 0            // just generally useful

typedef struct {
  uint8_t timestamp[8];
  uint8_t pendingTimestamp[8];
  uint32_t delayTimer;
  uint8_t networkId[16];
  uint8_t extendedPanId[8];
  uint8_t ulaPrefix[8];
  uint8_t masterKey[16];
  uint8_t pskc[16];
  uint8_t securityPolicy[3];
  uint16_t panId;
  // channel mask:
  // [0]: page
  // [1]: mask length, 4
  // [2-5]: mask
  uint8_t channelMask[6];
  // channel:
  // [0]: channel page
  // [1-2]: channel
  uint8_t channel[3];
  bool haveTimestamp;
} CommissioningDataset;

#define RIP_MAX_ROUTERS 32

typedef struct {
  uint8_t longId[8];
  uint32_t macFrameCounter;
  uint32_t mleFrameCounter;
  int8_t sequenceDelta;
  uint8_t rollingLinkMargin;
  uint8_t routerId;
  uint8_t nextHop; // rip id of the next hop (not the index)
  uint8_t metric; // rip = bits 0-3, incoming = bits 4-5, outgoing = bits 6-7
  uint8_t flags;  // age = bottom nibble, using-old-key = bit 5, mle-in = bit 6,
                  // bit 7 unused
  uint8_t routeDelta;
} RipEntry;

//----------------------------------------------------------------
// Stack configuration

// The 'option' test command depends on the order of the flag bits.
// If you change the bits you must also change common-test-commands.c.
enum {
  STACK_CONFIG_FULL_ROUTER   = 0x0001,
  STACK_CONFIG_LONG_ID_ONLY  = 0x0002,
  STACK_CONFIG_LURKER_NETWORK = 0x0004,
  STACK_CONFIG_FULL_THREAD_DEVICE = 0x0008,

  // status values
  STACK_CONFIG_NETWORK_IS_UP = 0x8000
};

extern uint16_t emStackConfiguration;

#define emSetStackConfig(x) (emStackConfiguration |= (x))
#define emCheckStackConfig(x) ((emStackConfiguration & (x)) == (x))

#define emAmRouter()      (emCheckStackConfig(STACK_CONFIG_FULL_ROUTER))
#define emUseLongIdOnly() (emCheckStackConfig(STACK_CONFIG_LONG_ID_ONLY))
#define emAmFullThreadDevice() \
  (emCheckStackConfig(STACK_CONFIG_FULL_THREAD_DEVICE))
#define emNetworkIsUp()   (emCheckStackConfig(STACK_CONFIG_NETWORK_IS_UP))
#define emOnLurkerNetwork() (WAKEUP_STACK \
                             && emCheckStackConfig(STACK_CONFIG_LURKER_NETWORK))

#define emAmRouting() \
  (emCheckStackConfig(STACK_CONFIG_NETWORK_IS_UP | STACK_CONFIG_FULL_ROUTER))

// Not exposed publicly.
typedef enum {
  ATTACH_REASON_API_CALLED,
  ATTACH_REASON_CHANGE_NODE_TYPE,
  ATTACH_REASON_PARENT_SCAN_FAILED,
  ATTACH_REASON_CHILD_ID_REQUEST_FAILED,
  ATTACH_REASON_CHILD_UPDATE_ERROR,
  ATTACH_REASON_PARENT_SENT_REJECT,
  ATTACH_REASON_PARENT_SENT_DISASSOCIATE,
  ATTACH_REASON_TRANSMIT_TO_PARENT_FAILED,
  ATTACH_REASON_PARENT_CHANGED_PARTITION,
  ATTACH_REASON_PARENT_TIMED_OUT,
  ATTACH_REASON_REBOOT_FAILED,
  ATTACH_REASON_LOST_LEADER,
  ATTACH_REASON_PARENT_LOST_LEADER,
  ATTACH_REASON_HEARD_BETTER_PARTITION,
  ATTACH_REASON_ROUTER_ID_UNASSIGNED,
  ATTACH_REASON_PARENT_ID_UNASSIGNED,
  ATTACH_REASON_ROUTER_SELECTION,
  ATTACH_REASON_HEARD_NEWER_COMM_TIMESTAMP,
  ATTACH_REASON_ORPHAN_ANNOUNCE,
  ATTACH_REASON_ADDRESS_IN_USE,
  ATTACH_REASON_ID_VERIFY_FAILED,
  ATTACH_REASON_SWITCH_TO_PENDING,
  ATTACH_REASON_PENDING_ATTACH_FAILED,
  ATTACH_REASON_JOINED,
  ATTACH_REASON_LAST,
} AttachReason;

void emStackInit(void);
void emSetNetworkStatus(EmberNetworkStatus newStatus,
                        EmberJoinFailureReason reason);
void emSaveNetwork(void);
void emBecomeLeader(void);

extern uint8_t emMaxEndDeviceChildren;    // maximum for this node
extern uint8_t emEndDeviceChildCount;     // how many we have
extern EmberNodeId emParentId;
extern uint8_t emParentLongId[8];
extern int8_t emParentPriority;

// Maximum number of addresses that a parent will allow a sleepy child to
// register.  This is the MLE64 address plus three GUA.
#define MAX_CHILD_ADDRESS_COUNT 4

// The '+ 0' prevents anyone from accidentally assigning to these.
#define emberChildCount()          (emEndDeviceChildCount + 0)
#define emberRouterChildCount()    0
#define emberMaxChildCount()       (emMaxEndDeviceChildren + 0)
#define emberMaxRouterChildCount() 0
#define emberGetParentNodeId()     (emParentId          + 0)

//----------------------------------------------------------------
void emResetNetworkState(void);

EmberNodeId emberGetNodeId(void);

void emSetNodeId(uint16_t nodeId);
void emSetPanId(uint16_t panId);
void emSetRadioPower(int8_t power);

void emNetworkStateChangedHandler(void);  // Use for ip-modem.
//----------------------------------------------------------------
// Private clusters for our application profile.  The public ones are
// in ember-types.h and start from 0x0000.  Replies use the cluster
// with the high-bit (0x8000) set, as the ZDO does.
#define EMBER_MOBILE_JOIN_CLUSTER           0x4000
#define EMBER_MOBILE_REJOIN_CLUSTER         0x4001

#define DEFAULT_PREFIX_BYTES 8
#define DEFAULT_PREFIX_BITS (DEFAULT_PREFIX_BYTES << 3)

uint8_t *emLookupLongId(uint16_t shortId);

//**************************************************
// Other Stack public stuff
EmberStatus emberSendRawMessage(uint8_t *contents, uint8_t length);

//**************************************************
// Other Stack private stuff

// For when we want actual milliseconds, or we want timing to agree
// across platforms.
#define MS_TO_MSTICKS(ms) (((ms) * MILLISECOND_TICKS_PER_SECOND) / 1000)

// modulo increment and decrement
// useful for circular buffers
#define MOD_INC(num, modulus) \
  (((num) == ((modulus) - 1)) ? 0 : ((num) + 1))

#define MOD_DEC(num, modulus) \
  (((num) == 0) ? ((modulus) - 1) : ((num) - 1))

// Returns true if the first 'count' bytes pointed to by 'bytes' are
// all 'target'.
bool emMemoryByteCompare(const uint8_t *bytes, uint8_t count, uint8_t target);
#define emIsMemoryZero(bytes, count) (emMemoryByteCompare((uint8_t *)(bytes), (count), 0))
#define isNullEui64(eui64) (emIsMemoryZero((eui64), EUI64_SIZE))

// For comparing structs with a 'contents' field, like keys or dagIds.
#define equalContents(a, b, len) \
  (MEMCOMPARE((a)->contents, (b)->contents, len) == 0)

// Returns true if t1 is a later time than t2.
#define timeGTint8u(t1, t2) \
  (!timeGTorEqualInt8u(t2, t1))

#undef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Choose a random number between 'low' and 'high'.
#define randomRange(low, high) ((low) + (halCommonGetRandom() % ((high) - (low))))

// Random number between 0 and 2^log - 1.
#define expRandom(log) (halCommonGetRandom() & (BIT32((log)) - 1))

// Random number between  -2^logJitter and 2^logJitter - 1
#define expJitter(logJitter) (expRandom((logJitter) + 1) - BIT32((logJitter)))

// Used to reverse bit order of channel masks.
uint32_t emReverseBitsInEachByte(uint32_t v);

// Decrements the contents of delayTicksLoc by ticksPassed and, if the
// result is greater than zero, sets the contents of minTicksNeededLoc
// to be the minimum of its current contents and the new contents of
// delayTicksLoc.

void emDecrementTicks(uint16_t *delayTicksLoc,
                      uint16_t ticksPassed,
                      uint16_t *minTicksNeededLoc);

extern EmberTaskId emStackTask;
void emPrintStackEvents(uint8_t port);
void emPrintStackBuffers(uint8_t port);

typedef uint8_t PacketDispatchType;
typedef uint8_t MacFrameType;
typedef uint8_t MacAddressMode;
typedef uint8_t HeaderOptionType;

// MAC transmit priority.
// High priority headers go on the front of the queue.
// Normal priority headers go on the back of the queue.
// Only beacon requests and orphan notifications can be
// sent during a scan.  They are submitted with SCAN_OKAY
// and go on the front of the queue.

enum {
  TRANSMIT_PRIORITY_HIGH,
  TRANSMIT_PRIORITY_NORMAL,
  TRANSMIT_PRIORITY_SCAN_OKAY
};
typedef uint8_t TransmitPriority;

// MAC scan types.  Note:  this is an expanded list that includes internal as
// well as external scan types.
//
// The RADIO_OFF scan is not really a scan at all.  It is used to
// temporarilly turn off the radio in order to use the TX and RX
// buffers for other purposes.
// Start the scan with:
//   emberStartScan(EM_START_RADIO_OFF_SCAN, 0, 0);
// Then wait for emMacScanType to be set to EM_RADIO_OFF_SCAN,
// at which point the radio is off.  There may be a brief
// delay while the radio finishes transmitting.
//
// Call emberStopScan() to restart the radio and the MAC.

enum {
  EM_STACK_ENERGY_SCAN = EMBER_ACTIVE_SCAN + 1,
  EM_STACK_ACTIVE_SCAN,
  EM_MGMT_ENERGY_SCAN, // When asked by commissioner
  EM_MGMT_ACTIVE_SCAN, // When asked by commissioner
  EM_PASSIVE_SCAN,
  EM_ORPHAN_SCAN,
  EM_START_RADIO_OFF_SCAN,    // Fake scan that is used to turn off the radio.
  EM_RADIO_OFF_SCAN,          // Indicates that the radio is off.
  EM_STACK_ZLL_ACTIVE_SCAN,
  EM_SCAN_STOPPING,
  EM_SCAN_IDLE,
};

// Defined in stack/core/ip-stack.c, but used widely.
extern EmberNodeType emNodeType;

// An enumeration in zigbee/aps-security.h.
//typedef uint8_t ZigbeeKeyType;

#define SECURITY_BLOCK_SIZE         16 // Key, Nonce, and standalone block size

typedef enum {
  NETWORK_KEY,
  MLE_KEY,
  LEGACY_KEY
} MessageKeyType;

typedef struct RetryEntryS {
  PacketHeader header;
  uint8_t attempts;  // Top/bottom nibble is successful/remaining attempts.
  uint16_t timer;
  uint16_t startTime;
} RetryEntry;

typedef enum {
  DHCPV6_REPLY,
  PARENT_ADDRESS_REGISTRATION,
} ChildPendingReplies;

// Bitmask indexed by ChildPendingReplies.  If the i'th bit is set then
// the corresponding reply is expected and the stack must check for
// inocming messages.
extern uint16_t emPendingReplies;

extern EventQueue emStackEventQueue;

#ifdef IP_MODEM_LIBRARY
// For RTOS stack thread: an event queue event that runs all stack events.
extern Event emStackEvent;
#endif

typedef struct {
  EmberMacBeaconData apiBeaconData;
  uint16_t joinPort;
  uint16_t commissionPort;
  bool allowingCommission;
} MacBeaconData;

#ifdef UNIX_HOST
#define emMicroBusy(busy)
#else
void emMicroBusy(bool busy);
#endif

//----------------------------------------------------------------
// Node Type Macros.

#define LURKER_NODE_TYPE_BIT 0x80
#define EMBER_LURKER 0x06 // must not conflict with EmberNodeType enums

void emSetNodeType(EmberNodeType type, bool preserveLurker);

#define emNodeTypeIsLurker() \
  (EMBER_LURKER == emNodeType)

#define emNodeTypeIsRouter() \
  (EMBER_ROUTER == emNodeType)

#define emNodeTypeIsEndDevice()   \
  (EMBER_END_DEVICE == emNodeType \
   || EMBER_SLEEPY_END_DEVICE == emNodeType)

#define emNodeTypeIsSleepy() \
  (EMBER_SLEEPY_END_DEVICE == emNodeType)

#define emAmDarkRouter()      \
  (EMBER_ROUTER == emNodeType \
   && !emCheckStackConfig(STACK_CONFIG_FULL_ROUTER))

#define emAmEndDevice() (emNodeTypeIsEndDevice() || emAmDarkRouter())

// EMIPSTACK-1494: we don't use EMBER_MINIMAL_END_DEVICE internally, instead
// we set the node type to EMBER_END_DEVICE. We distinguish between the two
// by looking at the config flags (minimal end devices don't have the
// STACK_CONFIG_FULL_THREAD_DEVICE flag set).
#define emAmMinimalEndDevice() \
  (EMBER_END_DEVICE == emNodeType && !emAmFullThreadDevice())

// TODO Need something for end devices + minimal end devices
// (example, they have similar aging behavior in child-aging.c)

#ifdef EMBER_TEST
// This function asserts that the length of the freelist equals
// emPacketBufferFreeCount.
uint8_t emFreePacketBufferCount(void);
void printPacketBuffers(EmberMessageBuffer buffer);
void simPrintBytes(char *prefix, uint8_t *bytes, uint16_t count);
void simPrintBuffer(char *prefix, Buffer buffer);
void simPrintStartLine(void);
#endif

//----------------------------------------------------------------
// Internal buffer utilities.

// uint8_t emNormalizeBufferIndex(EmberMessageBuffer *bufferLoc, uint16_t index);
// void emCopyToNormalizedBuffers(PGM_P contents,
//                                EmberMessageBuffer buffer,
//                                uint8_t startIndex,
//                                uint8_t length,
//                                uint8_t direction);
// bool emSetLinkedBuffersLength(EmberMessageBuffer buffer,
//                                  uint16_t oldLength,
//                                  uint16_t newLength);
// extern uint8_t emAvailableStackBuffer;

//----------------------------------------------------------------
// A simple printf()-like facility for creating messages.  For now this only
// works for messages that don't cross buffer boundaries.
//
// This has not been released to customers, mostly because of the restriction
// about crossing buffer boundaries.
//
// The code here uses format strings to specify how values are to be encoded.
// Each character in a format string gives the format for one
// value.  The formats are case-insenstive.  The current format characters are:
// '1'      A one-byte unsigned value.
// '2'      A two-byte unsigned value encoded as the less-significant byte
//          followed by the more-significant byte.
// '4'      A 4-byte unsigned value encoded as the least-significant byte
//          to the most significant byte.  This use takes precedence over the
//          formatting ranges described below.
// '3'-'9'  A pointer to three to nine bytes (usually 8 for an EUI64) that
//          is to be copied into the packet.
// 'A'-'G'  A pointer to ten to sixteen bytes that is to be copied into the
//          packet.
// 's'      A sequence of unsigned bytes.  When encoding the first supplied
//          value is a pointer to the data and the second value is the number
//          of bytes.
// [Not yet implemented:
// 'p'      Same as 's' except that the bytes are in program space.
// ]

EmberMessageBuffer emMakeMessage(uint8_t startIndex, PGM_P format, ...);

EmberMessageBuffer emMakeMessageUsingVaList(uint8_t startIndex,
                                            PGM_P format,
                                            va_list argPointer);

// Similar to emMakeMessageUsingVaList(), except that it operates
// on a flat buffer (up to maxLength).  The buffer is assumed to be
// big enough to hold the message
uint8_t emWriteMessage(uint8_t* buffer,
                       uint8_t maxLength,
                       uint8_t startIndex,
                       PGM_P format,
                       ...);
uint8_t emWriteMessageUsingVaList(uint8_t* buffer,
                                  uint8_t maxLength,
                                  uint8_t startIndex,
                                  PGM_P format,
                                  va_list argPointer);

uint8_t emParseBuffer(EmberMessageBuffer message,
                      uint8_t startIndex,
                      PGM_P format,
                      ...);

uint8_t emParseBufferFromVaList(EmberMessageBuffer message,
                                uint8_t startIndex,
                                PGM_P format,
                                va_list elements);

//----------------------------------------------------------------
// Data structs to contain all the things the Thread Test Harness
// may need to print to pass tests.
typedef enum {
  THREAD_TEST_LOG_STACK,
  THREAD_TEST_LOG_COAP_TX,
  THREAD_TEST_LOG_COAP_ACK,
  THREAD_TEST_LOG_REQUEST_URI
} ThreadTestLogType;

typedef struct {
  bool tx;
  CoapType type;
  EmberCoapCode code;
  const uint8_t *uri;
  const uint8_t *payload;
  uint16_t payloadLength;
  const EmberCoapRequestInfo *requestInfo;
} ThreadTestLogStackMessage;

typedef struct {
  CoapMessage *message;
  const uint8_t *uri;
  Buffer payloadExtraBuffer;
} ThreadTestLogTxMessage;

typedef struct {
  CoapMessage *message;
  CoapRetryEvent *event;
} ThreadTestLogAckUriMessage;

typedef union {
  ThreadTestLogStackMessage stackMessage;
  ThreadTestLogTxMessage txMessage;
  ThreadTestLogAckUriMessage ackUriMessage;
} ThreadTestLogData;

//----------------------------------------------------------------
// Application registered callbacks (see stack/include/network-management.h)

extern bool (*emDropCallback)(PacketHeader header, Ipv6Header *ipHeader);
extern void (*emSerialTransmitCallback)(uint8_t type, PacketHeader header);
extern void (*emThreadTestLogCallback)(ThreadTestLogType type, ThreadTestLogData *data);

// Thread test log message callback
void emRegisterThreadTestLogCallback
  (void (*log)(ThreadTestLogType type, ThreadTestLogData *data));

//----------------------------------------------------------------
void emNoteTimeHandler(const char *label);
#define emNoteTime(label)
// #define emNoteTime(label) emNoteTimeHandler(label)

#if defined EMBER_TEST
#define EMBER_TEST_EXTERNAL
#define EMBER_TEST_EXTERNAL_PGM
#else
#define EMBER_TEST_EXTERNAL static
#define EMBER_TEST_EXTERNAL_PGM PGM static
#endif

#ifdef EMBER_TEST
const char *emErrorString(EmberStatus errorCode);
extern uint16_t simulatorId;
extern uint16_t rebootCount;
void simPrint(char *format, ...);
void debugSimPrint(char *format, ...);
void debugPrintTextAndHex(const char* text,
                          const uint8_t* hexData,
                          uint8_t length,
                          uint8_t spaceEveryXChars,
                          bool finalCr);

typedef struct ParcelS {
  uint32_t tag;           // for safety
  int length;
  uint8_t contents[0];
} Parcel;

void scriptTestCheckpoint(char* string);

#elif !defined(EMBER_TEST) && !defined(EMBER_SCRIPTED_TEST)
  #define debugSimPrint(...)
  #define scriptTestCheckpoint(string)
  #define debugPrintTextAndHex(...)
#endif

void emInitStackCoapMessage(CoapMessage *message,
                            const EmberIpv6Address *destination,
                            uint8_t *payload,
                            uint16_t payloadLength);
void emInitStackMl16CoapMessage(CoapMessage *message,
                                EmberNodeId destination,
                                uint8_t *payload,
                                uint16_t payloadLength);

// The assertReturn macro helps resolve CSTAT reported issues where
// the condition being reported as possibly being invalid was
// previously checked using assert.  For example, the following
// code produces this CSTAT warning.
//
//      MISRAC2012-Rule-18.1_b - Array `nodeTypeNames' 1st
//      subscript interval [0,2147483519] may be out of bounds [0,7]
//
//    assert(index < COUNTOF(nodeTypeNames));
//    return (const uint8_t*)nodeTypeNames[index];
//
// To fix the warning you can change the code to the following
//
//    if (index >= COUNTOF(nodeTypeNames)) {
//      assert(false);
//      return NULL;
//    }
//    return (const uint8_t*)nodeTypeNames[index];
//
// or use the assertReturn macro and change the code to
//
//    assertReturn(index < COUNTOF(nodeTypeNames));
//    return (const uint8_t*)nodeTypeNames[index];
#define assertReturn(condition, ...) ASSERT_RETURN((condition), ##__VA_ARGS__)
#define ASSERT_RETURN(...) ASSERT_RETURN_X(__VA_ARGS__, _VALUE, _VOID)(__VA_ARGS__)
#define ASSERT_RETURN_X(X, Y, Z, ...) ASSERT_RETURN ## Z
#define ASSERT_RETURN_VOID(condition) \
  do {                                \
    if (!(condition)) {               \
      assert(false);                  \
      return;                         \
    }                                 \
  } while (0)
#define ASSERT_RETURN_VALUE(condition, returnValue) \
  do {                                              \
    if (!(condition)) {                             \
      assert(false);                                \
      return (returnValue);                         \
    }                                               \
  } while (0)

#endif // EMBER_STACK_H
