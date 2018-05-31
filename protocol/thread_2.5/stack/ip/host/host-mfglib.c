// File: host-mfglib.c
//
// Description: Functions on the host for the manufacturing library, so as to
// test and verify the RF component of products at manufacture time.
//
// Copyright 2013 by Silicon Laboratories. All rights reserved.             *80*

#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/ip-ncp/binary-management.h"
#include "app/tmsp/tmsp-mfglib.h"
#include "app/tmsp/tmsp-enum.h"

static void (*hostMfglibRxCallback)(uint8_t *packet, uint8_t linkQuality, int8_t rssi);

// Host -> NCP management commands

void mfglibStart(void (*mfglibRxCallback)(uint8_t *packet,
                                          uint8_t linkQuality,
                                          int8_t rssi))
{
  hostMfglibRxCallback = mfglibRxCallback;
  tmspMfglibStart(mfglibRxCallback != NULL);
}

void mfglibEnd(void)
{
  hostMfglibRxCallback = NULL;
  tmspMfglibEnd();
}

void mfglibStartTone(void)
{
  tmspMfglibStartActivity(TMSP_MFGLIB_TONE);
}

void mfglibStopTone(void)
{
  tmspMfglibStopActivity(TMSP_MFGLIB_TONE);
}

void mfglibStartStream(void)
{
  tmspMfglibStartActivity(TMSP_MFGLIB_STREAM);
}

void mfglibStopStream(void)
{
  tmspMfglibStopActivity(TMSP_MFGLIB_STREAM);
}

void mfglibSendPacket(uint8_t *packet, uint16_t repeat)
{
  tmspMfglibSendPacket(packet, packet[0] + 1, repeat);
}

void mfglibSetChannel(uint8_t channel)
{
  tmspMfglibSet(TMSP_MFGLIB_CHANNEL, channel, 0);
}

void mfglibGetChannel(void)
{
  tmspMfglibGet(TMSP_MFGLIB_CHANNEL);
}

void mfglibSetPower(uint16_t txPowerMode, int8_t power)
{
  tmspMfglibSet(TMSP_MFGLIB_POWER, txPowerMode, power);
}

void mfglibGetPower(void)
{
  tmspMfglibGet(TMSP_MFGLIB_POWER);
}

void mfglibGetPowerMode(void)
{
  tmspMfglibGet(TMSP_MFGLIB_POWER_MODE);
}

void mfglibSetOptions(uint8_t options)
{
  tmspMfglibSet(TMSP_MFGLIB_OPTIONS, options, 0);
}

void mfglibGetOptions(void)
{
  tmspMfglibGet(TMSP_MFGLIB_OPTIONS);
}

void mfglibSetSynOffset(int8_t synOffset)
{
  tmspMfglibSet(TMSP_MFGLIB_SYN_OFFSET, 0, synOffset);
}

void mfglibGetSynOffset(void)
{
  tmspMfglibGet(TMSP_MFGLIB_SYN_OFFSET);
}

void mfglibTestContModCal(uint8_t channel, uint32_t duration)
{
  tmspMfglibTestContModCal(channel, duration);
}

// NCP -> host callback methods

void tmspMfglibStartTestReturn(EmberStatus status)
{
  mfglibStartReturn(status);
}

void tmspMfglibRxReturn(const uint8_t *payload,
                        uint8_t payloadLength,
                        uint8_t lqi,
                        int8_t rssi)
{
  if (hostMfglibRxCallback) {
    hostMfglibRxCallback((uint8_t *)payload, lqi, rssi);
  }
}

void tmspMfglibEndTestReturn(EmberStatus status,
                             uint32_t mfgReceiveCount)
{
  mfglibEndReturn(status, mfgReceiveCount);
}

void tmspMfglibStartReturn(TmspMfglibActivities type,
                           EmberStatus status)
{
  switch (type) {
    case TMSP_MFGLIB_TONE:
      mfglibStartToneReturn(status);
      break;

    case TMSP_MFGLIB_STREAM:
      mfglibStartStreamReturn(status);
      break;

    default:
      break;
  }
}

void tmspMfglibStopReturn(TmspMfglibActivities type,
                          EmberStatus status)
{
  switch (type) {
    case TMSP_MFGLIB_TONE:
      mfglibStopToneReturn(status);
      break;

    case TMSP_MFGLIB_STREAM:
      mfglibStopStreamReturn(status);
      break;

    default:
      break;
  }
}

void tmspMfglibSendPacketEventHandler(EmberStatus status)
{
  mfglibSendPacketReturn(status);
}

void tmspMfglibSetReturn(TmspMfglibValues type,
                         EmberStatus status)
{
  switch (type) {
    case TMSP_MFGLIB_CHANNEL:
      mfglibSetChannelReturn(status);
      break;

    case TMSP_MFGLIB_POWER:
      mfglibSetPowerReturn(status);
      break;

    case TMSP_MFGLIB_OPTIONS:
      mfglibSetOptionsReturn(status);
      break;

    default:
      break;
  }
}

void tmspMfglibGetChannelReturn(uint8_t channel)
{
  mfglibGetChannelReturn(channel);
}

void tmspMfglibGetPowerReturn(int8_t power)
{
  mfglibGetPowerReturn(power);
}

void tmspMfglibGetPowerModeReturn(uint16_t txPowerMode)
{
  mfglibGetPowerModeReturn(txPowerMode);
}

void tmspMfglibGetSynOffsetReturn(int8_t synOffset)
{
  mfglibGetSynOffsetReturn(synOffset);
}

void tmspMfglibGetOptionsReturn(uint8_t options)
{
  mfglibGetOptionsReturn(options);
}
