/** @file tmsp-mfglib.h
 *
 * <!--Copyright 2017 by Silicon Labs. All rights reserved.              *80*-->
 */

#ifndef TMSP_MFGLIB_H
#define TMSP_MFGLIB_H

void tmspMfglibEnd(void);
void tmspMfglibEndTestReturn(EmberStatus status, uint32_t mfgReceiveCount);
void tmspMfglibGet(TmspMfglibValues type);
void tmspMfglibGetChannelReturn(uint8_t channel);
void tmspMfglibGetOptionsReturn(uint8_t options);
void tmspMfglibGetPowerModeReturn(uint16_t txPowerMode);
void tmspMfglibGetPowerReturn(int8_t power);
void tmspMfglibGetSynOffsetReturn(int8_t synOffset);
void tmspMfglibRxReturn(const uint8_t *payload, uint8_t payloadLength, uint8_t lqi, int8_t rssi);
void tmspMfglibSendPacket(const uint8_t *packet, uint8_t packetLength, uint16_t repeat);
void tmspMfglibSendPacketEventHandler(EmberStatus status);
void tmspMfglibSet(TmspMfglibValues type, uint16_t arg1, int8_t arg2);
void tmspMfglibSetReturn(TmspMfglibValues type, EmberStatus status);
void tmspMfglibStart(bool requestRxCallback);
void tmspMfglibStartActivity(TmspMfglibActivities type);
void tmspMfglibStartReturn(TmspMfglibActivities type, EmberStatus status);
void tmspMfglibStartTestReturn(EmberStatus status);
void tmspMfglibStopActivity(TmspMfglibActivities type);
void tmspMfglibStopReturn(TmspMfglibActivities type, EmberStatus status);
void tmspMfglibTestContModCal(uint8_t channel, uint32_t duration);

#endif // TMSP_MFGLIB_H
