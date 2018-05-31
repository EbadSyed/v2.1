#include COMMON_HEADER
#include "mpsi_handlers.h"
#include "mpsi_ble_transport_server.h"
#include "command_interpreter.h"
#include "mfg_data_interface.h"

void cliSendBleTestData(int abc, char **buf)
{
  sendBleTestData(ciGetUnsigned(buf[1]));
}

void cliResponseYes(int abc, char **buf)
{
  bleMpsiTransportConfirmResponse(true);
}

void cliResponseNo(int abc, char **buf)
{
  bleMpsiTransportConfirmResponse(false);
}

void cliStartAdvertise(int abc, char **buf)
{
  bleStartAdvertisement();
}

void cliStopAdvertise(int abc, char **buf)
{
  bleStopAdvertisement();
}

void cliAllowAllConnections(int abc, char **buf)
{
  uint8_t timeoutMin = ciGetUnsigned(buf[1]);
  bleSetTimeoutForAcceptingAllConnections(timeoutMin);
}

void cliDisconnectRemoteClient(int abc, char **buf)
{
  bleDisconnect();
}

void cliPrintManufData(int abc, char **buf)
{
  if (!strcmp(buf[1], "eui64")) {
    uint8_t eui64[EUI64_LEN];
    int8_t ret;

    ret = getEui64(eui64);

    if (EUI64_DEVICE_ID == ret) {
      printf("EUI64 (unique device ID): ");
    } else if (EUI64_CUSTOM == ret) {
      printf("EUI64 (custom): ");
    } else {
      printf("EUI64 ERROR! Unknown return value!");
      return;
    }

    printf("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
           eui64[0],
           eui64[1],
           eui64[2],
           eui64[3],
           eui64[4],
           eui64[5],
           eui64[6],
           eui64[7]);
  } else if (!strcmp(buf[1], "install")) {
    uint8_t len, i;
    uint8_t installCode[INSTALL_CODE_MAX_LEN];
    uint16_t crc;

    if (INSTALL_CODE_SUCCESS == getInstallCode(&len, installCode, &crc)) {
      printf("Install code - len: %d, crc: 0x%04x, data: ", len, crc);
      for (i = 0; i < len; i++) {
        printf("0x%02x ", installCode[i]);
      }
      printf("\n");
    } else {
      printf("Install Code ERROR! Install code not configured!");
    }
  } else {
    printf("Wrong 1st parameter!\n");
  }
}

void cliTest(int abc, char **buf)
{
  printf("Test OK!\n");
}

#define DB_FLASH_PAGE_SIZE  0x800
#define DB_FLASH_ADDRESS    0x3F000

void cliPsRawDump(int abc, char **buf)
{
  uint8_t currentDb;
  uint32_t i;
  uint8_t * bytePtr;

  for (currentDb = 0; currentDb < 2; currentDb++) {
    bytePtr = (uint8_t *)(DB_FLASH_ADDRESS + currentDb * DB_FLASH_PAGE_SIZE);
    printf("\r\n\r\nPrinting a HW page for db_store[%d] starting @ 0x%4X\r\n", currentDb, bytePtr);

    for (i = 0; i < DB_FLASH_PAGE_SIZE; i++) {
      if ((i % 16) == 0) {
        printf("\r\n");
        printf("%02X ", bytePtr[i]);
      } else {
        printf("%02X ", bytePtr[i]);
      }
    }
  }
}
