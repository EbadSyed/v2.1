#include "mpsi.h"
#include "ble-callbacks.h"

MpsiZigbeeJoiningDeviceInfoMessage_t appJoiningDeviceInfo;

// Note: We are referencing for mpsi_transport server list here (ie. available
// server devices) as the mobile application cannot differentiate between
// Trust Center or Joining Device servers simply by checking MPSI appID.
#include "mpsi_ble_transport_client.h"

serverList_t appServerToSend;

// Sends request to applications info from a BLE server
uint8_t appMpsiConfigureAndSendMessage(
  serverList_t server,
  uint8_t destAppId,
  uint16_t msgId,
  uint8_t msgPayloadLen,
  uint8_t *buffer)
{
  MpsiMessage_t appMpsiMessage;

  appMpsiMessage.destinationAppId = destAppId;
  appMpsiMessage.messageId = msgId;
  appMpsiMessage.payloadLength = msgPayloadLen;

  if (msgPayloadLen) {
    if (NULL != buffer) {
      MEMCOPY(appMpsiMessage.payload, buffer, msgPayloadLen);
    } else {
      return MPSI_ERROR;
    }
  }

  // Store server to send to. This is needed because the MPSI API is not able
  // to handle different outgoing destinations. Hence, this is sold in the
  // application.
  appServerToSend = server;

  return emberAfPluginMpsiSendMessage(&appMpsiMessage);
}

// Sends request to applications info from a BLE server
uint8_t appMpsiGetApplicationInfo(serverList_t server, uint8_t appId)
{
  return appMpsiConfigureAndSendMessage(
    server,
    appId,
    MPSI_MESSAGE_ID_GET_APPS_INFO,
    0,
    NULL);
}

// Sends request to joining device to bootload the spcified slot.
uint8_t appMpsiBootloadSlot(serverList_t server, uint8_t slotId)
{
  return appMpsiConfigureAndSendMessage(
    server,
    MPSI_APP_ID_BLE,
    MPSI_MESSAGE_ID_BOOTLOAD_SLOT,
    1,
    &slotId);
}

// Sends the initiate joining message
uint8_t appMpsiInitiateJoining(serverList_t server, uint8_t appId, uint8_t option)
{
  return appMpsiConfigureAndSendMessage(
    server,
    appId,
    MPSI_MESSAGE_ID_INITIATE_JOINING,
    1,
    &option);
}

// Sends request to get zigbee joining info from a BLE server
uint8_t appMpsiGetZigbeeJoiningDeviceInfo(serverList_t server)
{
  return appMpsiConfigureAndSendMessage(
    server,
    MPSI_APP_ID_BLE,
    MPSI_MESSAGE_ID_GET_ZIGBEE_JOINING_DEVICE_INFO,
    0,
    NULL);
}

// Sends zigbee joining info directly with mpsi message
// (ie. appCliMpsiSetZigbeeJoiningDeviceInfo() does it from CLI)
uint8_t appMpsiSetZigbeeJoiningDeviceInfo(serverList_t server, MpsiMessage_t* mpsiMessage)
{
  return appMpsiConfigureAndSendMessage(
    server,
    MPSI_APP_ID_ZIGBEE,
    MPSI_MESSAGE_ID_SET_ZIGBEE_JOINING_DEVICE_INFO,
    mpsiMessage->payloadLength,
    mpsiMessage->payload);
}

// Sends zigbee joining info from cli
uint8_t appCliMpsiSetZigbeeJoiningDeviceInfo(serverList_t server)
{
  MpsiMessage_t mpsiJoiningInfo;
  uint8_t msgLen = appJoiningDeviceInfo.installCodeLength + COUNTOF(appJoiningDeviceInfo.eui64);

  // Only setting payload here, the rest is handled by the called function.
  mpsiJoiningInfo.payloadLength = msgLen;

  emAfPluginMpsiSerializeSpecificMessage(
    &appJoiningDeviceInfo,
    MPSI_MESSAGE_ID_SET_ZIGBEE_JOINING_DEVICE_INFO,
    mpsiJoiningInfo.payload);

  // Call API that sends the specific message.
  appMpsiSetZigbeeJoiningDeviceInfo(server, &mpsiJoiningInfo);

  return MPSI_SUCCESS;
}

// Serialize and send out MPSI message to BLE
uint8_t emAfPluginSendMpsiMessageToStack(MpsiMessage_t* mpsiMessage)
{
  uint8_t buffer[MPSI_MAX_MESSAGE_LENGTH];
  uint16_t len;
  int8_t ret;

  len = emberAfPluginMpsiSerialize(mpsiMessage, buffer);

  ret = bleSendMpsiTransportLongMessage(appServerToSend, buffer, len);

  if (MPSI_TRANSPORT_SUCCESS != ret) {
    printf("MPSI transport: message send failure!\n");
    return MPSI_ERROR;
  }

  return MPSI_SUCCESS;
}

// Sends request to get zigbee joining credentials info from the trust center
uint8_t appMpsiGetZigbeeTrustCenterJoiningCredentials(serverList_t server)
{
  return appMpsiConfigureAndSendMessage(
    server,
    MPSI_APP_ID_ZIGBEE,
    MPSI_MESSAGE_ID_GET_ZIGBEE_TC_JOINING_CREDENTIALS,
    0,
    NULL);
}

// Sends zigbee trust center joining credentials to the joining device directly with mpsi message
uint8_t appMpsiSetZigbeeTrustCenterJoiningCredentials(serverList_t server, MpsiMessage_t* mpsiMessage)
{
  return appMpsiConfigureAndSendMessage(
    server,
    MPSI_APP_ID_ZIGBEE,
    MPSI_MESSAGE_ID_SET_ZIGBEE_TC_JOINING_CREDENTIALS,
    mpsiMessage->payloadLength,
    mpsiMessage->payload);
}
