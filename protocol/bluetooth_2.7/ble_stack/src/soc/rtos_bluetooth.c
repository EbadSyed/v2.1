#include <stdint.h>
#include <string.h>
#include <em_device.h>
#include <rtos/kernel/include/os.h>

#include "rtos_gecko.h"
#include "gecko_configuration.h"
#include "rtos_bluetooth.h"

#if defined(EMBER_AF_PLUGIN_BLE) && defined(EMBER_AF_PLUGIN_MICRIUM_RTOS)
void emberRtosStackWakeupIsrHandler(void);
#endif

void bluetooth_task(void *p);
void BluetoothUpdate();
volatile struct gecko_cmd_packet*  bluetooth_evt;
OS_MUTEX           BluetoothMutex;

volatile static uint32_t command_header;
volatile static void*    command_data;
volatile static gecko_cmd_handler command_handler_func = NULL;

//Bluetooth task
#define BLUETOOTH_STACK_SIZE (1500 / sizeof(CPU_STK))
static  void  BluetoothTask (void  *p_arg);
static  OS_TCB            BluetoothTaskTCB;
static  CPU_STK           BluetoothTaskStk[BLUETOOTH_STACK_SIZE];

//Linklayer task
#define LINKLAYER_STACK_SIZE (1000 / sizeof(CPU_STK))
static  void  LinklayerTask (void  *p_arg);
static  OS_TCB            LinklayerTaskTCB;
static  CPU_STK           LinklayerTaskStk[LINKLAYER_STACK_SIZE];

OS_FLAG_GRP bluetooth_event_flags;

void bluetooth_start_task(OS_PRIO ll_priority,
                          OS_PRIO stack_priority)
{
  RTOS_ERR os_err;

  OSFlagCreate(&bluetooth_event_flags,
               "Bluetooth Flags",
               (OS_FLAGS)0,
               &os_err);
  OSMutexCreate(&BluetoothMutex,
                "Bluetooth Mutex",
                &os_err);

  //create tasks for Bluetooth host stack
  OSTaskCreate(&BluetoothTaskTCB,
               "Bluetooth Task",
               BluetoothTask,
               0u,
               stack_priority,
               &BluetoothTaskStk[0u],
               BLUETOOTH_STACK_SIZE / 10u,
               BLUETOOTH_STACK_SIZE,
               0u,
               0u,
               0u,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &os_err);
  //create tasks for Linklayer
  OSTaskCreate(&LinklayerTaskTCB,
               "Linklayer Task",
               LinklayerTask,
               0u,
               ll_priority,
               &LinklayerTaskStk[0u],
               LINKLAYER_STACK_SIZE / 10u,
               LINKLAYER_STACK_SIZE,
               0u,
               0u,
               0u,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &os_err);
}
//This callback is called from interrupt context (Kernel Aware)
//sets flag to trigger Link Layer Task
void BluetoothLLCallback()
{
  RTOS_ERR os_err;
  OSFlagPost(&bluetooth_event_flags, (OS_FLAGS)BLUETOOTH_EVENT_FLAG_LL, OS_OPT_POST_FLAG_SET, &os_err);
}
//This callback is called from Bluetooth stack
//Called from kernel aware interrupt context (RTCC interrupt) and from Bluetooth task
//sets flag to trigger running Bluetooth stack
void BluetoothUpdate()
{
  RTOS_ERR os_err;
  OSFlagPost(&bluetooth_event_flags, (OS_FLAGS)BLUETOOTH_EVENT_FLAG_STACK, OS_OPT_POST_FLAG_SET, &os_err);
}
//Bluetooth task, it waits for events from bluetooth and handles them
void BluetoothTask(void *p)
{
  RTOS_ERR      os_err;
  OS_FLAGS      flags = BLUETOOTH_EVENT_FLAG_EVT_HANDLED | BLUETOOTH_EVENT_FLAG_STACK;

  while (DEF_TRUE) {
    //Command needs to be sent to Bluetooth stack
    if (flags & BLUETOOTH_EVENT_FLAG_CMD_WAITING) {
      uint32_t header = command_header;
      gecko_cmd_handler cmd_handler = command_handler_func;
      sli_bt_cmd_handler_delegate(header, cmd_handler, (void*)command_data);
      command_handler_func = NULL;
      flags &= ~BLUETOOTH_EVENT_FLAG_CMD_WAITING;
      OSFlagPost(&bluetooth_event_flags, (OS_FLAGS)BLUETOOTH_EVENT_FLAG_RSP_WAITING, OS_OPT_POST_FLAG_SET, &os_err);
    }

    //Bluetooth stack needs updating, and evt can be used
    if ((flags & BLUETOOTH_EVENT_FLAG_STACK)
        && (flags & BLUETOOTH_EVENT_FLAG_EVT_HANDLED)) {//update bluetooth & read event
      bluetooth_evt = gecko_wait_event();
      if (bluetooth_evt != NULL) {//we got event, notify event handler. evt state is now waiting handling
        OSFlagPost(&bluetooth_event_flags, (OS_FLAGS)BLUETOOTH_EVENT_FLAG_EVT_WAITING, OS_OPT_POST_FLAG_SET, &os_err);
        flags &= ~(BLUETOOTH_EVENT_FLAG_EVT_HANDLED);
#if defined(EMBER_AF_PLUGIN_BLE) && defined(EMBER_AF_PLUGIN_MICRIUM_RTOS)
        emberRtosStackWakeupIsrHandler();
#endif
      } else {//nothing to do in stack, clear the flag
        flags &= ~(BLUETOOTH_EVENT_FLAG_STACK);
      }
    }

    // Ask from Bluetooth stack how long we can sleep
    // UINT32_MAX = sleep indefinitely
    // 0 = cannot sleep, stack needs update and we need to check if evt is handled that we can actually update it
    uint32_t timeout = gecko_can_sleep_ticks();
    if (timeout == 0 && (flags & BLUETOOTH_EVENT_FLAG_EVT_HANDLED)) {
      flags |= BLUETOOTH_EVENT_FLAG_STACK;
      continue;
    }
    //OSFlagPend expects 0 to be indefinite
    if (timeout == UINT32_MAX) {
      timeout = 0;
    }
    flags |= OSFlagPend(&bluetooth_event_flags,
                        (OS_FLAGS)BLUETOOTH_EVENT_FLAG_STACK + BLUETOOTH_EVENT_FLAG_EVT_HANDLED + BLUETOOTH_EVENT_FLAG_CMD_WAITING,
                        timeout,
                        OS_OPT_PEND_BLOCKING + OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME,
                        NULL,
                        &os_err);
    if (RTOS_ERR_CODE_GET(os_err) == RTOS_ERR_TIMEOUT) {
      //timeout occurred, set the flag to update the Bluetooth stack
      flags |= BLUETOOTH_EVENT_FLAG_STACK;
    }
  }
}

static  void  LinklayerTask(void *p_arg)
{
  RTOS_ERR      os_err;
  (void)p_arg;

  while (DEF_TRUE) {
    OSFlagPend(&bluetooth_event_flags, (OS_FLAGS)BLUETOOTH_EVENT_FLAG_LL,
               0,
               OS_OPT_PEND_BLOCKING + OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME,
               NULL,
               &os_err);

    gecko_priority_handle();
  }
}

//hooks for API
//called from tasks using BGAPI
void rtos_gecko_handle_command(uint32_t header, void* payload)
{
  sli_bt_cmd_handler_rtos_delegate(header, NULL, payload);
}
void rtos_gecko_handle_command_noresponse(uint32_t header, void* payload)
{
  sli_bt_cmd_handler_rtos_delegate(header, NULL, payload);
}
void sli_bt_cmd_handler_rtos_delegate(uint32_t header, gecko_cmd_handler handler, const void* payload)
{
  RTOS_ERR os_err;

  command_header = header;
  command_handler_func = handler;
  command_data = (void*)payload;
  //Command structure is filled, notify the stack
  OSFlagPost(&bluetooth_event_flags, (OS_FLAGS)BLUETOOTH_EVENT_FLAG_CMD_WAITING, OS_OPT_POST_FLAG_SET, &os_err);
  //wait for response
  OSFlagPend(&bluetooth_event_flags, (OS_FLAGS)BLUETOOTH_EVENT_FLAG_RSP_WAITING,
             0,
             OS_OPT_PEND_BLOCKING + OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME,
             NULL,
             &os_err);
}

void BluetoothPend(RTOS_ERR *err)
{
  OSMutexPend((OS_MUTEX *)&BluetoothMutex,
              (OS_TICK   )0,
              (OS_OPT    )OS_OPT_PEND_BLOCKING,
              (CPU_TS   *)NULL,
              (RTOS_ERR *)err);
}
void BluetoothPost(RTOS_ERR *err)
{
  OSMutexPost((OS_MUTEX *)&BluetoothMutex,
              (OS_OPT    )OS_OPT_POST_NONE,
              (RTOS_ERR *)err);
}
