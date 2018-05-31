#ifndef __MPSI_HANDLERS__
#define __MPSI_HANDLERS__

#include "mpsi.h"
#include "mpsi_ble_transport_server.h"

void bleMpsiTransportConfirmBondingRequest(struct gecko_cmd_packet *evt);
void bleMpsiTransportConfirmPasskeyRequest(struct gecko_cmd_packet *evt);
void bleMpsiTransportConfirmResponse(bool response);

#endif // __MPSI_HANDLERS__
