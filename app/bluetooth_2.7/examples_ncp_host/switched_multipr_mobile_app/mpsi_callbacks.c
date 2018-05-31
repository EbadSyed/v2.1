#include "mpsi.h"
#include "ble-callbacks.h"
#include "mpsi_handlers.h"

// Note: We are referencing for mpsi_transport server list here (ie. available
// server devices) as the mobile application cannot differentiate between
// Trust Center or Joining Device servers simply by checking MPSI appID.
#include "mpsi_ble_transport_client.h"

uint8_t mpsiHandleMessageGetAppsInfo(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("MPSI callback: mpsiGetAppsInfoHandler. - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}

uint8_t mpsiHandleMessageAppsInfo(MpsiMessage_t* mpsiMessage)
{
  MpsiAppsInfoMessage_t appsInfoMessage;
  uint8_t * infoMessageInstance = mpsiMessage->payload;
  uint8_t msgLen, totalMsgLen;

  if (!mpsiMessage) {
    printf("MPSI mpsiAppsInfoHandler ERROR: NULL message!");
    return MPSI_INVALID_PARAMETER;
  }

  totalMsgLen = mpsiMessage->payloadLength;

  printf("Apps Info message is received!\n Full length: %d\n", totalMsgLen);

  // There can be several apps info messages in one MPSI message. Print all of those.
  while (totalMsgLen) {
    msgLen = emAfPluginMpsiDeserializeSpecificMessage(
      infoMessageInstance,
      MPSI_MESSAGE_ID_APPS_INFO,
      &appsInfoMessage);

    printf("Apps Info:\n - slotId: %d\n - appId: %d\n - appVersion: 0x%08x\n - maxMsgIdSupported: %d\n",
           appsInfoMessage.slotId,
           appsInfoMessage.applicationId,
           appsInfoMessage.applicationVersion,
           appsInfoMessage.maxMessageIdSupported);

    infoMessageInstance += msgLen;
    totalMsgLen -= msgLen;
  }

  return MPSI_SUCCESS;
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

  return MPSI_SUCCESS;
}

uint8_t mpsiHandleMessageInitiateJoining(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("MPSI callback: mpsiHandleMessageInitiateJoining. - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}

uint8_t mpsiHandleMessageGetZigbeeJoiningDeviceInfo(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("ERROR: MPSI callback: mpsiHandleMessageGetZigbeeJoiningDeviceInfo - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}

uint8_t mpsiHandleMessageZigbeeJoiningDeviceInfo(MpsiMessage_t* mpsiMessage)
{
  uint8_t i;
  MpsiZigbeeJoiningDeviceInfoMessage_t deviceInfoMessage;
  uint8_t bytesDeserialized;

  if (!mpsiMessage) {
    printf("MPSI mpsiHandleMessageZigbeeJoiningDeviceInfo ERROR: NULL message!");
    return MPSI_INVALID_PARAMETER;
  }

  printf("MPSI callback: mpsiHandleMessageZigbeeJoiningDeviceInfo\n");
  // Forward the message to the trust-center.
  appMpsiSetZigbeeJoiningDeviceInfo(serverNum_Tc, mpsiMessage);

  bytesDeserialized = emAfPluginMpsiDeserializeSpecificMessage(
    mpsiMessage->payload,
    MPSI_MESSAGE_ID_ZIGBEE_JOINING_DEVICE_INFO,
    &deviceInfoMessage);

  if (mpsiMessage->payloadLength != bytesDeserialized) {
    printf("MPSI (0x%x) error: deserialize error with len %d (%d)",
           mpsiMessage->messageId, bytesDeserialized,
           mpsiMessage->payloadLength);
    return MPSI_INVALID_PARAMETER;
  }

  // Print stuff.
  printf("  - EUI64:");
  for (i = 0; i < 8; i++) {
    printf(" 0x%02x", deviceInfoMessage.eui64[i]);
  }
  printf("\n");

  printf("  - Install Code Length: %d", deviceInfoMessage.installCodeLength);

  printf("  - Install Code:");
  for (i = 0; i < deviceInfoMessage.installCodeLength - 2; i++) {
    printf(" 0x%02x", deviceInfoMessage.installCode[i]);
  }
  printf("\n");
  printf("  - Install Code CRC:");
  for (; i < deviceInfoMessage.installCodeLength; i++) {
    printf(" 0x%02x", deviceInfoMessage.installCode[i]);
  }
  printf("\n");

  return MPSI_SUCCESS;
}

uint8_t mpsiHandleMessageSetZigbeeJoiningDeviceInfo(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("MPSI callback: mpsiHandleMessageSetZigbeeJoiningDeviceInfo. - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}

uint8_t mpsiHandleMessageGetZigbeeTrustCenterJoiningCredentials(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("MPSI callback: mpsiHandleMessageGetZigbeeTrustCenterJoiningCredentials - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}

uint8_t mpsiHandleMessageZigbeeTrustCenterJoiningCredentials(MpsiMessage_t* mpsiMessage)
{
  uint8_t i;
  MpsiZigbeeTrustCenterJoiningCredentialsMessage_t joiningCredentialMessages;
  uint8_t bytesDeserialized;

  if (!mpsiMessage) {
    printf("MPSI mpsiHandleMessageZigbeeTrustCenterJoiningCredentials ERROR: NULL message!");
    return MPSI_INVALID_PARAMETER;
  }

  printf("MPSI callback: mpsiHandleMessageZigbeeTrustCenterJoiningCredentials\n");
  // Forward the message to the joining device.
  appMpsiSetZigbeeTrustCenterJoiningCredentials(serverNum_Smpd, mpsiMessage);

  bytesDeserialized = emAfPluginMpsiDeserializeSpecificMessage(
    mpsiMessage->payload,
    MPSI_MESSAGE_ID_ZIGBEE_TC_JOINING_CREDENTIALS,
    &joiningCredentialMessages);

  if (mpsiMessage->payloadLength != bytesDeserialized) {
    printf("MPSI (0x%x) error: deserialize error with len %d (%d)",
           mpsiMessage->messageId, bytesDeserialized,
           mpsiMessage->payloadLength);
    return MPSI_INVALID_PARAMETER;
  }

  // Print stuff.
  printf("  - Channel mask: %0x08", joiningCredentialMessages.channelMask);

  printf("  - Extended PAN ID:");
  for (i = 0; i < ZIGBEE_EXT_PAN_LENGTH; i++) {
    printf(" 0x%02x", joiningCredentialMessages.extendedPanId[i]);
  }
  printf("\n");

  printf("  - Preconfigured key:");
  for (i = 0; i < ZIGBEE_KEY_LENGTH; i++) {
    printf(" 0x%02x", joiningCredentialMessages.preconfiguredKey[i]);
  }
  printf("\n");

  return MPSI_SUCCESS;
}

uint8_t mpsiHandleMessageSetZigbeeTrustCenterJoiningCredentials(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("MPSI callback: mpsiHandleMessageSetZigbeeTrustCenterJoiningCredentials - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}
