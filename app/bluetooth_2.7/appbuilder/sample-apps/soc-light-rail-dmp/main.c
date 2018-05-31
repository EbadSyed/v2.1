/**************************************************************************//**
 * Copyright 2017 Silicon Laboratories, Inc.
 *
 * @section License
 * LICENSE_PLACEHOLDER
 *****************************************************************************/
/**************************************************************************//**
 * @file   main.c
 * @brief  Implements the Light (GATT Server) Role, which enables a Switch
 *         device to connect to and interact with.  The device acts as a
 *         connection Peripheral. This is a Dynamic-Multiprotocol reference
 *         sample application, running on top of Micrium OS RTOS and
 *         multiprotocol RAIL.
 *****************************************************************************/
/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                            INCLUDE FILES
 *********************************************************************************************************
 *********************************************************************************************************
 */

#include "bsp/siliconlabs/generic/include/bsp_os.h"
#include "bsp/siliconlabs/generic/include/bsp_cpu.h"

#include <rtos/cpu/include/cpu.h>
#include <rtos/common/include/common.h>
#include <rtos/kernel/include/os.h>

#include <rtos/common/include/lib_def.h>
#include <rtos/common/include/rtos_utils.h>
#include <rtos/common/include/toolchains.h>
#include <rtos/common/include/rtos_prio.h>

#include "sleep.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "rtos_bluetooth.h"
//Bluetooth API definition
#include "rtos_gecko.h"
//GATT DB
#include "gatt_db.h"

/* Board Headers */
#include "hal-config.h"
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"

#include "bsphalconfig.h"
#include "bsp.h"

#include "em_cmu.h"
#include "gpiointerrupt.h"

#include "demo.h"

#include "rail.h"
#include "rail_types.h"
#include "rail_config.h"

#include "dmp-ui.h"

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                             LOCAL DEFINES
 *********************************************************************************************************
 *********************************************************************************************************
 */

#define APP_CFG_TASK_BLUETOOTH_LL_PRIO    3u
#define APP_CFG_TASK_BLUETOOTH_STACK_PRIO 4u

// Ex Main Start task
#define EX_MAIN_START_TASK_PRIO           21u
#define EX_MAIN_START_TASK_STK_SIZE       512u
static CPU_STK exMainStartTaskStk[EX_MAIN_START_TASK_STK_SIZE];
static OS_TCB  exMainStartTaskTCB;
static void    exMainStartTask(void *p_arg);

// Bluetooth Application task
#define BLUETOOTH_APP_TASK_PRIO           5u
#define BLUETOOTH_APP_TASK_STACK_SIZE     (1024 / sizeof(CPU_STK))
static CPU_STK bluetoothAppTaskStk[BLUETOOTH_APP_TASK_STACK_SIZE];
static OS_TCB  bluetoothAppTaskTCB;
static void    bluetoothAppTask(void *p_arg);

// Proprietary Application task
#define PROPRIETARY_APP_TASK_PRIO         6u
#define PROPRIETARY_APP_TASK_STACK_SIZE   (2000 / sizeof(CPU_STK))
static CPU_STK proprietaryAppTaskStk[PROPRIETARY_APP_TASK_STACK_SIZE];
static OS_TCB  proprietaryAppTaskTCB;
static void    proprietaryAppTask(void *p_arg);

// Demo Application task
#define DEMO_APP_TASK_PRIO                7u
#define DEMO_APP_TASK_STK_SIZE            512u
static CPU_STK App_TaskDemoStk[DEMO_APP_TASK_STK_SIZE];
static OS_TCB  demoAppTaskTCB;
static void    demoAppTask(void *p_arg);

#define  DEV_ID_STR_LEN                   9u

OS_Q  demoEventQueue;
OS_MUTEX lightMutex;
OS_TMR demoTimer;
OS_Q  railTransmitPacketQueue;

// Memory manager configuration
#define MAX_BUFFER_SIZE  (256)

// Minimum allowed size of the TX FIFO
#define RAIL_TX_FIFO_SIZE (64)

uint8_t receiveBuffer[MAX_BUFFER_SIZE];

static uint8_t txFifo[RAIL_TX_FIFO_SIZE];

// Byte index for Light Control
#define LIGHT_CONTROL_DATA_BYTE         (0x0F)
#define LIGHT_CONTROL_TOGGLE_BIT        (0x01)
#define LIGHT_CONTROL_ON_OFF_STATUS_BIT (0x02)
#define LIGHT_CONTROL_STATE_MASK        (0x03)
#define PACKET_HEADER_LEN (2)

static void proprietaryInit(void);
static void proprietaryTxPacket(bool lightState);
static void radioRxEventHandler(RAIL_Handle_t railHandle,
                                RAIL_Events_t events);

static void radioTxEventHandler(RAIL_Handle_t railHandle,
                                RAIL_Events_t events);

static bool bleAddConn(uint8_t handle, bd_addr* address);
static bool bleRemoveConn(uint8_t handle);
static bd_addr* bleGetAddr(uint8_t handle);
static void beaconAdvertisements(void);
static void enableBleAdvertisements(void);

static void idleHook(void);
static void setupHooks(void);

static void demoTimerCallback (void  *p_tmr, void  *p_arg);

static void appUiLedOff(void);
static void appUiLedOn(void);
static void GpioSetup(void);
static void GPIO_Button_IRQHandler(uint8_t pin);

static inline void lightPend(RTOS_ERR* err);
static inline void lightPost(RTOS_ERR* err);

static uint8_t dataPacket[] = {
  0x0F, 0x16, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
  0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0x00
};

static RAIL_Handle_t gRailHandleRx = NULL;
static RAIL_Handle_t gRailHandleTx = NULL;

// Create structure for RAIL reception
static RAILSched_Config_t railSchedStateRx;
static RAIL_Config_t railCfgRx = { // Must never be const
  .eventsCallback = &radioRxEventHandler,
  .protocol = NULL, // For BLE, pointer to a RAIL_BLE_State_t
  .scheduler = &railSchedStateRx,
};

// Create structure RAIL transmission
static RAILSched_Config_t railSchedStateTx;
static RAIL_Config_t railCfgTx = { // Must never be const
  .eventsCallback = &radioTxEventHandler,
  .protocol = NULL, // For BLE, pointer to a RAIL_BLE_State_t
  .scheduler = &railSchedStateTx,
};

#define railTransmitQueuedPacket(lightState, error) OSQPost( \
    (OS_Q *)&railTransmitPacketQueue,                        \
    (void *)(lightState),                                    \
    (OS_MSG_SIZE)sizeof(void *),                             \
    (OS_OPT)OS_OPT_POST_FIFO,                                \
    (RTOS_ERR *)&error                                       \
    )

/* Write response codes*/
#define ES_WRITE_OK                         0
#define ES_ERR_CCCD_CONF                    0x81
#define ES_ERR_PROC_IN_PROGRESS             0x80
#define ES_NO_CONNECTION                    0xFF
// Advertisement data
#define UINT16_TO_BYTES(n)        ((uint8_t) (n)), ((uint8_t)((n) >> 8))
#define UINT16_TO_BYTE0(n)        ((uint8_t) (n))
#define UINT16_TO_BYTE1(n)        ((uint8_t) ((n) >> 8))
// Ble TX test macros and functions
#define BLE_TX_TEST_DATA_SIZE   2
// We need to put the device name into a scan response packet,
// since it isn't included in the 'standard' beacons -
// I've included the flags, since certain apps seem to expect them
#define DEVNAME "DMP%02X%02X"
#define DEVNAME_LEN 8  // incl term null
#define UUID_LEN 16 // 128-bit UUID

#define IBEACON_MAJOR_NUM 0x0200 // 16-bit major number

#define LE_GAP_MAX_DISCOVERABLE_MODE   0x04
#define LE_GAP_MAX_CONNECTABLE_MODE    0x03
#define LE_GAP_MAX_DISCOVERY_MODE      0x02
#define BLE_INDICATION_TIMEOUT         30000
#define BUTTON_LONG_PRESS_TIME_MSEC    3000

#define LIGHT_STATE_GATTDB     gattdb_light_state_rail
#define TRIGGER_SOURCE_GATTDB  gattdb_trigger_source_rail
#define SOURCE_ADDRESS_GATTDB  gattdb_source_address_rail

#define BLUETOOTH_EVENT_FLAG_APP_INDICATE_START   ((OS_FLAGS)64)   //notify clients
#define BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS ((OS_FLAGS)128)  //notified clients
#define BLUETOOTH_EVENT_FLAG_APP_INDICATE_TIMEOUT ((OS_FLAGS)256)  //notify timeout
#define BLUETOOTH_EVENT_FLAG_APP_MASK             (BLUETOOTH_EVENT_FLAG_APP_INDICATE_START     \
                                                   + BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS \
                                                   + BLUETOOTH_EVENT_FLAG_APP_INDICATE_TIMEOUT)

// Timer Frequency used.
#define TIMER_CLK_FREQ ((uint32)32768)
// Convert msec to timer ticks.
#define TIMER_MS_2_TIMERTICK(ms) ((TIMER_CLK_FREQ * ms) / 1000)
#define TIMER_S_2_TIMERTICK(s) (TIMER_CLK_FREQ * s)

typedef enum {
  BLE_STATE_READY                   = 0,
  BLE_STATE_INDICATE_LIGHT          = 1,
  BLE_STATE_INDICATE_TRIGGER_SOURCE = 2,
  BLE_STATE_INDICATE_SOURCE_ADDRESS = 3
} BleState;

typedef struct {
  uint8_t handle;
  bd_addr address;
  bool inUse;
  uint8_t indicated;
} BleConn;

// iBeacon structure and data
static struct {
  uint8_t flagsLen;     /* Length of the Flags field. */
  uint8_t flagsType;    /* Type of the Flags field. */
  uint8_t flags;        /* Flags field. */
  uint8_t mandataLen;   /* Length of the Manufacturer Data field. */
  uint8_t mandataType;  /* Type of the Manufacturer Data field. */
  uint8_t compId[2];    /* Company ID field. */
  uint8_t beacType[2];  /* Beacon Type field. */
  uint8_t uuid[16];     /* 128-bit Universally Unique Identifier (UUID). The UUID is an identifier for the company using the beacon*/
  uint8_t majNum[2];    /* Beacon major number. Used to group related beacons. */
  uint8_t minNum[2];    /* Beacon minor number. Used to specify individual beacons within a group.*/
  uint8_t txPower;      /* The Beacon's measured RSSI at 1 meter distance in dBm. See the iBeacon specification for measurement guidelines. */
}
iBeaconData
  = {
  /* Flag bits - See Bluetooth 4.0 Core Specification , Volume 3, Appendix C, 18.1 for more details on flags. */
  2,  /* length  */
  0x01, /* type */
  0x04 | 0x02, /* Flags: LE General Discoverable Mode, BR/EDR is disabled. */

  /* Manufacturer specific data */
  26,  /* length of field*/
  0xFF, /* type of field */

  /* The first two data octets shall contain a company identifier code from
   * the Assigned Numbers - Company Identifiers document */
  { UINT16_TO_BYTES(0x004C) },

  /* Beacon type */
  /* 0x0215 is iBeacon */
  { UINT16_TO_BYTE1(0x0215), UINT16_TO_BYTE0(0x0215) },

  /* 128 bit / 16 byte UUID - generated specially for the DMP Demo */
  { 0x00, 0x47, 0xe7, 0x0a, 0x5d, 0xc1, 0x47, 0x25, 0x87, 0x99, 0x83, 0x05, 0x44, 0xae, 0x04, 0xf6 },

  /* Beacon major number */
  { UINT16_TO_BYTE1(IBEACON_MAJOR_NUM), UINT16_TO_BYTE0(IBEACON_MAJOR_NUM) },

  /* Beacon minor number  - not used for this application*/
  { UINT16_TO_BYTE1(0), UINT16_TO_BYTE0(0) },

  /* The Beacon's measured RSSI at 1 meter distance in dBm */
  /* 0xC3 is -61dBm */
  // TBD: check?
  0xC3
  };

static struct {
  uint8_t flagsLen;          /**< Length of the Flags field. */
  uint8_t flagsType;         /**< Type of the Flags field. */
  uint8_t flags;             /**< Flags field. */
  uint8_t serLen;            /**< Length of Complete list of 16-bit Service UUIDs. */
  uint8_t serType;           /**< Complete list of 16-bit Service UUIDs. */
  uint8_t serviceList[2];    /**< Complete list of 16-bit Service UUIDs. */
  uint8_t serDataLength;     /**< Length of Service Data. */
  uint8_t serDataType;       /**< Type of Service Data. */
  uint8_t uuid[2];           /**< 16-bit Eddystone UUID. */
  uint8_t frameType;         /**< Frame type. */
  uint8_t txPower;           /**< The Beacon's measured RSSI at 0 meter distance in dBm. */
  uint8_t urlPrefix;         /**< URL prefix type. */
  uint8_t url[10];           /**< URL. */
} eddystone_data = {
  /* Flag bits - See Bluetooth 4.0 Core Specification , Volume 3, Appendix C, 18.1 for more details on flags. */
  2,  /* length  */
  0x01, /* type */
  0x04 | 0x02, /* Flags: LE General Discoverable Mode, BR/EDR is disabled. */
  /* Service field length */
  0x03,
  /* Service field type */
  0x03,
  /* 16-bit Eddystone UUID */
  { UINT16_TO_BYTES(0xFEAA) },
  /* Eddystone-TLM Frame length */
  0x10,
  /* Service Data data type value */
  0x16,
  /* 16-bit Eddystone UUID */
  { UINT16_TO_BYTES(0xFEAA) },
  /* Eddystone-URL Frame type */
  0x10,
  /* Tx power */
  0x00,
  /* URL prefix - standard */
  0x00,
  /* URL */
  { 's', 'i', 'l', 'a', 'b', 's', '.', 'c', 'o', 'm' }
};

struct responseData_t{
  uint8_t flagsLen;          /**< Length of the Flags field. */
  uint8_t flagsType;         /**< Type of the Flags field. */
  uint8_t flags;             /**< Flags field. */
  uint8_t shortNameLen;      /**< Length of Shortened Local Name. */
  uint8_t shortNameType;     /**< Shortened Local Name. */
  uint8_t shortName[DEVNAME_LEN]; /**< Shortened Local Name. */
  uint8_t uuidLength;        /**< Length of UUID. */
  uint8_t uuidType;          /**< Type of UUID. */
  uint8_t uuid[UUID_LEN];    /**< 128-bit UUID. */
};

static struct responseData_t responseData = {
  2,  /* length (incl type) */
  0x01, /* type */
  0x04 | 0x02, /* Flags: LE General Discoverable Mode, BR/EDR is disabled. */
  DEVNAME_LEN + 1,        // length of local name (incl type)
  0x08,               // shortened local name
  { 'D', 'M', '0', '0', ':', '0', '0' },
  UUID_LEN + 1,           // length of UUID data (incl type)
  0x06,               // incomplete list of service UUID's
  // custom service UUID for silabs light in little-endian format
  { 0x13, 0x87, 0x37, 0x25, 0x42, 0xb0, 0xc3, 0xbf, 0x78, 0x40, 0x83, 0xb5, 0xe4, 0x96, 0xf5, 0x63 }
};

// Bluetooth stack configuration
#define MAX_CONNECTIONS 1
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];
/* Gecko configuration parameters (see gecko_configuration.h) */
static const gecko_configuration_t bluetooth_config =
{
  .config_flags = GECKO_CONFIG_FLAG_RTOS,
  .sleep.flags = 0,
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap),
  .gattdb = &bg_gattdb_data,
  .scheduler_callback = BluetoothLLCallback,
  .stack_schedule_callback = BluetoothUpdate,
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
  .pa.config_enable = 1, // Enable high power PA
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
  .mbedtls.flags = GECKO_MBEDTLS_FLAGS_NO_MBEDTLS_DEVICE_INIT,
  .mbedtls.dev_number = 0,
};

static BleConn bleConn[MAX_CONNECTIONS];

#ifndef FEATURE_IOEXPANDER
/* Periodically called Display Polarity Inverter Function for the LCD.
   Toggles the the EXTCOMIN signal of the Sharp memory LCD panel, which prevents building up a DC
   bias according to the LCD's datasheet */
static void (*dispPolarityInvert)(void *);
#endif /* FEATURE_IOEXPANDER */

#define DEMO_CONTROL_PAYLOAD_CMD_DATA                  (0x0F)
#define DEMO_CONTROL_PAYLOAD_SRC_ROLE_BIT              (0x80)
#define DEMO_CONTROL_PAYLOAD_SRC_ROLE_BIT_SHIFT        (7)
#define DEMO_CONTROL_PAYLOAD_CMD_MASK                  (0x70)
#define DEMO_CONTROL_PAYLOAD_CMD_MASK_SHIFT            (4)

typedef enum {
  DEMO_CONTROL_CMD_LIGHT_TOGGLE = 1,
  DEMO_CONTROL_CMD_LIGHT_STATE_REPORT = 2,
  DEMO_CONTROL_CMD_LIGHT_STATE_GET = 3,
} DemoControlCommandType;

typedef enum {
  DEMO_CONTROL_ROLE_LIGHT = 0,
  DEMO_CONTROL_ROLE_SWITCH = 1,
} DemoControlRole;

typedef enum {
  demoLightOff = 0,
  demoLightOn  = 1
} DemoLight;

typedef enum {
  demoLightDirectionBluetooth   = 0,
  demoLightDirectionProprietary = 1,
  demoLightDirectionButton      = 2,
  demoLightDirectionInvalid     = 3
} DemoLightDirection;

typedef struct {
  uint8_t addr[8];
} DemoLightSrcAddr;

typedef enum {
  demoTmrDispPolInv = 0x01,
  demoTmrIndicate   = 0x02,
  demoTmrClearDirection = 0x03
} DemoTmr;

typedef enum {
  DEMO_STATE_INIT       = 0x00,
  DEMO_STATE_READY      = 0x01
} DemoState;

typedef struct {
  DemoState state;
  DemoLight light;
  enum gatt_client_config_flag lightInd;
  DemoLightDirection direction;
  enum gatt_client_config_flag directionInd;
  DemoLightSrcAddr srcAddr;
  enum gatt_client_config_flag srcAddrInd;
  uint8_t connBluetoothInUse;
  uint8_t connProprietaryInUse;
  bool indicationOngoing;
  bool indicationPending;
  DemoLightSrcAddr ownAddr;
} Demo;

static Demo demo = {
  .state = DEMO_STATE_INIT,
  .light = demoLightOff,
  .lightInd = gatt_disable,
  .direction = demoLightDirectionButton,
  .directionInd = gatt_disable,
  .srcAddr = { { 0, 0, 0, 0, 0, 0, 0, 0 } },
  .srcAddrInd = gatt_disable,
  .connBluetoothInUse = 0,
  .connProprietaryInUse = 0,
  .indicationOngoing = false,
  .indicationPending = false,
  .ownAddr = { { 0, 0, 0, 0, 0, 0, 0, 0 } }
};

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                          GLOBAL FUNCTIONS
 *********************************************************************************************************
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *                                                main()
 *
 * Description : This is the standard entry point for C applications. It is assumed that your code will
 *               call main() once you have performed all necessary initialization.
 *
 * Argument(s) : None.
 *
 * Return(s)   : None.
 *
 * Note(s)     : None.
 *********************************************************************************************************
 */
int main(void)
{
  RTOS_ERR err;

  // Initialize device
  initMcu();
  // Initialize board
  initBoard();
  // Initialize application
  initApp();
  // Initialize CPU and make all interrupts Kernel Aware.
  BSP_CPUInit();
  // Setup a 1024 Hz tick instead of the default 1000 Hz. This improves accuracy when using dynamic
  // tick which runs of the RTCC at 32768 Hz.
  OS_TASK_CFG tickTaskCfg = {
    .StkBasePtr = DEF_NULL,
    .StkSize    = 256u,
    .Prio       = KERNEL_TICK_TASK_PRIO_DFLT,
    .RateHz     = 1024u
  };
  OS_ConfigureTickTask(&tickTaskCfg);
  // Initialize the Kernel.
  OSInit(&err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  // Initialize Kernel tick source.
  BSP_TickInit();
  // Setup the Micrium OS hooks
  setupHooks();
  // Initialise Geckonen (Bluetooth) stack
  gecko_init(&bluetooth_config);
  gecko_init_multiprotocol(NULL);
  // Start Bluetooth Link Layer and stack tasks
  bluetooth_start_task(APP_CFG_TASK_BLUETOOTH_LL_PRIO, APP_CFG_TASK_BLUETOOTH_STACK_PRIO);
  // Create the Ex Main Start task
  OSTaskCreate(&exMainStartTaskTCB,
               "Ex Main Start Task",
               exMainStartTask,
               DEF_NULL,
               EX_MAIN_START_TASK_PRIO,
               &exMainStartTaskStk[0],
               (EX_MAIN_START_TASK_STK_SIZE / 10u),
               EX_MAIN_START_TASK_STK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  // Start the kernel.
  OSStart(&err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  return (1);
}

/***************************************************************************//**
 * @brief
 *   This is the idle hook.
 *
 * @detail
 *   This will be called by the Micrium OS idle task when there is no other
 *   task ready to run. We just enter the lowest possible energy mode.
 ******************************************************************************/
void SleepAndSyncProtimer();
static void idleHook(void)
{
  // Put MCU in the lowest sleep mode available, usually EM2.
  SleepAndSyncProtimer();
}

/***************************************************************************//**
 * @brief
 *   Setup the Micrium OS hooks. We are only using the idle task hook in this
 *   example. See the Mcirium OS documentation for more information on the
 *   other available hooks.
 ******************************************************************************/
static void setupHooks(void)
{
  CPU_SR_ALLOC();

  CPU_CRITICAL_ENTER();
  // Don't allow EM3, since we use LF clocks.
  SLEEP_SleepBlockBegin(sleepEM3);
  OS_AppIdleTaskHookPtr = idleHook;
  CPU_CRITICAL_EXIT();
}

/*
 *********************************************************************************************************
 *                                          Ex_MainStartTask()
 *
 * Description : This is the task that will be called by the Startup when all services are initializes
 *               successfully.
 *
 * Argument(s) : p_arg   Argument passed from task creation. Unused, in this case.
 *
 * Return(s)   : None.
 *
 * Notes       : None.
 *********************************************************************************************************
 */
static void exMainStartTask(void *p_arg)
{
  PP_UNUSED_PARAM(p_arg);
  RTOS_ERR err;

#if (OS_CFG_STAT_TASK_EN == DEF_ENABLED)
  // Initialize CPU Usage.
  OSStatTaskCPUUsageInit(&err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE),; );
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
  // Initialize interrupts disabled measurement.
  CPU_IntDisMeasMaxCurReset();
#endif

  // Initialize the BSP. It is expected that the BSP will register all the hardware controller to
  // the platform manager at this moment.
  BSP_OS_Init();

  OSMutexCreate(&lightMutex,
                "Light Mutex",
                &err);

  OSQCreate((OS_Q     *)&demoEventQueue,
            (CPU_CHAR *)"Demo Queue",
            (OS_MSG_QTY)10,
            (RTOS_ERR   *)&err);

  // Create the Bluetooth Application task
  OSTaskCreate(&bluetoothAppTaskTCB,
               "Bluetooth App Task",
               bluetoothAppTask,
               0u,
               BLUETOOTH_APP_TASK_PRIO,
               &bluetoothAppTaskStk[0u],
               (BLUETOOTH_APP_TASK_STACK_SIZE / 10u),
               BLUETOOTH_APP_TASK_STACK_SIZE,
               0u,
               0u,
               0u,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &err);

  // Create the Proprietary Application task
  OSTaskCreate(&proprietaryAppTaskTCB,
               "Proprietary App Task",
               proprietaryAppTask,
               0u,
               PROPRIETARY_APP_TASK_PRIO,
               &proprietaryAppTaskStk[0u],
               (PROPRIETARY_APP_TASK_STACK_SIZE / 10u),
               PROPRIETARY_APP_TASK_STACK_SIZE,
               0u,
               0u,
               0u,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &err);

  // Create the Demo task.
  OSTaskCreate((OS_TCB     *)&demoAppTaskTCB,
               (CPU_CHAR   *)"Demo Task",
               (OS_TASK_PTR ) demoAppTask,
               (void       *) 0,
               (OS_PRIO     ) DEMO_APP_TASK_PRIO,
               (CPU_STK    *)&App_TaskDemoStk[0],
               (CPU_STK     )(DEMO_APP_TASK_STK_SIZE / 10u),
               (CPU_STK_SIZE) DEMO_APP_TASK_STK_SIZE,
               (OS_MSG_QTY  ) 0,
               (OS_TICK     ) 0,
               (void       *) 0,
               (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               (RTOS_ERR     *)&err);

  // Done starting everyone else so let's exit.
  OSTaskDel((OS_TCB *)0, &err);
}

/*********************************************************************************************************
 *                                             bluetoothAppTask()
 *
 * Description : Bluetooth Application task.
 *
 * Argument(s) : p_arg       the argument passed by 'OSTaskCreate()'.
 *
 * Return(s)   : none.
 *
 * Caller(s)   : This is a task.
 *
 * Note(s)     : none.
 *********************************************************************************************************
 */
static void bluetoothAppTask(void *p_arg)
{
  PP_UNUSED_PARAM(p_arg);
  RTOS_ERR err;

  BleState bleState = BLE_STATE_READY;
  memset(bleConn, 0, sizeof(bleConn));
  uint8_t addr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  uint8_t* addrPoi = addr;
  OS_FLAGS flags = 0;

  while (DEF_TRUE) {
    // wait for Bluetooth events; do not consume set flag
    flags |= OSFlagPend(&bluetooth_event_flags,
                        (OS_FLAGS)BLUETOOTH_EVENT_FLAG_EVT_WAITING
                        + (OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_START
                        + (OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS
                        + (OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_TIMEOUT,
                        (OS_TICK)0,
                        OS_OPT_PEND_BLOCKING
                        + OS_OPT_PEND_FLAG_SET_ANY,
                        NULL,
                        &err);

    // application events
    if (flags & (OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_MASK) {
      switch (bleState) {
        case BLE_STATE_READY:
          // send out indication
          if (flags & (OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_START) {
            flags &= ~(OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_START;
            bleFlagClrPost(BLUETOOTH_EVENT_FLAG_APP_INDICATE_START, &err);
            if (demo.lightInd == gatt_indication) {
              // start timer for light indication confirmation
              gecko_cmd_hardware_set_soft_timer(TIMER_S_2_TIMERTICK(1), demoTmrIndicate, true);
              lightPend(&err);
              /* Send notification/indication data */
              gecko_cmd_gatt_server_send_characteristic_notification(bleConn[0].handle,
                                                                     LIGHT_STATE_GATTDB,
                                                                     (uint8_t)sizeof(demo.light),
                                                                     (uint8_t*)&demo.light);
              lightPost(&err);
            } else {
              bleFlagSetPost(BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS, &err);
            }
            bleState = BLE_STATE_INDICATE_LIGHT;
          }
          break;

        case BLE_STATE_INDICATE_LIGHT:
          if (flags & (OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS) {
            flags &= ~(OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS;
            bleFlagClrPost(BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS, &err);
            if (demo.directionInd == gatt_indication) {
              // start timer for trigger source indication confirmation
              gecko_cmd_hardware_set_soft_timer(TIMER_S_2_TIMERTICK(1), demoTmrIndicate, true);
              lightPend(&err);
              /* Send notification/indication data */
              gecko_cmd_gatt_server_send_characteristic_notification(bleConn[0].handle,
                                                                     TRIGGER_SOURCE_GATTDB,
                                                                     (uint8_t)sizeof(demo.direction),
                                                                     (uint8_t*)&demo.direction);
              lightPost(&err);
            } else {
              bleFlagSetPost(BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS, &err);
            }
            bleState = BLE_STATE_INDICATE_TRIGGER_SOURCE;
          } else if (flags & (OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_TIMEOUT) {
            flags &= ~(OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_TIMEOUT;
            bleFlagClrPost(BLUETOOTH_EVENT_FLAG_APP_INDICATE_TIMEOUT, &err);
            demoEventQueuePost(DEMO_EVT_INDICATION_FAILED, &err);
            bleState = BLE_STATE_READY;
          }
          break;

        case BLE_STATE_INDICATE_TRIGGER_SOURCE:
          if (flags & (OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS) {
            flags &= ~(OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS;
            bleFlagClrPost(BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS, &err);
            if (demo.srcAddrInd == gatt_indication) {
              // start timer for source address indication confirmation
              gecko_cmd_hardware_set_soft_timer(TIMER_S_2_TIMERTICK(1), demoTmrIndicate, true);
              lightPend(&err);
              /* Send notification/indication data */
              gecko_cmd_gatt_server_send_characteristic_notification(bleConn[0].handle,
                                                                     SOURCE_ADDRESS_GATTDB,
                                                                     (uint8_t)sizeof(demo.srcAddr.addr),
                                                                     (uint8_t*)demo.srcAddr.addr);
              lightPost(&err);
            } else {
              bleFlagSetPost(BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS, &err);
            }
            bleState = BLE_STATE_INDICATE_SOURCE_ADDRESS;
          } else if (flags & (OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_TIMEOUT) {
            flags &= ~(OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_TIMEOUT;
            bleFlagClrPost(BLUETOOTH_EVENT_FLAG_APP_INDICATE_TIMEOUT, &err);
            demoEventQueuePost(DEMO_EVT_INDICATION_FAILED, &err);
            bleState = BLE_STATE_READY;
          }
          break;

        case BLE_STATE_INDICATE_SOURCE_ADDRESS:
          if (flags & (OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS) {
            flags &= ~(OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS;
            bleFlagClrPost(BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS, &err);
            bleState = BLE_STATE_READY;
            demoEventQueuePost(DEMO_EVT_INDICATION_SUCCESSFUL, &err);
          } else if (flags & (OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_TIMEOUT) {
            flags &= ~(OS_FLAGS)BLUETOOTH_EVENT_FLAG_APP_INDICATE_TIMEOUT;
            bleFlagClrPost(BLUETOOTH_EVENT_FLAG_APP_INDICATE_TIMEOUT, &err);
            demoEventQueuePost(DEMO_EVT_INDICATION_FAILED, &err);
            bleState = BLE_STATE_READY;
          }
          break;

        default:
          APP_RTOS_ASSERT_DBG(false, DEF_NULL);
          break;
      }
    }

    // stack events
    if (flags & (OS_FLAGS)BLUETOOTH_EVENT_FLAG_EVT_WAITING) {
      flags &= ~(OS_FLAGS)BLUETOOTH_EVENT_FLAG_EVT_WAITING;
      bleFlagClrPost(BLUETOOTH_EVENT_FLAG_EVT_WAITING, &err);
      // handle bluetooth events
      switch (BGLIB_MSG_ID(bluetooth_evt->header)) {
        case gecko_evt_system_boot_id:
          lightPend(&err);
          // own address: Bluetooth device address
          memset((void*)demo.ownAddr.addr, 0, sizeof(demo.ownAddr.addr));
          memcpy((void*)demo.ownAddr.addr, (void*)gecko_cmd_system_get_bt_address()->address.addr, sizeof(demo.ownAddr.addr));
          demo.indicationOngoing = false;
          lightPost(&err);
          bleState = BLE_STATE_READY;
          demoEventQueuePost(DEMO_EVT_BOOTED, &err);
          break;

        case gecko_evt_le_connection_bt5_opened_id:
          APP_RTOS_ASSERT_DBG(bleAddConn(bluetooth_evt->data.evt_le_connection_bt5_opened.connection,
                                         (bd_addr*)&bluetooth_evt->data.evt_le_connection_opened.address),
                              DEF_NULL);
          demoEventQueuePost(DEMO_EVT_BLUETOOTH_CONNECTED, &err);
          break;

        case gecko_evt_le_connection_opened_id:
          APP_RTOS_ASSERT_DBG(bleAddConn(bluetooth_evt->data.evt_le_connection_opened.connection,
                                         (bd_addr*)&bluetooth_evt->data.evt_le_connection_opened.address),
                              DEF_NULL);
          demoEventQueuePost(DEMO_EVT_BLUETOOTH_CONNECTED, &err);
          break;

        case gecko_evt_le_connection_closed_id:
          APP_RTOS_ASSERT_DBG(bleRemoveConn(bluetooth_evt->data.evt_le_connection_closed.connection),
                              DEF_NULL);
          demoEventQueuePost(DEMO_EVT_BLUETOOTH_DISCONNECTED, &err);
          break;

        /* This event indicates that a remote GATT client is attempting to write a value of an
         * attribute in to the local GATT database, where the attribute was defined in the GATT
         * XML firmware configuration file to have type="user".  */
        case gecko_evt_gatt_server_user_write_request_id:
          // light state write
          if (LIGHT_STATE_GATTDB == bluetooth_evt->data.evt_gatt_server_user_write_request.characteristic) {
            addrPoi = (uint8_t*)bleGetAddr(bluetooth_evt->data.evt_gatt_server_user_write_request.connection);
            APP_RTOS_ASSERT_DBG(addrPoi, DEF_NULL);
            lightPend(&err);
            demo.light = (DemoLight)bluetooth_evt->data.evt_gatt_server_user_write_request.value.data[0];
            demo.direction = demoLightDirectionBluetooth;
            memcpy(demo.srcAddr.addr, (void const*)addrPoi, sizeof(bd_addr));
            lightPost(&err);
            /* Send response to write request */
            gecko_cmd_gatt_server_send_user_write_response(bluetooth_evt->data.evt_gatt_server_user_write_request.connection,
                                                           LIGHT_STATE_GATTDB,
                                                           ES_WRITE_OK);
            demoEventQueuePost(DEMO_EVT_LIGHT_CHANGED_BLUETOOTH, &err);
            // unhandled write
          } else {
//            APP_RTOS_ASSERT_DBG(false, DEF_NULL);
          }
          break;

        /* This event indicates that a remote GATT client is attempting to read a value of an
         *  attribute from the local GATT database, where the attribute was defined in the GATT
         *  XML firmware configuration file to have type="user". */
        case gecko_evt_gatt_server_user_read_request_id:
          // light state read
          if (LIGHT_STATE_GATTDB == bluetooth_evt->data.evt_gatt_server_user_read_request.characteristic) {
            /* Send response to read request */
            lightPend(&err);
            gecko_cmd_gatt_server_send_user_read_response(bluetooth_evt->data.evt_gatt_server_user_write_request.connection,
                                                          LIGHT_STATE_GATTDB,
                                                          0,
                                                          sizeof(demo.light),
                                                          (uint8_t*)&demo.light);
            lightPost(&err);
            // trigger source read
          } else if (TRIGGER_SOURCE_GATTDB == bluetooth_evt->data.evt_gatt_server_user_read_request.characteristic) {
            /* Send response to read request */
            lightPend(&err);
            gecko_cmd_gatt_server_send_user_read_response(bluetooth_evt->data.evt_gatt_server_user_write_request.connection,
                                                          TRIGGER_SOURCE_GATTDB,
                                                          0,
                                                          sizeof(demo.direction),
                                                          (uint8_t*)&demo.direction);
            lightPost(&err);
            // source address read
          } else if (SOURCE_ADDRESS_GATTDB == bluetooth_evt->data.evt_gatt_server_user_read_request.characteristic) {
            /* Send response to read request */
            lightPend(&err);
            gecko_cmd_gatt_server_send_user_read_response(bluetooth_evt->data.evt_gatt_server_user_write_request.connection,
                                                          SOURCE_ADDRESS_GATTDB,
                                                          0,
                                                          sizeof(demo.srcAddr.addr),
                                                          (uint8_t*)demo.srcAddr.addr);
            lightPost(&err);
            // unhandled read
          } else {
//            APP_RTOS_ASSERT_DBG(false, DEF_NULL);
          }
          break;

        /* This event indicates either that a local Client Characteristic Configuration descriptor
         * has been changed by the remote GATT client, or that a confirmation from the remote GATT
         * client was received upon a successful reception of the indication. */
        case gecko_evt_gatt_server_characteristic_status_id:
          if (LIGHT_STATE_GATTDB == bluetooth_evt->data.evt_gatt_server_characteristic_status.characteristic) {
            // confirmation of indication received from remote GATT client
            if (gatt_server_confirmation == (enum gatt_server_characteristic_status_flag)bluetooth_evt->data.evt_gatt_server_characteristic_status.status_flags) {
              // stop light indication confirmation timer
              gecko_cmd_hardware_set_soft_timer(0, demoTmrIndicate, false);
              bleFlagSetPost(BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS, &err);
              // client characteristic configuration changed by remote GATT client
            } else if (gatt_server_client_config == (enum gatt_server_characteristic_status_flag)bluetooth_evt->data.evt_gatt_server_characteristic_status.status_flags) {
              lightPend(&err);
              demo.lightInd = (enum gatt_client_config_flag)bluetooth_evt->data.evt_gatt_server_characteristic_status.client_config_flags;
              lightPost(&err);
              // unhandled event
            } else {
              APP_RTOS_ASSERT_DBG(false, DEF_NULL);
            }
          } else if (TRIGGER_SOURCE_GATTDB == bluetooth_evt->data.evt_gatt_server_characteristic_status.characteristic) {
            // confirmation of indication received from GATT client
            if (gatt_server_confirmation == (enum gatt_server_characteristic_status_flag)bluetooth_evt->data.evt_gatt_server_characteristic_status.status_flags) {
              // stop direction indication confirmation timer
              gecko_cmd_hardware_set_soft_timer(0, demoTmrIndicate, false);
              bleFlagSetPost(BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS, &err);
              // client characteristic configuration changed by remote GATT client
            } else if (gatt_server_client_config == (enum gatt_server_characteristic_status_flag)bluetooth_evt->data.evt_gatt_server_characteristic_status.status_flags) {
              lightPend(&err);
              demo.directionInd = (enum gatt_client_config_flag)bluetooth_evt->data.evt_gatt_server_characteristic_status.client_config_flags;
              lightPost(&err);
              // unhandled event
            } else {
              APP_RTOS_ASSERT_DBG(false, DEF_NULL);
            }
          } else if (SOURCE_ADDRESS_GATTDB == bluetooth_evt->data.evt_gatt_server_characteristic_status.characteristic) {
            // confirmation of indication received from GATT client
            if (gatt_server_confirmation == (enum gatt_server_characteristic_status_flag)bluetooth_evt->data.evt_gatt_server_characteristic_status.status_flags) {
              // stop direction indication confirmation timer
              gecko_cmd_hardware_set_soft_timer(0, demoTmrIndicate, false);
              bleFlagSetPost(BLUETOOTH_EVENT_FLAG_APP_INDICATE_SUCCESS, &err);
              // client characteristic configuration changed by remote GATT client
            } else if (gatt_server_client_config == (enum gatt_server_characteristic_status_flag)bluetooth_evt->data.evt_gatt_server_characteristic_status.status_flags) {
              lightPend(&err);
              demo.srcAddrInd = (enum gatt_client_config_flag)bluetooth_evt->data.evt_gatt_server_characteristic_status.client_config_flags;
              lightPost(&err);
              // unhandled event
            } else {
              APP_RTOS_ASSERT_DBG(false, DEF_NULL);
            }
          } else {
//            APP_RTOS_ASSERT_DBG(false, DEF_NULL);
          }
          break;

        case gecko_evt_le_gap_scan_request_id:
          break;

        case gecko_evt_le_gap_scan_response_id:
          break;

        /* Software Timer event */
        case gecko_evt_hardware_soft_timer_id:
          // timer for periodic display polarity inversion
          if (demoTmrDispPolInv == bluetooth_evt->data.evt_hardware_soft_timer.handle) {
#ifndef FEATURE_IOEXPANDER
            /*Toggle the the EXTCOMIN signal, which prevents building up a DC bias  within the
             * Sharp memory LCD panel */
            dispPolarityInvert(0);
#endif /* FEATURE_IOEXPANDER */
            // timer for indication confirmation
          } else if (demoTmrIndicate == bluetooth_evt->data.evt_hardware_soft_timer.handle) {
            bleFlagSetPost(BLUETOOTH_EVENT_FLAG_APP_INDICATE_TIMEOUT, &err);
          } else if (demoTmrClearDirection == bluetooth_evt->data.evt_hardware_soft_timer.handle) {
            demoEventQueuePost(DEMO_EVT_CLEAR_DIRECTION, &err);
          } else {
            APP_RTOS_ASSERT_DBG(false, DEF_NULL);
          }
          break;
      }
    }
    OSFlagPost(&bluetooth_event_flags, (OS_FLAGS)BLUETOOTH_EVENT_FLAG_EVT_HANDLED, OS_OPT_POST_FLAG_SET, &err);
  }
}

// return false on error
static bool bleAddConn(uint8_t handle, bd_addr* address)
{
  for (uint8_t i = 0; i < MAX_CONNECTIONS; i++) {
    if (bleConn[i].inUse == false) {
      bleConn[i].handle = handle;
      memcpy((void*)&bleConn[i].address, (void const*)address, sizeof(bleConn[i].address));
      bleConn[i].inUse = true;
      return true;
    }
  }
  return false;
}

static bool bleRemoveConn(uint8_t handle)
{
  for (uint8_t i = 0; i < MAX_CONNECTIONS; i++) {
    if (bleConn[i].handle == handle) {
      bleConn[i].handle = 0;
      memset((void*)&bleConn[i].address.addr, 0, sizeof(bleConn[i].address.addr));
      bleConn[i].inUse = false;
      return true;
    }
  }
  return false;
}

static bd_addr* bleGetAddr(uint8_t handle)
{
  for (uint8_t i = 0; i < MAX_CONNECTIONS; i++) {
    if (bleConn[i].handle == handle) {
      return &(bleConn[i].address);
    }
  }
  return (bd_addr*)DEF_NULL;
}

static void beaconAdvertisements(void)
{
  static uint8_t *advData;
  static uint8_t advDataLen;

  advData = (uint8_t*)&iBeaconData;
  advDataLen = sizeof(iBeaconData);
  // Set custom advertising data.
  gecko_cmd_le_gap_bt5_set_adv_data(1, 0, advDataLen, advData);
  gecko_cmd_le_gap_bt5_set_adv_parameters(1, 160, 160, 7, 0);
  gecko_cmd_le_gap_bt5_set_mode(1,
                                le_gap_user_data,
                                le_gap_non_connectable,
                                0,
                                le_gap_non_resolvable);

  advData = (uint8_t*)&eddystone_data;
  advDataLen = sizeof(eddystone_data);
  // Set custom advertising data.
  gecko_cmd_le_gap_bt5_set_adv_data(2, 0, advDataLen, advData);
  gecko_cmd_le_gap_bt5_set_adv_parameters(2, 160, 160, 7, 0);
  gecko_cmd_le_gap_bt5_set_mode(2,
                                le_gap_user_data,
                                le_gap_non_connectable,
                                0,
                                le_gap_non_resolvable);
}

static void enableBleAdvertisements(void)
{
  // set transmit power to 0 dBm.
  gecko_cmd_system_set_tx_power(0);
  // Create the device name based on the 16-bit device ID.
  uint16_t devId;
  devId = *((uint16*)demo.ownAddr.addr);

  // Copy to the local GATT database - this will be used by the BLE stack
  // to put the local device name into the advertisements, but only if we are
  // using default advertisements
  static char devName[DEVNAME_LEN];
  snprintf(devName, DEVNAME_LEN, DEVNAME, devId >> 8, devId & 0xff);
  gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name,
                                              0,
                                              strlen(devName),
                                              (uint8_t *)devName);
  // Copy the shortened device name to the response data, overwriting
  // the default device name which is set at compile time
  memcpy(((uint8_t*)&responseData) + 5, devName, 8);
  // Set the response data
  gecko_cmd_le_gap_bt5_set_adv_data(0, 0, sizeof(responseData), (uint8_t*)&responseData);
  // Set nominal 100ms advertising interval, so we just get
  // a single beacon of each type
  gecko_cmd_le_gap_bt5_set_adv_parameters(0, 160, 160, 7, 1);
  /* Start advertising in user mode and enable connections, if we are
   * not already connected */
  if (demo.connBluetoothInUse) {
    gecko_cmd_le_gap_bt5_set_mode(0,
                                  le_gap_user_data,
                                  le_gap_non_connectable,
                                  0,
                                  le_gap_identity_address);
  } else {
    gecko_cmd_le_gap_bt5_set_mode(0,
                                  le_gap_user_data,
                                  le_gap_undirected_connectable,
                                  0,
                                  le_gap_identity_address);
  }
  beaconAdvertisements();
}

/*********************************************************************************************************
 *                                             proprietaryAppTask()
 *
 * Description : RAIL Application task.
 *
 * Argument(s) : p_arg       the argument passed by 'OSTaskCreate()'.
 *
 * Return(s)   : none.
 *
 * Caller(s)   : This is a task.
 *
 * Note(s)     : none.
 *********************************************************************************************************
 */
static void proprietaryAppTask(void *p_arg)
{
  PP_UNUSED_PARAM(p_arg);
  RTOS_ERR err;
  bool lightState;
  CPU_TS  timeStamp;
  OS_MSG_SIZE msgQSize;

  proprietaryInit();

  while (DEF_TRUE) {
    lightState = (bool)OSQPend((OS_Q *)&railTransmitPacketQueue, 0,
                               (OS_OPT       )OS_OPT_PEND_BLOCKING,
                               (OS_MSG_SIZE *)&msgQSize,
                               (CPU_TS      *)&timeStamp,
                               (RTOS_ERR      *)&err);

    proprietaryTxPacket(lightState);
  }
}

static void radioRxEventHandler(RAIL_Handle_t railHandle,
                                RAIL_Events_t events)
{
  RTOS_ERR err;
  volatile DemoControlRole role;
  volatile DemoControlCommandType command;

  if (events & RAIL_EVENT_RX_PACKET_RECEIVED) {
    RAIL_RxPacketInfo_t packetInfo;
    RAIL_GetRxPacketInfo(railHandle,
                         RAIL_RX_PACKET_HANDLE_NEWEST,
                         &packetInfo);

    if ((packetInfo.packetStatus != RAIL_RX_PACKET_READY_SUCCESS)
        && (packetInfo.packetStatus != RAIL_RX_PACKET_READY_CRC_ERROR)) {
      // RAIL_EVENT_RX_PACKET_RECEIVED must be handled last in order to return
      // early on aborted packets here.
      return;
    }

    // Read packet data into our packet structure
    uint16_t length = packetInfo.packetBytes;
    memcpy(receiveBuffer,
           packetInfo.firstPortionData,
           packetInfo.firstPortionBytes);
    memcpy(receiveBuffer + packetInfo.firstPortionBytes,
           packetInfo.lastPortionData,
           length - packetInfo.firstPortionBytes);

    // packet sent by switch
    role = (DemoControlRole)((receiveBuffer[DEMO_CONTROL_PAYLOAD_CMD_DATA] & DEMO_CONTROL_PAYLOAD_SRC_ROLE_BIT) >> DEMO_CONTROL_PAYLOAD_SRC_ROLE_BIT_SHIFT);
    if (DEMO_CONTROL_ROLE_SWITCH == role) {
      // packet contains toggle command
      command = (DemoControlCommandType)((receiveBuffer[DEMO_CONTROL_PAYLOAD_CMD_DATA] & DEMO_CONTROL_PAYLOAD_CMD_MASK) >> DEMO_CONTROL_PAYLOAD_CMD_MASK_SHIFT);
      if (DEMO_CONTROL_CMD_LIGHT_TOGGLE == command) {
        lightPend(&err);
        if (demo.light == demoLightOff) {
          demo.light = demoLightOn;
        } else {
          demo.light = demoLightOff;
        }
        demo.direction = demoLightDirectionProprietary;
        memcpy((void*)demo.srcAddr.addr, &receiveBuffer[PACKET_HEADER_LEN], sizeof(demo.srcAddr.addr));
        lightPost(&err);
        demoEventQueuePost(DEMO_EVT_LIGHT_CHANGED_RAIL, &err);
      }
    } else {
      // TODO: Anything app specific when something else is received
    }
  }
}

static void radioTxEventHandler(RAIL_Handle_t railHandle,
                                RAIL_Events_t events)
{
  volatile RAIL_Events_t events_tmp = events;

  if (events & (RAIL_EVENT_TX_ABORTED)) {
    //TODO: anything after pkt tx ?
    events_tmp += 1;
  }
  if (events & (RAIL_EVENT_TX_PACKET_SENT)) {
    //TODO: anything after pkt tx ?
    events_tmp += 1;
  }
  if (events & (RAIL_EVENT_TX_UNDERFLOW)) {
    //TODO: anything after pkt tx ?
    events_tmp += 1;
  }
  if (events & (RAIL_EVENT_SCHEDULER_STATUS)) {
//    RAIL_SchedulerStatus_t stat;
//    stat = RAIL_GetSchedulerStatus(railHandle);
  }

  RAIL_YieldRadio(railHandle);
}

// Initialize the two PHY configurations for the radio
static void radioInitialize(void)
{
  // Create each RAIL handle with their own configuration structures
  gRailHandleRx = RAIL_Init(&railCfgRx, NULL);
  gRailHandleTx = RAIL_Init(&railCfgTx, NULL);
  // Setup the transmit FIFOs
  RAIL_SetTxFifo(gRailHandleTx, txFifo, 0, RAIL_TX_FIFO_SIZE);
  // Configure radio according to the generated radio settings
  RAIL_TxPowerConfig_t railTxPowerConfig = {
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
    railTxPowerConfig.mode = RAIL_TX_POWER_MODE_SUBGIG;
  }
  if (RAIL_ConfigTxPower(gRailHandleTx, &railTxPowerConfig) != RAIL_STATUS_NO_ERROR) {
//    printf("Error: The PA could not be initialized due to an improper configuration.\n");
//    printf("       Please ensure your configuration is valid for the selected part.\n");
    while (1) ;
  }
  // We must reapply the Tx power after changing the PA above
  RAIL_SetTxPower(gRailHandleTx, HAL_PA_POWER);
  RAIL_ConfigChannels(gRailHandleRx, channelConfigs[0], NULL);
  RAIL_ConfigChannels(gRailHandleTx, channelConfigs[0], NULL);
  // Configure the most useful callbacks plus catch a few errors
  RAIL_ConfigEvents(gRailHandleRx,
                    RAIL_EVENTS_ALL,
                    RAIL_EVENT_TX_PACKET_SENT
                    | RAIL_EVENT_TX_ABORTED
                    | RAIL_EVENT_TX_UNDERFLOW
                    | RAIL_EVENT_SCHEDULER_STATUS
                    | RAIL_EVENT_RX_PACKET_RECEIVED
                    | RAIL_EVENT_RX_FRAME_ERROR // invalid CRC
                    | RAIL_EVENT_RX_ADDRESS_FILTERED);
  RAIL_ConfigEvents(gRailHandleTx,
                    RAIL_EVENTS_ALL,
                    RAIL_EVENT_TX_PACKET_SENT
                    | RAIL_EVENT_TX_ABORTED
                    | RAIL_EVENT_TX_UNDERFLOW
                    | RAIL_EVENT_SCHEDULER_STATUS
                    | RAIL_EVENT_RX_PACKET_RECEIVED
                    | RAIL_EVENT_RX_FRAME_ERROR // invalid CRC
                    | RAIL_EVENT_RX_ADDRESS_FILTERED);
}

static void proprietaryInit(void)
{
  RAIL_SchedulerInfo_t schedulerInfo;
  RTOS_ERR err;

  OSQCreate((OS_Q     *)&railTransmitPacketQueue,
            (CPU_CHAR *)"RAIL Packet Transmit Queue",
            (OS_MSG_QTY)10,
            (RTOS_ERR   *)&err);

  // Initialize the radio and create the two RAIL handles
  radioInitialize();
  // Only set priority because transactionTime is meaningless for infinite
  // operations and slipTime has a reasonable default for relative operations.
  schedulerInfo = (RAIL_SchedulerInfo_t){ .priority = 200 };
  RAIL_StartRx(gRailHandleRx, 0, &schedulerInfo);
}

static void proprietaryTxPacket(bool lightState)
{
  RAIL_SchedulerInfo_t schedulerInfo;
  RAIL_Status_t res;

  // This assumes the Tx time is around 200us
  schedulerInfo = (RAIL_SchedulerInfo_t){ .priority = 100,
                                          .slipTime = 100000,
                                          .transactionTime = 200 };
  // light role
  dataPacket[LIGHT_CONTROL_DATA_BYTE] &= ~DEMO_CONTROL_PAYLOAD_SRC_ROLE_BIT;
  dataPacket[LIGHT_CONTROL_DATA_BYTE] |= (DEMO_CONTROL_ROLE_LIGHT << DEMO_CONTROL_PAYLOAD_SRC_ROLE_BIT_SHIFT)
                                         & DEMO_CONTROL_PAYLOAD_SRC_ROLE_BIT;
  // state report command
  dataPacket[LIGHT_CONTROL_DATA_BYTE] &= ~DEMO_CONTROL_PAYLOAD_CMD_MASK;
  dataPacket[LIGHT_CONTROL_DATA_BYTE] |= (DEMO_CONTROL_CMD_LIGHT_STATE_REPORT << DEMO_CONTROL_PAYLOAD_CMD_MASK_SHIFT)
                                         & DEMO_CONTROL_PAYLOAD_CMD_MASK;
  dataPacket[LIGHT_CONTROL_DATA_BYTE] &= ~0x01;
  dataPacket[LIGHT_CONTROL_DATA_BYTE] |= (uint8_t)lightState;

  RAIL_WriteTxFifo(gRailHandleTx, dataPacket, sizeof(dataPacket), true);

  res = RAIL_StartTx(gRailHandleTx,
                     0,
                     RAIL_TX_OPTIONS_DEFAULT,
                     &schedulerInfo);

  if (res != RAIL_STATUS_NO_ERROR) {
    // Try once to resend the packet 100ms later in case of error
    RAIL_ScheduleTxConfig_t scheduledTxConfig = { .when = RAIL_GetTime() + 100000,
                                                  .mode = RAIL_TIME_ABSOLUTE };

    // Transmit this packet at the specified time or up to 50 ms late
    res = RAIL_StartScheduledTx(gRailHandleTx,
                                0,
                                RAIL_TX_OPTIONS_DEFAULT,
                                &scheduledTxConfig,
                                &schedulerInfo);
  }
}

/*
 *********************************************************************************************************
 *                                             demoAppTask()
 *
 * Description : Demo task.
 *
 * Argument(s) : p_arg       the argument passed by 'OSTaskCreate()'.
 *
 * Return(s)   : none.
 *
 * Caller(s)   : This is a task.
 *
 * Note(s)     : none.
 *********************************************************************************************************
 */
static void demoAppTask(void *p_arg)
{
  PP_UNUSED_PARAM(p_arg);
  RTOS_ERR err;
  CPU_TS timeStamp;
  OS_MSG_SIZE msgQSize;
  DemoEvt demoEvent;
  bool bluetoothInd = false;

  char devIdStr[DEV_ID_STR_LEN];
  snprintf(devIdStr, DEV_ID_STR_LEN, "DMP:%04X", *((uint16*)demo.ownAddr.addr));

  // create timer for periodic indications
  OSTmrCreate(&demoTimer,            /*   Pointer to user-allocated timer.     */
              "Demo Timer",          /*   Name used for debugging.             */
              0,                  /*     0 initial delay.                   */
              10,                  /*   100 Timer Ticks period.              */
              OS_OPT_TMR_PERIODIC,  /*   Timer is periodic.                   */
              &demoTimerCallback,    /*   Called when timer expires.           */
              DEF_NULL,             /*   No arguments to callback.            */
              &err);

  demo.state = DEMO_STATE_INIT;
  GpioSetup();
  BSP_LedsInit();
  dmpUiInit();
  dmpUiClearMainScreen((uint8_t*)"Light", true, true);
  dmpUiDisplayId(DMP_UI_PROTOCOL2, (uint8_t*)devIdStr);

  /* Task body, always written as an infinite loop.       */
  while (DEF_TRUE) {
    demoEvent = (DemoEvt)OSQPend((OS_Q *)&demoEventQueue,
                                 0,
                                 (OS_OPT       )OS_OPT_PEND_BLOCKING,
                                 (OS_MSG_SIZE *)&msgQSize,
                                 (CPU_TS      *)&timeStamp,
                                 (RTOS_ERR      *)&err);

    switch (demo.state) {
      case DEMO_STATE_INIT:
        switch (demoEvent) {
          // bluetooth booted
          case DEMO_EVT_BOOTED:
            // initialise demo variables:
            // make it look like the last trigger source of the light was a button press
            lightPend(&err);
            demo.connBluetoothInUse = 0;
            demo.connProprietaryInUse = 1;
            demo.indicationOngoing = false;
            demo.direction = demoLightDirectionButton;
            memcpy((void*)demo.srcAddr.addr, (void*)demo.ownAddr.addr, sizeof(demo.srcAddr.addr));
            lightPost(&err);
            enableBleAdvertisements();
            // bluetooth client connected or proprietary link is used; start demo timer
            if (demo.connBluetoothInUse || demo.connProprietaryInUse) {
              OSTmrStart(&demoTimer, &err);
            }
            demo.state = DEMO_STATE_READY;
            break;

          default:
            break;
        }
        break;

      case DEMO_STATE_READY:
        switch (demoEvent) {
          case DEMO_EVT_BLUETOOTH_CONNECTED:
            demo.indicationOngoing = false;
            if (demo.connBluetoothInUse < MAX_CONNECTIONS) {
              demo.connBluetoothInUse += 1;
            }
            // first bluetooth client connected; start demo timer
            if (demo.connBluetoothInUse) {
              OSTmrStart(&demoTimer, &err);
            }
            dmpUiDisplayProtocol(DMP_UI_PROTOCOL2, true);
            break;

          case DEMO_EVT_BLUETOOTH_DISCONNECTED:
            demo.indicationOngoing = false;
            if (demo.connBluetoothInUse > 0) {
              demo.connBluetoothInUse -= 1;
            }
            dmpUiClearDirection(DMP_UI_DIRECTION_PROT2);
            dmpUiDisplayProtocol(DMP_UI_PROTOCOL2, false);
            // restart connectable advertising
            enableBleAdvertisements();
            // no bluetooth client connected; stop demo timer
            if (!demo.connBluetoothInUse) {
              OSTmrStop(&demoTimer, OS_OPT_TMR_NONE, DEF_NULL, &err);
            }
            break;

          case DEMO_EVT_RAIL_CONNECTED:
            dmpUiDisplayProtocol(DMP_UI_PROTOCOL1, true);
            break;

          case DEMO_EVT_RAIL_DISCONNECTED:
            dmpUiClearDirection(DMP_UI_DIRECTION_PROT1);
            dmpUiDisplayProtocol(DMP_UI_PROTOCOL1, false);
            break;

          case DEMO_EVT_LIGHT_CHANGED_BLUETOOTH:
            if (demoLightOff == demo.light) {
              appUiLedOff();
            } else {
              appUiLedOn();
            }
            dmpUiDisplayLight(demo.light);
            dmpUiDisplayId(DMP_UI_PROTOCOL2, (uint8_t*)devIdStr);
            dmpUiDisplayDirection(DMP_UI_DIRECTION_PROT2);
            gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(500), demoTmrClearDirection, true);
            demoEventQueuePost(DEMO_EVT_INDICATION, &err);
            break;

          case DEMO_EVT_LIGHT_CHANGED_RAIL:
            lightPend(&err);
            demo.direction = demoLightDirectionProprietary;
            if (demoLightOff == demo.light) {
              appUiLedOff();
            } else {
              appUiLedOn();
            }
            lightPost(&err);
            railTransmitQueuedPacket(demo.light, err);
            dmpUiDisplayLight(demo.light);
            dmpUiDisplayId(DMP_UI_PROTOCOL2, (uint8_t*)devIdStr);
            dmpUiDisplayDirection(DMP_UI_DIRECTION_PROT1);
            gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(500), demoTmrClearDirection, true);
            demoEventQueuePost(DEMO_EVT_INDICATION, &err);
            break;

          case DEMO_EVT_BUTTON_PRESSED:
            lightPend(&err);
            if (demoLightOff == demo.light) {
              demo.light = demoLightOn;
              appUiLedOn();
            } else {
              demo.light = demoLightOff;
              appUiLedOff();
            }
            demo.direction = demoLightDirectionButton;
            memcpy((void*)demo.srcAddr.addr, (void const*)demo.ownAddr.addr, sizeof(demo.srcAddr.addr));
            lightPost(&err);
            railTransmitQueuedPacket(demo.light, err);
            dmpUiDisplayLight(demo.light);
            dmpUiDisplayId(DMP_UI_PROTOCOL2, (uint8_t*)devIdStr);
            demoEventQueuePost(DEMO_EVT_INDICATION, &err);
            break;

          case DEMO_EVT_CLEAR_DIRECTION:
            if (demoLightDirectionProprietary == demo.direction) {
              dmpUiClearDirection(DMP_UI_DIRECTION_PROT1);
            } else {
              dmpUiClearDirection(DMP_UI_DIRECTION_PROT2);
            }
            break;

          case DEMO_EVT_INDICATION:
            lightPend(&err);
            bluetoothInd = demo.connBluetoothInUse
                           && (demo.lightInd == gatt_indication
                               || demo.directionInd == gatt_indication
                               || demo.srcAddrInd == gatt_indication);
            // no ongoing indication, free to start sending them out
            if (!demo.indicationOngoing) {
              if (bluetoothInd || demo.connProprietaryInUse) {
                demo.indicationOngoing = true;
              }
              // send indication on BLE side
              if (bluetoothInd) {
                bleFlagSetPost(BLUETOOTH_EVENT_FLAG_APP_INDICATE_START, &err);
              }
              // send indication on proprietary side
              // there is no protocol on proprietary side; BLE side transmission is the slower
              // send out proprietary packets at maximum the same rate as BLE indications
              if (demo.connProprietaryInUse) {
                railTransmitQueuedPacket(demo.light, err);
                if (!bluetoothInd) {
                  demoEventQueuePost(DEMO_EVT_INDICATION_SUCCESSFUL, &err);
                }
              }
              // ongoing indication; set pending flag
            } else {
              demo.indicationPending = true;
            }
            lightPost(&err);
            break;

          case DEMO_EVT_INDICATION_SUCCESSFUL:
          case DEMO_EVT_INDICATION_FAILED:
            demo.indicationOngoing = false;
            if (demo.indicationPending) {
              demo.indicationPending = false;
              demoEventQueuePost(DEMO_EVT_INDICATION, &err);
            }
            break;

          default:
            APP_RTOS_ASSERT_DBG(false, DEF_NULL);
            break;
        }
        break;

      // error
      default:
        APP_RTOS_ASSERT_DBG(false, DEF_NULL);
        break;
    }
  }
}

static void demoTimerCallback(void *p_tmr,
                              void *p_arg)
{
  RTOS_ERR err;
  /* Called when timer expires:                            */
  /*   'p_tmr' is pointer to the user-allocated timer.     */
  /*   'p_arg' is argument passed when creating the timer. */
  demoEventQueuePost(DEMO_EVT_INDICATION, &err);
}

/***************************************************************************//**
 * @brief Setup GPIO interrupt for pushbuttons.
 ******************************************************************************/
static void GpioSetup(void)
{
  // Enable GPIO clock.
  CMU_ClockEnable(cmuClock_GPIO, true);
  // Initialize GPIO interrupt.
  GPIOINT_Init();
  // Configure PB0 as input and enable interrupt.
  GPIO_PinModeSet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN, gpioModeInputPull, 1);
  GPIO_IntConfig(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN, false, true, true);
  GPIOINT_CallbackRegister(BSP_BUTTON0_PIN, GPIO_Button_IRQHandler);
  // Configure PB1 as input and enable interrupt.
  GPIO_PinModeSet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN, gpioModeInputPull, 1);
  GPIO_IntConfig(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN, false, true, true);
  GPIOINT_CallbackRegister(BSP_BUTTON1_PIN, GPIO_Button_IRQHandler);
}

/***************************************************************************//**
 * @brief GPIO Interrupt handler (PB1)
 *        Switches between analog and digital clock modes.
 ******************************************************************************/
static void GPIO_Button_IRQHandler(uint8_t pin)
{
  PP_UNUSED_PARAM(pin);
  RTOS_ERR err;
  demoEventQueuePost(DEMO_EVT_BUTTON_PRESSED, &err);
}

void appUiLedOff(void)
{
  BSP_LedClear(0);
  BSP_LedClear(1);
}

void appUiLedOn(void)
{
  BSP_LedSet(0);
  BSP_LedSet(1);
}

/**************************************************************************//**
 * @brief   Register a callback function at the given frequency.
 *
 * @param[in] pFunction  Pointer to function that should be called at the
 *                       given frequency.
 * @param[in] argument   Argument to be given to the function.
 * @param[in] frequency  Frequency at which to call function at.
 *
 * @return  0 for successful or
 *         -1 if the requested frequency does not match the RTC frequency.
 *****************************************************************************/
int rtcIntCallbackRegister(void (*pFunction)(void*),
                           void* argument,
                           unsigned int frequency)
{
#ifndef FEATURE_IOEXPANDER
  dispPolarityInvert =  pFunction;
  // Start timer with required frequency.
  gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(1000 / frequency),
                                    demoTmrDispPolInv,
                                    false);
#endif /* FEATURE_IOEXPANDER */
  return 0;
}

static inline void lightPend(RTOS_ERR* err)
{
  OSMutexPend((OS_MUTEX *)&lightMutex,
              (OS_TICK   )0,
              (OS_OPT    )OS_OPT_PEND_BLOCKING,
              (CPU_TS   *)NULL,
              (RTOS_ERR *)err);
}

static inline void lightPost(RTOS_ERR* err)
{
  OSMutexPost((OS_MUTEX *)&lightMutex,
              (OS_OPT    )OS_OPT_POST_NONE,
              (RTOS_ERR *)err);
}
