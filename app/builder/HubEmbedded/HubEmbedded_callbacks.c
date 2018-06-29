// Copyright 2016 Silicon Laboratories, Inc.                                *80*

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include "af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/util.h"
#include "app/framework/plugin/concentrator/source-route-common.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "stack/include/trust-center.h"
#include <stdlib.h>

// the number of tokens that can be written using ezspSetToken and read
// using ezspGetToken
#define MFGSAMP_NUM_EZSP_TOKENS 8
// the size of the tokens that can be written using ezspSetToken and
// read using ezspGetToken
#define MFGSAMP_EZSP_TOKEN_SIZE 8
// the number of manufacturing tokens
#define MFGSAMP_NUM_EZSP_MFG_TOKENS 11
// the size of the largest EZSP Mfg token, EZSP_MFG_CBKE_DATA
// please refer to app/util/ezsp/ezsp-enum.h
#define MFGSAMP_EZSP_TOKEN_MFG_MAXSIZE 92

EmberStatus emberAfTrustCenterStartNetworkKeyUpdate(void);

// Forward declarations of custom cli command functions
static void printSourceRouteTable(void);
static void mfgappTokenDump(void);
static void changeNwkKeyCommand(void);
static void printNextKeyCommand(void);
static void versionCommand(void);
static void setTxPowerCommand(void);
void send_message(void);
void set_temp(void);

EmberCommandEntry emberAfCustomCommands[] = {
  emberCommandEntryAction("print_srt", printSourceRouteTable, "", ""),
  emberCommandEntryAction("tokdump", mfgappTokenDump, "", ""),
  emberCommandEntryAction("changeNwkKey", changeNwkKeyCommand, "", ""),
  emberCommandEntryAction("printNextKey", printNextKeyCommand, "", ""),
  emberCommandEntryAction("version", versionCommand, "", ""),
  emberCommandEntryAction("txPower", setTxPowerCommand, "s", ""),
  emberCommandEntryAction("sendkey", send_message,"vuu",""),
  emberCommandEntryAction("settemp", set_temp,"vuu",""),
  emberCommandEntryTerminator()
};

//send unicast message to node
void send_message(void)
{
	EmberApsFrame customApsFrame;
	customApsFrame.profileId = 0x0104;
	customApsFrame.clusterId = 0x0999;
	customApsFrame.sourceEndpoint = emberAfEndpointFromIndex(0);
	customApsFrame.destinationEndpoint = 0x01;
	customApsFrame.options = EMBER_APS_OPTION_RETRY|\
							 EMBER_APS_OPTION_SOURCE_EUI64|\
							 EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY|\
							 EMBER_APS_OPTION_DESTINATION_EUI64;

//arguments passed input
	int16u destination = (int16u)emberSignedCommandArgument(0);
	int8u p1 = (int8u)emberSignedCommandArgument(1);
	int8u p2 = (int8u)emberSignedCommandArgument(2);
	//int8u p3 = (int8u)emberSignedCommandArgument(3);
	//int8u p4 = (int8u)emberSignedCommandArgument(4);

	//int16u destination = 0x675F;
	int8u testBuffer[7] = {0x00,0x00,0xFF,0x00,0x00,p1,p2};
	// 0 = framCounter
	// 1 = seq
	// 2 =command id
	testBuffer[1] = emberAfNextSequence();

	emberAfSendUnicast(EMBER_OUTGOING_DIRECT,
	                                destination,
	                                &customApsFrame,
	                                sizeof(testBuffer),
									testBuffer);

}

//send unicast message to node
void set_temp(void)
{
	EmberApsFrame customApsFrame;
	customApsFrame.profileId = 0x0104;
	customApsFrame.clusterId = 0x7388;
	customApsFrame.sourceEndpoint = emberAfEndpointFromIndex(0);
	customApsFrame.destinationEndpoint = 0x01;
	customApsFrame.options = EMBER_APS_OPTION_RETRY|\
							 EMBER_APS_OPTION_SOURCE_EUI64|\
							 EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY|\
							 EMBER_APS_OPTION_DESTINATION_EUI64;

//arguments passed input
	int16u destination = (int16u)emberSignedCommandArgument(0);
	int8u set_temperature_u = (int8u)emberSignedCommandArgument(1);
	int8u set_temperature_l = (int8u)emberSignedCommandArgument(2);
	//int8u p3 = (int8u)emberSignedCommandArgument(3);
	//int8u p4 = (int8u)emberSignedCommandArgument(4);

	//int16u destination = 0x675F;
	int8u testBuffer[7] = {0x00,0x00,0xFF,0x11,0x14,set_temperature_u,set_temperature_l};
	// 0 = framCounter
	// 1 = seq
	// 2 =command id
	testBuffer[1] = emberAfNextSequence();

	emberAfSendUnicast(EMBER_OUTGOING_DIRECT,
	                                destination,
	                                &customApsFrame,
	                                sizeof(testBuffer),
									testBuffer);

}



//// ******* test of token dump code

// the manufacturing tokens are enumerated in app/util/ezsp/ezsp-protocol.h
// the names are enumerated here to make it easier for the user
PGM_NO_CONST PGM_P ezspMfgTokenNames[] =
{
  "EZSP_MFG_CUSTOM_VERSION...",
  "EZSP_MFG_STRING...........",
  "EZSP_MFG_BOARD_NAME.......",
  "EZSP_MFG_MANUF_ID.........",
  "EZSP_MFG_PHY_CONFIG.......",
  "EZSP_MFG_BOOTLOAD_AES_KEY.",
  "EZSP_MFG_ASH_CONFIG.......",
  "EZSP_MFG_EZSP_STORAGE.....",
  "EZSP_STACK_CAL_DATA.......",
  "EZSP_MFG_CBKE_DATA........",
  "EZSP_MFG_INSTALLATION_CODE"
};

// IAS ACE Server side callbacks
bool emberAfIasAceClusterArmCallback(uint8_t armMode,
                                     uint8_t* armDisarmCode,
                                     uint8_t zoneId)
{
  uint16_t armDisarmCodeLength = emberAfStringLength(armDisarmCode);
  EmberNodeId sender = emberGetSender();
  uint16_t i;

  emberAfAppPrint("IAS ACE Arm Received %x", armMode);

  // Start i at 1 to skip over leading character in the byte array as it is the
  // length byte
  for (i = 1; i < armDisarmCodeLength; i++) {
    emberAfAppPrint("%c", armDisarmCode[i]);
  }
  emberAfAppPrintln(" %x", zoneId);

  emberAfFillCommandIasAceClusterArmResponse(armMode);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, sender);

  return true;
}

bool emberAfIasAceClusterBypassCallback(uint8_t numberOfZones,
                                        uint8_t* zoneIds,
                                        uint8_t* armDisarmCode)
{
  EmberNodeId sender = emberGetSender();
  uint8_t i;

  emberAfAppPrint("IAS ACE Cluster Bypass for zones ");

  for (i = 0; i < numberOfZones; i++) {
    emberAfAppPrint("%d ", zoneIds[i]);
  }
  emberAfAppPrintln("");

  emberAfFillCommandIasAceClusterBypassResponse(numberOfZones,
                                                zoneIds,
                                                numberOfZones);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, sender);

  return true;
}

// code to print out the source route table
static void printSourceRouteTable(void)
{
  uint8_t i;
  for (i = 0; i < sourceRouteTableSize; i++) {
    if (sourceRouteTable[i].destination != 0x0000) {
      emberAfCorePrintln("[ind]%x[dest]%2x[closer]%x[older]%x",
                         i,
                         sourceRouteTable[i].destination,
                         sourceRouteTable[i].closerIndex,
                         sourceRouteTable[i].olderIndex);
    }
    emberSerialWaitSend(APP_SERIAL);
  }
  emberAfCorePrintln("<print srt>");
  emberSerialWaitSend(APP_SERIAL);
}

// Called to dump all of the tokens. This dumps the indices, the names,
// and the values using ezspGetToken and ezspGetMfgToken. The indices
// are used for read and write functions below.
static void mfgappTokenDump(void)
{
  EmberStatus status;
  uint8_t tokenData[MFGSAMP_EZSP_TOKEN_MFG_MAXSIZE];
  uint8_t index, i, tokenLength;

  // first go through the tokens accessed using ezspGetToken
  emberAfCorePrintln("(data shown little endian)");
  emberAfCorePrintln("Tokens:");
  emberAfCorePrintln("idx  value:");
  for (index = 0; index < MFGSAMP_NUM_EZSP_TOKENS; index++) {
    // get the token data here
    status = ezspGetToken(index, tokenData);
    emberAfCorePrint("[%d]", index);
    if (status == EMBER_SUCCESS) {
      // Print out the token data
      for (i = 0; i < MFGSAMP_EZSP_TOKEN_SIZE; i++) {
        emberAfCorePrint(" %X", tokenData[i]);
      }

      emberSerialWaitSend(APP_SERIAL);
      emberAfCorePrintln("");
    } else {
      // handle when ezspGetToken returns an error
      emberAfCorePrintln(" ... error 0x%x ...",
                         status);
    }
  }

  // now go through the tokens accessed using ezspGetMfgToken
  // the manufacturing tokens are enumerated in app/util/ezsp/ezsp-protocol.h
  // this file contains an array (ezspMfgTokenNames) representing the names.
  emberAfCorePrintln("Manufacturing Tokens:");
  emberAfCorePrintln("idx  token name                 len   value");
  for (index = 0; index < MFGSAMP_NUM_EZSP_MFG_TOKENS; index++) {
    // ezspGetMfgToken returns a length, be careful to only access
    // valid token indices.
    tokenLength = ezspGetMfgToken(index, tokenData);
    emberAfCorePrintln("[%x] %p: 0x%x:", index,
                       ezspMfgTokenNames[index], tokenLength);

    // Print out the token data
    for (i = 0; i < tokenLength; i++) {
      if ((i != 0) && ((i % 8) == 0)) {
        emberAfCorePrintln("");
        emberAfCorePrint("                                    :");
        emberSerialWaitSend(APP_SERIAL);
      }
      emberAfCorePrint(" %X", tokenData[i]);
    }
    emberSerialWaitSend(APP_SERIAL);
    emberAfCorePrintln("");
  }
  emberAfCorePrintln("");
}

static void changeNwkKeyCommand(void)
{
  EmberStatus status = emberAfTrustCenterStartNetworkKeyUpdate();

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Change Key Error %x", status);
  } else {
    emberAfCorePrintln("Change Key Success");
  }
}

static void dcPrintKey(uint8_t label, uint8_t *key)
{
  uint8_t i;
  emberAfCorePrintln("key %x: ", label);
  for (i = 0; i < EMBER_ENCRYPTION_KEY_SIZE; i++) {
    emberAfCorePrint("%x", key[i]);
  }
  emberAfCorePrintln("");
}

static void printNextKeyCommand(void)
{
  EmberKeyStruct nextNwkKey;
  EmberStatus status;

  status = emberGetKey(EMBER_NEXT_NETWORK_KEY,
                       &nextNwkKey);

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("Error getting key");
  } else {
    dcPrintKey(1, nextNwkKey.key.contents);
  }
}

static void versionCommand(void)
{
  emberAfCorePrintln("Version:  0.1 Alpha");
  emberAfCorePrintln(" %s", __DATE__);
  emberAfCorePrintln(" %s", __TIME__);
  emberAfCorePrintln("");
}

static void setTxPowerCommand(void)
{
  int8_t dBm = (int8_t)emberSignedCommandArgument(0);

  emberSetRadioPower(dBm);
}
