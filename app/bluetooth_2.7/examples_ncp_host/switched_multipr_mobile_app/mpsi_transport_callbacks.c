#include "ble-callbacks.h"
#include "mpsi_ble_transport_client.h"

void bleMpsiTransportLongMessageReceived(uint8_t *buffer, uint8_t len)
{
  int8_t ret;
  MpsiMessage_t mpsiMessage;

  printf("MPSI transport callback: Long Message is received succesfully! Handling MPSI command.\n");

  // Get MPSI data from buffer
  emberAfPluginMpsiDeserialize(buffer, &mpsiMessage);

  // MPSI message received, handle that.
  ret = emberAfPluginMpsiReceiveMessage(&mpsiMessage);
  if (MPSI_WRONG_APP == ret) {
    printf("Received MPSI message with mismatching app ID!");
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
