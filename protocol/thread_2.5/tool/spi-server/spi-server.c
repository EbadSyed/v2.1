// File: spi-server.c
//
// Description: Expose SPI transport over an IPv6 stream socket
//
// Copyright 2012 by Silicon Labs. All rights reserved.                     *80*

// For sigaction(2) and sigemptyset(3) in glibc.
#define _POSIX_C_SOURCE 1

#define PROGNAME "spi-server"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <net/if.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdbool.h>
#include <semaphore.h>

#if defined(__linux__) || defined(__linux)
  #include <linux/types.h>
  #include <linux/spi/spidev.h>
#else

// stubs

struct spi_ioc_transfer{
  unsigned long tx_buf;
  unsigned long rx_buf;
  int len;
  int delay_usecs;
  int speed_hz;
  int bits_per_word;
  bool cs_change;
};

  #define SPI_IOC_MESSAGE(x) 1
  #define SPI_IOC_WR_MODE 1
  #define SPI_IOC_RD_MODE 1
  #define SPI_IOC_WR_BITS_PER_WORD 1
  #define SPI_IOC_RD_BITS_PER_WORD 1
  #define SPI_IOC_WR_MAX_SPEED_HZ 1
  #define SPI_IOC_RD_MAX_SPEED_HZ 1

#endif

#define VALID_FD_ISSET(fd, set) (((fd) >= 0) && (FD_ISSET((fd), (set))))

#define STRINGIFY(x) #x
#define STRINGIFYX(x) STRINGIFY(x)

#define TRUE  1
#define FALSE 0
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define elapsedTimeInt32u(oldTime, newTime) \
  ((uint32_t) ((uint32_t)(newTime) - (uint32_t)(oldTime)))
#define timeGTorEqualInt32u(t1, t2) \
  (elapsedTimeInt32u(t2, t1) <= (uint32_t) 0x80000000)

typedef struct {
  void (*handler)(void);
  uint32_t timeToExecute;
  bool active;
} EventControl;

static uint32_t getInt32uMillisecondTick(void);
static void setTimeout(EventControl *event, uint32_t delay);
static void checkEvent(EventControl *event);

#define DEV_STRING_MAX   256
#define GPIO_STRING_MAX  256
static char wakeGpioArg[DEV_STRING_MAX + 1];  //FIXME: Currently not used

static int logging = 1;
static bool logEnabled = true;
static FILE *logFile = NULL;
static int ncpSpiFd = -1;
static int ncpIntFd = -1;
static int ncpResetFd = -1;
static int ncpChipSelectFd = -1;
static int clientFd = -1;
static int listenFd = -1;
static bool multiClient = false;
static bool doFlush = false;

#define INIT_DELAY  2000
#define ERROR_DELAY 1000
static void initHandler(void);
static void errorHandler(void);
static EventControl rxErrorEvent = { &errorHandler, 0, 0 };
static EventControl initEvent = { &initHandler, 0, 0 };

#define TIMEOUT_LONG_US 100000
#define TIMEOUT_SHORT_US 10000
#define TIMEOUT_INTERTRANSACTION_US 1000
#define TIMEOUT_RESET_US 1000

static uint32_t traceMask = 0;
typedef enum {
  LOG_CLIENT_RX,
  LOG_CLIENT_TX,
  LOG_SPI_RX,
  LOG_SPI_TX,
  LOG_SPI_LOGIC,
  LOG_MAX_TYPE  // Must be last
} LogEvent;

typedef enum {
  DIR_INPUT,
  DIR_OUTPUT
} GpioDir;

static const char * const logEventNames[] = {
  "CLI-RX",
  "CLI-TX",
  "SPI-RX",
  "SPI-TX",
  "SPI-LOG",
};

static void logTimestamp(void);
static void logEvent(LogEvent type, char *subType, uint8_t *data, uint16_t length);
static void logStatus(char *format, ...);

static void cleanupAndExit(int exitCode);
static void cleanupBeforeExit(void);

#define assertWithCleanup(condition) \
  if (!(condition)) {                \
    cleanupBeforeExit();             \
    assert(#condition == 0);         \
  } else {                           \
    (void) 0;                        \
  };

//--------------------------------------------------------------------------
/* NCP SPI Protocol
   - NCP and Host flow control supported via in-transaction NAK mechanism
   - Simple one byte command (bitmask):
 +-7-+-6-+-5-+-4-+-3-+-2-+-1-+-0-+
 # 0 | 0 | 1 |pay#rst|nak|mbz|dir#
 +---+---+---+---+---+---+---+---+
     0x80 = 0 \
     0x40 = 0  > These avoid any confusion with PP bytes of 00 or FF
     0x20 = 1 /
     0x10 = payload bit (1=next byte is <len> length of payload in bytes
                         followed by <pay> payload of <len> bytes,
                         which if truncated results in no <pay> data accepted)
     0x08 = reset bit - set in 1st transaction after a reboot
     0x04 = nak (1=NAK - data received from peer is currently ignored
     0x02 = mbz (must be zero -- for future definition)
     0x01 = direction bit (0=host-to-NCP 1=NCP-to-host)
     For simplicity, payload bit can always be set to guarantee <len> byte is
     present in which <len> of 0x00 indicates presence of <pay>.
     SPI packet format therefore is:
     <cmd> [<len> <pay...>]
   - <len> can range from 0 to 254 (0x00 to 0xFE).
     Value of 0xFF is reserved.
 */
#define NCP_SPI_PKT_OVERHEAD      2   // <cmd> <len>
#define NCP_SPI_MAX_PAYLOAD       254 // Max payload length per SPI transaction
#define NCP_SPI_RX_SLOP           5   // Rx tolerance of initial pad bytes
#define NCP_SPI_BUFSIZE           (NCP_SPI_RX_SLOP        \
                                   + NCP_SPI_PKT_OVERHEAD \
                                   + NCP_SPI_MAX_PAYLOAD)

#define NCP_SPI_PKT_CMD_IDX       0
#define NCP_SPI_PKT_LEN_IDX       1
#define NCP_SPI_PKT_PAY_IDX       2

#define NCP_SPI_PKT_CMD_PAT_MASK  0xE0
#define NCP_SPI_PKT_CMD_PAT       0x20  // Pattern for all commands
#define NCP_SPI_PKT_CMD_PAY       0x10  // Payload (length) is present
#define NCP_SPI_PKT_CMD_RST       0x08  // Reset happened
#define NCP_SPI_PKT_CMD_NAK       0x04  // NAK - can't accept data now
#define NCP_SPI_PKT_CMD_RSP       0x01  // Direction is response (NCP-to-Host)

#define NCP_SPI_LEN_ERROR         0xFF  // <len> value that indicates error

#define NCP_SPI_MODE              0
#define NCP_SPI_SPEED_HZ          (1024 * 1024) // 1 MHz
#define NCP_SPI_BITS_PER_WORD     8
#define NCP_SPI_DELAY_BEG_US      20 // Delay from NSEL assert to 1st SCLK
#define NCP_SPI_DELAY_END_US      0  // Delay after last SSLK to NSEL deassert

#define NCP_EM3XX_RX_PAD          0xFF
#define NCP_HOST_TX_PAD           0x00

static uint8_t ncpRxBuffer[NCP_SPI_BUFSIZE];
static uint8_t ncpTxBuffer[NCP_SPI_BUFSIZE] = { NCP_SPI_PKT_CMD_PAT
                                                | NCP_SPI_PKT_CMD_PAY
                                                | 0 // Always ACK NCP
                                                | NCP_SPI_PKT_CMD_RST,
                                                0x00, // <cmd>
                                                0x00, // <len>
};
static int16_t ncpRxAvail = 0;
static int16_t ncpTxAvail = 0;

//------------------------------------------------------------------------------
// Serial connection to NCP.

static void interTransactionDelay(void)
{
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = TIMEOUT_INTERTRANSACTION_US;
  select(0, NULL, NULL, NULL, &timeout);
}

static int openGpioDevice(const char *gpioDevice, const char *displayName, GpioDir dir)
{
  int fd = -1;
  if (gpioDevice && *gpioDevice) {
    char gpioDevicePath[GPIO_STRING_MAX + 1] = "/sys/class/gpio/";
    if (gpioDevice[0] == '/') { // Full path on command line overrides
      strncpy(gpioDevicePath, gpioDevice, sizeof(gpioDevicePath) - 1);
    } else {
      strncat(gpioDevicePath, gpioDevice, sizeof(gpioDevicePath) - strlen(gpioDevicePath) - 1);
      strncat(gpioDevicePath, "/value", sizeof(gpioDevicePath) - strlen(gpioDevicePath) - 1);
    }
    fd = open(gpioDevicePath, (dir == DIR_INPUT ? O_RDONLY : O_WRONLY) /* | O_NONBLOCK */); //FIXME: Need NONBLOCK?
    if (fd < 0) {
      logStatus("Cannot open %s device %s.", displayName, gpioDevice);
      perror("gpio open failed");
      cleanupAndExit(1);
    }
    logStatus("Opened %s device %s.", displayName, gpioDevice);
  }
  return fd;
}

static void openNcpReset(const char *resetDevice)
{
  ncpResetFd = openGpioDevice(resetDevice, "nRESET", DIR_OUTPUT);
}

static void openNcpChipSelect(const char *chipSelectGpio)
{
  ncpChipSelectFd = openGpioDevice(chipSelectGpio, "nCS", DIR_OUTPUT);
}

static void openNcpSpiSerial(const char *spiDevice, const char *intDevice)
{
  ncpSpiFd = open(spiDevice, O_RDWR /* | O_NONBLOCK */); //FIXME: Need NONBLOCK?
  if (ncpSpiFd < 0) {
    logStatus("Cannot open SPI device %s.", spiDevice);
    perror("spi open failed");
    cleanupAndExit(1);
  }
  logStatus("Opened SPI device %s.", spiDevice);
  ncpIntFd = openGpioDevice(intDevice, "nHOST_INT", DIR_INPUT);

  // SPI mode
  uint8_t mode = NCP_SPI_MODE;
  assertWithCleanup(ioctl(ncpSpiFd, SPI_IOC_WR_MODE, &mode) >= 0);
  assertWithCleanup(ioctl(ncpSpiFd, SPI_IOC_RD_MODE, &mode) >= 0);
  logEvent(LOG_SPI_LOGIC, "SPI-Mode", (uint8_t*)&mode, sizeof(mode));
  uint8_t bits = NCP_SPI_BITS_PER_WORD;
  assertWithCleanup(ioctl(ncpSpiFd, SPI_IOC_WR_BITS_PER_WORD, &bits) >= 0);
  assertWithCleanup(ioctl(ncpSpiFd, SPI_IOC_RD_BITS_PER_WORD, &bits) >= 0);
  logEvent(LOG_SPI_LOGIC, "SPI-Bits", (uint8_t*)&bits, sizeof(bits));
  uint32_t speed = NCP_SPI_SPEED_HZ;
  assertWithCleanup(ioctl(ncpSpiFd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) >= 0);
  assertWithCleanup(ioctl(ncpSpiFd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) >= 0);
  logEvent(LOG_SPI_LOGIC, "SPI-Speed", (uint8_t*)&speed, sizeof(speed));
}

sem_t *spiSemaphore = NULL;
bool semaphoreLocked = false;

static void lockSemaphore(void);
static void unlockSemaphore(void);

static void openNcpSemaphore(char *name, bool create)
{
  if (name == NULL || name[0] == '\0') {
    fprintf(stderr, "Semaphore name must not be blank");
    cleanupAndExit(1);
  }

  const char *action = create ? "create" : "open existing";
  logStatus("Attempting to %s semaphore \"%s\"", action, name);

  if (create) {
    if (sem_unlink(name) == 0) {
      logStatus("unlinked previous semaphore");
    } else if (errno == ENOENT) {
      logStatus("no previous semaphore to unlink");
    } else {
      logStatus("sem_unlink failed with errno %d (%s)", errno, strerror(errno));
      fprintf(stderr, "Old semaphore named \"%s\" could not be unlinked: %s\n", name, strerror(errno));
      cleanupAndExit(1);
    }

    mode_t prevMask = umask(0);

    spiSemaphore = sem_open(name, O_CREAT | O_EXCL, S_IRWXU, 1);

    umask(prevMask);
  } else {
    spiSemaphore = sem_open(name, 0);
  }

  if (spiSemaphore == SEM_FAILED) {
    logStatus("sem_open failed with errno %d (%s)", errno, strerror(errno));
    fprintf(stderr, "Could not %s semaphore \"%s\": %s\n", action, name, strerror(errno));
    cleanupAndExit(1);
  }

  logStatus("Semaphore \"%s\" open", name);
}

static void closeNcpSemaphore(void)
{
  if (spiSemaphore != NULL) {
    unlockSemaphore();

    if (sem_close(spiSemaphore) != 0) {
      perror("Semaphore could not be closed");
      logStatus("sem_close failed with errno %d (%s)", errno, strerror(errno));
    }
  }
}

static void lockSemaphore(void)
{
  if (spiSemaphore == NULL || semaphoreLocked) {
    return;
  }

  if (sem_trywait(spiSemaphore) == 0) {
    semaphoreLocked = true;
  } else {
    if (errno == EAGAIN) {
      logStatus("Waiting for locked semaphore");
      if (sem_wait(spiSemaphore) == 0) {
        logStatus("Semaphore obtained");
        semaphoreLocked = true;
      } else {
        perror("Unable to lock semaphore");
        logStatus("sem_wait failed with errno %d (%s)", errno, strerror(errno));
      }
    } else {
      perror("Unable to lock semaphore");
      logStatus("sem_trywait failed with errno %d (%s)", errno, strerror(errno));
    }
  }
}

static void unlockSemaphore(void)
{
  if (spiSemaphore == NULL || !semaphoreLocked) {
    return;
  }

  if (sem_post(spiSemaphore) == 0) {
    semaphoreLocked = false;
  } else {
    perror("Unable to unlock semaphore");
    logStatus("sem_post failed with errno %d (%s)", errno, strerror(errno));
  }
}

static bool ncpHostIntAsserted(void)
{
  assertWithCleanup(ncpIntFd >= 0);

  uint8_t gpioPinState = '1'; // Assume deasserted, is low-true signal
  (void) lseek(ncpIntFd, 0, SEEK_SET);
  (void) read(ncpIntFd, &gpioPinState, 1);
  logEvent(LOG_SPI_LOGIC, "INT=", &gpioPinState, sizeof(gpioPinState));
  return(gpioPinState == '0');
}

static void resetNcp(void)
{
  assertWithCleanup(ncpResetFd >= 0);

  // lock the semaphore, it will be unlocked the first time through
  // handleClientAndNcpInput (or if the process is signalled).
  lockSemaphore();

  // set nRESET low
  uint8_t gpioPinState = '0';
  (void) write(ncpResetFd, &gpioPinState, 1);

  // delay for a bit
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = TIMEOUT_RESET_US;
  select(0, NULL, NULL, NULL, &timeout);

  // set nRESET high and leave it that way
  gpioPinState = '1';
  (void) write(ncpResetFd, &gpioPinState, 1);

  logStatus("Reset NCP");
}

static void ncpChipSelectAssert(void)
{
  if (ncpChipSelectFd != -1) {
    uint8_t gpioPinState = '0';
    (void) write(ncpChipSelectFd, &gpioPinState, 1);
  }
}

static void ncpChipSelectDeassert(void)
{
  if (ncpChipSelectFd != -1) {
    uint8_t gpioPinState = '1';
    (void) write(ncpChipSelectFd, &gpioPinState, 1);
  }
}

/** @brief parses one SPI response from the NCP
 * @param buf    buffer containing response (must not be NULL)
 * @param trLen  Number of bytes in buffer (must be > 0)
 * @param pAcked Set true if Tx bytes sent were consumed by NCP
 * @return       Number of buf bytes parsed (consumed)
 */
static int16_t spiParseRsp(uint8_t *buf, int16_t trLen, bool *pAcked)
{
  assertWithCleanup((buf != NULL) && (trLen > 0));
  uint8_t *trBuf = buf;
  uint8_t cmd = *trBuf++;
  trLen--;
  if ( (cmd & (NCP_SPI_PKT_CMD_PAT_MASK | NCP_SPI_PKT_CMD_RSP))
       !=       (NCP_SPI_PKT_CMD_PAT      | NCP_SPI_PKT_CMD_RSP) ) {
    //FIXME: What to do with junk from peer?
    return -1; // for now just ignore this byte; negate signals error to caller
  }

  // Tell caller whether NCP agreed to accept the payload we just sent
  if (pAcked != NULL) {
    *pAcked = !(cmd & NCP_SPI_PKT_CMD_NAK);
  }

  if (cmd & NCP_SPI_PKT_CMD_RST) { // NCP has reset
    //FIXME: take action on NCP reset?
    // Hopefully higher-level protocols have own mechanism for this
    // At SPI protocol level, things are pretty stateless so nothing
    // much we need to do in this situation.
  }

  if ( (cmd & NCP_SPI_PKT_CMD_PAY) // response claimed to have len for payload
       && (trLen > 0) ) {          // response actually has len for payload
    ncpRxAvail = *trBuf++;         // amount of payload data NCP had to send
    trLen--;
    if (ncpRxAvail == NCP_SPI_LEN_ERROR) {
      //FIXME: Deal with ncpRxAvail == NCP_SPI_LEN_ERROR -- support it?
      ncpRxAvail = 0;
    }
    uint8_t rxLen = MIN(trLen, ncpRxAvail); // amount actually received

    // Forward newly received NCP data out to client
    if (rxLen > 0) {
      logEvent(LOG_CLIENT_TX, NULL, trBuf, rxLen);
      if (clientFd >= 0) {
        assertWithCleanup(write(clientFd, trBuf, rxLen) == rxLen); //FIXME: Handle EWOULDBLOCK
      }
      ncpRxAvail -= rxLen; // update amount still to be retrieved from NCP
      trBuf += rxLen;
      trLen -= rxLen;
    }
  } else { // No payload
    // No new data available
    ncpRxAvail = 0;
  }

  return (trBuf - buf);
}

/** @brief parses all SPI response data from the NCP
 * @param len  Number of bytes in ncpRxbuffer[] (could be 0)
 * @return     true if Tx bytes sent were consumed by NCP
 */
static bool spiParseRx(int16_t len)
{
  int16_t i = 0;
  bool acked = false;
  uint8_t errors = 0;
  while ( (i < len) && (errors <= 1) ) { // Only tolerate one error per Rx
    if (ncpRxBuffer[i] == NCP_EM3XX_RX_PAD) {
      i++;
    } else {
      // Looks like we have a command byte, parse the response
      int16_t consumed = spiParseRsp(&ncpRxBuffer[i], len - i, &acked);
      if (consumed < 0) { // Parse error signalled, consumed is negated
        errors += 1;
        consumed = -consumed;
        logEvent(LOG_SPI_LOGIC, "BadRx=", &ncpRxBuffer[i], consumed);
        i += consumed;
      } else {
        // don't continue trying to parse because the NCP will include garbage
        // after the packet which must be ignored
        break;
      }
    }
  }
  return acked;
}

static bool spiTransact(int16_t pktLen)
{
  #define SPI_IOC_TRANSFERS 2
  struct spi_ioc_transfer tr[SPI_IOC_TRANSFERS] = {
    { // The first transfer is just to delay a bit after nSEL assertion
      .tx_buf        = (unsigned long)NULL,
      .rx_buf        = (unsigned long)NULL,
      .len           = 0,
      .delay_usecs   = NCP_SPI_DELAY_BEG_US,
      .speed_hz      = 0,
      .bits_per_word = 0,
      .cs_change     = false,
    }, { // The 2nd transfer is the actual data transfer
      .tx_buf        = (unsigned long)ncpTxBuffer,
      .rx_buf        = (unsigned long)ncpRxBuffer,
      .len           = pktLen,
      .delay_usecs   = NCP_SPI_DELAY_END_US,
      .speed_hz      = NCP_SPI_SPEED_HZ,
      .bits_per_word = NCP_SPI_BITS_PER_WORD,
      .cs_change     = false,// Could be true as well
    },
  };

  // Perform the SPI transaction
  lockSemaphore();
  ncpChipSelectAssert();
  logEvent(LOG_SPI_TX, NULL, ncpTxBuffer, pktLen);
  assertWithCleanup(ioctl(ncpSpiFd, SPI_IOC_MESSAGE(SPI_IOC_TRANSFERS), tr) >= 0);
  logEvent(LOG_SPI_RX, NULL, ncpRxBuffer, pktLen);
  ncpChipSelectDeassert();
  unlockSemaphore();

  // Turn off reset indication after 1st transaction
  if (ncpTxBuffer[NCP_SPI_PKT_CMD_IDX] & NCP_SPI_PKT_CMD_RST) {
    ncpTxBuffer[NCP_SPI_PKT_CMD_IDX] &= ~NCP_SPI_PKT_CMD_RST;
  }

  return spiParseRx(pktLen);
}

static void initHandler(void)
{
  //FIXME!!
}

static void errorHandler(void)
{
  //FIXME!!
  rxErrorEvent.active = false;
  setTimeout(&initEvent, ERROR_DELAY);
}

//------------------------------------------------------------------------------
// Client socket interface.

static void openListenSocket(uint16_t port, bool allInterfaces)
{
  listenFd = socket(AF_INET6, SOCK_STREAM, 0);
  if (listenFd < 0) {
    perror("socket creation failed");
    cleanupAndExit(1);
  }
  // Set SO_REUSEADDR prior to bind() to avoid re-bind() hassles
  int set = 1;
  (void) setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set));
  struct sockaddr_in6 address;
  memset(&address, 0, sizeof(struct sockaddr_in6));
  address.sin6_family = AF_INET6;
  address.sin6_addr = (allInterfaces ? in6addr_any : in6addr_loopback);
  address.sin6_port = htons(port);
  if (bind(listenFd, (struct sockaddr *) &address, sizeof(address)) != 0) {
    perror("bind failed");
    cleanupAndExit(1);
  }
  // Put listen socket into non-block mode for accept()
  int flags = fcntl(listenFd, F_GETFL);
  if (flags >= 0) {
    fcntl(listenFd, F_SETFL, flags | O_NONBLOCK);
  }

  if (listen(listenFd, 2) == -1) {
    perror("listen failed");
    cleanupAndExit(1);
  }
  logStatus("Listening on port %u.", port);
}

static void handleListenInput(void)
{
  // Terminate an existing client if new one comes along
  //FIXME: Do we really want to allow this?
  if (clientFd >= 0) {
    // shutdown(clientFd, SHUT_RDWR); //FIXME: Need to shutdown() before close()?
    close(clientFd);
    clientFd = -1;
  }
  struct sockaddr addr;
  socklen_t addrlen = sizeof(addr);
  clientFd = accept(listenFd, &addr, &addrlen);
  if (clientFd < 0) {
    return; // False alarm -- client has already disappeared
  }
  logEvent(LOG_CLIENT_RX, "ACCEPT", NULL, 0);
  logStatus("Client accepted.");
  // Put client socket into non-block mode for read()/write()
  int flags = fcntl(clientFd, F_GETFL);
  if (flags >= 0) {
    fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
  }
}

static bool handleClientAndNcpInput(bool clientPending, bool ncpPending)
{
  if (ncpPending) {
    ncpPending = ncpHostIntAsserted(); // Necessary to 'ack' the interrupt
  }
  while (1) {
    // Check for more data to send to NCP
    if ( (ncpTxAvail < NCP_SPI_MAX_PAYLOAD) && (clientFd >= 0) ) {
      int got = read(clientFd, &ncpTxBuffer[NCP_SPI_PKT_PAY_IDX + ncpTxAvail],
                     NCP_SPI_MAX_PAYLOAD - ncpTxAvail);
      if (got > 0) {
        logEvent(LOG_CLIENT_RX,
                 NULL,
                 &ncpTxBuffer[NCP_SPI_PKT_PAY_IDX + ncpTxAvail],
                 got);
        ncpTxAvail += got;
        assertWithCleanup(ncpTxAvail >= 0 && ncpTxAvail <= NCP_SPI_MAX_PAYLOAD);
      } else
      if ( (got == 0) || (errno != EWOULDBLOCK) ) { // EOF/error
        // shutdown(clientFd, SHUT_RDWR); //FIXME: Need to shutdown() before close()?
        close(clientFd);
        clientFd = -1;
        logEvent(LOG_CLIENT_RX, "EOF", NULL, 0);
        logStatus("Client disconnected.");
        if (!multiClient) {
          logStatus("Terminating.");
          cleanupAndExit(0);
        } else {
          // Don't return here - there still might be data to transact
          // and more clients to handle.
        }
      } else {
        // EWOULDBLOCK -- no data to forward yet
      }
    }
    if ( ncpPending && (ncpRxAvail == 0) ) {
      // NCP is saying it has data for us, but we don't know how much.
      // Rather than doing a tiny probe just to find out, assume it is
      // a bunch and just go grab as much as possible.
      ncpRxAvail = NCP_SPI_MAX_PAYLOAD;
    }
    ncpPending = false;
    if ( (ncpTxAvail == 0) && (ncpRxAvail == 0) ) {
      unlockSemaphore(); // make sure we don't starve the other host when sharing the bus
      return false; // Nothing more to do -- caller can use long leash
    }
    // ncpTxAvail > 0 indicates data to send to  NCP
    // ncpRxAvail > 0 indicates data to get from NCP
    // Our SPI transaction payload shall be the greater of the two
    int16_t payLen = MAX(ncpTxAvail, ncpRxAvail);
    // ncpTxBuffer[NCP_SPI_PKT_CMD_IDX] is already set up; we rarely change it
    ncpTxBuffer[NCP_SPI_PKT_LEN_IDX] = ncpTxAvail;
    // ncpTxBuffer[] already contains data to be sent, but pad it out
    // to the actual transaction length with NCP_HOST_TX_PAD
    memset(&ncpTxBuffer[NCP_SPI_PKT_PAY_IDX + ncpTxAvail], NCP_HOST_TX_PAD,
           payLen - ncpTxAvail + NCP_SPI_RX_SLOP);
    // ncpRxBuffer[] gets cleared by padding using NCP_EM3XX_RX_PAD
    memset(ncpRxBuffer, NCP_EM3XX_RX_PAD, payLen + NCP_SPI_RX_SLOP);
    if (spiTransact(NCP_SPI_PKT_OVERHEAD + payLen + NCP_SPI_RX_SLOP)) {
      // NCP accepted the data sent!
      ncpTxAvail = 0;
    }
    if ( (ncpTxAvail > 0) && (ncpRxAvail == 0) ) {
      // NCP NAK'd data sent to it, and has no data to return.
      // This is likely a flow-control situation.
      // Return -- caller should use a short leash
      return true;
    }
    // Continue loop to try to (re)send or retrieve pending data
    interTransactionDelay(); // Ensure enough time between NCP transactions
  }
  assertWithCleanup(false); // we shouldn't get here...
}

//------------------------------------------------------------------------------
// Timing.

static uint32_t unixTimeToMilliseconds(struct timeval *tv)
{
  uint32_t now = (tv->tv_sec * 1000) + (tv->tv_usec / 1000);
  return now;
}

static uint32_t getInt32uMillisecondTick(void)
{
  struct timeval tv;
  uint32_t now;
  gettimeofday(&tv, NULL);
  now = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
  return now;
}

static void setTimeout(EventControl *event, uint32_t delay)
{
  event->timeToExecute = getInt32uMillisecondTick() + delay;
  event->active = true;
}

static void checkEvent(EventControl *event)
{
  uint32_t now = getInt32uMillisecondTick();
  if (event->active
      && timeGTorEqualInt32u(now, event->timeToExecute)) {
    event->active = false;
    event->handler();
  }
}

//------------------------------------------------------------------------------
// Command-line arguments.

static bool parseArguments(int argc, char *argv[])
{
  if (argc < 3) {
    return false;
  }
  char portArg[DEV_STRING_MAX + 1];
  char spiDevArg[DEV_STRING_MAX + 1];
  char intGpioArg[GPIO_STRING_MAX + 1];
  char resetGpioArg[GPIO_STRING_MAX + 1];
  char chipSelGpioArg[GPIO_STRING_MAX + 1];
  bool allInterfaces;
  int port = -1;
  bool useSemaphore = false;
  char semaphoreName[DEV_STRING_MAX + 1];
  bool createSemaphore = false;

  memset(portArg, 0, sizeof(portArg));
  memset(spiDevArg, 0, sizeof(spiDevArg));
  memset(intGpioArg, 0, sizeof(intGpioArg));
  memset(resetGpioArg, 0, sizeof(resetGpioArg));
  memset(chipSelGpioArg, 0, sizeof(chipSelGpioArg));
  memset(semaphoreName, 0, sizeof(semaphoreName));

  while (true) {
    static struct option long_options[] = {
      { "nolog", no_argument, &logging, 0  },
      { "port", required_argument, 0, 'p' },
      { "spiDev", required_argument, 0, 's' },
      { "intGpio", required_argument, 0, 'i' },
      { "resetGpio", required_argument, 0, 'r' },
      { "wakeGpio", required_argument, 0, 'w' },
      { "traceMask", required_argument, 0, 'T' },
      { "allInterfaces", no_argument, 0, 'a' },
      { "multiClient", no_argument, 0, 'm' },
      { "doFlush", no_argument, 0, 'f' },
      { "semaphore", required_argument, 0, 'x' },
      { "semaphore-create", required_argument, 0, 'X' },
      { "chipSelGpio", required_argument, 0, 'c' },
      { 0, 0, 0, 0 }
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "p:s:i:r:w:T:amfx:X:c:", long_options, &option_index);
    if (c == -1) {
      if (option_index != argc && option_index != 0) {
        fprintf(stderr, "Unexpected argument %s\n", argv[option_index]);
        return false;
      }
      break;
    }

    switch (c) {
      case 0:
        break;
      case 'a': // listen on all interfaces (undocumented)
        allInterfaces = true;
        break;
      case 'm': // allow multiple clients (don't exit after first) (undocumented)
        multiClient = true;
        break;
      case 'f': // flush logFile on each entry (can cause pauses on BeagleBoard)
        doFlush = true;
        break;
      case 'p': // port
        sscanf(optarg, "%" STRINGIFYX(DEV_STRING_MAX) "s", portArg);
        port = atoi(portArg);
        if (port < 0 || port > 65535) {
          fprintf(stderr, "Invalid simulator port number %d\n", port);
          return false;
        }
        break;
      case 's': // spidev
        sscanf(optarg, "%" STRINGIFYX(DEV_STRING_MAX) "s", spiDevArg);
        break;
      case 'i': // nHOST_INT GPIO
        sscanf(optarg, "%" STRINGIFYX(DEV_STRING_MAX) "s", intGpioArg);
        break;
      case 'r': // nRESET GPIO
        sscanf(optarg, "%" STRINGIFYX(DEV_STRING_MAX) "s", resetGpioArg);
        break;
      case 'w': // nWAKE GPIO
        sscanf(optarg, "%" STRINGIFYX(DEV_STRING_MAX) "s", wakeGpioArg);
        break;
      case 'T': // trace mask
        traceMask = (uint8_t) strtoul(optarg, NULL, 16);
        break;
      case 'x': // open semaphore
      case 'X': // create semaphore
        useSemaphore = true;
        sscanf(optarg, "%" STRINGIFYX(DEV_STRING_MAX) "s", semaphoreName);
        createSemaphore = (c == (int)'X');
        break;
      case 'c': // chip select GPIO
        sscanf(optarg, "%" STRINGIFYX(DEV_STRING_MAX) "s", chipSelGpioArg);
        break;
      default:
        return false;
    }
  }

  logEnabled = (bool) logging;
  if (logEnabled) {
    logFile = fopen(PROGNAME ".log", "w");
    if (logFile == NULL) {
      perror("failed to open log file");
      cleanupAndExit(1);
    }
  }

  logStatus("traceMask = 0x%02X", traceMask);
  openNcpSpiSerial(spiDevArg, intGpioArg);
  openNcpReset(resetGpioArg);
  openNcpChipSelect(chipSelGpioArg);
  openListenSocket(port, allInterfaces);

  if (useSemaphore) {
    openNcpSemaphore(semaphoreName, createSemaphore);
  }

  return true;
}

//------------------------------------------------------------------------------
// Cleanup

static int signalCaught = 0;

static void signalHandler(int signal)
{
  // save signal for DSR processing to avoid race condition of processing now
  signalCaught = signal;
}

static void cleanupAndExit(int exitCode)
{
  if (signalCaught) {
    // we test this here because some syscalls, notably select(), return an
    // error when interrupted by a signal and we want to still be able to
    // inform the user that the signal was the underlying cause
    logStatus("Caught signal %d, terminating", signalCaught);
  }

  cleanupBeforeExit();
  exit(exitCode);
}

static void cleanupBeforeExit(void)
{
  if (ncpResetFd != -1) {
    // set nRESET high and leave it that way
    uint8_t gpioPinState = '1';
    (void) write(ncpResetFd, &gpioPinState, 1);
  }
  ncpChipSelectDeassert();
  closeNcpSemaphore();
  fflush(logFile);
}

//------------------------------------------------------------------------------
// Main.

int main(int argc, char **argv)
{
  bool ncpFlowControlled = false;

  if (!parseArguments(argc, argv)) {
    fprintf(stderr, "Usage: %s -p port -s spiDev -i intGpio -r resetGpio"
                    " [-w wakeGpio] [-T traceMask] [-f] [-c chipSelGpio]"
                    " [--semaphore[-create] semaphoreName]\n", PROGNAME);
    cleanupAndExit(1);
  }

  resetNcp();

  // clear preexisting nHOST_INT interrupt, if any
  ncpHostIntAsserted();

  // wait to give the NCP a chance to start
  setTimeout(&initEvent, INIT_DELAY);

  logStatus("SPI Server running...");

  // configure signal handlers
  struct sigaction sigHandler;
  sigHandler.sa_handler = signalHandler;
  sigemptyset(&sigHandler.sa_mask);
  sigHandler.sa_flags = 0;
  sigaction(SIGINT, &sigHandler, NULL);
  sigaction(SIGTERM, &sigHandler, NULL);
  sigaction(SIGABRT, &sigHandler, NULL);

  while (1) {
    if (signalCaught) {
      cleanupAndExit(1);
    }

    checkEvent(&rxErrorEvent);
    checkEvent(&initEvent);
    if (initEvent.active) {
      //FIXME: Anything to do?
    } else {
      fd_set input, except;
      int maxFd = -1;
      FD_ZERO(&input);
      FD_ZERO(&except);
      if (listenFd >= 0) {
        FD_SET(listenFd, &input);
        maxFd = listenFd;
      }
      if ( (clientFd >= 0) && (!ncpFlowControlled) ) {
        FD_SET(clientFd, &input);
        maxFd = MAX(maxFd, clientFd);
      }
      if (ncpIntFd >= 0) {
        FD_SET(ncpIntFd, &except);
        maxFd = MAX(maxFd, ncpIntFd);
      }
      struct timeval timeout;
      timeout.tv_sec = 0;
      timeout.tv_usec = ncpFlowControlled ? TIMEOUT_SHORT_US : TIMEOUT_LONG_US;
      int n = select(maxFd + 1, &input, NULL, &except, &timeout);
#if     VERBOSE_DEBUG
      logEvent(LOG_SPI_LOGIC, "Wake", (uint8_t*)&n, sizeof(n));
      logEvent(LOG_SPI_LOGIC, "Time", (uint8_t*)&timeout.tv_usec, sizeof(timeout.tv_usec));
#endif//VERBOSE_DEBUG
      if ((n < 0) && (errno != EINTR)) {
        perror("select failed");
        cleanupAndExit(1);
      } else if (n == 0) {
        // Timeout.
        ncpFlowControlled = handleClientAndNcpInput(false, false);
      } else {
        if (VALID_FD_ISSET(listenFd, &input)) {
          handleListenInput();
        }
        ncpFlowControlled = handleClientAndNcpInput(
          VALID_FD_ISSET(clientFd, &input),
          VALID_FD_ISSET(ncpIntFd, &except));
      }
    }
  }
}

//------------------------------------------------------------------------------
// Log.

static uint32_t lastLogEventTime;

static void logEvent(LogEvent type, char *subType, uint8_t *data, uint16_t length)
{
  if (logEnabled) {
    if ((1 << type) & traceMask) {
      logTimestamp();
      fprintf(logFile, "[%s] [", logEventNames[type]);
      if (subType != NULL) {
        fprintf(logFile, "%s", subType);
      }
      uint16_t i;
      for (i = 0; i < length; i++) {
        fprintf(logFile, " %02X", data[i]);
      }
      fprintf(logFile, "]\n");
      if (doFlush) {
        fflush(logFile);
      }
    }
  }
}

static void logTimestamp(void)
{
  struct timeval tv;
  struct tm *tm;
  char datetime[64];
  gettimeofday(&tv, NULL);
  tm = localtime(&tv.tv_sec);
  assertWithCleanup(tm != NULL);
  strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S", tm);
  uint32_t now = unixTimeToMilliseconds(&tv);
  uint32_t delta = elapsedTimeInt32u(lastLogEventTime, now);
  lastLogEventTime = now;
  if (delta == 0) {
    fprintf(logFile, "                                    ");
  } else {
    if (delta > 1000) {
      fprintf(logFile, "\n");
    }
    fprintf(logFile, "[%s.%03d +%4d.%03d] ",
            datetime, (int)(tv.tv_usec / 1000), delta / 1000, delta % 1000);
  }
  if (doFlush) {
    fflush(logFile);
  }
}

static void logStatus(char *format, ...)
{
  if (logEnabled) {
    logTimestamp();
    fprintf(logFile, "[status] [");
    va_list ap;
    va_start(ap, format);
    vfprintf(logFile, format, ap);
    va_end(ap);
    fprintf(logFile, "]\n");
    if (doFlush) {
      fflush(logFile);
    }

    printf("[%s] ", PROGNAME);
    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);
    fprintf(stdout, "\n");
    fflush(stdout);
  }
}
