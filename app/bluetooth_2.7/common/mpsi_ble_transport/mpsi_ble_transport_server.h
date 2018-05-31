#ifndef __MPSI_BLE_TRANSPORT_SERVER__
#define __MPSI_BLE_TRANSPORT_SERVER__

#include "mpsi_ble_transport.h"

// Device name text definitions
// Must match length of APP_DEVNAME_DEFAULT after printf formatting
#if defined(BLE_NCP_TRUST_CENTER)
#define APP_DEVNAME                  "SLTC%s"
#define APP_DEVNAME_DEFAULT          "SLTC0000"
#else
#define APP_DEVNAME                  "SLJD%s"
#define APP_DEVNAME_DEFAULT          "SLJD0000"
#endif

// Subtract 1 because of terminating NULL character
#define APP_DEVNAME_LEN              (sizeof(APP_DEVNAME_DEFAULT) - 1)

#define APP_ENABLE_AUTHENTICATION
#define APP_REMOVE_ALL_BONDING_AT_STARTUP

#define APP_MAX_NUMBER_OF_BONDINGS 3

#define MPSI_BLE_TRANSPORT_PRINT_ENABLED

#if defined(MPSI_BLE_TRANSPORT_PRINT_ENABLED)
 #define mpsiTransportPrint(...)    do { printf("MPSI transp: "); printf(__VA_ARGS__); } while (0)
 #define mpsiTransportPrintln(...)  do { printf("MPSI transp: "); printf(__VA_ARGS__); printf("\n"); } while (0)
#else
 #define mpsiTransportPrint(...)    (void)0
 #define mpsiTransportPrintln(...)  (void)0
#endif

void sendBleTestData(uint8_t dataNum);
int8_t bleSendMpsiTransportLongMessage(uint8_t *buffer, uint16_t len);

void bleStartAdvertisement(void);
void bleStopAdvertisement(void);
void bleDisconnect(void);

#if defined(APP_ENABLE_AUTHENTICATION)
void bleSetTimeoutForAcceptingAllConnections(uint8_t timeoutMin);
#endif

#endif // __MPSI_BLE_TRANSPORT_SERVER__
