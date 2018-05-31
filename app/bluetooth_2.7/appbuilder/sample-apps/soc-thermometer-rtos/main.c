/*
 *********************************************************************************************************
 *                                             EXAMPLE CODE
 *********************************************************************************************************
 * Licensing terms:
 *   This file is provided as an example on how to use Micrium products. It has not necessarily been
 *   tested under every possible condition and is only offered as a reference, without any guarantee.
 *
 *   Please feel free to use any application code labeled as 'EXAMPLE CODE' in your application products.
 *   Example code may be used as is, in whole or in part, or may be used as a reference only. This file
 *   can be modified as required.
 *
 *   You can find user manuals, API references, release notes and more at: https://doc.micrium.com
 *
 *   You can contact us at: http://www.micrium.com
 *
 *   Please help us continue to provide the Embedded community with the finest software available.
 *
 *   Your honesty is greatly appreciated.
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *
 *                                             EXAMPLE MAIN
 *
 * File : ex_main.c
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                            INCLUDE FILES
 *********************************************************************************************************
 *********************************************************************************************************
 */

#include  "bsp/siliconlabs/generic/include/bsp_os.h"
#include  "bsp/siliconlabs/generic/include/bsp_cpu.h"

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/common.h>
#include  <rtos/kernel/include/os.h>

#include  <rtos/common/include/lib_def.h>
#include  <rtos/common/include/rtos_utils.h>
#include  <rtos/common/include/toolchains.h>
#include  <rtos/common/include/rtos_prio.h>

#include  "sleep.h"
#include  <stdio.h>

#include "rtos_bluetooth.h"
//Bluetooth API definition
#include "rtos_gecko.h"
//GATT DB
#include "gatt_db.h"

/* Board Headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"

#include "em_cmu.h"

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                             LOCAL DEFINES
 *********************************************************************************************************
 *********************************************************************************************************
 */

#define OTA

#define  EX_MAIN_START_TASK_PRIO              21u
#define  EX_MAIN_START_TASK_STK_SIZE         512u

#define  APP_CFG_TASK_START_PRIO                       2u
#define  APP_CFG_TASK_BLUETOOTH_LL_PRIO                3u
#define  APP_CFG_TASK_BLUETOOTH_STACK_PRIO             4u
#define  APP_CFG_TASK_APPLICATION_PRIO                 5u
#define  APP_CFG_TASK_THERMOMETER_PRIO                 6u

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                        LOCAL GLOBAL VARIABLES
 *********************************************************************************************************
 *********************************************************************************************************
 */

/* Start Task Stack.                                    */
static  CPU_STK  Ex_MainStartTaskStk[EX_MAIN_START_TASK_STK_SIZE];
/* Start Task TCB.                                      */
static  OS_TCB   Ex_MainStartTaskTCB;

// Thermometer Task.
#define  APP_CFG_TASK_STK_SIZE                       256u
static  void  App_TaskThermometer(void *p_arg);
OS_TCB        App_TaskThermometerTCB;
CPU_STK_SIZE  App_TaskThermometerStk[APP_CFG_TASK_STK_SIZE];

//event handler task
#define APPLICATION_STACK_SIZE (500 / sizeof(CPU_STK))
static  void  BluetoothApplicationTask (void  *p_arg);
static  OS_TCB            ApplicationTaskTCB;
static  CPU_STK           ApplicationTaskStk[APPLICATION_STACK_SIZE];

/*
 * Bluetooth stack configuration
 */

#define MAX_CONNECTIONS 1
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];
/* Gecko configuration parameters (see gecko_configuration.h) */
static const gecko_configuration_t bluetooth_config =
{
  .config_flags = GECKO_CONFIG_FLAG_RTOS,
  .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
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
#ifdef OTA
  .ota.flags = 0,
  .ota.device_name_len = 3,
  .ota.device_name_ptr = "OTA",
#endif
};
/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                       LOCAL FUNCTION PROTOTYPES
 *********************************************************************************************************
 *********************************************************************************************************
 */

static void Ex_MainStartTask (void  *p_arg);
static void idleHook(void);
static void setupHooks(void);
#ifdef OTA
static uint8_t boot_to_dfu = 0;
#endif

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
  RTOS_ERR  err;

  // Initialize device
  initMcu();
  // Initialize board
  initBoard();
  // Initialize application
  initApp();

  CMU_ClockEnable(cmuClock_PRS, true);

  /* Setup a 1024 Hz tick instead of the default 1000 Hz. This
   * improves accuracy when using dynamic tick which runs of
   * the RTCC at 32768 Hz. */
  OS_TASK_CFG tickTaskCfg = {
    .StkBasePtr = DEF_NULL,
    .StkSize    = 256u,
    .Prio       = KERNEL_TICK_TASK_PRIO_DFLT,
    .RateHz     = 1024u
  };

  BSP_CPUInit();                                                /* Initialize CPU and make all interrupts Kernel Aware. */
  //system already initialized by enter_DefaultMode_from_RESET
  //BSP_SystemInit();                                           /* Initialize System.                                   */

  OS_ConfigureTickTask(&tickTaskCfg);
  OSInit(&err);                                                 /* Initialize the Kernel.                               */
                                                                /*   Check error code.                                  */
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  BSP_TickInit();                                               /* Initialize Kernel tick source.                       */
  setupHooks();

  gecko_init(&bluetooth_config);
  bluetooth_start_task(APP_CFG_TASK_BLUETOOTH_LL_PRIO, APP_CFG_TASK_BLUETOOTH_STACK_PRIO);

  OSTaskCreate(&Ex_MainStartTaskTCB,                            /* Create the Start Task.                               */
               "Ex Main Start Task",
               Ex_MainStartTask,
               DEF_NULL,
               EX_MAIN_START_TASK_PRIO,
               &Ex_MainStartTaskStk[0],
               (EX_MAIN_START_TASK_STK_SIZE / 10u),
               EX_MAIN_START_TASK_STK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);
  /*   Check error code.                                  */
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  OSStart(&err);                                                /* Start the kernel.                                    */
                                                                /*   Check error code.                                  */
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  return (1);
}

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                           LOCAL FUNCTIONS
 *********************************************************************************************************
 *********************************************************************************************************
 */
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
static  void  Ex_MainStartTask(void  *p_arg)
{
  RTOS_ERR  err;

  PP_UNUSED_PARAM(p_arg);                                       /* Prevent compiler warning.                            */

#if (OS_CFG_STAT_TASK_EN == DEF_ENABLED)
  OSStatTaskCPUUsageInit(&err);                                 /* Initialize CPU Usage.                                */
                                                                /*   Check error code.                                  */
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE),; );
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
  CPU_IntDisMeasMaxCurReset();                                  /* Initialize interrupts disabled measurement.          */
#endif

  BSP_OS_Init();                                                /* Initialize the BSP. It is expected that the BSP ...  */
                                                                /* ... will register all the hardware controller to ... */
                                                                /* ... the platform manager at this moment.             */

//create tasks for Event handler
  OSTaskCreate(&ApplicationTaskTCB,
               "Bluetooth Application Task",
               BluetoothApplicationTask,
               0u,
               APP_CFG_TASK_APPLICATION_PRIO,
               &ApplicationTaskStk[0u],
               ApplicationTaskStk[APPLICATION_STACK_SIZE / 10u],
               APPLICATION_STACK_SIZE,
               0u,
               0u,
               0u,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &err);
  // Create the Thermometer task.
  OSTaskCreate((OS_TCB     *)&App_TaskThermometerTCB,
               (CPU_CHAR   *)"Thermometer Task",
               (OS_TASK_PTR ) App_TaskThermometer,
               (void       *) 0,
               (OS_PRIO     ) APP_CFG_TASK_THERMOMETER_PRIO,
               (CPU_STK    *)&App_TaskThermometerStk[0],
               (CPU_STK     )(APP_CFG_TASK_STK_SIZE / 10u),
               (CPU_STK_SIZE) APP_CFG_TASK_STK_SIZE,
               (OS_MSG_QTY  ) 0,
               (OS_TICK     ) 0,
               (void       *) 0,
               (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               (RTOS_ERR     *)&err);

//   GPIO->P[5].DOUT |= (1 << 4);
//   GPIO->P[5].DOUT |= (1 << 5);

  /* Done starting everyone else so let's exit */
  OSTaskDel((OS_TCB *)0, &err);
}

void BluetoothEventHandler(struct gecko_cmd_packet* evt)
{
  switch (BGLIB_MSG_ID(evt->header)) {
    case gecko_evt_system_boot_id:
    case gecko_evt_le_connection_closed_id:
#ifdef OTA
      if (boot_to_dfu) {
        gecko_cmd_system_reset(2);
      }
#endif
      //Start advertisement at boot, and after disconnection
      gecko_cmd_le_gap_set_mode(le_gap_general_discoverable, le_gap_undirected_connectable);
      break;
#ifdef OTA
    case gecko_evt_gatt_server_user_write_request_id:
      if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_ota_control) {
        //boot to dfu mode after disconnecting
        boot_to_dfu = 1;
        gecko_cmd_gatt_server_send_user_write_response(evt->data.evt_gatt_server_user_write_request.connection, gattdb_ota_control, bg_err_success);
        gecko_cmd_endpoint_close(evt->data.evt_gatt_server_user_write_request.connection);
      }
      break;
#endif
  }
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
  /* Put MCU in the lowest sleep mode available, usually EM2 */
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
  /* Don't allow EM3, since we use LF clocks. */
  SLEEP_SleepBlockBegin(sleepEM3);
  OS_AppIdleTaskHookPtr = idleHook;
  CPU_CRITICAL_EXIT();
}

/*********************************************************************************************************
 *                                             BluetoothApplicationTask()
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
static  void  BluetoothApplicationTask(void *p_arg)
{
  RTOS_ERR      os_err;
  (void)p_arg;

  while (DEF_TRUE) {
    OSFlagPend(&bluetooth_event_flags, (OS_FLAGS)BLUETOOTH_EVENT_FLAG_EVT_WAITING,
               0,
               OS_OPT_PEND_BLOCKING + OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME,
               NULL,
               &os_err);
    BluetoothEventHandler((struct gecko_cmd_packet*)bluetooth_evt);

    OSFlagPost(&bluetooth_event_flags, (OS_FLAGS)BLUETOOTH_EVENT_FLAG_EVT_HANDLED, OS_OPT_POST_FLAG_SET, &os_err);
  }
}
/*
 *********************************************************************************************************
 *                                             App_TaskThermometer()
 *
 * Description : 'Thermometer' task.
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

enum bg_thermometer_temperature_measurement_flag{
  bg_thermometer_temperature_measurement_flag_units    =0x1,
  bg_thermometer_temperature_measurement_flag_timestamp=0x2,
  bg_thermometer_temperature_measurement_flag_type     =0x4,
};
/**
 * Convert mantissa & exponent values to IEEE float type
 */
static inline uint32_t bg_uint32_to_float(uint32_t mantissa, int32_t exponent)
{
  return (mantissa & 0xffffff) | (uint32_t)(exponent << 24);
}

/**
 * Create temperature measurement value from IEEE float and temperature type flag
 */
static inline void bg_thermometer_create_measurement(uint8_t* buffer, uint32_t measurement, int fahrenheit)
{
  buffer[0] = fahrenheit ? bg_thermometer_temperature_measurement_flag_units : 0;
  buffer[1] = measurement & 0xff;
  buffer[2] = (measurement >> 8) & 0xff;
  buffer[3] = (measurement >> 16) & 0xff;
  buffer[4] = (measurement >> 24) & 0xff;
}
static  void  App_TaskThermometer(void *p_arg)
{
  RTOS_ERR      err;
  int temperature_counter = 20;

  /* Prevent compiler warning for not using 'p_arg'       */
  (void)&p_arg;

  /* Task body, always written as an infinite loop.       */
  while (DEF_TRUE) {
    OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_DLY | OS_OPT_TIME_HMSM_NON_STRICT, &err);

    temperature_counter++;
    if (temperature_counter > 40) {
      temperature_counter = 20;
    }

    uint8_t temp_buffer[5];
    bg_thermometer_create_measurement(temp_buffer,
                                      bg_uint32_to_float(temperature_counter, 0),
                                      0);
    gecko_cmd_gatt_server_send_characteristic_notification(0xff, gattdb_temp_measurement, 5, temp_buffer);
  }
}
