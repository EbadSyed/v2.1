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
