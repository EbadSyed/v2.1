#include COMMON_HEADER
#include "ble-callbacks.h"
#include "mpsi_ble_transport_server.h"
#include "mpsi_handlers.h"

void bleMpsiTransportLongMessageReceived(uint8_t *buffer, uint8_t len)
{
  int8_t ret;
  MpsiMessage_t mpsiMessage;

  printf("MPSI transport callback: Long Message is received succesfully! Handling MPSI command.\n");

  // Get MPSI data from buffer
  emberAfPluginMpsiDeserialize(buffer, &mpsiMessage);

  // MPSI message received, handle that.
  // First try to process the message.
  ret = emberAfPluginMpsiReceiveMessage(&mpsiMessage);

  // Forward the message if it is not meant for us.
  if (MPSI_WRONG_APP == ret) {
    emberAfPluginMpsiSendMessage(&mpsiMessage);
  } else if (MPSI_UNSUPPORTED_COMMAND == ret) {
    printf("Received MPSI message with unsupported command!");
  } else if (MPSI_SUCCESS != ret) {
    printf("Received MPSI message processing error: %d!\n", ret);
  }
}

void bleMpsiTransportLongMessageReceptionCrcFail(void)
{
  printf("MPSI transport callback: Long Message reception CRC fail!\n");
}

void bleMpsiTransportLongMessageSent(void)
{
  printf("MPSI transport callback: Long Message is sent succesfully!\n");
}

void bleMpsiTransportLongMessageSendingCrcFail(void)
{
  printf("MPSI transport callback: Long Message sending CRC fail!\n");
}

void bleMpsiTransportConfirmBonding(struct gecko_cmd_packet *evt)
{
  printf("MPSI transport callback: Confirm Bonding\n");
  bleMpsiTransportConfirmBondingRequest(evt);
}

void bleMpsiTransportConfirmPasskey(struct gecko_cmd_packet *evt)
{
  printf("MPSI transport callback: Confirm Passkey\n");
  bleMpsiTransportConfirmPasskeyRequest(evt);
}

#if defined(SILABS_AF_PLUGIN_SLOT_MANAGER)
#include "slot-manager.h"
#endif

#define MAX_SLOT_ID 0xFFFFFFFF
uint32_t otaDfuOffset;
uint32_t otaDfuSlotId = MAX_SLOT_ID;

int8_t bleMpsiTransportOtaDfuTransactionBegin(uint8_t len, uint8_t *data)
{
  int8_t ret = MPSI_TRANSPORT_SUCCESS;

  // Erasing specified slot of external flash and starting data download
  otaDfuSlotId = (len > 0) ? BYTES_TO_UINT32(data[0], data[1], data[2], data[3]) : 0;
  otaDfuOffset = 0;

  printf("OTA DFU started.\n");

#if defined(SILABS_AF_PLUGIN_SLOT_MANAGER)
  printf("Erasing slot: %d\n", otaDfuSlotId);
  if (SLOT_MANAGER_SUCCESS != emberAfPluginSlotManagerEraseSlot(otaDfuSlotId)) {
    printf("Erasing slot is failed!\n");
    ret = MPSI_TRANSPORT_ERROR;
  }
#else
  printf("Slot Manager is not available. Received data will only be sent to terminal.\n");
#endif // SILABS_AF_PLUGIN_SLOT_MANAGER

  return ret;
}

void bleMpsiTransportOtaDfuTransactionFinish(uint32_t totalLen)
{
  otaDfuSlotId = MAX_SLOT_ID;
  printf("OTA-DFU flash download finished. Total length: %d\n", totalLen);
}

int8_t bleMpsiTransportOtaDfuDataReceived(uint8_t len, uint8_t *data)
{
  int8_t ret;

#if defined(SILABS_AF_PLUGIN_SLOT_MANAGER)
  ret = emberAfPluginSlotManagerWriteToSlot(otaDfuSlotId, otaDfuOffset, data, len);

  if (SLOT_MANAGER_SUCCESS == ret) {
    otaDfuOffset += len;
    return MPSI_TRANSPORT_SUCCESS;
  }
  return MPSI_TRANSPORT_ERROR;
#else // SILABS_AF_PLUGIN_SLOT_MANAGER
  return MPSI_TRANSPORT_SUCCESS;
#endif // SILABS_AF_PLUGIN_SLOT_MANAGER
}
