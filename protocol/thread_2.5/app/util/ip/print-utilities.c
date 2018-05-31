/*
 * File: print-utilities.c
 * Description: Functions for printing out IP stack info.
 *
 * Copyright 2013 by Silicon Laboratories. All rights reserved.             *80*
 */

#include <stdio.h>

#include PLATFORM_HEADER
#include "stack/core/ember-stack.h"
#include "stack/include/ember.h"
#include "hal/hal.h"
#include "plugin/serial/serial.h"

#if CORTEXM3_EFM32_MICRO
#include "em_device.h"
#include "plugin/serial/com.h"
#endif
#include "plugin/serial/ember-printf.h"

#include "stack/ip/ip-address.h"
#include "stack/ip/network-data.h"
#include "app/util/serial/command-interpreter2.h"

#include "print-utilities.h"

#ifdef CORTEXM3
  #include "hal/micro/cortexm3/diagnostic.h"
  #include "stack/config/token-stack.h"
#endif

#if !defined APP_SERIAL
extern uint8_t serialPort;
  #define APP_SERIAL serialPort
#endif

//------------------------------------------------------------------------------
// String interpretations

const char * const networkStatusNames[] = {
  "no network",
  "saved network",
  "joining",
  "joined attached",
  "joined no parent",
  "joined attaching"
};

const uint8_t *networkStatusString(uint8_t networkStatus)
{
  assertReturn(networkStatus < COUNTOF(networkStatusNames), NULL);
  return (const uint8_t*)networkStatusNames[networkStatus];
}

static const char * const nodeTypeNames[] = {
  "unknown",
  "invalid",
  "router",
  "end device",
  "sleepy end device",
  "invalid",
  "lurker",
  "commissioner"
};

const uint8_t *nodeTypeString(uint8_t nodeType)
{
  uint8_t index = nodeType & ~LURKER_NODE_TYPE_BIT;
  assertReturn(index < COUNTOF(nodeTypeNames), NULL);
  return (const uint8_t*)nodeTypeNames[index];
}

static bool isOnLurkerNetwork(uint8_t nodeType)
{
  return (nodeType == EMBER_LURKER || (nodeType & LURKER_NODE_TYPE_BIT));
}

static bool isOnThreadNetwork(uint8_t nodeType)
{
  return (nodeType & ~LURKER_NODE_TYPE_BIT) != EMBER_LURKER;
}

static const char * const wakeupReasonNames[] = {
  "via an over the air packet",
  "via an interrupt",
  "aborted"
};

const uint8_t *wakeupReasonString(uint8_t wakeupReason)
{
  assert(wakeupReason
         < sizeof(wakeupReasonNames) / sizeof(wakeupReasonNames[0]));
  return (const uint8_t*)wakeupReasonNames[wakeupReason];
}

static const char * const appWakeupStateNames[] = {
  "listening",
  "transmitting",
  "not listening"
};

const uint8_t *appWakeupStateString(uint8_t wakeupState)
{
  assert(wakeupState
         < sizeof(appWakeupStateNames) / sizeof(appWakeupStateNames[0]));
  return (const uint8_t*)appWakeupStateNames[wakeupState];
}

static const char * const wakeupStateNames[] = {
  "listen",
  "waking",
  "seq_reset",
  "sync",
  "awake"
};

const uint8_t *wakeupStateString(uint8_t wakeupState)
{
  assert(wakeupState
         < sizeof(wakeupStateNames) / sizeof(wakeupStateNames[0]));
  return (const uint8_t*)wakeupStateNames[wakeupState];
}

static const char * const resetCauses[] = {
  "UNKNOWN",
  "FIB",
  "BOOTLOADER",
  "EXTERNAL",
  "POWER ON",
  "WATCHDOG",
  "SOFTWARE",
  "CRASH",
  "FLASH",
  "FATAL",
  "FAULT",
  "BROWNOUT"
};

const uint8_t *resetCauseString(uint8_t resetCause)
{
  assert(resetCause < sizeof(resetCauses) / sizeof(resetCauses[0]));
  return (const uint8_t *)resetCauses[resetCause];
}

typedef struct {
  uint8_t type;
  const char *name;
} TlvNameMap;

static const TlvNameMap networkDataTlvMap[] = {
  { NWK_DATA_HAS_ROUTE, "NWK_DATA_HAS_ROUTE" },
  { NWK_DATA_PREFIX, "NWK_DATA_PREFIX" },
  { NWK_DATA_BORDER_ROUTER, "NWK_DATA_BORDER_ROUTER" },
  { NWK_DATA_6LOWPAN_ID, "NWK_DATA_6LOWPAN_ID" },
  { NWK_DATA_COMMISSION, "NWK_DATA_COMMISSION" },
};

//------------------------------------------------------------------------------
// Print Utilities

void printNetworkId(const uint8_t* networkId)
{
  uint8_t nwId[EMBER_NETWORK_ID_SIZE + 1];
  MEMCOPY(nwId, networkId, EMBER_NETWORK_ID_SIZE);
  nwId[EMBER_NETWORK_ID_SIZE] = '\0';
  emberSerialPrintf(APP_SERIAL, "%p", nwId);
}

void printEui(const EmberEui64 *eui)
{
  uint8_t i;
  emberSerialPrintf(APP_SERIAL, "0x");

  for (i = 0; i < 8; i++) {
    emberSerialPrintf(APP_SERIAL, "%x", eui->bytes[7 - i]);
  }
}

void printIpHeader(Ipv6Header *ipHeader)
{
  emberSerialPrintf(APP_SERIAL, "s=");
  printIpAddress(ipHeader->source);
  emberSerialPrintf(APP_SERIAL, " d=");
  printIpAddress(ipHeader->destination);
}

void printUdpHeader(Ipv6Header *ipHeader)
{
  emberSerialPrintf(APP_SERIAL,
                    "UDP sp=%u dp=%u",
                    ipHeader->sourcePort,
                    ipHeader->destinationPort);
}

void printIcmpHeader(Ipv6Header *ipHeader)
{
  emberSerialPrintf(APP_SERIAL,
                    (ipHeader->icmpType == ICMP_ECHO_REPLY)
                    ? "ICMP ECHO_REPLY"
                    : (ipHeader->icmpType == ICMP_ECHO_REQUEST
                       ? "ICMP ECHO_REQUEST"
                       : "ICMP t=%d c=%d"),
                    ipHeader->icmpType,
                    ipHeader->icmpCode);
}

void printPayload(const uint8_t *payload, uint16_t length)
{
  uint16_t i;
  emberSerialPrintf(APP_SERIAL, "[");
  for (i = 0; i < length; i++) {
    emberSerialPrintf(APP_SERIAL, "%x", payload[i]);
  }
  emberSerialPrintf(APP_SERIAL, "]");
}

void printIpPacket(Ipv6Header *ipHeader)
{
  printIpHeader(ipHeader);
  emberSerialPrintf(APP_SERIAL, " ");
  switch (ipHeader->transportProtocol) {
    case IPV6_NEXT_HEADER_UDP:
      printUdpHeader(ipHeader);
      break;
    case IPV6_NEXT_HEADER_ICMPV6:
      printIcmpHeader(ipHeader);
      break;
  }
  emberSerialPrintf(APP_SERIAL, " ");
  printPayload(ipHeader->transportPayload, ipHeader->transportPayloadLength);
  emberSerialPrintCarriageReturn(APP_SERIAL);
}

void printUdpPacket(EmberUdpConnectionData *connectionData,
                    uint8_t *packet,
                    uint16_t packetLength)
{
  emberSerialPrintf(APP_SERIAL, "UDP ");
  printUdpConnection(connectionData);
  emberSerialPrintf(APP_SERIAL, " ");
  printPayload(packet, packetLength);
  emberSerialPrintCarriageReturn(APP_SERIAL);
}

void printUdpPacketVerbose(const uint8_t *destinationIpv6Address,
                           const uint8_t *sourceIpv6Address,
                           uint16_t localPort,
                           uint16_t remotePort,
                           const uint8_t *packet,
                           uint16_t packetLength)
{
  emberSerialPrintf(APP_SERIAL, "UDP ");
  emberSerialPrintf(APP_SERIAL, "dest=");
  printIpAddress(destinationIpv6Address);
  emberSerialPrintf(APP_SERIAL, " source=");
  printIpAddress(sourceIpv6Address);
  emberSerialPrintf(APP_SERIAL,
                    " local=%u remote=%u ",
                    localPort,
                    remotePort);
  printPayload(packet, packetLength);
  emberSerialPrintCarriageReturn(APP_SERIAL);
}

void printDefaultIp(void)
{
  EmberIpv6Address localIp;
  if (emApiGetLocalIpAddress(0, &localIp)) {
    emberSerialPrintf(APP_SERIAL, "default ip: ");
    printIpAddress(localIp.bytes);
    emberSerialPrintCarriageReturn(APP_SERIAL);
  }
}

void printAllAddresses(void)
{
  EmberIpv6Address address;

  EmberNetworkParameters network;
  emberGetNetworkParameters(&network);

  if (isOnThreadNetwork(network.nodeType)) {
    emberSerialPrintf(APP_SERIAL, "ula prefix: ");
    printHexWords(network.ulaPrefix.bytes, 4);
    emberSerialPrintCarriageReturn(APP_SERIAL);
    printDefaultIp();

    emberSerialPrintf(APP_SERIAL, "mesh local identifier: ");
  #ifdef CORTEXM3
    if (emApiNetworkStatus() == EMBER_SAVED_NETWORK) {
      tokTypeIpStackMeshLocalInterfaceId mlIdToken = { 0 };
      halCommonGetToken(&mlIdToken, TOKEN_IP_STACK_MESH_LOCAL_INTERFACE_ID);
      printExtendedPanId(mlIdToken.meshLocalInterfaceId);
    } else {
      printExtendedPanId(emMeshLocalIdentifier);
    }
  #else
    printExtendedPanId(emMeshLocalIdentifier);
  #endif
    emberSerialPrintCarriageReturn(APP_SERIAL);

    emberSerialPrintf(APP_SERIAL, "mac extended id: {");
    uint8_t i;
    for (i = 0; i < 8; i++) {
      emberSerialPrintf(APP_SERIAL, "%x", emMacExtendedId[7 - i]);
    }
    emberSerialPrintf(APP_SERIAL, "}");
    emberSerialPrintCarriageReturn(APP_SERIAL);

    if (isOnLurkerNetwork(network.nodeType)) {
      emberSerialPrintf(APP_SERIAL, "legacy prefix: ");
      printHexWords(network.legacyUla.bytes, 4);
      emberSerialPrintCarriageReturn(APP_SERIAL);
      emberSerialPrintf(APP_SERIAL, "legacy ip: ");
      printLurkerNetworkIpAddress(network.legacyUla.bytes);
      emberSerialPrintCarriageReturn(APP_SERIAL);
    }

  #if (!defined(UNIX_HOST) && !defined(UNIX_HOST_SIM) && !defined(RTOS))
    uint16_t nodeId = emberGetNodeId();
    if (emApiNetworkStatus() == EMBER_SAVED_NETWORK) {
      tokTypeIpStackNodeData nodeToken = { 0 };
      halCommonGetToken(&nodeToken, TOKEN_IP_STACK_NODE_DATA);
      nodeId = nodeToken.shortId;
    }

    // Testing purposes only.
    if (emStoreGp16(nodeId, address.bytes)) {
      emberSerialPrintf(APP_SERIAL, "ml16 (rloc): ");
      printIpAddress(address.bytes);
      emberSerialPrintCarriageReturn(APP_SERIAL);
    }
  #endif

    for (i = 0; i < EMBER_MAX_IPV6_ADDRESS_COUNT; i++) {
      if (emApiGetLocalIpAddress(i, &address)) {
        if (i == 0) {
          emberSerialPrintf(APP_SERIAL, "ml64: ");
        } else if (i == 1) {
          emberSerialPrintf(APP_SERIAL, "ll64: ");
        } else {
          emberSerialPrintf(APP_SERIAL, " gua: ");
        }
        printIpAddress(address.bytes);
        emberSerialPrintCarriageReturn(APP_SERIAL);
      } else {
        return;
      }
    }
  }

  if (isOnLurkerNetwork(network.nodeType)) {
    emberSerialPrintf(APP_SERIAL, "legacy ula prefix: ");
    printHexWords(network.legacyUla.bytes, 4);
    emberSerialPrintCarriageReturn(APP_SERIAL);
    emberSerialPrintf(APP_SERIAL, "legacy default ip: ");
    printLurkerNetworkIpAddress(network.legacyUla.bytes);
    emberSerialPrintCarriageReturn(APP_SERIAL);
    emberSerialPrintf(APP_SERIAL, "legacy ll64: ");
    emApiGetLocalIpAddress(1, &address);
    printIpAddress(address.bytes);
    emberSerialPrintCarriageReturn(APP_SERIAL);
  }
}

void printAsciiPrintable(uint8_t *bytes, uint16_t length)
{
  uint16_t i;
  for (i = 0; i < length; i++) {
    uint8_t next = bytes[i];
    emberSerialPrintf(APP_SERIAL, "%c",
                      (next < 0x20 || next > 0x7E) ? '.' : next);
  }
}

void printUdpConnection(EmberUdpConnectionData *connectionData)
{
  emberSerialPrintf(APP_SERIAL, "peer=");
  printIpAddress(connectionData->remoteAddress);
  emberSerialPrintf(APP_SERIAL,
                    " local=%u remote=%u",
                    connectionData->localPort,
                    connectionData->remotePort);
}

uint16_t joinStartTimeQs;

void printStatus(EmberStatus status, const char *name, bool printTime)
{
  bool success = (status == EMBER_SUCCESS);
  uint16_t time = halCommonGetInt16uQuarterSecondTick() - joinStartTimeQs;
  if (success) {
    printNetworkState();
  }
  emberSerialPrintf(APP_SERIAL, "%s %s", name, success ? "complete" : "failed");
  if (printTime) {
    emberSerialPrintf(APP_SERIAL,
                      " in %u.%u seconds",
                      time >> 2,
                      25 * (time & 3));
  }
  emberSerialPrintfLine(APP_SERIAL, " (status 0x%x)", status);
}

void printFormStatus(EmberStatus status)
{
  printStatus(status, "Form", true);
}

void printJoinStatus(EmberStatus status)
{
  printStatus(status, "Join", true);
}

void printExtendedPanId(uint8_t *extendedPanId)
{
  uint16_t i;
  emberSerialPrintf(APP_SERIAL, "{");
  for (i = 0; i < EXTENDED_PAN_ID_SIZE; i++) {
    emberSerialPrintf(APP_SERIAL, "%x", extendedPanId[i]);
  }
  emberSerialPrintf(APP_SERIAL, "}");
}

void printNetworkState(void)
{
  EmberNetworkStatus status = emApiNetworkStatus();
  EmberNetworkParameters network;
  emApiGetNetworkParameters(&network);

  emberSerialPrintfLine(APP_SERIAL,
                        "network status: %p",
                        networkStatusString(status));
  emberSerialPrintf(APP_SERIAL, "eui64: ");
  printEui((EmberEui64 *)emberEui64()->bytes);
  emberSerialPrintCarriageReturn(APP_SERIAL);

  if (status != EMBER_NO_NETWORK) {
    emberSerialPrintf(APP_SERIAL, "network id: ");
    printNetworkId(network.networkId);
    emberSerialPrintCarriageReturn(APP_SERIAL);
    emberSerialPrintf(APP_SERIAL,
                      "node type: %p",
                      nodeTypeString(network.nodeType));
    if (isOnThreadNetwork(network.nodeType)
        && isOnLurkerNetwork(network.nodeType)) {
      emberSerialPrintfLine(APP_SERIAL, "-lurker");
    } else {
      emberSerialPrintCarriageReturn(APP_SERIAL);
    }

    emberSerialPrintf(APP_SERIAL, "extended pan id: ");
    printExtendedPanId(network.extendedPanId);
    emberSerialPrintCarriageReturn(APP_SERIAL);

    emberSerialPrintfLine(APP_SERIAL, "pan id: 0x%2X", network.panId);
    emberSerialPrintfLine(APP_SERIAL, "channel: %u", network.channel);
    emberSerialPrintfLine(APP_SERIAL,
                          "radio tx power: %d dBm ",
                          network.radioTxPower);
    printAllAddresses();
  }
}

void printResetCause(EmberResetCause cause)
{
  emberSerialPrintfLine(APP_SERIAL, "Reset: %p", resetCauseString(cause));
#ifdef EMBER_STACK_COBRA
  halPrintAssertInfo(APP_SERIAL);   // if an assert prints add'l data
#endif
}

// Hal-specific version.
void printResetInfo(void)
{
  emberSerialPrintfLine(APP_SERIAL, "Reset: %s", halGetResetString());
  #if defined (CORTEXM3)
  if (halResetWasCrash()) {
    halPrintCrashSummary(APP_SERIAL);
    uint16_t reason = halGetExtendedResetInfo();
    const HalAssertInfoType *assertInfo = halGetAssertInfo();
    if (reason != RESET_CRASH_ASSERT || assertInfo->file != NULL) {
      halPrintCrashDetails(APP_SERIAL);
      halPrintCrashData(APP_SERIAL);
    }
  }
  #endif
}

void printHexWords(const uint8_t *contents, uint8_t wordCount)
{
  uint8_t i;
  for (i = 0; i < wordCount; i++) {
    uint16_t word = emberFetchHighLowInt16u(contents + (i << 1));
    emberSerialPrintf(APP_SERIAL, "%p%2x", (i == 0) ? "" : ":", word);
  }
}

void printIpAddress(const uint8_t *address)
{
  printHexWords(address, 8);
}

void printLurkerNetworkIpAddress(const uint8_t *prefix)
{
  uint8_t interface[8];
  printHexWords(prefix, 4);
  emberSerialPrintf(APP_SERIAL, ":");
  // Do not change this to some other interface id, please.
  emberReverseMemCopy(interface, emMacExtendedId, 8);
  interface[0] ^= 0x02;
  printHexWords(interface, 4);
}

void ipAddressToString(const uint8_t *address, uint8_t *buffer, uint8_t bufferLength)
{
  uint8_t *finger = buffer;
  uint8_t i;
  for (i = 0; i < 16; i += 2) {
    if (i != 0) {
      *finger++ = ':';
    }
    finger = emWriteHexInternal(finger,
                                HIGH_LOW_TO_INT(address[i], address[i + 1]),
                                4);
  }
  *finger++ = '\0';
  assert(finger <= buffer + bufferLength);
}

void printRipEntry(uint8_t index, const EmberRipEntry *entry)
{
  uint8_t in = entry->incomingLinkQuality;
  uint8_t out = entry->outgoingLinkQuality;
  if (entry->ripMetric != 0
      || entry->nextHopIndex != 0
      || in != 0
      || out != 0
      || !isNullEui64(entry->longId)) {
    if (entry->type == EMBER_ROUTER) {
      uint8_t in = entry->incomingLinkQuality;
      uint8_t out = entry->outgoingLinkQuality;
      emberSerialPrintf(APP_SERIAL, "index:%u longId:", index);
      uint8_t i;
      for (i = 0; i < 8; i++) {
        emberSerialPrintf(APP_SERIAL, "%x", entry->longId[7 - i]);
      }
      emberSerialPrintfLine(APP_SERIAL,
                            " next_hop_index:%u metric:%u in:%u out:%u rssi:%d sync:%s age:%d",
                            entry->nextHopIndex,
                            entry->ripMetric,
                            in,
                            out,
                            entry->rollingRssi,
                            entry->mleSync ? "y" : "-",
                            entry->age);
    } else {
      emberSerialPrintf(APP_SERIAL, "LURKER index:%u longId:", index);
      uint8_t i;
      for (i = 0; i < 8; i++) {
        emberSerialPrintf(APP_SERIAL, "%x", entry->longId[7 - i]);
      }
      emberSerialPrintfLine(APP_SERIAL,
                            " in:%u out:%u rssi:%d sync:%s age:%d",
                            entry->incomingLinkQuality,
                            entry->outgoingLinkQuality,
                            entry->rollingRssi,
                            entry->mleSync ? "y" : "-",
                            entry->age);
    }
  }
}

void printWakeupState(uint8_t wakeupState, uint16_t wakeupSequenceNumber)
{
  emberSerialPrintfLine(APP_SERIAL,
                        "state: %p | sequence no. %d",
                        wakeupStateString(wakeupState),
                        wakeupSequenceNumber);
}

void printTlvs(const uint8_t *finger, const uint8_t *end)
{
  while (finger < end) {
    uint8_t type = emNetworkDataType(finger);
    uint8_t length = emNetworkDataSize(finger);
    const uint8_t *data = emNetworkDataPointer(finger);
    uint8_t i, j;
    uint8_t index = 0xFF;

    for (i = 0; i < COUNTOF(networkDataTlvMap); i++) {
      if (networkDataTlvMap[i].type == type) {
        emberSerialPrintf(APP_SERIAL, "[%u] %s", i, networkDataTlvMap[i].name);
        index = i;
        break;
      }
    }

    assertReturn(index < COUNTOF(networkDataTlvMap));

    emberSerialPrintf(APP_SERIAL, " %u bytes: ", length);
    for (j = 0; j < length; j++) {
      emberSerialPrintf(APP_SERIAL, "%s%x", (j > 0 ? " " : ""), data[j]);
    }

    if (networkDataTlvMap[index].type == NWK_DATA_PREFIX) {
      emberSerialPrintf(APP_SERIAL, "\nservice TLVs:\n");
      // print the service tlvs
      printTlvs(finger + prefixTlvHeaderSize(finger), data + length);
      emberSerialPrintf(APP_SERIAL, "/service TLVs\n");
    }

    emberSerialPrintfLine(APP_SERIAL, "");

    finger += (length + 2);
  }
}
