// File: management-serial.c
//
// Description: Implementation of the Ember serial functions for IP NCP
// management.
//
// Copyright 2012 by Silicon Laboratories. All rights reserved.             *80*

#include PLATFORM_HEADER
#include "stack/include/ember-types.h"
#include "stack/include/byte-utilities.h"
#include "stack/include/ember-debug.h"
#include "hal/hal.h"
#include "stack/include/error.h"
#include "plugin/serial/serial.h"
#include "app/ip-ncp/serial-link.h"
#include "app/ip-ncp/uart-link-protocol.h"
#ifdef CORTEXM3_EFM32_MICRO
#include "em_device.h"
#include "plugin/serial/com.h"
#endif
#include "plugin/serial/ember-printf.h"

#ifdef EMBER_TEST

// When this file is part of a library, we won't have a CONFIGURATION_HEADER,
// which means we won't have any of the EMBER_SERIALX_MODE defines, and
// hal/micro/serial.h will set them all to EMBER_SERIAL_UNUSED.  That's bad, at
// least for testing.  When we run into this case, just manually define two
// serial ports, because we know we need them for testing.
#ifndef CONFIGURATION_HEADER
  #undef EMBER_SERIAL0_MODE
  #define EMBER_SERIAL0_MODE EMBER_SERIAL_FIFO
  #define EMBER_SERIAL0_RX_QUEUE_SIZE 128
  #define EMBER_SERIAL0_TX_QUEUE_SIZE 128

  #undef EMBER_SERIAL1_MODE
  #define EMBER_SERIAL1_MODE EMBER_SERIAL_FIFO
  #define EMBER_SERIAL1_RX_QUEUE_SIZE 128
  #define EMBER_SERIAL1_TX_QUEUE_SIZE 128
#endif

// Can't use a typedef because the qSizes might be different.
#define DEFINE_FIFO_QUEUE(qSize, qName) \
  static struct {                       \
    uint16_t head;                      \
    uint16_t tail;                      \
    volatile uint16_t used;             \
    uint8_t fifo[qSize];                \
  } qName;

DEFINE_FIFO_QUEUE(EMBER_SERIAL0_TX_QUEUE_SIZE, emSerial0TxQueue)
DEFINE_FIFO_QUEUE(EMBER_SERIAL0_RX_QUEUE_SIZE, emSerial0RxQueue)
#define EM_SERIAL0_TX_QUEUE_ADDR (&emSerial0TxQueue)
#define EM_SERIAL0_RX_QUEUE_ADDR (&emSerial0RxQueue)

DEFINE_FIFO_QUEUE(EMBER_SERIAL1_TX_QUEUE_SIZE, emSerial1TxQueue)
DEFINE_FIFO_QUEUE(EMBER_SERIAL1_RX_QUEUE_SIZE, emSerial1RxQueue)
#define EM_SERIAL1_TX_QUEUE_ADDR (&emSerial1TxQueue)
#define EM_SERIAL1_RX_QUEUE_ADDR (&emSerial1RxQueue)

// Not everyone defines sizes for ports 2 and 3

#ifdef EMBER_SERIAL2_TX_QUEUE_SIZE
DEFINE_FIFO_QUEUE(EMBER_SERIAL2_TX_QUEUE_SIZE, emSerial2TxQueue)
DEFINE_FIFO_QUEUE(EMBER_SERIAL2_RX_QUEUE_SIZE, emSerial2RxQueue)
  #define EM_SERIAL2_TX_QUEUE_ADDR (&emSerial2TxQueue)
  #define EM_SERIAL2_RX_QUEUE_ADDR (&emSerial2RxQueue)
#else
  #define EMBER_SERIAL2_TX_QUEUE_SIZE 0
  #define EMBER_SERIAL2_RX_QUEUE_SIZE 0
  #define EM_SERIAL2_TX_QUEUE_ADDR NULL
  #define EM_SERIAL2_RX_QUEUE_ADDR NULL
#endif

#ifdef EMBER_SERIAL3_TX_QUEUE_SIZE
DEFINE_FIFO_QUEUE(EMBER_SERIAL3_TX_QUEUE_SIZE, emSerial3TxQueue)
DEFINE_FIFO_QUEUE(EMBER_SERIAL3_RX_QUEUE_SIZE, emSerial3RxQueue)
  #define EM_SERIAL3_TX_QUEUE_ADDR (&emSerial3TxQueue)
  #define EM_SERIAL3_RX_QUEUE_ADDR (&emSerial3RxQueue)
#else
  #define EMBER_SERIAL3_TX_QUEUE_SIZE 0
  #define EMBER_SERIAL3_RX_QUEUE_SIZE 0
  #define EM_SERIAL3_TX_QUEUE_ADDR NULL
  #define EM_SERIAL3_RX_QUEUE_ADDR NULL
#endif

void *emSerialTxQueues[EM_NUM_SERIAL_PORTS] =
{ FOR_EACH_PORT((void *), EM_SERIAL, _TX_QUEUE_ADDR) };

uint16_t PGM emSerialTxQueueSizes[EM_NUM_SERIAL_PORTS] =
{ FOR_EACH_PORT((uint16_t), EMBER_SERIAL, _TX_QUEUE_SIZE) };

uint16_t PGM emSerialTxQueueMasks[EM_NUM_SERIAL_PORTS] =
{ FOR_EACH_PORT((uint16_t), EMBER_SERIAL, _TX_QUEUE_SIZE - 1) };

uint16_t PGM emSerialTxQueueWraps[EM_NUM_SERIAL_PORTS] =
{ FOR_EACH_PORT((uint16_t), EMBER_SERIAL, _TX_QUEUE_SIZE) };

EmSerialFifoQueue *emSerialRxQueues[EM_NUM_SERIAL_PORTS] =
{ FOR_EACH_PORT((EmSerialFifoQueue *), EM_SERIAL, _RX_QUEUE_ADDR) };

uint16_t PGM emSerialRxQueueSizes[EM_NUM_SERIAL_PORTS] =
{ FOR_EACH_PORT((uint16_t), EMBER_SERIAL, _RX_QUEUE_SIZE) };

uint16_t PGM emSerialRxQueueWraps[EM_NUM_SERIAL_PORTS] =
{ FOR_EACH_PORT((uint16_t), EMBER_SERIAL, _RX_QUEUE_SIZE) };

uint8_t PGM emSerialPortModes[EM_NUM_SERIAL_PORTS] =
{ FOR_EACH_PORT((uint8_t), EMBER_SERIAL, _MODE) };

#endif

EmberStatus emberSerialInit(uint8_t port,
                            SerialBaudRate rate,
                            SerialParity parity,
                            uint8_t stopBits)
{
  EmberStatus status = EMBER_SUCCESS;
#ifdef CORTEXM3_EFM32_MICRO
  if (port == COM_VCP) {
    status = COM_Init((COM_Port_t)port, NULL);
  }
#endif
  return status;
}

EmberStatus emberSerialReadByte(uint8_t port, uint8_t *dataByte)
{
  EmberStatus status = EMBER_SERIAL_RX_EMPTY;
#ifdef CORTEXM3_EFM32_MICRO
  if (port == COM_VCP) {
    status = COM_ReadByte((COM_Port_t)port, dataByte);
  }
#endif
  return status;
}

EmberStatus emberSerialWaitSend(uint8_t port)
{
  EmberStatus status = EMBER_SUCCESS;
#ifdef CORTEXM3_EFM32_MICRO
  if (port == COM_VCP) {
    status = COM_WaitSend((COM_Port_t)port);
  }
#endif
  return status;
}

EmberStatus emberSerialGuaranteedPrintf(uint8_t port, PGM_P formatString, ...)
{
  EmberStatus stat;
  va_list ap;
  va_start(ap, formatString);
  stat = emberSerialPrintfVarArg(port, formatString, ap);
  va_end(ap);
  emberSerialWaitSend(port);
  return stat;
}

static Ecode_t managementWrite(COM_Port_t port,
                               uint8_t *contents,
                               uint8_t length)
{
  emDebugSendVuartMessage(contents, length);
  return EMBER_SUCCESS;
}

EmberStatus emberSerialPrintfVarArg(uint8_t port, PGM_P string, va_list args)
{
  if (emPrintfInternal(managementWrite, (COM_Port_t)port, string, args)) {
    return EMBER_SUCCESS;
  } else {
    return EMBER_ERR_FATAL;
  }
}

EmberStatus emberSerialWriteData(uint8_t port, uint8_t *data, uint8_t length)
{
  managementWrite((COM_Port_t)port, data, length);
  return EMBER_SUCCESS;
}

EmberStatus emberSerialWriteString(uint8_t port, PGM_P string)
{
  managementWrite((COM_Port_t)port,
                  (uint8_t *)string,
                  emStrlen((uint8_t *)string));
  return EMBER_SUCCESS;
}

uint16_t emberSerialWriteAvailable(uint8_t port)
{
  assert(false);
  return 0;
}
