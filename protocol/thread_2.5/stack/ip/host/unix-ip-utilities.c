// File: unix-ip-utilities.c
//
// Description: Utilities and stubs for running ip on a Unix host.
//
// Copyright 2015 by Silicon Laboratories. All rights reserved.                *80*

// For kill(2), pclose(3), popen(3) in glibc.
#define _POSIX_C_SOURCE 2

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stddef.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>

#ifdef __linux__
  #include <linux/if.h>
  #include <sys/sysctl.h>
  #include <linux/sysctl.h>
  #include <sys/syscall.h>
  #include <linux/if.h>
#endif

#include PLATFORM_HEADER
#include "stack/core/ember-stack.h"
#include "stack/ip/host/unix-interface.h"
#include "stack/ip/ip-address.h"
#include "stack/ip/host/host-address-table.h"

const char *emUnixInterface = "tun0";

#if (defined (UNIX_HOST) && (!defined(__APPLE__)) && !defined(__CYGWIN__))

struct in6_ifreq {
  struct in6_addr ifr6_addr;
  uint32_t ifr6_prefixlen;
  unsigned int ifr6_ifindex;
};

#define INVALID_SOCKET (-1)

// a macro that describes when a temporary socket should be used for ioctls
#define USE_TEMP_SOCKET(sockfd) ((sockfd) == INVALID_SOCKET)

// takes an ifaceName string and gets a corresponding ifreq, if sockfdIn is not
// set to INVALID_SOCKET, it uses that socket file descriptor, otherwise it
// temporarily opens its own socket
static int getIfreq(const char *ifaceName,
                    int sockfdIn,
                    struct ifreq *ifaceRequest)
{
  // get interface name
  int returnCode = 0;
  int sockfd = 0;

  if (!ifaceRequest || !ifaceName) {
    errno = EINVAL;
    return -1;
  }

  if (USE_TEMP_SOCKET(sockfdIn) == false) {
    sockfd = sockfdIn;
  } else {
    // create a temporary socket
    sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
    if (sockfd == -1) {
      printf("Fatal error getIfreq: socket | errno: %d (%s)\n", errno, strerror(errno));
      return sockfd;
    }
  }

  strncpy(ifaceRequest->ifr_name, ifaceName, IFNAMSIZ);

  // get interface index
  if ((returnCode = ioctl(sockfd, SIOGIFINDEX, ifaceRequest)) < 0) {
    printf("Fatal error getIfreq ioctl | errno: %d (%s)\n", errno, strerror(errno));
  }

  if (USE_TEMP_SOCKET(sockfdIn)) {
    close(sockfd);
  }

  return returnCode;
}

// places an EmberIpv6Address into an in6_ifreq and sets in6_ifreq->ifr_ifindex
static int constructIn6AddrIfreq(const EmberIpv6Address *address,
                                 struct in6_ifreq *ifaceRequest6,
                                 int ifrIfindex)
{
  struct sockaddr_in6 sockaddrIn6 = { 0 };
  char ipAddressString[INET6_ADDRSTRLEN] = { 0 };

  if (!address || !ifaceRequest6) {
    errno = EINVAL;
    return -1;
  }

  sockaddrIn6.sin6_family = AF_INET6;
  sockaddrIn6.sin6_port = 0;

  // configure address
  if (inet_ntop(AF_INET6,
                address->bytes,
                ipAddressString,
                sizeof(ipAddressString)) == NULL) {
    printf("Fatal error addAddress: Bad input \"%s\"\n", address->bytes);
    return errno;
  }
  if (inet_pton(AF_INET6,
                ipAddressString,
                (void *)&sockaddrIn6.sin6_addr) <= 0) {
    printf("Fatal error addAddress: Bad input \"%s\"\n", address->bytes);
    return errno;
  }

  memcpy((char *) &ifaceRequest6->ifr6_addr,
         (char *) &sockaddrIn6.sin6_addr,
         sizeof(struct in6_addr));

  ifaceRequest6->ifr6_ifindex = ifrIfindex;
  ifaceRequest6->ifr6_prefixlen = 64;

  return 0;
}

void emberRemoveHostAddress(const EmberIpv6Address *address)
{
  struct ifreq ifaceRequest;
  memset(&ifaceRequest, 0, sizeof(ifaceRequest));
  struct in6_ifreq ifaceRequest6;
  memset(&ifaceRequest6, 0, sizeof(ifaceRequest6));
  int returnCode = 0;

  if (!address
      || emberIsIpv6LoopbackAddress(address)
      || emberIsIpv6UnspecifiedAddress(address)) {
    return;
  }
  //------------------------------------------------------------------------
  // The following ioctls do the equivalent of the following shell commands:
  // ip -6 addr del <address>/64 dev tun0
  // route -A inet6 add <prefix>/64 dev tun0
  //------------------------------------------------------------------------

  // create socket
  int sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
  if (sockfd == -1) {
    printf("Fatal error removeAddress: socket | errno: %d\n", errno);
    return;
  }

  // get ifreq for the emUnixInterface
  if (getIfreq(emUnixInterface, sockfd, &ifaceRequest) != 0) {
    close(sockfd);
    return;
  }

  if ((returnCode = constructIn6AddrIfreq(address,
                                          &ifaceRequest6,
                                          ifaceRequest.ifr_ifindex) != 0)) {
    printf("Fatal error removeAddress: unable to construct ipv6 ifreq.");
  } else {
    // delete address and route
    if (ioctl(sockfd, SIOCDIFADDR, &ifaceRequest6) < 0) {
      if (errno != EEXIST && errno != EADDRNOTAVAIL) {
        // all errors except 'address does not exist' are logged.
        char addr[EMBER_IPV6_BYTES] = { 0 };
        inet_ntop(AF_INET6, address->bytes, addr, sizeof(addr));
        printf("Fatal error removeAddress [%s:%s]: SIOCDIFADDR | errno: %d (%s)\n", addr, emUnixInterface, errno, strerror(errno));
      } else {
        emberAddressConfigurationChangeHandler(address, 0, 0, 0);
      }
    }
  }
  close(sockfd);
}

void emberRemoveAllHostAddresses(void)
{
  printf("Removing any IPv6 addresses configured on the host...\n");
  struct ifreq ifaceRequest;
  memset(&ifaceRequest, 0, sizeof(ifaceRequest));
  struct in6_ifreq ifaceRequest6;
  memset(&ifaceRequest6, 0, sizeof(ifaceRequest6));
  int returnCode = 0;

  struct ifaddrs *ifa = NULL;
  struct ifaddrs *ifa_tmp = NULL;

  if (getifaddrs(&ifa) == -1) {
    printf("Fatal error addAddress: getifaddrs");
    return;
  }

  // create socket
  int sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
  if (sockfd == -1) {
    printf("Fatal error emberRemoveAllHostAddresses: socket | errno: %d (%s)\n", errno, strerror(errno));
    freeifaddrs(ifa);
    return;
  }

  // get ifreq for the emUnixInterface
  if (getIfreq(emUnixInterface, sockfd, &ifaceRequest) != 0) {
    close(sockfd);
    freeifaddrs(ifa);
    return;
  }

  char addr[EMBER_IPV6_PREFIX_STRING_SIZE] = { 0 };

  ifa_tmp = ifa;
  while (ifa_tmp) {
    // if the address is an ipv6 address that is not null or loopback,
    // remove the address from the interface
    if ((ifa_tmp->ifa_addr)
        && (ifa_tmp->ifa_addr->sa_family == AF_INET6)) {
      struct sockaddr_in6 *in6 = ((struct sockaddr_in6*)ifa_tmp->ifa_addr);
      // debug
      // inet_ntop(AF_INET6, &in6->sin6_addr, addr, sizeof(addr));
      // printf("name: %s addr: %s\n", ifa_tmp->ifa_name, addr);
      EmberIpv6Address emberIpv6Address = { { 0 } };

      // copy 128 bits in network-order from sin6_addr to emberIpv6Address
      memcpy(emberIpv6Address.bytes, in6->sin6_addr.s6_addr, EMBER_IPV6_BYTES);

      if (inet_ntop(AF_INET6, emberIpv6Address.bytes, addr, sizeof(addr)) == NULL) {
        printf("Fatal error emberRemoveAllHostAddresses: inet_ntop %d (%s)\n", errno, strerror(errno));
        close(sockfd);
        freeifaddrs(ifa);
        return;
      }

      // skip null and loopback addresses
      if (!emberIsIpv6LoopbackAddress(&emberIpv6Address)
          && !emberIsIpv6UnspecifiedAddress(&emberIpv6Address)) {
        // remove any ipv6 address found
        if ((returnCode = constructIn6AddrIfreq(&emberIpv6Address,
                                                &ifaceRequest6,
                                                ifaceRequest.ifr_ifindex) != 0)) {
          printf("Fatal error emberRemoveAllHostAddresses: unable to construct ipv6 ifreq.");
        } else {
          // delete address and route
          if (ioctl(sockfd, SIOCDIFADDR, &ifaceRequest6) < 0) {
            if (errno != EEXIST && errno != EADDRNOTAVAIL) {
              // all errors except 'address does not exist' are logged.
              printf("Fatal error removeAddress [%s%%%s]: SIOCDIFADDR | errno: %d (%s)\n", addr, emUnixInterface, errno, strerror(errno));
            } else {
              emberAddressConfigurationChangeHandler(&emberIpv6Address, 0, 0, 0);
            }
          }
        }
      }
    }
    ifa_tmp = ifa_tmp->ifa_next;
  }

  if (ifa) {
    freeifaddrs(ifa);
  }

  if (sockfd != -1) {
    close(sockfd);
  }
}

// unsets the IFF_UP and IFF_RUNNING flags of the interface described by the
// ifName string. If sockFdIn is not INAVLID_SOCKET, it uses that socket file
// descriptor, otherwise it temporarily opens its own socket
static int ifUp(const char * ifName, int sockfdIn)
{
  int returnCode = 0;
  int sockfd = 0;
  struct ifreq ifaceRequest;
  memset(&ifaceRequest, 0, sizeof(ifaceRequest));

  if (USE_TEMP_SOCKET(sockfdIn) == false) {
    sockfd = sockfdIn;
  } else {
    // create socket
    sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
    if (sockfd == -1) {
      printf("Fatal error ifUp: socket | errno: %d\n", errno);
      return sockfd;
    }
  }

  // get ifreq for the iface
  if (getIfreq(ifName, sockfd, &ifaceRequest) != 0) {
    if (USE_TEMP_SOCKET(sockfdIn)) {
      close(sockfd);
    }
    return -1;
  }

  // interface up
  ifaceRequest.ifr_flags |= IFF_UP | IFF_RUNNING;

  // set interface flags
  if ((returnCode = ioctl(sockfd, SIOCSIFFLAGS, &ifaceRequest)) < 0) {
    printf("Fatal error ifUp: SIOCSIFFLAGS | errno: %d (%s)\n", errno, strerror(errno));
  }

  if (USE_TEMP_SOCKET(sockfdIn)) {
    close(sockfd);
  }

  return returnCode;
}

static void addAddress(const EmberIpv6Address *address,
                       uint32_t preferredLifetime,
                       uint32_t validLifetime,
                       uint8_t addressFlags)
{
  struct ifreq ifaceRequest;
  memset(&ifaceRequest, 0, sizeof(ifaceRequest));
  struct sockaddr_in6 sockaddrIn6 = { 0 };
  struct in6_ifreq ifaceRequest6;
  memset(&ifaceRequest6, 0, sizeof(ifaceRequest6));
  bool already_assigned = false;
  char ipAddressString[INET6_ADDRSTRLEN];

  memset(&sockaddrIn6, 0, sizeof(struct sockaddr));
  sockaddrIn6.sin6_family = AF_INET6;
  sockaddrIn6.sin6_port = 0;

  // configure address
  if (inet_ntop(AF_INET6,
                address->bytes,
                ipAddressString,
                sizeof(ipAddressString)) == NULL) {
    printf("Fatal error addAddress: Bad input \"%s\"\n", address->bytes);
    return;
  }
  if (inet_pton(AF_INET6,
                ipAddressString,
                (void *) &sockaddrIn6.sin6_addr) <= 0) {
    printf("Fatal error addAddress: Bad input \"%s\"\n", address->bytes);
    return;
  }

  memcpy((char *) &ifaceRequest6.ifr6_addr,
         (char *) &sockaddrIn6.sin6_addr,
         sizeof(struct in6_addr));

  //------------------------------------------------------------------------
  // The following ioctls do the equivalent of the following shell commands:
  // ip -6 addr add <address>/64 dev tun0
  // ifconfig tun0 up
  // route -A inet6 add <prefix>/64 dev tun0
  //------------------------------------------------------------------------

  // create socket
  int sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
  if (sockfd == -1) {
    printf("Fatal error addAddress: socket | errno: %d\n", errno);
    return;
  }

  // get ifreq for the emUnixInterface
  if (getIfreq(emUnixInterface, sockfd, &ifaceRequest) != 0) {
    close(sockfd);
    return;
  }

  if (constructIn6AddrIfreq(address,
                            &ifaceRequest6,
                            ifaceRequest.ifr_ifindex) != 0) {
    printf("Fatal error addAddress: unable to construct ipv6 ifreq.");
    close(sockfd);
    return;
  }

  // add route and ipv6 addr
  if (ioctl(sockfd, SIOCSIFADDR, &ifaceRequest6) < 0) {
    if (errno != EEXIST) {
      printf("Fatal error addAddress: SIOCSIFADDR | errno: %d\n", errno);
      close(sockfd);
      return;
    } else {
      already_assigned = true;
    }
  }

  if (!already_assigned) {
    // get interface flags
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifaceRequest) < 0) {
      printf("Fatal error addAddress: SIOCGIFFLAGS | errno: %d\n", errno);
      close(sockfd);
      return;
    }
    if (ifUp(ifaceRequest.ifr_name, sockfd) < 0) {
      close(sockfd);
      return;
    }
  }

  close(sockfd);

  // check whether address is assigned on interface correctly, and only then
  // call the application handler

  struct ifaddrs *ifa, *ifa_tmp;
  char addr[EMBER_IPV6_PREFIX_STRING_SIZE] = { 0 };
  // Try to search for this address 10 times on the interface.
  uint8_t retries = 10;
  bool addressAssigned = false;

  while (retries > 0) {
    ifa = NULL;
    ifa_tmp = NULL;
    if (getifaddrs(&ifa) == -1) {
      printf("Fatal error addAddress: getifaddrs");
      return;
    }

    ifa_tmp = ifa;
    struct sockaddr_in6 *in6 = NULL;
    while (ifa_tmp) {
      if ((ifa_tmp->ifa_addr)
          && (ifa_tmp->ifa_addr->sa_family == AF_INET6)) {
        // create IPv6 string
        in6 = ((struct sockaddr_in6*)ifa_tmp->ifa_addr);
        inet_ntop(AF_INET6, &in6->sin6_addr.s6_addr, addr, sizeof(addr));
        // debug
        // printf("name: %s addr: %s\n", ifa_tmp->ifa_name, addr);
        if ((strcmp(ifa_tmp->ifa_name, emUnixInterface) == 0)
            && (strcmp(addr, ipAddressString) == 0)) {
          addressAssigned = true;
          goto callback;
        }
      }
      ifa_tmp = ifa_tmp->ifa_next;
    }
    retries--;
  }

  callback:
  if (ifa) {
    freeifaddrs(ifa);
  }

  if (addressAssigned) {
    emberAddressConfigurationChangeHandler(address,
                                           preferredLifetime,
                                           validLifetime,
                                           addressFlags);
  } else {
    printf("Fatal error addAddress: Failed to assign IPv6 address"
           " \"%s\" on \"%s\"\n",
           ipAddressString,
           emUnixInterface);
  }
}

// Configure IP addresses on the host.
void emberConfigureDefaultHostAddress(const EmberIpv6Address *ulAddress)
{
  //------------------------------------------------------------------------
  // Disable ND (RA, RS, etc.) messages from the Linux IPv6 stack
  // Documentation:
  // . https://www.kernel.org/doc/Documentation/networking/ip-sysctl.txt
  //   (for accept_ra, forwarding and router_solicitations)
  // . http://linux.die.net/man/8/radvd
  // . http://www.freebsd.org/cgi/man.cgi?query=rtsol
  // . sysctl interface: http://www.linux.it/~rubini/docs/sysctl/
  //------------------------------------------------------------------------
  // There exists a sysctl syscall to do the following steps, by defining
  // arguments matching the /proc/sys file corresponding to the setting (see
  // http://www.linux.it/~rubini/docs/sysctl/), but using it is strongly
  // discouraged as it is likely to be deprecated in future Linux versions
  // (see man 2 sysctl). Use the /proc/sys/ interface instead.
  //------------------------------------------------------------------------

  // set accept_ra, forwarding and router_solicitations to 0 on the tun0
  // interface
  FILE* procfile = NULL;

  const char * tun0_accept_ra = "/proc/sys/net/ipv6/conf/tun0/accept_ra";
  const char * tun0_forwarding = "/proc/sys/net/ipv6/conf/tun0/forwarding";
  const char * tun0_solicits = "/proc/sys/net/ipv6/conf/tun0/router_solicitations";

  int tun0_accept_ra_value = 0;
  int tun0_forwarding_value = 0;
  int tun0_solicits_value = 0;

  if ((procfile = fopen(tun0_accept_ra, "wb")) != NULL) {
    fprintf(procfile, "%d", tun0_accept_ra_value);
    fclose(procfile);
  } else {
    printf("Warning emberConfigureDefaultHostAddress: fopen accept_ra | errno: %d (%s)\n", errno, strerror(errno));
  }

  if ((procfile = fopen(tun0_forwarding, "wb")) != NULL) {
    fprintf(procfile, "%d", tun0_forwarding_value);
    fclose(procfile);
  } else {
    printf("Warning emberConfigureDefaultHostAddress: fopen forwarding | errno: %d (%s)\n", errno, strerror(errno));
  }

  if ((procfile = fopen(tun0_solicits, "wb")) != NULL) {
    fprintf(procfile, "%d", tun0_solicits_value);
    fclose(procfile);
  } else {
    printf("Warning emberConfigureDefaultHostAddress: fopen router_solicitations | errno: %d (%s)\n", errno, strerror(errno));
  }

  // kill the radvd and rtsol processes if they're running
  char line[10];
  FILE *cmd;
  pid_t pid;
  if ((cmd = popen("pidof -s rtsol", "r")) != NULL) {
    memset(line, 0, 10);
    fgets(line, 10, cmd);
    if ((pid = strtoul(line, NULL, 10)) != 0) {
      if (kill(pid, SIGTERM) < 0) {
        kill(pid, SIGKILL);
      }
    }
    pclose(cmd);
  }

  if (ulAddress == NULL) {
    return;
  }

  // ULA
  addAddress(ulAddress,
             EMBER_MAX_LIFETIME_DELAY_SEC,
             EMBER_MAX_LIFETIME_DELAY_SEC,
             EMBER_LOCAL_ADDRESS);

  // FE80::
  EmberIpv6Address linkLocalAddress;
  emStoreLongFe8Address(emMacExtendedId, linkLocalAddress.bytes);
  addAddress(&linkLocalAddress,
             EMBER_MAX_LIFETIME_DELAY_SEC,
             EMBER_MAX_LIFETIME_DELAY_SEC,
             EMBER_LOCAL_ADDRESS);
}

void emberConfigureLegacyHostAddress(const EmberIpv6Address *address)
{
  if (address == NULL) {
    return;
  }

  // Legacy ULA
  addAddress(address,
             EMBER_MAX_LIFETIME_DELAY_SEC,
             EMBER_MAX_LIFETIME_DELAY_SEC,
             EMBER_LOCAL_ADDRESS);
}

void emberConfigureGlobalHostAddress(const EmberIpv6Address *address,
                                     uint32_t preferredLifetime,
                                     uint32_t validLifetime,
                                     uint8_t addressFlags)
{
  if (address == NULL) {
    return;
  }

  // GUA
  addAddress(address, preferredLifetime, validLifetime, addressFlags);
}

#else
void emberConfigureDefaultHostAddress(const EmberIpv6Address *ulAddress)
{
}

void emberConfigureLegacyHostAddress(const EmberIpv6Address *address)
{
}

void emberConfigureGlobalHostAddress(const EmberIpv6Address *address,
                                     uint32_t preferredLifetime,
                                     uint32_t validLifetime,
                                     uint8_t addressFlags)
{
}

void emberRemoveAllHostAddresses(void)
{
}
#endif // UNIX_HOST
