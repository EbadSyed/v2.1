// Copyright 2015 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

#ifndef ALIAS
  #define ALIAS(x) x
#endif

void ALIAS(emberActiveScanHandler)(const EmberMacBeaconData * beaconData)
{
  uint8_t networkId[EMBER_NETWORK_ID_SIZE + 1] = { 0 };

  MEMCOPY(networkId, beaconData->networkId, EMBER_NETWORK_ID_SIZE);
  emberAfCorePrintln("network id: %s", networkId);

  emberAfCorePrint("extended pan id: ");
  emberAfCoreDebugExec(emberAfPrintExtendedPanId(beaconData->extendedPanId));
  emberAfCorePrintln("");

  emberAfCorePrint("long id: ");
  emberAfCoreDebugExec(
    emberAfPrintBigEndianEui64((const EmberEui64 *)beaconData->longId);
    );
  emberAfCorePrintln("");

  emberAfCorePrintln("pan id: 0x%2x", beaconData->panId);
  emberAfCorePrintln("protocol id: %u", beaconData->protocolId);
  emberAfCorePrintln("channel: %u", beaconData->channel);
  emberAfCorePrintln("allowing join: %s",
                     (beaconData->allowingJoin ? "true" : "false"));
  emberAfCorePrintln("lqi: %u", beaconData->lqi);
  emberAfCorePrintln("rssi: %d dBm", beaconData->rssi);
  emberAfCorePrintln("version: %u", beaconData->version);
  emberAfCorePrintln("short id: 0x%2x", beaconData->shortId);
  emberAfCorePrintln("steering data length: %u", beaconData->steeringDataLength);
  emberAfCorePrint("steering data: ");
  emberAfCorePrintBuffer(beaconData->steeringData,
                         beaconData->steeringDataLength,
                         true);
  emberAfCorePrintln("");
}

void ALIAS(emberEnergyScanHandler)(uint8_t channel, int8_t maxRssiValue)
{
  emberAfCorePrintln("channel %u rssi: %d dBm", channel, maxRssiValue);
}

void ALIAS(emberScanReturn)(EmberStatus status)
{
  emberAfCorePrintln("%p 0x%x", "scan", status);
}
