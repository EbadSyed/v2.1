/***************************************************************************//**
 * @file demo.h
 * @copyright Copyright 2017 Silicon Laboratories, Inc. http://www.silabs.com
 ******************************************************************************/

// Define exactly one of these to support the BLE+ZB or BLE_RAIL light service UUID
// Note: UUID in GATT DB still needs to be changed manually.
//#define BLE_ZB_DMP_APP
#define BLE_RAIL_DMP_APP

// This is to disable source address indication if mobile app does not support it.
// Otherwise this needs to be excluded.
//#define DISABLE_SOURCE_ADDRESS_INDICATION

typedef enum {
  DEMO_EVT_NONE                     = 0x00,
  DEMO_EVT_BOOTED                   = 0x01,
  DEMO_EVT_BLUETOOTH_CONNECTED      = 0x02,
  DEMO_EVT_BLUETOOTH_DISCONNECTED   = 0x03,
  DEMO_EVT_RAIL_CONNECTED           = 0x04,
  DEMO_EVT_RAIL_DISCONNECTED        = 0x05,
  DEMO_EVT_LIGHT_CHANGED_BLUETOOTH  = 0x06,
  DEMO_EVT_LIGHT_CHANGED_RAIL       = 0x07,
  DEMO_EVT_INDICATION               = 0x08,
  DEMO_EVT_INDICATION_SUCCESSFUL    = 0x09,
  DEMO_EVT_INDICATION_FAILED        = 0x0A,
  DEMO_EVT_BUTTON_PRESSED           = 0x0B,
  DEMO_EVT_CLEAR_DIRECTION          = 0x0C
} DemoEvt;

// Queue for handling demo events
extern OS_Q  demoEventQueue;

#define demoEventQueuePost(demoEvent, error) OSQPost( \
    (OS_Q *)&demoEventQueue,                          \
    (void *)demoEvent,                                \
    (OS_MSG_SIZE)sizeof(void *),                      \
    (OS_OPT)OS_OPT_POST_FIFO + OS_OPT_POST_ALL,       \
    (RTOS_ERR *)error)

#define bleFlagSetPost(bleEvent, error) OSFlagPost( \
    (OS_FLAG_GRP*)&bluetooth_event_flags,           \
    (OS_FLAGS)bleEvent,                             \
    OS_OPT_POST_FLAG_SET,                           \
    (RTOS_ERR *)error)

#define bleFlagClrPost(bleEvent, error) OSFlagPost( \
    (OS_FLAG_GRP*)&bluetooth_event_flags,           \
    (OS_FLAGS)bleEvent,                             \
    OS_OPT_POST_FLAG_CLR,                           \
    (RTOS_ERR *)error)

/******************************************************************************
 * Configuration Utility Functions
 *****************************************************************************/
