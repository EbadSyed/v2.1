#include "mpsi.h"
#include "mpsi_ble_transport_server.h"
#include "ble-callbacks.h"

#define BG_APP_PROPERTIES_VERSION 0

uint8_t mpsiHandleMessageGetAppsInfo(MpsiMessage_t* mpsiMessage)
{
  MpsiMessage_t response;
  MpsiAppsInfoMessage_t appsInfoMessage;
  uint8_t bytesSerialized;

  if (!mpsiMessage) {
    return MPSI_INVALID_PARAMETER;
  }

  printf("Collecting Apps Info Data...\n");

  response.destinationAppId = MPSI_APP_ID_MOBILE_APP;
  response.messageId = MPSI_MESSAGE_ID_APPS_INFO;
  response.payloadLength = 0;

  appsInfoMessage.slotId = INVALID_SLOT;
  appsInfoMessage.applicationId = MPSI_APP_ID;
  appsInfoMessage.applicationVersion = BG_APP_PROPERTIES_VERSION;
  appsInfoMessage.maxMessageIdSupported = MPSI_MESSAGE_ID_MAX_ID;

  printf("Slot: %d, AppId: %d, App Version: 0x%8x, Max msg ID supported: %d\n",
         appsInfoMessage.slotId,
         appsInfoMessage.applicationId,
         appsInfoMessage.applicationVersion,
         appsInfoMessage.maxMessageIdSupported);

  bytesSerialized = emAfPluginMpsiSerializeSpecificMessage(&appsInfoMessage,
                                                           response.messageId,
                                                           response.payload);

  if (0 == bytesSerialized) {
    mpsiPrintln("MPSI (0x%x) error: serialize error with len %d",
                mpsiMessage->messageId, sizeof(appsInfoMessage));
    return MPSI_ERROR;
  }
  response.payloadLength += bytesSerialized;

  return emberAfPluginMpsiSendMessage(&response);
}

uint8_t mpsiHandleMessageAppsInfo(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("MPSI callback: Apps Info Handler - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}

uint8_t mpsiHandleMessageBootloadSlot(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("MPSI callback: Bootload Slot. - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}

uint8_t mpsiHandleMessageError(MpsiMessage_t* mpsiMessage)
{
  MpsiErrorMessage_t message;
  uint8_t bytesDeserialized;

  if (!mpsiMessage) {
    return MPSI_INVALID_PARAMETER;
  }

  bytesDeserialized = emAfPluginMpsiDeserializeSpecificMessage(
    mpsiMessage->payload,
    mpsiMessage->messageId,
    &message);

  if (mpsiMessage->payloadLength != bytesDeserialized) {
    mpsiPrintln("MPSI (0x%x) error: deserialize error with len %d (%d)",
                mpsiMessage->messageId, bytesDeserialized,
                mpsiMessage->payloadLength);
    return MPSI_INVALID_PARAMETER;
  }

  printf("MPSI ERROR message received!\n - Application ID: 0x%02x\n"
         " - Message ID: 0x%04x\n - Error code: 0x%02x\n",
         message.sourceApplicationId,
         message.messageIdInError,
         message.errorCode);

  printf("MPSI callback: mpsiHandleMessageError.  - Needs to be forwarded to MA!.\n");
  return MPSI_WRONG_APP;
}

uint8_t mpsiHandleMessageInitiateJoining(MpsiMessage_t* mpsiMessage)
{
  MpsiInitiateJoiningMessage_t message;
  uint8_t bytesDeserialized;

  printf("MPSI callback: mpsiHandleMessageInitiateJoining\n");

  if (!mpsiMessage) {
    return MPSI_INVALID_PARAMETER;
  }

  if (mpsiMessage->payloadLength != sizeof(message)) {
    return MPSI_INVALID_PARAMETER;
  }

  bytesDeserialized = emAfPluginMpsiDeserializeSpecificMessage(
    mpsiMessage->payload,
    mpsiMessage->messageId,
    &message);

  if (mpsiMessage->payloadLength != bytesDeserialized) {
    mpsiPrintln("MPSI (0x%x) error: deserialize error with len %d (%d)",
                mpsiMessage->messageId, bytesDeserialized,
                mpsiMessage->payloadLength);
    return MPSI_INVALID_PARAMETER;
  }

  bleSetTimeoutForAcceptingAllConnections(message.option);

  return MPSI_SUCCESS;
}

uint8_t mpsiHandleMessageGetZigbeeJoiningDeviceInfo(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("MPSI callback: mpsiHandleMessageGetZigbeeJoiningDeviceInfo.  - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}

uint8_t mpsiHandleMessageZigbeeJoiningDeviceInfo(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("MPSI callback: mpsiHandleMessageZigbeeJoiningDeviceInfo.  - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}

uint8_t mpsiHandleMessageSetZigbeeJoiningDeviceInfo(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("MPSI callback: mpsiHandleMessageSetZigbeeJoiningDeviceInfo.  - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}

uint8_t emBleSendMpsiMessageToMobileApp(MpsiMessage_t* mpsiMessage)
{
  uint8_t buffer[MPSI_MAX_MESSAGE_LENGTH];
  uint16_t packetLength;

  if (!mpsiMessage) {
    return MPSI_INVALID_PARAMETER;
  }

  packetLength = emberAfPluginMpsiSerialize(mpsiMessage, buffer);

  if (packetLength != (mpsiMessage->payloadLength + MPSI_MESSAGE_OVERHEAD)) {
    printf("MPSI: emBleSendMpsiMessageToMobileApp: Serialize error with len %d (%d)\n",
           packetLength,
           mpsiMessage->payloadLength + MPSI_MESSAGE_OVERHEAD);
    return MPSI_INVALID_PARAMETER;
  }

  bleSendMpsiTransportLongMessage(buffer, packetLength);

  return MPSI_SUCCESS;
}

uint8_t mpsiHandleMessageGetZigbeeTrustCenterJoiningCredentials(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("MPSI callback: mpsiHandleMessageGetZigbeeTrustCenterJoiningCredentials - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}

uint8_t mpsiHandleMessageZigbeeTrustCenterJoiningCredentials(MpsiMessage_t* mpsiMessage)
{
  // TODO: Check if this is truely not suppoerted or error?
  (void)mpsiMessage;
  printf("MPSI callback: mpsiHandleMessageZigbeeTrustCenterJoiningCredentials - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}

uint8_t mpsiHandleMessageSetZigbeeTrustCenterJoiningCredentials(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("MPSI callback: mpsiHandleMessageSetZigbeeTrustCenterJoiningCredentials - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}
