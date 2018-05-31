// (c) Ember, 2008.
//
// This file is generated! Please do not modify manually.
// If you wish to change, check apitrace_header.py.

#ifndef __APITRACE_GEN__
#define __APITRACE_GEN__

enum {
  EM_DEBUG_SET_BINDING = 0x00,   // Set binding function call.
  EM_DEBUG_DELETE_BINDING = 0x01,   // Delete binding function call.
  EM_DEBUG_CLEAR_BINDING_TABLE = 0x02,   // Clear binding table function call.
  EM_DEBUG_SEND_LIMITED_MULTICAST = 0x06,   // Send limited multicast function call.
  EM_DEBUG_SEND_UNICAST = 0x08,   // Send unicast function call.
  EM_DEBUG_SEND_BROADCAST = 0x09,   // Send broadcast function call.
  EM_DEBUG_CANCEL_MESSAGE = 0x0B,   // Cancel message function call.
  EM_DEBUG_SEND_REPLY = 0x0C,   // Send reply function call.
  EM_DEBUG_SET_REPLY_BINDING = 0x0D,   // Set reply binding function call.
  EM_DEBUG_MESSAGE_SENT = 0x0E,   // Message sent function call.
  EM_DEBUG_INCOMING_MESSAGE_HANDLER = 0x11,   // Incoming message handler function call.
  EM_DEBUG_STACK_STATUS_HANDLER = 0x13,   // Stack status handler function call.
  EM_DEBUG_NETWORK_INIT = 0x14,   // Network init function call.
  EM_DEBUG_FORM_NETWORK = 0x15,   // Form network function call.
  EM_DEBUG_JOIN_NETWORK = 0x16,   // Join network function call.
  EM_DEBUG_LEAVE_NETWORK = 0x17,   // Leave network function call.
  EM_DEBUG_PERMIT_JOINING = 0x18,   // Permit joining function call.
  EM_DEBUG_POLL_FOR_DATA = 0x19,   // Poll for data function call.
  EM_DEBUG_POLL_HANDLER = 0x1A,   // Poll handler function call.
  EM_DEBUG_TRUST_CENTER_JOIN_HANDLER = 0x1E,   // Trust center join handler function call.
  EM_DEBUG_SET_MESSAGE_FLAG = 0x20,   // Set message flag function call.
  EM_DEBUG_CLEAR_MESSAGE_FLAG = 0x21,   // Clear message flag function call.
  EM_DEBUG_POLL_COMPLETE_HANDLER = 0x22,   // Poll complete handler function call.
  EM_DEBUG_CHILD_JOIN_HANDLER = 0x23,   // Child join handler function call.
  EM_DEBUG_START_SCAN = 0x24,   // Start scan function call.
  EM_DEBUG_STOP_SCAN = 0x25,   // Stop scan function call.
  EM_DEBUG_SCAN_COMPLETE_HANDLER = 0x26,   // Scan complete handler function call.
  EM_DEBUG_NETWORK_FOUND_HANDLER = 0x27,   // Network found handler function call.
  EM_DEBUG_ENERGY_SCAN_RESULT_HANDLER = 0x28,   // Energy scan result handler function call.
  EM_DEBUG_SET_INITIAL_SECURITY_STATE = 0x2B,   // Set initial security state function call.
  EM_DEBUG_REJOIN_NETWORK = 0x2C,   // Rejoin network function call.
  EM_DEBUG_STACK_POWER_DOWN = 0x2D,   // Stack power down function call.
  EM_DEBUG_STACK_POWER_UP = 0x2E,   // Stack power up function call.
  EM_DEBUG_SET_EXTENDED_SECURITY_BITMASK = 0x2F,   // Set extended security bitmask function call.
};

#define API_TRACE_SET_BINDING(index)                 \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,   \
                               "BB",                 \
                               EM_DEBUG_SET_BINDING, \
                               (index)               \
                               ))
#define API_TRACE_DELETE_BINDING(index)                 \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,      \
                               "BB",                    \
                               EM_DEBUG_DELETE_BINDING, \
                               (index)                  \
                               ))
#define API_TRACE_CLEAR_BINDING_TABLE( )                    \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,          \
                               "B",                         \
                               EM_DEBUG_CLEAR_BINDING_TABLE \
                               ))
#define API_TRACE_SEND_LIMITED_MULTICAST(groupId, profileId, clusterId, sourceEndpoint, destinationEndpoint, options, radius, nonmemberRadius) \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,                                                                                             \
                               "BWWBBBWBB",                                                                                                    \
                               EM_DEBUG_SEND_LIMITED_MULTICAST,                                                                                \
                               (groupId),                                                                                                      \
                               (profileId),                                                                                                    \
                               (clusterId),                                                                                                    \
                               (sourceEndpoint),                                                                                               \
                               (destinationEndpoint),                                                                                          \
                               (options),                                                                                                      \
                               (radius),                                                                                                       \
                               (nonmemberRadius)                                                                                               \
                               ))
#define API_TRACE_SEND_UNICAST(indexOrDestination, profileId, clusterId, sourceEndpoint, destinationEndpoint, options) \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,                                                                     \
                               "BWWWBBW",                                                                              \
                               EM_DEBUG_SEND_UNICAST,                                                                  \
                               (indexOrDestination),                                                                   \
                               (profileId),                                                                            \
                               (clusterId),                                                                            \
                               (sourceEndpoint),                                                                       \
                               (destinationEndpoint),                                                                  \
                               (options)                                                                               \
                               ))
#define API_TRACE_SEND_BROADCAST(destination, profileId, clusterId, sourceEndpoint, destinationEndpoint, options, radius) \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,                                                                        \
                               "BWWWBBWB",                                                                                \
                               EM_DEBUG_SEND_BROADCAST,                                                                   \
                               (destination),                                                                             \
                               (profileId),                                                                               \
                               (clusterId),                                                                               \
                               (sourceEndpoint),                                                                          \
                               (destinationEndpoint),                                                                     \
                               (options),                                                                                 \
                               (radius)                                                                                   \
                               ))
#define API_TRACE_CANCEL_MESSAGE(message)               \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,      \
                               "BB",                    \
                               EM_DEBUG_CANCEL_MESSAGE, \
                               (message)                \
                               ))
#define API_TRACE_SEND_REPLY(clusterId, reply)      \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,  \
                               "BWB",               \
                               EM_DEBUG_SEND_REPLY, \
                               (clusterId),         \
                               (reply)              \
                               ))
#define API_TRACE_SET_REPLY_BINDING(index)                 \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,         \
                               "BB",                       \
                               EM_DEBUG_SET_REPLY_BINDING, \
                               (index)                     \
                               ))
#define API_TRACE_MESSAGE_SENT(mode, destination, profileId, clusterId, sourceEndpoint, destinationEndpoint, options, status) \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,                                                                            \
                               "BBWWWBBWB",                                                                                   \
                               EM_DEBUG_MESSAGE_SENT,                                                                         \
                               (mode),                                                                                        \
                               (destination),                                                                                 \
                               (profileId),                                                                                   \
                               (clusterId),                                                                                   \
                               (sourceEndpoint),                                                                              \
                               (destinationEndpoint),                                                                         \
                               (options),                                                                                     \
                               (status)                                                                                       \
                               ))
#define API_TRACE_INCOMING_MESSAGE_HANDLER(type, profileId, clusterId, sourceEndpoint, destinationEndpoint, options) \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,                                                                   \
                               "BBWWBBW",                                                                            \
                               EM_DEBUG_INCOMING_MESSAGE_HANDLER,                                                    \
                               (type),                                                                               \
                               (profileId),                                                                          \
                               (clusterId),                                                                          \
                               (sourceEndpoint),                                                                     \
                               (destinationEndpoint),                                                                \
                               (options)                                                                             \
                               ))
#define API_TRACE_STACK_STATUS_HANDLER(stackStatus)           \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,            \
                               "BB",                          \
                               EM_DEBUG_STACK_STATUS_HANDLER, \
                               (stackStatus)                  \
                               ))
#define API_TRACE_NETWORK_INIT(nodeType)              \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,    \
                               "BB",                  \
                               EM_DEBUG_NETWORK_INIT, \
                               (nodeType)             \
                               ))
#define API_TRACE_FORM_NETWORK(extendedPanId, panId, radioTxPower, radioChannel) \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,                               \
                               "B8pWBB",                                         \
                               EM_DEBUG_FORM_NETWORK,                            \
                               (extendedPanId),                                  \
                               (panId),                                          \
                               (radioTxPower),                                   \
                               (radioChannel)                                    \
                               ))
#define API_TRACE_JOIN_NETWORK(nodeType, extendedPanId, panId, radioTxPower, radioChannel, joinMethod, nwkManagerId, nwkUpdateId, channels) \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,                                                                                          \
                               "BB8pWBBBWBD",                                                                                               \
                               EM_DEBUG_JOIN_NETWORK,                                                                                       \
                               (nodeType),                                                                                                  \
                               (extendedPanId),                                                                                             \
                               (panId),                                                                                                     \
                               (radioTxPower),                                                                                              \
                               (radioChannel),                                                                                              \
                               (joinMethod),                                                                                                \
                               (nwkManagerId),                                                                                              \
                               (nwkUpdateId),                                                                                               \
                               (channels)                                                                                                   \
                               ))
#define API_TRACE_LEAVE_NETWORK( )                    \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,    \
                               "B",                   \
                               EM_DEBUG_LEAVE_NETWORK \
                               ))
#define API_TRACE_PERMIT_JOINING(duration)              \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,      \
                               "BB",                    \
                               EM_DEBUG_PERMIT_JOINING, \
                               (duration)               \
                               ))
#define API_TRACE_POLL_FOR_DATA( )                    \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,    \
                               "B",                   \
                               EM_DEBUG_POLL_FOR_DATA \
                               ))
#define API_TRACE_POLL_HANDLER(id, sendAppJitMessage) \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,    \
                               "BWB",                 \
                               EM_DEBUG_POLL_HANDLER, \
                               (id),                  \
                               (sendAppJitMessage)    \
                               ))
#define API_TRACE_TRUST_CENTER_JOIN_HANDLER(status, decision)      \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,                 \
                               "BBB",                              \
                               EM_DEBUG_TRUST_CENTER_JOIN_HANDLER, \
                               (status),                           \
                               (decision)                          \
                               ))
#define API_TRACE_SET_MESSAGE_FLAG(childId)               \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,        \
                               "BW",                      \
                               EM_DEBUG_SET_MESSAGE_FLAG, \
                               (childId)                  \
                               ))
#define API_TRACE_CLEAR_MESSAGE_FLAG(childId)               \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,          \
                               "BW",                        \
                               EM_DEBUG_CLEAR_MESSAGE_FLAG, \
                               (childId)                    \
                               ))
#define API_TRACE_POLL_COMPLETE_HANDLER(status)                \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,             \
                               "BB",                           \
                               EM_DEBUG_POLL_COMPLETE_HANDLER, \
                               (status)                        \
                               ))
#define API_TRACE_CHILD_JOIN_HANDLER(childIndex, joining)   \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,          \
                               "BBB",                       \
                               EM_DEBUG_CHILD_JOIN_HANDLER, \
                               (childIndex),                \
                               (joining)                    \
                               ))
#define API_TRACE_START_SCAN(scanType, channelMask, duration) \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,            \
                               "BBDB",                        \
                               EM_DEBUG_START_SCAN,           \
                               (scanType),                    \
                               (channelMask),                 \
                               (duration)                     \
                               ))
#define API_TRACE_STOP_SCAN( )                     \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE, \
                               "B",                \
                               EM_DEBUG_STOP_SCAN  \
                               ))
#define API_TRACE_SCAN_COMPLETE_HANDLER(data, status)          \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,             \
                               "BBB",                          \
                               EM_DEBUG_SCAN_COMPLETE_HANDLER, \
                               (data),                         \
                               (status)                        \
                               ))
#define API_TRACE_NETWORK_FOUND_HANDLER(panId, permitJoin, stackProfile) \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,                       \
                               "BWBB",                                   \
                               EM_DEBUG_NETWORK_FOUND_HANDLER,           \
                               (panId),                                  \
                               (permitJoin),                             \
                               (stackProfile)                            \
                               ))
#define API_TRACE_ENERGY_SCAN_RESULT_HANDLER(channel, rssi)         \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,                  \
                               "BBB",                               \
                               EM_DEBUG_ENERGY_SCAN_RESULT_HANDLER, \
                               (channel),                           \
                               (rssi)                               \
                               ))
#define API_TRACE_SET_INITIAL_SECURITY_STATE(mask, preconfiguredKey, networkKey, keySequence) \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,                                            \
                               "BW16p16pB",                                                   \
                               EM_DEBUG_SET_INITIAL_SECURITY_STATE,                           \
                               (mask),                                                        \
                               (preconfiguredKey),                                            \
                               (networkKey),                                                  \
                               (keySequence)                                                  \
                               ))
#define API_TRACE_REJOIN_NETWORK(haveKey, channelMask, status) \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,             \
                               "BBDB",                         \
                               EM_DEBUG_REJOIN_NETWORK,        \
                               (haveKey),                      \
                               (channelMask),                  \
                               (status)                        \
                               ))
#define API_TRACE_STACK_POWER_DOWN( )                    \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,       \
                               "B",                      \
                               EM_DEBUG_STACK_POWER_DOWN \
                               ))
#define API_TRACE_STACK_POWER_UP( )                    \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,     \
                               "B",                    \
                               EM_DEBUG_STACK_POWER_UP \
                               ))
#define API_TRACE_SET_EXTENDED_SECURITY_BITMASK(mask)                  \
  IF_DEBUG(emDebugBinaryFormat(EM_DEBUG_API_TRACE,                     \
                               "BW",                                   \
                               EM_DEBUG_SET_EXTENDED_SECURITY_BITMASK, \
                               (mask)                                  \
                               ))

#endif // __APITRACE_GEN__
