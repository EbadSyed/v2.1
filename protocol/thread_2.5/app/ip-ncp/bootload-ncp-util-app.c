// File: bootload-ncp-util-app.c
//
// Description: This application is used to send an NCP into bootload mode or request NCP stack version information.
//
// Copyright 2016 Silicon Laboratories, Inc.                                *80*
//----------------------------------------------------------------
// Build with:
// make -f ./app/ip-ncp/bootload-ncp-util-app.mak
//
// This app requires two arguments, the USB tty driver -uart <USB tty driver>
// and either --launchbootloader or --version
//
// For example:
// sudo build/bootload-ncp-util-app/bootload-ncp-util-app -uart /dev/ttyUSB0 --version

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

#define ARG_LENGTH 40
#define STRINGIFY(x) #x
#define STRINGIFYX(x) STRINGIFY(x)

#define FOUR_SECOND_TIMEOUT 4000
#define STANDALONE_BOOTLOADER_NORMAL_MODE 1

#define EMBER_READ   read
#define EMBER_WRITE  write
#define EMBER_SELECT select

#define USE_STUB_emberGetNodeId
#define USE_STUB_emIsDefaultGlobalPrefix
#define USE_STUB_emIsLocalIpAddress
#define USE_STUB_emMacExtendedId
#define USE_STUB_emStoreDefaultGlobalPrefix
#include "stack/ip/stubs.c"

typedef enum {
  NO_ACTION,
  LAUNCH_BOOTLOADER,
  GET_VERSIONS
} AppAction;

static void printVersionJSON(const uint8_t *versionName,
                             uint16_t managementVersionNumber,
                             uint16_t stackVersionNumber,
                             uint16_t stackBuildNumber,
                             EmberVersionType versionType,
                             const uint8_t *buildTimestamp);

EventQueue emApiAppEventQueue;

static AppAction appAction = NO_ACTION;

bool verbose = false;
bool stackIsInitialized = false;
bool sentCommand = false;
bool logEnabled = false;

void launchStandaloneBootloader(void)
{
  emSendBinaryManagementCommand(EMBER_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER,
                                "u",
                                STANDALONE_BOOTLOADER_NORMAL_MODE);
}

void getVersionsCommand(void)
{
  emSendBinaryManagementCommand(EMBER_GET_VERSIONS_COMMAND_IDENTIFIER, "");
}

void launchBootloaderAshCallback(void)
{
  if (appAction == LAUNCH_BOOTLOADER) {
    emberSerialPrintfLine(APP_SERIAL, "Launched Bootloader");
    appAction = NO_ACTION;
  }
}

void getVersionsReturnCallback(void)
{
  if (appAction == GET_VERSIONS) {
    uint8_t *versionName;
    uint16_t managementVersionNumber;
    uint16_t stackVersionNumber;
    uint16_t stackBuildNumber;
    EmberVersionType versionType;
    uint8_t *buildTimestamp;
    versionName = emberStringCommandArgument(0, NULL);
    managementVersionNumber = (uint16_t) emberUnsignedCommandArgument(1);
    stackVersionNumber = (uint16_t) emberUnsignedCommandArgument(2);
    stackBuildNumber = (uint16_t) emberUnsignedCommandArgument(3);
    versionType = (EmberVersionType) emberUnsignedCommandArgument(4);
    buildTimestamp = emberStringCommandArgument(5, NULL);
    printVersionJSON(versionName,
                     managementVersionNumber,
                     stackVersionNumber,
                     stackBuildNumber,
                     versionType,
                     buildTimestamp);
    appAction = NO_ACTION;
  }
}

static const char * const versionTypeNames[] = {
  EMBER_VERSION_TYPE_NAMES
};

void printVersionJSON(const uint8_t *versionName,
                      uint16_t managementVersionNumber,
                      uint16_t stackVersionNumber,
                      uint16_t stackBuildNumber,
                      EmberVersionType versionType,
                      const uint8_t *buildTimestamp)
{
  emberSerialPrintf(APP_SERIAL, "{\"stackType\": \"%s\", ", versionName);
  emberSerialPrintf(APP_SERIAL, "\"stackVersion\": \"%u.%u.%u.%u\", ",
                    (stackVersionNumber & 0xF000) >> 12,
                    (stackVersionNumber & 0x0F00) >> 8,
                    (stackVersionNumber & 0x00F0) >> 4,
                    (stackVersionNumber & 0x000F));
  if (versionType <= EMBER_VERSION_TYPE_MAX) {
    emberSerialPrintf(APP_SERIAL, "\"versionType\": \"%s\", ", versionTypeNames[versionType]);
  }
  emberSerialPrintf(APP_SERIAL, "\"buildNumber\": %u, ", stackBuildNumber);
  emberSerialPrintf(APP_SERIAL, "\"managementNumber\": %u, ", managementVersionNumber);
  emberSerialPrintfLine(APP_SERIAL, "\"timeStamp\": \"%s\"} ", buildTimestamp);
}

static void initReturnCallback(void)
{
  EmberStatus status;
  status = (EmberStatus) emberUnsignedCommandArgument(0);
  // If initialization fails, we have to assert because something is wrong.
  // Whenever the stack initializes, the application and plugins must be
  // reinitialized.
  emberSerialPrintfLine(APP_SERIAL, "Init: 0x%x", status);
  assert(status == EMBER_SUCCESS);
  stackIsInitialized = true;
}

const EmberCommandEntry managementCallbackCommandTable[] = {
  emberBinaryCommand(CB_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER,
                     launchBootloaderAshCallback,
                     "",
                     NULL),
  emberBinaryCommand(CB_GET_VERSIONS_COMMAND_IDENTIFIER,
                     getVersionsReturnCallback,
                     "bvvvub",
                     NULL),
  emberBinaryCommand(CB_INIT_COMMAND_IDENTIFIER,
                     initReturnCallback,
                     "u",
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

const EmberCommandEntry emberCommandTable[] = {
  emberCommandEntryTerminator(),
};

static void signalHandler(int signal)
{
  close(driverNcpFd);
}

static void printUsage(void)
{
  printf("usage: bootload-ncp-util-app --uart tty_port --launchbootloader [--verbose]\n"
         "usage: bootload-ncp-util-app --uart tty_port --version [--verbose]\n\n"
         "--uart:            required, its value is the uart file\n"
         "--launchbootloader launches the ncp bootloader\n"
         "--version          returns the ncp version information\n"
         "--verbose          view all TX'd and RX'd ASHv3 frames\n"
         "Note: --launchbootloader and --version cannot be given together\n");
}

static void initialize(int argc, char **argv)
{
  char uartArg[ARG_LENGTH + 1] = { 0 };
  bool uartSet = false;
  bool breakout = false;

  while (!breakout) {
    static struct option long_options[] = {
      { "uart", required_argument, 0, 'u' },
      { "launchbootloader", no_argument, 0, 'l' },
      { "version", no_argument, 0, 'g' },
      { "verbose", no_argument, 0, 'v' },
      { 0, 0, 0, 0 }
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "u:ve:b", long_options, &option_index);

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
      case 'l':
        appAction = LAUNCH_BOOTLOADER;
        break;
      case 'g':
        appAction = GET_VERSIONS;
        break;
      case '?':
        printUsage();
        exit(1);
        break;
      case 'u':
        sscanf(optarg, "%" STRINGIFYX(ARG_LENGTH) "s", uartArg);
        uartSet = true;
        break;
      case 'v':
        verbose = true;
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

// Messages from the management client are forwarded to the NCP.
bool halHostUartTxIdle(void)
{
  return true;
}
void halHostUartLinkTx(const uint8_t *data, uint16_t length)
{
  if (EMBER_WRITE(driverNcpFd, data, length) != length) {
    fprintf(stderr, "error: cannot write to the uart\n");
    exit(1);
  }

  emAshReallyNotifyTxComplete(false);
}

// stubs
void emberXOnHandler(void)
{
}

void emberXOffHandler(void)
{
}

void txBufferFullHandler(const uint8_t *packet,
                         uint16_t packetLength,
                         uint16_t written)
{
}

void txFailedHandler(uint8_t fd,
                     const uint8_t *packet,
                     uint16_t packetLength,
                     uint16_t written)
{
}

Stream dataStream = { { 0 } };

void dataHandler(const uint8_t *packet,
                 SerialLinkMessageType type,
                 uint16_t length)
{
}

//----------------------------------------------------------------
// Main

const char *emAppName = "bootload-ncp-util-app";

int main(MAIN_FUNCTION_PARAMETERS)
{
  halInit();
  INTERRUPTS_ON();
  emberSerialInit(1, BAUD_115200, PARITY_NONE, 1);
  initialize(argc, argv);

  if (verbose) {
    emLogConfig(EM_LOG_ASHV3, APP_SERIAL, true);
  }

  emInitializeEventQueue(&emApiAppEventQueue);
  emInitializeBuffers();

  uartLinkReset();
  emberInit();

  uint32_t initTimeMs = halCommonGetInt32uMillisecondTick();
  uint32_t timeout = initTimeMs + FOUR_SECOND_TIMEOUT;

  while (appAction != NO_ACTION) {
    halResetWatchdog();
    tick();

    // Check if we have reached the timeout to fail gracefully
    uint32_t nowMs = halCommonGetInt32uMillisecondTick();
    if (nowMs > timeout) {
      emberSerialPrintfLine(APP_SERIAL, "Failed");
      break;
    }

    // Wait until the stack is initialized before allowing the application to
    // send any commands.
    if (!stackIsInitialized) {
      continue;
    }

    if (!sentCommand) {
      if (appAction == LAUNCH_BOOTLOADER) {
        sentCommand = true;
        launchStandaloneBootloader();
      } else if (appAction == GET_VERSIONS) {
        sentCommand = true;
        getVersionsCommand();
      }
    }
  }

  return 0;
}
