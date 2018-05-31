// File: management-client.c
//
// Description: Open a connection to the 6LoWPAN driver's management socket and
// send and receive commands from it.
//
// Copyright 2012 by Silicon Laboratories. All rights reserved.             *80*

#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <getopt.h>

#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "hal/hal.h"
#include "plugin/serial/serial.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/util/serial/command-interpreter2-util.h"
#include "app/ip-ncp/uart-link-protocol.h"
#include "app/ip-ncp/ip-modem-link.h"
#include "app/ip-ncp/binary-management.h"
#include "app/ip-ncp/host-stream.h"
#include "app/ip-ncp/data-client.h"
#include "app/util/ip/wakeup-commands.h"
#include "stack/ip/host/management-client.h"
#include "app/tmsp/tmsp-enum.h"

#ifdef EMBER_TEST
#include "stack/core/ember-stack.h"
#endif

#ifdef UNIX_HOST
  #define EMBER_READ   read
  #define EMBER_WRITE  write
  #define EMBER_SELECT select
#else
// simulated I/O for testing
  #include "tool/simulator/child/posix-sim.h"
#endif

#define STRINGIFY(x) #x
#define STRINGIFYX(x) STRINGIFY(x)

#define ARG_LENGTH 40
#define RESPONSE_TIMEOUT_MS 2000

//----------------------------------------------------------------
// Local state.

#ifdef EMBER_TEST
static int dataFd = 1;       // in simulation file descriptors are ports
static int managementFd = 2;
#else
static int managementFd = -1;
#endif

static Stream inputStream = { { 0 } };
static Stream inputCommandStream = { { 0 } };
static EmberCommandState callbackCommandState;

//----------------------------------------------------------------
// Connecting to the 6LoWPAN driver.

#ifdef EMBER_TEST

static Stream dataStream = { { 0 } };

void parseManagementArguments(int argc, char *argv[])
{
  emberSerialInit(dataFd, BAUD_115200, PARITY_NONE, 1);
  emberSerialInit(managementFd, BAUD_115200, PARITY_NONE, 1);
  emberInitializeCommandState(&callbackCommandState);
}

void emSendHostIpv6(SerialLinkMessageType type,
                    const uint8_t *packet,
                    uint16_t length)
{
  // ignore the type, app sends only MAC secured packets
  assert(EMBER_WRITE(dataFd, packet, length) == length);
}

#else // ifdef EMBER_TEST

int emConnectManagementSocket(uint16_t port)
{
  emberInitializeCommandState(&callbackCommandState);

  managementFd = socket(AF_INET6, SOCK_STREAM, 0);

  if (managementFd < 0) {
    perror("socket creation failed");
    return -1;
  }

  struct sockaddr_in6 address;
  memset(&address, 0, sizeof(struct sockaddr_in6));
  address.sin6_family = AF_INET6;
  address.sin6_addr.s6_addr[15] = 1;
  address.sin6_port = htons(port);

  if (connect(managementFd, (struct sockaddr *) &address, sizeof(address))
      != 0) {
    fprintf(stderr, "Failed to connect to management port %d\n", port);
    perror("");
    return -1;
  }

  return managementFd;
}

static void argumentError(void)
{
  fprintf(stderr, "Usage: -m mgmt_port\n");
  exit(1);
}

void parseManagementArguments(int argc, char *argv[])
{
  if (argc < 2) {
    argumentError();
  }
  char portArg[ARG_LENGTH + 1];
  memset(portArg, 0, sizeof(portArg));

  while (true) {
    int c = getopt(argc, argv, "m:");
    if (c == -1) {
      if (optind != argc) {
        fprintf(stderr, "Unexpected argument %s\n", argv[optind]);
        argumentError();
      }
      break;
    }
    switch (c) {
      case 'm':
        sscanf(optarg, "%" STRINGIFYX(ARG_LENGTH) "s", portArg);
        break;
      default:
        argumentError();
    }
  }
  int mgmtPort = atoi(portArg);
  if (mgmtPort <= 0 || mgmtPort > 65535) {
    fprintf(stderr, "Invalid management port number %d\n", mgmtPort);
    argumentError();
  }
  emConnectManagementSocket(mgmtPort);
}
#endif // ifdef EMBER_TEST

//----------------------------------------------------------------
// Receiving commands.

// debugging variable
static uint32_t receiveCount = 0;

static void handleManagementInput(SerialLinkMessageType messageType,
                                  const uint8_t *data,
                                  uint16_t length)
{
  receiveCount++;

  if (messageType == UART_LINK_TYPE_MANAGEMENT) {
    assert(1 <= length);
    ManagementType type = data[0];

    if (type == MANAGEMENT_COMMAND) {
      data += 1;        // skip over type
      length -= 1;
      assert(inputCommandStream.index + length
             < sizeof(inputCommandStream.buffer));

      // store the length
      inputCommandStream.buffer[inputCommandStream.index] = length;
      inputCommandStream.index += 1;

      // store the data
      memmove(inputCommandStream.buffer + inputCommandStream.index,
              data,
              length);
      inputCommandStream.index += length;
    }
  } else {
    assert(false);      // should only see managment messages
  }
}

static bool checkInput(void)
{
  assert(managementFd >= 0);
  fd_set input;
  FD_ZERO(&input);
  FD_SET(managementFd, &input);
  bool result = false;

  int maxFd = managementFd + 1;
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 100000;
  int n = EMBER_SELECT(maxFd, &input, NULL, NULL, &timeout);
  if (n < 0) {
    perror("select failed");
    exit(1);
  } else if (n == 0) {
    // Timeout.
  } else {
    if (FD_ISSET(managementFd, &input)) {
      processManagementInputStream();
      result = true;
    }
  }

#ifdef EMBER_TEST
  // In simulation we need to read the data stream directly.
  readIpv6Input(dataFd, &dataStream, UART_LINK_TYPE_THREAD_DATA, emHostIpv6Dispatch);
#endif

  return result;
}

extern EmberCommandEntry managementCallbackCommandTable[];

static void processManagementCommand(void)
{
  if (inputCommandStream.index > 0) {
    // emLogCharsLine(IP_MODEM, "APP read command ",
    //                inputCommandStream.buffer,
    //                inputCommandStream.index);
    const uint8_t *finger;

    //
    // start at the first byte in buffer
    // inputCommandStream.index is the farthest we can go
    // the length of each mangement packet is stored at the first byte that
    // finger points to
    //
    for (finger = inputCommandStream.buffer;
         (finger - inputCommandStream.buffer) < inputCommandStream.index;
         finger += *finger + 1) {
      emberRunBinaryCommandInterpreter(&callbackCommandState,
                                       managementCallbackCommandTable,
                                       emberCommandErrorHandler,
                                       finger + 1,
                                       *finger);
    }

    inputCommandStream.index = 0;
    MEMSET(inputCommandStream.buffer, 0, sizeof(inputCommandStream.buffer));
  }
}

void processManagementInputStream(void)
{
  IpModemReadStatus status =
    readIpModemInput(managementFd, &inputStream, handleManagementInput);
  if (status == IP_MODEM_READ_FORMAT_ERROR
      || status == IP_MODEM_READ_IO_ERROR) {
    fprintf(stderr, "corrupt input\n");
    exit(1);
  } else {
    processManagementCommand();
  }
}

void managementCommandTick(void)
{
  checkInput();
}

//----------------------------------------------------------------
// Sending commands.

void emReallySendManagementCommand(const uint8_t *command, uint16_t length)
{
  assert(EMBER_WRITE(managementFd, command, length) == length);
}
