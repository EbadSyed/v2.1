#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <signal.h>

#ifdef __linux__
  #include <linux/if.h>
  #include <linux/if_tun.h>
#endif

#include PLATFORM_HEADER
#include "stack/core/ember-stack.h"

// These are in hal/micro/serial.h, which we normally get from hal/hal.h.
// However, serial.h has an enumeration called PARITY_NONE that conflicts with
// a #define in linux/if.h.
void emLoadSerialTx(void);
uint16_t halHostEnqueueTx(const uint8_t *data, uint16_t length);

#include "app/ip-ncp/uart-link-protocol.h"
#include "app/ip-ncp/ip-modem-link.h"
#include "hal/micro/generic/ash-v3.h"
#include "app/ip-ncp/host-stream.h"
#include "app/ip-ncp/ip-driver.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/util/serial/command-interpreter2-util.h"
#include "app/ip-ncp/ncp-uart-interface.h"

bool ncpUartUseAsh = true;
bool rtsCts = true;
int driverNcpFd = -1;
// connects to TUN->Host
int driverDataFd = -1;
// connects to joiner port of commission-proxy-app
int driverCommProxyAppDataFd = -1;

static Stream ncpStream = { { 0 } };
static Stream outgoingDataStream = { { 0 } };
static bool loadingSerialTx = false;
static uint16_t ashIndex = 0;
static Stream ashStream = { { 0 } };

#if defined(EMBER_NCP_RESET_ENABLE) && !defined(UNIX_HOST_SIM)
#if defined(UNIX_HOST)
static int fdDirection, fdValue;
#ifdef EMBER_NCP_RESET_REQUIRE_ENABLE
static bool resetNcpEnabled = false;
#else
static bool resetNcpEnabled = true;
#endif
#endif

// Define gpio files. The usual filenames for gpio xx are:
// GPIO_EXPORT_FILE          /sys/class/gpio/export
// GPIO_DIRECTION_FILE       /sys/class/gpio/gpioxx/direction
// GPIO_ACTIVE_LOW_FILE      /sys/class/gpio/gpioxx/active_low
// GPIO_VALUE_FILE           /sys/class/gpio/gpioxx/value
#define GPIO_EXPORT_FILE     EMBER_GPIO_PATH "export"
#define GPIO_DIRECTORY       EMBER_GPIO_PATH "gpio" EMBER_NCP_RESET_GPIO "/"
#define GPIO_DIRECTION_FILE  GPIO_DIRECTORY "direction"
#define GPIO_ACTIVE_LOW_FILE GPIO_DIRECTORY "active_low"
#define GPIO_VALUE_FILE      GPIO_DIRECTORY "value"
#define OPEN_MODE            (S_IWUSR | S_IWOTH | S_IRUSR | S_IROTH)
#define RESET_TIME_US        50
#ifdef EMBER_NCP_RESET_ASSERT_LOW
#define ASSERT_VALUE    "0"
#define DEASSERT_VALUE  "1"
#else
#define ASSERT_VALUE    "1"
#define DEASSERT_VALUE  "0"
#endif
#ifdef EMBER_NCP_RESET_DEASSERT_TRISTATED
#define TRISTATE_GPIO_IF_ENABLED()  write(fdDirection, "in", 2)
#else
#define TRISTATE_GPIO_IF_ENABLED()
#endif

#endif // #if defined(EMBER_NCP_RESET_ENABLE) && !defined(UNIX_HOST_SIM)

// Define CRTSCTS for both ingoing and outgoing hardware flow control
// Try to resolve the numerous aliases for the bit flags
#if defined(CCTS_OFLOW) && defined(CRTS_IFLOW) && !defined(__NetBSD__)
  #undef CRTSCTS
  #define CRTSCTS (CCTS_OFLOW | CRTS_IFLOW)
#endif
#if defined(CTSFLOW) && defined(RTSFLOW)
  #undef CRTSCTS
  #define CRTSCTS (CTSFLOW | RTSFLOW)
#endif
#ifdef CNEW_RTSCTS
  #undef CRSTCTS
  #define CRTSCTS CNEW_RTSCTS
#endif
#ifndef CRTSCTS
  #define CRTSCTS 0
#endif

// Define the termios bit fields modified by ezspSerialInit
// (CREAD is omitted as it is often not supported by modern hardware)
#define CFLAG_MASK (CLOCAL | CSIZE | PARENB | HUPCL | CRTSCTS)
#define IFLAG_MASK (IXON | IXOFF | IXANY | BRKINT | INLCR | IGNCR | ICRNL \
                    | INPCK | ISTRIP | IMAXBEL)
#define LFLAG_MASK (ICANON | ECHO | IEXTEN | ISIG)
#define OFLAG_MASK ( OPOST )

void emResetSerialState(bool external)
{
  MEMSET(&ncpStream, 0, sizeof(Stream));
  ncpStream.needsFrameSynchronization = true;
  MEMSET(&outgoingDataStream, 0, sizeof(Stream));

  if (!external) {
    MEMSET(&ashStream, 0, sizeof(Stream));
  }

  ashIndex = 0;
}

#define UART_BAUD B115200

#ifdef UNIX_HOST
  #define EMBER_READ   read
  #define EMBER_WRITE  write
  #define EMBER_SELECT select
  #include "ip-driver-log.h"
  #define LOG(x) x
#else
// simulated I/O for testing
  #include "tool/simulator/child/posix-sim.h"
  #define LOG(x)
#endif

void emOpenNcpUart(const char *uartDevice)
{
  struct termios tios, checkTios;
  int i;
  driverNcpFd = open(uartDevice, O_RDWR | O_NOCTTY | O_NONBLOCK);

  if (driverNcpFd < 0) {
    perror("uart open failed");
    exit(1);
  }

  tcflush(driverNcpFd, TCIOFLUSH);
  fcntl(driverNcpFd, F_SETFL, O_NONBLOCK);
  tcgetattr(driverNcpFd, &tios);
  cfsetispeed(&tios, UART_BAUD);
  cfsetospeed(&tios, UART_BAUD);
  tios.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
  tios.c_cflag |= (CLOCAL | CREAD | CS8);
  tios.c_iflag &= ~(BRKINT | INLCR | IGNCR | ICRNL | INPCK
                    | ISTRIP | IMAXBEL | IXOFF | IXANY);
  if (rtsCts) {
    tios.c_cflag |= CRTSCTS;
    tios.c_iflag &= ~IXON;
  } else {
    tios.c_cflag &= ~CRTSCTS;
    tios.c_iflag |= IXON;
  }
  tios.c_lflag &= ~(ICANON | ECHO | IEXTEN | ISIG);
  tios.c_oflag &= ~OPOST;
  memset(tios.c_cc, _POSIX_VDISABLE, NCCS);
  tios.c_cc[VSTART] = CSTART;
  tios.c_cc[VSTOP] = CSTOP;
  tios.c_cc[VMIN] = 1;
  tios.c_cc[VTIME] = 0;
  tcsetattr(driverNcpFd, TCSAFLUSH, &tios);
  tcgetattr(driverNcpFd, &checkTios);      // and read back the result

  while (true) {
    // Verify that the fields written have the right values
    i = (tios.c_cflag ^ checkTios.c_cflag) & CFLAG_MASK;
    if (i) {
      fprintf(stderr,
              "Termios cflag(s) in error: 0x%04X\r\n",
              i);
      break;
    }
    i = (tios.c_iflag ^ checkTios.c_iflag) & IFLAG_MASK;
    if (i) {
      fprintf(stderr,
              "Termios iflag(s) in error: 0x%04X\r\n",
              i);
      break;
    }
    i = (tios.c_lflag ^ checkTios.c_lflag) & LFLAG_MASK;
    if (i) {
      fprintf(stderr,
              "Termios lflag(s) in error: 0x%04X\r\n",
              i);
      break;
    }
    i = (tios.c_oflag ^ checkTios.c_oflag) & OFLAG_MASK;
    if (i) {
      fprintf(stderr,
              "Termios oflag(s) in error: 0x%04X\r\n",
              i);
      break;
    }
    for (i = 0; i < NCCS; i++) {
      if (tios.c_cc[i] != checkTios.c_cc[i]) {
        break;
      }
    }
    if (i != NCCS) {
      fprintf(stderr,
              "Termios error at c_cc[%d]\r\n",
              i);
      break;
    }
    return;
  }
  exit(1);
}

uint16_t ipModemWrite(int fd,
                      SerialLinkMessageType type,
                      const uint8_t *buffer,
                      uint16_t bufferLength)
{
  uint16_t result = 0;

  if (fd < 0) {
    return result;
  }

  uint16_t bigBufferLength = bufferLength + UART_LINK_HEADER_SIZE;
  uint8_t *bigBuffer = malloc(bufferLength + UART_LINK_HEADER_SIZE);

  bigBuffer[0] = '[';
  bigBuffer[1] = type;
  emberStoreHighLowInt16u(bigBuffer + 2, bufferLength);
  MEMCOPY(bigBuffer + UART_LINK_HEADER_SIZE, buffer, bufferLength);

  // if we're sending to the NCP, work with ASH
  if (ncpUartUseAsh
      && fd == driverNcpFd) {
    if (outgoingDataStream.index + bigBufferLength
        < sizeof(outgoingDataStream.buffer)) {
      MEMCOPY(outgoingDataStream.buffer + outgoingDataStream.index,
              bigBuffer,
              bigBufferLength);
      outgoingDataStream.index += bigBufferLength;
      result = bufferLength;
      emLoadSerialTx();
    } else {
      // This shouldn't happen because we make sure there's space before
      // reading from the socket.
      txBufferFullHandler(buffer,
                          bufferLength,
                          sizeof(outgoingDataStream.buffer)
                          - outgoingDataStream.index);
    }
  } else {
    result = EMBER_WRITE(fd, bigBuffer, bigBufferLength);

    if (result != bigBufferLength) {
      txFailedHandler(fd, bigBuffer, bigBufferLength, result);
    }
  }

  free(bigBuffer);
  return result;
}

// Data messages from the NCP are forwarded to the IP stack; all
// others are processed by ip-driver.c.

void ncpMessageHandler(SerialLinkMessageType type,
                       const uint8_t *message,
                       uint16_t length)
{
  bool isDataForHost = (type == UART_LINK_TYPE_THREAD_DATA
                        || type == UART_LINK_TYPE_ALT_DATA
                        || type == UART_LINK_TYPE_UNSECURED_DATA
                        || type == UART_LINK_TYPE_COMMISSIONER_DATA);
  LOG(ipDriverLogEvent((isDataForHost
                        ? LOG_NCP_TO_IP_STACK
                        : LOG_NCP_TO_DRIVER_MGMT),
                       message,
                       length,
                       type); )

  if (isDataForHost) {
    if (type == UART_LINK_TYPE_COMMISSIONER_DATA
        && driverCommProxyAppDataFd != -1) {
      // forwarded to commission-proxy-app
      LOG(ipDriverLogEvent(LOG_DRIVER_TO_COMM_PROXY_APP, message, length, type); )
      ipModemWrite(driverCommProxyAppDataFd, type, message, length);
    } else if (driverDataFd != -1) {
      assert(EMBER_WRITE(driverDataFd, message, length) == length);
    }
  } else {
    ncpDriverMessageHandler(type, message, length);
  }
}

void emTestIpModemReadStatusResult(IpModemReadStatus status, Stream *stream)
{
#ifdef EMBER_TEST
  if (stream->readStatusErrorHandler != NULL
      && stream->readStatusErrorHandler(status, stream)) {
    return;
  }
#endif
  if (status == IP_MODEM_READ_IO_ERROR) {
    perror("IO error (IP_MODEM_READ_IO_ERROR)");
  } else if (status == IP_MODEM_READ_FORMAT_ERROR) {
    uint16_t i;
#ifdef EMBER_TEST
    // For ASHv3 testing only
    fprintf(stderr, "\nashRxTestState.rawData: [");
    for (i = 0; i < ashRxTestState.rawDataIndex; i++) {
      fprintf(stderr, "%X ", ashRxTestState.rawData[i]);
    }
    fprintf(stderr,
            "] | CRC should be: 0x%4X\n",
            emGetAshCrc(ashRxTestState.rawData, MAX_ASH_PACKET_SIZE - 3));
#endif
    fprintf(stderr,
            "Management formatting error (IP_MODEM_READ_FORMAT_ERROR), "
            "%u bytes: ",
            stream->index);

    for (i = 0; i < stream->index; i++) {
      fprintf(stderr, "%X ", stream->buffer[i]);
    }

    fprintf(stderr, "\n");
    assert(false);
  } else {
    if (!(status == IP_MODEM_READ_DONE
          || status == IP_MODEM_READ_PROGRESS
          || status == IP_MODEM_READ_PENDING
          || status == IP_MODEM_READ_EOF)) {
      fprintf(stderr, "failed status: %u\n", status);
      assert(false);
    }
  }
}

#ifdef EMBER_TEST
Stream *emGetNcpStream(void)
{
  return &ncpStream;
}
#endif

uint16_t serialCopyFromRx(const uint8_t *data, uint16_t length)
{
  if (!simNotifySerialRx(data, length, NULL_BUFFER)) {
    return length;
  }

  assert(length > 0);
  MEMCOPY(ncpStream.buffer + ncpStream.index, data, length);
  ncpStream.index += length;
  uint16_t index;

  do {
    index = ncpStream.index;
    IpModemReadStatus result =
      processIpModemInput(&ncpStream, ncpMessageHandler);
    emTestIpModemReadStatusResult(result, &ncpStream);

    // do while we have eaten a little
  } while (ncpStream.index >= UART_LINK_HEADER_SIZE
           && ncpStream.index < index);

  return length;
}

IpModemReadStatus readIpModemAshInput(int fd,
                                      IpModemMessageHandler ncpMessageHandler)
{
  assert(ncpUartUseAsh);

  int got = EMBER_READ(fd,
                       ashStream.buffer + ashStream.index,
                       sizeof(ashStream.buffer) - ashStream.index);

  if (got < 0) {
    return IP_MODEM_READ_IO_ERROR;
  } else if (got > 0) {
    // eat ASH input

    // ashIndex is in the range: [ashStream.index, ashStream.index + got]
    // it corresponds to the current ash input index
    // it may be reset
    ashIndex = ashStream.index;
    ashStream.index += got;

    // totalEaten monotonically increases and is in the range: [0, got]
    // it corresponds to how much data has been eaten from ashStream.buffer
    uint16_t totalEaten = 0;

    do {
      uint16_t increment = emProcessAshRxInput(ashStream.buffer + ashIndex,
                                               got - totalEaten);

      if (increment == 0) {
        break;
      }

      totalEaten += increment;
      ashIndex += increment;

      if (isAshActive()) {
        ashIndex += increment;
      } else if (ashStream.index > 0) {
        // ^^ reset from ASH was a possibility, check that ashStream
        // still has valid data before removing bytes from it below
        emRemoveStreamBytes(&ashStream, ashIndex);
        ashIndex = 0;
      }
    } while (totalEaten < got);
  } else if (ncpStream.index > 0) {
    processIpModemInput(&ncpStream, ncpMessageHandler);
  }

  return (ashStream.index == 0
          ? IP_MODEM_READ_DONE
          : IP_MODEM_READ_PROGRESS);
}

void emLoadSerialTx(void)
{
  assert(ncpUartUseAsh);

  if (loadingSerialTx) {
    // no loops
    return;
  }

  loadingSerialTx = true;

  // enqueue as much data as possible
  if (outgoingDataStream.index > 0) {
    uint16_t eaten =
      halHostEnqueueTx(outgoingDataStream.buffer,
                       outgoingDataStream.index >= MAX_ASH_PAYLOAD_SIZE
                       ? MAX_ASH_PAYLOAD_SIZE
                       : outgoingDataStream.index);

    if (eaten > 0) {
      emRemoveStreamBytes(&outgoingDataStream, eaten);
      uartFlushTx();
    }
  }

  loadingSerialTx = false;
}

bool emHaveOutgoingBufferSpace(void)
{
  return (outgoingDataStream.index + EMBER_IPV6_MTU
          < sizeof(outgoingDataStream.buffer));
}

void emCheckNcpUartInput(fd_set *input)
{
  // EMIPSTACK-1847: If we're using ASH, make sure we have sufficient outgoing
  // buffer space prior to reading the packet from the tun0 interface.  If
  // there's not enough space, we let things buffer up in the IP stack
  // rather than asserting due to a full buffer error.
  if (driverDataFd != -1
      && (!ncpUartUseAsh || emHaveOutgoingBufferSpace())
      && FD_ISSET(driverDataFd, input)) {
    IpModemReadStatus status =
      readIpv6Input(driverDataFd, &dataStream, UART_LINK_TYPE_THREAD_DATA, dataHandler);
    assert(status != IP_MODEM_READ_FORMAT_ERROR
           && status != IP_MODEM_READ_IO_ERROR);
  }

  if (FD_ISSET(driverNcpFd, input)) {
    IpModemReadStatus status;

    if (ncpUartUseAsh) {
      status = readIpModemAshInput(driverNcpFd, ncpMessageHandler);
    } else {
      status = readIpModemInput(driverNcpFd, &ncpStream, ncpMessageHandler);
    }

    switch (status) {
      case IP_MODEM_READ_FORMAT_ERROR:
        assert(false);
        break;
      case IP_MODEM_READ_IO_ERROR:
        // what to do?
        break;
      default:
        break;
    }
  }
}

void emOpenTunnel(const char *tunName)
{
  char tunDevice[200] = { 0 };
  strcat(tunDevice, "/dev/");

#if (defined __APPLE__) || (defined __CYGWIN__)
  strcat(tunDevice, tunName);
#else
  strcat(tunDevice, "net/tun");
#endif

  driverDataFd = open(tunDevice, O_RDWR | O_NONBLOCK);

  if (driverDataFd < 0) {
    perror("tunnel open failed");
    exit(1);
  }

#if !(defined __APPLE__) && !(defined __CYGWIN__)
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
  strncpy(ifr.ifr_name, tunName, IFNAMSIZ);
  int status = ioctl(driverDataFd, TUNSETIFF, (void *) &ifr);

  if (status < 0) {
    perror("TUNSETIFF ioctl failed");
    close(driverDataFd);
    exit(1);
  }
#endif
}

void emReallySendManagementCommand(const uint8_t *command, uint16_t length)
{
  emLogBytesLine(IP_MODEM, "MGMT driver->ncp:", command, length);
  LOG(ipDriverLogEvent(LOG_DRIVER_TO_NCP_MGMT, command, length, UART_LINK_TYPE_MANAGEMENT); )
  ipModemWrite(driverNcpFd,
               UART_LINK_TYPE_MANAGEMENT,
               command + UART_LINK_HEADER_SIZE,
               length - UART_LINK_HEADER_SIZE);
}

void ncpDriverMessageHandler(SerialLinkMessageType type,
                             const uint8_t *message,
                             uint16_t length)
{
  emCommandState.defaultBase = 0xFF;

  switch (type) {
    case UART_LINK_TYPE_THREAD_DATA:
    case UART_LINK_TYPE_ALT_DATA:
    case UART_LINK_TYPE_UNSECURED_DATA:
      assert(false);    // should have been handled by caller
      break;
    case UART_LINK_TYPE_MANAGEMENT: {
      uint8_t managementType = message[0];
      const uint8_t *body = message + 1;

      if (managementType == MANAGEMENT_COMMAND) {
        emLogBytesLine(IP_MODEM,
                       "MGMT ncp->app %d ",
                       body,
                       length - 3,    // skip managmentType and newlines
                       managementType);
      }

      emProcessNcpManagementCommand(type, message, length);
      break;
    }
    default:
      break;
  }
}

void emSendHostIpv6(SerialLinkMessageType type,
                    const uint8_t *packet,
                    uint16_t length)
{
  emLogBytesLine(IP_MODEM, "DATA driver->ncp ", packet, length);
  LOG(ipDriverLogEvent(LOG_DRIVER_TO_NCP_DATA, packet, length, type));
  ipModemWrite(driverNcpFd, type, packet, length);
}

#if defined(EMBER_NCP_RESET_ENABLE) && !defined(UNIX_HOST_SIM) && defined(UNIX_HOST)

// Open the gpio used to reset the ncp and leave it deasserted
void openNcpReset(void)
{
  int fdExport, fdActiveLow;

  fdExport = open(GPIO_EXPORT_FILE, O_WRONLY, OPEN_MODE);
  write(fdExport, EMBER_NCP_RESET_GPIO, strlen(EMBER_NCP_RESET_GPIO));
  close(fdExport);

  fdDirection = open(GPIO_DIRECTION_FILE, O_WRONLY, OPEN_MODE);
  if (fdDirection < 0) {
    perror("ncp reset gpio open failed");
    exit(1);
  }
  write(fdDirection, "in", 2);

  fdActiveLow = open(GPIO_ACTIVE_LOW_FILE, O_WRONLY, OPEN_MODE);
  write(fdActiveLow, "0", 1);
  close(fdActiveLow);

  fdValue = open(GPIO_VALUE_FILE, O_WRONLY, OPEN_MODE);
  write(fdDirection, "out", 3);
  write(fdValue, DEASSERT_VALUE, 1);
  TRISTATE_GPIO_IF_ENABLED();
}

// If enabled, briefly assert the reset output to cause a pin reset on the ncp
void resetNcp(void)
{
  assert((fdDirection >= 0) && (fdValue >= 0));
  if (resetNcpEnabled) {
    write(fdDirection, "out", 3);
    write(fdValue, ASSERT_VALUE, 1);
    usleep(RESET_TIME_US);
    write(fdValue, DEASSERT_VALUE, 1);
    TRISTATE_GPIO_IF_ENABLED();
  }
}

void enableResetNcp(uint8_t enable)
{
  resetNcpEnabled = enable ? true : false;
}

#else
void openNcpReset(void)
{
}
void resetNcp(void)
{
}
void enableResetNcp(uint8_t enable)
{
}
#endif
