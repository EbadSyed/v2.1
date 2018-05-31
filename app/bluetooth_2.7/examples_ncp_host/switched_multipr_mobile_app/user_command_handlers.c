#include COMMON_HEADER
#include "mpsi_handlers.h"
#include "mpsi_ble_transport_client.h"
#include "command_interpreter.h"

void cliSendBleTestData(int abc, char **buf)
{
  sendTestData(ciGetUnsigned(buf[1]));
}

void cliConnectRemoteServer(int abc, char **buf)
{
  serverList_t server;

  if (!strcmp(buf[1], "jd")) {
    server = serverNum_Smpd;
  } else if (!strcmp(buf[1], "tc")) {
    server = serverNum_Tc;
  } else {
    printf("Wrong parameter!\n");
    return;
  }

  bleConnect(server);
}

void cliMpsiAppInfo(int abc, char **buf)
{
  serverList_t server;
  uint8_t appId;

  // Check first parameter
  if (!strcmp(buf[1], "jd")) {
    server = serverNum_Smpd;
  } else if (!strcmp(buf[1], "tc")) {
    server = serverNum_Tc;
  } else {
    printf("Wrong 1st parameter!\n");
    return;
  }

  // Check second parameter
  if (!strcmp(buf[2], "ble")) {
    appId = MPSI_APP_ID_BLE;
  } else if (!strcmp(buf[2], "zb")) {
    appId = MPSI_APP_ID_ZIGBEE;
  } else {
    printf("Wrong 2nd parameter!\n");
    return;
  }

  appMpsiGetApplicationInfo(server, appId);
}

void cliMpsiBootloadSlot(int abc, char **buf)
{
  uint32_t slotId = ciGetUnsigned(buf[1]);
  appMpsiBootloadSlot(serverNum_Smpd, slotId);
}

void cliStartDeviceDiscovery(int abc, char **buf)
{
  uint8_t param = ciGetUnsigned(buf[1]);
  bleStartDeviceDiscovery(param);
}

void cliStopDeviceDiscovery(int abc, char **buf)
{
  bleStopDeviceDiscovery();
}

void cliMpsiInitiateJoining(int abc, char **buf)
{
  serverList_t server;
  uint8_t appId;
  uint8_t option = ciGetUnsigned(buf[3]);

  // Check first parameter
  if (!strcmp(buf[1], "jd")) {
    server = serverNum_Smpd;
  } else if (!strcmp(buf[1], "tc")) {
    server = serverNum_Tc;
  } else {
    printf("Wrong 1st parameter!\n");
    return;
  }

  // Check second parameter
  if (!strcmp(buf[2], "ble")) {
    appId = MPSI_APP_ID_BLE;
  } else if (!strcmp(buf[2], "zb")) {
    appId = MPSI_APP_ID_ZIGBEE;
  } else {
    printf("Wrong 2nd parameter!\n");
    return;
  }

  appMpsiInitiateJoining(server, appId, option);
}

void cliMpsiGetZigbeeJoiningInfo(int abc, char **buf)
{
  appMpsiGetZigbeeJoiningDeviceInfo(serverNum_Smpd);
}

void cliMpsiSetZigbeeJoiningInfo(int abc, char **buf)
{
  // Note: This expects that the joining information is stored already in appJoiningDeviceInfo
  appCliMpsiSetZigbeeJoiningDeviceInfo(serverNum_Tc);
}

void cliMpsiGetZigbeeTrustCenterJoiningCredentials(int abc, char **buf)
{
  appMpsiGetZigbeeTrustCenterJoiningCredentials(serverNum_Tc);
}

extern FILE *otaFile;
void cliSendOtaFile(int abc, char **buf)
{
  uint32_t slotId = ciGetUnsigned(buf[2]);
  uint8_t writeOp;

  if (!strcmp(buf[1], "wc")) {
    writeOp = SWITCHED_MULTIPROTOCOL_OTA_WRITE_COMMAND;
  } else if (!strcmp(buf[1], "wr")) {
    writeOp = SWITCHED_MULTIPROTOCOL_OTA_WRITE_REQUEST;
  } else {
    printf("Wrong 1st parameter!\n");
    return;
  }

  bleSendOtaFile(serverNum_Smpd, writeOp, slotId, otaFile);
}

void cliTest(int abc, char **buf)
{
  printf("Test OK!\n");
}
