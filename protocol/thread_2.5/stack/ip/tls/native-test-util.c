/*
 * File: native-test-util.c
 * Description: interface to native sockets
 * Author(s): Richard Kelsey
 *
 * Copyright 2012 by Ember Corporation. All rights reserved.                *80*
 */

#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <poll.h>

#include "core/ember-stack.h"
#include "debug.h"
#include "native-test-util.h"

int maxNativeReadTimeMs = -1;

bool emReadingTrace = false;
bool emWritingTrace = false;
char emMyTag = 's';
char emTheirTag = 'c';
static bool testingDone = false;

bool usingTraceFile(void)
{
  return (emWritingTrace || emReadingTrace);
}

// When writing a trace
static FILE *traceFile = NULL;

typedef struct TraceRecord_S {
  char tag;
  struct TraceRecord_S *previous;
  struct TraceRecord_S *next;
  unsigned short port;
  uint8_t address[16];
  uint32_t length;
  uint8_t contents[0];
} TraceRecord;

// When reading a trace
static TraceRecord *theTrace = NULL;
static uint32_t bytesUsed = 0;

bool emTestingDone(void)
{
  return (testingDone
          || (emReadingTrace
              && theTrace == NULL));
}

typedef struct FdTrace_S {
  int fd;
  char readTag;
  char writeTag;
  unsigned short port;
  EmberIpv6Address address;
  struct FdTrace_S *next;
} FdTrace;

static FdTrace *theFdTraces = NULL;

static FdTrace *findFdTrace(int fd)
{
  FdTrace *fdTrace = theFdTraces;
  while (fdTrace != NULL) {
    if (fdTrace->fd == fd) {
      return fdTrace;
    }

    fdTrace = fdTrace->next;
  }

  return NULL;
}

static FdTrace *addFdTrace(int fd,
                           char myTag,
                           char theirTag,
                           unsigned short port,
                           const uint8_t *address)
{
  FdTrace *fdTrace = (FdTrace *) malloc(sizeof(FdTrace));

  if (fdTrace == NULL) {
    fprintf(stderr, "Failed to allocate FdTrace.");
    exit(-1);
  }

  memset(fdTrace, 0, sizeof(FdTrace));
  fdTrace->fd = fd,
  fdTrace->writeTag = myTag;
  fdTrace->readTag = theirTag;
  fdTrace->next = theFdTraces;
  fdTrace->port = port;

  if (address != NULL) {
    memcpy(fdTrace->address.bytes, address, sizeof(EmberIpv6Address));
  }

  theFdTraces = fdTrace;

  return fdTrace;
}

// Hopefully this is large enough not to conflict with any read fd.
// We have to stay under 255 because the Ember TCP code stores these
// as bytes.
static int nextFd = 200;

// returns the last port from the trace file
uint16_t openInputTraceFile(const char *name)
{
  FILE *in = fopen(name, "r");
  TraceRecord *previous = NULL;
  uint16_t i;

  if (in == NULL) {
    fprintf(stderr, "Open \"%s\" failed.\n", name);
    exit(-1);
  }

  emReadingTrace = true;
  int result = 0;

  while (true) {
    int tag = fgetc(in);
    unsigned int length;
    TraceRecord *traceRecord = { 0 };
    int port;
    uint8_t addressFromFile[100] = { 0 };

    if (tag == EOF) {
      break;
    } else if (isspace(tag)) {
      continue;
    }

    if (fscanf(in, " %d", &port) != 1) {
      fprintf(stderr, "Failed to read port");
      exit(-1);
    }

    ASSERT(port <= 0xFFFF);

    if (fscanf(in, " %s", addressFromFile) != 1) {
      fprintf(stderr, "Failed to read address");
      exit(-1);
    }

    if (fscanf(in, " %u", &length) != 1) {
      fprintf(stderr, "Failed to read trace length.");
      exit(-1);
    }

    traceRecord = (TraceRecord *) malloc(sizeof(TraceRecord) + length);

    if (traceRecord == NULL) {
      fprintf(stderr, "Failed to allocate TraceRecord.");
      exit(-1);
    }

    traceRecord->tag = tag;
    traceRecord->previous = previous;
    traceRecord->next = NULL;
    traceRecord->length = length;
    traceRecord->port = port;

    result = traceRecord->port;

    // i don't like having an #ifdef here, but it's easier than
    // pulling in ip-address.c to the tls and pana tests
#ifdef UNIX_SCRIPTED_HOST
    stringToIpAddress(addressFromFile,
                      strlen(addressFromFile),
                      traceRecord->address);
#endif

    if (previous == NULL) {
      theTrace = traceRecord;
    } else {
      previous->next = traceRecord;
    }

    previous = traceRecord;

    for (i = 0; i < length; i++) {
      unsigned int n;
      if (fscanf(in, " %2X", &n) != 1) {
        fprintf(stderr, "Failed to read trace value.");
        exit(-1);
      }
      traceRecord->contents[i] = n;
    }
  }

  fprintf(stderr, "[Running \"%s\" ", name);
  return result;
}

int openTraceFd(char myTag,
                char theirTag,
                unsigned short port,
                const uint8_t *address)
{
  FdTrace *fdTrace = addFdTrace(nextFd++, myTag, theirTag, port, address);
  return fdTrace->fd;
}

static void nextTraceRecord(void)
{
  theTrace = theTrace->next;
  bytesUsed = 0;
  if (theTrace != NULL) {
    fprintf(stderr, "%c", theTrace->tag);
  }
}

void replayTraceRecords(int count)
{
  fprintf(stderr, "<<%d", count);
  for (; 0 < count; count--) {
    theTrace = theTrace->previous;
  }
  bytesUsed = 0;
  fprintf(stderr, "%c", theTrace->tag);
}

void finishRunningTrace(void)
{
  ASSERT(theTrace == NULL);
  if (emReadingTrace) {
    fprintf(stderr, "]\n");
  }
}

void openOutputTraceFile(const char *filename)
{
  traceFile = fopen(filename, "w");
  if (traceFile == NULL) {
    fprintf(stderr, "Open \"%s\" failed.\n", filename);
    exit(-1);
  }
  emWritingTrace = true;
}

void emNoteTraceFd(int fd, unsigned short port, const uint8_t *address)
{
  noteTraceFd(fd, emMyTag, emTheirTag, port, address);
}

void noteTraceFd(int fd,
                 char myTag,
                 char theirTag,
                 unsigned short port,
                 const uint8_t *address)
{
  addFdTrace(fd, myTag, theirTag, port, address);
}

static void writeTraceRecord(int fd,
                             bool isRead,
                             const uint8_t *contents,
                             uint16_t length,
                             struct sockaddr *remoteAddress)
{
  FdTrace *fdTrace = findFdTrace(fd);
  if (fdTrace != NULL
      && traceFile != NULL) {
    uint16_t i;
    char addressString[100] = { 0 };
    char realAddress[16] = { 0 };

    if (remoteAddress != NULL) {
      memcpy(realAddress,
             ((struct sockaddr_in6 *) remoteAddress)->sin6_addr.s6_addr,
             16);
    }

    // i don't like having an #ifdef here, but it's easier than
    // pulling in ip-address.c to the tls and pana tests
#ifdef UNIX_SCRIPTED_HOST
    ipAddressToString(realAddress,
                      addressString,
                      sizeof(addressString));
#else
    strcpy(addressString, "0000:0000:0000:0000:0000:0000:0000:0000");
#endif

    fprintf(traceFile, "%c %u %s %d",
            (isRead
             ? fdTrace->readTag
             : fdTrace->writeTag),
            fdTrace->port,
            addressString,
            length);
    for (i = 0; i < length; i++) {
      fprintf(traceFile, "%s%02X",
              (i % 16 == 0
               ? "\n"
               : " "),
              contents[i]);
    }
    fprintf(traceFile, "\n");
    fflush(traceFile);
  }
}

//----------------------------------------------------------------

bool nativeLookupHost(const char *host,
                      int port,
                      struct sockaddr_in *hostAddress)
{
  struct hostent *serverHost = gethostbyname(host);
  if (serverHost == NULL) {
    return false;
  }
  memcpy(&hostAddress->sin_addr,
         serverHost->h_addr_list[0],
         serverHost->h_length);
  hostAddress->sin_family = AF_INET;
  hostAddress->sin_port = htons(port);
  // fprintf(stderr, "[%s:%d -> %08X]\n",
  //         host,
  //         port,
  //         hostAddress->sin_addr.s_addr);
  return true;
}

EmberStatus nativeConnect(const char *host, int port, int *fdResult)
{
  struct sockaddr_in serverAddress;
  int fd;

  signal(SIGPIPE, SIG_IGN);

  if (!nativeLookupHost(host, port, &serverAddress)) {
    return EMBER_ERR_FATAL;
  }

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (fd < 0) {
    return EMBER_ERR_FATAL;
  }

  if (connect(fd,
              (struct sockaddr *) &serverAddress,
              sizeof(serverAddress))
      < 0) {
    close(fd);
    return EMBER_ERR_FATAL;
  }

  *fdResult = fd;
  return EMBER_SUCCESS;
}

EmberStatus nativeBind(uint16_t port, int *fdResult)
{
  int fd;
  int optionValue = true;
  struct sockaddr_in serverAddress;

  signal(SIGPIPE, SIG_IGN);

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (fd < 0) {
    return EMBER_ERR_FATAL;
  }

  setsockopt(fd,
             SOL_SOCKET,
             SO_REUSEADDR,
             &optionValue,
             sizeof(optionValue));

  serverAddress.sin_addr.s_addr = INADDR_ANY;
  serverAddress.sin_family      = AF_INET;
  serverAddress.sin_port        = htons(port);

  if (!((bind(fd,
              (struct sockaddr *) &serverAddress,
              sizeof(serverAddress))
         == 0)
        && listen(fd, 3) == 0)) {
    close(fd);
    return EMBER_ERR_FATAL;
  }

  *fdResult = fd;
  return EMBER_SUCCESS;
}

EmberStatus nativeAccept(int bindFd, int *fdResult)
{
  struct sockaddr_in socketAddress;
  socklen_t socketAddressLength = (socklen_t) sizeof(socketAddress);

  *fdResult = accept(bindFd,
                     (struct sockaddr *) &socketAddress,
                     &socketAddressLength);

  if (*fdResult < 0) {
    if (errno == EAGAIN
        || errno == EWOULDBLOCK) {
      return(EMBER_NETWORK_BUSY);
    } else {
      return(EMBER_ERR_FATAL);
    }
  }
  return EMBER_SUCCESS;
}

EmberStatus nativeOpenUdp(int *localPortLoc, int *fdResult)
{
  struct sockaddr_in localAddress;
  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (fd < 0) {
    return EMBER_ERR_FATAL;
  }

  memset((char *) &localAddress, 0, sizeof(localAddress));
  localAddress.sin_family = AF_INET;
  localAddress.sin_port = htons(*localPortLoc);
  localAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(fd, (struct sockaddr *) &localAddress, sizeof(localAddress))
      < 0) {
    fprintf(stderr, "UDP bind to port %d failed: %s\n",
            *localPortLoc, strerror(errno));
    return EMBER_ERR_FATAL;
  }

  socklen_t length = sizeof(localAddress);
  if (getsockname(fd, (struct sockaddr *) &localAddress, &length)
      < 0) {
    fprintf(stderr, "getsockname failed: %s\n", strerror(errno));
    return EMBER_ERR_FATAL;
  }

  *localPortLoc = ntohs(localAddress.sin_port);
  debug("bound fd %d to port %d", fd, *localPortLoc);

  *fdResult = fd;
  //fprintf(stderr, "[opened UDP %d]\n", fd);
  return EMBER_SUCCESS;
}

bool nativeDataReady(int fd, unsigned short port)
{
  bool result = false;

  if (theTrace != NULL) {
    FdTrace *fdTrace = findFdTrace(fd);
    ASSERT(fdTrace != NULL
           && fdTrace->port == port);

    if (theTrace->port == port
        && theTrace->tag == fdTrace->readTag) {
      // data's ready!
      result = true;
    }
  }

  return result;
}

int nativeReadWithSender(int fd,
                         uint8_t *bytes,
                         uint16_t want,
                         struct sockaddr *sender,
                         socklen_t *senderLength)
{
  FdTrace *fdTrace = findFdTrace(fd);
  int result = 0;

  if (fdTrace != NULL
      && theTrace != NULL) {
    if (theTrace->port != fdTrace->port) {
      errno = EAGAIN;
      return -1;
    }

    uint8_t *contents = theTrace->contents + bytesUsed;
    uint16_t have = theTrace->length - bytesUsed;

    ASSERT(theTrace->tag == fdTrace->readTag);

    if (sender != NULL) {
      struct sockaddr_in6 *sender6 = (struct sockaddr_in6 *)sender;
      sender6->sin6_port = htons(theTrace->port);
      sender6->sin6_family = AF_INET6;
      memcpy(sender6->sin6_addr.s6_addr, theTrace->address, 16);
    }

    if (want < have) {
      have = want;
      bytesUsed += want;
    } else {
      nextTraceRecord();
    }

    memcpy(bytes, contents, have);
    result = have;
  } else {
    int status = 0;

    if (sender != NULL) {
      status = recvfrom(fd,
                        bytes,
                        want,
                        0,
                        (struct sockaddr *)sender,
                        senderLength);
    } else {
      if (maxNativeReadTimeMs == -1) {
        status = read(fd, bytes, want);
      } else {
        struct pollfd pollFd;
        pollFd.fd = fd;
        pollFd.events = POLLIN;
        switch (poll(&pollFd, 1, maxNativeReadTimeMs)) {
          case 0:
            writeTraceRecord(fd, true, 0, 0, sender);
            return 0;   // timed out, nothing read;
          case 1:
            status = read(fd, bytes, want);
            break;
          default:
            fprintf(stderr, "poll on %d failed: %s\n", fd, strerror(errno));
            return 0;
        }
      }
    }

    if (status < 0
        && errno != EAGAIN) {
      fprintf(stderr, "read on %d failed: %s\n", fd, strerror(errno));
    } else if (status == 0
               && 0 < want) {
#ifdef UNIX_HOST
      fprintf(stderr, "received empty payload on %d", fd);
#else
      fprintf(stderr, "read EOF on %d", fd);
      // don't ASSERT(false) for non-UNIX_HOST - aka SE2.0 tests
      // the EOF signifies that the SE2.0 scripted test is done
      // via client closing the connection
      ASSERT(false);
#endif
      testingDone = true;
    } else if (status > 0) {
      // danger for ipv4
      writeTraceRecord(fd, true, bytes, status, sender);
    }

    result = status;
  }

  return result;
}

void nativeWrite(int fd,
                 const uint8_t *bytes,
                 uint16_t count,
                 struct sockaddr *remoteUdpAddress,
                 uint16_t udpAddressSize)
{
  FdTrace *fdTrace = findFdTrace(fd);

  if (fdTrace != NULL
      && theTrace != NULL) {
    ASSERT(theTrace->tag == fdTrace->writeTag);

    if (count != theTrace->length
        || memcmp(bytes, theTrace->contents + bytesUsed, count) != 0) {
      dump("sending  ", bytes, count);
      dump("trace has", theTrace->contents, theTrace->length);
      ASSERT(false);
    }
    nextTraceRecord();
  } else {
    // danger for ipv4
    if (remoteUdpAddress != NULL) {
      writeTraceRecord(fd, false, bytes, count, remoteUdpAddress);
      int status = sendto(fd,
                          bytes,
                          count,
                          0,
                          remoteUdpAddress,
                          udpAddressSize);
      if (status < 0) {
        fprintf(stderr, "UDP write on %d failed: %s\n", fd, strerror(errno));
        ASSERT(false);
      } else if (status != count) {
        fprintf(stderr, "UDP write on %d fail to send all: %s\n",
                fd,
                strerror(errno));
        ASSERT(false);
      } else {
        dump("UDP write", bytes, status);
        ASSERT(status == count);
      }
    } else {
      writeTraceRecord(fd, false, bytes, count, NULL);
      while (0 < count) {
        int status = write(fd, bytes, count);
        if (status < 0) {
          fprintf(stderr, "write on %d failed: %s", fd, strerror(errno));
          ASSERT(false);
        } else {
          count -= status;
          bytes += status;
        }
      }
    }
  }
}

// Current time.

void emWriteCurrentTime(uint8_t *loc)
{
  time_t now;

  #ifdef UNIX_HOST_SIM
  now = 0xAABBCCDD;
  #else
  if (usingTraceFile()) {
    now = 0x4BDB4AB5;
  } else {
    now = time(NULL);
  }
  #endif

  *loc++ = now >> 24;
  *loc++ = now >> 16;
  *loc++ = now >>  8;
  *loc++ = now;
}
