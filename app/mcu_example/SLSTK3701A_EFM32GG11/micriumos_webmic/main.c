/***************************************************************************//**
 * @file main.c
 * @brief Ethernet Microphone demo
 * @version 5.3.5
 *******************************************************************************
 * # License
 * <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/
#include <bsp_os.h>
#include <bsp_cpu.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <rtos/cpu/include/cpu.h>
#include <rtos/common/include/common.h>
#include <rtos/kernel/include/os.h>
#include <rtos/common/include/lib_def.h>
#include <rtos/common/include/rtos_utils.h>
#include <rtos/common/include/toolchains.h>
#include <rtos/net/source/http/server/http_server_priv.h>
#include <rtos/net/include/net_if.h>
#include <rtos/net/include/net_ipv4.h>

#include "net/network.h"
#include "net/core/net_core.h"
#include "net/http/http_server.h"

#include "bsp.h"
#include "glib.h"
#include "graphics.h"
#include "retargetserial.h"

#include "em_device.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_prs.h"
#include "em_usart.h"
#include "em_ldma.h"
#include "mic/mic_i2s.h"

#include "common_declarations.h"

#include "opus.h"
#include "ogg/ogg.h"

// -----------------------------------------------------------------------------
// Global variables

OS_MUTEX        audioMutex;

// Flag to be set when one of the mic sample buffers are ready to be used
OS_FLAG_GRP     bufferFlags;

// Flag to be set when audio is ready to be transfered to the clients
OS_FLAG_GRP     micFlags;

// Struct that contains oggpage
MIC_Data_Typedef micData;

// -----------------------------------------------------------------------------
// Defines

#define MAIN_START_TASK_PRIO         10u
#define MAIN_START_TASK_STK_SIZE     34 * 1024u

// -----------------------------------------------------------------------------
// Local Global Variables

// Start Task Stack.
static CPU_STK MainStartTaskStk[MAIN_START_TASK_STK_SIZE];

// Start Task TCB
static OS_TCB MainStartTaskTCB;

static uint16_t leftSampleBufOne[SAMPLE_BUFFER_LEN];
static uint16_t leftSampleBufTwo[SAMPLE_BUFFER_LEN];

static uint16_t rightSampleBufOne[SAMPLE_BUFFER_LEN];
static uint16_t rightSampleBufTwo[SAMPLE_BUFFER_LEN];

// -----------------------------------------------------------------------------
// Local Function Prototypes

static void MainStartTask(void  *p_arg);
static void micInit(void);
static int  ipv4Ready(void);
static OpusEncoder *createOpusEncoder(void);
static bool dmaCompleteCallback(unsigned int channel,
                                unsigned int sequenceNo,
                                void *userParam);
static void fillOggPacket(ogg_packet *packet, void *data, opus_int32 len,
                          ogg_int64_t granule, ogg_int64_t num);

// -----------------------------------------------------------------------------
// Configuration erros

#ifndef RTOS_MODULE_NET_AVAIL
#error "RTOS_MODULE_NET_AVAIL must be defined in rtos_description.h"
#endif

#ifndef RTOS_MODULE_NET_HTTP_SERVER_AVAIL
#error "RTOS_MODULE_NET_HTTP_SERVER_AVAIL must be defined in rtos_description.h"
#endif

// -----------------------------------------------------------------------------
// Global functions

/***************************************************************************//**
 * This is the standard entry point for C applications.
 * It is assumed that your code will call main() once you have performed
 * all necessary initialization.
 ******************************************************************************/
int main(void)
{
  RTOS_ERR  err;

  BSP_SystemInit(); // Initialize System

  // Select reference clock for High Freq. clock
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
  CMU_HFRCOBandSet(cmuHFRCOFreq_72M0Hz);

  BSP_CPUInit(); // Initialize CPU

  GRAPHICS_Init(); // Initialize Graphics

  RETARGET_SerialInit();
  RETARGET_SerialCrLf(1);
  printf("\n\n*** MicriumOS WebMic Example. ***\n");

  OS_TRACE_INIT();

  OSInit(&err); // Initialize the Kernel
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  OSMutexCreate(&audioMutex, // Create Audio Mutex
                "Audio Mutex",
                &err);

  OSFlagCreate(&micFlags, // Create New Mic Flag
               "New Mic Flag",
               (OS_FLAGS)0,
               &err);

  OSFlagCreate(&bufferFlags, // Create Buf ready flag
               "Mic Buffer Ready",
               (OS_FLAGS)0,
               &err);

  OSTaskCreate(&MainStartTaskTCB, // Create the Start Task
               "Main Start Task",
               MainStartTask,
               DEF_NULL,
               MAIN_START_TASK_PRIO,
               &MainStartTaskStk[0],
               (MAIN_START_TASK_STK_SIZE / 10u),
               MAIN_START_TASK_STK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);

  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  OSStart(&err); // Start the kernel
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  return (1);
}

// -----------------------------------------------------------------------------
// Local functions

/***************************************************************************//**
 * This is the task that will be called by the Startup when all services
 * are initializes successfully.
 *
 * @param p_arg: Argument passed from task creation. Unused, in this case.
 ******************************************************************************/
static void MainStartTask(void *p_arg)
{
  PP_UNUSED_PARAM(p_arg); // Prevent compiler warning
  RTOS_ERR  err;
  CPU_TS ts;
  OpusEncoder *encoder;
  ogg_stream_state streamState = { 0 };
  ogg_packet oggPacket = { 0 };
  ogg_page oggPage;
  unsigned char encData[BIT_RATE * TIME_MS / 1000 / 8];
  opus_int32 packetLen;
  ogg_int64_t granule = 0;
  ogg_int64_t packetNum = 2;
  uint16_t *p_txBuf = 0;

  BSP_TickInit(); // Initialize Kernel tick source

#if (OS_CFG_STAT_TASK_EN == DEF_ENABLED)
  // Initialize CPU Usage.
  OSStatTaskCPUUsageInit(&err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE),; );
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
  // Initialize interrupts disabled measurement.
  CPU_IntDisMeasMaxCurReset();
#endif

  Common_Init(&err); // micrium rtos init
  APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE,; );

  BSP_OS_Init();

  Ex_Net_CoreInit(); // Call Network core initialization example.
  Ex_Net_CoreStartIF(); // Call network interface start example.
  Ex_HTTP_Server_Init(); // Start the http Server
  Ex_HTTP_Server_InstanceCreateNoFS();

  // Application specific initialization
  micInit();
  encoder = createOpusEncoder();
  ogg_stream_init(&streamState, SERIALNO);
  printf("\n\n*** MicriumOS WebMic Example -----> Init done. ***\n");

  // Show IPv4
  GRAPHICS_ShowStatus();
  while (!ipv4Ready()) {
    OSTimeDly(100, OS_OPT_TIME_DLY, &err);
  }
  GRAPHICS_ShowStatus();

  // Start recording using mic's
  MIC_start();

  // Main LOOP
  while (DEF_ON) {
    // Wait for one of the LDMA to be ready
    OSFlagPend(&bufferFlags,
               READ_BUF_ONE | READ_BUF_TWO,
               (OS_TICK )0,
               (OS_OPT)OS_OPT_PEND_FLAG_SET_ANY,
               &ts,
               &err);

    if (bufferFlags.Flags & READ_BUF_ONE) {
      if (MIC_CH == MIC_CH1) {
        p_txBuf = leftSampleBufOne;
      } else {
        p_txBuf = leftSampleBufOne;
      }

      OSFlagPost(&bufferFlags, READ_BUF_ONE, (OS_OPT)OS_OPT_POST_FLAG_CLR, &err);
    } else if (bufferFlags.Flags & READ_BUF_TWO) {
      if (MIC_CH == MIC_CH1) {
        p_txBuf = leftSampleBufTwo;
      } else {
        p_txBuf = rightSampleBufTwo;
      }

      OSFlagPost(&bufferFlags, READ_BUF_TWO, (OS_OPT)OS_OPT_POST_FLAG_CLR, &err);
    } else {
      APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE),; );
    }

    packetLen = opus_encode(encoder,
                            (const int16_t *)p_txBuf,
                            SAMPLE_BUFFER_LEN,
                            encData,
                            sizeof(encData));

    fillOggPacket(&oggPacket, encData, packetLen, granule, packetNum++);
    ogg_stream_packetin(&streamState, &oggPacket);
    ogg_stream_flush(&streamState, &oggPage);

    // Get mutex of micData struct
    OSMutexPend(&audioMutex,
                0,
                OS_OPT_PEND_BLOCKING,
                (CPU_TS *)0,
                &err);

    micData.oggPage = &oggPage;
    micData.pcmBuf = p_txBuf;

    OSMutexPost(&audioMutex,
                OS_OPT_POST_NONE,
                &err);

    // Set new mic data flag
    OSFlagPost(&micFlags,
               (OS_FLAGS)0xFFFF,
               (OS_OPT)OS_OPT_POST_FLAG_SET,
               &err);

    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE),; );
  }
}

/***************************************************************************//**
 * Initialize microphones
 ******************************************************************************/
static void micInit(void)
{
  CMU_ClockEnable(cmuClock_GPIO, true); // Enable clocks

  GPIO_PinModeSet(MIC_ENABLE_PORT, MIC_ENABLE_PIN, gpioModePushPull, 1);

  MIC_init(dmaCompleteCallback,
           EX_SAMPLE_FREQ,
           leftSampleBufOne,
           leftSampleBufTwo,
           rightSampleBufOne,
           rightSampleBufTwo,
           SAMPLE_BUFFER_LEN);
}

/***************************************************************************//**
 * Check if ipv4 is Ready
 ******************************************************************************/
static int ipv4Ready(void)
{
  RTOS_ERR         err;
  NET_IF_NBR       ifnNbr;
  NET_IPv4_ADDR    addrTable[4];
  NET_IP_ADDRS_QTY addrTableSize = 4;

  ifnNbr = NetIF_NbrGetFromName("eth0");
  if (!NetIPv4_GetAddrHost(ifnNbr, addrTable, &addrTableSize, &err)) {
    return 0;
  }

  if (addrTableSize <= 0 || !addrTable[0]) {
    return 0;
  }

  return 1;
}

/***************************************************************************//**
 * Alloc and initialize an opus encoder
 *
 * @returns a ready to use opus encoder
 ******************************************************************************/
static OpusEncoder *createOpusEncoder(void)
{
  RTOS_ERR err;
  OpusEncoder *enc;

  enc = Mem_SegAlloc("opus encoder",
                     NULL,
                     opus_encoder_get_size(CHANNELS),
                     &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE),; );

  opus_encoder_init(enc,
                    EX_SAMPLE_FREQ,
                    CHANNELS,
                    OPUS_APPLICATION_AUDIO);

  // LOFI low delay options
  opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(0));
  opus_encoder_ctl(enc, OPUS_SET_BITRATE(BIT_RATE));
  opus_encoder_ctl(enc, OPUS_SET_PREDICTION_DISABLED(1));

  return enc;
}

/***************************************************************************//**
 * Alloc and initialize an opus encoder
 *
 * @param packet struct holding raw data to send in ogg stream
 * @param data pointer to the actual data
 * @param len size of data (in bytes)
 * @param granule opus sample position
 * @param num packets sequence number in stream
 ******************************************************************************/
static void fillOggPacket(ogg_packet *packet, void *data, opus_int32 len,
                          ogg_int64_t granule, ogg_int64_t num)
{
  packet->packet = data;
  packet->bytes = len;
  packet->b_o_s = 0;
  packet->e_o_s = 0;
  packet->granulepos = (granule += SAMPLE_BUFFER_LEN);
  packet->packetno = num;
}

/***************************************************************************//**
 * @brief
 *    Called when the DMA complete interrupt fired
 *
 * @param[in] channel
 *    DMA channel
 *
 * @param[in] sequenceNo
 *    Sequence number (number of times the callback has been called since
 *    the dma transfer was started
 *
 * @param[in] userParam
 *    User parameters
 *
 * @return
 *    Returns false to stop transfers
 ******************************************************************************/
static bool dmaCompleteCallback(unsigned int channel,
                                unsigned int sequenceNo,
                                void *userParam)
{
  RTOS_ERR  err;
  MIC_Context *context;

  context = (MIC_Context *) userParam;
  GPIO_PinOutToggle(CALLBACK_PORT, context->gpioPin);

  if (MIC_CH == channel) {
    if ((sequenceNo % 2) == 1) {
      OSFlagPost(&bufferFlags,
                 READ_BUF_TWO,
                 (OS_OPT)OS_OPT_POST_FLAG_CLR,
                 &err);

      OSFlagPost(&bufferFlags,
                 READ_BUF_ONE,
                 (OS_OPT)OS_OPT_POST_FLAG_SET,
                 &err);
    } else {
      OSFlagPost(&bufferFlags,
                 READ_BUF_ONE,
                 (OS_OPT)OS_OPT_POST_FLAG_CLR,
                 &err);

      OSFlagPost(&bufferFlags,
                 READ_BUF_TWO,
                 (OS_OPT)OS_OPT_POST_FLAG_SET,
                 &err);
    }
  }
  return true;
}
