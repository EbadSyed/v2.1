// Enclosing macro to prevent multiple inclusion
#ifndef __BLE_CALLBACKS__
#define __BLE_CALLBACKS__

#include "mpsi.h"
#include "mpsi-message-ids.h"
#include COMMON_HEADER

// Called when MPSI Get Apps Info message is received
uint8_t mpsiHandleMessageGetAppsInfo(MpsiMessage_t* mpsiMessage);

// Called when MPSI Apps Info message is received
uint8_t mpsiHandleMessageAppsInfo(MpsiMessage_t* mpsiMessage);

// Called when MPSI Bootload Slot message is received
uint8_t mpsiHandleMessageBootloadSlot(MpsiMessage_t* mpsiMessage);

// Called when MPSI Error message is received
uint8_t mpsiHandleMessageError(MpsiMessage_t* mpsiMessage);

// Called when MPSI Initiate Joining message is received
uint8_t mpsiHandleMessageInitiateJoining(MpsiMessage_t* mpsiMessage);

// Called when MPSI Get Zigbee Joining Device Info message is received
uint8_t mpsiHandleMessageGetZigbeeJoiningDeviceInfo(MpsiMessage_t* mpsiMessage);

// Called when MPSI Zigbee Joining Device Info message is received
uint8_t mpsiHandleMessageZigbeeJoiningDeviceInfo(MpsiMessage_t* mpsiMessage);

// Called when MPSI Set Zigbee Joining Device Info message is received
uint8_t mpsiHandleMessageSetZigbeeJoiningDeviceInfo(MpsiMessage_t* mpsiMessage);

// Called when MPSI Get Zigbee Trust Center Joining Credentials message is received
uint8_t mpsiHandleMessageGetZigbeeTrustCenterJoiningCredentials(MpsiMessage_t* mpsiMessage);

// Called when MPSI Zigbee Trust Center Joining Credentials message is received
uint8_t mpsiHandleMessageZigbeeTrustCenterJoiningCredentials(MpsiMessage_t* mpsiMessage);

// Called when MPSI Set Zigbee Trust Center Joining Credentials message is received
uint8_t mpsiHandleMessageSetZigbeeTrustCenterJoiningCredentials(MpsiMessage_t* mpsiMessage);

// Called when an MPSI message is received succesfully
void bleMpsiTransportLongMessageReceived(uint8_t *buffer, uint8_t len);

// Called when an MPSI message is received with CRC failure
void bleMpsiTransportLongMessageReceptionCrcFail(void);

// Called when an MPSI message is sent succesfully
void bleMpsiTransportLongMessageSent(void);

// Called when an MPSI message is sent with CRC failure
void bleMpsiTransportLongMessageSendingCrcFail(void);

// Called when bonding confirmation is requested
void bleMpsiTransportConfirmBonding(struct gecko_cmd_packet *evt);

// Called when passkey confirmation is requested
void bleMpsiTransportConfirmPasskey(struct gecko_cmd_packet *evt);

#endif // __BLE_CALLBACKS__
