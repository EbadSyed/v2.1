#ifndef __MPSI_BLE_TRANSPORT_CLIENT__
#define __MPSI_BLE_TRANSPORT_CLIENT__

#include "mpsi_ble_transport.h"

typedef enum {
  serverNum_Smpd,
  serverNum_Tc,

  serverNum_MaxNum
} serverList_t;

#define SWITCHED_MULTIPROTOCOL_OTA_WRITE_COMMAND        0x01
#define SWITCHED_MULTIPROTOCOL_OTA_WRITE_REQUEST        0x02

#define bleStartDeviceDiscovery(param)  gecko_cmd_le_gap_discover(param)
#define bleStopDeviceDiscovery()        gecko_cmd_le_gap_end_procedure()

#define MPSI_BLE_TRANSPORT_PRINT_ENABLED

#if defined(MPSI_BLE_TRANSPORT_PRINT_ENABLED)
 #define mpsiTransportPrint(...)    do { printf("MPSI transp: "); printf(__VA_ARGS__); } while (0)
 #define mpsiTransportPrintln(...)  do { printf("MPSI transp: "); printf(__VA_ARGS__); printf("\n"); } while (0)
#else
 #define mpsiTransportPrint(...)    (void)0
 #define mpsiTransportPrintln(...)  (void)0
#endif

void bleConnect(serverList_t server);
void sendTestData(uint8_t server);
int8_t bleSendOtaFile(serverList_t server, uint8_t writeOperation, uint32_t slotId, FILE *fileToSend);
int8_t bleSendMpsiTransportLongMessage(serverList_t server, uint8_t *buffer, uint16_t len);

#endif // __MPSI_BLE_TRANSPORT_CLIENT__
