#include "mpsi.h"
#include "ble-callbacks.h"

MpsiMessage_t mpsiMessage;

uint8_t securityConnection;

typedef enum {
  bleMpsiTransportConnectSecurityPhase_No = 1,
  bleMpsiTransportConnectSecurityPhase_Bonding = 2,
  bleMpsiTransportConnectSecurityPhase_Passkey = 3,
} bleMpsiTransportConnectSecurityPhase_t;

bleMpsiTransportConnectSecurityPhase_t securityPhase = bleMpsiTransportConnectSecurityPhase_No;

void bleMpsiTransportConfirmBondingRequest(struct gecko_cmd_packet *evt)
{
  printf("Client: Confirm bonding to new device! (y/n)\n");
  securityConnection = evt->data.evt_sm_confirm_bonding.connection;
  securityPhase = bleMpsiTransportConnectSecurityPhase_Bonding;
}

void bleMpsiTransportConfirmBondingResponse(uint8_t acceptConnection)
{
  gecko_cmd_sm_bonding_confirm(securityConnection, acceptConnection);
  securityPhase = bleMpsiTransportConnectSecurityPhase_No;
}

void bleMpsiTransportConfirmPasskeyRequest(struct gecko_cmd_packet *evt)
{
  printf("Client: Confirm Passkey: %d ! (y/n)\n", evt->data.evt_sm_confirm_passkey.passkey);
  securityConnection = evt->data.evt_sm_confirm_bonding.connection;
  securityPhase = bleMpsiTransportConnectSecurityPhase_Passkey;
}

void bleMpsiTransportConfirmPasskeyResponse(uint8_t acceptConnection)
{
  gecko_cmd_sm_passkey_confirm(securityConnection, acceptConnection);
  securityPhase = bleMpsiTransportConnectSecurityPhase_No;
}

void bleMpsiTransportConfirmResponse(bool response)
{
  if (bleMpsiTransportConnectSecurityPhase_Passkey == securityPhase) {
    bleMpsiTransportConfirmPasskeyResponse((uint8_t)response);
  } else if (bleMpsiTransportConnectSecurityPhase_Bonding == securityPhase) {
    bleMpsiTransportConfirmBondingResponse((uint8_t)response);
  }
}
