// *******************************************************************
// * throughput-library.c
// *
// * Copyright 2017 Silicon Laboratories, Inc.                              *80*
// *******************************************************************

#include "../../include/af.h"
#include "../../util/common.h"
#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/counters/counters.h"
#include "app/framework/plugin/counters/counters-cli.h"
#include "stdio.h"
#include "math.h"
#include "app/framework/util/af-main.h"
#ifdef UNIX_HOST
#include <time.h>
#endif // UNIX_HOST

static uint8_t inflightCount = 1;
static uint32_t okToSendCounter = 0;
static uint32_t packetsApsAcked = 0;
static uint16_t packetsReceived = 0;
static bool isTestRunning = FALSE;
static uint32_t startTime = 0;
static uint32_t stopTime = 0;
static uint32_t testTimeout = 8000000;
static uint32_t runTime = 0;
static uint8_t apsAckEnabled = 0;
static uint16_t duplicates = 0;
static uint16_t hiPacketId = 0;
#define ID_FLAG_SIZE 2048
static uint8_t idFlags[ID_FLAG_SIZE];
#define MAX_PACKET_LENGTH 127
#define END_TEST_DELAY 500
#define MAX_PACKETS_IN_FLIGHT 4

static uint8_t testPacketSrcEndpoint = 1;
static uint8_t testPacketDstEndpoint = 1;
static uint16_t testPacketDstNode = 0;
static uint16_t payloadLen = 32;
static uint16_t headerLen = 0;
static uint16_t packetLen = 0;
static uint16_t packetSendCounter = 0;
static uint16_t packetSendSuccessCounter = 0;
static uint16_t packetSendTargetCount = 10;
static boolean okToSend = FALSE;
static uint16_t txInterval = 250;
static EmberApsFrame* testFrame;
static uint16_t apsOptMask = 0xFFFF;
static uint32_t latencyStartTime = 0;
static uint32_t latencyStopTime = 0;
static int32_t latencyMin = -1;
static int32_t latencyMax = 0;
static int32_t latencyMean = 0;
static int32_t latencyStd = 0;
static boolean delayActive = FALSE;
extern uint16_t emberCounters[EMBER_COUNTER_TYPE_COUNT];
extern PGM_NO_CONST PGM_P titleStrings[];

uint8_t voiceData[] = {
  0x00, 0x00,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB
};

// CLI-runnable functions
void emberAfPluginThroughputLibraryStartTx(void);
void emberAfPluginThroughputLibraryStopTest(void);
void emberAfPluginThroughputLibraryPrintResult(void);
void emberAfPluginThroughputLibrarySetDestination(void);
void emberAfPluginThroughputLibrarySetInterval(void);
void emberAfPluginThroughputLibrarySetTestTimeout(void);
void emberAfPluginThroughputLibrarySetTestCount(void);
void emberAfPluginThroughputLibrarySetInFlightCount(void);
void emberAfPluginThroughputLibrarySetPacketSize(void);
void emberAfPluginThroughputLibrarySetApsAckOff(void);
void emberAfPluginThroughputLibrarySetApsAckOn(void);
void emberAfPluginThroughputLibraryGetTestParams(void);
void emberAfPluginThroughputLibraryClearCounters(void);
//stack events and handlers
EmberEventControl emberAfPluginThroughputLibraryPacketSendEventControl;
void emberAfPluginThroughputLibraryPacketSendEventHandler(void);
EmberEventControl emberAfPluginThroughputLibraryEndTestDelayEventControl;
void emberAfPluginThroughputLibraryEndTestDelayEventHandler(void);
//Other internal functions
static void startTest(void);
static void stopTx(void);
static void activateEndOfTestDelay(void);
static void testCleanup(void);
static void calcRunTime(void);
static void getHeaderLen(void);
static void packetSend(void);
static void printCounter(uint8_t);
static void doubleToString(char* str, double fval);
static void customPrintln(char* str, double fval);
static void notifyApsAck(void);
static void reportRx(uint16_t id);
static void markIdRx(uint16_t id);

void emberAfPluginThroughputLibraryGetTestParams(void)
{
  emberAfCorePrintln(" ");
  emberAfCorePrintln("TEST PARAMETERS ");
  emberAfCorePrintln("Destination nodeID: 0x%2x ", testPacketDstNode);
  emberAfCorePrintln("Packets to send: %d ", packetSendTargetCount);
  emberAfCorePrintln("Transmit interval (ms): %d ", txInterval);
  emberAfCorePrintln("Packet Size: %d bytes", packetLen);
  emberAfCorePrintln("Packets in flight: %d", inflightCount);
  emberAfCorePrintln("APS ACK enabled: %d ", apsAckEnabled);
  emberAfCorePrintln("Timeout: %d ms", testTimeout);
}

/**
 * Set the destination node ID for sending the packet. Also checks packet size
 * again in case source routing has changed.
 */
void emberAfPluginThroughputLibrarySetDestination(void)
{
  testPacketDstNode = (uint16_t)emberUnsignedCommandArgument(0);
  emberAfCorePrintln("Destination nodeID: 0x%2x ", testPacketDstNode);
  getHeaderLen();
  packetLen = payloadLen + headerLen;
  if (packetLen > MAX_PACKET_LENGTH) {
    payloadLen = MAX_PACKET_LENGTH - headerLen;
  }
  emberAfCorePrintln("New packet size: %d ", packetLen);
}

/**
 * Set the target number of packets to be sent for this test. The test will end
 * if this count is reached.
 */
void emberAfPluginThroughputLibrarySetTestCount(void)
{
  packetSendTargetCount = (uint16_t)emberUnsignedCommandArgument(0);
  emberAfCorePrintln("Packets to send: %d ", packetSendTargetCount);
}

/**
 * Set number of ms between transmissions.
 */
void emberAfPluginThroughputLibrarySetInterval(void)
{
  txInterval = (uint16_t)emberUnsignedCommandArgument(0);
  emberAfCorePrintln("Transmit interval (ms): %d ", txInterval);
}

void emberAfPluginThroughputLibrarySetPacketSize(void)
{
  uint8_t newsize = (uint8_t)emberUnsignedCommandArgument(0);
  static uint8_t minPacketLen = 2;

  packetLen = newsize;
  getHeaderLen();
  minPacketLen = headerLen + 2;

  if (packetLen < minPacketLen) {
    emberAfCorePrintln("Too small: adjusting");
    packetLen = minPacketLen;
  } else if (packetLen > MAX_PACKET_LENGTH) {
    emberAfCorePrintln("Too large: adjusting");
    packetLen = MAX_PACKET_LENGTH;
  }
  payloadLen = packetLen - headerLen;

  emberAfCorePrintln("Max packet: 127");
  emberAfCorePrintln("Header size: %d ", headerLen);
  emberAfCorePrintln("Payload size: %d ", payloadLen);
  emberAfCorePrintln("Packet size: %d ", packetLen);
}

static void getHeaderLen(void)
{
  testFrame = emberAfGetCommandApsFrame();
  uint8_t maxPayloadLen = emberAfMaximumApsPayloadLength(
    EMBER_OUTGOING_DIRECT,
    testPacketDstNode,
    testFrame) - EMBER_AF_ZCL_MANUFACTURER_SPECIFIC_OVERHEAD;
  headerLen = MAX_PACKET_LENGTH - maxPayloadLen;
}

/**
 * Set the number of packets in flight for the next test
 * (up to MAX_PACKETS_IN_FLIGHT)
 */
void emberAfPluginThroughputLibrarySetInFlightCount(void)
{
  if (isTestRunning) {
    emberAfCorePrintln("ERR: Test is running.");
    return;
  }
  inflightCount = (uint8_t)emberUnsignedCommandArgument(0);
  if (inflightCount > MAX_PACKETS_IN_FLIGHT) {
    inflightCount = MAX_PACKETS_IN_FLIGHT;
  } else if (inflightCount == 0) {
    inflightCount = 1;
  }
  emberAfCorePrintln("Packets in flight: %d", inflightCount);
}

void emberAfPluginThroughputLibrarySetApsAckOn(void)
{
  apsOptMask |= EMBER_APS_OPTION_RETRY;
  apsAckEnabled = 1;
  emberAfCorePrintln("APS ACK ON");
}

void emberAfPluginThroughputLibrarySetApsAckOff(void)
{
  apsOptMask &= ~EMBER_APS_OPTION_RETRY;
  apsAckEnabled = 0;
  emberAfCorePrintln("APS ACK OFF");
}

void emberAfPluginThroughputLibrarySetTestTimeout(void)
{
  if (isTestRunning) {
    emberAfCorePrintln("ERR: Test is running.");
    return;
  }
  testTimeout = (uint32_t)emberUnsignedCommandArgument(0);
  emberAfCorePrintln("Timeout: %d ms", testTimeout);
}

/**
 * Start a new test.
 */
static void startTest(void)
{
  if (isTestRunning) {
    return;
  }
  isTestRunning = true;
  okToSendCounter = 0;
  packetsApsAcked = 0;
  delayActive = FALSE;
  startTime = halCommonGetInt32uMillisecondTick();
}

/**
 * Start sending packets. Clears stack counters, sets packet count to 0, and then sets
 * events that trigger packets to start being sent
 */
void emberAfPluginThroughputLibraryStartTx(void)
{
  emberAfCorePrintln("");
  emberAfCorePrintln("Starting");
  #ifdef EZSP_HOST
  ezspReadAndClearCounters(emberCounters);
  #else
  emberAfPluginCountersClear();
  #endif //EZSP_HOST
  emberAfCorePrintln("Counters cleared");
  packetSendCounter = 0;
  packetSendSuccessCounter = 0;
  latencyMin = -1; // value < 0 resets latency stats
  emberAfNetworkEventControlSetActive(&emberAfPluginThroughputLibraryPacketSendEventControl);
  startTest();
}

void emberAfPluginThroughputLibraryStopTest(void)
{
  calcRunTime();
  stopTime = runTime;
  stopTx();
  activateEndOfTestDelay();
}

/**
 * Deactivates the event that triggers sending of packets
 */
static void stopTx(void)
{
  if (emberAfNetworkEventControlGetActive(&emberAfPluginThroughputLibraryPacketSendEventControl)) {
    emberAfNetworkEventControlSetInactive(&emberAfPluginThroughputLibraryPacketSendEventControl);
  }
}

/**
 * Activates the event that triggers and end of test delay
 * Needed to wait for last APS acks
 */
static void activateEndOfTestDelay(void)
{
  if (!emberAfNetworkEventControlGetActive(&emberAfPluginThroughputLibraryEndTestDelayEventControl)) {
    emberAfNetworkEventControlSetActive(&emberAfPluginThroughputLibraryEndTestDelayEventControl);
  }
}

/**
 * This function typically runs twice at the end of a test.
 * First time through it sets a delay to wait for more packets and sets delayActive = TRUE
 * Second time through it deactivates the end test Event and calls testCleanup()
 */
void emberAfPluginThroughputLibraryEndTestDelayEventHandler(void)
{
  if (!delayActive) {
    delayActive = TRUE;
    emberAfEventControlSetDelay(&emberAfPluginThroughputLibraryEndTestDelayEventControl, END_TEST_DELAY);
  } else {
    delayActive = FALSE;
    if (emberAfNetworkEventControlGetActive(&emberAfPluginThroughputLibraryEndTestDelayEventControl)) {
      emberAfNetworkEventControlSetInactive(&emberAfPluginThroughputLibraryEndTestDelayEventControl);
    }
    testCleanup();
  }
}

/**
 * Just some final cleanup
 */
static void testCleanup(void)
{
  if (isTestRunning) {
    isTestRunning = FALSE;
    emberAfCorePrintln("Test ended at %d ms", runTime);
  }
}

/**
 * During the test, this will be called at intervals to try to send a packet.
 * However, no packet will be sent until more packets in flight are needed.
 */
void emberAfPluginThroughputLibraryPacketSendEventHandler(void)
{
  if (packetSendCounter == packetSendTargetCount) {
    emberAfPluginThroughputLibraryStopTest();
    return;
  }
  emberAfEventControlSetDelay(&emberAfPluginThroughputLibraryPacketSendEventControl, txInterval);
  if (okToSend) {
    okToSend = FALSE;
    packetSend();
    packetSendCounter++;
  }
}

/**
 * Build the packet, applying option bitmasks, and send it out.
 */
static void packetSend(void)
{
  voiceData[0] = (packetSendCounter & 0xFF);
  voiceData[1] = (packetSendCounter >> 8);
  emberAfFillExternalManufacturerSpecificBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                                 | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                                 | ZCL_MANUFACTURER_SPECIFIC_MASK
                                                 | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
                                                ZCL_VOICE_OVER_ZIGBEE_CLUSTER_ID,
                                                0x1002,
                                                ZCL_VOICE_TRANSMISSION_COMMAND_ID,
                                                "b",
                                                voiceData,
                                                payloadLen);

  EmberApsFrame* frame = emberAfGetCommandApsFrame();
  frame->sourceEndpoint = testPacketSrcEndpoint;
  frame->destinationEndpoint = testPacketDstEndpoint;
  frame->options &= apsOptMask;
  frame->options &= ~EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY;
  frame->options &= ~EMBER_APS_OPTION_FORCE_ROUTE_DISCOVERY;
  #ifdef UNIX_HOST // For UNIX host, simply use the ms counter
  latencyStartTime = halCommonGetInt32uMillisecondTick();
  #else // For SoC, use micro-s counter since latency may be only a few ms
  latencyStartTime = halStackGetInt32uSymbolTick();
  #endif // UNIX_HOST
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, testPacketDstNode);
}

/**
 * Show test results: duration, packets sent, and relevant stack counters.
 */
void emberAfPluginThroughputLibraryClearCounters(void)
{
  emberAfCorePrintln("Clearing counters");
  emberAfPluginCountersClear();
}

/**
 * Get the time elapsed since the test started
 */
static void calcRunTime(void)
{
  uint32_t currentTime = halCommonGetInt32uMillisecondTick();
  if ( startTime > currentTime ) {
    runTime = (0xffffffff - (startTime - currentTime));
  } else {
    runTime = (currentTime - startTime);
  }
}

void emberAfPluginThroughputLibraryPrintResult(void)
{
  static double min;
  static double max;
  static double mean;
  static double var;
  static double std;
  static double throughput;
  static double bytesSent;

  min = max = mean = std = throughput = -1.0;
  if (packetSendCounter && runTime) { // avoid div by 0
    if (inflightCount == 1) {
      min = (double)latencyMin;
      max = (double)latencyMax;
      mean = (double)latencyMean / (double)packetSendCounter;
      // change scale back from 100us to 1us as range handled by double and to match mean scale
      var = (double)latencyStd * 100.0 * 100.0 / (double)packetSendCounter - mean * mean;
      // prevent sqrt of negative number, particularly with scaling issues and small counts
      std = sqrt(fabs(var));

      // scale from us to ms
      min /= 1000.0;
      max /= 1000.0;
      mean /= 1000.0;
      std /= 1000.0;
    }
    bytesSent = packetSendCounter * packetLen;
    throughput = 8 * bytesSent / stopTime;
  }
  if (!isTestRunning) {
    emberAfCorePrintln(" ");
    emberAfCorePrintln("TEST RESULTS ");
    emberAfCorePrintln("Test duration (ms):      %d ", stopTime);
    emberAfCorePrintln("Messages send attempts:  %d ", packetSendCounter);
    emberAfCorePrintln("Messages send successes: %d ", packetSendSuccessCounter);
    emberAfCorePrintln("Messages APS ACKed:      %d ", packetsApsAcked);
    customPrintln("Throughput (kbps):      %s ", throughput);
    #ifdef EZSP_HOST
    ezspReadCounters(emberCounters);
    #endif // EZSP_HOST
    printCounter(EMBER_COUNTER_PHY_CCA_FAIL_COUNT);
    printCounter(EMBER_COUNTER_MAC_TX_UNICAST_SUCCESS);
    printCounter(EMBER_COUNTER_MAC_TX_UNICAST_RETRY);
    printCounter(EMBER_COUNTER_MAC_TX_UNICAST_FAILED);
    printCounter(EMBER_COUNTER_APS_DATA_TX_UNICAST_SUCCESS);
    printCounter(EMBER_COUNTER_APS_DATA_TX_UNICAST_RETRY);
    printCounter(EMBER_COUNTER_APS_DATA_TX_UNICAST_FAILED);
    emberAfCorePrintln("Latency Data (Valid only with one packet in flight):");
    customPrintln("Min (ms):%s ", min);
    customPrintln("Max (ms):%s ", max);
    customPrintln("Mean(ms):%s ", mean);
    customPrintln("Std (ms):%s ", std);
  }
}

static void customPrintln(char* str, double fval)
{
  static char text[12];
  // Value is limited to 12 characters: e.g. [-9999999.99\0]
  // Conservatively limit it to 9M to eliminate rounding issues
  if (fabs(fval) < 9000000) {
    doubleToString(text, fval);
    emberAfCorePrintln(str, text);
  } else {
    emberAfCorePrintln("Warning: customPrintln value too large to print");
  }
}

/**
 * Adding double support for printing. Always uses 2 decimal places.
 */
static void doubleToString(char* str, double fval)
{
  char *sign     = (fval < 0) ? "-" : "";
  double valAbs  = fabs(fval);
  int intPart    = (int) valAbs;
  double fracTmp = valAbs - intPart;
  int fracPart   = (int) trunc(fracTmp * 100);
  sprintf(str, " %s%d.%02d", sign, intPart, fracPart);
}

/**
 * Show the name and value of a stack counter.
 */
static void printCounter(uint8_t id)
{
  emberAfCorePrintln("%p: %u ", titleStrings[id], emberCounters[id]);
}

static void markIdRx(uint16_t id)
{
  uint16_t index = id >> 3;
  uint8_t bit = 1 << (id & 0x07);
  uint8_t entry = idFlags[index];
  idFlags[index] |= bit;
  if (entry & bit) {
    duplicates++;
  }
}

static void reportRx(uint16_t id)
{
  packetsReceived++;
  if (hiPacketId < id) {
    hiPacketId = id;
  }
  if (id < (ID_FLAG_SIZE << 3)) {
    markIdRx(id);
  }
}

/**
 * Users will call this to notify the plugin that a packet has completed its
 * round trip.
 */
static void notifyApsAck(void)
{
  if (isTestRunning) {
    packetsApsAcked++;
  }
}

/** @brief Client Message Sent
 *
 * Voice over ZigBee cluster, Client Message Sent
 *
 * @param type The type of message sent  Ver.: always
 * @param indexOrDestination The destination or address to which the message was
 * sent  Ver.: always
 * @param apsFrame The APS frame for the message  Ver.: always
 * @param msgLen The length of the message  Ver.: always
 * @param message The message that was sent  Ver.: always
 * @param status The status of the sent message  Ver.: always
 */
void emberAfVoiceOverZigbeeClusterClientMessageSentCallback(EmberOutgoingMessageType type,
                                                            uint16_t indexOrDestination,
                                                            EmberApsFrame * apsFrame,
                                                            uint16_t msgLen,
                                                            uint8_t * message,
                                                            EmberStatus status)
{
  static int32_t delta;
  #ifdef UNIX_HOST // For UNIX host, simply use the ms counter
  latencyStopTime = halCommonGetInt32uMillisecondTick();
  #else // For SoC, use micro-s counter since latency may be only a few ms
  latencyStopTime = halStackGetInt32uSymbolTick();
  #endif // UNIX_HOST
  if (msgLen >= EMBER_AF_ZCL_MANUFACTURER_SPECIFIC_OVERHEAD) {
    if (message[0] & ZCL_CLUSTER_SPECIFIC_COMMAND) {
      if (message[4] & ZCL_VOICE_TRANSMISSION_COMMAND_ID) {
        notifyApsAck();
        if (status == EMBER_SUCCESS) {
          packetSendSuccessCounter++;
        }

        if (inflightCount == 1) { //Only calculate latency for 1 in flight
          if (latencyStartTime > latencyStopTime) {
            #ifdef UNIX_HOST // HOST: ms counter
            delta = (int32_t)(1000000000 - (latencyStartTime - latencyStopTime));
            #else // SoC: us counter
            delta = (int32_t)(0xffffffff - (latencyStartTime - latencyStopTime));
            #endif
          } else {
            delta = (int32_t)(latencyStopTime - latencyStartTime);
          }
          #ifdef UNIX_HOST // convert ms delta to us delta
          delta *= 1000;
          #endif //UNIX_HOST

          if (latencyMin < 0) { // first time seed?
            latencyMin = delta;
            latencyMax = delta;
            latencyMean = delta;
            // change scale from 1us to 100us to decrease "count"*delta^2 range for std
            latencyStd = delta / 100 * delta / 100;
          } else {              // if not, update stats
            if (delta < latencyMin) {
              latencyMin = delta;
            }
            if (delta > latencyMax) {
              latencyMax = delta;
            }
            latencyMean += delta;
            // change scale from 1us to 100us to decrease "count"*delta^2 range for std
            latencyStd += delta / 100 * delta / 100;
          }
        } else { // inflight > 1, do not calculate latency
          latencyMin = latencyMax = latencyMean = latencyStd = -1;
        }
      }
    }
  }
}

/** @brief Voice Transmission
 *
 *
 */
boolean emberAfVoiceOverZigbeeClusterVoiceTransmissionCallback(uint8_t* voiceData)
{
  static uint16_t id = 0;
  id += voiceData[0];
  id += (voiceData[1] << 8);
  reportRx(id);

  return TRUE;
}

void emberAfMainTickCallback(void)
{
  //Is the test running?
  if (isTestRunning) {
    calcRunTime();
    if (runTime >= testTimeout) {
      emberAfPluginThroughputLibraryStopTest();
      return;
    }
    if ((okToSendCounter - packetsApsAcked) < inflightCount) {
      okToSend = TRUE;
      // packetSendCounter is the number actually transmitted
      okToSendCounter++;
    }
  }
}
