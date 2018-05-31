#include "mpsi.h"
#include "mpsi_ble_transport_server.h"
#include "ble-callbacks.h"
#include "mfg_data_interface.h"
#include "ble-configuration.h"

#if defined (SILABS_AF_PLUGIN_SLOT_MANAGER)
#include "slot-manager.h"
#endif // EMBER_AF_PLUGIN_SLOT_MANAGER

// Application version also defined in application_properties.c
// Best practise is to define this as an input definition to the complier.
#ifndef APP_PROPERTIES_VERSION
  #define APP_PROPERTIES_VERSION 0
#endif // APP_PROPERTIES_VERSION

#if defined (SILABS_AF_PLUGIN_SLOT_MANAGER)
uint8_t bootloaderAppTypeToMpsiAppType(uint32_t type);
#endif // EMBER_AF_PLUGIN_SLOT_MANAGER

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

  // First put in ourself in the response
  appsInfoMessage.slotId = INVALID_SLOT;
  appsInfoMessage.applicationId = MPSI_APP_ID;
  appsInfoMessage.applicationVersion = APP_PROPERTIES_VERSION;
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

#if defined (SILABS_AF_PLUGIN_SLOT_MANAGER)
  // Now fill out the rest based on what's in the slots
  uint8_t slot = 0;
  SlotManagerSlotInfo_t slotInfo;
  while (SLOT_MANAGER_SUCCESS
         == emberAfPluginSlotManagerGetSlotInfo(slot, &slotInfo)) {
    appsInfoMessage.slotId = slot;
    appsInfoMessage.applicationId =
      bootloaderAppTypeToMpsiAppType(slotInfo.slotAppInfo.type);
    appsInfoMessage.applicationVersion = slotInfo.slotAppInfo.version;
    appsInfoMessage.maxMessageIdSupported = 0;

    printf("Slot: %d, AppId: %d, App Version: 0x%8x, Max msg ID supported: %d\n",
           appsInfoMessage.slotId,
           appsInfoMessage.applicationId,
           appsInfoMessage.applicationVersion,
           appsInfoMessage.maxMessageIdSupported);

    bytesSerialized = emAfPluginMpsiSerializeSpecificMessage(
      &appsInfoMessage,
      response.messageId,
      (uint8_t*)(response.payload + response.payloadLength) );

    if (0 == bytesSerialized) {
      mpsiPrintln("MPSI (0x%x) error: serialize error with len %d",
                  mpsiMessage->messageId, sizeof(appsInfoMessage));
      return MPSI_ERROR;
    }
    response.payloadLength += bytesSerialized;

    slot++;
  }
#endif // EMBER_AF_PLUGIN_SLOT_MANAGER

  return emberAfPluginMpsiSendMessage(&response);
}

uint8_t mpsiHandleMessageAppsInfo(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("MPSI callback: Apps Info Handler - ERROR: Message is not meant for us!.\n");
  return MPSI_ERROR;
}

typedef struct {
  uint8_t slotId;
  bool isWarned;
} appSlotToBoot_t;

// ZigBee and Thread features an MPSI support bit in the application
// properties field. The running application can check this bit of an
// image in a slot to make sure it supports MPSI.
#define APPLICATION_PROPERTIES_CAPABILITIES_MPSI_SUPPORT_BIT 31
#define APPLICATION_PROPERTIES_CAPABILITIES_MPSI_SUPPORT_MASK \
  (((uint32_t)1u) << APPLICATION_PROPERTIES_CAPABILITIES_MPSI_SUPPORT_BIT)

uint8_t mpsiHandleMessageBootloadSlot(MpsiMessage_t* mpsiMessage)
{
#if defined (SILABS_AF_PLUGIN_SLOT_MANAGER)
  static appSlotToBoot_t appSlotToBoot = { 0xFF, false };
  SlotManagerSlotInfo_t slotInfo;
  MpsiBootloadSlotMessage_t message;
  uint8_t bytesDeserialized;
  uint8_t status;
  bool isSlotBootable = false;

  printf("MPSI callback: Bootload Slot.\n");

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

  status = emberAfPluginSlotManagerGetSlotInfo(message.slotId, &slotInfo);

  // The image should be booted in case any of the following applies:
  //  - BLE image
  //  - Other image that supports MPSI
  //  - Other image that does not support MPSI,
  //    but the client is already warned about it.
  // Otherwise warning is sent back to the client.
  if (SLOT_MANAGER_SUCCESS == status) {
    // Bluetooth image is bootable (no need to test MPSI bit)
    if ( (APPLICATION_TYPE_BLUETOOTH_APP == slotInfo.slotAppInfo.type)
         || (APPLICATION_TYPE_BLUETOOTH == slotInfo.slotAppInfo.type) ) {
      isSlotBootable = true;

      //Test MPSI bit of the image in the slot (not for BLE)
    } else if (APPLICATION_PROPERTIES_CAPABILITIES_MPSI_SUPPORT_MASK
               & slotInfo.slotAppInfo.capabilities) {
      isSlotBootable = true;

      // Client is already warned that image in this slot does not support MPSI
    } else if ((message.slotId == appSlotToBoot.slotId)
               && (appSlotToBoot.isWarned) ) {
      isSlotBootable = true;
    }

    // Boot the slot if requested
    if (isSlotBootable) {
      // This won't return if it succeeds
      status = emberAfPluginSlotManagerVerifyAndBootloadSlot(message.slotId);

      // Send MPSI error about the issue.
    } else {
      MpsiMessage_t response;
      MpsiErrorMessage_t errorMessage;

      // Store slot info, so that slot can be booted nex time
      // if it is still requested
      appSlotToBoot.slotId = message.slotId;
      appSlotToBoot.isWarned = true;

      // Store MPSI error message info
      errorMessage.errorCode = MPSI_UNSUPPORTED_MPSI_FEATURE;
      errorMessage.sourceApplicationId = MPSI_APP_ID_BLE;
      errorMessage.messageIdInError = MPSI_MESSAGE_ID_BOOTLOAD_SLOT;

      // Store header info for MPSI message
      response.destinationAppId = MPSI_APP_ID_MOBILE_APP;
      response.messageId = MPSI_MESSAGE_ID_ERROR;

      // Store info fields to MPSI message
      response.payloadLength = emAfPluginMpsiSerializeSpecificMessage(
        &errorMessage,
        response.messageId,
        response.payload);

      // Return error if no data is stored
      if (!response.payloadLength) {
        return MPSI_ERROR;
      }

      // Send message
      return emberAfPluginMpsiSendMessage(&response);
    }
  }

  return (SLOT_MANAGER_SUCCESS == status) ? MPSI_SUCCESS : MPSI_ERROR;
#else
  printf("MPSI callback: Bootload Slot - ERROR: Message is not supported!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
#endif // EMBER_AF_PLUGIN_SLOT_MANAGER
}

uint8_t mpsiHandleMessageError(MpsiMessage_t* mpsiMessage)
{
  if (!mpsiMessage) {
    return MPSI_INVALID_PARAMETER;
  }

  (void)mpsiMessage;
  printf("MPSI callback: mpsiHandleMessageError.  - ERROR: Message is not meant for us!.\n");
  return MPSI_ERROR;

  // Nothing needs to be done here for the stacks currently
  // Receiving this message is of value to the Mobile App
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
  MpsiMessage_t response;
  MpsiZigbeeJoiningDeviceInfoMessage_t deviceInfoMessage;

  int8_t  ret;
  uint8_t installCodeLengthWoCrc;
  uint16_t installCodeCrc;
  uint8_t i;

  printf("MPSI callback: mpsiHandleMessageGetZigbeeJoiningDeviceInfo\n");

  if (!mpsiMessage) {
    return MPSI_INVALID_PARAMETER;
  }

  // Read install code from main flash
  if (INSTALL_CODE_SUCCESS != getInstallCode(&installCodeLengthWoCrc, deviceInfoMessage.installCode, &installCodeCrc)) {
    printf("Warning: Zigbee install code is not configured on device!\n");
  }

  // Copy CRC to the installCode array in LSByte format.
  deviceInfoMessage.installCode[installCodeLengthWoCrc] = UINT16_TO_BYTE0(installCodeCrc);
  deviceInfoMessage.installCode[installCodeLengthWoCrc + 1] = UINT16_TO_BYTE1(installCodeCrc);

  // Set length of install code field
  deviceInfoMessage.installCodeLength = installCodeLengthWoCrc + INSTALL_CODE_CRC_LEN;

  // Read EUI64 from main flash
  ret = getEui64(deviceInfoMessage.eui64);
  if (EUI64_DEVICE_ID == ret) {
    printf("EUI64: Using unique device ID.\n");
  } else if (EUI64_CUSTOM != ret) {
    printf("EUI64: Using custom value.\n");
  } else {
    printf("Warning: EUI64 is not configured on device! This should not happen...\n");
  }

  // Print stuff.
  printf("  - EUI64:");
  for (i = 0; i < 8; i++) {
    printf(" 0x%02x", deviceInfoMessage.eui64[i]);
  }
  printf("\n");

  printf("  - Install Code length: %d\n", installCodeLengthWoCrc);
  printf("  - Install Code:");
  for (i = 0; i < installCodeLengthWoCrc; i++) {
    printf(" 0x%02x", deviceInfoMessage.installCode[i]);
  }
  printf("\n");
  printf("  - Install Code CRC:");
  for (i = installCodeLengthWoCrc; i < installCodeLengthWoCrc + INSTALL_CODE_CRC_LEN; i++) {
    printf(" 0x%02x", deviceInfoMessage.installCode[i]);
  }
  printf("\n");

  // Store header info for MPSI message
  response.destinationAppId = MPSI_APP_ID_MOBILE_APP;
  response.messageId = MPSI_MESSAGE_ID_ZIGBEE_JOINING_DEVICE_INFO;

  // Store info fields to MPSI message
  response.payloadLength = emAfPluginMpsiSerializeSpecificMessage(
    &deviceInfoMessage,
    response.messageId,
    response.payload);

  // Return error if no data is stored
  if (!response.payloadLength) {
    return MPSI_ERROR;
  }

  // Send message
  return emberAfPluginMpsiSendMessage(&response);
}

uint8_t mpsiHandleMessageZigbeeJoiningDeviceInfo(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("MPSI callback: mpsiHandleMessageZigbeeJoiningDeviceInfo - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}

uint8_t mpsiHandleMessageSetZigbeeJoiningDeviceInfo(MpsiMessage_t* mpsiMessage)
{
  (void)mpsiMessage;
  printf("MPSI callback: mpsiHandleMessageSetZigbeeJoiningDeviceInfo - ERROR: Message is not meant for us!.\n");
  return MPSI_UNSUPPORTED_COMMAND;
}

#if defined (SILABS_AF_PLUGIN_SLOT_MANAGER)
uint8_t bootloaderAppTypeToMpsiAppType(uint32_t type)
{
  uint8_t mpsiAppId = MPSI_APP_ID_NONE;

  switch (type) {
    case APPLICATION_TYPE_ZIGBEE:
      mpsiAppId = MPSI_APP_ID_ZIGBEE;
      break;
    case APPLICATION_TYPE_THREAD:
      mpsiAppId = MPSI_APP_ID_THREAD;
      break;
    case APPLICATION_TYPE_FLEX:
      mpsiAppId = MPSI_APP_ID_CONNECT;
      break;
    case APPLICATION_TYPE_BLUETOOTH_APP:
      mpsiAppId = MPSI_APP_ID_BLE;
      break;
    case APPLICATION_TYPE_MCU:
      mpsiAppId = MPSI_APP_ID_MCU;
      break;
    // APPLICATION_TYPE_BLUETOOTH is left to MPSI_APP_ID_NONE intentionally.
    // Such an app type means that there is no real BLE application only
    // standalone stack section presents.
    case APPLICATION_TYPE_BLUETOOTH:
    case 0:
    default:
      break;
  }

  return mpsiAppId;
}
#endif // SILABS_AF_PLUGIN_SLOT_MANAGER

uint8_t emBleSendMpsiMessageToMobileApp(MpsiMessage_t* mpsiMessage)
{
  uint8_t buffer[MPSI_MAX_MESSAGE_LENGTH];
  uint16_t packetLength;

  if (!mpsiMessage) {
    return MPSI_INVALID_PARAMETER;
  }

  packetLength = emberAfPluginMpsiSerialize(mpsiMessage, buffer);

  if (!packetLength) {
    return MPSI_ERROR;
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
