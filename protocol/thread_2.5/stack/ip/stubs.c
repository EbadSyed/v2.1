// File: stub-file.c
// Description: The global stub file.
//
// To use:
//
// #define the stub files you want to use in your application, then #include
//  this file.
//
// Example to use the emFoo() and emBar() stubs in your application:
//
// #define USE_STUB_emFoo
// #define USE_STUB_emBar
// #include "stack/ip/stubs.c"
//
// Copyright 2014 by Silicon Laboratories. All rights reserved.             *80*

#ifdef USE_STUB_emGleanShortIdFromInterfaceId
bool emGleanShortIdFromInterfaceId(uint8_t *interfaceId, uint16_t* shortId)
{
  return false;
}
#endif

#ifdef USE_STUB_emIsActiveKeySequence
bool emIsActiveKeySequence(uint32_t keySequence)
{
  return false;
}
#endif

#ifdef USE_STUB_FrameCounterStatus
typedef enum {
  FRAME_COUNTER_NOT_VALID       // not the actual value - its a stub
} FrameCounterStatus;
#endif

#ifdef USE_STUB_emSetKeySequenceDelta
bool emSetKeySequenceDelta(uint32_t keySequence,
                           int8_t *sequenceDeltaLoc)
{
  return false;
}
#endif

#ifdef USE_STUB_emNodeType
EmberNodeType emNodeType = EMBER_ROUTER;
#endif

#ifdef USE_STUB_halCommonGetInt32uMillisecondTick
uint32_t halCommonGetInt32uMillisecondTick(void)
{
  return 0;
}
#endif

#ifdef USE_STUB_halGetWakeInfo
uint32_t halGetWakeInfo(void)
{
  return 0;
}
#endif

#ifdef USE_STUB_emUseParentLongId
bool emUseParentLongId = false;
#endif

#ifdef USE_STUB_emberGetNodeId
EmberNodeId emberGetNodeId(void)
{
  return 0;
}
#endif

#ifdef USE_STUB_emberStateReturn
void emberStateReturn(const EmberNetworkParameters *parameters,
                      const EmberEui64 *eui64,
                      const EmberEui64 *extendedMacId,
                      EmberNetworkStatus networkStatus)
{
}
#endif

#ifdef USE_STUB_emberGetNodeStatusReturn
void emberGetNodeStatusReturn(EmberStatus status,
                              uint8_t ripId,
                              EmberNodeId nodeId,
                              uint8_t parentRipId,
                              EmberNodeId parentId,
                              const uint8_t *networkFragmentIdentifier,
                              uint32_t networkFrameCounter)
{
}
#endif

#ifdef USE_STUB_emberMarkApplicationBuffersHandler
void emberMarkApplicationBuffersHandler(void)
{
}
#endif

#ifdef USE_STUB_emberFreeMemoryForPacketHandler
void emberFreeMemoryForPacketHandler(void *objectRef)
{
}
#endif

#ifdef USE_STUB_emLongIdToInterfaceId
void emLongIdToInterfaceId(const uint8_t *longId, uint8_t *interfaceId)
{
}
#endif

#ifdef USE_STUB_emInterfaceIdToLongId
void emInterfaceIdToLongId(const uint8_t *interfaceId, uint8_t *longId)
{
}
#endif

#ifdef USE_STUB_emReallyLogLine
void emReallyLogLine(uint8_t logType, PGM_P formatString, ...)
{
}
#endif

#ifdef USE_STUB_emReallyLogCodePoint
void emReallyLogCodePoint(uint8_t logType,
                          PGM_P file,
                          uint16_t line,
                          PGM_P formatString,
                          ...)
{
}
#endif

#ifdef USE_STUB_emLogConfigFromName
bool emLogConfigFromName(const char *typeName,
                         uint8_t typeNameLength,
                         bool on,
                         uint8_t port)
{
  return false;
}
#endif

#ifdef USE_STUB_emMacRequestPoll
void emMacRequestPoll(void)
{
}
#endif

#ifdef USE_STUB_emSetSleepyBroadcast
void emSetSleepyBroadcast(PacketHeader header)
{
}
#endif

#ifdef USE_STUB_emParentId
uint16_t emParentId;
#endif

#ifdef USE_STUB_emParentPriority
int8_t emParentPriority;
#endif

#ifdef USE_STUB_emberEventIsScheduled
bool emberEventIsScheduled(Event *event)
{
  return false;
}
#endif

#ifdef USE_STUB_ChildUpdateState
typedef enum {
  CHILD_KEEP_ALIVE,
  CHILD_UPDATE,
  PARENT_REBOOT
} ChildUpdateState;
#endif

#ifdef USE_STUB_emSendMleAnnounceOnChannel
void emSendMleAnnounceOnChannel(uint8_t channel, uint8_t count, uint16_t period)
{
}
#endif

#ifdef USE_STUB_emSendMleChildUpdate
bool emSendMleChildUpdate(const EmberIpv6Address *destination,
                          ChildUpdateState state)
{
  return false;
}
#endif

#ifdef USE_STUB_emSendMleDiscoveryRequest
bool emSendMleDiscoveryRequest(const EmberEui64 *extendedPanIds,
                               uint8_t count,
                               bool joinerFlag)
{
  return false;
}
#endif

#ifdef USE_STUB_emProcessIncomingDiscoveryResponse
void emProcessIncomingDiscoveryResponse(const uint8_t *mleTlv,
                                        const uint8_t *senderLongId,
                                        PacketHeader header)
{
}
#endif

#ifdef USE_STUB_emProcessIncomingJoinBeacon
void emProcessIncomingJoinBeacon(MacBeaconData *data)
{
}
#endif

#ifdef USE_STUB_emberEventSetInactive
void emberEventSetInactive(Event *event)
{
}
#endif

#ifdef USE_STUB_emberEventSetDelayMs
void emberEventSetDelayMs(Event *event, uint32_t delay)
{
}
#endif

#ifdef USE_STUB_emJoinInit
void emJoinInit(void)
{
}
#endif

#ifdef USE_STUB_emStartAttach
void emStartAttach(AttachReason reason)
{
}
#endif

#ifdef USE_STUB_emMaybeAttach
void emMaybeAttach(AttachReason reason)
{
}
#endif

#ifdef USE_STUB_emOrphanStartAttach
void emOrphanStartAttach(void)
{
}
#endif

#ifdef USE_STUB_emGetAttachReason
uint8_t emGetAttachReason(void)
{
  return 0;
}
#endif

#ifdef USE_STUB_emStackConfiguration
uint16_t emStackConfiguration = STACK_CONFIG_FULL_ROUTER;
#endif

#ifdef USE_STUB_emStackEventQueue
EventQueue emStackEventQueue;
#endif

#ifdef USE_STUB_emApiEventDelayUpdatedFromIsrHandler
void emApiEventDelayUpdatedFromIsrHandler(Event *event)
{
}
#endif

#ifdef USE_STUB_emGetPollTimeout
uint32_t emGetPollTimeout(void)
{
  return 0;
}
#endif

#ifdef USE_STUB_emSetLeaderEui64BeaconData
void emSetLeaderEui64BeaconData(const uint8_t *value)
{
}
#endif

#ifdef USE_STUB_emGetLeaderEui64BeaconPointer
uint8_t allZerosForEmGetLeaderEui64BeaconPointer[8] = { 0 };
uint8_t *emGetLeaderEui64BeaconPointer(void)
{
  return allZerosForEmGetLeaderEui64BeaconPointer;
}
#endif

#ifdef USE_STUB_emFindNeighborIndex
uint8_t emFindNeighborIndex(PacketHeader header)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emNoteLinkQuality
void emNoteLinkQuality(uint8_t neighborIndex)
{
}
#endif

#ifdef USE_STUB_emRipInit
void emRipInit(void)
{
}
#endif

#ifdef USE_STUB_emStartRip
void emStartRip(void)
{
}
#endif

#ifdef USE_STUB_emStartReedAdvertisements
void emStartReedAdvertisements(void)
{
}
#endif

#ifdef USE_STUB_emSetNetworkStatus
void emSetNetworkStatus(EmberNetworkStatus newStatus,
                        EmberJoinFailureReason reason)
{
}
#endif

#ifdef USE_STUB_emLinkIsEstablished
bool emLinkIsEstablished(uint8_t index)
{
  return false;
}
#endif

#ifdef USE_STUB_emBeaconPayloadBuffer
uint8_t emBeaconPayloadBuffer[100] = { 0 };
#endif

#ifdef USE_STUB_emGetNetworkKeySequence
bool emGetNetworkKeySequence(uint32_t *sequenceLoc)
{
  return false;
}
#endif

#ifdef USE_STUB_emGetActiveMacKeyAndSequence
const uint8_t *emGetActiveMacKeyAndSequence(uint32_t *sequenceNumberLoc)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emNoteMacKeyUse
void emNoteMacKeyUse(uint32_t keySequence)
{
}
#endif

#ifdef USE_STUB_emGetAuxFrameSize
uint8_t emGetAuxFrameSize(const uint8_t *auxFrame)
{
  return 0;
}
#endif

#ifdef USE_STUB_emJoiningNewFragment
bool emJoiningNewFragment = false;
#endif

#ifdef USE_STUB_emIncrementBorderRouterVersion
void emIncrementBorderRouterVersion(void)
{
}
#endif

#ifdef USE_STUB_emIdIsMyChild
bool emIdIsMyChild(EmberNodeId shortMacId)
{
  return false;
}
#endif

#ifdef USE_STUB_emLocalRipId
uint8_t emLocalRipId(void)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emLookupLongId
uint8_t *emLookupLongId(EmberNodeId shortId)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emIsAssignedInMask
bool emIsAssignedInMask(const uint8_t *mask, uint8_t ripId)
{
  return false;
}
#endif

#ifdef USE_STUB_emIsOnMeshPrefix
bool emIsOnMeshPrefix(const uint8_t *prefix, uint8_t prefixLengthInBits)
{
  return false;
}
#endif

#ifdef USE_STUB_emMarkAssignedRipIds
void emMarkAssignedRipIds(uint8_t sequenceNumber, const uint8_t *mask)
{
}
#endif

#ifdef USE_STUB_emLocalServerDataInit
void emLocalServerDataInit(void)
{
}
#endif

#ifdef USE_STUB_emHandleLocalServerDataPost
bool emHandleLocalServerDataPost(const uint8_t *uri,
                                 const uint8_t *payload,
                                 uint16_t payloadLength,
                                 const EmberCoapRequestInfo *info)
{
  return false;
}
#endif

#ifdef USE_STUB_emGlobalAddressTable
Buffer emGlobalAddressTable = NULL_BUFFER;
#endif

#ifdef USE_STUB_emAddGlobalAddress
GlobalAddressEntry *emAddGlobalAddress(const uint8_t *address,
                                       uint8_t prefixLengthInBits,
                                       uint32_t preferredLifetime,
                                       uint32_t validLifetime,
                                       const uint8_t *serverLongId,
                                       uint16_t localConfigurationFlags)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emDefaultPrefixInit
void emDefaultPrefixInit(void)
{
}
#endif

#ifdef USE_STUB_emGlobalAddressTableInit
void emGlobalAddressTableInit(void)
{
}
#endif

#ifdef USE_STUB_emberGetGlobalPrefixReturn
void emberGetGlobalPrefixReturn(uint8_t borderRouterFlags,
                                bool isStable,
                                const uint8_t *prefix,
                                uint8_t prefixLength,
                                uint8_t domainId,
                                uint32_t preferredLifetime,
                                uint32_t validLifetime)
{
}
#endif

#ifdef USE_STUB_emberGetGlobalAddressReturn
void emberGetGlobalAddressReturn(const EmberIpv6Address *address,
                                 uint32_t preferredLifetime,
                                 uint32_t validLifetime,
                                 uint8_t addressFlags)
{
}
#endif

#ifdef USE_STUB_emUpdateGlobalAddresses
void emUpdateGlobalAddresses(Buffer networkData, bool localServer)
{
}
#endif

#ifdef USE_STUB_emRipTable
RipEntry ripTable[64] = { { { 0 } } };
#endif

#ifdef USE_STUB_emRipTableCount
uint8_t emRipTableCount = 0;
#endif

#ifdef USE_STUB_emRipRouterCount
uint8_t emRipRouterCount(void)
{
  return 1;
}
#endif

#ifdef USE_STUB_emRouterSelectionJitterMs
uint32_t emRouterSelectionJitterMs = ROUTER_SELECTION_JITTER_MS;
#endif

#ifdef USE_STUB_emUnassignRipId
bool emUnassignRipId(const uint8_t *longId)
{
  return true;
}
#endif

#ifdef USE_STUB_emberExternalRouteChangeHandler
void emberExternalRouteChangeHandler(const uint8_t *prefix,
                                     uint8_t prefixLengthInBits,
                                     bool open)
{
}

#endif

#ifdef USE_STUB_emberDhcpServerChangeHandler
void emberDhcpServerChangeHandler(const uint8_t *prefix,
                                  uint8_t prefixLengthInBits,
                                  bool available)
{
}
#endif

#ifdef USE_STUB_emNetworkDataSize
uint16_t emNetworkDataSize(const uint8_t *finger)
{
  return 31337;
}
#endif

#ifdef USE_STUB_emNetworkDataTlvSize
uint16_t emNetworkDataTlvSize(const uint8_t *finger)
{
  return 31337;
}
#endif

#ifdef USE_STUB_emSetNetworkDataSize
void emSetNetworkDataSize(uint8_t *finger, uint16_t length)
{
}
#endif

#ifdef USE_STUB_emNetworkDataTlvOverhead
uint8_t emNetworkDataTlvOverhead(const uint8_t *finger)
{
  return 0;
}
#endif

#ifdef USE_STUB_emFindNextTlv
uint8_t *emFindNextTlv(Buffer buffer, uint8_t baseType, uint8_t *prefixTlv)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emFindInPrefixTlv
uint8_t *emFindInPrefixTlv(uint8_t *prefixTlv, uint8_t innerType)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emberNetworkDataChangeHandler
void emberNetworkDataChangeHandler(const uint8_t *networkData, uint16_t length)
{
}
#endif

#ifdef USE_STUB_emberSlaacServerChangeHandler
void emberSlaacServerChangeHandler(const uint8_t *prefix,
                                   uint8_t prefixLengthInBits,
                                   bool available)
{
}
#endif

#ifdef USE_STUB_emberAddressConfigurationChangeHandler
void emberAddressConfigurationChangeHandler(const EmberIpv6Address *address,
                                            uint32_t preferredLifetime,
                                            uint32_t validLifetime,
                                            uint8_t addressFlags)
{
}
#endif

#ifdef USE_STUB_emberRequestDhcpAddressReturn
void emberRequestDhcpAddressReturn(EmberStatus status,
                                   const uint8_t *prefix,
                                   uint8_t prefixLengthInBits)
{
}
#endif

#ifdef USE_STUB_emberRequestSlaacAddressReturn
void emberRequestSlaacAddressReturn(EmberStatus status,
                                    const uint8_t *prefix,
                                    uint8_t prefixLengthInBits)
{
}
#endif

#ifdef USE_STUB_emberResignGlobalAddressReturn
void emberResignGlobalAddressReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emSendMyNetworkData
void emSendMyNetworkData(EmberNodeId leader, Buffer myNetworkData)
{
}
#endif

#ifdef USE_STUB_emSendNetworkData
PacketHeader emSendNetworkData(const uint8_t *ipDest, uint8_t options)
{
  return NULL_BUFFER;
}
#endif

#ifdef USE_STUB_OperationalDatasetTlvs
typedef struct {} OperationalDatasetTlvs;
#endif

#ifdef USE_STUB_emGetActiveDataset
const CommissioningDataset *emGetActiveDataset(void)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emSetActiveDataset
void emSetActiveDataset(OperationalDatasetTlvs *new)
{
}
#endif

#ifdef USE_STUB_emClearDataset
void emClearDataset(bool isActive)
{
}
#endif

#ifdef USE_STUB_emSendNetworkDataRequest
bool emSendNetworkDataRequest(const uint8_t *ipDest,
                              bool includeIdMask,
                              uint16_t jitter)
{
  return false;
}
#endif

#ifdef USE_STUB_emModifyLeaderNetworkData
bool emModifyLeaderNetworkData(uint8_t *tlv,
                               uint8_t tlvLength,
                               EmberNodeId sender,
                               bool removeChildren)
{
  return false;
}
#endif

#ifdef USE_STUB_emAddDiscoveryResponseTlvs
uint8_t *emAddDiscoveryResponseTlvs(uint8_t *finger)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emProcessNetworkDataChangeRequest
uint16_t emProcessNetworkDataChangeRequest(const uint8_t *ipSource,
                                           uint8_t *data,
                                           uint16_t dataLength,
                                           bool removeChildren)
{
  return 0;
}
#endif

#ifdef USE_STUB_emGetLeaderNodeId
EmberNodeId emGetLeaderNodeId(void)
{
  return 0xFFFE;
}
#endif

#ifdef USE_STUB_emPreviousNodeId
EmberNodeId emPreviousNodeId;
#endif

#ifdef USE_STUB_emStoreMulticastSequence
void emStoreMulticastSequence(uint8_t *target)
{
}
#endif

#ifdef USE_STUB_emStackTask
EmberTaskId emStackTask = 0;
#endif

#ifdef USE_STUB_halButtonIsr
void halButtonIsr(uint8_t button, uint8_t state)
{
}
#endif

#ifdef USE_STUB_halCommonIdleForMilliseconds
EmberStatus halCommonIdleForMilliseconds(uint32_t *duration)
{
  return EMBER_SUCCESS;
}
#endif

#ifdef USE_STUB_halInternalGetTokenData
void halInternalGetTokenData(void *data, uint16_t addr, uint8_t index, uint8_t len)
{
}
#endif

#ifdef USE_STUB_halInternalSetTokenData
void halInternalSetTokenData(uint16_t token, uint8_t index, void *data, uint8_t len)
{
}
#endif

#ifdef USE_STUB_halStackSymbolDelayAIsr
void halStackSymbolDelayAIsr(void)
{
}
#endif

#ifdef USE_STUB_emSetTransportChecksum
void emSetTransportChecksum(PacketHeader header, Ipv6Header *ipHeader)
{
}
#endif

#ifdef USE_STUB_emUsePresharedTlsSessionState
bool emUsePresharedTlsSessionState = false;
#endif

#ifdef USE_STUB_emMleLurkerAcceptHandler
void emMleLurkerAcceptHandler(void)
{
}
#endif

#ifdef USE_STUB_emMleAttachCompleteHandler
void emMleAttachCompleteHandler(bool success)
{
}
#endif

#ifdef USE_STUB_emMleModeIsLurker
bool emMleModeIsLurker(MleMessage *message)
{
  return (message->mode == BIT(7)
          || (message->isLegacy && message->mode == 0));
}
#endif

#ifdef USE_STUB_emZigbeeNetworkSecurityLevel
uint8_t emZigbeeNetworkSecurityLevel = 4;
#endif

#ifdef USE_STUB_sendLogEvent
void sendLogEvent(char *foo, char *bar)
{
}
#endif

#ifdef USE_STUB_emCoapInitialize
void emCoapInitialize(void)
{
}
#endif

#ifdef USE_STUB_emResendNodeServerData
void emResendNodeServerData(EmberNodeId oldNodeId)
{
}
#endif

#ifdef USE_STUB_emRemoveChildServerData
void emRemoveChildServerData(EmberNodeId nodeId)
{
}
#endif

#ifdef USE_STUB_emCostDivision
uint8_t emCostDivision(uint8_t cost)
{
  return 1;
}
#endif

#ifdef USE_STUB_emIsCustomLinkQualitySet
bool (*emIsCustomLinkQualitySet)(const uint8_t *eui64, uint8_t *newQuality) = NULL;
#endif

#ifdef USE_STUB_emEnableMle
bool emEnableMle = true;
#endif

#ifdef USE_STUB_printNeighborTable
void printNeighborTable(void)
{
}
#endif

#ifdef USE_STUB_emEnableMacEncryption
bool emEnableMacEncryption = true;
#endif

#ifdef USE_STUB_emEncryptDataPolls
bool emEncryptDataPolls = true;
#endif

#ifdef USE_STUB_emFindChild
EmberNodeId emFindChild(const uint8_t *id)
{
  return EMBER_NULL_NODE_ID;
}
#endif

#ifdef USE_STUB_emFindChildIndex
uint8_t emFindChildIndex(uint8_t startIndex, const uint8_t *eui64)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emFindChildIndexByAddress
uint8_t emFindChildIndexByAddress(const EmberIpv6Address *address)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emInitializeChild
bool emInitializeChild(uint8_t index,
                       EmberNodeId id,
                       const uint8_t *eui64,
                       uint8_t capabilities,
                       uint32_t timeoutSeconds)
{
  return true;
}
#endif

#ifdef USE_STUB_emNoteChildTransaction
void emNoteChildTransaction(uint8_t index)
{
}
#endif

#ifdef USE_STUB_emRetrySubmit
bool emRetrySubmit(PacketHeader header, uint8_t retries, uint16_t delayMs)
{
  return true;
}
#endif

#ifdef USE_STUB_emSubmitIndirectOrRetry
bool emSubmitIndirectOrRetry(PacketHeader header,
                             uint8_t retries,
                             uint16_t delayMs)
{
  return true;
}
#endif

#ifdef USE_STUB_emReallySubmitIpHeader
bool emReallySubmitIpHeader(PacketHeader header,
                            Ipv6Header *ipHeader,
                            bool allowLoopback,
                            uint8_t retries,
                            uint16_t delayMs)
{
  return true;
}
#endif

#ifdef USE_STUB_emForwardPacketFromHost
bool emForwardPacketFromHost(PacketHeader header, Ipv6Header *ipHeader)
{
  return true;
}
#endif

#ifdef USE_STUB_emRetryInit
void emRetryInit(void)
{
}
#endif

#ifdef USE_STUB_emberSerialPrintf
EmberStatus emberSerialPrintf(uint8_t port, PGM_P formatString, ...)
{
  return EMBER_SUCCESS;
}
#endif

#ifdef USE_STUB_emberSerialPrintfLine
EmberStatus emberSerialPrintfLine(uint8_t port, PGM_P formatString, ...)
{
  return EMBER_SUCCESS;
}
#endif

#ifdef USE_STUB_emberSerialWaitSend
EmberStatus emberSerialWaitSend(uint8_t port)
{
  return 0;
}
#endif

#ifdef USE_STUB_emInUseAsNextHop
bool emInUseAsNextHop(uint16_t id)
{
  return false;
}
#endif

#ifdef USE_STUB_emBrokenDagParent
void emBrokenDagParent(uint16_t parentId)
{
}
#endif

#ifdef USE_STUB_emSetPhyRadioChannel
uint8_t emSetPhyRadioChannel(uint8_t radioChannel)
{
  return 0;
}
#endif

#ifdef USE_STUB_emSetOptionalBeaconPayload
void emSetOptionalBeaconPayload(uint8_t *payload, uint8_t length)
{
}
#endif

#ifdef USE_STUB_printEui
void printEui(uint8_t *eui)
{
}
#endif

#ifdef USE_STUB_emberCounterHandler
void emberCounterHandler(EmberCounterType type, uint16_t increment)
{
}
#endif

#ifdef USE_STUB_emberCounterValueHandler
uint16_t emberCounterValueHandler(EmberCounterType type)
{
  return 0;
}
#endif

#ifdef USE_STUB_emDefaultSourceLongId
const uint8_t emDefaultSourceLongId[8] = {
  0x42, 0x8A, 0x2F, 0x98, 0x71, 0x37, 0x44, 0x91
};
#endif

#ifdef USE_STUB_emKeyIdMode2Key
const uint8_t emKeyIdMode2Key[16] = {
  0x08, 0x58, 0x16, 0x86, 0xfd, 0xb4, 0x58, 0x0f,
  0xb0, 0x92, 0x54, 0x6a, 0xec, 0xbd, 0x15, 0x66
};
#endif

#ifdef USE_STUB_emKeyIdMode2LongId
const uint8_t emKeyIdMode2LongId[8] = {
  0x12, 0x87, 0xd4, 0x23, 0xb8, 0xfe, 0x06, 0x35
};
#endif

#ifdef USE_STUB_emSendIcmpMessage
bool emSendIcmpMessage(uint8_t type,
                       uint8_t code,
                       const uint8_t *ipDest,
                       const uint8_t *payload,
                       uint16_t payloadLength)
{
  return false;
}
#endif

#ifdef USE_STUB_emSetNodeType
void emSetNodeType(EmberNodeType type, bool preserveLurker)
{
}
#endif

#ifdef USE_STUB_emberGetPanId
EmberPanId emberGetPanId(void)
{
  return 0;
}
#endif

#ifdef USE_STUB_emberGetRadioChannel
uint8_t emberGetRadioChannel(void)
{
  return 0;
}
#endif

#ifdef USE_STUB_emSetNodeId
void emSetNodeId(uint16_t nodeId)
{
}
#endif

#ifdef USE_STUB_emberSleepyChildPollTimeout
uint32_t emberSleepyChildPollTimeout = 300;
#endif

#ifdef USE_STUB_emberEndDevicePollTimeout
uint8_t emberEndDevicePollTimeout = 30;
#endif

#ifdef USE_STUB_emChildFrameCounters
static uint32_t macFrameCounters[16];
uint32_t *emChildMacFrameCounters = macFrameCounters;
static uint32_t mleFrameCounters[16];
uint32_t *emChildMleFrameCounters = mleFrameCounters;
#endif

#ifdef USE_STUB_emChildSecondsSinceLastTransaction
uint16_t emChildSecondsSinceLastTransaction(uint8_t index)
{
  return 0;
}
#endif

#ifdef USE_STUB_emChildSecondsSinceAttaching
uint16_t emChildSecondsSinceAttaching(uint8_t index)
{
  return 0;
}
#endif

#ifdef USE_STUB_emSendMleParentRequest
bool emSendMleParentRequest(bool everything)
{
  return false;
}
#endif

#ifdef USE_STUB_emSendMleChildIdRequest
bool emSendMleChildIdRequest(uint8_t *ipDest,
                             const uint8_t *response,
                             uint8_t responseLength,
                             bool amSleepy,
                             bool darkRouterActivationOnly)
{
  return false;
}
#endif

#ifdef USE_STUB_emChildCount
uint8_t emChildCount(void)
{
  return 0;
}
#endif

#ifdef USE_STUB_emChildIdTable
static uint16_t childIdTable[16];
uint16_t *emChildIdTable = childIdTable;
#endif

#ifdef USE_STUB_emChildStatus
static uint16_t childStatus[16];
uint16_t *emChildStatus = childStatus;
#endif

#ifdef USE_STUB_emChildTimeouts
static uint32_t childTimeouts[16];
uint32_t *emChildTimeouts = childTimeouts;
#endif

#ifdef USE_STUB_emChildSetFrameCounter
void emChildSetFrameCounter(uint8_t index,
                            uint32_t macFrameCounter,
                            uint32_t mleFrameCounter,
                            uint32_t keySequence)
{
}
#endif

#ifdef USE_STUB_emberNetworkStatus
EmberNetworkStatus emberNetworkStatus(void)
{
  return EMBER_JOINED_NETWORK_ATTACHED;
}
#endif

#ifdef USE_STUB_emberNetworkStatusHandler
void emberNetworkStatusHandler(EmberNetworkStatus newNetworkStatus,
                               EmberNetworkStatus oldNetworkStatus,
                               EmberJoinFailureReason reason)
{
}

#endif

#ifdef USE_STUB_emVerifyLocalServerData
void emVerifyLocalServerData(void)
{
}
#endif

#ifdef USE_STUB_emLocalServerData
Buffer emLocalServerData = NULL_BUFFER;
#endif

#ifdef USE_STUB_emInitializeLeaderServerData
void emInitializeLeaderServerData(void)
{
}
#endif

#ifdef USE_STUB_emProcessIdResponse
void emProcessIdResponse(uint16_t shortAddress,
                         uint8_t assignedIdSequence,
                         const uint8_t *assignedIdMask)
{
}
#endif

#ifdef USE_STUB_emMakeAssignedRipIdMask
bool emMakeAssignedRipIdMask(uint8_t *mask)
{
  memset(mask, 0xFF, ASSIGNED_RIP_ID_MASK_SIZE);
  return ASSIGNED_RIP_ID_MASK_SIZE;
}
#endif

#ifdef USE_STUB_emRipIdSequenceNumber
uint8_t emRipIdSequenceNumber = 0;
#endif

#ifdef USE_STUB_emProcessRipNeighborTlv
void emProcessRipNeighborTlv(MleMessage *message)
{
}
#endif

#ifdef USE_STUB_JoinState
// Values for emJoinState.
typedef enum {
  JOIN_COMPLETE                  = 0,
} JoinState;
#endif

#ifdef USE_STUB_emJoinState
uint8_t emJoinState = JOIN_COMPLETE;
#endif

#ifdef USE_STUB_AttachState
typedef enum {
  ATTACH_IDLE                = 0,
} AttachState;
#endif

#ifdef USE_STUB_emAttachState
uint8_t emAttachState = ATTACH_IDLE;
#endif

#ifdef USE_STUB_emAttachParentLongId
const uint8_t *emAttachParentLongId(void)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emEraseChild
void emEraseChild(uint8_t childIndex)
{
}
#endif

#ifdef USE_STUB_emIsAssignedId
bool emIsAssignedId(EmberNodeId id, const uint8_t *neighborData)
{
  return true;
}
#endif

#ifdef USE_STUB_emCurrentRssi
int8_t emCurrentRssi;
#endif

#ifdef USE_STUB_emGetRoute
uint8_t emGetRoute(uint8_t destIndex, uint8_t *routeCostResult)
{
  *routeCostResult = 0;
  return 0xFF;
}
#endif

#ifdef USE_STUB_emGetRouteCost
uint8_t emGetRouteCost(uint16_t shortMacId)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emHastenRipAdvertisement
void emHastenRipAdvertisement(void)
{
}
#endif

#ifdef USE_STUB_emMaybeHastenRipAdvertisement
void emMaybeHastenRipAdvertisement(uint8_t index, uint8_t oldRouteCost)
{
}
#endif

#ifdef USE_STUB_emRouterRebootSuccess
void emRouterRebootSuccess(void)
{
}
#endif

#ifdef USE_STUB_emStopLeaderTimeout
void emStopLeaderTimeout(void)
{
}
#endif

#ifdef USE_STUB_emIpConnections
Buffer emIpConnections;
#endif

#ifdef USE_STUB_emChildInit
void emChildInit(void)
{
}
#endif

#ifdef USE_STUB_emRestoreChildTable
void emRestoreChildTable(void)
{
}
#endif

#ifdef USE_STUB_emBeaconPayload
uint8_t *emBeaconPayload(PacketHeader beacon)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emBeaconPayloadSize
uint8_t emBeaconPayloadSize = 0;
#endif

#ifdef USE_STUB_emBorderRouterTableInit
void emBorderRouterTableInit(void)
{
}
#endif

#ifdef USE_STUB_emEraseHostRegistry
void emEraseHostRegistry(void)
{
}
#endif

#ifdef USE_STUB_emFormSecurityInit
void emFormSecurityInit(void)
{
}
#endif

#ifdef USE_STUB_emFragmentState
uint8_t emFragmentState = 0;
#endif

#ifdef USE_STUB_emIncomingForMeToUart
bool emIncomingForMeToUart = false;
#endif

#ifdef USE_STUB_emJoinSecurityInit
void emJoinSecurityInit(void)
{
}
#endif

#ifdef USE_STUB_emLocalPermitJoinUpdate
void emLocalPermitJoinUpdate(uint32_t durationMs)
{
}
#endif

#ifdef USE_STUB_emMacScanType
uint8_t emMacScanType = 0;
#endif

#ifdef USE_STUB_emRadioGetRandomNumbers
bool emRadioGetRandomNumbers(uint16_t *buffer, uint8_t length)
{
  return true;
}
#endif

#ifdef USE_STUB_emSaveNetwork
void emSaveNetwork(void)
{
}
#endif

#ifdef USE_STUB_emSecurityToUart
bool emSecurityToUart = false;
#endif

#ifdef USE_STUB_emSendMleLurkerJoinRequest
bool emSendMleLurkerJoinRequest(uint8_t *ipDest)
{
  return false;
}
#endif

#ifdef USE_STUB_emSendMleRequest
bool emSendMleRequest(uint8_t *ipDest, bool jitter, bool isLegacy)
{
  return false;
}
#endif

#ifdef USE_STUB_emSendMleRouterIdVerifyRequest
bool emSendMleRouterIdVerifyRequest(void)
{
  return false;
}
#endif

#ifdef USE_STUB_emSendMleReject
void emSendMleReject(uint8_t *ipDest, MleStatus status, bool useKeySource)
{
}
#endif

#ifdef USE_STUB_emSendPermitJoinUpdate
bool emSendPermitJoinUpdate(uint32_t duration)
{
  return false;
}
#endif

#ifdef USE_STUB_emSetPanId
void emSetPanId(uint16_t panId)
{
}
#endif

#ifdef USE_STUB_emSetRadioPower
void emSetRadioPower(int8_t power)
{
}
#endif

#ifdef USE_STUB_emStartDagRoot
bool emStartDagRoot(uint8_t instanceId,
                    const DagConfiguration *configuration,
                    bool dao,
                    uint8_t *pioPrefix)
{
  return false;
}
#endif

#ifdef USE_STUB_emStartLeaderTimeout
void emStartLeaderTimeout(void)
{
}
#endif

#ifdef USE_STUB_emStartJoinClient
bool emStartJoinClient(const uint8_t *address,
                       const uint8_t *key,
                       uint8_t keyLength)
{
  return false;
}
#endif

#ifdef USE_STUB_emberStartHostJoinClientHandler
void emberStartHostJoinClientHandler(const uint8_t *parentAddress)
{
}
#endif

#ifdef USE_STUB_emStartPolling
void emStartPolling(void)
{
}
#endif

#ifdef USE_STUB_emStartRouterSelection
void emStartRouterSelection(AttachReason reason)
{
}
#endif

#ifdef USE_STUB_emStartLurkerAdvertisements
void emStartLurkerAdvertisements(void)
{
}
#endif

#ifdef USE_STUB_emStopLurkerAdvertisements
void emStopLurkerAdvertisements(void)
{
}
#endif

#ifdef USE_STUB_emStopRip
void emStopRip(void)
{
}
#endif

#ifdef USE_STUB_emberAddAddressDataReturn
void emberAddAddressDataReturn(uint16_t shortId)
{
}
#endif

#ifdef USE_STUB_emberClearAddressCacheReturn
void emberClearAddressCacheReturn(void)
{
}
#endif

#ifdef USE_STUB_emberLookupAddressData
void emberLookupAddressData(const uint8_t *longId)
{
}
#endif

#ifdef USE_STUB_emberClearAddressCache
void emberClearAddressCache(void)
{
}
#endif

#ifdef USE_STUB_emberGetNodeStatus
void emberGetNodeStatus(void)
{
}
#endif

#ifdef USE_STUB_emberGetNetworkKeyInfo
void emberGetNetworkKeyInfo(bool keyInUse)
{
}
#endif

#ifdef USE_STUB_emberGpioRadioPowerMask
void emberGpioRadioPowerMask(uint32_t mask)
{
}
#endif

#ifdef USE_STUB_emberGpioCurrentConfig
void emberGpioCurrentConfig(uint8_t portPin, uint8_t cfg, uint8_t out)
{
}
#endif

#ifdef USE_STUB_emberGpioPowerConfig
void emberGpioPowerConfig(uint8_t portPin, uint8_t puCfg, uint8_t puOut,
                          uint8_t pdCfg, uint8_t pdOut)
{
}
#endif

#ifdef USE_STUB_emberGpioCurrentInput
void emberGpioCurrentInput(uint8_t portPin)
{
}
#endif

#ifdef USE_STUB_emberSetClockMonitor
void emberSetClockMonitor(uint8_t enable, uint32_t timerId)
{
}
#endif

#ifdef USE_STUB_emberSetPassthroughPort
void emberSetPassthroughPort(uint16_t port)
{
}
#endif

#ifdef USE_STUB_emberLookupAddressDataReturn
void emberLookupAddressDataReturn(uint16_t shortId)
{
}
#endif

#ifdef USE_STUB_emberActiveScanHandler
void emberActiveScanHandler(const EmberMacBeaconData *beaconData)
{
}
#endif

#ifdef USE_STUB_emberCommissionNetworkReturn
void emberCommissionNetworkReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberFormNetworkReturn
void emberFormNetworkReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emIpIncomingBeaconHandler
void emIpIncomingBeaconHandler(PacketHeader header)
{
}
#endif

#ifdef USE_STUB_emberJoinNetworkReturn
void emberJoinNetworkReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberAttachToNetworkReturn
void emberAttachToNetworkReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberChangeNodeTypeReturn
void emberChangeNodeTypeReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberPermitJoiningReturn
void emberPermitJoiningReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberPermitJoiningHandler
void emberPermitJoiningHandler(bool joiningAllowed)
{
}
#endif

#ifdef USE_STUB_emberResumeNetworkReturn
void emberResumeNetworkReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberSetRadioPower
void emberSetRadioPower(int8_t power)
{
}
#endif

#ifdef USE_STUB_emberStartScan
void emberStartScan(EmberNetworkScanType scanType,
                    uint32_t channelMask,
                    uint8_t duration)
{
}
#endif

#ifdef USE_STUB_AddressData
typedef struct {
  uint8_t iid[8];
  EmberNodeId shortId;  // 0xFFFF = unused, 0xFFFE = not known
  uint8_t context;        // 6LoWPAN context for the prefix
  uint8_t key;            // retrieval key
  uint8_t flags;          // flags for DHCP, discovery, etc.
  uint8_t head;           // least-recently-used queue
  uint8_t tail;
  uint32_t lastTransactionSeconds;
} AddressData;
#endif

#ifdef USE_STUB_emAddAddressData
AddressData *emAddAddressData(uint8_t context,
                              const uint8_t *iid,
                              EmberNodeId shortId,
                              bool update)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emAddressCache
Vector emAddressCache = {
  NULL_BUFFER,
  sizeof(AddressData),
  0,
  0
};
#endif

#ifdef USE_STUB_emAmLeader
bool emAmLeader = false;
#endif

#ifdef USE_STUB_emAmThreadCommissioner
bool emAmThreadCommissioner(void)
{
  return false;
}
#endif

#ifdef USE_STUB_emFindNextNetworkDataTlv
uint8_t *emFindNextNetworkDataTlv(uint8_t type, uint8_t *start)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emRemoveNetworkDataSubTlv
void emRemoveNetworkDataSubTlv(uint8_t *outerTlv, uint8_t *innerTlv)
{
}
#endif

#ifdef USE_STUB_emSetNetworkDataTlvLength
uint8_t *emSetNetworkDataTlvLength(uint8_t *tlv, uint16_t tlvDataLength)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emberAddAddressData
void emberAddAddressData(const uint8_t *longId, uint16_t shortId)
{
}
#endif

#ifdef USE_STUB_emFindNearestDhcpServer
EmberNodeId emFindNearestDhcpServer(const uint8_t *prefix, uint8_t prefixLengthInBits)
{
  return 0xFFFE;
}
#endif

#ifdef USE_STUB_emStoreIpv6Header
bool emStoreIpv6Header(Ipv6Header *source, PacketHeader header)
{
  return false;
}
#endif

#ifdef USE_STUB_emberUdpMulticastListen
EmberStatus emberUdpMulticastListen(uint16_t port, const uint8_t *multicastAddress)
{
  return 0;
}
#endif

#ifdef USE_STUB_emberInitReturn
void emberInitReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberInitializeHostAddressTable
void emberInitializeHostAddressTable(void)
{
}
#endif

#ifdef USE_STUB_emberGetHostGlobalAddress
GlobalAddressEntry *emberGetHostGlobalAddress(uint8_t index)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emberAddHostGlobalAddress
EmberStatus emberAddHostGlobalAddress(const EmberIpv6Address *address,
                                      uint32_t preferredLifetime,
                                      uint32_t validLifetime,
                                      uint8_t addressFlags)
{
  return EMBER_SUCCESS;
}
#endif

#ifdef USE_STUB_emberRemoveHostGlobalAddress
EmberStatus emberRemoveHostGlobalAddress(const EmberIpv6Address *address)
{
  return EMBER_SUCCESS;
}
#endif

#ifdef USE_STUB_emberInitializeListeners
void emberInitializeListeners(void)
{
}
#endif

#ifdef USE_STUB_emberCloseListeners
void emberCloseListeners(void)
{
}
#endif

#ifdef USE_STUB_emAllocateTlsState
Buffer emAllocateTlsState(uint8_t fd, uint16_t flags)
{
  return NULL_BUFFER;
}
#endif

#ifdef USE_STUB_emMarkTlsState
void emMarkTlsState(Buffer *tlsBufferLoc)
{
}
#endif

#ifdef USE_STUB_emSavedTlsSessions
Buffer emSavedTlsSessions = NULL_BUFFER;
#endif

#ifdef USE_STUB_TcpConnection
typedef struct {} *TcpConnection;
#endif

#ifdef USE_STUB_emStartTlsResume
void emStartTlsResume(TcpConnection *connection)
{
}
#endif

#ifdef USE_STUB_emTlsClose
bool emTlsClose(TcpConnection *connection)
{
  return true;
}
#endif

#ifdef USE_STUB_emTlsSendApplicationBuffer
EmberStatus emTlsSendApplicationBuffer(TcpConnection *connection,
                                       Buffer content)
{
  return 0;
}
#endif

#ifdef USE_STUB_emTlsSendBufferedApplicationData
EmberStatus emTlsSendBufferedApplicationData(TcpConnection *connection,
                                             const uint8_t *data,
                                             uint16_t length,
                                             Buffer moreData,
                                             uint16_t moreLength)
{
  return EMBER_ERR_FATAL;
}
#endif

#ifdef USE_STUB_emTlsStatusHandler
bool emTlsStatusHandler(TcpConnection *connection)
{
  return 0;
}
#endif

#ifdef USE_STUB_emDtlsJoinMode
uint8_t emDtlsJoinMode;
#endif

#ifdef USE_STUB_emParentConnectionHandle
uint8_t emParentConnectionHandle;
#endif

#ifdef USE_STUB_emHandleJoinDtlsMessage
void emHandleJoinDtlsMessage(EmberUdpConnectionData *connection,
                             uint8_t *packet,
                             uint16_t length,
                             Buffer buffer)
{
}
#endif

#ifdef USE_STUB_emSubmitRelayedJoinPayload
void emSubmitRelayedJoinPayload(DtlsConnection *connection, Buffer payload)
{
}
#endif

#ifdef USE_STUB_emSendJoinerEntrust
void emSendJoinerEntrust(DtlsConnection *connection, const uint8_t *key)
{
}
#endif

#ifdef USE_STUB_emberSetDtlsDeviceCertificate
void emberSetDtlsDeviceCertificate(const CertificateAuthority **certAuthority,
                                   const DeviceCertificate *deviceCert)
{
}
#endif

#ifdef USE_STUB_emberSetDtlsDeviceCertificateReturn
void emberSetDtlsDeviceCertificateReturn(uint32_t result)
{
}
#endif

#ifdef USE_STUB_emberSetDtlsPresharedKey
void emberSetDtlsPresharedKey(const uint8_t *key,
                              uint8_t keyLength,
                              const EmberIpv6Address *remoteAddress)
{
}
#endif

#ifdef USE_STUB_emberSetDtlsPresharedKeyReturn
void emberSetDtlsPresharedKeyReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberOpenDtlsConnection
void emberOpenDtlsConnection(const EmberIpv6Address *remoteAddress,
                             uint16_t localPort,
                             uint16_t remotePort)
{
}
#endif

#ifdef USE_STUB_emberOpenDtlsConnectionReturn
void emberOpenDtlsConnectionReturn(uint32_t result,
                                   const EmberIpv6Address *remoteAddress,
                                   uint16_t localPort,
                                   uint16_t remotePort)
{
}
#endif

#ifdef USE_STUB_emberDtlsSecureSessionEstablished
void emberDtlsSecureSessionEstablished(uint8_t flags,
                                       uint8_t sessionId,
                                       const EmberIpv6Address *localAddress,
                                       const EmberIpv6Address *remoteAddress,
                                       uint16_t localPort,
                                       uint16_t remotePort)
{
}
#endif

#ifdef USE_STUB_emberGetSecureDtlsSessionId
void emberGetSecureDtlsSessionId(const EmberIpv6Address *remoteAddress,
                                 uint16_t localPort,
                                 uint16_t remotePort)
{
}
#endif

#ifdef USE_STUB_emberGetSecureDtlsSessionIdReturn
void emberGetSecureDtlsSessionIdReturn(uint8_t sessionId,
                                       const EmberIpv6Address *remoteAddress,
                                       uint16_t localPort,
                                       uint16_t remotePort)
{
}
#endif

#ifdef USE_STUB_emberCloseDtlsConnection
void emberCloseDtlsConnection(uint8_t sessionId)
{
}
#endif

#ifdef USE_STUB_emberCloseDtlsConnectionReturn
void emberCloseDtlsConnectionReturn(uint8_t sessionId, EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emLoseHelper
void emLoseHelper(void)
{
}
#endif

#ifdef USE_STUB_emHeaderSetTag
void emHeaderSetTag(PacketHeader header, uint8_t tag)
{
}
#endif

#ifdef USE_STUB_emHeaderMacInfoField
uint8_t emHeaderMacInfoField(PacketHeader header)
{
  return 0;
}
#endif

#ifdef USE_STUB_emHeaderSetMacInfoField
void emHeaderSetMacInfoField(PacketHeader header, uint8_t info)
{
}
#endif

#ifdef USE_STUB_emMakeDataHeader
PacketHeader emMakeDataHeader(bool isBroadcast,
                              bool longSource,
                              bool longDest,
                              uint8_t *payload,
                              uint16_t payloadLength)
{
  return NULL_BUFFER;
}
#endif

#ifdef USE_STUB_emMarkBufferPointer
void emMarkBufferPointer(void **pointerLoc)
{
}
#endif

#ifdef USE_STUB_emSetMacLongDestination
void emSetMacLongDestination(PacketHeader header, const uint8_t *longId)
{
}
#endif

#ifdef USE_STUB_emSetMacShortDestination
void emSetMacShortDestination(PacketHeader header, EmberNodeId id)
{
}
#endif

#ifdef USE_STUB_emDhcpClientIncomingMessageHandler
bool emDhcpClientIncomingMessageHandler(Ipv6Header *ipHeader)
{
  return false;
}
#endif

#ifdef USE_STUB_emDnsClientIncomingMessageHandler
void emDnsClientIncomingMessageHandler(PacketHeader header,
                                       Ipv6Header *ipHeader)
{
}
#endif

#ifdef USE_STUB_emberDnsLookup
EmberStatus emberDnsLookup(const uint8_t *domainName,
                           uint8_t domainNameLength,
                           const uint8_t *prefix64,
                           EmberDnsResponseHandler responseHandler,
                           uint8_t *appData,
                           uint16_t appDataLength)
{
  return EMBER_ERR_FATAL;
}
#endif

#ifdef USE_STUB_emFindNeighborIndexByIpAddress
uint8_t emFindNeighborIndexByIpAddress(const uint8_t *ipAddress)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emFindNeighborIndexByShortId
uint8_t emFindNeighborIndexByShortId(uint16_t shortId)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emMleIncomingMessageHandler
void emMleIncomingMessageHandler(PacketHeader header,
                                 Ipv6Header *ipHeader,
                                 PacketHeader headerForReuse)
{
}
#endif

#ifdef USE_STUB_emNeighborShortId
uint16_t emNeighborShortId(uint8_t index)
{
  return 0xFFFE;
}
#endif

#ifdef USE_STUB_emNeighborHaveShortId
bool emNeighborHaveShortId(uint8_t index)
{
  return false;
}
#endif

#ifdef USE_STUB_emNeighborSetShortId
uint8_t emNeighborSetShortId(uint8_t index, EmberNodeId id)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emberPollHandler
void emberPollHandler(EmberNodeId childId, bool transmitExpected)
{
}
#endif

#ifdef USE_STUB_emberTcpAcceptHandler
void emberTcpAcceptHandler(uint16_t port, uint8_t fd)
{
}
#endif

#ifdef USE_STUB_emberTcpReadHandler
void emberTcpReadHandler(uint8_t fd, uint8_t *buffer, uint16_t count)
{
}
#endif

#ifdef USE_STUB_emberTcpStatusHandler
void emberTcpStatusHandler(uint8_t fd, uint8_t status)
{
}
#endif

#ifdef USE_STUB_emberUdpListen
EmberStatus emberUdpListen(uint16_t port, const uint8_t *address)
{
  return EMBER_SUCCESS;
}
#endif

#ifdef USE_STUB_emberUdpHandler
void emberUdpHandler(const uint8_t *destination,
                     const uint8_t *source,
                     uint16_t localPort,
                     uint16_t remotePort,
                     const uint8_t *payload,
                     uint16_t payloadLength)
{
}
#endif

#ifdef USE_STUB_emberUdpMulticastHandler
void emApiUdpMulticastHandler(const uint8_t *destinationIpv6Address,
                              const uint8_t *sourceIpv6Address,
                              uint16_t localPort,
                              uint16_t remotePort,
                              const uint8_t *packet,
                              uint16_t length)
{
}
#endif

#ifdef USE_STUB_emGetMacKey
uint8_t *emGetMacKey(uint32_t sequence, uint8_t *storage)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emGetLegacyNetworkKey
uint8_t *emGetLegacyNetworkKey(void)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emMacHeaderLength
uint8_t emMacHeaderLength(uint8_t *macFrame)
{
  return 0;
}
#endif

#ifdef USE_STUB_emCcmDecryptPacket
bool emCcmDecryptPacket(const uint8_t *nonce,
                        uint8_t *packet,
                        uint16_t authenticateLength,
                        uint8_t *encrypt,
                        uint16_t encryptLength,
                        uint8_t micLength)
{
  return true;
}
#endif

#ifdef USE_STUB_emEapWriteBuffer
void emEapWriteBuffer(TlsState *tls, Buffer buffer)
{
}
#endif

#ifdef USE_STUB_emFinishDtlsServerJoin
EmberStatus emFinishDtlsServerJoin(void *connection)
{
  return EMBER_SUCCESS;
}
#endif

#ifdef USE_STUB_emCloseDtlsJoinConnection
void emCloseDtlsJoinConnection(void)
{
}
#endif

#ifdef USE_STUB_emJoinSecurityFailed
void emJoinSecurityFailed(void)
{
}
#endif

#ifdef USE_STUB_emGetCommissioningMacKey
uint8_t *emGetCommissioningMacKey(const uint8_t *senderEui64)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emMaxEndDeviceChildren
uint8_t emMaxEndDeviceChildren = 0;
#endif

#ifdef USE_STUB_emTaskCount
uint8_t emTaskCount = 0;
#endif

#ifdef USE_STUB_emTasks
EmberTaskControl emTasks[1];
#endif

#ifdef USE_STUB_emberEventDelayUpdatedFromIsrHandler
void emberEventDelayUpdatedFromIsrHandler(Event *event)
{
}
#endif

#ifdef USE_STUB_heapMemory
uint16_t heapMemory[8000];
#endif

#ifdef USE_STUB_heapMemorySize
const uint32_t heapMemorySize = 8000;
#endif

#ifdef USE_STUB_emGetOutgoingCommissioningMacKey
uint8_t *emGetOutgoingCommissioningMacKey(const uint8_t *senderEui64)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emLoadKeyIntoCore
void emLoadKeyIntoCore(const uint8_t *key)
{
}
#endif

#ifdef USE_STUB_emReadAndProcessRecord
TlsStatus emReadAndProcessRecord(TlsState *tls, Buffer *incomingQueue)
{
  return TLS_SUCCESS;
}
#endif

#ifdef USE_STUB_emRunHandshake
TlsStatus emRunHandshake(TlsState *tls, uint8_t *incoming, uint16_t length)
{
  return TLS_SUCCESS;
}
#endif

#ifdef USE_STUB_emSha256HashBytes
void emSha256HashBytes(Sha256State *hashState, const uint8_t *bytes, uint16_t count)
{
}
#endif

#ifdef USE_STUB_emSha256Start
void emSha256Start(Sha256State *hashState)
{
}
#endif

#ifdef USE_STUB_emTlsSetState
void emTlsSetState(TlsState *tls, uint8_t state)
{
}
#endif

#ifdef USE_STUB_emAddDhcpClient
bool emAddDhcpClient(uint8_t *address, const uint8_t *identifier)
{
  return false;
}
#endif

#ifdef USE_STUB_emCcmEncryptPacket
void emCcmEncryptPacket(const uint8_t *nonce,
                        uint8_t *packet,
                        uint16_t authenticateLength,
                        uint16_t encryptLength,
                        uint8_t micLength)
{
}
#endif

#ifdef USE_STUB_emCompressContextPrefix
uint8_t emCompressContextPrefix(uint8_t *address,
                                uint8_t matchLength,
                                uint8_t *context)
{
  return 0;
}
#endif

#ifdef USE_STUB_emFe8Prefix
const Bytes8 emFe8Prefix = { { 0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
#endif

#ifdef USE_STUB_emIsLl64
bool emIsLl64(const uint8_t *address)
{
  return false;
}
#endif

#ifdef USE_STUB_emGetMleKey
uint8_t *emGetMleKey(uint32_t sequence, uint8_t *storage)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emGetLegacyMleKey
uint8_t *emGetLegacyMleKey(void)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emGetNetworkMasterKey
const uint8_t *emGetNetworkMasterKey(uint8_t *storage)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emSetNetworkMasterKey
void emSetNetworkMasterKey(uint8_t const *key, uint32_t sequence)
{
}
#endif

#ifdef USE_STUB_emSecurityIncrementOutgoingFrameCounter
uint32_t emSecurityIncrementOutgoingFrameCounter(void)
{
  return 1;
}
#endif

#ifdef USE_STUB_emGetSecurityFrameCounter
uint32_t emGetSecurityFrameCounter(void)
{
  return 0;
}
#endif

#ifdef USE_STUB_emGetSequenceFromAuxFrame
uint8_t emGetSequenceFromAuxFrame(const uint8_t *auxFrame, uint32_t *sequence)
{
  return false;
}
#endif

#ifdef USE_STUB_emSendMleChildReboot
bool emSendMleChildReboot(void)
{
  return false;
}
#endif

#ifdef USE_STUB_emGetThreadJoin
bool emGetThreadJoin(void)
{
  return true;
}
#endif

#ifdef USE_STUB_emGetThreadNativeCommission
bool emGetThreadNativeCommission(void)
{
  return true;
}
#endif

#ifdef USE_STUB_emAcceptUnsecuredFragment
bool emAcceptUnsecuredFragment(PacketHeader firstFragment)
{
  return false;
}
#endif

#ifdef USE_STUB_emNoteExternalCommissioner
void emNoteExternalCommissioner(EmberUdpConnectionHandle handle, bool available)
{
}
#endif

#ifdef USE_STUB_emHandleIncomingTcp
void emHandleIncomingTcp(PacketHeader header, Ipv6Header *ipHeader)
{
}
#endif

#ifdef USE_STUB_emHandleIncomingUdp
void emHandleIncomingUdp(PacketHeader header, Ipv6Header *ipHeader)
{
}
#endif

#ifdef USE_STUB_emHandleIncomingCoapCommission
void emHandleIncomingCoapCommission(PacketHeader header, Ipv6Header *ipHeader)
{
}
#endif

#ifdef USE_STUB_emTrickleMulticastInterval
uint8_t emTrickleMulticastInterval = 6;
#endif

#ifdef USE_STUB_emIsDuplicateMulticast
bool emIsDuplicateMulticast(Ipv6Header *ipHeader)
{
  return false;
}
#endif

#ifdef USE_STUB_emIncomingJoinMessageHandler
void emIncomingJoinMessageHandler(PacketHeader header,
                                  Ipv6Header *ipHeader,
                                  bool relayed)
{
}
#endif

#ifdef USE_STUB_emStartAppCommissioning
bool emStartAppCommissioning(bool joinerSession)
{
  return false;
}
#endif

#ifdef USE_STUB_emForwardToCommissioner
void emForwardToCommissioner(PacketHeader header, Ipv6Header *ipHeader)
{
}
#endif

#ifdef USE_STUB_emForwardToJoiner
void emForwardToJoiner(const uint8_t *joinerIid,
                       uint16_t joinerPort,
                       EmberNodeId joinerRouterNodeId,
                       const uint8_t *kek,
                       Buffer payload)
{
}
#endif

#ifdef USE_STUB_emGenerateKek
void emGenerateKek(TlsState *tls, uint8_t *key)
{
}
#endif

#ifdef USE_STUB_emSteeringDataMatch
bool emSteeringDataMatch(uint8_t *data, uint8_t length)
{
  return false;
}
#endif

#ifdef USE_STUB_emberBecomeCommissionerReturn
void emberBecomeCommissionerReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberCommissionerStatusHandler
void emberCommissionerStatusHandler(uint16_t flags,
                                    const uint8_t *commissionerName,
                                    uint8_t commissionerNameLength)
{
}
#endif

#ifdef USE_STUB_emberAllowNativeCommissionerReturn
void emberAllowNativeCommissionerReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberSetCommissionerKeyReturn
void emberSetCommissionerKeyReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberSetJoinKeyReturn
void emberSetJoinKeyReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emSetBeaconSteeringData
void emSetBeaconSteeringData(const uint8_t *steeringData, uint8_t steeringDataLength)
{
}
#endif

#ifdef USE_STUB_emIsSleepyChild
bool emIsSleepyChild(EmberNodeId id)
{
  return false;
}
#endif

#ifdef USE_STUB_emIsFullThreadDeviceChild
bool emIsFullThreadDeviceChild(EmberNodeId id)
{
  return false;
}
#endif

#ifdef USE_STUB_emMacCommandHandler
void emMacCommandHandler(PacketHeader header)
{
}
#endif

#ifdef USE_STUB_emMacSubmitIndirect
EmberStatus emMacSubmitIndirect(PacketHeader header)
{
  return 0;
}
#endif

#ifdef USE_STUB_emLoopbackMessageQueue
Buffer emLoopbackMessageQueue = NULL_BUFFER;
#endif

#ifdef USE_STUB_emMaybeAddChild
uint8_t emMaybeAddChild(EmberNodeId id,
                        const uint8_t *longId,
                        uint8_t capabilities,
                        uint32_t timeoutSeconds,
                        uint8_t *macFrameCounter,
                        uint8_t *mleFrameCounter)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emMulticastContext
uint8_t *emMulticastContext(uint8_t context)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emMulticastTableInit
void emMulticastTableInit(void)
{
}
#endif

#ifdef USE_STUB_emNoteIdMapping
bool emNoteIdMapping(uint8_t context,
                     const uint8_t *longId,
                     EmberNodeId shortId,
                     uint32_t lastTransactionSeconds,
                     const uint8_t *mlEid)
{
  return true;
}
#endif

#ifdef USE_STUB_emProcessIncomingBeacon
void emProcessIncomingBeacon(PacketHeader header)
{
}
#endif

#ifdef USE_STUB_emRetryTransmit
PacketHeader emRetryTransmit(void)
{
  return NULL_BUFFER;
}
#endif

#ifdef USE_STUB_emRplConnectivityHandler
void emRplConnectivityHandler(bool connected)
{
}
#endif

#ifdef USE_STUB_emSendIcmpErrorMessage
bool emSendIcmpErrorMessage(uint8_t type,
                            uint8_t code,
                            PacketHeader header,
                            Ipv6Header *ipHeader)
{
  return false;
}
#endif

#ifdef USE_STUB_emSendIcmpErrorMessageToDest
bool emSendIcmpErrorMessageToDest(uint8_t type,
                                  uint8_t code,
                                  uint8_t *ipDest,
                                  PacketHeader header,
                                  Ipv6Header *ipHeader)
{
  return false;
}
#endif

#ifdef USE_STUB_emSetPermitJoin
void emSetPermitJoin(bool on)
{
}
#endif

#ifdef USE_STUB_emUncompressContextPrefix
bool emUncompressContextPrefix(uint8_t context, uint8_t *address)
{
  return false;
}
#endif

#ifdef USE_STUB_emExtraJoinAttempts
uint8_t emExtraJoinAttempts = 0;
#endif

#ifdef USE_STUB_emEventControlSetDelayMS
void emEventControlSetDelayMS(EmberEventControl*event, uint32_t delay)
{
}
#endif

#ifdef USE_STUB_emIsDefaultGlobalPrefix
bool emIsDefaultGlobalPrefix(const uint8_t *prefix)
{
  return true;
}
#endif

#ifdef USE_STUB_emIsLegacyUla
bool emIsLegacyUla(const uint8_t *prefix)
{
  return false;
}
#endif

#ifdef USE_STUB_emIsFe8Address
bool emIsFe8Address(const uint8_t *address)
{
  return true;
}
#endif

#ifdef USE_STUB_emIsFf32MulticastAddress
bool emIsFf32MulticastAddress(const uint8_t *address)
{
  return true;
}
#endif

#ifdef USE_STUB_emMacExtendedId
uint8_t emMacExtendedId[8];
#endif

#ifdef USE_STUB_emSetMacExtendedId
void emSetMacExtendedId(const uint8_t *macExtendedId)
{
}
#endif

#ifdef USE_STUB_emSetMacExtendedIdToEui64
void emSetMacExtendedIdToEui64(void)
{
}
#endif

#ifdef USE_STUB_emLocalEui64
EmberEui64 emLocalEui64 = { { 0 } };
#endif

#ifdef USE_STUB_emberEui64
const EmberEui64 *emberEui64(void)
{
  return &emLocalEui64;
}
#endif

#ifdef USE_STUB_emMacShortSource
EmberNodeId emMacShortSource(PacketHeader header)
{
  return EMBER_NULL_NODE_ID;
}
#endif

#ifdef USE_STUB_emMacSourceMode
uint16_t emMacSourceMode(PacketHeader header)
{
  return 0;
}
#endif

#ifdef USE_STUB_emMacSourcePointer
uint8_t *emMacSourcePointer(PacketHeader header)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emMleTlvPresent
bool emMleTlvPresent(const MleMessage *message, uint8_t tlv)
{
  return false;
}
#endif

#ifdef USE_STUB_emReallyEndLogLine
void emReallyEndLogLine(uint8_t logType)
{
}
#endif

#ifdef USE_STUB_emReallyLog
void emReallyLog(uint8_t logType, PGM_P formatString, ...)
{
}
#endif

#ifdef USE_STUB_emReallyLogBytes
void emReallyLogBytes(uint8_t logType,
                      bool lineStartAndEnd,
                      bool useHex,
                      uint8_t const *bytes,
                      uint16_t length,
                      PGM_P formatString,
                      ...)
{
}
#endif

#ifdef USE_STUB_emReallyStartLogLine
void emReallyStartLogLine(uint8_t logType)
{
}
#endif

#ifdef USE_STUB_halCommonGetRandomTraced
uint16_t halCommonGetRandomTraced(char *file, int line)
{
  return 0;
}
#endif

#ifdef USE_STUB_emberSerialWriteString
EmberStatus emberSerialWriteString(uint8_t port, PGM_P string)
{
  return EMBER_SUCCESS;
}
#endif

#ifdef USE_STUB_emGetLeaderEui64
const uint8_t *emGetLeaderEui64(void)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emContentType
uint8_t emContentType = 0;
#endif

#ifdef USE_STUB_emRssiToCost
uint8_t emRssiToCost(int8_t rssi)
{
  return 0;
}
#endif

#ifdef USE_STUB_emRadioIdleMicroForUs
void emRadioIdleMicroForUs(uint32_t delayUs)
{
}
#endif

#ifdef USE_STUB_emberAbortWakeupReturn
void emberAbortWakeupReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emMarkWakeupBuffers
void emMarkWakeupBuffers(void)
{
}
#endif

#ifdef USE_STUB_emberChildId
EmberNodeId emberChildId(uint8_t index)
{
  return EMBER_NULL_NODE_ID;
}
#endif

#ifdef USE_STUB_emberChildIndex
uint8_t emberChildIndex(EmberNodeId childId)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emberChildLongIndex
uint8_t emberChildLongIndex(uint8_t *childLongId)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emberEnergyScanHandler
void emberEnergyScanHandler(uint8_t channel, int8_t rssi)
{
}
#endif

#ifdef USE_STUB_emComScanBuffer
Buffer emComScanBuffer = NULL_BUFFER;
#endif

#ifdef USE_STUB_emMgmtEnergyScanHandler
void emMgmtEnergyScanHandler(uint8_t channel, int8_t maxRssiValue)
{
}
#endif

#ifdef USE_STUB_emMgmtActiveScanHandler
void emMgmtActiveScanHandler(const EmberMacBeaconData *beaconData)
{
}
#endif

#ifdef USE_STUB_emMgmtEnergyScanComplete
void emMgmtEnergyScanComplete(void)
{
}
#endif

#ifdef USE_STUB_emMgmtActiveScanComplete
void emMgmtActiveScanComplete(void)
{
}
#endif

#ifdef USE_STUB_emberGetCcaThresholdReturn
void emberGetCcaThresholdReturn(int8_t threshold)
{
}
#endif

#ifdef USE_STUB_emberGetNetworkKeyInfoReturn
void emberGetNetworkKeyInfoReturn(EmberStatus status,
                                  uint32_t sequence,
                                  uint8_t state)
{
}
#endif

#ifdef USE_STUB_emberGetRadioPowerReturn
void emberGetRadioPowerReturn(int8_t power)
{
}
#endif

#ifdef USE_STUB_emberGetRipEntryReturn
void emberGetRipEntryReturn(uint8_t index, const EmberRipEntry *entry)
{
}
#endif

#ifdef USE_STUB_emberGetTxPowerModeReturn
void emberGetTxPowerModeReturn(uint16_t txPowerMode)
{
}
#endif

#ifdef USE_STUB_emberListenForWakeupReturn
void emberListenForWakeupReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberRadioNeedsCalibratingHandler
void emberRadioNeedsCalibratingHandler(void)
{
}
#endif

#ifdef USE_STUB_emberResetMicroHandler
void emberResetMicroHandler(EmberResetCause cause)
{
}
#endif

#ifdef USE_STUB_emberScanReturn
void emberScanReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberSetCcaThresholdReturn
void emberSetCcaThresholdReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberSetNextNetworkKeyReturn
void emberSetNextNetworkKeyReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberSetRadioPowerReturn
void emberSetRadioPowerReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberSetSecurityParametersReturn
void emberSetSecurityParametersReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberStartWakeupReturn
void emberStartWakeupReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberSwitchToNextNetworkKeyReturn
void emberSwitchToNextNetworkKeyReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberWakeupHandler
void emberWakeupHandler(EmberWakeupReason reason,
                        EmberWakeupState state,
                        uint16_t remainingMs,
                        uint8_t dataByte,
                        uint16_t otaSequence)
{
}
#endif

#ifdef USE_STUB_emberSleepNoTimerReturn
void emApiSleepNoTimerReturn(void)
{
}
#endif

#ifdef USE_STUB_emberWakeupStateReturn
void emApiWakeupStateReturn(uint8_t wakeupState,
                            uint16_t wakeupSequenceNumber)
{
}
#endif

#ifdef USE_STUB_emberSetWakeupSequenceNumberReturn
void emberSetWakeupSequenceNumberReturn(void)
{
}
#endif

#ifdef USE_STUB_halSleep
void halSleep(SleepModes sleepMode)
{
}
#endif

#ifdef USE_STUB_halSleepForMilliseconds
EmberStatus halSleepForMilliseconds(uint32_t *duration)
{
  return EMBER_SUCCESS;
}
#endif

#ifdef USE_STUB_simulatedTimePasses
void simulatedTimePasses(void)
{
}
#endif

#ifdef USE_STUB_simulatedTimePassesUs
void simulatedTimePassesUs(uint32_t delayUs)
{
}
#endif

#ifdef USE_STUB_simulatedTimePassesMs
void simulatedTimePassesMs(uint32_t delayMs)
{
}
#endif

#ifdef USE_STUB_simulatorId
uint16_t simulatorId = 0;
#endif

#ifdef USE_STUB_emAddUnicastTableEntry
void emAddUnicastTableEntry(EmberNodeId source, uint8_t sequence)
{
}
#endif

#ifdef USE_STUB_emAddUdpConnection
UdpConnectionData *emAddUdpConnection
  (const uint8_t *remoteAddress,
  uint16_t localPort,
  uint16_t remotePort,
  uint16_t flags,
  uint16_t recordSize,
  EmberUdpConnectionStatusHandler statusHandler,
  EmberUdpConnectionReadHandler readHandler)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emFindConnection
UdpConnectionData *emFindConnection(const uint8_t *remoteAddress,
                                    uint16_t localPort,
                                    uint16_t remotePort)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emLookupHandle
void *emLookupHandle(uint8_t handle, bool remove)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emUdpProxyRxHandler
void emUdpProxyRxHandler(const Ipv6Header* ipHeader)
{
}
#endif

#ifdef USE_STUB_emUdpSendDtlsRecord
void emUdpSendDtlsRecord(EmberUdpConnectionHandle handle, Buffer tlsRecord)
{
}
#endif

#ifdef USE_STUB_emAdvanceFragmentPointer
bool emAdvanceFragmentPointer(PacketHeader packet)
{
  return false;
}
#endif

#ifdef USE_STUB_emChildIsFrameCounterValid
bool emChildIsFrameCounterValid(uint32_t frameCounter,
                                const uint8_t *longId,
                                uint32_t keySequence,
                                MessageKeyType keyType)
{
  return false;
}
#endif

#ifdef USE_STUB_emConvert6lowpanToInternal
bool emConvert6lowpanToInternal(PacketHeader header,
                                uint8_t *headerLength)
{
  return true;
}
#endif

#ifdef USE_STUB_MeshHeader
typedef struct {
  uint8_t hopLimit;
  EmberNodeId source;
  EmberNodeId destination;
} MeshHeader;
#endif

#ifdef USE_STUB_emReadMeshHeader
bool emReadMeshHeader(PacketHeader header, MeshHeader *mesh)
{
  return false;
}
#endif

#ifdef USE_STUB_emDecrementMeshHeaderHopLimit
void emDecrementMeshHeaderHopLimit(PacketHeader header)
{
}
#endif

#ifdef USE_STUB_emIsInUnicastTable
bool emIsInUnicastTable(EmberNodeId source, uint8_t sequence)
{
  return false;
}
#endif

#ifdef USE_STUB_emMacMicLength
uint8_t emMacMicLength(void)
{
  return 4;
}
#endif

#ifdef USE_STUB_emMoreFragments
bool emMoreFragments(PacketHeader packet)
{
  return false;
}
#endif

#ifdef USE_STUB_emNeighborIsFrameCounterValid
bool emNeighborIsFrameCounterValid(uint32_t frameCounter,
                                   const uint8_t *longId,
                                   bool updateFrameCounter)
{
  return false;
}
#endif

#ifdef USE_STUB_emJoinIsParentFrameCounterValid
bool emJoinIsFrameCounterValid(uint32_t frameCounter,
                               const uint8_t *longId,
                               uint32_t keySequence,
                               MessageKeyType keyType)
{
  return false;
}
#endif

#ifdef USE_STUB_emPhySetEui64
void emPhySetEui64(void)
{
}
#endif

#ifdef USE_STUB_emPhyToMacQueueItemAddedCallback
void emPhyToMacQueueItemAddedCallback(void)
{
}
#endif

#ifdef USE_STUB_emScanReturn
void emScanReturn(EmberStatus status, uint8_t scanType, uint32_t txFailureMask)
{
  assert(false);
}
#endif

#ifdef USE_STUB_emWakeupKeepIncomingPacketIsr
bool emWakeupKeepIncomingPacketIsr(uint8_t *rawPacket)
{
  return true;
}
#endif

#ifdef USE_STUB_emMacTransmitCompleteCallback
void emMacTransmitCompleteCallback(PacketHeader header, EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emSerialTransmitCallback
void (*emSerialTransmitCallback)(uint8_t type, PacketHeader header) = NULL;
#endif

#ifdef USE_STUB_emThreadTestLogCallback
void (*emThreadTestLogCallback)(ThreadTestLogType type, ThreadTestLogData * data) = NULL;
#endif

#ifdef USE_STUB_emMakeNextFragment
uint8_t emMakeNextFragment(PacketHeader header,
                           uint8_t *txBuffer,
                           uint8_t maxLength,
                           uint8_t *destinationLoc)
{
  return 0;
}
#endif

#ifdef USE_STUB_emMakeStackJitMessage
PacketHeader emMakeStackJitMessage(void)
{
  return EMBER_NULL_MESSAGE_BUFFER;
}
#endif

#ifdef USE_STUB_emProcessWakeupMessage
bool emProcessWakeupMessage(PacketHeader header)
{
  return false;
}
#endif

#ifdef USE_STUB_emSetAllSleepyChildFlags
bool emSetAllSleepyChildFlags(uint16_t mask, bool set)
{
  return false;
}
#endif

#ifdef USE_STUB_emWakeupMessageFrameCounter
uint32_t emWakeupMessageFrameCounter(void)
{
  return 0;
}
#endif

#ifdef USE_STUB_emGetFdConnection
void *emGetFdConnection(uint8_t fd)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emPaaAddressNotification
void emPaaAddressNotification(uint32_t sessionId, uint8_t const *newAddress)
{
}
#endif

#ifdef USE_STUB_emRemoveConnection
void emRemoveConnection(void *connection)
{
}
#endif

#ifdef USE_STUB_emStartConnectionTimerMs
void emStartConnectionTimerMs(uint16_t delayMs)
{
}
#endif

#ifdef USE_STUB_emStartConnectionTimerQs
void emStartConnectionTimerQs(void *connection, uint16_t delayQs)
{
}
#endif

#ifdef USE_STUB_emberTcpClose
EmberStatus emberTcpClose(uint8_t fd)
{
  return EMBER_ERR_FATAL;
}
#endif

#ifdef USE_STUB_emberTcpRemoteIpAddress
EmberIpv6Address *emberTcpRemoteIpAddress(uint8_t fd)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emberPollForDataReturn
void emberPollForDataReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emLocalIslandId
uint8_t dummyIslandId[ISLAND_ID_SIZE] = { 0 };
uint8_t *emLocalIslandId(void)
{
  return dummyIslandId;
}
#endif

#ifdef USE_STUB_dataHandler
void dataHandler(const uint8_t *packet, SerialLinkMessageType type, uint16_t length)
{
}
#endif

#ifdef USE_STUB_emMakeIpHeader
PacketHeader emMakeIpHeader(Ipv6Header *ipHeader,
                            uint8_t tag,
                            uint8_t options,
                            const uint8_t *destination,
                            uint8_t hopLimit,
                            uint16_t payloadBufferLength)
{
  return NULL_BUFFER;
}
#endif

#ifdef USE_STUB_emPosixRead
ssize_t emPosixRead(int fd, void *buf, size_t count)
{
  return 0;
}
#endif

#ifdef USE_STUB_emPosixWrite
ssize_t emPosixWrite(int fd, const void *buf, size_t count)
{
  return 0;
}
#endif

#ifdef USE_STUB_emberConfigureDefaultHostAddress
void emberConfigureDefaultHostAddress(const EmberIpv6Address *mlAddress)
{
}
#endif

#ifdef USE_STUB_emberConfigureLegacyHostAddress
void emberConfigureLegacyHostAddress(const EmberIpv6Address *address)
{
}
#endif

#ifdef USE_STUB_emberConfigureGlobalHostAddress
void emberConfigureGlobalHostAddress(const EmberIpv6Address *address,
                                     uint32_t preferredLifetime,
                                     uint32_t validLifetime,
                                     uint8_t addressFlags)
{
}
#endif

#ifdef USE_STUB_emberRemoveHostAddress
void emberRemoveHostAddress(const EmberIpv6Address *address)
{
}
#endif

#ifdef USE_STUB_emberRemoveAllHostAddresses
void emberRemoveAllHostAddresses(void)
{
}
#endif

#ifdef USE_STUB_emberAssertInfoReturn
void emberAssertInfoReturn(const uint8_t *fileName, uint32_t lineNumber)
{
}
#endif

#ifdef USE_STUB_emberConfigUartReturn
void emberConfigUartReturn(void)
{
}
#endif

#ifdef USE_STUB_emberConfigureGatewayReturn
void emberConfigureGatewayReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberConfigureExternalRouteReturn
void emberConfigureExternalRouteReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberSetNdDataReturn
void emberSetNdDataReturn(EmberStatus status, uint16_t length)
{
}
#endif

#ifdef USE_STUB_emberSetLocalNetworkDataReturn
void emberSetLocalNetworkDataReturn(EmberStatus status, uint16_t length)
{
}
#endif

#ifdef USE_STUB_emberEchoReturn
void emberEchoReturn(const uint8_t *data, uint8_t length)
{
}
#endif

#ifdef USE_STUB_emberGetChannelCalDataTokenReturn
void emberGetChannelCalDataTokenReturn(uint8_t lna,
                                       int8_t tempAtLna,
                                       uint8_t modDac,
                                       int8_t tempAtModDac)
{
}
#endif

#ifdef USE_STUB_emberGetCounterReturn
void emberGetCounterReturn(EmberCounterType type, uint16_t value)
{
}
#endif

#ifdef USE_STUB_emberGetMulticastEntryReturn
void emberGetMulticastEntryReturn(uint8_t lastSequence,
                                  uint8_t windowBitmask,
                                  uint8_t dwellQs,
                                  const uint8_t *seed)
{
}
#endif

#ifdef USE_STUB_emberGetVersionsReturn
void emberGetVersionsReturn(const uint8_t *versionName,
                            uint16_t binaryManagementVersionNumber,
                            uint16_t stackVersionNumber,
                            uint16_t stackBuildNumber,
                            EmberVersionType versionType,
                            const uint8_t *buildTimestamp)
{
}
#endif

#ifdef USE_STUB_emberIncomingIcmpHandler
void emberIncomingIcmpHandler(Ipv6Header *ipHeader)
{
}
#endif

#ifdef USE_STUB_emberNcpUdpStormReturn
void emberNcpUdpStormReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberNcpUdpStormCompleteHandler
void emberNcpUdpStormCompleteHandler(void)
{
}
#endif

#ifdef USE_STUB_emberResetNcpAshReturn
void emberResetNcpAshReturn(void)
{
}
#endif

#ifdef USE_STUB_emberResetNetworkStateReturn
void emberResetNetworkStateReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberSerialReadByte
EmberStatus emberSerialReadByte(uint8_t port, uint8_t *dataByte)
{
  return EMBER_SUCCESS;
}
#endif

#ifdef USE_STUB_sendDebugMessage
void sendDebugMessage(void)
{
}
#endif

#ifdef USE_STUB_emberSetTxPowerModeReturn
void emberSetTxPowerModeReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberStartUartStormReturn
void emberStartUartStormReturn(void)
{
}
#endif

#ifdef USE_STUB_emberStopUartStormReturn
void emberStopUartStormReturn(void)
{
}
#endif

#ifdef USE_STUB_logEnabled
bool logEnabled = true;
#endif

#ifdef USE_STUB_managementHandler
void managementHandler(SerialLinkMessageType type,
                       const uint8_t *message,
                       uint16_t length)
{
}
#endif

#ifdef USE_STUB_emFindNearestGateway
EmberNodeId emFindNearestGateway(const uint8_t *ipDestination,
                                 const uint8_t *ipSource)
{
  return EMBER_NULL_NODE_ID;
}
#endif

#ifdef USE_STUB_emFindNeighborByLongId
uint16_t emFindNeighborByLongId(const uint8_t *longId)
{
  return 0xFFFE;
}
#endif

#ifdef USE_STUB_emPrepareRetriedMleMessage
PacketHeader emPrepareRetriedMleMessage(PacketHeader header)
{
  return NULL_BUFFER;
}
#endif

#ifdef USE_STUB_emProcessDuplicateAddressMessage
void emProcessDuplicateAddressMessage(PacketHeader header,
                                      Ipv6Header *ipHeader)
{
}
#endif

#ifdef USE_STUB_emProcessDhcpReply
void emProcessDhcpReply(DhcpMessage *message, uint8_t *ipSource)
{
}
#endif

#ifdef USE_STUB_emRipLookupNextHop
uint16_t emRipLookupNextHop(uint16_t shortMacId)
{
  return EMBER_NULL_NODE_ID;
}
#endif

#ifdef USE_STUB_emSyncNeighbor
void emSyncNeighbor(const uint8_t *destination)
{
}
#endif

#ifdef USE_STUB_emExpandSequenceNumber
uint32_t emExpandSequenceNumber(uint8_t littleSequence)
{
  return 0;
}
#endif

#ifdef USE_STUB_halInternalStartSymbolTimer
void halInternalStartSymbolTimer(void)
{
}
#endif

#ifdef USE_STUB_emButtonTick
void emButtonTick(void)
{
}
#endif

#ifdef USE_STUB_emAddLurkerNeighbor
uint8_t emAddLurkerNeighbor(const uint8_t *longId)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emRipLinkCost
uint8_t emRipLinkCost(uint8_t ripId)
{
  return 0;
}
#endif

#ifdef USE_STUB_emSendDhcpAddressRelease
bool emSendDhcpAddressRelease(uint16_t shortId, const uint8_t *longId)
{
  return false;
}
#endif

#ifdef USE_STUB_emberConfigureGateway
void emberConfigureGateway(uint16_t borderRouterFlags,
                           bool isStable,
                           const uint8_t *prefix,
                           const uint8_t prefixLengthInBits,
                           uint8_t domainId,
                           uint32_t preferredLifetime,
                           uint32_t validLifetime)
{
}
#endif

#ifdef USE_STUB_emGetBinaryCommandIdentifierString
const char *emGetBinaryCommandIdentifierString(uint16_t identifier)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emLookupChildEui64ById
uint8_t *emLookupChildEui64ById(EmberNodeId id)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emNextNwkFrameCounter
uint32_t emNextNwkFrameCounter = 0;
#endif

#ifdef USE_STUB_emSetNwkFrameCounter
void emSetNwkFrameCounter(uint32_t newFrameCounter)
{
}
#endif

#ifdef USE_STUB_emPrintRipTable
void emPrintRipTable(uint8_t port)
{
}
#endif

#ifdef USE_STUB_emberDeepSleep
void emberDeepSleep(bool sleep)
{
}
#endif

#ifdef USE_STUB_emberDeepSleepTick
bool emberDeepSleepTick(void)
{
  return false;
}
#endif

#ifdef USE_STUB_emberDeepSleepReturn
void emberDeepSleepReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberDeepSleepCompleteHandler
void emberDeepSleepCompleteHandler(uint16_t sleepDuration)
{
}
#endif

#ifdef USE_STUB_emberStackPollForDataReturn
void emberStackPollForDataReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberOkToNapReturn
void emberOkToNapReturn(uint8_t stateMask)
{
}
#endif

#ifdef USE_STUB_emberSendDoneReturn
void emberSendDoneReturn(void)
{
}
#endif

#ifdef USE_STUB_emProcessMleParentResponse
void emProcessMleParentResponse(PacketHeader header, MleMessage *message)
{
}
#endif

#ifdef USE_STUB_emAddParentLink
bool emAddParentLink(MleMessage *message,
                     EmberNodeId parentId,
                     int32u frameCounter)
{
  return false;
}
#endif

#ifdef USE_STUB_emClearAddressCache
void emClearAddressCache(void)
{
}
#endif

#ifdef USE_STUB_emClearAddressCacheId
void emClearAddressCacheId(EmberNodeId id)
{
}
#endif

#ifdef USE_STUB_emCurrentIpSource
uint8_t *emCurrentIpSource;
#endif

#ifdef USE_STUB_emCurrentMeshSource
EmberNodeId emCurrentMeshSource = 0xFFFE;
#endif

#ifdef USE_STUB_emMacDecrypt
bool emMacDecrypt(uint8_t *packet,
                  uint8_t *lengthLoc,
                  uint8_t *sourceEui64,
                  bool combineKeyWithNetworkFragmentIdentifier)
{
  return false;
}
#endif

#ifdef USE_STUB_printBytes
void printBytes(const uint8_t *bytes, uint16_t length)
{
}
#endif

#ifdef USE_STUB_emberSerialPrintBytes
EmberStatus emberSerialPrintBytes(uint8_t port,
                                  PGM_P prefix,
                                  uint8_t *bytes,
                                  uint16_t count)
{
  return EMBER_SUCCESS;
}
#endif

#ifdef USE_STUB_emRouterIndex
uint8_t emRouterIndex(uint8_t ripId)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emRouterOrLurkerIndexByLongId
uint8_t emRouterOrLurkerIndexByLongId(const uint8_t *longId, bool isLurker)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emRouterDowngradeThreshold
uint8_t emRouterDowngradeThreshold;
#endif

#ifdef USE_STUB_emEndDeviceRebootSuccess
void emEndDeviceRebootSuccess(void)
{
}
#endif

#ifdef USE_STUB_emEndDeviceRebootFailed
void emEndDeviceRebootFailed(void)
{
}
#endif

#ifdef USE_STUB_emStartNewIsland
void emStartNewIsland(void)
{
}
#endif

#ifdef USE_STUB_printIpAddress
void printIpAddress(const uint8_t *address)
{
}
#endif

#ifdef USE_STUB_emberUartSpeedTestReturn
void emberUartSpeedTestReturn(uint32_t totalBytesSent,
                              uint32_t bytesSent,
                              uint32_t testLengthMs)
{
}
#endif

#ifdef USE_STUB_emParentLongId
uint8_t emParentLongId[8];
#endif

#ifdef USE_STUB_emParentCostToLeader
uint8_t emParentCostToLeader = 0xFF;
#endif

#ifdef USE_STUB_emCompareIslandIds
int8_t emCompareIslandIds(const uint8_t *first,
                          bool firstIsSingleton,
                          const uint8_t *second,
                          bool secondIsSingleton)
{
  return 0;
}
#endif

#ifdef USE_STUB_emIsCandidateIslandId
bool emIsCandidateIslandId(const uint8_t *otherIslandId,
                           bool otherIslandSize,
                           uint8_t sequence)
{
  return false;
}
#endif

#ifdef USE_STUB_emJoinBetterIsland
void emJoinBetterIsland(void)
{
}
#endif

#ifdef USE_STUB_emForceParentId
uint16_t emForceParentId = 0xFFFF;
#endif

#ifdef USE_STUB_emForceParentLongId
uint8_t emForceParentLongId[8];
#endif

#ifdef USE_STUB_emSetPendingToActiveEvent
void emSetPendingToActiveEvent(uint32_t delay)
{
}
#endif

#ifdef USE_STUB_emSendNetworkFragmentMleAdvertisement
void emSendNetworkFragmentMleAdvertisement(bool retry)
{
}
#endif

#ifdef USE_STUB_emResetChildsTimer
void emResetChildsTimer(uint8_t index)
{
}
#endif

#ifdef USE_STUB_emberResetSerialState
void emberResetSerialState(void)
{
}
#endif

#ifdef USE_STUB_emResetNetworkState
void emResetNetworkState(void)
{
}
#endif

#ifdef USE_STUB_emIsLocalIpAddress
bool emIsLocalIpAddress(const uint8_t *address)
{
  return false;
}
#endif

#ifdef USE_STUB_emStoreDefaultGlobalPrefix
bool emStoreDefaultGlobalPrefix(uint8_t *target)
{
  return true;
}
#endif

#ifdef USE_STUB_emEccSign
bool emEccSign(const uint8_t *hash,
               const EccPrivateKey *key,
               uint8_t *r,
               uint8_t *s)
{
  return false;
}
#endif

#ifdef USE_STUB_emMleInit
void emMleInit(void)
{
}
#endif

#ifdef USE_STUB_emGenerateEcdh
bool emGenerateEcdh(uint8_t **publicKeyLoc, uint8_t *secret)
{
  return false;
}
#endif

#ifdef USE_STUB_emEccVerify
bool emEccVerify(const uint8_t *hash,
                 EccPublicKey *key,
                 const uint8_t *r,
                 uint16_t rLength,
                 const uint8_t *s,
                 uint16_t sLength)
{
  return false;
}
#endif

#ifdef USE_STUB_emEcdhSharedSecret
bool emEcdhSharedSecret(const EccPublicKey *remotePublicKey,
                        const uint8_t *localSecret,
                        uint8_t *sharedSecret)
{
  return false;
}
#endif

#ifdef USE_STUB_emStoreLongFe8Address
void emStoreLongFe8Address(const uint8_t *eui64, uint8_t *target)
{
}
#endif

#ifdef USE_STUB_emberRunBinaryCommandInterpreter
bool emberRunBinaryCommandInterpreter(EmberCommandState *state,
                                      EmberCommandEntry *commandTable,
                                      EmberCommandErrorHandler *errorHandler,
                                      const uint8_t *input,
                                      uint16_t length)
{
  return true;
}
#endif

#ifdef USE_STUB_emberRunAsciiCommandInterpreter
bool emberRunAsciiCommandInterpreter(EmberCommandState *state,
                                     EmberCommandEntry *commandTable,
                                     EmberCommandErrorHandler *errorHandler,
                                     const uint8_t *input,
                                     uint16_t length)
{
  return true;
}
#endif

#ifdef USE_STUB_driverNcpFd
uint8_t driverNcpFd;
#endif

#ifdef USE_STUB_ipModemLinkMarkBuffers
void ipModemLinkMarkBuffers(void)
{
}
#endif

#ifdef USE_STUB_emGetNcpStream
Stream *emGetNcpStream(void)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emProcessCommissionerProxyMessage
void emProcessCommissionerProxyMessage(Buffer buffer,
                                       uint8_t *payload,
                                       uint16_t length)
{
}
#endif

#ifdef USE_STUB_emGetShortAddress
NextHopResult emGetShortAddress(uint8_t context,
                                const uint8_t *iid,
                                EmberNodeId *resultLoc,
                                bool discover)
{
  return DROP;
}
#endif

#ifdef USE_STUB_emSetNetworkKeySequence
void emSetNetworkKeySequence(uint32_t amount)
{
}
#endif

#ifdef USE_STUB_emScheduleKeyRotation
void emScheduleKeyRotation(void)
{
}
#endif

#ifdef USE_STUB_emberActiveWakeupReturn
void emberActiveWakeupReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emMaybeClearRoute
void emMaybeClearRoute(uint16_t destId, uint16_t nextHop)
{
}
#endif

#ifdef USE_STUB_emJitTransmitComplete
bool emJitTransmitComplete(Buffer header, EmberStatus status)
{
  return false;
}
#endif

#ifdef USE_STUB_emMacRetries
uint8_t emMacRetries;
#endif

#ifdef USE_STUB_txBufferFullHandler
void txBufferFullHandler(const uint8_t *packet,
                         uint16_t packetLength,
                         uint16_t written)
{
}
#endif

#ifdef USE_STUB_emBufferUsage
void emBufferUsage(const char *tag)
{
}
#endif

#ifdef USE_STUB_emEndBufferUsage
void emEndBufferUsage(void)
{
}
#endif

#ifdef USE_STUB_txFailedHandler
void txFailedHandler(uint8_t fd,
                     const uint8_t *packet,
                     uint16_t packetLength,
                     uint16_t written)
{
}
#endif

#ifdef USE_STUB_emNoteSuccessfulPoll
void emNoteSuccessfulPoll(void)
{
}
#endif

#ifdef USE_STUB_emBecomeLeader
void emBecomeLeader(void)
{
}
#endif

#ifdef USE_STUB_emInitializeCommissioningData
void emInitializeCommissioningData(void)
{
}
#endif

#ifdef USE_STUB_emRetryTransmitComplete
bool emRetryTransmitComplete(PacketHeader header, EmberStatus status)
{
  return false;
}
#endif

#ifdef USE_STUB_emUdpJoinSrcPort
uint16_t emUdpJoinSrcPort;
#endif

#ifdef USE_STUB_emUdpJoinPort
uint16_t emUdpJoinPort;
#endif

#ifdef USE_STUB_emUdpCommissionPort
uint16_t emUdpCommissionPort;
#endif

#ifdef USE_STUB_emUdpTransmitComplete
void emUdpTransmitComplete(PacketHeader header, EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emFindNeighborIndexByLongId
uint8_t emFindNeighborIndexByLongId(const uint8_t *longId)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emClearNeighbor
void emClearNeighbor(uint16_t shortId, bool preserveMultihopRoute)
{
}
#endif

#ifdef USE_STUB_emRadioSetIdleMode
EmberStatus emRadioSetIdleMode(uint8_t mode)
{
  return EMBER_SUCCESS;
}
#endif

#ifdef USE_STUB_emIsAddressManagementPost
bool emIsAddressManagementPost(Ipv6Header *ipHeader)
{
  return false;
}
#endif

#ifdef USE_STUB_emSendAddressQuery
EmberStatus emSendAddressQuery(const EmberIpv6Address *address)
{
  return EMBER_ERR_FATAL;
}
#endif

#ifdef USE_STUB_emSendAddressSolicitOrRelease
bool emSendAddressSolicitOrRelease(bool isSolicit, AddressManagementStatus status)
{
  return false;
}
#endif

#ifdef USE_STUB_emProcessCommissioningData
void emProcessCommissioningData(PacketHeader header, Ipv6Header *ipHeader)
{
}
#endif

#ifdef USE_STUB_emProcessCommissioningMessage
void emProcessCommissioningMessage(PacketHeader header)
{
}
#endif

#ifdef USE_STUB_emGetCommissionKey
uint8_t emGetCommissionKey(uint8_t *key)
{
  return 0;
}
#endif

#ifdef USE_STUB_emSetCommissioningMacKey
void emSetCommissioningMacKey(uint8_t *key)
{
}
#endif

#ifdef USE_STUB_emCommissioningHandshakeComplete
void emCommissioningHandshakeComplete(void)
{
}
#endif

#ifdef USE_STUB_emCommissionInit
void emCommissionInit(void)
{
}
#endif

#ifdef USE_STUB_emComputeEui64Hash
void emComputeEui64Hash(const EmberEui64 *input, EmberEui64 *output)
{
}
#endif

#ifdef USE_STUB_emSetCommissionState
void emSetCommissionState(const uint8_t *networkDataTlv)
{
}
#endif

#ifdef USE_STUB_emSetLinkLocalIdentifier
void emSetLinkLocalIdentifier(const uint8_t *extendedId)
{
}
#endif

#ifdef USE_STUB_emCoapCommissionRequestHandler
void emCoapCommissionRequestHandler(EmberCoapCode code,
                                    uint8_t *uri,
                                    EmberCoapReadOptions *options,
                                    const uint8_t *payload,
                                    uint16_t payloadLength,
                                    const EmberCoapRequestInfo *info)
{
}
#endif

#ifdef USE_STUB_makeMessage
Parcel *makeMessage(char *format, ...)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emCoapMarkBuffers
void emCoapMarkBuffers(void)
{
}
#endif

#ifdef USE_STUB_emCommissioner
Buffer emCommissioner;
#endif

#ifdef USE_STUB_emResetRipTable
void emResetRipTable(bool keepParent)
{
}
#endif

#ifdef USE_STUB_emStoreGp16
bool emStoreGp16(uint16_t id, uint8_t *target)
{
  return false;
}
#endif

#ifdef USE_STUB_setRandomDataType
void setRandomDataType(bool isClient)
{
}
#endif

#ifdef USE_STUB_emSubmitCoapMessage
EmberStatus emSubmitCoapMessage(CoapMessage *message,
                                const uint8_t *uri,
                                Buffer payloadBuffer)
{
  return EMBER_ERR_FATAL;
}
#endif

#ifdef USE_STUB_emberCoapRequestHandler
void emberCoapRequestHandler(EmberCoapCode code,
                             uint8_t *uri,
                             EmberCoapReadOptions *options,
                             const uint8_t *payload,
                             uint16_t payloadLength,
                             const EmberCoapRequestInfo *info)
{
}
#endif

#ifdef USE_STUB_emInitStackMl16CoapMessage
void emInitStackMl16CoapMessage(CoapMessage *message,
                                EmberNodeId destination,
                                uint8_t *payload,
                                uint16_t payloadLength)
{
}
#endif

#ifdef USE_STUB_assertRxBuffersSize
void assertRxBuffersSize(uint8_t size)
{
}
#endif

#ifdef USE_STUB_emFf02AllNodesMulticastAddress
const Bytes16 emFf02AllNodesMulticastAddress =
{ { 0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 } };
#endif

#ifdef USE_STUB_emSequenceNumberIsNewer
bool emSequenceNumberIsNewer(uint32_t sequenceNumber)
{
  return false;
}
#endif

#ifdef USE_STUB_emJpakeEccGetCkxaOrSkxbData
bool emJpakeEccGetCkxaOrSkxbData(uint8_t *buf,
                                 uint16_t len,
                                 uint16_t *olen)
{
  return false;
}
#endif

#ifdef USE_STUB_emJpakeEccVerifyCkxaOrSkxbData
bool emJpakeEccVerifyCkxaOrSkxbData(uint8_t *buf,
                                    uint16_t len)
{
  return false;
}
#endif

#ifdef USE_STUB_emJpakeEccStart
void emJpakeEccStart(bool isClient,
                     const uint8_t *sharedSecret,
                     const uint8_t sharedSecretLength)
{
}
#endif

#ifdef USE_STUB_emMacKickStart
void emMacKickStart(void)
{
}
#endif

#ifdef USE_STUB_emRadioSetChannelAndForceCalibration
EmberStatus emRadioSetChannelAndForceCalibration(uint8_t channel)
{
  return EMBER_ERR_FATAL;
}
#endif

#ifdef USE_STUB_emMacSetChannel
bool emMacSetChannel(uint8_t channel)
{
  return false;
}
#endif

#ifdef USE_STUB_emVectorSearch
void *emVectorSearch(Vector *vector,
                     EqualityPredicate predicate,
                     const void *target,
                     uint16_t *indexLoc)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emVectorMatchCount
uint16_t emVectorMatchCount(Vector *vector,
                            EqualityPredicate predicate,
                            const void *target)
{
  return 0;
}
#endif

#ifdef USE_STUB_emVectorAdd
void *emVectorAdd(Vector *vector, uint16_t quanta)
{
  return NULL;
}
#endif

#ifdef USE_STUB_RadioPowerMode
enum {
  EMBER_RADIO_POWER_MODE_RX_ON,
  EMBER_RADIO_POWER_MODE_OFF
};
typedef uint8_t RadioPowerMode;
#endif

#ifdef USE_STUB_emRadioInit
void emRadioInit(RadioPowerMode initialRadioPowerMode)
{
}
#endif

#ifdef USE_STUB_emRadioGetIdleMode
RadioPowerMode emRadioGetIdleMode(void)
{
  return EMBER_RADIO_POWER_MODE_RX_ON;
}
#endif

#ifdef USE_STUB_emJpakeEccFinish
bool emJpakeEccFinish(uint8_t *pms,
                      uint16_t len,
                      uint16_t *olen)
{
  return false;
}
#endif

#ifdef USE_STUB_emJpakeEccGetHxaHxbData
bool emJpakeEccGetHxaHxbData(uint8_t *buf,
                             uint16_t len,
                             uint16_t *olen)
{
  return true;
}
#endif

#ifdef USE_STUB_emSha256Hash
void emSha256Hash(const uint8_t *input, int count, uint8_t *output)
{
}
#endif

#ifdef USE_STUB_emCoapHostIncomingMessageHandler
void emCoapHostIncomingMessageHandler(const uint8_t *bytes,
                                      uint16_t bytesLength,
                                      const EmberIpv6Address *localAddress,
                                      uint16_t localPort,
                                      const EmberIpv6Address *remoteAddress,
                                      uint16_t remotePort)
{
}
#endif

#ifdef USE_STUB_emCoapStackIncomingMessageHandler
void emCoapStackIncomingMessageHandler(PacketHeader header,
                                       Ipv6Header *ipHeader,
                                       void *transmitHandler,
                                       void *transmitHandlerData,
                                       void *handler)
{
}
#endif

#ifdef USE_STUB_emCoapRequestHandler
bool emCoapRequestHandler(EmberCoapCode code,
                          const uint8_t *uri,
                          EmberCoapReadOptions *options,
                          const uint8_t *payload,
                          uint16_t payloadLength,
                          const EmberCoapRequestInfo *info,
                          Buffer header)
{
  return false;
}
#endif

#ifdef USE_STUB_emberCoapRespondWithPath
EmberStatus emberCoapRespondWithPath(const EmberCoapRequestInfo *requestInfo,
                                     EmberCoapCode code,
                                     const uint8_t *path,
                                     const EmberCoapOption *options,
                                     uint8_t numberOfOptions,
                                     const uint8_t *payload,
                                     uint16_t payloadLength)
{
  return EMBER_SUCCESS;
}
#endif

#ifdef USE_STUB_emCoapRespondWithBuffer
EmberStatus emCoapRespondWithBuffer(const EmberCoapRequestInfo *requestInfo,
                                    EmberCoapCode code,
                                    const uint8_t *payload,
                                    uint16_t payloadLength,
                                    Buffer morePayload)
{
  return EMBER_SUCCESS;
}
#endif

#ifdef USE_STUB_emSetChildAddresses
uint8_t *emSetChildAddresses(uint8_t index,
                             const uint8_t *data,
                             uint8_t dataLength,
                             uint8_t *echoFinger)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emJpakeEccVerifyHxaHxbData
bool emJpakeEccVerifyHxaHxbData(uint8_t *buf,
                                uint16_t len)
{
  return false;
}
#endif

#ifdef USE_STUB_emMarkUdpBuffers
void emMarkUdpBuffers(void)
{
}
#endif

#ifdef USE_STUB_emGeneratePskc
bool emGeneratePskc(const uint8_t *extendedPanId,
                    const uint8_t *networkId,
                    uint8_t *pskc)
{
  return false;
}
#endif

#ifdef USE_STUB_emberSetPskcHandler
void emberSetPskcHandler(const uint8_t *pskc)
{
}
#endif

#ifdef USE_STUB_emCommissionerSetPskcTlv
bool emCommissionerSetPskcTlv(void)
{
  return false;
}
#endif

#ifdef USE_STUB_emDerivePskc
void emDerivePskc(const uint8_t *passphrase,
                  int16_t passphraseLen,
                  const uint8_t *extendedPanId,
                  const uint8_t *networkName,
                  uint8_t *result)
{
}
#endif

#ifdef USE_STUB_emDeriveMleAndMacKeys
uint8_t *emDeriveMleAndMacKeys(uint32_t sequence, uint8_t *keyBuffer)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emStoreLegacyUla
void emStoreLegacyUla(uint8_t *target)
{
}
#endif

#ifdef USE_STUB_emSetLegacyUla
void emSetLegacyUla(const uint8_t *prefix)
{
}
#endif

#ifdef USE_STUB_emCallMarkFunctions
void emCallMarkFunctions(void)
{
}
#endif

#ifdef USE_STUB_emStoreIpSourceAddress
bool emStoreIpSourceAddress(uint8_t *source, const uint8_t *destination)
{
  return false;
}
#endif

#ifdef USE_STUB_emAppCommissionCompleteHandler
void emAppCommissionCompleteHandler(bool success)
{
}
#endif

#ifdef USE_STUB_emCommissionDatasetInit
void emCommissionDatasetInit(void)
{
}
#endif

#ifdef USE_STUB_emNoteCommissionTimestamps
void emNoteCommissionTimestamps(const uint8_t *activeTimestampTlv,
                                const uint8_t *pendingTimestampTlv)
{
}
#endif

#ifdef USE_STUB_emCancelCommissioningKey
void emCancelCommissioningKey(void)
{
}
#endif

#ifdef USE_STUB_emberGetLocalIpAddress
bool emberGetLocalIpAddress(uint8_t index, EmberIpv6Address *address)
{
  return true;
}
#endif

#ifdef USE_STUB_emJoinCommissionCompleteHandler
void emJoinCommissionCompleteHandler(bool success)
{
}
#endif

#ifdef USE_STUB_emSetDefaultGlobalPrefix
void emSetDefaultGlobalPrefix(const uint8_t *suppliedPrefix)
{
}
#endif

#ifdef USE_STUB_emHandleCommissionDatasetPost
bool emHandleCommissionDatasetPost(const uint8_t *uri,
                                   const uint8_t *payload,
                                   uint16_t payloadLength,
                                   const EmberCoapRequestInfo *info)
{
  return false;
}
#endif

#ifdef USE_STUB_emDtlsStatusHandler
void emDtlsStatusHandler(DtlsConnection *connection)
{
}
#endif

#ifdef USE_STUB_emAddNetworkData
uint8_t *emAddNetworkData(uint8_t *finger, bool stableDataOnly)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emAddLeaderTlv
uint8_t *emAddLeaderTlv(uint8_t *start)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emAddConnectivityTlv
uint8_t *emAddConnectivityTlv(uint8_t *finger)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emOpenDtlsServer
void emOpenDtlsServer(DtlsConnection *connection)
{
}
#endif

#ifdef USE_STUB_emSubmitDtlsPayload
void emSubmitDtlsPayload(DtlsConnection *connection, Buffer payload)
{
}
#endif

#ifdef USE_STUB_emAddRoute64Tlv
uint8_t *emAddRoute64Tlv(uint8_t *finger, const uint8_t *ipDest)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emAddDefaultSecurityPolicyTlv
uint8_t *emAddDefaultSecurityPolicyTlv(uint8_t *finger)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emAddChannelMaskEntry
uint8_t *emAddChannelMaskEntry(uint8_t *finger,
                               uint8_t channelPage,
                               uint32_t channelMask)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emAddChannelMaskTlv
uint8_t *emAddChannelMaskTlv(uint8_t *finger,
                             uint8_t channelPage,
                             uint32_t channelMask)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emFetchChannelMask
uint32_t emFetchChannelMask(const uint8_t *channelMaskTlv)
{
  return 0;
}
#endif

#ifdef USE_STUB_emHasPendingDataset
bool emHasPendingDataset(void)
{
  return false;
}
#endif

#ifdef USE_STUB_emGetActiveDatasetTlvs
void emGetActiveDatasetTlvs(OperationalDatasetTlvs *tlvs)
{
}
#endif

#ifdef USE_STUB_emMaybeFinishDatasetShuffle
void emMaybeFinishDatasetShuffle(void)
{
}
#endif

#ifdef USE_STUB_emMergeTlvs
bool emMergeTlvs(OperationalDatasetTlvs *target,
                 const uint8_t *newTlvs,
                 const uint16_t newLength)
{
  return false;
}
#endif

#ifdef USE_STUB_emReadCommissioningDatasetTokens
void emReadCommissioningDatasetTokens(void)
{
}
#endif

#ifdef USE_STUB_emSetActiveDatasetTlvs
void emSetActiveDatasetTlvs(OperationalDatasetTlvs *tlvs)
{
}
#endif

#ifdef USE_STUB_emSetPendingDatasetTlvs
void emSetPendingDatasetTlvs(OperationalDatasetTlvs *tlvs)
{
}
#endif

#ifdef USE_STUB_emHasDatasetTlv
bool emHasDatasetTlv(OperationalDatasetTlvs *opTlvs, uint8_t tlvId)
{
  return false;
}
#endif

#ifdef USE_STUB_emAddInt16uTlv
uint8_t *emAddInt16uTlv(uint8_t *finger, uint8_t tlvType, uint16_t value)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emAddTlv
uint8_t *emAddTlv(uint8_t *finger, uint8_t type, const uint8_t *data, uint8_t length)
{
  return NULL;
}
#endif

#ifdef USE_STUB_MeshCopTlv
typedef enum {
  DATASET_CHANNEL_TLV                 = 0,
  DATASET_PAN_ID_TLV                  = 1,
  DATASET_EXTENDED_PAN_ID_TLV         = 2,
  DATASET_NETWORK_NAME_TLV            = 3,
  DATASET_PSKC_TLV                    = 4,
  DATASET_NETWORK_MASTER_KEY_TLV      = 5,
  DATASET_MESH_LOCAL_PREFIX_TLV       = 7,
  DATASET_SESSION_ID_TLV              = 11,
  DATASET_SECURITY_POLICY_TLV         = 12,
  DATASET_GET_TLV                     = 13,
  DATASET_ACTIVE_TIMESTAMP_TLV        = 14,
  DATASET_STATE_TLV                   = 16,
  DATASET_PENDING_TIMESTAMP_TLV       = 51,
  DATASET_DELAY_TIMER_TLV             = 52,
  DATASET_CHANNEL_MASK_TLV            = 53,
  DATASET_COUNT_TLV                   = 54,
  DATASET_PERIOD_TLV                  = 55,
  DATASET_SCAN_DURATION_TLV           = 56,
  DATASET_ENERGY_LIST_TLV             = 57
} MeshCopTlv;
#endif

#ifdef USE_STUB_emSendMleAnnounceFromActiveDataset
bool emSendMleAnnounceFromActiveDataset(uint8_t channel)
{
  return false;
}
#endif

#ifdef USE_STUB_emTimestampGreater
bool emTimestampGreater(const uint8_t *theirTimestampTlv,
                        const uint8_t *ourTimestampTlv,
                        bool active)
{
  return false;
}
#endif

#ifdef USE_STUB_emCacheParameters
void emCacheParameters(const EmberNetworkParameters *parameters,
                       uint16_t options)
{
}
#endif

#ifdef USE_STUB_emSetupNextJoinState
void emSetupNextJoinState(void)
{
}
#endif

#ifdef USE_STUB_emAddNeighbor
uint8_t emAddNeighbor(EmberNodeId shortId, const uint8_t *longId)
{
  return 0;
}
#endif

#ifdef USE_STUB_emHasSavedActiveDataset
bool emHasSavedActiveDataset(void)
{
  return false;
}
#endif

#ifdef USE_STUB_emSaveDataset
bool emSaveDataset(const uint8_t *payload,
                   uint16_t length,
                   const uint8_t *timestamp,
                   bool isActive)
{
  return false;
}
#endif

#ifdef USE_STUB_emGetPendingTimestamp
uint8_t *emGetPendingTimestamp(void)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emResetCommissionDatasetState
void emResetCommissionDatasetState(void)
{
}
#endif

#ifdef USE_STUB_emShuffleActiveDatasetToPending
void emShuffleActiveDatasetToPending(void)
{
}
#endif

#ifdef USE_STUB_emShufflePendingDatasetToActive
void emShufflePendingDatasetToActive(void)
{
}
#endif

#ifdef USE_STUB_emConnectivityBucketCounts
void emConnectivityBucketCounts(uint8_t *buckets)
{
}
#endif

#ifdef USE_STUB_emFragmentRepairScanHandler
void emFragmentRepairScanHandler(PacketHeader header, MleMessage *message)
{
}
#endif

#ifdef USE_STUB_emHaveLegacyUla
bool emHaveLegacyUla(void)
{
  return false;
}
#endif

#ifdef USE_STUB_emLeaderCost
uint8_t emLeaderCost(void)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emLinkMarginToQuality
uint8_t emLinkMarginToQuality(uint8_t linkMargin)
{
  return 0;
}
#endif

#ifdef USE_STUB_emNeighborSetFrameCounter
void emNeighborSetFrameCounter(uint8_t index,
                               uint32_t macFrameCounter,
                               uint32_t mleFrameCounter,
                               uint32_t keySequence,
                               bool isLegacy)
{
}
#endif

#ifdef USE_STUB_emNeighborWriteMleRecords
uint8_t emNeighborWriteMleRecords(uint8_t *message, const uint8_t *ipDest)
{
  return 0;
}
#endif

#ifdef USE_STUB_emNoteMleJoiner
bool emNoteMleJoiner(MleMessage *message, bool isLurker)
{
  return false;
}
#endif

#ifdef USE_STUB_emNoteMleSender
uint8_t emNoteMleSender(MleMessage *message)
{
  return 0;
}
#endif

#ifdef USE_STUB_emRssiToLinkMargin
uint8_t emRssiToLinkMargin(int8_t rssi)
{
  return 0;
}
#endif

#ifdef USE_STUB_TlvData
typedef const struct {
  uint16_t  offset;
  uint8_t   flagIndex;
  uint8_t   type;
  uint8_t   minLength;
  uint16_t  maxLength;
} TlvData;
#endif

#ifdef USE_STUB_emAddSessionIdTlv
uint8_t *emAddSessionIdTlv(uint8_t *finger)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emSetNetworkId
void emSetNetworkId(const uint8_t *networkId, uint8_t length)
{
}
#endif

#ifdef USE_STUB_emCommissionerSessionId
uint16_t emCommissionerSessionId;
#endif

#ifdef USE_STUB_emBorderRouterNodeId
EmberNodeId emBorderRouterNodeId = EMBER_NULL_NODE_ID;
#endif

#ifdef USE_STUB_TlvDataGetter
typedef TlvData *(*TlvDataGetter)(uint8_t tlvId);
#endif

#ifdef USE_STUB_emParseTlvMessage
bool emParseTlvMessage(void *message,
                       uint32_t *tlvMask,
                       const uint8_t *payload,
                       uint16_t length,
                       TlvDataGetter getTlvData)
{
  return false;
}
#endif

#ifdef USE_STUB_emCopyDatasetTlvs
bool emCopyDatasetTlvs(uint8_t **output,
                       const uint8_t *limit,
                       uint8_t options)
{
  return false;
}
#endif

#ifdef USE_STUB_emCopyFromLinkedBuffers
void emCopyFromLinkedBuffers(uint8_t *to, Buffer from, uint16_t count)
{
}
#endif

#ifdef USE_STUB_emSetIncomingMleState
void emSetIncomingMleState(uint8_t index, bool accept)
{
}
#endif

#ifdef USE_STUB_emSetExtendedPanId
void emSetExtendedPanId(const uint8_t* extendedPanId)
{
}
#endif

#ifdef USE_STUB_emberGetNetworkParameters
void emberGetNetworkParameters(EmberNetworkParameters *parameters)
{
}
#endif

#ifdef USE_STUB_emStartActiveDataset
void emStartActiveDataset(uint32_t timestamp)
{
}
#endif

#ifdef USE_STUB_emStartActiveDatasetFromParameters
void emStartActiveDatasetFromParameters(const EmberNetworkParameters *params,
                                        uint16_t options)
{
}
#endif

#ifdef USE_STUB_emAmShufflingDataset
bool emAmShufflingDataset(void)
{
  return false;
}
#endif

#ifdef USE_STUB_emberSendSteeringDataReturn
void emberSendSteeringDataReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberGetRoutingLocatorReturn
void emberGetRoutingLocatorReturn(const EmberIpv6Address *rloc)
{
}
#endif

#ifdef USE_STUB_emGetCoapCodeName
const uint8_t *emGetCoapCodeName(EmberCoapCode type)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emSendCommissionTimestamps
void emSendCommissionTimestamps(void)
{
}
#endif

#ifdef USE_STUB_emGetCoapTypeName
const uint8_t *emGetCoapTypeName(CoapType type)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emberReadIntegerOption
bool emberReadIntegerOption(EmberCoapReadOptions *options,
                            EmberCoapOptionType type,
                            uint32_t *valueLoc)
{
  return false;
}
#endif

#ifdef USE_STUB_emOldChannel
uint8_t emOldChannel;
#endif

#ifdef USE_STUB_emMacGetChannel
uint8_t emMacGetChannel(void)
{
  return 0;
}
#endif

#ifdef USE_STUB_emSetOldChannelForAttach
void emSetOldChannelForAttach(void)
{
}
#endif

#ifdef USE_STUB_emHandleAddressManagementPost
bool emHandleAddressManagementPost(const uint8_t *uri,
                                   const uint8_t *payload,
                                   uint16_t payloadLength,
                                   const EmberCoapRequestInfo *info)
{
  return false;
}
#endif

#ifdef USE_STUB_emHandleThreadDiagnosticMessage
bool emHandleThreadDiagnosticMessage(CoapMessage *coap,
                                     const uint8_t *convertedUri)
{
  return false;
}

#endif

#ifdef USE_STUB_ipAddressToString
void ipAddressToString(const uint8_t *address, uint8_t *buffer, uint8_t bufferLength)
{
  strcpy((char *)buffer, "0000:0000:0000:0000:0000:0000:0000:0000");
}
#endif

#ifdef USE_STUB_emberCustomNcpToHostMessageHandler
void emberCustomNcpToHostMessageHandler(const uint8_t *message, uint8_t messageLength)
{
}
#endif

#ifdef USE_STUB_emberCustomHostToNcpMessageHandler
void emberCustomHostToNcpMessageHandler(const uint8_t *message, uint8_t messageLength)
{
}
#endif

#ifdef USE_STUB_emberSetRandomizeMacExtendedIdReturn
void emberSetRandomizeMacExtendedIdReturn(void)
{
}
#endif

#ifdef USE_STUB_emberHostToNcpNoOp
void emberHostToNcpNoOp(const uint8_t *bytes, uint8_t bytesLength)
{
}
#endif

#ifdef USE_STUB_emAssignRipId
uint8_t emAssignRipId(uint8_t requestedRipId,
                      const uint8_t *longId,
                      AddressManagementStatus status)
{
  return 0xFF;
}
#endif

#ifdef USE_STUB_emberNcpToHostNoOp
void emberNcpToHostNoOp(const uint8_t *bytes, uint8_t bytesLength)
{
}
#endif

#ifdef USE_STUB_emberLeaderDataHandler
void emberLeaderDataHandler(const uint8_t *leaderData)
{
}
#endif

#ifdef USE_STUB_emGetCompleteNetworkData
bool emGetCompleteNetworkData(void)
{
  return true;
}
#endif

#ifdef USE_STUB_emberGetNetworkDataReturn
void emberGetNetworkDataReturn(EmberStatus status,
                               uint8_t *networkData,
                               uint16_t bufferLength)
{
}
#endif

#ifdef USE_STUB_emberGetNetworkDataTlvReturn
void emberGetNetworkDataTlvReturn(uint8_t typeByte,
                                  uint8_t index,
                                  uint8_t versionNumber,
                                  const uint8_t *tlv,
                                  uint8_t tlvLength)
{
}
#endif

#ifdef USE_STUB_emGetPhyRadioChannel
uint8_t emGetPhyRadioChannel(void)
{
  return 0;
}
#endif

#ifdef USE_STUB_emGetPrefixAddressEntry
GlobalAddressEntry *emGetPrefixAddressEntry(const uint8_t *prefix,
                                            uint8_t prefixLengthInBits,
                                            bool match)
{
  return NULL;
}
#endif

#ifdef USE_STUB_emHaveExternalRoute
bool emHaveExternalRoute(const uint8_t *prefix)
{
  return false;
}
#endif

#ifdef USE_STUB_emGenericQueueRemove
uint16_t emGenericQueueRemove(Buffer *queue, Buffer buffer, uint16_t i)
{
  return 0;
}
#endif

#ifdef USE_STUB_emDeleteGlobalAddressEntry
void emDeleteGlobalAddressEntry(const uint8_t *prefix, uint8_t prefixLengthInBits)
{
}
#endif

#ifdef USE_STUB_emDeleteGatewayAddressEntry
void emDeleteGatewayAddressEntry(const uint8_t *prefix, uint8_t prefixLengthInBits)
{
}
#endif

#ifdef USE_STUB_emberGetMfgTokenReturn
void emberGetMfgTokenReturn(EmberMfgTokenId tokenId,
                            EmberStatus status,
                            const uint8_t *tokenData,
                            uint8_t tokenDataLength)
{
}
#endif

#ifdef USE_STUB_emberSetMfgTokenReturn
void emberSetMfgTokenReturn(EmberMfgTokenId tokenId,
                            EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberGetStandaloneBootloaderInfoReturn
void emberGetStandaloneBootloaderInfoReturn(uint16_t version,
                                            uint8_t platformId,
                                            uint8_t microId,
                                            uint8_t phyId)
{
}
#endif

#ifdef USE_STUB_emberLaunchStandaloneBootloaderReturn
void emberLaunchStandaloneBootloaderReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberGetCtuneReturn
void emberGetCtuneReturn(uint16_t tune,
                         EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberSetCtuneReturn
void emberSetCtuneReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emBlacklistMyIslandId
void emBlacklistMyIslandId(void)
{
}
#endif

#ifdef USE_STUB_emNetworkIncomingMessageHandler
void emNetworkIncomingMessageHandler(PacketHeader header)
{
}
#endif

#ifdef USE_STUB_emberCoapSend
EmberStatus emberCoapSend(const EmberIpv6Address *destination,
                          EmberCoapCode code,
                          const uint8_t *path,
                          const uint8_t *payload,
                          uint16_t payloadLength,
                          EmberCoapResponseHandler responseHandler,
                          const EmberCoapSendInfo *info)
{
  return EMBER_SUCCESS;
}
#endif

#ifdef USE_STUB_emCoapDtlsTransmitHandler
bool emCoapDtlsTransmitHandler(const uint8_t *payload,
                               uint16_t payloadLength,
                               const EmberIpv6Address *localAddress,
                               uint16_t localPort,
                               const EmberIpv6Address *remoteAddress,
                               uint16_t remotePort,
                               void *transmitHandlerData)
{
  return true;
}
#endif

#ifdef USE_STUB_emJoinerEntrustTransmitHandler
bool emJoinerEntrustTransmitHandler(const uint8_t *payload,
                                    uint16_t payloadLength,
                                    const EmberIpv6Address *localAddress,
                                    uint16_t localPort,
                                    const EmberIpv6Address *remoteAddress,
                                    uint16_t remotePort,
                                    void *transmitHandlerData)
{
  return false;
}
#endif

#ifdef USE_STUB_emberSetRadioHoldOffReturn
void emberSetRadioHoldOffReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberGetPtaEnableReturn
void emberGetPtaEnableReturn(bool enabled)
{
}
#endif

#ifdef USE_STUB_emberSetPtaEnableReturn
void emberSetPtaEnableReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberGetPtaOptionsReturn
void emberGetPtaOptionsReturn(uint32_t options)
{
}
#endif

#ifdef USE_STUB_emberSetPtaOptionsReturn
void emberSetPtaOptionsReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberGetAntennaModeReturn
void emberGetAntennaModeReturn(EmberStatus status, uint8_t mode)
{
}
#endif

#ifdef USE_STUB_emberSetAntennaModeReturn
void emberSetAntennaModeReturn(EmberStatus status)
{
}
#endif

#ifdef USE_STUB_emberRadioGetRandomNumbersReturn
void emberRadioGetRandomNumbersReturn(EmberStatus status,
                                      const uint16_t *rn,
                                      uint8_t count)
{
}
#endif

#ifdef USE_STUB_emSendNewerManagementDataset
void emSendNewerManagementDataset(void)
{
}
#endif

#ifdef USE_STUB_emUpdateLeaderCommissioningDatasets
void emUpdateLeaderCommissioningDatasets(void)
{
}
#endif

#ifdef USE_STUB_emParseCoapMessage
bool emParseCoapMessage(const uint8_t *data,
                        uint16_t dataLength,
                        CoapMessage *coapMessage)
{
  return false;
}
#endif

#ifdef USE_STUB_emMicroBusy
void emMicroBusy(bool busy)
{
}
#endif
