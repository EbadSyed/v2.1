/***************************************************************************//**
 * @file main.c
 * @brief Wireless M-Bus Meter Application
 * @copyright Copyright 2017 Silicon Laboratories, Inc. http://www.silabs.com
 ******************************************************************************/
#include "rail.h"
#include "rail_types.h"
#include "hal_common.h"
#include "rail_config.h"

#include <stdlib.h>
#include <string.h>
#include "em_chip.h"
#include "em_emu.h"

#include "rtcdriver.h"
#include "bsp.h"

#include "wmbus/wmbus.h"

//address consts
const static char manuf[3] = { 'S', 'I', 'L' }; //FLAG assigned for Silicon Labs
static const WMBUS_deviceType_t deviceType = WMBUS_DEVICE_WATER_METER;
static const uint8_t version = 0x01;
static const uint32_t devId = 0x01; //BCD
static const uint8_t cryptoKey[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                     0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

//start the rx window earlier and close it later by this amount
static const uint32_t responseDelaySafetyMargin = 200;

static const WMBUS_Mode_t mode = WMBUS_MODE_S;

/*
 * Sync messages should be sent with an accurate delay. Encoding/CRCing a 255B
 * packet in T mode takes about 1ms, and loading the actual data and possibly
 * encoding takes some more. So when our RTC timer triggers, we schedule
 * transmission with some delay. During this delay, we must load
 * (and encrypt/encode) the payload.
 */
#define TIMEOUT_FOR_PACKET_LOAD_IN_US 5000

/*
 * 256 would be enough for an uncoded payload, but T mode does need 435B
 * However, EFR32 can handle 2 powers only.
 */
#define BUFFER_LENGTH 512

uint8_t gAccessNumber = 0;
uint8_t gTxBuffer[BUFFER_LENGTH];
RTCDRV_TimerID_t gWmbusPeriodicTimer;
const uint16_t gWmbusAppPeriodNom = 10e3; //10s
uint16_t gWmbusAppPeriodAcc = 500; //the first delay does not matter, so send it early
volatile uint32_t gLastEventTime;
WMBUS_accessibility_t gAccessibility = WMBUS_ACCESSIBILITY_LIMITED_ACCESS;
static RAIL_Handle_t railHandle;

typedef enum {
  S_UNINITED,
  S_STARTTX,
  S_TX,
  S_RESPONSE_DELAY,
  S_STARTRX,
  S_RX,
  S_SLEEP,
} state_t;

static volatile state_t state;

//callback protos
void wmbusPeriodicTimerCb(RTCDRV_TimerID_t id, void *user);
static void RAILCb_Generic(RAIL_Handle_t railHandle, RAIL_Events_t events);
static void RAILCb_RfReady(RAIL_Handle_t railHandle);

static RAIL_Config_t railCfg = {
  .eventsCallback = &RAILCb_Generic,
};

void initRadio()
{
  halInit();
  railHandle = RAIL_Init(&railCfg, &RAILCb_RfReady);

  RAIL_Idle(railHandle, RAIL_IDLE, true);

  RAIL_ConfigCal(railHandle, RAIL_CAL_ALL);

  RAIL_ConfigChannels(railHandle, channelConfigs[0], NULL);

  // Initialize the PA now that the HFXO is up and the timing is correct
  RAIL_TxPowerConfig_t txPowerConfig = {
#if HAL_PA_2P4_LOWPOWER
    .mode = RAIL_TX_POWER_MODE_2P4_LP,
#else
    .mode = RAIL_TX_POWER_MODE_2P4_HP,
#endif
    .voltage = HAL_PA_VOLTAGE,
    .rampTime = HAL_PA_RAMP,
  };
  if (channelConfigs[0]->configs[0].baseFrequency < 1000000UL) {
    // Use the Sub-GHz PA if required
    txPowerConfig.mode = RAIL_TX_POWER_MODE_SUBGIG;
  }
  if (RAIL_ConfigTxPower(railHandle, &txPowerConfig) != RAIL_STATUS_NO_ERROR) {
    // Error: The PA could not be initialized due to an improper configuration.
    // Please ensure your configuration is valid for the selected part.
    while (1) ;
  }
  RAIL_SetTxPower(railHandle, HAL_PA_POWER);

  RAIL_ConfigRxOptions(railHandle, RAIL_RX_OPTIONS_ALL, RAIL_RX_OPTIONS_DEFAULT);

  RAIL_ConfigEvents(railHandle, RAIL_EVENTS_ALL,
                    RAIL_EVENT_TX_PACKET_SENT | RAIL_EVENT_CAL_NEEDED | RAIL_EVENT_RX_PACKET_RECEIVED | RAIL_EVENT_RX_SCHEDULED_RX_END);
}

void setupDLLHeader(void *buffer, bool isFrameB, uint16_t frameLength, WMBUS_functionCode_t functionCode, bool primary, bool fcvDcf, bool fcbAcd)
{
  WMBUS_dll_header_t *dllHeader = (WMBUS_dll_header_t*)buffer;
  memset(dllHeader, 0, sizeof(WMBUS_dll_header_t));//clear the header
  dllHeader->cField.detailed.primary = primary;
  dllHeader->cField.detailed.fcbAcd = fcbAcd;
  dllHeader->cField.detailed.fcvDcf = fcvDcf;
  dllHeader->cField.detailed.functionCode = functionCode;
  dllHeader->address.detailed.manufacturer = WMBUSframe_Chars2MField(manuf);
  dllHeader->address.detailed.id = devId;
  dllHeader->address.detailed.version = version;
  dllHeader->address.detailed.deviceType = deviceType;

  frameLength -= 1; //l field shouldn't be counted
  if ( isFrameB ) {
    if ( frameLength < 125 ) {
      dllHeader->lField = frameLength + 2; //2block, 1 crc
    } else {
      dllHeader->lField = frameLength + 4; //3block, 2 crc
    }
  } else {
    dllHeader->lField = frameLength;
  }
}

uint16_t setupSTLHeader(void *buffer, uint8_t mode, uint8_t ciField, uint8_t accessNumber, WMBUS_accessibility_t accessibility, bool synchronized, void *payload, uint16_t payloadLength)
{
  WMBUS_stl_header_t *stlHeader = (WMBUS_stl_header_t*)buffer;
  memset(stlHeader, 0, sizeof(WMBUS_stl_header_t));//clear the header
  stlHeader->ciField = WMBUS_CI_EN13757_3_APPLICATION_SHORT;
  stlHeader->accessNumber = accessNumber;
  stlHeader->confWord.mode_0_5.mode = mode;
  stlHeader->confWord.mode_0_5.accessibility = accessibility;
  stlHeader->confWord.mode_0_5.synchronized = synchronized;
  if ( mode == 5 ) {
    uint8_t iv[16];
    uint16_t mf = WMBUSframe_Chars2MField(manuf);
    //with long transport layer header, the address from the header should be used
    memcpy(iv, &mf, 2);
    memcpy(iv + 2, &devId, 4);
    memcpy(iv + 6, &version, 1);
    memcpy(iv + 7, &deviceType, 1);
    memset(iv + 8, accessNumber, 8);
    stlHeader->confWord.mode_0_5.numOfEncBlocks = WMBUSframe_crypto5encrypt(payload, payload, iv, payloadLength, 0);
    return 16 * stlHeader->confWord.mode_0_5.numOfEncBlocks; //payloadlength probably changed due to padding
  } else {
    return payloadLength;
  }
}

uint16_t setupFrame(int32_t volume, int16_t volumeFlow, bool periodic, bool encrypt)
{
  uint8_t *payloadPtr = gTxBuffer + sizeof(WMBUS_dll_header_t) + sizeof(WMBUS_stl_header_t);
  uint8_t *stlPtr = gTxBuffer + sizeof(WMBUS_dll_header_t);
  uint8_t *dllPtr = gTxBuffer;

  //setup the payload before the headers, so we'll know the length
  uint16_t frameLength = 0;
  if ( encrypt ) { //2B AES check
    frameLength += WMBUSframe_addIdleFiller(payloadPtr + frameLength, 2);
  }
  //VIF 0x13 is volume in m^3
  frameLength += WMBUSframe_addPayloadI32(payloadPtr + frameLength, WMBUS_DIB_FUNCTION_INSTANTANEOUS, 0x13, volume);
  //VIF 0x3B is volume flow in m^3
  frameLength += WMBUSframe_addPayloadI16(payloadPtr + frameLength, WMBUS_DIB_FUNCTION_INSTANTANEOUS, 0x3B, volumeFlow);

  //continue with STL header, since encryption could change the length
  frameLength = setupSTLHeader(stlPtr, encrypt ? 5 : 0, WMBUS_CI_EN13757_3_APPLICATION_SHORT, gAccessNumber++,
                               gAccessibility, periodic, payloadPtr, frameLength);

  //add the header lengths to frameLnegth
  frameLength += sizeof(WMBUS_dll_header_t) + sizeof(WMBUS_stl_header_t);
  //finally, set up the DLL header
  setupDLLHeader(dllPtr, false, frameLength, WMBUS_FUNCTION_SND_NR, true, false, false);

  return frameLength;
}

void sendPacket(uint16_t length, uint32_t sendAt)
{
  uint16_t finalLength = WMBUS_phy_software(gTxBuffer, length, BUFFER_LENGTH);
  RAIL_SetTxFifo(railHandle, gTxBuffer, finalLength, BUFFER_LENGTH);
  if ( finalLength != length ) {
    RAIL_SetFixedLength(railHandle, finalLength);
    //this won't work for the first time, due to RAIL_LIB-1349
  }
  RAIL_ScheduleTxConfig_t schedule = {
    .mode = RAIL_TIME_ABSOLUTE,
    .when = sendAt,
  };
  RAIL_StartScheduledTx(railHandle, 0, 0, &schedule, NULL);
}

int main(void)
{
  CHIP_Init();
  state = S_UNINITED;
  initRadio();

  WMBUS_SetMode(mode);

  WMBUSframe_crypto5Init();
  WMBUSframe_crypto5SetKey(cryptoKey);
  gAccessNumber = rand() % 256;

  RTCDRV_Init();
  RTCDRV_AllocateTimer(&gWmbusPeriodicTimer);
  RTCDRV_StartTimer(gWmbusPeriodicTimer, rtcdrvTimerTypeOneshot, gWmbusAppPeriodAcc, wmbusPeriodicTimerCb, NULL);

  uint16_t length;
  while (1) {
    switch (state) {
      case S_STARTTX:
        length = setupFrame(12345, 123, true, true);
        RAIL_Idle(railHandle, RAIL_IDLE, true);
        sendPacket(length, gLastEventTime + TIMEOUT_FOR_PACKET_LOAD_IN_US);
        state = S_TX;
        break;
      case S_SLEEP:
      {
        //we don't use the clock sync to keep railtime
        bool canDeepSleep;
        RAIL_Sleep(0, &canDeepSleep);
        if ( canDeepSleep ) {
          EMU_EnterEM2(true);
          RAIL_Wake(0);
        }
      }
      break;
      case S_RESPONSE_DELAY:
      {
        RAIL_ScheduleRxConfig_t schedule = {
          .start = gLastEventTime + WMBUS_getMeterLimitedAccRxStart(false) - responseDelaySafetyMargin,
          .startMode = RAIL_TIME_ABSOLUTE,
          .end = gLastEventTime + WMBUS_getMeterLimitedAccRxStop(false) + responseDelaySafetyMargin,
          .endMode = RAIL_TIME_ABSOLUTE,
          .hardWindowEnd = 0,   //extend rx window if we're receiving
        };
        RAIL_Idle(railHandle, RAIL_IDLE, true);
        RAIL_ScheduleRx(railHandle, 0, &schedule, NULL);
        state = S_RX;
      }
      break;
      case S_STARTRX:
        RAIL_Idle(railHandle, RAIL_IDLE, true);
        RAIL_StartRx(railHandle, 0, NULL);
        state = S_RX;
        break;
      default:
        //in idle states, we need the main oscillator, but waiting for an interrupt. EM1 is safe
        EMU_EnterEM1();
        break;
    }
  }
}

void wmbusPeriodicTimerCb(RTCDRV_TimerID_t id, void *user)
{
  state = S_STARTTX;
  gLastEventTime = RAIL_GetTime();

  /*
   * from EN13757-4:
   * tACC=(1+(|nACC-128|-64)/2048*tNOM
   * a bit easier to calculate:
   * tACC=(2048+|nACC-128|-64)*tNOM/2048
   */

  gWmbusAppPeriodAcc = gAccessNumber > 128 ? gAccessNumber - 128 : 128 - gAccessNumber; //abs(gAccessNumber-128)
  gWmbusAppPeriodAcc = (2048 + gWmbusAppPeriodAcc - 64) * gWmbusAppPeriodNom / 2048;
  RTCDRV_StartTimer(gWmbusPeriodicTimer, rtcdrvTimerTypeOneshot, gWmbusAppPeriodAcc, wmbusPeriodicTimerCb, NULL);
}

static void RAILCb_Generic(RAIL_Handle_t railHandle, RAIL_Events_t events)
{
  //txpacketsent
  if ( events & RAIL_EVENT_TX_PACKET_SENT ) {
    RAIL_TxPacketDetails_t txPacketDetails;
    txPacketDetails.timeSent.timePosition = RAIL_PACKET_TIME_AT_PACKET_END;
    txPacketDetails.timeSent.totalPacketBytes = 0;
    txPacketDetails.isAck = false;
    RAIL_GetTxPacketDetails(railHandle, &txPacketDetails);
    gLastEventTime = txPacketDetails.timeSent.packetTime;

    BSP_LedToggle(0);

    switch (gAccessibility) {
      case WMBUS_ACCESSIBILITY_LIMITED_ACCESS:
        state = S_RESPONSE_DELAY;
        break;
      case WMBUS_ACCESSIBILITY_UNLIMITED_ACCESS:
        state = S_STARTRX;
        break;
      default:
        state = S_SLEEP;
        break;
    }
  }
  if ( events & RAIL_EVENT_CAL_NEEDED ) {
    RAIL_Calibrate(railHandle, (RAIL_CalValues_t*)channelConfigs[0]->configs[0].attr->calValues, RAIL_CAL_ALL_PENDING);
  }
  if ( (events & RAIL_EVENT_RX_SCHEDULED_RX_END) && (state == S_RX) ) {
    state = S_SLEEP;
  }
  if ( (events & RAIL_EVENT_RX_PACKET_RECEIVED) && (state == S_RX) ) {
    BSP_LedToggle(1);
    if ( gAccessibility == WMBUS_ACCESSIBILITY_LIMITED_ACCESS ) {
      state = S_STARTRX;
    } else {
      state = S_SLEEP;
    }
  }
}

static void RAILCb_RfReady(RAIL_Handle_t railHandle)
{
  if ( gAccessibility == WMBUS_ACCESSIBILITY_UNLIMITED_ACCESS ) {
    state = S_STARTRX;
  } else {
    state = S_SLEEP;
  }
}
