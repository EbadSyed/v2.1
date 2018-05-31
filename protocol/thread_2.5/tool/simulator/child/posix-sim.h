// File: posix-sim.h
//
// Description: Running POSIX code on a simulated host.
//
// Copyright 2013 by Ember Corporation. All rights reserved.                *80*

#ifndef POSIX_SIM_H
#define POSIX_SIM_H

#ifdef EMBER_TEST
  #define EMBER_READ   emPosixRead
  #define EMBER_WRITE  emPosixWrite
  #define EMBER_SELECT emPosixSelect
#else
// the real thing
  #define EMBER_READ   read
  #define EMBER_WRITE  write
  #define EMBER_SELECT select
#endif

// In simulation, file descriptors are actually ports.

ssize_t emPosixRead(int fd, void *buf, size_t count);

ssize_t emPosixWrite(int fd, const void *buf, size_t count);

struct timeval;

int emPosixSelect(int nfds,
                  fd_set *readfds,
                  fd_set *writefds,
                  fd_set *exceptfds,
                  struct timeval *timeout);

#endif // POSIX_SIM_H
