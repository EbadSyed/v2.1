// File: ash-v3-test-app.c
//
// Description: An ASHv3 test application.
//
// Copyright 2014 by Silicon Laboratories. All rights reserved.             *80*
//----------------------------------------------------------------

//
// Compile this app with jam:
//
// build.pl ash-v3-test-app HAL=micro PLAT=unix COMP=gcc MICRO=host PHY=null BOARD=host
//
// This app requires two arguments, the USB tty driver, and the tunnel
// interface:
// -u <USB tty driver> -t <TUN>
//
// For example:
// sudo build/ash-v3-test-app-unix-host/ash-v3-test-app -u /dev/ttyUSB0 -t tun0
//
// Add the --verbose argument to view all TX'd and RX'd ASHv3 frames
//

// For sigHandler
#define _POSIX_C_SOURCE 1

#include <sys/types.h>
#include <sys/time.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include PLATFORM_HEADER
#include "stack/core/ember-stack.h"
#include "stack/framework/event-queue.h"
#include "hal/hal.h"
#include "plugin/serial/serial.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/ip-ncp/binary-management.h"

#include "uart-link-protocol.h"
#include "host-stream.h"
#include "ip-driver.h"
#include "hal/micro/generic/ash-v3.h"
#include "app/ip-ncp/ncp-uart-interface.h"

#ifdef UNIX_HOST
#  define EMBER_READ   read
#  define EMBER_WRITE  write
#  define EMBER_SELECT select
#  include "ip-driver-log.h"
#  define LOG(x) x
#else
// simulated I/O for testing
#  include "tool/simulator/child/posix-sim.h"
#  define LOG(x)
#define USE_STUB_halButtonIsr
#define USE_STUB_halInternalStartSymbolTimer
#endif

#define USE_STUB_emberGetNodeId
#define USE_STUB_emIsDefaultGlobalPrefix
#define USE_STUB_emIsLocalIpAddress
#define USE_STUB_emMacExtendedId
#define USE_STUB_emStoreDefaultGlobalPrefix
#include "stack/ip/stubs.c"

#define ARG_LENGTH 40
#define STRINGIFY(x) #x
#define STRINGIFYX(x) STRINGIFY(x)

typedef enum {
  NO_TEST,
  TEST_BOOTSTRAP,
  TEST_ECHO,
  TEST_XON_XOFF
} TestMode;

typedef enum {
  NO_XON_XOFF_TEST_STATE,
  XON_SENT,
  DATA_SENT,
  ECHO_RECEIVED
} XOnXOffTestState;

static XOnXOffTestState xOnXOffTestState = NO_XON_XOFF_TEST_STATE;
static TestMode testMode = NO_TEST;
static bool displayPrompt = true;
static uint8_t echoArgument[ARG_LENGTH + 1];
bool verbose = false;
bool logEnabled = false;
EventQueue emApiAppEventQueue;
static uint16_t echoRepeaterIterations = 0;
static uint32_t echoRepeaterStartTime = 0;
static uint32_t echoRepeaterBytes = 0;

static void echoRepeaterEventHandler(void);
static void xOnXOffTestEventHandler(void);
static EmberEventControl echoRepeaterEvent = { 0 };
static EmberEventControl xOnXOffTestEvent = { 0 };

static EmberEventData events[] = {
  { &echoRepeaterEvent, echoRepeaterEventHandler },
  { &xOnXOffTestEvent, xOnXOffTestEventHandler },
  { NULL }
};

//----------------------------------------------------------------
// Interface to ip-driver.c.

// connects to Host
int driverManagementFd = -1;

Stream dataStream = { { 0 } };
Stream managementStream = { { 0 } };

// What management clients connect to.
int mgmtListenFd = -1;   // listen for managment client
bool ipDriverUseAsh = true;

// Data messages from the IP stack are forwarded to the NCP.

void dataHandler(const uint8_t *packet, SerialLinkMessageType type, uint16_t length)
{
  LOG(ipDriverLogEvent(LOG_IP_STACK_TO_NCP, packet, length, 0); )
  emLogBytesLine(IP_MODEM, "DATA app->ncp ", packet, length);
  ipModemWrite(driverNcpFd, UART_LINK_TYPE_THREAD_DATA, packet, length);
}

// Messages from the management client are forwarded to the NCP.

bool halHostUartTxIdle(void)
{
  // we're always idle
  return true;
}

void emNotifyTxComplete(void)
{
}

void halHostUartLinkTx(const uint8_t *data, uint16_t length)
{
  if (EMBER_WRITE(driverNcpFd, data, length) != length) {
    fprintf(stderr, "error: cannot write to the uart\n");
    exit(1);
  }

  if (ipDriverUseAsh) {
    emAshReallyNotifyTxComplete(false);
  }
}

static void quitCommand(void)
{
  exit(1);
}

static AshState wantedAshState = ASH_STATE_RESET_TX_PRE;
static int32_t echoCount = 0;
static uint16_t echoDelay = 0xFFFF;
static Buffer expectedEchoRxQueue = NULL_BUFFER;
static uint16_t maxEchoLength = MAX_ASH_PAYLOAD_SIZE;
static uint16_t echoLength = 1;
static int32_t echoWatermark = 0;

static void printSuccess(void)
{
  emberSerialPrintfLine(APP_SERIAL, "Success");
}

static void printFailure(void)
{
  emberSerialPrintfLine(APP_SERIAL, "Failure");
}

void txBufferFullHandler(const uint8_t *packet,
                         uint16_t packetLength,
                         uint16_t written)
{
  emberSerialPrintfLine(APP_SERIAL,
                        "warning: ASH's buffers are full. "
                        "tried to write %u bytes but was only able to write %u",
                        packetLength,
                        written);
}

void txFailedHandler(uint8_t fd,
                     const uint8_t *packet,
                     uint16_t packetLength,
                     uint16_t written)
{
  assert(false);
}

void emberAshStatusHandler(AshState state)
{
  assert(state < ASH_STATE_LAST);

  // this call to emberSerialPrintfLine() sometimes causes the simulator
  // to claim that serial port 3 is not initialized and exit

  //static const char *stateStrings[] = {ASH_STATE_STRINGS};
  //if (wantedAshState != state) {
  //  emberSerialPrintfLine(APP_SERIAL,
  //                        "State mismatch. I wanted: %s, "
  //                        "but the new state is: %s",
  //                        stateStrings[wantedAshState],
  //                        stateStrings[state]);
  //}

  if (wantedAshState < ASH_STATE_LAST) {
    wantedAshState += 1;
  }

  if (state == ASH_STATE_RESET_TX_PRE) {
    echoCount = 0;
    echoWatermark = 0;
    expectedEchoRxQueue = NULL_BUFFER;
  }

  if (state == ASH_STATE_RUNNING) {
    emberSerialPrintfLine(APP_SERIAL, "ASHv3 is up");

    if (testMode == TEST_BOOTSTRAP) {
      printSuccess();
      // we're done
      exit(1);
    }
  }
}

static void resetAshCommand(void)
{
  wantedAshState = ASH_STATE_RESET_TX_PRE;
  uartLinkReset();
}

static void printBytes(const uint8_t *data, uint8_t length)
{
  uint8_t i;

  for (i = 0; i < length; i++) {
    emberSerialPrintf(APP_SERIAL,
                      "%X%s",
                      data[i],
                      (i < length - 1
                       ? " "
                       : ""));
  }
}

void emberEcho(const uint8_t *data, uint8_t length)
{
  echoCount++;
  echoWatermark++;

  emberSerialPrintf(APP_SERIAL,
                    "@%u self -> NCP: Echo %u bytes: [",
                    halCommonGetInt32uMillisecondTick(),
                    length);
  printBytes(data, length);
  emberSerialPrintfLine(APP_SERIAL, "]");

  emSendBinaryManagementCommand(EMBER_ECHO_COMMAND_IDENTIFIER,
                                "b",
                                data,
                                length);

  Buffer echoData = emFillBuffer(data, length);
  assert(echoData != NULL_BUFFER);
  if (expectedEchoRxQueue == NULL_BUFFER) {
    expectedEchoRxQueue = echoData;
  } else {
    Buffer previous = expectedEchoRxQueue;
    while (emGetQueueLink(previous) != NULL_BUFFER) {
      previous = emGetQueueLink(previous);
    }
    emSetQueueLink(previous, echoData);
  }
}

static void echoCommand(void)
{
  uint8_t length;
  uint8_t *data = emberStringCommandArgument(0, &length);
  emberEcho(data, length);
}

void emberStopEchoRepeater(void)
{
  echoDelay = 0xFFFF;
  echoCount = 0;
  expectedEchoRxQueue = NULL_BUFFER;
  emberSerialPrintfLine(APP_SERIAL, "Stopped echo repeater.");
  emberEventControlSetInactive(echoRepeaterEvent);
}

static uint32_t xOnCount = 0;
static uint32_t xOffCount = 0;
static uint32_t xOnTime = 0;
static uint32_t xOffTime = 0;

void emberXOnHandler(void)
{
  xOnCount++;
  xOnTime = halCommonGetInt32uMillisecondTick();
  emberSerialPrintfLine(APP_SERIAL, "RX XON");
}

void emberXOffHandler(void)
{
  xOffCount++;
  xOffTime = halCommonGetInt32uMillisecondTick();

  // when we get an XOFF we reset the XON counter
  xOnCount = 0;

  emberSerialPrintfLine(APP_SERIAL, "RX XOFF");
}

void startXOnXOffTestCommand(void)
{
  emSendBinaryManagementCommand(EMBER_START_XON_XOFF_TEST_COMMAND_IDENTIFIER, "");
}

static void printOk(void)
{
  emberSerialPrintfLine(APP_SERIAL, "OK");
}

void startXOnXOffTestCallback(void)
{
  printOk();
}

void sendBigDataCommand(void)
{
  // send wakeup bytes
  uint8_t wakeups[100];
  uint8_t i;

  for (i = 0; i < sizeof(wakeups); i++) {
    wakeups[i] = 0xFF;
  }

  assert(EMBER_WRITE(driverNcpFd, wakeups, sizeof(wakeups)));
  printOk();
}

void stopXOnXOffTestCommand(void)
{
  if (xOffCount > 0
      // 5 is the minimum number of XONs ip-modem-app will send
      && xOnCount >= 5
      && timeGTorEqualInt16u(xOnTime, xOffTime)) {
    emberSerialPrintfLine(APP_SERIAL, "Passed");
  } else {
    emberSerialPrintfLine(APP_SERIAL,
                          "Failed: %u %u %u %u",
                          xOnCount,
                          xOffCount,
                          xOnTime,
                          xOffTime);
  }
}

void echoCallback(void)
{
  if (expectedEchoRxQueue == NULL_BUFFER) {
    return;
  }
  Buffer expected = expectedEchoRxQueue;
  expectedEchoRxQueue = emGetQueueLink(expected);

  echoWatermark--;

  uint8_t length;
  const uint8_t *data = emberStringCommandArgument(0, &length);

  emberSerialPrintf(APP_SERIAL,
                    "@%u self <- NCP: Echo %u bytes: [",
                    halCommonGetInt32uMillisecondTick(),
                    length);
  printBytes(data, length);
  emberSerialPrintfLine(APP_SERIAL, "]");

  uint16_t i;
  uint16_t expectedLength = emGetBufferLength(expected);
  const uint8_t *expectedBytes = emGetBufferPointer(expected);
  bool success = true;

  if (expectedLength != length) {
    //
    // FAILURE
    //
    emberSerialPrintf(APP_SERIAL,
                      "Length mismatch, expected %u but got %u\n",
                      expectedLength,
                      length);
    emberStopEchoRepeater();
    success = false;
  } else {
    for (i = 0; i < length; i++) {
      if (expectedBytes[i] != data[i]) {
        //
        // FAILURE
        //
        emberSerialPrintf(APP_SERIAL,
                          "Byte mismatch at index %u, expected %d but got %d\n",
                          i,
                          expectedBytes[i],
                          data[i]);
        emberStopEchoRepeater();
        success = false;
      }
    }
  }

  if (success) {
    // we sent and received 'length' bytes
    echoRepeaterBytes += length * 2;
  }

  if (testMode == TEST_ECHO) {
    emberSerialWaitSend(APP_SERIAL);

    if (success) {
      printSuccess();
    } else {
      printFailure();
    }

    // we're done
    exit(1);
  }
  if (testMode == TEST_XON_XOFF) {
    if ((xOnXOffTestState == DATA_SENT) && success) {
      xOnXOffTestState = ECHO_RECEIVED;
    }
  }
}

void resetNcpAshCommand(void)
{
  emberSerialPrintfLine(APP_SERIAL, "Resetting NCP's ASH");
  emSendBinaryManagementCommand(EMBER_RESET_NCP_ASH_COMMAND_IDENTIFIER, "");
}

void configUartCommand(void)
{
  printOk();
  uint8_t dropPercentage = emberUnsignedCommandArgument(0);
  uint8_t corruptPercentage = emberUnsignedCommandArgument(1);
  emAshConfigUart(dropPercentage, corruptPercentage);
  emSendBinaryManagementCommand(EMBER_CONFIG_UART_COMMAND_IDENTIFIER,
                                "uu",
                                dropPercentage,
                                corruptPercentage);
}

static void runEvents(void)
{
  emberRunEvents(events);
}

static void echoRepeaterEventHandler(void)
{
  assert(echoLength > 0 && echoLength <= maxEchoLength);
  Buffer echoBuffer = emAllocateBuffer(echoLength);
  assert(echoBuffer != NULL_BUFFER);
  uint8_t *echoData = emGetBufferPointer(echoBuffer);
  uint16_t i = 0;

  for (i = 0; i < echoLength; i++) {
    echoData[i] = rand();
  }

  emberEcho(echoData, echoLength);
  echoLength = (echoLength % maxEchoLength) + 1;

  if (echoRepeaterIterations > 0) {
    emberSerialPrintfLine(APP_SERIAL,
                          "%u iterations left",
                          echoRepeaterIterations);
    emberEventControlSetDelayMS(echoRepeaterEvent, echoDelay);
    echoRepeaterIterations--;
  } else {
    //uint32_t runtime = (halCommonGetInt32uMillisecondTick()
    //                    - echoRepeaterStartTime);
    //emberSerialPrintfLine(APP_SERIAL,
    //                      "%u bytes pushed in %u ms: %u b/s",
    //                      echoRepeaterBytes,
    //                      runtime,
    //                      (int)((float)(echoRepeaterBytes * 1000) / (float)runtime));
    echoRepeaterBytes = 0;
    emberStopEchoRepeater();
  }
}

static uint32_t testStartTime = 0;
static void xOnXOffTestEventHandler(void)
{
  // create the data
  uint8_t echoData[75] = { 0 };
  uint8_t i;

  for (i = 0; i < sizeof(echoData); i++) {
    echoData[i] = i;
  }

  if (xOnXOffTestState == XON_SENT) {
    emberEcho(echoData, sizeof(echoData));
    testStartTime = halCommonGetInt32uMillisecondTick();
    emberEventControlSetDelayMS(xOnXOffTestEvent, 100);
    xOnXOffTestState = DATA_SENT;
  } else if (xOnXOffTestState == DATA_SENT) {
    if (halCommonGetInt32uMillisecondTick() - testStartTime < 6000) {
      emberEventControlSetDelayMS(xOnXOffTestEvent, 100);
    } else {
      emberSerialPrintfLine(APP_SERIAL, "Test Failed: Timeout waiting for echo");
      exit(1);
    }
  } else if (xOnXOffTestState == ECHO_RECEIVED) {
    if (halCommonGetInt32uMillisecondTick() - testStartTime < 3000) {
      emberSerialPrintfLine(APP_SERIAL, "Test Failed: Host not held off (echo returned too quickly)");
    } else {
      emberSerialPrintfLine(APP_SERIAL, "Success");
    }
    // we're done
    exit(1);
  }
}

void emberStartEchoRepeater(uint16_t delay)
{
  emberSerialPrintfLine(APP_SERIAL, "Starting echo repeater");
  expectedEchoRxQueue = NULL_BUFFER;
  echoDelay = delay;
  echoLength = 1;
  echoCount = 0;
  echoRepeaterBytes = 0;
  echoRepeaterEventHandler();
}

void startEchoRepeaterCommand(void)
{
  echoRepeaterStartTime = halCommonGetInt32uMillisecondTick();
  echoRepeaterIterations = emberUnsignedCommandArgument(0);
  emberStartEchoRepeater(emberUnsignedCommandArgument(1));
}

void stopEchoRepeaterCommand(void)
{
  emberStopEchoRepeater();
}

void emberAssertInfoReturn(const uint8_t *fileName, uint32_t lineNumber)
{
  emberSerialPrintfLine(APP_SERIAL,
                        "** NCP Assert: file name: %s line: %u",
                        fileName,
                        lineNumber);
}

void assertInfoCallback(void)
{
  emberAssertInfoReturn(emberStringCommandArgument(0, NULL),
                        emberUnsignedCommandArgument(1));
}

void resetNcpAshCallback(void)
{
  emberSerialPrintfLine(APP_SERIAL, "NCP's ASH is up.");
}

void helpCommand(void)
{
  EmberCommandEntry *finger = emberCommandTable;
  emberSerialPrintf(APP_SERIAL, "\r\nCommands: \r\n\r\n");

  // Every emberCommandEntryAction should have a valid 'action'
  // OR valid 'argumentTypes'
  // (For example, emberCommandEntrySubMenus have their actions dictated by
  // their nested commands, specified by the 'argumentTypes')
  for (;
       (finger->action != NULL
        || finger->argumentTypes != NULL);
       finger++) {
    emberPrintCommandUsage(finger);
  }
  emberSerialPrintf(APP_SERIAL, "\r\n");
  emberPrintCommandUsageNotes();
}

const EmberCommandEntry managementCallbackCommandTable[] = {
  emberBinaryCommand(CB_ECHO_COMMAND_IDENTIFIER,
                     echoCallback,
                     "b",
                     NULL),
  emberBinaryCommand(CB_ASSERT_INFO_COMMAND_IDENTIFIER,
                     assertInfoCallback,
                     "bw",
                     NULL),
  emberBinaryCommand(CB_RESET_NCP_ASH_COMMAND_IDENTIFIER,
                     resetNcpAshCallback,
                     "",
                     NULL),
  emberCommandEntryTerminator(),
};

static EmberCommandState callbackCommandState = { 0 };

void emProcessNcpManagementCommand(SerialLinkMessageType type,
                                   const uint8_t *message,
                                   uint16_t length)
{
  emberRunBinaryCommandInterpreter(&callbackCommandState,
                                   managementCallbackCommandTable,
                                   emberCommandErrorHandler,
                                   message + 1,
                                   length - 1);
}

void emLogCommand(void);

const EmberCommandEntry emberCommandTable[] = {
  emberCommand("reset_ash",
               resetAshCommand,
               "",
               "reset our ASH link"),
  emberCommand("reset_ncp_ash",
               resetNcpAshCommand,
               "",
               "reset the NCP's ASH link"),
  emberCommand("echo",
               echoCommand,
               "b",
               "perform an echo"),
  emberCommand("config_uart",
               configUartCommand,
               "uu",
               "configure corruption and dropped packets"),
  emberCommand("start_echo_repeater",
               startEchoRepeaterCommand,
               "vv",
               "start the automatic echo repeater. "
               "argument 0: the max number of iterations. "
               "argument 1: the delay, in ms, between packets"),
  emberCommand("stop_echo_repeater",
               stopEchoRepeaterCommand,
               "",
               "stop the automatic echo repeater"),
  emberCommand("start_xon_xoff_test",
               startXOnXOffTestCommand,
               "",
               "start the XON/XOFF test"),
  emberCommand("stop_xon_xoff_test",
               stopXOnXOffTestCommand,
               "",
               "stop the XON/XOFF test"),
  emberCommand("quit",
               quitCommand,
               "",
               "quit"),
  emberCommand("log",
               emLogCommand,
               "ub*",
               "turn on logging"),
  emberCommand("log_off",
               emLogCommand,
               "ub*",
               "turn off logging"),
  emberCommand("help",
               helpCommand,
               "",
               "show this help"),
  emberCommandEntryTerminator(),
};

#ifndef EMBER_TEST
static void signalHandler(int signal)
{
  close(driverNcpFd);
}

static void tick(void)
{
  fd_set input;
  FD_ZERO(&input);
  FD_SET(driverNcpFd, &input);

  int maxFd = driverNcpFd;

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 100000;

  int selectResult = select(maxFd + 1, &input, NULL, NULL, &timeout);
  assert(selectResult >= 0);

  if (selectResult > 0) {
    emCheckNcpUartInput(&input);
  }
}
#endif

#ifdef EMBER_TEST
static void assertState(IpModemReadStatus status)
{
  assert(status != IP_MODEM_READ_FORMAT_ERROR
         && status != IP_MODEM_READ_IO_ERROR);
}

static void tickSimulation(void)
{
  assertState(readIpv6Input(driverDataFd, &dataStream, UART_LINK_TYPE_THREAD_DATA, dataHandler));
  assertState(readIpModemAshInput(driverNcpFd, ncpMessageHandler));
  // try to push data to ASH
  emLoadSerialTx();
  simulatedTimePasses();
}

static void initializeSimulation(void)
{
  driverNcpFd = 1;
  driverDataFd = 2;
  emberSerialInit(1, BAUD_115200, PARITY_NONE, 1); // data socket
  emberSerialInit(2, BAUD_115200, PARITY_NONE, 1); // management socket
  emberSerialInit(APP_SERIAL, BAUD_115200, PARITY_NONE, 1);
}
#endif

#ifndef EMBER_TEST
static void printUsage(void)
{
  printf("usage: ash-v3-test-app --uart uart_file [-f r,x] [--test-echo echo_string] "
         "[--test-bootstrap] [--verbose]\n\n"
         "--uart:           required, its value is the uart file\n"
         "--flow-control    flow control: r=RTS/CTS, x=XON/XOFF\n"
         "--test-echo:      perform an echo test with the given string\n"
         "--test-bootstrap: perform a bootstrap test\n\n"
         "Note: --test-echo and --test-bootstrap cannot be given together\n");
}

static void initialize(int argc, char **argv)
{
  char uartArg[ARG_LENGTH + 1] = { 0 };
  bool uartSet = false;
  bool breakout = false;

  while (!breakout) {
    static struct option long_options[] = {
      { "uart", required_argument, 0, 'u' },
      { "flow-control", required_argument, 0, 'f' },
      { "test-echo", required_argument, 0, 'e' },
      { "test-bootstrap", no_argument, 0, 'b' },
      { "test-xon-xoff", no_argument, 0, 'x' },
      { "verbose", no_argument, 0, 'v' },
      { 0, 0, 0, 0 }
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "u:f:ve:b", long_options, &option_index);

    if (c == -1) {
      if (option_index != argc && option_index != 0) {
        fprintf(stderr, "Unexpected argument %s\n\n", argv[option_index]);
        printUsage();
        exit(1);
      }
    }

    switch (c) {
      case 0:
        break;
      case '?':
        printUsage();
        exit(1);
        break;
      case 'u':
        sscanf(optarg, "%" STRINGIFYX(ARG_LENGTH) "s", uartArg);
        uartSet = true;
        break;
      case 'b':
        testMode = TEST_BOOTSTRAP;
        break;
      case 'e':
        testMode = TEST_ECHO;
        sscanf(optarg, "%" STRINGIFYX(ARG_LENGTH) "s", echoArgument);
        break;
      case 'v':
        verbose = true;
        break;
      case 'x':
        testMode = TEST_XON_XOFF;
        break;
      case 'f':
        switch (*optarg) {
          case 'r':
            rtsCts = true;
            break;
          case 'x':
            rtsCts = false;
            break;
          default:
            fprintf(stderr, "Invalid flow control choice %s.\n", optarg);
            printUsage();
            exit(1);
        }
        break;
      default:
        breakout = true;
        break;
    }
  }

  if (!uartSet) {
    printUsage();
    exit(1);
  }

  emOpenNcpUart(uartArg);

  // configure signal handlers
  struct sigaction sigHandler;
  sigHandler.sa_handler = signalHandler;
  sigemptyset(&sigHandler.sa_mask);
  sigHandler.sa_flags = 0;
  sigaction(SIGINT, &sigHandler, NULL);
  sigaction(SIGTERM, &sigHandler, NULL);
  sigaction(SIGABRT, &sigHandler, NULL);
}
#endif

bool initIpDriver(int argc, char **argv);

//----------------------------------------------------------------
// Main

#ifdef UNIX_HOST
  #define RETURN_TYPE int
#else
  #define RETURN_TYPE void
  #define argc 0
  #define argv NULL
#endif

const char *emAppName = "ash-v3-test-app";
extern EmberCommandState emCommandState0;

void emberMarkApplicationBuffersHandler(void)
{
  emMarkBuffer(&expectedEchoRxQueue);
  emberMarkEventQueue(&emApiAppEventQueue);
}

static BufferMarker markers[] = {
  emberMarkApplicationBuffersHandler,
  NULL
};

RETURN_TYPE main(MAIN_FUNCTION_PARAMETERS)
{
  halInit();
  INTERRUPTS_ON();
  emberSerialInit(1, BAUD_115200, PARITY_NONE, 1);

#ifdef EMBER_TEST
  initializeSimulation();
#else
  initialize(argc, argv);
#endif

  if (verbose) {
    emLogConfig(EM_LOG_ASHV3, APP_SERIAL, true);
  }

  emInitializeEventQueue(&emApiAppEventQueue);
  emInitializeBuffers();

  ncpUartUseAsh = true;
  uartLinkReset();
  emberInit();
  emberCommandReaderInit();

  const char *prompt = "ash-v3-test-app";

  if (testMode == TEST_BOOTSTRAP) {
    displayPrompt = false;
  } else if (testMode == TEST_ECHO) {
    displayPrompt = false;
    emberEcho(echoArgument, strlen((char*)echoArgument));
  } else if (testMode == TEST_XON_XOFF) {
    displayPrompt = false;
    startXOnXOffTestCommand();
    xOnXOffTestState = XON_SENT;
    emberEventControlSetDelayMS(xOnXOffTestEvent, 500);
    emberSerialPrintfLine(APP_SERIAL,
                          "\nTesting XON/XOFF, please wait "
                          "for a few seconds...\n");
  }

  if (displayPrompt) {
    emberSerialPrintf(APP_SERIAL, "%s> ", prompt);
  }

  while (true) {
    halResetWatchdog();
    emReclaimUnusedBuffers(markers);
    emberRunEventQueue(&emApiAppEventQueue);
    runEvents();

    emCommandState0.defaultBase = 10;

    if (displayPrompt && emberProcessCommandInput(APP_SERIAL)) {
      emberSerialPrintf(APP_SERIAL, "%s> ", prompt);
    }

#ifdef EMBER_TEST
    tickSimulation();
#else
    tick();
#endif
  }

#ifdef UNIX_HOST
  return 0;
#endif
}
