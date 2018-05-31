/*
 * File: commission-dataset.h
 * Description: Thread Commissioning Dataset
 *
 * Copyright 2015 Silicon Laboratories, Inc.                                *80*
 */

#ifndef COMMISSION_DATASET_H
#define COMMISSION_DATASET_H

typedef enum {
  COMMISSION_DATASET_ACCEPT = 1,
  COMMISSION_DATASET_REJECT = 0xFF
} CommissionDatasetStatus;

typedef enum {
  DATASET_CHANNEL_TLV                 = 0,  // 0x00
  DATASET_PAN_ID_TLV                  = 1,  // 0x01
  DATASET_EXTENDED_PAN_ID_TLV         = 2,  // 0x02
  DATASET_NETWORK_NAME_TLV            = 3,  // 0x03
  DATASET_PSKC_TLV                    = 4,  // 0x04
  DATASET_NETWORK_MASTER_KEY_TLV      = 5,  // 0x05
  DATASET_MESH_LOCAL_PREFIX_TLV       = 7,  // 0x07
  DATASET_SESSION_ID_TLV              = 11, // 0x0B
  DATASET_SECURITY_POLICY_TLV         = 12, // 0x0C
  DATASET_GET_TLV                     = 13, // 0x0D
  DATASET_ACTIVE_TIMESTAMP_TLV        = 14, // 0x0E
  DATASET_STATE_TLV                   = 16, // 0x10
  DATASET_PENDING_TIMESTAMP_TLV       = 51, // 0x33
  DATASET_DELAY_TIMER_TLV             = 52, // 0x34
  DATASET_CHANNEL_MASK_TLV            = 53, // 0x35
  DATASET_COUNT_TLV                   = 54, // 0x36
  DATASET_PERIOD_TLV                  = 55, // 0x37
  DATASET_SCAN_DURATION_TLV           = 56, // 0x38
  DATASET_ENERGY_LIST_TLV             = 57, // 0x39
} MeshCopTlv;

#define MESH_LOCAL_PREFIX_TLV_LENGTH      10
#define SESSION_ID_TLV_LENGTH   4
#define CHANNEL_MASK_TLV_LENGTH 8
#define COUNT_TLV_LENGTH        3
#define PERIOD_TLV_LENGTH       4
#define CHANNEL_TLV_LENGTH      5
#define TIMESTAMP_TLV_LENGTH    10
#define PAN_ID_TLV_LENGTH       4
#define STATE_TLV_LENGTH        3
#define DELAY_TIMER_TLV_LENGTH  6

#define MANAGEMENT_ANNOUNCE_BEGIN_TLVS \
  (BIT(DATASET_CHANNEL_MASK_TLV_FLAG)  \
   | BIT(DATASET_SESSION_ID_TLV_FLAG)  \
   | BIT(DATASET_COUNT_TLV_FLAG)       \
   | BIT(DATASET_PERIOD_TLV_FLAG))

#define MANAGEMENT_ANNOUNCE_TLVS (BIT(DATASET_CHANNEL_TLV_FLAG)            \
                                  | BIT(DATASET_ACTIVE_TIMESTAMP_TLV_FLAG) \
                                  | BIT(DATASET_PAN_ID_TLV_FLAG))

#define MANAGEMENT_ACTIVE_GET_TLVS 0 // GET is optional
#define MANAGEMENT_ACTIVE_SET_TLVS BIT(DATASET_ACTIVE_TIMESTAMP_TLV_FLAG)

#define MANAGEMENT_PENDING_GET_TLVS 0 // GET is optional

// Commissioner Session ID TLV is optional
#define MANAGEMENT_PENDING_SET_TLVS (BIT(DATASET_PENDING_TIMESTAMP_TLV_FLAG)  \
                                     | BIT(DATASET_ACTIVE_TIMESTAMP_TLV_FLAG) \
                                     | BIT(DATASET_DELAY_TIMER_TLV_FLAG))

// From the Thread spec
#define MAX_OPERATIONAL_DATASET_SIZE 256

typedef struct {
  uint16_t length;
  uint8_t tlvs[MAX_OPERATIONAL_DATASET_SIZE];
} OperationalDatasetTlvs;

const CommissioningDataset *emGetActiveDataset(void);
void emSetActiveDataset(OperationalDatasetTlvs * new);

void emReadCommissioningDatasetTokens(void);
uint8_t *emGetPendingTimestamp(void);

EmberStatus emSendManagementAnnounceBegin(const EmberIpv6Address *remoteAddress,
                                          uint8_t channelPage,
                                          uint32_t channelMask,
                                          uint8_t count,
                                          uint16_t period);
bool emSendMleAnnounceFromActiveDataset(uint8_t channel);

bool emHandleCommissionDatasetPost(const uint8_t *uri,
                                   const uint8_t *payload,
                                   uint16_t payloadLength,
                                   const EmberCoapRequestInfo *info);

void emCommissionDatasetInit(void);

uint8_t *emAddChannelMaskEntry(uint8_t *finger,
                               uint8_t channelPage,
                               uint32_t channelMask);
uint8_t *emAddChannelMaskTlv(uint8_t *finger,
                             uint8_t channelPage,
                             uint32_t channelMask);
uint8_t *emAddDatasetChannelTlv(uint8_t *finger,
                                uint8_t channelPage,
                                uint16_t channel);
uint8_t *emAddDefaultSecurityPolicyTlv(uint8_t *finger);

// Minimum delay timer for a Pending operational dataset
// (as set by a commissioner & enforced by the leader):
#define DELAY_TIMER_MINIMAL_MS            30000       // 30s

// Default delay timer for a Pending operational dataset
// Also, the min value enforced by leader for a Master Key change
#define DELAY_TIMER_DEFAULT_MS            300000      // 5 mins

// Recommended minimum delay timer for a Pending Dataset
// set by a Commissioner when updating the Network Master Key
#define DELAY_TIMER_EXTENDED_MS           28800000UL  // 8 hours

#define MAX_DELAY_TIMER_MS 259200000UL

#define DEFAULT_SECURITY_POLICY_BITS 0xF8  // ONRCB bits
typedef enum {
  OUT_OF_BAND_MASTER_KEY = 0x80,
  NATIVE_COMMISSIONING   = 0x40,
  ROUTER_COMPATIBILITY   = 0x20,
  COMMISSIONING_EXTERNAL = 0x10,
  BEACONS_ENABLED        = 0x08,
} CommissionSecurityPolicyBits;

bool emTimestampGreater(const uint8_t *theirTimestampTlv,
                        const uint8_t *ourTimestampTlv,
                        bool active);

EmberStatus emSendManagementActiveSet(void);
EmberStatus emSendManagementPendingSet(void);

EmberStatus emSendManagementPendingGet(const EmberIpv6Address *destination,
                                       const uint8_t *tlvs,
                                       uint8_t tlvsLength);
EmberStatus emSendManagementActiveGet(const EmberIpv6Address *destination,
                                      const uint8_t *tlvs,
                                      uint8_t tlvsLength);
void emSendNewerManagementDataset(void);

void emNoteCommissionTimestamps(const uint8_t *activeTimestampTlv,
                                const uint8_t *pendingTimestampTlv);
void emSendCommissionTimestamps(void);

extern uint32_t emForceDelayTimerMs;

enum {
  ACTIVE_DATASET     = 0x01,    // there are separate active and pending flags
  PENDING_DATASET    = 0x02,    // to force callers to specify which is wanted
  INCLUDE_TIMESTAMP  = 0x04,
  INCLUDE_MASTER_KEY = 0x08
};

#define COMPLETE_ACTIVE_DATASET \
  (ACTIVE_DATASET | INCLUDE_TIMESTAMP | INCLUDE_MASTER_KEY)

#define COMPLETE_PENDING_DATASET \
  (PENDING_DATASET | INCLUDE_TIMESTAMP | INCLUDE_MASTER_KEY)

bool emCopyDatasetTlvs(uint8_t **output,
                       const uint8_t *outputLimit,
                       uint8_t options);

bool emUnmarshallDataset(CommissioningDataset *dataset,
                         const uint8_t *payload,
                         const uint8_t *limit);

bool emHasPendingDataset(void);

bool emSaveDataset(const uint8_t *payload,
                   uint16_t length,
                   const uint8_t *timestamp,
                   bool isActive);
void emClearDataset(bool isActive);
void emSendMleAnnounceOnChannel(uint8_t channel,
                                uint8_t count,
                                uint16_t period);

void emShufflePendingDatasetToActive(void);
void emShuffleActiveDatasetToPending(void);
bool emAmShufflingDataset(void);
void emMaybeFinishDatasetShuffle(void);

bool emGeneratePskc(const uint8_t *extendedPanId,
                    const uint8_t *networkId,
                    uint8_t *pskc);

// As a commissioner, generate the PSKc value and add it to the active dataset.
bool emCommissionerSetPskcTlv(void);

void emStartActiveDataset(uint32_t timestamp);
void emStartActiveDatasetFromParameters(const EmberNetworkParameters *params,
                                        uint16_t options);

// There is always an active dataset, there may or may not be a pending one,
// so the pending version returns a boolean.
void emGetActiveDatasetTlvs(OperationalDatasetTlvs *tlvs);
bool emGetPendingDatasetTlvs(OperationalDatasetTlvs *tlvs);

void emSetActiveDatasetTlvs(OperationalDatasetTlvs *tlvs);
void emSetPendingDatasetTlvs(OperationalDatasetTlvs *tlvs);

bool emHasDatasetTlv(OperationalDatasetTlvs *opTlvs, uint8_t tlvId);

// Adds 'newTlvs' to 'target', removing any existing TLVs of the same
// type as a new TLV.  Returns true if it all worked, false if the merged
// TLVs were too large to fit.

bool emMergeTlvs(OperationalDatasetTlvs *target,
                 const uint8_t *newTlvs,
                 const uint16_t newLength);

#endif // COMMISSION_DATASET_H
