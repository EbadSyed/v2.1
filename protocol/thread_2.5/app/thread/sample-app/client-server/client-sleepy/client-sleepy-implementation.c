// Copyright 2015 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_HAL
#include EMBER_AF_API_COMMAND_INTERPRETER2
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#ifdef CORTEXM3_EFM32_MICRO
  #include "tempdrv.h"
#endif

#define ALIAS(x) x##Alias
#include "app/thread/plugin/udp-debug/udp-debug.c"

// WARNING: Random join keys MUST be used for real devices, with the keys
// typically programmed during product manufacturing and printed on the devices
// themselves for access by the user.  This application will read the join key
// from a manufacturing token.  For ease of use, this application will generate
// a random join key at runtime if the manufacturing token was not set
// previously.  This will then be saved in a manufacturing token.  Join keys
// SHOULD NOT be generated at runtime.  They should be produced at
// manufacturing time.

// The client/server sample applications use a fixed network id to simplify
// the join process.
static const uint8_t networkId[EMBER_NETWORK_ID_SIZE] = "client/server";

#define SLEEP_BUTTON BUTTON0
#define JOIN_BUTTON BUTTON1
#define JOIN_KEY_SIZE 8

static void resumeNetwork(void);
static void joinNetwork(void);
static void getJoinKey(uint8_t *joinKey, uint8_t *joinKeyLength);
static void createRandomJoinKey(uint8_t *joinKey, uint8_t *joinKeyLength);
static void attachToNetwork(void);
static void waitForServerAdvertisement(void);
static void attachToServer(const EmberIpv6Address *newServer);
static void reportDataToServer(void);
static int32_t getTemp_mC(void);
static void detachFromServer(void);
static void resetNetworkState(void);

static EmberIpv6Address server;
static uint8_t failedReports;
#define REPORT_FAILURE_LIMIT 3
#define WAIT_PERIOD_MS   (30 * MILLISECOND_TICKS_PER_SECOND)
#define REPORT_PERIOD_MS (10 * MILLISECOND_TICKS_PER_SECOND)

static const uint8_t clientReportUri[] = "client/report";

static bool okToSleep = true;

enum {
  INITIAL                       = 0,
  RESUME_NETWORK                = 1,
  JOIN_NETWORK                  = 2,
  ATTACH_TO_NETWORK             = 3,
  WAIT_FOR_SERVER_ADVERTISEMENT = 4,
  REPORT_DATA_TO_SERVER         = 5,
  WAIT_FOR_DATA_CONFIRMATION    = 6,
  RESET_NETWORK_STATE           = 7,
};
static uint8_t state = INITIAL;
EmberEventControl stateEventControl;
static void setNextStateWithDelay(uint8_t nextState, uint32_t delayMs);
#define setNextState(nextState)       setNextStateWithDelay((nextState), 0)
#define repeatState()                 setNextStateWithDelay(state, 0)
#define repeatStateWithDelay(delayMs) setNextStateWithDelay(state, (delayMs))

static const uint8_t *printableNetworkId(void);

void emberAfNetworkStatusCallback(EmberNetworkStatus newNetworkStatus,
                                  EmberNetworkStatus oldNetworkStatus,
                                  EmberJoinFailureReason reason)
{
  // This callback is called whenever the network status changes, like when
  // we finish joining to a network or when we lose connectivity.  If we have
  // no network, we try joining to one.  If we have a saved network, we try to
  // resume operations on that network.  When we are joined and attached to the
  // network, we wait for an advertisement and then begin reporting.

  emberEventControlSetInactive(stateEventControl);

  switch (newNetworkStatus) {
    case EMBER_NO_NETWORK:
      if (oldNetworkStatus == EMBER_JOINING_NETWORK) {
        emberAfCorePrintln("ERR: Joining failed: 0x%x", reason);
      }
      break;
    case EMBER_SAVED_NETWORK:
      setNextState(RESUME_NETWORK);
      break;
    case EMBER_JOINING_NETWORK:
      // Wait for either the "attaching" or "no network" state.
      break;
    case EMBER_JOINED_NETWORK_ATTACHING:
      // Wait for either the "attached" or "no parent" state.
      if (oldNetworkStatus == EMBER_JOINED_NETWORK_ATTACHED) {
        emberAfCorePrintln("Trying to re-connect...");
      }
      break;
    case EMBER_JOINED_NETWORK_ATTACHED:
      emberAfCorePrintln("%s network \"%s\"",
                         (state == RESUME_NETWORK
                          ? "Resumed operation on"
                          : (state == JOIN_NETWORK
                             ? "Joined"
                             : "Rejoined")),
                         printableNetworkId());
      // TODO: For a brief interruption in connectivity, the client could attempt
      // to continue reporting to its previous server, rather than wait for a new
      // server.
      setNextState(WAIT_FOR_SERVER_ADVERTISEMENT);
      break;
    case EMBER_JOINED_NETWORK_NO_PARENT:
      if (state == ATTACH_TO_NETWORK) {
        emberAfCorePrintln("ERR: Rejoining failed");
      } else {
        emberAfCorePrintln("ERR: No connection to network");
      }
      setNextState(ATTACH_TO_NETWORK);
      break;
    default:
      assert(false);
      break;
  }
}

static void resumeNetwork(void)
{
  assert(state == RESUME_NETWORK);

  emberAfCorePrintln("Resuming operation on network \"%s\"", printableNetworkId());

  emberResumeNetwork();
}

void emberResumeNetworkReturn(EmberStatus status)
{
  // This return indicates whether the stack is going to attempt to resume.  If
  // so, the result is reported later as a network status change.  If we cannot
  // even attempt to resume, we just give up and try joining instead.

  assert(state == RESUME_NETWORK);

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: Unable to resume: 0x%x", status);
    setNextState(JOIN_NETWORK);
  }
}

static void joinNetwork(void)
{
  // When joining a network, we look for one specifically with our network id.
  // The commissioner must have our join key for this to succeed.

  EmberNetworkParameters parameters = { { 0 } };
  uint8_t joinKey[EMBER_JOIN_KEY_MAX_SIZE + 1] = { 0 };

  assert(state == JOIN_NETWORK);

  MEMCOPY(parameters.networkId, networkId, sizeof(networkId));
  parameters.nodeType = EMBER_SLEEPY_END_DEVICE;
  parameters.radioTxPower = 3;
  getJoinKey(joinKey, &parameters.joinKeyLength);
  MEMCOPY(parameters.joinKey, joinKey, parameters.joinKeyLength);

  emberAfCorePrint("Joining network \"%s\" with EUI64 ", networkId);
  emberAfCoreDebugExec(emberAfPrintBigEndianEui64(emberEui64()));
  emberAfCorePrintln(" and join key \"%s\"", joinKey);

  emberJoinNetwork(&parameters,
                   (EMBER_NETWORK_ID_OPTION
                    | EMBER_NODE_TYPE_OPTION
                    | EMBER_TX_POWER_OPTION
                    | EMBER_JOIN_KEY_OPTION),
                   EMBER_ALL_802_15_4_CHANNELS_MASK);
}

// join
void joinCommand(void)
{
  // If we are not in a join state, we will attempt to join a network.
  // This is functionally the same as pushing the JOIN_BUTTON.

  if (emberNetworkStatus() == EMBER_NO_NETWORK) {
    setNextState(JOIN_NETWORK);
  }
}

static void getJoinKey(uint8_t *joinKey, uint8_t *joinKeyLength)
{
  tokTypeMfgThreadJoinKey token;
  halCommonGetMfgToken(&token, TOKEN_MFG_THREAD_JOIN_KEY);
  if (token.joinKeyLength == 0xFFFF) {
    createRandomJoinKey(joinKey, joinKeyLength);
    emberAfCorePrintln("WARNING: Join key not set");
    token.joinKeyLength = *joinKeyLength;
    MEMCOPY(token.joinKey, joinKey, token.joinKeyLength);
    halCommonSetMfgToken(TOKEN_MFG_THREAD_JOIN_KEY, &token);
  } else {
    *joinKeyLength = token.joinKeyLength;
    MEMCOPY(joinKey, token.joinKey, *joinKeyLength);
    joinKey[*joinKeyLength] = '\0';
  }
}

static void createRandomJoinKey(uint8_t *joinKey, uint8_t *joinKeyLength)
{
  // If the manufacturing token containing the join key was not set, a random
  // join key is generated.  The Thread specification disallows the characters
  // I, O, Q, and Z, for readability.

  const char characters[] = "ABCDEFGHJKLMNPRSTUVWXY1234567890";

  for (*joinKeyLength = 0; *joinKeyLength < JOIN_KEY_SIZE; (*joinKeyLength)++ ) {
    uint16_t key = halCommonGetRandom() % (int) (sizeof(characters) - 1);
    joinKey[*joinKeyLength] = characters[key];
  }
  joinKey[*joinKeyLength] = '\0';
}

// get-join-key
void getJoinKeyCommand(void)
{
  // This function gets the join key and then prints it.  If the join key has
  // not been created yet, it will be created in the getJoinKey function and
  // then printed.

  uint8_t joinKey[EMBER_JOIN_KEY_MAX_SIZE + 1];
  uint8_t joinKeyLength;

  getJoinKey(joinKey, &joinKeyLength);
  emberAfCorePrintln("Join key: \"%s\"", joinKey);
}

void emberJoinNetworkReturn(EmberStatus status)
{
  // This return indicates whether the stack is going to attempt to join.  If
  // so, the result is reported later as a network status change.  Otherwise,
  // we just try again.

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: Unable to join: 0x%x", status);
    repeatState();
  }
}

static void attachToNetwork(void)
{
  assert(state == ATTACH_TO_NETWORK);

  emberAfCorePrintln("Rejoining network \"%s\"", printableNetworkId());

  emberAttachToNetwork();
}

void emberAttachToNetworkReturn(EmberStatus status)
{
  // This return indicates whether the stack is going to attempt to attach.  If
  // so, the result is reported later as a network status change.  If we cannot
  // even attempt to attach, we just give up and reset our network state, which
  // will trigger a fresh join attempt.

  assert(state == ATTACH_TO_NETWORK);

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: Unable to rejoin: 0x%x", status);
    setNextState(RESET_NETWORK_STATE);
  }
}

static void waitForServerAdvertisement(void)
{
  // Once on the network, we wait for a server to advertise itself.  We
  // periodically print a message while waiting to prove we are alive.

  assert(state == WAIT_FOR_SERVER_ADVERTISEMENT);

  emberAfCorePrintln("Waiting for an advertisement from a server");

  repeatStateWithDelay(WAIT_PERIOD_MS);
}

void serverAdvertiseHandler(EmberCoapCode code,
                            uint8_t *uri,
                            EmberCoapReadOptions *options,
                            const uint8_t *payload,
                            uint16_t payloadLength,
                            const EmberCoapRequestInfo *info)
{
  // Advertisements from servers are sent as CoAP POST requests to the
  // "server/advertise" URI.  When we receive an advertisement, we attach to
  // the that sent it and beginning sending reports.

  EmberCoapCode responseCode;

  if (state != WAIT_FOR_SERVER_ADVERTISEMENT) {
    responseCode = EMBER_COAP_CODE_503_SERVICE_UNAVAILABLE;
  } else {
    attachToServer(&info->remoteAddress);
    responseCode = EMBER_COAP_CODE_204_CHANGED;
  }

  if (emberCoapIsSuccessResponse(responseCode)
      || info->localAddress.bytes[0] != 0xFF) { // not multicast
    emberCoapRespondWithCode(info, responseCode);
  }
}

static void attachToServer(const EmberIpv6Address *newServer)
{
  // We attach to a server in response to an advertisement (or a CLI command).
  // Once we have a server, we begin reporting periodically.  We start from a
  // clean state with regard to failed reports.

  assert(state == WAIT_FOR_SERVER_ADVERTISEMENT);

  MEMCOPY(&server, newServer, sizeof(EmberIpv6Address));
  failedReports = 0;

  emberAfCorePrint("Attached to server at ");
  emberAfCoreDebugExec(emberAfPrintIpv6Address(newServer));
  emberAfCorePrintln("");

  setNextState(REPORT_DATA_TO_SERVER);
}

// attach <server>
void attachCommand(void)
{
  // If we are waiting for a server, we can manually attach to one using a CLI
  // command.

  if (state == WAIT_FOR_SERVER_ADVERTISEMENT) {
    EmberIpv6Address newServer = { { 0 } };
    if (emberGetIpv6AddressArgument(0, &newServer)) {
      attachToServer(&newServer);
    } else {
      emberAfCorePrintln("ERR: Invalid ip");
    }
  }
}

static void processServerDataAck(EmberCoapStatus status,
                                 EmberCoapCode code,
                                 EmberCoapReadOptions *options,
                                 uint8_t *payload,
                                 uint16_t payloadLength,
                                 EmberCoapResponseInfo *info)
{
  // We track the success or failure of reports so that we can determine when
  // we have lost the server.  A series of consecutive failures is the trigger
  // to detach from the current server and find a new one.  Any successfully-
  // transmitted report clears past failures.

  if (state == WAIT_FOR_DATA_CONFIRMATION) {
    if (status == EMBER_COAP_MESSAGE_ACKED
        || status == EMBER_COAP_MESSAGE_RESPONSE) {
      failedReports = 0;
    } else {
      failedReports++;
      emberAfCorePrintln("ERR: Report timed out - failure %u of %u",
                         failedReports,
                         REPORT_FAILURE_LIMIT);
    }
    if (failedReports < REPORT_FAILURE_LIMIT) {
      setNextStateWithDelay(REPORT_DATA_TO_SERVER, REPORT_PERIOD_MS);
    } else {
      detachFromServer();
    }
  }
}

static void reportDataToServer(void)
{
  // We peridocally send data to the server.  The data is the temperature,
  // measured in 10^-3 degrees Celsius.  The success or failure of the reports
  // is tracked so we can determine if the server has disappeared and we should
  // find a new one.

  EmberCoapSendInfo info = { 0 }; // use defaults
  EmberStatus status;
  int32_t data;

  assert(state == REPORT_DATA_TO_SERVER);

  data = getTemp_mC();

  emberAfCorePrint("Reporting %ld to server at ", data);
  emberAfCoreDebugExec(emberAfPrintIpv6Address(&server));
  emberAfCorePrintln("");

  // Convert from host-order to network-order before sending so the data can be
  // reliably reconstructed by the server.
  data = HTONL(data);

  status = emberCoapPost(&server,
                         clientReportUri,
                         (const uint8_t *)&data,
                         sizeof(data),
                         processServerDataAck,
                         &info);
  if (status == EMBER_SUCCESS) {
    setNextState(WAIT_FOR_DATA_CONFIRMATION);
  } else {
    emberAfCorePrintln("ERR: Reporting failed: 0x%x", status);
    repeatStateWithDelay(REPORT_PERIOD_MS);
  }
}

#ifdef CORTEXM3_EFM32_MICRO
static int32_t getTemp_mC(void)
{
  return TEMPDRV_GetTemp() * 1000;
}
#else
static int32_t getTemp_mC(void)
{
  uint16_t value;
  int16_t volts;
  halStartAdcConversion(ADC_USER_APP,
                        ADC_REF_INT,
                        TEMP_SENSOR_ADC_CHANNEL,
                        ADC_CONVERSION_TIME_US_256);
  halReadAdcBlocking(ADC_USER_APP, &value);
  volts = halConvertValueToVolts(value / TEMP_SENSOR_SCALE_FACTOR);
  return (1591887L - (171 * (int32_t)volts)) / 10;
}
#endif

// report
void reportCommand(void)
{
  // If we have a server and are reporting data, we can manually send a new
  // report using a CLI command.

  if (state == REPORT_DATA_TO_SERVER) {
    reportDataToServer();
  }
}

static void detachFromServer(void)
{
  // We detach from a server in response to failed reports (or a CLI command).
  // Once we detach, we wait for a new server to advertise itself.

  assert(state == REPORT_DATA_TO_SERVER
         || state == WAIT_FOR_DATA_CONFIRMATION);

  emberAfCorePrint("Detached from server at ");
  emberAfCoreDebugExec(emberAfPrintIpv6Address(&server));
  emberAfCorePrintln("");

  setNextState(WAIT_FOR_SERVER_ADVERTISEMENT);
}

// detach
void detachCommand(void)
{
  // If we have a server and are reporting data, we can manually detach and try
  // to find a new server by using a CLI command.

  if (state == REPORT_DATA_TO_SERVER
      || state == WAIT_FOR_DATA_CONFIRMATION) {
    detachFromServer();
  }
}

static void resetNetworkState(void)
{
  emberAfCorePrintln("Resetting network state");
  emberResetNetworkState();
}

void emberResetNetworkStateReturn(EmberStatus status)
{
  // If we ever leave the network, we go right back to joining again.  This
  // could be triggered by an external CLI command.

  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("Reset network state");
  }
}

void emberUdpHandler(const uint8_t *destination,
                     const uint8_t *source,
                     uint16_t localPort,
                     uint16_t remotePort,
                     const uint8_t *payload,
                     uint16_t payloadLength)
{
  ALIAS(emberUdpHandler)(destination,
                         source,
                         localPort,
                         remotePort,
                         payload,
                         payloadLength);
}

bool emberAfPluginIdleSleepOkToSleepCallback(uint32_t durationMs)
{
  // The application normally permits itself to go to sleep whenever possible,
  // but the buttons can be used to keep the device awake.

  return okToSleep;
}

void halButtonIsr(uint8_t button, uint8_t buttonState)
{
  // The CLI is unusable during sleep, which makes it difficult to interact
  // with the node.  To address this, the buttons can be used to alter the
  // sleep characteristics of the node.  Pressing SLEEP_BUTTON will toggle
  // between sleeping when possible and staying awake.  Pressing JOIN_BUTTON
  // will cause the node to try to join a network.

  if (buttonState == BUTTON_PRESSED) {
    if (button ==  SLEEP_BUTTON) {
      okToSleep = !okToSleep;
    } else if (button == JOIN_BUTTON) {
      if (emberNetworkStatus() == EMBER_NO_NETWORK) {
        setNextState(JOIN_NETWORK);
      }
    }
  }
}

bool emberAfPluginPollingOkToLongPollCallback(void)
{
  // While waiting for a server advertisement or for the server to confirm our
  // data report, we need to short poll, so that we can actually receive the
  // messages via our parent.  At all other times, we can long poll.

  return (state != WAIT_FOR_SERVER_ADVERTISEMENT
          && state != WAIT_FOR_DATA_CONFIRMATION);
}

void stateEventHandler(void)
{
  emberEventControlSetInactive(stateEventControl);

  switch (state) {
    case RESUME_NETWORK:
      resumeNetwork();
      break;
    case JOIN_NETWORK:
      joinNetwork();
      break;
    case ATTACH_TO_NETWORK:
      attachToNetwork();
      break;
    case WAIT_FOR_SERVER_ADVERTISEMENT:
      waitForServerAdvertisement();
      break;
    case REPORT_DATA_TO_SERVER:
      reportDataToServer();
      break;
    case WAIT_FOR_DATA_CONFIRMATION:
      break;
    case RESET_NETWORK_STATE:
      resetNetworkState();
      break;
    default:
      assert(false);
      break;
  }
}

static void setNextStateWithDelay(uint8_t nextState, uint32_t delayMs)
{
  state = nextState;
  emberEventControlSetDelayMS(stateEventControl, delayMs);
}

static const uint8_t *printableNetworkId(void)
{
  EmberNetworkParameters parameters = { { 0 } };
  static uint8_t networkId[EMBER_NETWORK_ID_SIZE + 1] = { 0 };
  emberGetNetworkParameters(&parameters);
  MEMCOPY(networkId, parameters.networkId, EMBER_NETWORK_ID_SIZE);
  return networkId;
}
