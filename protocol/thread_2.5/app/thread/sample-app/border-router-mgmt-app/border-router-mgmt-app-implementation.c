// Copyright 2017 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_HAL
#include EMBER_AF_API_COMMAND_INTERPRETER2
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif

// Required for 'getline'
#define _GNU_SOURCE

#include <arpa/inet.h>
#include <ctype.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <linux/if.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// Used by hook script processors
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ALIAS(x) x##Alias

// Preprocessor convert var to "var"
#define STRINGIFY(var) #var
// Preprocessor get number of digits in var minus null
#define LENGTHOF(var) (sizeof(STRINGIFY(var)) - 1)

#include "app/thread/plugin/address-configuration-debug/address-configuration-debug.c"
#include "stack/ip/ip-address.h"

// This flag is used to control whether or not a border-router-mgmt-app will
// auto-form a network if there is no network. This is set to true by default
// to form a network out of the box. This flag can be set to with the CLI
// set-auto-form <bool>. Setting this to false will allow for the Border Router
// to join a precommissioned network out-of-band using the network-management
// CLI commands, since they need to be used in the no-network state.
static bool autoFormNetwork = true;

// This variable is set based off of the USE_PREFIX_DELEGATION flag in the
// configuration file. It will be used in future multi-border-router support to
// allow sub-border-routers to obtain an address via prefix delegation or
// through coordinating with a leader through the mesh
static bool usePrefixDelegation = false;

#define MAX_REJOIN_ATTEMPTS 20
static uint8_t joinKey[EMBER_JOIN_KEY_MAX_SIZE + 1] = { 0 };
static uint8_t joinKeyLength = 0;
static uint8_t joinAttemptsRemaining = 0;

#define DEFAULT_GLOBAL_NETWORK_PREFIX { 0x20, 0x01, 0x0D, 0xB8, 0x03, 0x85, 0x93, 0x18 }
#define DEFAULT_GLOBAL_NETWORK_PREFIX_WIDTH 64
#define DEFAULT_ULA_PREFIX { 0xfd, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }

// The default networkId for this mesh network
// WARNING: networkId is 16 + 1 bytes wide; Do not overflow!
#define DEFAULT_NETWORK_ID "border-router"
static uint8_t networkId[EMBER_NETWORK_ID_SIZE + 1] = DEFAULT_NETWORK_ID;

// Network management data for global address
// This /64 prefix will be handed to the mesh to assign ipv6 addresses from
//
// WARNING: the 2001:DB8::/32 prefix is specified as the 'Documentation Only'
// prefix and is not suitable for globally-routed traffic use a different
// prefix for a production device! Border router is using
// 2001:0db8:0385:9318::/64 as a default mesh subnet.
static EmberIpv6Address globalNetworkPrefix = {
  DEFAULT_GLOBAL_NETWORK_PREFIX,
};

// The default prefix above is 64 bits wide
static uint8_t globalNetworkPrefixBitWidth = DEFAULT_GLOBAL_NETWORK_PREFIX_WIDTH;

// MAX_PREFIX in IPv6 is 128 bits
static EmberIpv6Address ulaPrefix = {
  DEFAULT_ULA_PREFIX,
};

// An optional configuration file that will be parsed to get default network
// settings
#define CONFIGURATION_FILE "/etc/siliconlabs/border-router.conf"
static FILE *inputFp = NULL;

// Maximum width in chars of any conf file entry
#define MAX_CONF_KEY_LENGTH 32
// Maximum width in chars of any conf file line
#define MAX_CONF_LINE_LENGTH (MAX_CONF_KEY_LENGTH + 128)
// Maximum format width of any confile entry
#define MAX_CONF_FORMAT_LENGTH 32

// The version number of the configuration file
static uint8_t confFileVersion = 0;

// An enumeration to identify valid keys in the conf file. When support for new
// conf is added, update this enum and the corresponding 'ConfKey' below
typedef enum ConfKeyEnum {
  FILE_VERSION_KEY = 0,
  AUTO_FORM_NETWORK_KEY,
  USE_PREFIX_DELEGATION_KEY,
  NETWORK_ID_KEY,
  IPV6_SUBNET_KEY,

  // WARNING: this must always appear at the end of the key enum. When new keys
  // are added, you must update validConfEntries, and
  // all switch statements acting on ConfKey
  MAX_CONF_KEY,
  UNKNOWN_KEY,
} ConfKey;

typedef struct ConfEntryStruct {
  char key[MAX_CONF_KEY_LENGTH];
  char format[MAX_CONF_FORMAT_LENGTH];
  uint8_t sizeInBytes;
  void *addr;
} ConfEntry;

static  ConfEntry validConfEntries[MAX_CONF_KEY] = {
  { "FILE_VERSION", " %2d", sizeof(confFileVersion), &confFileVersion },
  { "AUTO_FORM_NETWORK", " %d", sizeof(autoFormNetwork), &autoFormNetwork },
  { "USE_PREFIX_DELEGATION", "%d", sizeof(usePrefixDelegation), &usePrefixDelegation },
  { "NETWORK_ID", " %%%ds", EMBER_NETWORK_ID_SIZE, networkId },
  { "MESH_SUBNET", " %s", sizeof(globalNetworkPrefix.bytes), globalNetworkPrefix.bytes },
};

static void loadBorderRouterConfiguration(void);
static void generateJoinKey(void);

static ConfKey identifyEntry(char *confFileLine, ConfEntry *validEntries);
static void trimWhitespace(char* inString, size_t inStringLength);
static uint32_t parseConfigurationFile(FILE *confFp);
static int simpleParser(const char *inString, ConfEntry *entry);
static int ipv6PrefixParser(const char *inString, ConfEntry *entry);

static void networkStateTransition(EmberNetworkStatus newNetworkStatus,
                                   EmberNetworkStatus oldNetworkStatus,
                                   EmberJoinFailureReason reason);
static void resumeNetwork(void);
static void formNetwork(void);
static void joinNetwork(void);
static void joinNetworkCommissionedCompletion(void);
static void getCommissioner(void);
static EmberStatus coapListen(const EmberIpv6Address*);
static void printAllThreadNodesAddresses(void);
static void configureListeners(void);
static void discover(void);
static void resetNetworkState(void);

// A second coap port for the web GUI (used for discover)
#define MGMT_COAP_PORT 4983

static void deviceAnnounceResponseHandler(EmberCoapStatus status,
                                          EmberCoapCode code,
                                          EmberCoapReadOptions *options,
                                          uint8_t *payload,
                                          uint16_t payloadLength,
                                          EmberCoapResponseInfo *info);

#define HEARTBEAT_PERIOD_TIMEOUT_MS (60 * MILLISECOND_TICKS_PER_SECOND)
#define INIT_PERIOD_TIMEOUT_MS (5 * MILLISECOND_TICKS_PER_SECOND)

// The border router assumes it will be running on a Raspberry Pi with a fixed
// wlan0 IPv6 address of 2001:db8:8569:b2b1::1, this should be obtained
// automatically in the future, or use localhost
static const EmberIpv6Address serverAddress = {
  { 0x20, 0x01, 0x0D, 0xB8, 0x85, 0x69, 0xB2, 0xB1,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, }
};

// DNS64 ND Option Types
#define ND_RECURSIVE_DNS_SERVER 25
#define ND_DNS_SEARCH_LIST      31

// Setup DNS64 to use BR's nat64 to Google's IPv4 DNS (FC01:6464::0808:0808)
#define ND_DATA_ADDRESS_OFFSET 8
static uint8_t ndData[] = { ND_RECURSIVE_DNS_SERVER, // Type
                            3, // Length of this structure in groups of 8 octets
                            0x00, 0x00, // 2 bytes reserved
                            0xFF, 0xFF, 0xFF, 0xFF, // Lifetime of infinity
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // DNS server IPv6 address
};

static bool ndUpdateInProgress = false;
static const EmberIpv6Address defaultNdAddress = { // Google's nat64'd IPv6 address
  { 0xFC, 0x01, 0x64, 0x64, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, }
};

static void updateNdData(const EmberIpv6Address* newAddress);

static const uint8_t borderRouterDiscoverUri[] = "borderrouter/discover";
static const uint8_t nodeJoinedUri[] = "borderrouter/nodejoined";
static const uint8_t nodeOnOffOutUri[] = "device/onoffout";
static const uint8_t onPayload[] = { '1' };
static const uint8_t offPayload[] = { '0' };

// This commissionerKey is used by the Commissioner to establish trust with
// this border router.
// WARNING: Using a static password across all devices is unsafe for
// productized releases, use pseudorandom paswords
static const uint8_t commissionerKey[] = "COMMPW1234";

static bool stackInitialized = false;
static bool pskcInitialized = false;

enum {
  INITIAL                       = 0,
  RESUME_NETWORK                = 1,
  FORM_OR_JOIN_NETWORK          = 2,
  FORM_NETWORK                  = 3,
  JOIN_NETWORK                  = 4,
  JOIN_NETWORK_COMPLETION       = 5,
  GET_COMMISSIONER              = 6,
  CONFIGURE_BORDER_ROUTER       = 7,
  INITIALIZE_BORDER_ROUTER      = 8,
  UP_STATE                      = 9,
  RESET_NETWORK_STATE           = 10,
};

static uint8_t state = INITIAL;
EmberEventControl stateEventControl;
static void setNextStateWithDelay(uint8_t nextState, uint32_t delayMs);
#define setNextState(nextState)       setNextStateWithDelay((nextState), 0)
#define repeatStateWithDelay(delayMs) setNextStateWithDelay(state, (delayMs))

static const uint8_t *printableNetworkId(void);

static EmberNetworkStatus previousNetworkStatus = EMBER_NO_NETWORK;
static EmberNetworkStatus networkStatus = EMBER_NO_NETWORK;
static EmberJoinFailureReason joinFailureReason = EMBER_JOIN_FAILURE_REASON_NONE;

// WARNING: be careful when using the function, below. This function uses
// border-router-mgmt-app's privileges to execute code and could be used to
// elevate privileges of a malicious user.  Please ensure all file permissions
// of any hook scripts executed by the function below are always maintained at
// border-router-mgmt-app's privilege level or higher

#define ALLOW_HOOK_SCRIPTS true
// forks and executes 'hookScript' with command line arguments argv[] and
// environment variables envp[] (key=value)
#if ALLOW_HOOK_SCRIPTS
static void executeHookScript(const char* hookScript,
                              char *const argv[],
                              char *const envp[]);
#define HOOK_SCRIPT_DIRECTORY "/opt/siliconlabs/threadborderrouter/hook-scripts"
#else
#define executeHookScript(...)
#define HOOK_SCRIPT_DIRECTORY
#endif

// All asynchronous callbacks except emberAfnetworkStateTransition that will
// change 'state' must first ensure that an ongoing reset isn't occuring
// before proceeding. This prevents a race condition where callbacks that
// assert( state == X) can be entered after having the state transitioned
// by a network reset.
//
// Because state == RESET_NETWORK_STATE can only be exited by the stack when
// emberAfNetworkStateTransition is called, it is the only asynchronous
// callback allowed to change state during reset
#define returnIfInNetworkReset(state) \
  do { if ((state) == RESET_NETWORK_STATE) { return; } } while (0)

void initialState(void)
{
  assert(state == INITIAL);
  emberEventControlSetInactive(stateEventControl);
  if (stackInitialized) {
    networkStateTransition(previousNetworkStatus,
                           networkStatus,
                           joinFailureReason);
  } else {
    emberAfCorePrintln("Border Router waiting on stack initialization...");
    repeatStateWithDelay(INIT_PERIOD_TIMEOUT_MS);
  }
}

void emberAfMainCallback(void)
{
  loadBorderRouterConfiguration();
}

static void loadBorderRouterConfiguration(void)
{
  inputFp = fopen(CONFIGURATION_FILE, "r");
  if (inputFp != NULL) {
    parseConfigurationFile(inputFp);
    fclose(inputFp);
    inputFp = NULL;
  } else {
    switch (errno) {
      case ENOENT:
        // ENOENT is hit when a file doesn't exist.
        emberAfCorePrintln("No Configuration file \"%s\"; Defaulting to precompiled settings.",
                           CONFIGURATION_FILE);
        break;
      case EACCES: case EAGAIN: case EBADF:  case EDEADLK: case EFAULT:
      case EFBIG:  case EINTR:  case EINVAL: case EISDIR:  case EMFILE:
      case ENFILE: case ENOLCK: case ENOMEM: case EPERM:
        //default:
        emberAfCorePrintln("Failed to read configuration file (%d): %s",
                           errno,
                           strerror(errno));
        break;
    }
  }
}

static void generateJoinKey(void)
{
  // Using a join key generated from the EUI64 of the node is a security risk,
  // but is useful for demonstration purposes. See the warning above. This
  // hash generates alphanumeric characters in the ranges 0-9 and A-U. The
  // Thread specification disallows the characters I, O, Q, and Z, for
  // readability. I, O, and Q are therefore remapped to V, W, and X. Z is not
  // generated, so it is not remapped.
  const EmberEui64 *eui64;
  eui64 = emberEui64();
  for (joinKeyLength = 0; joinKeyLength < EUI64_SIZE; (joinKeyLength)++) {
    joinKey[joinKeyLength] = ((eui64->bytes[joinKeyLength] & 0x0F)
                              + (eui64->bytes[joinKeyLength] >> 4));
    joinKey[joinKeyLength] += (joinKey[joinKeyLength] < 10 ? '0' : 'A' - 10);
    if (joinKey[joinKeyLength] == 'I') {
      joinKey[joinKeyLength] = 'V';
    } else if (joinKey[joinKeyLength] == 'O') {
      joinKey[joinKeyLength] = 'W';
    } else if (joinKey[joinKeyLength] == 'Q') {
      joinKey[joinKeyLength] = 'X';
    }
  }
}

void emberAfInitCallback(void)
{
  if (!stackInitialized) {
    emberAfCorePrintln("Stack initialized");
    stackInitialized = true;

    generateJoinKey();

    // On init, both previous and current status are equal
    previousNetworkStatus = emberNetworkStatus();
    networkStatus = previousNetworkStatus;
  } else {
    // emberAfInitCallback can be called multiple times if the NCP resets. This
    // should not be occurring, so we'll log this and trigger an assert
    emberAfCorePrintln("NCP reset occured, emberAfInitCallback called twice");
    assert(false);
  }
}

static void networkStateTransition(EmberNetworkStatus newNetworkStatus,
                                   EmberNetworkStatus oldNetworkStatus,
                                   EmberJoinFailureReason reason)
{
  EmberNetworkParameters parameters = { { 0 } };
  emberGetNetworkParameters(&parameters);

  // If we have no network, we either form a network, or reside in a
  // "no network" state, depending on the configuration to auto-form. If we have
  // a saved network, we try to resume operations on that network.

  switch (newNetworkStatus) {
    case EMBER_NO_NETWORK:
      if (oldNetworkStatus == EMBER_JOINING_NETWORK) {
        // Determine whether we were joining, if so join again to count down
        // any remaining join attempts and break out immediately
        if (state == JOIN_NETWORK) {
          emberAfCorePrintln("ERR: Joining failed: 0x%x", reason);
          setNextState(JOIN_NETWORK);
          break;
        }
        // If we were forming, then post an error about forming
        if (state == FORM_NETWORK) {
          emberAfCorePrintln("ERR: Forming failed: 0x%x", reason);
        }
      }
      setNextState(FORM_OR_JOIN_NETWORK);
      break;
    case EMBER_SAVED_NETWORK:
      setNextState(RESUME_NETWORK);
      break;
    case EMBER_JOINING_NETWORK:
      // Wait for either the "attaching" or "no network" state.
      break;
    case EMBER_JOINED_NETWORK_ATTACHING:
      // Wait for the "attached" state.
      break;
    case EMBER_JOINED_NETWORK_ATTACHED:
      emberAfCorePrintln("%s network \"%s\"",
                         (state == RESUME_NETWORK
                          ? "Resumed operation on"
                          : (state == FORM_NETWORK
                             ? "Formed"
                             : "Rejoined")),
                         printableNetworkId());

      // Reset any remaining join attempts since we have joined
      joinAttemptsRemaining = 0;

      // Re-enable the host as a DTLS client to be able to interface with a
      // commissioner since we disabled it when starting the join
      emberEnableHostDtlsClient(true);

      // Setup commissioning
      setNextState(GET_COMMISSIONER);
      break;
    case EMBER_JOINED_NETWORK_NO_PARENT:
      // We always form as a router, so we should never end up in the "no parent"
      // state.
      emberAfCorePrintln("ERR: Network state transitioned to an invalid EMBER_JOINED_NETWORK_NO_PARENT state");
      break;
    default:
      emberAfCorePrintln("ERR: Network state transitioned to an UNKNOWN state");
      break;
  }
}

void emberAfNetworkStatusCallback(EmberNetworkStatus newNetworkStatus,
                                  EmberNetworkStatus oldNetworkStatus,
                                  EmberJoinFailureReason reason)
{
  // This callback is called whenever the network status changes, like when
  // we finish joining to a network or when we lose connectivity.

  emberEventControlSetInactive(stateEventControl);

  // If the stack is initialized accept network transitions
  networkStateTransition(newNetworkStatus, oldNetworkStatus, reason);
}

static void resumeNetwork(void)
{
  assert(state == RESUME_NETWORK);

  emberAfCorePrintln("Resuming operation on network \"%s\"",
                     printableNetworkId());
  emberResumeNetwork();
}

void emberResumeNetworkReturn(EmberStatus status)
{
  returnIfInNetworkReset(state);

  // This return indicates whether the stack is going to attempt to resume.  If
  // so, the result is reported later as a network status change.  If we cannot
  // resume, give up and try to form/join instead.

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: Unable to resume: 0x%x", status);
    setNextState(FORM_OR_JOIN_NETWORK);
  }
}

static void setupCommissioningData(void)
{
  // Allow commissioners to connect, and set our commissioner key.
  emberAllowNativeCommissioner(true);
  emberAfCorePrintln("Setting fixed commissioner key: \"%s\"",
                     commissionerKey);
  emberSetCommissionerKey(commissionerKey, sizeof(commissionerKey) - 1);
}

void emberSetPskcHandler(const uint8_t *pskc)
{
  pskcInitialized = true;
  // Call this to resign as a commissioner if this PSKc was set
  // by us temporarily becoming a commissioner.
  // See IOTS_SW_THREAD-948.
  emberGetCommissioner();
}

static void formNetwork(void)
{
  assert(state == FORM_NETWORK);

  // set up our commissioning passphrase and PSKc.
  setupCommissioningData();
  pskcInitialized = true;

  EmberNetworkParameters parameters = { { 0 } };

  emberAfCorePrintln("Forming network \"%s\"", networkId);

  MEMCOPY(parameters.networkId, networkId, EMBER_NETWORK_ID_SIZE);
  parameters.nodeType = EMBER_ROUTER;
  parameters.radioTxPower = 3;

  emberFormNetwork(&parameters,
                   (EMBER_NETWORK_ID_OPTION
                    | EMBER_NODE_TYPE_OPTION
                    | EMBER_TX_POWER_OPTION),
                   EMBER_ALL_802_15_4_CHANNELS_MASK);
}

void emberFormNetworkReturn(EmberStatus status)
{
  returnIfInNetworkReset(state);

  // This return indicates whether the stack is going to attempt to form.  If
  // so, the result is reported later as a network status change.  Otherwise,
  // we just try again.

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: Unable to form: 0x%x", status);
    setNextState(FORM_OR_JOIN_NETWORK);
  }
}

void emberCommissionNetworkReturn(EmberStatus status)
{
  // This will only get called if emberCommissionNetwork() is called. In this
  // reference design this can only happen by using the network-management CLI
  // plugin and can be done when `set-auto-form-off` has been called to put
  // the border-router-management-app in the NO_NETWORK state.

  returnIfInNetworkReset(state);

  switch (status) {
    case EMBER_SUCCESS:
      // Delay one tick so the network stack can unwind
      emberAfCorePrintln("Pre-Commission stack call successful.");
      setNextState(JOIN_NETWORK_COMPLETION);
      break;
    case EMBER_BAD_ARGUMENT:
    case EMBER_INVALID_CALL:
      // Try to form the network again if it fails
      emberAfCorePrintln("Pre-Commission failed status %x, retrying", status);
      setNextState(FORM_OR_JOIN_NETWORK);
      break;
    default:
      break;
  }
}

void joinNetworkCommissionedCompletion(void)
{
  emberAfCorePrintln("Completing precommisioned join");

  // We could end up forming a network via this call, so
  // set up our commissioning passphrase and PSKc.
  setupCommissioningData();
  pskcInitialized = true;

  emberJoinCommissioned(3,            // radio tx power
                        EMBER_ROUTER, // type
                        true);        // require connectivity
}

// border-router start-connect
void startConnectCommand(void)
{
  if ((state == FORM_OR_JOIN_NETWORK)
      && !autoFormNetwork
      && (joinAttemptsRemaining == 0)) {
    emberAfCorePrintln("Attempting to join-network");
    joinAttemptsRemaining = MAX_REJOIN_ATTEMPTS;
    setNextState(JOIN_NETWORK);
  } else {
    emberAfCorePrintln("Cannot start-connect: state = %d, autoFormNetwork = %d, joinAttemptsRemaining = %d",
                       state,
                       autoFormNetwork,
                       joinAttemptsRemaining);
  }
}

// border-router stop-connect
void stopConnectCommand(void)
{
  if ((state == JOIN_NETWORK) && (joinAttemptsRemaining > 0)) {
    emberAfCorePrintln("Stopping join-network attempts, wait for the most recent join to fail before attempting start-connect again");
    joinAttemptsRemaining = 0;
  } else {
    emberAfCorePrintln("Cannot stop-connect: state = %d, joinAttemptsRemaining = %d",
                       state,
                       joinAttemptsRemaining);
  }
}

static void joinNetwork(void)
{
  // When joining a network, we look for one specifically with our network id.
  // The commissioner must have our join key for this to succeed.
  assert(state == JOIN_NETWORK);

  EmberNetworkParameters parameters = { { 0 } };

  if (joinAttemptsRemaining > 0) {
    // Decrement the join counter
    joinAttemptsRemaining--;

    emberAfCorePrintln("Joining network using join passphrase \"%s\", join attempts left = %d",
                       joinKey,
                       joinAttemptsRemaining);

    parameters.nodeType = EMBER_ROUTER;
    parameters.radioTxPower = 3;
    parameters.joinKeyLength = joinKeyLength;
    MEMCOPY(parameters.joinKey, joinKey, parameters.joinKeyLength);

    // Disable the host as a DTLS client so all the DTLS will be performed on
    // NCP
    emberEnableHostDtlsClient(false);

    emberJoinNetwork(&parameters,
                     (EMBER_NODE_TYPE_OPTION
                      | EMBER_TX_POWER_OPTION
                      | EMBER_JOIN_KEY_OPTION),
                     EMBER_ALL_802_15_4_CHANNELS_MASK);
  } else {
    // No join attempts remain
    emberAfCorePrintln("Timed out attempting to join-network");

    // Re-enable the host as a DTLS client to be able to interface with a
    // commissioner since we disabled it when starting the join
    emberEnableHostDtlsClient(true);

    setNextState(FORM_OR_JOIN_NETWORK);
  }
}

void emberJoinNetworkReturn(EmberStatus status)
{
  returnIfInNetworkReset(state);

  // This return indicates whether the stack is going to attempt to join.  If
  // so, the result is reported later as a network status change.  Otherwise,
  // we just try again.

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: Unable to join: 0x%x", status);
    setNextState(FORM_OR_JOIN_NETWORK);
  }
}

static void getCommissioner(void)
{
  assert(state == GET_COMMISSIONER);

  emberGetCommissioner();
}

void emberCommissionerStatusHandler(uint16_t flags,
                                    const uint8_t *commissionerName,
                                    uint8_t commissionerNameLength)
{
  returnIfInNetworkReset(state);

  if (flags == EMBER_NO_COMMISSIONER) {
    if (state == GET_COMMISSIONER) {
      emberAfCorePrintln("Network has no commissioner.");

      // We have no commissioner, so if the network did not have a PSKc TLV
      // before, attempt to become the commissioner, and set the PSKc.
      //
      // Networks that intend to connect to external commissioners must ALWAYS
      // start with a PSKc in their active dataset.  Devices in an earlier stack
      // version did not do so.  If they are upgraded and resume network state
      // from tokens, then the network won't have a PSKc.
      //
      // We will temporarily become a commissioner and set the PSKc on the leader's
      // dataset.  Devices that resume this way, or form a new network will
      // populate the PSKc accordingly, and this particular state machine may
      // not be needed at a later date.
      //
      // See IOTS_SW_THREAD-948 for more context.
      //
      if (!pskcInitialized) {
        emberAfCorePrintln("Becoming temporary commissioner to set missing PSKc");
        emberBecomeCommissioner((const uint8_t *) "thread", 6);
      }
    }
  } else if (!READBITS(flags, EMBER_AM_COMMISSIONER)) {
    if (state == GET_COMMISSIONER) {
      // If we are not the commissioner, print out the name of the commissioner
      emberAfCorePrint("Network already has a commissioner ");
      if (commissionerName != NULL) {
        emberAfCorePrint(": \"");
        uint8_t i;
        for (i = 0; i < commissionerNameLength; i++) {
          emberAfCorePrint("%c", commissionerName[i]);
        }
      }
      emberAfCorePrintln("\"");
    }
  } else if (!pskcInitialized) {
    // We're the commissioner.  Set up the PSKc and tell the commission-proxy-app.
    setupCommissioningData();
  } else {
    // Stop, so that the external commissioner can connect.
    emberAfCorePrintln("Done setting PSKc, resigning from temporary commissioner duty.");
    emberStopCommissioning();
  }

  if (state == GET_COMMISSIONER) {
    setNextState(CONFIGURE_BORDER_ROUTER);
  }
}

static void configureBorderRouter(void)
{
  assert(state == CONFIGURE_BORDER_ROUTER);

  emberConfigureGateway(EMBER_BORDER_ROUTER_ND_DNS_FLAG
                        | EMBER_BORDER_ROUTER_DEFAULT_ROUTE_FLAG
                        | EMBER_BORDER_ROUTER_SLAAC_FLAG
                        | EMBER_BORDER_ROUTER_PREFERRED_FLAG,
                        true, // use a static, stable prefix
                        globalNetworkPrefix.bytes, // the prefix
                        globalNetworkPrefixBitWidth, //prefixLengthInBits
                        0, // domain id
                        0, // Preferred lifetime unused
                        0);// Valid lifetime unused
}

void emberConfigureGatewayReturn(EmberStatus status)
{
  returnIfInNetworkReset(state);

  // Determine if we are in the configure border router state, as part of init,
  // and only move through the state machine in that case, otherwise print
  // an error result only
  if (state == CONFIGURE_BORDER_ROUTER) {
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln("ERR: unable to configure gateway");
      // Only move through the state machine if we are auto-forming
      if (autoFormNetwork) {
        setNextState(RESET_NETWORK_STATE);
      }
    } else {
      emberAfCorePrint("Configuring default gateway for: ");
      emberAfCoreDebugExec(
        emberAfPrintIpv6Prefix(&globalNetworkPrefix,
                               globalNetworkPrefixBitWidth));
      emberAfCorePrintln("");
      setNextState(INITIALIZE_BORDER_ROUTER);
    }
  } else {
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln("ERR: unable to configure gateway");
    }
  }
}

static void initializeBorderRouter(void)
{
  emberAfCorePrintln("Initializing Border Router");
  printAllThreadNodesAddresses();
  configureListeners();

#if ALLOW_HOOK_SCRIPTS
  char ulaAddrStr[EMBER_IPV6_ADDRESS_STRING_SIZE + 24]
    = { "ULA_ADDRESS=" };
  const uint8_t ulaPrefixOffset = strlen(ulaAddrStr);
  EmberIpv6Address ulaAddr = { 0 };
  assert(emberGetLocalIpAddress(REALM_SCOPE, &ulaAddr));
  assert(emberIpv6AddressToString(&ulaAddr,
                                  (uint8_t*)ulaAddrStr + ulaPrefixOffset,
                                  sizeof(ulaAddrStr) - ulaPrefixOffset));

  char llAllThreadNodesStr[EMBER_IPV6_ADDRESS_STRING_SIZE + 24]
    = { "LL_ALL_THREAD_NODES=" };
  const uint8_t llAllThreadNodesOffset = strlen(llAllThreadNodesStr);
  assert(emberIpv6AddressToString((EmberIpv6Address*)&emFf32AllThreadNodesMulticastAddress,
                                  (uint8_t*)llAllThreadNodesStr + llAllThreadNodesOffset,
                                  sizeof(llAllThreadNodesStr)
                                  - llAllThreadNodesOffset));
  char mlAllThreadNodesStr[EMBER_IPV6_ADDRESS_STRING_SIZE + 24]
    = { "ML_ALL_THREAD_NODES=" };
  const uint8_t mlAllThreadNodesOffset = strlen(mlAllThreadNodesStr);
  assert(emberIpv6AddressToString((EmberIpv6Address*)&emFf33AllThreadNodesMulticastAddress,
                                  (uint8_t*)mlAllThreadNodesStr + mlAllThreadNodesOffset,
                                  sizeof(mlAllThreadNodesStr)
                                  - mlAllThreadNodesOffset));
  char mlAllThreadRoutersStr[EMBER_IPV6_ADDRESS_STRING_SIZE + 24]
    = { "ML_ALL_THREAD_ROUTERS=" };
  const uint8_t mlAllThreadRoutersOffset = strlen(mlAllThreadRoutersStr);
  assert(emberIpv6AddressToString((EmberIpv6Address*)&emFf33AllThreadRoutersMulticastAddress,
                                  (uint8_t*)mlAllThreadRoutersStr + mlAllThreadRoutersOffset,
                                  sizeof(mlAllThreadRoutersStr)
                                  - mlAllThreadRoutersOffset));
  char *envp[] = {
    ulaAddrStr,
    llAllThreadNodesStr,
    mlAllThreadNodesStr,
    mlAllThreadRoutersStr,
    0
  };

  executeHookScript("border-router-mgmt-app-post-up.hook", NULL, envp);
#endif

  setNextState(UP_STATE);
}

static void printAllThreadNodesAddresses(void)
{
  // emSetDefaultGlobalPrefix() should have already been called. This call
  // constructs the following addrs. once we have knowledge of the ULA prefix.

  emberAfCorePrintln("Using the following thread multicast addresses:");
  emberAfCoreDebugExec(emberAfPrintIpv6Address((EmberIpv6Address*)&emFf32AllThreadNodesMulticastAddress));
  emberAfCorePrintln("");
  emberAfCoreDebugExec(emberAfPrintIpv6Address((EmberIpv6Address*)&emFf33AllThreadNodesMulticastAddress));
  emberAfCorePrintln("");
  emberAfCoreDebugExec(emberAfPrintIpv6Address((EmberIpv6Address*)&emFf33AllThreadRoutersMulticastAddress));
  emberAfCorePrintln("");
}

static void configureListeners(void)
{
  // Setup a listener on the multicast addresses
  emberAfCorePrintln("Listening on emFf02AllNodesMulticastAddress");
  coapListen((const EmberIpv6Address *)&emFf02AllNodesMulticastAddress);

  emberAfCorePrintln("Listening on emFf02AllRoutersMulticastAddress");
  coapListen((const EmberIpv6Address *)&emFf02AllRoutersMulticastAddress);

  emberAfCorePrintln("Listening on emFf03AllNodesMulticastAddress");
  coapListen((const EmberIpv6Address *)&emFf03AllNodesMulticastAddress);

  emberAfCorePrintln("Listening on emFf03AllRoutersMulticastAddress");
  coapListen((const EmberIpv6Address *)&emFf03AllRoutersMulticastAddress);

  emberAfCorePrintln("Listening on emFf32AllThreadNodesMulticastAddress");
  coapListen((const EmberIpv6Address *)&emFf32AllThreadNodesMulticastAddress);

  emberAfCorePrintln("Listening on emFf33AllThreadNodesMulticastAddress");
  coapListen((const EmberIpv6Address *)&emFf33AllThreadNodesMulticastAddress);

  emberAfCorePrintln("Listening on emFf33AllThreadRoutersMulticastAddress");
  coapListen((const EmberIpv6Address *)&emFf33AllThreadRoutersMulticastAddress);
}

static EmberStatus coapListen(const EmberIpv6Address *address)
{
  EmberStatus status = emberUdpListen(EMBER_COAP_PORT, address->bytes);
  if (status != EMBER_SUCCESS) {
    emberAfCorePrint("ERR: Listening for CoAP on ");
    emberAfCoreDebugExec(emberAfPrintIpv6Address(address));
    emberAfCorePrintln(" failed: 0x%x", status);
  } else {
    emberAfCorePrint("Listening for CoAP on ");
    emberAfCoreDebugExec(emberAfPrintIpv6Address(address));
    emberAfCorePrintln("");
  }
  return status;
}

static void heartbeat(void)
{
  assert(state == UP_STATE);

  emberAfCorePrintln("Border router is up");

  // Each heartbeat we should look up global addresses that exist and use these
  // to update our ND data for DNS lookups, or set a default server if no global
  // addresses return.
  ndUpdateInProgress = true;
  emberGetGlobalAddresses(NULL, 0); // force an update of the global address

  repeatStateWithDelay(HEARTBEAT_PERIOD_TIMEOUT_MS);
}

void emberGetGlobalAddressReturn(const EmberIpv6Address *address,
                                 uint32_t preferredLifetime,
                                 uint32_t validLifetime,
                                 uint8_t addressFlags)
{
  // Do nothing unless we have an ndUpdate request in progress
  if (!ndUpdateInProgress) {
    return;
  }

  // We got an address, so we will ignore future callbacks from this until we
  // try to update again
  ndUpdateInProgress = false;

  if (emberIsIpv6UnspecifiedAddress(address)) {
    // If we did not find a global address then update the ndData with the
    // default address
    updateNdData(&defaultNdAddress);
  } else {
    // If found a global address then compare it to the existing server set new
    // ndData if it is changing
    updateNdData(address);
  }
}

static void updateNdData(const EmberIpv6Address* newAddress)
{
  // This will compare to the existing ndData and only update if something has
  // changed
  if (MEMCOMPARE(ndData + ND_DATA_ADDRESS_OFFSET, newAddress->bytes, 16) != 0) {
    emberAfAppPrint("Updating ND server address: ");
    emberAfAppDebugExec(emberAfPrintIpv6Address(newAddress));
    emberAfAppPrint("\n");
    memcpy(ndData + ND_DATA_ADDRESS_OFFSET, newAddress->bytes, 16);
    emberSetNdData(ndData, sizeof(ndData));
  }
}

void emberSetNdDataReturn(EmberStatus status, uint16_t length)
{
  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: unable to configure DNS64: 0x%x", status);
  } else {
    emberAfCorePrintln("Configured DNS64");
  }
}

void emberSendSteeringDataReturn(EmberStatus status)
{
  // The steering data helps bring new devices into our network.

  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("Sent steering data");
  } else {
    emberAfCorePrintln("ERR: Sending steering data failed: 0x%x", status);
  }
}

void emberAddressConfigurationChangeHandler(const EmberIpv6Address *address,
                                            uint32_t preferredLifetime,
                                            uint32_t validLifetime,
                                            uint8_t addressFlags)
{
  ALIAS(emberAddressConfigurationChangeHandler)(address,
                                                preferredLifetime,
                                                validLifetime,
                                                addressFlags);
#if ALLOW_HOOK_SCRIPTS
  char scopeStr[8] = { "SCOPE=" };
  const uint8_t scopeOffset = strlen(scopeStr);
  assert(snprintf(scopeStr + scopeOffset,
                  sizeof(scopeStr) - scopeOffset,
                  "%d",
                  emberGetScopeFromAddress(address)) > 0);
  char meshAddrStr[EMBER_IPV6_ADDRESS_STRING_SIZE + 16] = { "MESH_ADDR=" };
  const uint8_t addrOffset = strlen(meshAddrStr);
  assert(emberIpv6AddressToString(address,
                                  (uint8_t*)meshAddrStr + addrOffset,
                                  sizeof(meshAddrStr) - addrOffset));
  char *envp[] = {
    scopeStr,
    meshAddrStr,
    0
  };
#endif

  if (validLifetime != 0) {
    // address added
    EmberStatus status;

    status = emberIcmpListen(address->bytes);
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln("ERR: Listening for ICMP failed: 0x%x", status);
    }

    // Log output is printed within coapListen
    coapListen(address);

    executeHookScript("mesh-address-added.hook",
                      NULL, envp);
  } else {
    // address removed
    executeHookScript("mesh-address-removed.hook",
                      NULL, envp);
  }
}

// set-auto-form-on
void setAutoFormOnCommand(void)
{
  autoFormNetwork = true;
}

// set-auto-form-off
void setAutoFormOffCommand(void)
{
  autoFormNetwork = false;
}

// discover
void discoverCommand(void)
{
  // If we are in an up state, we can manually send a new discovery request
  // using a CLI command.
  discover();
}

void serverDiscoverHandler(EmberCoapCode code,
                           uint8_t *uri,
                           EmberCoapReadOptions *options,
                           const uint8_t *payload,
                           uint16_t payloadLength,
                           const EmberCoapRequestInfo *info)
{
  emberAfCorePrintln("Received Device Discovery CoAP Message from Server");
  discover();
}

static void discover(void)
{
  // Discovery will send a discovery URI to all nodes in the mesh. Thread
  // devices that hear these advertisements will announce their global address
  // to the border-router-mgmt-app's global address.

  EmberStatus status;

  emberAfCorePrint("Sending discovery URI to ");
  emberAfCoreDebugExec(emberAfPrintIpv6Address((EmberIpv6Address*)&emFf33AllThreadNodesMulticastAddress));
  emberAfCorePrintln("");

  // Use defaults for everything except the NON.
  EmberCoapSendInfo info = {
    .nonConfirmed = true,
  };
  status = emberCoapPost((EmberIpv6Address*)&emFf33AllThreadNodesMulticastAddress,
                         borderRouterDiscoverUri,
                         NULL, // body
                         0,    // body length
                         NULL, // handler
                         &info);
  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: Discovery failed: 0x%x", status);
  }
}

// Helper function used to specify source/destination port
static EmberStatus coapSendUri(EmberCoapCode code,
                               const EmberIpv6Address *destination,
                               const uint16_t destinationPort,
                               const uint16_t sourcePort,
                               const uint8_t *uri,
                               const uint8_t *body,
                               uint16_t bodyLength,
                               EmberCoapResponseHandler responseHandler)
{
  // Use defaults for everything except the ports.
  EmberCoapSendInfo info = {
    .localPort = sourcePort,
    .remotePort = destinationPort,
  };
  return emberCoapSend(destination,
                       code,
                       uri,
                       body,
                       bodyLength,
                       responseHandler,
                       &info);
}

void deviceAnnounceHandler(EmberCoapCode code,
                           uint8_t *uri,
                           EmberCoapReadOptions *options,
                           const uint8_t *payload,
                           uint16_t payloadLength,
                           const EmberCoapRequestInfo *info)
{
  EmberStatus status;

  emberAfCorePrintln("Received Device Announce CoAP Message: "
                     "Payload String (length=%d) %s",
                     payloadLength,
                     payload);

  // Here we are relaying a CoAP message to the server address that contains
  // the same payload as our nodejoined message. This is done only so the
  // server running locally can be aware of devices that become available. A
  // proper discovery method should be added in for this.

  status = coapSendUri(EMBER_COAP_CODE_POST,
                       &serverAddress,
                       MGMT_COAP_PORT, //dport for mgmt coap to webserver
                       EMBER_COAP_PORT, //sport for border-router-mgmt-app listener
                       nodeJoinedUri,
                       payload,
                       payloadLength,
                       deviceAnnounceResponseHandler);

  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: Error reporting device to host, status: 0x%x",
                       status);
  }
}

static void deviceAnnounceResponseHandler(EmberCoapStatus status,
                                          EmberCoapCode code,
                                          EmberCoapReadOptions *options,
                                          uint8_t *payload,
                                          uint16_t payloadLength,
                                          EmberCoapResponseInfo *info)
{
  if (status != EMBER_COAP_MESSAGE_ACKED) {
    emberAfCorePrintln("ERR: CoAP Request Failed: 0x%x", status);
  }
}

// multicast-on
void multicastonCommand(void)
{
  emberAfCorePrint("Sending ALL ON URI to ");
  emberAfCoreDebugExec(emberAfPrintIpv6Address((EmberIpv6Address*)&emFf33AllThreadNodesMulticastAddress));
  emberAfCorePrintln("");

  // Use defaults for everything except the NON.
  EmberCoapSendInfo info = {
    .nonConfirmed = true,
  };
  EmberStatus status = emberCoapPost((EmberIpv6Address*)&emFf33AllThreadNodesMulticastAddress,
                                     nodeOnOffOutUri,
                                     onPayload,
                                     sizeof(onPayload),
                                     NULL, // handler
                                     &info);
  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: ALL ON failed: 0x%x", status);
  }
}

// multicast-off
void multicastoffCommand(void)
{
  emberAfCorePrint("Sending ALL OFF URI to ");
  emberAfCoreDebugExec(emberAfPrintIpv6Address((EmberIpv6Address*)&emFf33AllThreadNodesMulticastAddress));
  emberAfCorePrintln("");

  // Use defaults for everything except the NON.
  EmberCoapSendInfo info = {
    .nonConfirmed = true,
  };
  EmberStatus status = emberCoapPost((EmberIpv6Address*)&emFf33AllThreadNodesMulticastAddress,
                                     nodeOnOffOutUri,
                                     offPayload,
                                     sizeof(offPayload),
                                     NULL, // handler
                                     &info);
  if (status != EMBER_SUCCESS) {
    emberAfCorePrintln("ERR: ALL OFF failed: 0x%x", status);
  }
}

void netResetHandler(EmberCoapCode code,
                     uint8_t *uri,
                     EmberCoapReadOptions *options,
                     const uint8_t *payload,
                     uint16_t payloadLength,
                     const EmberCoapRequestInfo *info)
{
  if (state == UP_STATE) {
    emberAfCorePrintln("Performing net reset from CoAP");
    // No CoAP response required, these will be non-confirmable requests
    setNextState(RESET_NETWORK_STATE);
  }
}

static void resetNetworkState(void)
{
  emberAfCorePrintln("Resetting network state");
  emberResetNetworkState();
}

void emberResetNetworkStateReturn(EmberStatus status)
{
  // If we ever leave the network, we go right back to forming again.  This
  // could be triggered by an external CLI command.
  if (status == EMBER_SUCCESS) {
    emberAfCorePrintln("Reset network state complete");
  }
}

void stateEventHandler(void)
{
  emberEventControlSetInactive(stateEventControl);

  switch (state) {
    case INITIAL:
      initialState();
      break;
    case RESUME_NETWORK:
      resumeNetwork();
      break;
    case FORM_OR_JOIN_NETWORK:
      // Determine if the border-router-mgmt-app should form a network
      // automatically, otherwise sit in the no network state to wait for a
      // join-network, or other command
      if (autoFormNetwork) {
        setNextState(FORM_NETWORK);
      } else {
        emberAfCorePrintln("Border Router auto-form is set to OFF:");
        emberAfCorePrintln("  - Use the `network-management commission ...` command to form and join network manually");
        emberAfCorePrintln("  - Use the `border-router start-connect` command to join device to network using EUI \"%x%x%x%x%x%x%x%x\" and join passphrase \"%s\"",
                           emberEui64()->bytes[7],
                           emberEui64()->bytes[6],
                           emberEui64()->bytes[5],
                           emberEui64()->bytes[4],
                           emberEui64()->bytes[3],
                           emberEui64()->bytes[2],
                           emberEui64()->bytes[1],
                           emberEui64()->bytes[0],
                           joinKey);
      }
      break;
    case FORM_NETWORK:
      formNetwork();
      break;
    case JOIN_NETWORK:
      joinNetwork();
      break;
    case JOIN_NETWORK_COMPLETION:
      joinNetworkCommissionedCompletion();
      break;
    case GET_COMMISSIONER:
      getCommissioner();
      break;
    case CONFIGURE_BORDER_ROUTER:
      configureBorderRouter();
      break;
    case INITIALIZE_BORDER_ROUTER:
      initializeBorderRouter();
      break;
    case UP_STATE:
      heartbeat();
      break;
    case RESET_NETWORK_STATE:
      resetNetworkState();
      break;
    default:
      assert(false);
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

// Configuration file-parsing helpers
static ConfKey identifyEntry(char *confLine, ConfEntry *entries)
{
  // Searches 'confLine' for valid 'entries' and returns the index in entries
  // corresponding to the found entry or else 'UNKNOWN_KEY' on failure
  ConfKey key;
  ConfEntry *entry;
  int rc = 0;

  if (entries != NULL && confLine != NULL) {
    for (key = (ConfKey)0; key < MAX_CONF_KEY; key++) {
      entry = entries + key;

      rc = strncasecmp(confLine, entry->key, strlen(entry->key));

      if (rc == 0) {
        return key;
      }
    }
  }
  return UNKNOWN_KEY;
}

static void trimWhitespace(char* inString, size_t bufLen)
{
  // Trims leading and trailing whitespace from inString and does not exceed
  // bufLen
  assert(inString);

  if (bufLen == 0) {
    return;
  }

  const char* stringStart = inString;
  char *stringEnd = 0;
  size_t strLength = 0;

  // Remove all whitespace at the start of the string
  while (*stringStart != '\0'
         && isspace(*stringStart)
         && (stringStart < (inString + bufLen))) {
    stringStart++;
  }

  strLength = strlen(stringStart);

  // In-place shift the string including the terminating NULL
  if (inString != stringStart) {
    MEMMOVE(inString, stringStart, strLength + 1);
  }

  stringEnd = strchr(inString, '\0') - 1;

  // Remove all whitespace at the end of the string
  while (inString < stringEnd  && isspace(*stringEnd)) {
    stringEnd--;
  }
  *(stringEnd + 1) = '\0';
}

static uint32_t parseConfigurationFile(FILE *confFp)
{
  // Takes a valid file pointer and parses the entries into memory
  assert(confFp);
  char *buffer = 0;
  size_t buffSz = 0;
  ssize_t rc = 0;
  char *valuesStart = 0;
  ConfKey key = UNKNOWN_KEY;
  ConfEntry *entry = NULL;
  uint32_t keys = 0;

  // This value needs to hold at least 6 characters at the start
  // (% % % d s \0), but may hold more characters if the number of digits in
  // EMBER_NETWORK_ID_SIZE increases the simplest thing to do in preprocessor
  // is to just add 6 to LENGTHOF(EMBER_NETWORK_ID_SIZE)
  char tempFmt[LENGTHOF(EMBER_NETWORK_ID_SIZE) + 6] = { '%', '%', '%', 'd', 's', 0 };
  char *valueLocation = NULL;
  // The networkId string needs to be restricted to max of
  // EMBER_NETWORK_ID_SIZE bytes
  snprintf(validConfEntries[NETWORK_ID_KEY].format,
           sizeof(entry->format),
           tempFmt,
           EMBER_NETWORK_ID_SIZE);

  while (!feof(confFp)) {
    if (feof(confFp)) {
      break;
    }

    rc = getline(&buffer, &buffSz, confFp);

    if (rc < 0) {
      switch (errno) {
        //only print fatal errors:
        case EACCES: case EAGAIN: case EBADF:
        case EINVAL: case ENFILE: case ENOMEM:
        case EPERM:
          //default:
          emberAfCorePrintln("Unable to getline: errno: (%d) \"%s\"",
                             errno,
                             strerror(errno));
          break;
      }
    } else if (rc > 0) {
      // Bytes were read by getline, parse the line
      trimWhitespace(buffer, buffSz);
      if ((buffer[0] == '#')
          || (buffer[0] == '/')
          || (buffer[0] == '\n')
          || buffer[0] == 0) {
        // Detected a comment, skip this line
        continue;
      } else {
        // Detected a line to parse
        key = identifyEntry(buffer, validConfEntries);
        keys |= (1 << key);
        if (key == UNKNOWN_KEY) {
          //emberAfCorePrintln("Unknown configuration entry skipped: (%s)",
          //                    buffer);
          continue;
        } else {
          // Found a valid entry
          entry = validConfEntries + key;
          valuesStart = buffer + strlen(entry->key);
          //emberAfCorePrintln("Parsing: %s", entry->key);
          switch (key) {
            case FILE_VERSION_KEY:
              rc = simpleParser(valuesStart, entry);
              break;
            case AUTO_FORM_NETWORK_KEY:
            case USE_PREFIX_DELEGATION_KEY:
              trimWhitespace(valuesStart, strlen(valuesStart));
              if ((strncasecmp(valuesStart, "false", 5) == 0)
                  || (strncasecmp(valuesStart, "no", 2) == 0)
                  || (strncmp(valuesStart, "0", 1) == 0)) {
                *((bool*)(entry->addr)) = false;
              } else {
                *((bool*)(entry->addr)) = true;
              }
              rc = true;
              break;
            case NETWORK_ID_KEY:
              rc = simpleParser(valuesStart, entry);
              // Check to see if the networkId is too long
              valueLocation = strstr(buffer, entry->addr);
              if ((valueLocation != NULL)
                  && (strlen(valueLocation) > (sizeof(networkId) - 1))) {
                emberAfCorePrintln("WARNING: networkId: %s is longer than %d "
                                   "characters and will be truncated!",
                                   valueLocation,
                                   sizeof(networkId) - 1);
              }
              break;
            case IPV6_SUBNET_KEY:
              trimWhitespace(valuesStart, strlen(valuesStart));
              rc = ipv6PrefixParser(valuesStart, entry);
              break;
            default:
              // Not a valid key
              assert("Invalid .conf key detected.");
              break;
          } // end 'switch(key)'
          if (rc != true) {
            emberAfCorePrintln("Unable to parse: \"%s\"", valuesStart);
            keys &= ~(1 << key);
          }
        } // end 'else found known key'
      } // end 'else detected a line to parse
    } // end 'else (getline rc>0)
  } // end 'while (!feof(confFp))'

  if (buffer != NULL) {
    free(buffer);
    buffer = NULL;
  }
  return keys;
}

static int ipv6PrefixParser(const char *inString, ConfEntry *entry)
{
  // Extract the prefix from the line
  EmberIpv6Address temp = { { 0 } };
  uint8_t prefixWidth = 0;
  if (!emberIpv6StringToPrefix((const uint8_t *)inString, &temp, &prefixWidth)) {
    emberAfCorePrintln("Failed to parse \"%s\"", entry->key);
    return false;
  } else {
    // Copy the entire 16 bytes
    MEMCOPY(entry->addr, temp.bytes, entry->sizeInBytes);

    // Set the bit-width of the global address
    globalNetworkPrefixBitWidth = prefixWidth;
  }
  return true;
}

static int simpleParser(const char *inString, ConfEntry *entry)
{
  // Takes the entry found in 'string' and places it into 'entry' using
  // 'format'
  int rc = 0;
  char tempLineBuffer[MAX_CONF_LINE_LENGTH + 1] = { 0 };
  rc = sscanf(inString, entry->format, tempLineBuffer);
  if ((rc == 1)
      || (strcmp(entry->key, validConfEntries[NETWORK_ID_KEY].key) == 0)) {
    MEMCOPY(entry->addr, tempLineBuffer, entry->sizeInBytes);
  } else {
    emberAfCorePrintln("simpleParser returned false for entry->key: %s",
                       entry->key);
    return false;
  }
  return true;
}

#if ALLOW_HOOK_SCRIPTS
static void executeHookScript(const char* hookScript, char *const argv[], char *const envp[])
{
  if (hookScript == NULL) {
    return;
  }

  int rc = 0;

  // WNOHANG | WUNTRACED | WCONTINUED
  const int waitOptions = 0;

  pid_t childId = fork();
  if (childId == 0) {
    // The child process executes a hook script (or executable) and then exits.

    uint16_t scriptNameLen = strlen(hookScript);
    uint16_t scriptPathLen = strlen(HOOK_SCRIPT_DIRECTORY);
    if (scriptNameLen + scriptPathLen > PATH_MAX) {
      emberAfCorePrintln("ERROR: hook script name cannot exceed %d bytes.",
                         PATH_MAX - scriptPathLen);
      return;
    }

    // allocate just enough room for the path and the script name
    // plus a forward slash and NULL characters
    const uint16_t buffLen = scriptNameLen + scriptPathLen + 2;
    char *scriptPath = malloc(buffLen);
    assert(scriptPath != NULL);

    snprintf(scriptPath, buffLen, "%s/%s", HOOK_SCRIPT_DIRECTORY, hookScript);
    emberAfCorePrintln("Executing hook script \"%s\"", scriptPath);

    rc = execve(scriptPath, argv, envp);
    MEMSET(scriptPath, 0, buffLen);
    free(scriptPath);
    scriptPath = NULL;

    if (rc < 0) {
      emberAfCorePrintln("Hook script execution failed. (%d) %s", errno, strerror(errno));
      // execve only returns on error; you must exit(ERROR) the child process
      exit(1);
    }
  } else if (childId > 0) {
    // wait for child...
    waitpid(childId, &rc, waitOptions);
    if ( WIFEXITED(rc) ) {
      emberAfCorePrint("Hook script %d terminated", childId);
      if ( WEXITSTATUS(rc) ) {
        emberAfCorePrint(" with exit status %d.", WEXITSTATUS(rc));
      }
      emberAfCorePrintln("");
    }

    if ( WIFSIGNALED(rc) ) {
      emberAfCorePrint("Hook script %d received signal", childId);
      if ( WTERMSIG(rc) ) {
        emberAfCorePrint(" %d that caused the child process to terminate.", WTERMSIG(rc));
      }

       #ifdef WCOREDUMP
      if ( WCOREDUMP(rc) ) {
        emberAfCorePrint(" which terminated with a core dump. Verify hook script contents.");
      }
       #endif
      emberAfCorePrint("");
    }

    if ( WIFSTOPPED(rc) ) {
      emberAfCorePrint("Hook script %d was STOPPED", childId);
      if ( WSTOPSIG(rc) ) {
        emberAfCorePrint(" with signal %d.", WSTOPSIG(rc));
      }
      emberAfCorePrintln("\nWARNING: the child process exited because its execution is being traced."
                         "This is a potential sign of malicious activity. Please see 'ptrace(2)'");
    }

    if ( WIFCONTINUED(rc) ) {
      emberAfCorePrintln("Hook script %d was resumed by delivery of SIGCONT.\n", childId);
    }
  } else {
    emberAfCorePrintln("Failed to fork child process to execute hook script.");
  }
}
#endif
