/*
 * File: native-test-util.h
 * Description: interface to native sockets
 * Author(s): Richard Kelsey
 *
 * Copyright 2012 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef __NATIVE_TEST_UTIL_H__
#define __NATIVE_TEST_UTIL_H__

#ifdef EMBER_TEST
  #define ASSERT(arg) assert(arg)
#else
  #define ASSERT(arg)
#endif

void writeCurrentTime(uint8_t *loc);

struct sockaddr;

// Socket stuff
EmberStatus nativeConnect(const char *host, int port, int *fdResult);
EmberStatus nativeBind(uint16_t port, int *fdResult);
EmberStatus nativeAccept(int bindFd, int *fdResult);
EmberStatus nativeOpenUdp(int *localPortLoc, int *fdResult);
bool nativeLookupHost(const char *host,
                      int port,
                      struct sockaddr_in *hostAddress);

// is data ready on a port
bool nativeDataReady(int fd, unsigned short port);

extern int maxNativeReadTimeMs;

#define nativeRead(fd, bytes, want) \
  (nativeReadWithSender((fd), (bytes), (want), NULL, NULL))

int nativeReadWithSender(int fd,
                         uint8_t *bytes,
                         uint16_t want,
                         struct sockaddr *sender,
                         socklen_t *senderLength);

void nativeWrite(int fd,
                 const uint8_t *bytes,
                 uint16_t count,
                 struct sockaddr *optionalRemoteUdpAddress,
                 uint16_t sizeofAddress);

void setFdAsUdp(int fd);

// Random numbers
void randomize(uint8_t *blob, uint16_t length);

// Trace files
bool usingTraceFile(void);

// Creating trace files
void openOutputTraceFile(const char *name);
void noteTraceFd(int fd, char myTag,
                 char theirTag,
                 unsigned short port,
                 const uint8_t *address);
void emNoteTraceFd(int fd, unsigned short port, const uint8_t *address);

// Using trace files
uint16_t openInputTraceFile(const char *name);
int openTraceFd(char myTag,
                char theirTag,
                unsigned short port,
                const uint8_t *address);
void replayTraceRecords(int count);
void finishRunningTrace(void);

bool emTestingDone(void);

extern bool emReadingTrace;
extern bool emWritingTrace;
extern char emMyTag;
extern char emTheirTag;

#endif
