// *******************************************************************
// * green-power-client.h
// *
// *
// * Copyright 2014 by Silicon Laboratories. All rights reserved.           *80*
// *******************************************************************

/* bookkeeping for Commissioning related info. */

typedef enum  {
  EMBER_GP_GPD_MAC_SEQ_NUM_CAP_SEQUENTIAL  = 0x00,
  EMBER_GP_GPD_MAC_SEQ_NUM_CAP_RANDOM      = 0x01,
} EmberGpGpdMacSeqNumCap;

typedef enum {
  EMBER_AF_GPC_COMMISSIONING_EXIT_ON_COMMISSIONING_WINDOW_EXP = 0x1,
  EMBER_AF_GPC_COMMISSIONING_EXIT_ON_FIRST_PAIRING_SUCCESS = 0x2,
  EMBER_AF_GPC_COMMISSIONING_EXIT_ON_GP_PROXY_COMMISSIONING_MODE_EXIT = 0x4,
  EMBER_AF_GPC_COMMISSIONING_EXIT_MODE_MAX = 0xFF,
} EmberAfGreenPowerClientCommissioningExitMode;

typedef struct {
  bool inCommissioningMode;
  EmberAfGreenPowerClientCommissioningExitMode exitMode;
  uint16_t gppCommissioningWindow;
  uint8_t channel;
  bool unicastCommunication;
  EmberNodeId commissioningSink;
  uint8_t onTransmitChannel;  //we're on a temp channel to send channel info, XXX overloaded to save network channel
} EmberAfGreenPowerClientCommissioningState;

typedef struct {
  EmberGpAddress addrs[EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_ADDR_ENTRIES];
  uint8_t randomSeqNums[EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_ADDR_ENTRIES][EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR];
  uint32_t expirationTimes[EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_ADDR_ENTRIES][EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR];
} EmberAfGreenPowerDuplicateFilter;

bool emGpMessageChecking(EmberGpAddress *gpAddr, uint8_t sequenceNumber);
