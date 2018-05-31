/**
 * @file stack/include/ember-types.h
 * @brief Ember data type definitions.
 *
 *  See @ref utilities for details.
 *
 * <!--Author(s): Richard Kelsey (richard@ember.com) -->
 * <!--Lee Taylor (lee@ember.com) -->
 *
 * <!--Copyright 2006 by Ember Corporation. All rights reserved.         *80*-->
 */

/**
 * @addtogroup utilities
 *
 * See ember-types.h for source code.
 * @{
 */

#ifndef EMBER_TYPES_H
#define EMBER_TYPES_H

#include "stack/config/ember-configuration-defaults.h"

/**
 * @brief Defines the maximum value of an unsigned short data type.
 */
#define INT16U_MAX ((uint16_t)(~(uint16_t)0))

/**
 * @name Miscellaneous Ember Types
 */
//@{

/**
 * @brief Size of EUI64 (an IEEE address) in bytes (8).
 */
#define EUI64_SIZE 8

/**
 * @brief Size of an encryption key in bytes (16).
 */
#define EMBER_ENCRYPTION_KEY_SIZE 16

/**
 * @brief Size of an extended PAN identifier in bytes (8).
 */
#define EXTENDED_PAN_ID_SIZE 8

/**
 * @brief Size of a leader EUI64 in bytes (8).
 */
#define LEADER_SIZE EUI64_SIZE

/**
 * @brief Size of a network ID in bytes (16).
 */
#define EMBER_NETWORK_ID_SIZE 16

/**
 * @brief Maximum size of a device join key (PSKd) in bytes (32).
 */
#define EMBER_JOIN_KEY_MAX_SIZE 32

/**
 * @brief  Return type for Ember functions.
 */
#ifndef __EMBERSTATUS_TYPE__
#define __EMBERSTATUS_TYPE__
typedef uint8_t EmberStatus;
#endif //__EMBERSTATUS_TYPE__

#include "stack/include/error.h"

/**
 * @brief EUI 64-bit ID (an IEEE address).
 *
 * Due to an unfortunate choice by the IEEE, EUI64s are stored in reverse order
 * in 802.15.4 headers.  As a consequence, they are stored in reverse order in
 * the EmberEui64 type as well.
 */
typedef struct { uint8_t bytes[EUI64_SIZE];  } EmberEui64;

/**
 * @brief Obsolete version of EUI64 structure used by some platform-dependent
 * applications.  Use ::EmberEui64.
 */
typedef uint8_t EmberEUI64[EUI64_SIZE];

/** @brief  An IPv6 Prefix structure. */
typedef struct { uint8_t bytes[8];  } EmberIpv6Prefix;

/** @brief  An IPv6 Address structure. */
typedef struct { uint8_t bytes[16]; } EmberIpv6Address;

/** @brief This data structure contains the key data that is passed
 *   into various other functions. */
typedef struct {
  /** This is the key byte data. */
  uint8_t contents[EMBER_ENCRYPTION_KEY_SIZE];
} EmberKeyData;

/**
 * @brief 16-bit 802.15.4 network address.
 */
typedef uint16_t EmberNodeId;

/**
 * @brief 802.15.4 PAN ID.
 */
typedef uint16_t EmberPanId;

/**
 * @brief The maximum 802.15.4 channel number is 26.
 */
#define EMBER_MAX_802_15_4_CHANNEL_NUMBER 26

/**
 * @brief The minimum 802.15.4 channel number is 11.
 */
#define EMBER_MIN_802_15_4_CHANNEL_NUMBER 11

/**
 * @brief There are sixteen 802.15.4 channels.
 */
#define EMBER_NUM_802_15_4_CHANNELS \
  (EMBER_MAX_802_15_4_CHANNEL_NUMBER - EMBER_MIN_802_15_4_CHANNEL_NUMBER + 1)

/**
 * @brief Bitmask to scan all 802.15.4 channels.
 */
#define EMBER_ALL_802_15_4_CHANNELS_MASK 0x07FFF800UL

/**
 * @brief The network ID of the coordinator in a ZigBee network is 0x0000.
 */
#define EMBER_ZIGBEE_COORDINATOR_ADDRESS 0x0000

/**
 * @brief A distinguished network ID that will never be assigned
 * to any node.  Used to indicate the absence of a node ID.
 */
#define EMBER_NULL_NODE_ID 0xFFFF

/**
 * @brief Type of Ember software version
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberVersionType
#else
typedef uint8_t EmberVersionType;
enum
#endif
{
  EMBER_VERSION_TYPE_INTERNAL = 0,
  EMBER_VERSION_TYPE_ALPHA    = 1,
  EMBER_VERSION_TYPE_BETA     = 2,
  EMBER_VERSION_TYPE_GA       = 3,
  EMBER_VERSION_TYPE_SPECIAL  = 4,
  EMBER_VERSION_TYPE_LEGACY   = 5,
};
#define EMBER_VERSION_TYPE_MAX EMBER_VERSION_TYPE_LEGACY

#define EMBER_VERSION_TYPE_NAMES \
  "Internal",                    \
  "Alpha",                       \
  "Beta",                        \
  "GA",                          \
  "Special",                     \
  "Legacy",                      \

/**
 * @brief For use when declaring data that holds the
 * Ember software version type.
 */
typedef struct {
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
  EmberVersionType type;
  uint16_t build;
  uint32_t change;
} EmberVersion;

/**
 * @brief For use when declaring a Buffer.
 */
typedef uint16_t Buffer;

/**
 * @brief For use when declaring a buffer to hold a
 * message.
 */
typedef uint16_t EmberMessageBuffer;

/**
 * @brief For use when declaring a buffer to hold a
 * packet header.
 */
typedef Buffer PacketHeader;

/**
 * @brief Denotes a null buffer.
 */
#define NULL_BUFFER 0x0000

/**
 * @brief For use when declaring data that holds
 * child status flags.
 */
typedef uint16_t ChildStatusFlags;

/**
 * @brief A structure that holds an IPv6 header.
 * All values are in their local byte order (as opposed to network
 * byte order, which might be different).
 *
 * The order has been rearranged to avoid the need for padding.
 * The version is known to be 6 so it is not included.
 */
typedef struct {
  // The actual IPv6 header fields.
  uint16_t      ipPayloadLength;       // Includes extension headers
  uint32_t      flowLabel;
  uint8_t       trafficClass;
  uint8_t       nextHeader;
  uint8_t       hopLimit;
  uint8_t       source[16];
  uint8_t       destination[16];

  // A pointer to the first byte after the IP header.
  // The ipPayload includes any IPv6 extension headers present.
  uint8_t*      ipPayload;

  // These fields are necessary due to the presence of IPv6 extension headers.
  uint8_t       transportProtocol;
  uint8_t*      transportHeader;
  uint16_t      transportHeaderLength;

  // A pointer to the first byte of the UDP/ICMP/TCP payload and the
  // number of bytes to be found there.
  uint8_t*      transportPayload;
  uint16_t      transportPayloadLength;

  // The UDP header may be compressed, which makes it convenient
  // to include the UDP ports here.  Eventually these could be moved
  // to a separate struct and we would just have a pointer here.
  uint16_t      sourcePort;
  uint16_t      destinationPort;

  // Ditto for ICMP
  uint8_t       icmpType;
  uint8_t       icmpCode;
} Ipv6Header;

/**
 * @brief Definitions for ICMP message types.
 */
typedef enum {
  // error messages
  ICMP_DESTINATION_UNREACHABLE   = 1,
  ICMP_PACKET_TOO_BIG            = 2,
  ICMP_TIME_EXCEEDED             = 3,
  ICMP_PARAMETER_PROBLEM         = 4,

  // informational messages
  ICMP_PRIVATE_EXPERIMENTATION_0 = 100,
  ICMP_ECHO_REQUEST              = 128,
  ICMP_ECHO_REPLY                = 129,
  ICMP_ROUTER_SOLICITATION       = 133,
  ICMP_ROUTER_ADVERTISEMENT      = 134,
  ICMP_NEIGHBOR_SOLICITATION     = 135,
  ICMP_NEIGHBOR_ADVERTISEMENT    = 136,
  ICMP_RPL                       = 155,
  ICMP_DUPLICATE_ADDRESS_REQUEST = 157,
  ICMP_DUPLICATE_ADDRESS_CONFIRM = 158,
} EmberIcmpType;

/**
 * @brief Definitions for ICMP message codes.
 */
typedef enum {
  ICMP_CODE_NO_ROUTE_TO_DESTINATION        = 0,
  ICMP_CODE_ERROR_IN_SOURCE_ROUTING_HEADER = 7,
} EmberIcmpCode;

/**
 * @brief Structure to hold an IPv6 "Next Header"
 * See http://www.iana.org/assignments/protocol-numbers
 */
typedef enum {
  // Protocols
  IPV6_NEXT_HEADER_ICMP        =  1,
  IPV6_NEXT_HEADER_TCP         =  6,
  IPV6_NEXT_HEADER_UDP         = 17,
  IPV6_NEXT_HEADER_IPV6        = 41,
  IPV6_NEXT_HEADER_ICMPV6      = 58,
  IPV6_NEXT_HEADER_NO_NEXT     = 59,
  IPV6_NEXT_HEADER_MOBILITY    = 137,

  // Extension Headers
  IPV6_NEXT_HEADER_HOP_BY_HOP  =  0,
  IPV6_NEXT_HEADER_DESTINATION = 60,
  IPV6_NEXT_HEADER_ROUTING     = 43,
  IPV6_NEXT_HEADER_FRAGMENT    = 44,

  // Used internally by Ember
  IPV6_NEXT_HEADER_UNKNOWN     = 0xFF
} EmberIpv6NextHeader;

#define TLS_SESSION_ID_SIZE     32
#define TLS_MASTER_SECRET_SIZE  48

/**
 * @brief Defines a TLS session state.
 */
typedef struct {
  uint8_t idLength;
  uint16_t id[(TLS_SESSION_ID_SIZE + 1) / 2];
  uint8_t master[TLS_MASTER_SECRET_SIZE];
} TlsSessionState;

/**
 * @brief Defines a data type of size 8 bytes.
 */
typedef struct {
  uint8_t contents[8];
} Bytes8;

/**
 * @brief Defines a data type of size 16 bytes.
 */
typedef struct {
  uint8_t contents[16];
} Bytes16;

/**
 * @brief Defines a certificate authority structure.
 */
typedef struct {
  const uint8_t *name;
  uint16_t nameLength;
  uint8_t *publicKey;
  uint8_t maxPathLength;
} CertificateAuthority;

/**
 * @brief Defines a device certificate structure.
 */
typedef struct {
  const uint8_t *privateKey;
  const uint8_t *certificate;
  const uint16_t certificateSize;
} DeviceCertificate;

//@} \\END Misc. Ember Types

/**
 * @name Broadcast Addresses
 *@{
 *  Broadcasts are normally sent only to routers.  Broadcasts can also be
 *  forwarded to end devices, either all of them or only those that do not
 *  sleep.  Broadcasting to end devices is both significantly more
 *  resource-intensive and significantly less reliable than broadcasting to
 *  routers.
 */

/** Broadcast to all routers. */
#define EMBER_BROADCAST_ADDRESS 0xFFFC
/** Broadcast to all non-sleepy devices. */
#define EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS 0xFFFD
/** Broadcast to all devices, including sleepy end devices. */
#define EMBER_SLEEPY_BROADCAST_ADDRESS 0xFFFF

/** @} END Broadcast Addresses */

/**
 * @addtogroup device_types
 * @{
 * @brief Defines the possible types of nodes and the roles that a
 * node might play in a network.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberNodeType
#else
typedef uint8_t EmberNodeType;
enum
#endif
{
  /** Device is not joined. */
  EMBER_UNKNOWN_DEVICE = 0,
  /** Will relay messages and can act as a parent to other nodes. */
  EMBER_ROUTER = 2,
  /** Communicates only with its parent and will not relay messages. */
  EMBER_END_DEVICE = 3,
  /** An end device whose radio can be turned off to save power.
   *  The application must call ::emberPollForData() to receive messages. */
  EMBER_SLEEPY_END_DEVICE = 4,
  /** An always-on end device like ::EMBER_END_DEVICE, but IP address discovery
   * is performed by the parent on its behalf to help it conserve resources. */
  EMBER_MINIMAL_END_DEVICE = 5,
  /** Authentication server for new Thread devices and the authorizer
   *  for providing the network credentials they require joining the
   *  network. */
  EMBER_COMMISSIONER = 7
};
/**
 * @}
 */

/**
 * @brief Defines the possible join states for a node.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberNetworkStatus
#else
typedef uint8_t EmberNetworkStatus;
enum
#endif
{
  /** The node is not associated with a network in any way. */
  EMBER_NO_NETWORK,
  /** The node was part of a network prior to reset. */
  EMBER_SAVED_NETWORK,
  /** The node is currently attempting to join a network. */
  EMBER_JOINING_NETWORK,
  /** The node is joined and attached to a network. */
  EMBER_JOINED_NETWORK_ATTACHED,
  /** The node is joined but without a parent. */
  EMBER_JOINED_NETWORK_NO_PARENT,
  /** The node is joined but is currently attaching. */
  EMBER_JOINED_NETWORK_ATTACHING
};

/**
 * @brief Defines the reason why a network status change occurred.
 *
 * This information is passed up to the application via the
 * ::emberNetworkStatusHandler callback.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberJoinFailureReason
#else
typedef uint8_t EmberJoinFailureReason;
enum
#endif
{
  /** No failure.  This indicates that the network status change occurred as
   * part of regular network operation.
   */
  EMBER_JOIN_FAILURE_REASON_NONE,
  /** The operation ::emberFormNetwork failed while performing an energy scan
   * on a channel.
   */
  EMBER_JOIN_FAILURE_REASON_FORM_SCAN,
  /** ::emberJoinNetwork or ::emberJoinCommissioned failed while performing an
   * active scan on a channel.  This indicates that discovery failed due to
   * no network matching the network parameters being filtered on.
   */
  EMBER_JOIN_FAILURE_REASON_ACTIVE_SCAN,
  /** ::emberJoinNetwork failed during commissioning.  This usually indicates
   * that either the commissioning step timed out or a mismatch occurred with
   * one of the parameters passed into ::emberJoinNetwork.
   */
  EMBER_JOIN_FAILURE_REASON_COMMISSIONING,
  /** ::emberJoinNetwork failed during the DTLS handshake to establish a shared
   * key.  This usually indicates that either the join key (EMBER_JOIN_KEY_OPTION)
   * passed in this call is incorrect or some other fatal error occurred, such as
   * a timeout.
   */
  EMBER_JOIN_FAILURE_REASON_SECURITY,
};

/**
 * @brief Type for a network scan.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberNetworkScanType
#else
typedef uint8_t EmberNetworkScanType;
enum
#endif
{
  /** An energy scan for each channel looks for its RSSI value. */
  EMBER_ENERGY_SCAN,
  /** An active scan for each channel looks for available networks. */
  EMBER_ACTIVE_SCAN
};

/** @brief Default scan duration for an energy or active scan.
 *
 *  The value is the exponent of the number of scan periods, where a scan period
 *  is 960 symbols and a symbol is 16 microseconds.  The scan lasts for
 *  ((2^duration) + 1) scan periods.  The value of this duration must be less
 *  than 15.
 *  The time corresponding to the first few values is as follows:
 *  0 = 31 msec, 1 = 46 msec, 2 = 77 msec, 3 = 138 msec, 4 = 261 msec,
 *  5 = 507 msec, 6 = 998 msec.
 */
#define DEFAULT_SCAN_DURATION 5

/**
 * @brief Either marks an event as inactive or specifies the units for the
 * event execution time.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberEventUnits
#else
typedef uint8_t EmberEventUnits;
enum
#endif
{
  /** The event is not scheduled to run. */
  EMBER_EVENT_INACTIVE = 0,
  /** The execution time is in approximate milliseconds.  */
  EMBER_EVENT_MS_TIME,
  /** The execution time is in 'binary' quarter seconds (256 approximate
      milliseconds each). */
  EMBER_EVENT_QS_TIME,
  /** The execution time is in 'binary' minutes (65536 approximate milliseconds
      each). */
  EMBER_EVENT_MINUTE_TIME,
  /** The event is scheduled to run at the earliest opportunity. */
  EMBER_EVENT_ZERO_DELAY
};

/**
 * @brief Defines the events reported to the application
 * by the ::emberCounterHandler().
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberCounterType
#else
typedef uint8_t EmberCounterType;
enum
#endif
{
  /** Every packet that comes in over the radio (except MAC acks). */
  EMBER_COUNTER_PHY_IN_PACKETS,

  /** Every packet that goes out over the radio (except MAC acks). */
  EMBER_COUNTER_PHY_OUT_PACKETS,

  /** Every incoming byte, including the 802.15.4 length byte.
   * Note MAC acks are not counted. */
  EMBER_COUNTER_PHY_IN_OCTETS,

  /** Every outgoing byte, including the 802.15.4 length byte.
   * MAC retries contribute to the count, but not MAC acks. */
  EMBER_COUNTER_PHY_OUT_OCTETS,

  /** Incoming MAC unicasts post duplicate detection. */
  EMBER_COUNTER_MAC_IN_UNICAST,

  EMBER_COUNTER_MAC_IN_BROADCAST,

  /** Outgoing MAC unicasts for which an ack was received possibly
   * after retrying. */
  EMBER_COUNTER_MAC_OUT_UNICAST_SUCCESS,

  /** Outgoing unicasts for which ack was never received even
   * after retrying. */
  EMBER_COUNTER_MAC_OUT_UNICAST_ACK_FAIL,

  /** Outgoing MAC packets which were never transmitted because
   * clear channel assessment always returned busy. */
  EMBER_COUNTER_MAC_OUT_UNICAST_CCA_FAIL,

  /** Outgoing unicasts that failed even after extended MAC retries. */
  EMBER_COUNTER_MAC_OUT_UNICAST_EXT_FAIL,

  /** Outgoing unicast retries.  This does not count the initial
   * transmission.  Note a single MAC transmission can result in
   * multiple retries. */
  EMBER_COUNTER_MAC_OUT_UNICAST_RETRY,

  EMBER_COUNTER_MAC_OUT_BROADCAST,

  EMBER_COUNTER_MAC_OUT_BROADCAST_CCA_FAIL,

  /** Dropped incoming MAC packets (out of memory). */
  EMBER_COUNTER_MAC_DROP_IN_MEMORY,

  /** Dropped incoming MAC packets (no EUI). */
  EMBER_COUNTER_MAC_DROP_IN_NO_EUI,

  /** Dropped incoming MAC packets (invalid frame counter). */
  EMBER_COUNTER_MAC_DROP_IN_FRAME_COUNTER,

  /** Dropped incoming MAC packets (can't decrypt). */
  EMBER_COUNTER_MAC_DROP_IN_DECRYPT,

  /** Dropped incoming MAC packets (duplicate message). */
  EMBER_COUNTER_MAC_DROP_IN_DUPLICATE,

  /** IP packets */
  EMBER_COUNTER_IP_IN_UNICAST,
  EMBER_COUNTER_IP_OUT_UNICAST,
  EMBER_COUNTER_IP_IN_MULTICAST,
  EMBER_COUNTER_IP_OUT_MULTICAST,

  /** Application UDP messages.  Excludes DNS, PANA, and MLE. */
  EMBER_COUNTER_UDP_IN,
  EMBER_COUNTER_UDP_OUT,

  /** UART in and out data */
  EMBER_COUNTER_UART_IN_DATA,
  EMBER_COUNTER_UART_IN_MANAGEMENT,
  EMBER_COUNTER_UART_IN_FAIL,
  EMBER_COUNTER_UART_OUT_DATA,
  EMBER_COUNTER_UART_OUT_MANAGEMENT,
  EMBER_COUNTER_UART_OUT_FAIL,

  // Counters for non-packet events below.
  EMBER_COUNTER_ROUTE_2_HOP_LOOP,
  EMBER_COUNTER_BUFFER_ALLOCATION_FAIL,

  /** ASHv3 */
  EMBER_ASH_V3_ACK_SENT,
  EMBER_ASH_V3_ACK_RECEIVED,
  EMBER_ASH_V3_NACK_SENT,
  EMBER_ASH_V3_NACK_RECEIVED,
  EMBER_ASH_V3_RESEND,
  EMBER_ASH_V3_BYTES_SENT,
  EMBER_ASH_V3_TOTAL_BYTES_RECEIVED,
  EMBER_ASH_V3_VALID_BYTES_RECEIVED,
  EMBER_ASH_V3_PAYLOAD_BYTES_SENT,

  /** The number of times a low-priority packet traffic arbitration
      request has been made.
   */
  EMBER_COUNTER_PTA_LO_PRI_REQUESTED,

  /** The number of times a high-priority packet traffic arbitration
      request has been made.
   */
  EMBER_COUNTER_PTA_HI_PRI_REQUESTED,

  /** The number of times a low-priority packet traffic arbitration
      request has been denied.
   */
  EMBER_COUNTER_PTA_LO_PRI_DENIED,

  /** The number of times a high-priority packet traffic arbitration
      request has been denied.
   */
  EMBER_COUNTER_PTA_HI_PRI_DENIED,

  /** The number of times a low-priority packet traffic arbitration
      transmission has been aborted.
   */
  EMBER_COUNTER_PTA_LO_PRI_TX_ABORTED,

  /** The number of times a high-priority packet traffic arbitration
      transmission has been aborted.
   */
  EMBER_COUNTER_PTA_HI_PRI_TX_ABORTED,

  /** A placeholder giving the number of Ember counter types. */
  EMBER_COUNTER_TYPE_COUNT,

  //** Special type indicating all counters */
  EMBER_COUNTER_ALL = 0xFF
};

/**
 * @brief Defines the CLI enumerations for the ::EmberCounterType enum.
 */
#define EMBER_COUNTER_STRINGS   \
  "PHY in",                     \
  "PHY out",                    \
  "PHY in bytes",               \
  "PHY out bytes",              \
  "MAC in uni",                 \
  "MAC in bro",                 \
  "MAC out uni succ",           \
  "MAC out uni ack fail",       \
  "MAC out uni cca fail",       \
  "MAC out uni ext fail",       \
  "MAC out uni retry",          \
  "MAC out bro",                \
  "MAC out bro cca fail",       \
  "MAC drop in memory",         \
  "MAC drop in no eui",         \
  "MAC drop in frame counter",  \
  "MAC drop in decrypt",        \
  "MAC drop in dup",            \
  "IP in uni",                  \
  "IP out uni",                 \
  "IP in multi",                \
  "IP out multi",               \
  "UDP in",                     \
  "UDP out",                    \
  "UART in data",               \
  "UART in mgmt",               \
  "UART in fail",               \
  "UART out data",              \
  "UART out mgmt",              \
  "UART out fail",              \
  "ROUTE 2-hop loop",           \
  "BUFFER alloc fail",          \
  "ASHv3 ack sent",             \
  "ASHv3 ack received",         \
  "ASHv3 nack sent",            \
  "ASHv3 nack received",        \
  "ASHv3 resend",               \
  "ASHv3 bytes sent",           \
  "ASHv3 total bytes received", \
  "ASHv3 valid bytes received", \
  "ASHv3 payload bytes sent",   \
  "PTA lo pri requested",       \
  "PTA hi pri requested",       \
  "PTA lo pri denied",          \
  "PTA hi pri denied",          \
  "PTA lo pri tx aborted",      \
  "PTA hi pri tx aborted"

/** brief An identifier for a task */
typedef uint8_t EmberTaskId;

//----------------------------------------------------------------
// Events and event queues.

// Forward declarations to make up for C's one-pass type checking.
struct Event_s;
struct EventQueue_s;

/** @brief The static part of an event.  Each event can be used with only one
 * event queue.
 */

typedef const struct EventActions_s {
  struct EventQueue_s *queue;           // the queue this event goes on
  void (*handler)(struct Event_s *);    // called when the event fires
  void (*marker)(struct Event_s *);     // marking fuction, can be NULL
  const char *name;                     // event name for debugging purposes
} EventActions;

typedef struct Event_s {
  EventActions *actions;                // static data

  // For internal use only, but the 'next' field must be initialized
  // to NULL.
  struct Event_s *next;
  uint32_t timeToExecute;
} Event;

/** @brief An event queue is currently a list of events ordered by
 * execution time.
 */
typedef struct EventQueue_s {
  Event *isrEvents;     // Events to be run with no delay, protected by ATOMIC.
  Event *events;        // Events scheduled to be run.
  uint32_t runTime;     // These two fields are used to avoid running one
  bool running;         // event multiple times at a single moment in time.
} EventQueue;

/** @brief Control structure for events.
 *
 * This structure should not be accessed directly.
 * It holds the event status (one of the @e EMBER_EVENT_ values)
 * and the time left before the event fires.
 */
typedef struct {
  /** The event's status, either inactive or the units for timeToExecute. */
  EmberEventUnits status;
  /** The ID of the task this event belongs to. */
  EmberTaskId taskid;
  /** How long before the event fires.
   *  Units are always in milliseconds.
   */
  uint32_t timeToExecute;
} EmberEventControl;

/** @brief Complete events with a control and a handler procedure.
 *
 * An application typically creates an array of events
 * along with their handlers.
 * The main loop passes the array to ::emberRunEvents() to call
 * the handlers of any events whose time has arrived.
 */
typedef PGM struct {
  /** The control structure for the event. */
  EmberEventControl *control;
  /** The procedure to call when the event fires. */
  void (*handler)(void);
} EmberEventData;

/** @brief Control structure for tasks.
 *
 * This structure should not be accessed directly.
 */
typedef struct {
  // The time when the next event associated with this task will fire
  uint32_t nextEventTime;
  // The list of events associated with this task
  EmberEventData *events;
  // A flag that indicates the task has something to do other than events
  bool busy;
} EmberTaskControl;

/**
 * @name txPowerModes for emberSetTxPowerMode and mfglibSetPower
 */
//@{

/** @brief The application should call ::emberSetTxPowerMode() with the
 * txPowerMode parameter set to this value to disable all power mode options
 * resulting in normal power mode and bi-directional RF transmitter output.
 */
#define EMBER_TX_POWER_MODE_DEFAULT             0x0000
/** @brief The application should call ::emberSetTxPowerMode() with the
 * txPowerMode parameter set to this value to enable boost power mode.
 */
#define EMBER_TX_POWER_MODE_BOOST               0x0001
/** @brief The application should call ::emberSetTxPowerMode() with the
 * txPowerMode parameter set to this value to enable the alternate transmitter
 * output.
 */
#define EMBER_TX_POWER_MODE_ALTERNATE           0x0002
/** @brief The application should call ::emberSetTxPowerMode() with the
 * txPowerMode parameter set to this value to enable both boost mode and the
 * alternate transmitter output.
 */
#define EMBER_TX_POWER_MODE_BOOST_AND_ALTERNATE (EMBER_TX_POWER_MODE_BOOST \
                                                 | EMBER_TX_POWER_MODE_ALTERNATE)
#ifndef DOXYGEN_SHOULD_SKIP_THIS
// The application does not ever need to call emberSetTxPowerMode() with the
// txPowerMode parameter set to this value.  This value is used internally by
// the stack to indicate that the default token configuration has not been
// overridden by a prior call to emberSetTxPowerMode().
#define EMBER_TX_POWER_MODE_USE_TOKEN           0x8000
#endif//DOXYGEN_SHOULD_SKIP_THIS

#endif // EMBER_TYPES_H

/** @} // END addtogroup
 */
