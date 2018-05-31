#ifndef __MPSI_HANDLERS__
#define __MPSI_HANDLERS__

#include "mpsi.h"
#include "mpsi_ble_transport_client.h"

uint8_t appMpsiGetApplicationInfo(serverList_t server, uint8_t appId);
uint8_t appMpsiBootloadSlot(serverList_t server, uint8_t slotId);
uint8_t appMpsiInitiateJoining(serverList_t server, uint8_t appId, uint8_t option);
uint8_t appMpsiGetZigbeeJoiningDeviceInfo(serverList_t server);
uint8_t appCliMpsiSetZigbeeJoiningDeviceInfo(serverList_t server);
uint8_t appMpsiSetZigbeeJoiningDeviceInfo(serverList_t server, MpsiMessage_t* mpsiMessage);
uint8_t appMpsiGetZigbeeTrustCenterJoiningCredentials(serverList_t server);
uint8_t appMpsiSetZigbeeTrustCenterJoiningCredentials(serverList_t server, MpsiMessage_t* mpsiMessage);

#endif // __MPSI_HANDLERS__
