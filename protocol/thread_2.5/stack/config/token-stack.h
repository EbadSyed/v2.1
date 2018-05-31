/**
 * @file token-stack.h
 * @brief Definitions for stack tokens.
 * See @ref hal for documentation.
 *
 * The file token-stack.h should not be included directly.
 * It is accessed by the other token files.
 *
 * <!--Brooks Barrett-->
 * <!-- Copyright 2005 by Ember Corporation. All rights reserved. -->
 */

// Uncomment the line below to enable countertest
//#define ENABLE_COUNTERTEST

/**
 * @addtogroup hal
 * The tokens listed here are divided into three sections (the three main
 * types of tokens mentioned in token.h):
 * - Manufacturing
 * - Stack
 * - Application
 *
 * For a full explanation of the tokens, see hal/micro/token.h.
 * See token-stack.h for source code.
 *
 * A set of tokens is predefined in the APPLICATION DATA section at the
 * end of token-stack.h because these tokens are required by the stack,
 * but they are classified as application tokens since they are sized by the
 * application via its CONFIGURATION_HEADER.
 *
 * The user application can include its own tokens in a header file similar
 * to this one. The macro ::APPLICATION_TOKEN_HEADER should be defined to equal
 * the name of the header file in which application tokens are defined.
 * See the APPLICATION DATA section at the end of token-stack.h
 * for examples of token definitions.
 *
 * Since token-stack.h contains both the typedefs and the token defs, there are
 * two \#defines used to select which one is needed when this file is included.
 * \#define DEFINETYPES is used to select the type definitions and
 * \#define DEFINETOKENS is used to select the token definitions.
 * Refer to token.h and token.c to see how these are used.
 *
 * @{
 */

#ifndef DEFINEADDRESSES
/**
 * @brief By default, tokens are automatically located after the previous token.
 *
 * If a token needs to be placed at a specific location,
 * one of the DEFINE_FIXED_* definitions should be used.  This macro is
 * inherently used in the DEFINE_FIXED_* definition to locate a token, and
 * under special circumstances (such as manufacturing tokens) it may be
 * explicitely used.
 *
 * @param region   A name for the next region being located.
 * @param address  The address of the beginning of the next region.
 */
  #define TOKEN_NEXT_ADDRESS(region, address)
#endif

// The basic TOKEN_DEF macro should not be used directly since the simplified
//  definitions are safer to use.  For completeness of information, the basic
//  macro has the following format:
//
//  TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...)
//  name - The root name used for the token
//  creator - a "creator code" used to uniquely identify the token
//  iscnt - a bool flag that is set to identify a counter token
//  isidx - a bool flag that is set to identify an indexed token
//  type - the basic type or typdef of the token
//  arraysize - the number of elements making up an indexed token
//  ... - initializers used when reseting the tokens to default values
//
//
// The following convenience macros are used to simplify the definition
//  process for commonly specified parameters to the basic TOKEN_DEF macro
//  DEFINE_BASIC_TOKEN(name, type, ...)
//  DEFINE_INDEXED_TOKEN(name, type, arraysize, ...)
//  DEFINE_COUNTER_TOKEN(name, type, ...)
//  DEFINE_FIXED_BASIC_TOKEN(name, type, address, ...)
//  DEFINE_FIXED_INDEXED_TOKEN(name, type, arraysize, address, ...)
//  DEFINE_FIXED_COUNTER_TOKEN(name, type, address, ...)
//  DEFINE_MFG_TOKEN(name, type, address, ...)
//

/**
 * @name Convenience Macros
 * @brief The following convenience macros are used to simplify the definition
 * process for commonly specified parameters to the basic TOKEN_DEF macro.
 * Please see hal/micro/token.h for a more complete explanation.
 *@{
 */
#define DEFINE_BASIC_TOKEN(name, type, ...) \
  TOKEN_DEF(name, CREATOR_##name, 0, 0, type, 1, __VA_ARGS__)

#define DEFINE_COUNTER_TOKEN(name, type, ...) \
  TOKEN_DEF(name, CREATOR_##name, 1, 0, type, 1, __VA_ARGS__)

#define DEFINE_INDEXED_TOKEN(name, type, arraysize, ...) \
  TOKEN_DEF(name, CREATOR_##name, 0, 1, type, (arraysize), __VA_ARGS__)

#define DEFINE_FIXED_BASIC_TOKEN(name, type, address, ...) \
  TOKEN_NEXT_ADDRESS(name, (address))                      \
  TOKEN_DEF(name, CREATOR_##name, 0, 0, type, 1, __VA_ARGS__)

#define DEFINE_FIXED_COUNTER_TOKEN(name, type, address, ...) \
  TOKEN_NEXT_ADDRESS(name, (address))                        \
  TOKEN_DEF(name, CREATOR_##name, 1, 0, type, 1, __VA_ARGS__)

#define DEFINE_FIXED_INDEXED_TOKEN(name, type, arraysize, address, ...) \
  TOKEN_NEXT_ADDRESS(name, (address))                                   \
  TOKEN_DEF(name, CREATOR_##name, 0, 1, type, (arraysize), __VA_ARGS__)

#undef DEFINE_MFG_TOKEN
#define DEFINE_MFG_TOKEN(name, type, address, ...) \
  TOKEN_NEXT_ADDRESS(name, (address))              \
  TOKEN_MFG(name, CREATOR_##name, 0, 0, type, 1, __VA_ARGS__)

/** @} END Convenience Macros */

// The Simulated EEPROM unit tests define all of their own tokens.
#ifndef SIM_EEPROM_TEST

// The creator codes are here in one list instead of next to their token
// definitions so comparision of the codes is easier.  The only requirement
// on these creator definitions is that they all must be unique.  A favorite
// method for picking creator codes is to use two ASCII characters inorder
// to make the codes more memorable.

/**
 * @name Creator Codes
 * @brief The CREATOR is used as a distinct identifier tag for the
 * token.
 *
 * The CREATOR is necessary because the token name is defined
 * differently depending on the hardware platform, therefore the CREATOR makes
 * sure that token definitions and data stay tagged and known.  The only
 * requirement is that each creator definition must be unique.  Please
 * see hal/micro/token.h for a more complete explanation.
 *@{
 */

// STACK CREATORS
#define CREATOR_STACK_NVDATA_VERSION             0xFF01
#define CREATOR_STACK_BOOT_COUNTER               0xE263
#define CREATOR_STACK_NONCE_COUNTER              0xE563
#define CREATOR_STACK_ANALYSIS_REBOOT            0xE162
#define CREATOR_STACK_KEYS                       0xEB79
#define CREATOR_STACK_NODE_DATA                  0xEE64
#define CREATOR_STACK_CLASSIC_DATA               0xE364
#define CREATOR_STACK_ALTERNATE_KEY              0xE475
#define CREATOR_STACK_APS_FRAME_COUNTER          0xE123
#define CREATOR_STACK_TRUST_CENTER               0xE124
#define CREATOR_STACK_NETWORK_MANAGEMENT         0xE125
// APP CREATORS
#define CREATOR_STACK_BINDING_TABLE              0xE274
#define CREATOR_STACK_CHILD_TABLE                0xFF0D
#define CREATOR_STACK_KEY_TABLE                  0xE456
#define CREATOR_STACK_CERTIFICATE_TABLE          0xE500
#define CREATOR_STACK_PSL_DATA                   0xE501
#define CREATOR_STACK_HOST_REGISTRY              0xE502

/** @} END Creator Codes  */

//////////////////////////////////////////////////////////////////////////////
// MANUFACTURING DATA
// Since the manufacturing data is platform specific, we pull in the proper
// file here.
#if defined(MSP430)
  #include "hal/micro/msp430/token-manufacturing.h"
#elif defined(CORTEXM3)
// cortexm3 handles mfg tokens seperately via mfg-token.h
#elif defined(C8051_COBRA)
  #include "hal/micro/c8051/cobra/token-manufacturing.h"
#elif defined(EMBER_TEST)
  #include "hal/micro/unix/simulation/token-manufacturing.h"
#elif defined(UNIX_HOST) && defined(EMBER_AF_API_TOKEN)
// no mfg tokens on hosts
#else
  #error no platform defined
#endif

//////////////////////////////////////////////////////////////////////////////
// STACK DATA
// *the addresses of these tokens must not change*

/**
 * @brief The current version number of the stack tokens.
 * MSB is the version, LSB is a complement.
 *
 * Please see hal/micro/token.h for a more complete explanation.
 */
#define CURRENT_STACK_TOKEN_VERSION 0x03FC //MSB is version, LSB is complement

#ifdef DEFINETYPES
typedef uint16_t tokTypeStackNvdataVersion;
typedef uint16_t tokTypeStackAnalysisReboot;
typedef uint32_t tokTypeStackNonceCounter;

typedef struct {
  uint16_t shortId;
  uint8_t nodeType;
  int8_t powerLevel;
} tokTypeIpStackNodeData;

// This token was used prior to Thread 2.0a10.
// It remains to support upgrading without factory reset.
typedef struct {
  // This is the active timestamp, for both active and pending types.
  // The pending timestamp is stored in
  // tokTypeIpStackPendingCommissioningDatasetAddon.
  uint8_t timestamp[8];
  uint8_t networkId[16];
  uint8_t extendedPanId[8];
  uint8_t ulaPrefix[8];
  uint8_t masterKey[16];
  uint8_t pskc[16];
  uint8_t securityPolicy[3];
  uint16_t panId;
  // channel mask:
  // [0]: page
  // [1]: mask length, 4
  // [2-5]: mask
  uint8_t channelMask[6];
  // channel:
  // [0]: channel page
  // [1-2]: channel
  uint8_t channel[3];
  uint8_t unknownValues[100];
} tokTypeIpStackOldOperationalDataset;

// The 251 is the largest that the token system will allow (determined
// experimentally).  The Thread spec requires up to 256 bytes, so we
// have to put the remainder in a second token.  The 'tailIndex' field
// is the index where the rest is.
typedef struct {
  uint16_t length;
  uint8_t tailIndex;
  uint8_t tlvs[251];
} tokTypeIpStackOperationalDataset;

// We make an array of three of these.  One for the active dataset, one for
// the pending dataset, and an extra so that the tail of an updated dataset
// can be written without modifying either of the current values.
typedef struct {
  uint8_t tlvs[5];
} tokTypeIpStackOperationalDatasetTail;

typedef struct {
  uint8_t timestamp[8];
  uint32_t delayTimer;
} tokTypeIpStackPendingOperationalDatasetAddon;

typedef struct {
  uint8_t meshLocalInterfaceId[8];
} tokTypeIpStackMeshLocalInterfaceId;

typedef struct {
  uint8_t macExtendedId[8];
} tokTypeIpStackMacExtendedId;

typedef struct {
  uint8_t parentLongId[8];
} tokTypeIpStackParentLongId;

#ifdef EMBER_WAKEUP_STACK // For backwards compatibility.
typedef uint16_t tokTypeStackBootCounter;
typedef uint8_t tokTypeStackMulticastSequence;

typedef struct {
  // radio parameters
  uint16_t panId;
  uint8_t channel;

  // network parameters
  uint8_t networkId[16];
  uint8_t legacyUla[8];
  uint8_t extendedPanId[8];
  uint16_t shortId;

  // my parameters
  EmberNodeType nodeType;
  int8_t powerLevel;
} tokTypeIpStackWakeupNetworkData;

typedef struct {
  uint8_t masterKey[16];
  uint8_t sequence;
  uint8_t state;
} tokTypeIpStackWakeupSecurityData;

#else // EMBER_WAKEUP_STACK
// SIMEE2 requires 32 bit counter tokens.
typedef uint32_t tokTypeStackBootCounter;
typedef uint32_t tokTypeStackMulticastSequence;
#endif // EMBER_WAKEUP_STACK

#ifdef ENABLE_COUNTERTEST
typedef uint16_t dummyType[25];
#endif

#endif //DEFINETYPES

#ifdef DEFINETOKENS

//
// Define the token IDs
//
#define CREATOR_IP_STACK_WAKEUP_NETWORK_DATA     0xD001
#define CREATOR_IP_STACK_WAKEUP_SECURITY_DATA    0xD002
#define CREATOR_IP_STACK_MULTICAST_SEQUENCE      0xD003
#define CREATOR_IP_STACK_WAKEUP_SEQUENCE         0xD004
#define CREATOR_IP_STACK_SECURITY_SEQUENCE       0xD005
#define CREATOR_IP_STACK_MESH_LOCAL_INTERFACE_ID 0xD006
#define CREATOR_IP_STACK_MAC_EXTENDED_ID         0xD007
#define CREATOR_IP_STACK_PARENT_LONG_ID          0xD008
#define CREATOR_IP_STACK_NODE_DATA               0xD009
#define CREATOR_IP_STACK_OLD_ACTIVE_OPERATIONAL_DATASET  0xD00A
// Do not reuse these old creator ids in order to preserve upgradability.
// #define CREATOR_IP_STACK_PENDING_OPERATIONAL_DATASET 0xD00B
// #define CREATOR_IP_STACK_PENDING_OPERATIONAL_DATASET_ADDON 0xD00C
#define CREATOR_IP_STACK_ACTIVE_OPERATIONAL_DATASET 0xD00D
#define CREATOR_IP_STACK_PENDING_OPERATIONAL_DATASET 0xD00E
#define CREATOR_IP_STACK_OPERATIONAL_DATASET_TAIL 0xD00F

#ifdef ENABLE_COUNTERTEST
#define CREATOR_TESTCNT8   0x7008
#define CREATOR_TESTCNT16  0x7016
#define CREATOR_TESTCNT32  0x7032
#define CREATOR_DUMMY      0x7000
#endif

//
// Really define the tokens
//
DEFINE_BASIC_TOKEN(STACK_NVDATA_VERSION,
                   tokTypeStackNvdataVersion,
                   CURRENT_STACK_TOKEN_VERSION)
DEFINE_COUNTER_TOKEN(STACK_BOOT_COUNTER,
                     tokTypeStackBootCounter,
                     0x0000)
DEFINE_COUNTER_TOKEN(STACK_NONCE_COUNTER,
                     tokTypeStackNonceCounter,
                     0x00000000)
DEFINE_COUNTER_TOKEN(IP_STACK_MULTICAST_SEQUENCE,
                     tokTypeStackMulticastSequence,
                     0)
DEFINE_COUNTER_TOKEN(IP_STACK_SECURITY_SEQUENCE,
                     uint32_t,
                     0)
DEFINE_BASIC_TOKEN(IP_STACK_MESH_LOCAL_INTERFACE_ID,
                   tokTypeIpStackMeshLocalInterfaceId,
                   { { 0 } })
DEFINE_BASIC_TOKEN(IP_STACK_MAC_EXTENDED_ID,
                   tokTypeIpStackMacExtendedId,
                   { { 0 } })
DEFINE_BASIC_TOKEN(IP_STACK_PARENT_LONG_ID,
                   tokTypeIpStackParentLongId,
                   { { 0 } })
DEFINE_BASIC_TOKEN(IP_STACK_OLD_ACTIVE_OPERATIONAL_DATASET,
                   tokTypeIpStackOldOperationalDataset,
                   { { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, 0xFFFF, { 0 }, { 0 }, { 0 } })
DEFINE_BASIC_TOKEN(IP_STACK_ACTIVE_OPERATIONAL_DATASET,
                   tokTypeIpStackOperationalDataset,
                   { 0, 0, { 0 } })
DEFINE_BASIC_TOKEN(IP_STACK_PENDING_OPERATIONAL_DATASET,
                   tokTypeIpStackOperationalDataset,
                   { 0, 0, { 0 } })
DEFINE_INDEXED_TOKEN(IP_STACK_OPERATIONAL_DATASET_TAIL,
                     tokTypeIpStackOperationalDatasetTail,
                     3,  // active, pending, plus a temp for use when updating
                     { { 0 } })
DEFINE_BASIC_TOKEN(IP_STACK_NODE_DATA,
                   tokTypeIpStackNodeData,
                   { 0 })
#ifdef EMBER_WAKEUP_STACK
DEFINE_BASIC_TOKEN(IP_STACK_WAKEUP_NETWORK_DATA,
                   tokTypeIpStackWakeupNetworkData,
                   { 0xFFFF, 0, { 0 }, { 0 }, { 0 }, 0, 0, -1 })
DEFINE_INDEXED_TOKEN(IP_STACK_WAKEUP_SECURITY_DATA,
                     tokTypeIpStackWakeupSecurityData,
                     2,
                     { { 0 }, 0, 0 })
DEFINE_COUNTER_TOKEN(IP_STACK_WAKEUP_SEQUENCE,
                     uint16_t,
                     0)
#endif // EMBER_WAKEUP_STACK

#ifdef ENABLE_COUNTERTEST
DEFINE_COUNTER_TOKEN(TESTCNT8, uint8_t, 0x12)
DEFINE_COUNTER_TOKEN(TESTCNT16, uint16_t, 0x3456)
DEFINE_COUNTER_TOKEN(TESTCNT32, uint32_t, 0x789ABCDE)
DEFINE_BASIC_TOKEN(DUMMY, dummyType, { 0x00, })
#endif

#endif //DEFINETOKENS

//////////////////////////////////////////////////////////////////////////////
// PHY DATA
#include "token-phy.h"

// APPLICATION DATA
// *If a fixed application token is desired, its address must be above 384.*

#ifdef DEFINETYPES
typedef struct {
  uint8_t data[14];
} tokTypeChildTable;
#endif //DEFINETYPES

// The following application tokens are required by the stack, but are sized by
//  the application via its CONFIGURATION_HEADER, which is why they are present
//  within the application data section.  Any special application defined
//  tokens will follow.
// NOTE: changing the size of these tokens within the CONFIGURATION_HEADER
//  WILL move automatically move any custom application tokens that are defined
//  in the APPLICATION_TOKEN_HEADER
#ifdef DEFINETOKENS
#define CREATOR_CHILD_TABLE              0xD010

// Application tokens start at location 384 and are automatically positioned.
TOKEN_NEXT_ADDRESS(APP, 384)
DEFINE_INDEXED_TOKEN(CHILD_TABLE,
                     tokTypeChildTable,
                     EMBER_CHILD_TABLE_SIZE,
                     { { 0 } })
#endif //DEFINETOKENS
#ifdef APPLICATION_TOKEN_HEADER
  #include APPLICATION_TOKEN_HEADER
#endif

//The tokens defined below are test tokens.  They are normally not used by
//anything but are left here as a convenience so test tokens do not have to
//be recreated.  If test code needs temporary, non-volatile storage, simply
//uncomment and alter the set below as needed.
//#define CREATOR_TT01 1
//#define CREATOR_TT02 2
//#define CREATOR_TT03 3
//#define CREATOR_TT04 4
//#define CREATOR_TT05 5
//#define CREATOR_TT06 6
//#ifdef DEFINETYPES
//typedef uint32_t tokTypeTT01;
//typedef uint32_t tokTypeTT02;
//typedef uint32_t tokTypeTT03;
//typedef uint32_t tokTypeTT04;
//typedef uint16_t tokTypeTT05;
//typedef uint16_t tokTypeTT06;
//#endif //DEFINETYPES
//#ifdef DEFINETOKENS
//#define TT01_LOCATION 1
//#define TT02_LOCATION 2
//#define TT03_LOCATION 3
//#define TT04_LOCATION 4
//#define TT05_LOCATION 5
//#define TT06_LOCATION 6
//DEFINE_FIXED_BASIC_TOKEN(TT01, tokTypeTT01, TT01_LOCATION, 0x0000)
//DEFINE_FIXED_BASIC_TOKEN(TT02, tokTypeTT02, TT02_LOCATION, 0x0000)
//DEFINE_FIXED_BASIC_TOKEN(TT03, tokTypeTT03, TT03_LOCATION, 0x0000)
//DEFINE_FIXED_BASIC_TOKEN(TT04, tokTypeTT04, TT04_LOCATION, 0x0000)
//DEFINE_FIXED_BASIC_TOKEN(TT05, tokTypeTT05, TT05_LOCATION, 0x0000)
//DEFINE_FIXED_BASIC_TOKEN(TT06, tokTypeTT06, TT06_LOCATION, 0x0000)
//#endif //DEFINETOKENS

#else //SIM_EEPROM_TEST

//The Simulated EEPROM unit tests define all of their tokens via the
//APPLICATION_TOKEN_HEADER macro.
  #ifdef APPLICATION_TOKEN_HEADER
    #include APPLICATION_TOKEN_HEADER
  #endif

#endif //SIM_EEPROM_TEST

#ifndef DEFINEADDRESSES
  #undef TOKEN_NEXT_ADDRESS
#endif

/** @} END addtogroup */

/**
 * <!-- HIDDEN
 * @page 2p5_to_3p0
 * <hr>
 * The file token-stack.h is described in @ref hal and includes
 * the following:
 * <ul>
 * <li> <b>New items</b>
 *   - ::CREATOR_STACK_ALTERNATE_KEY
 *   - ::CREATOR_STACK_APS_FRAME_COUNTER
 *   - ::CREATOR_STACK_LINK_KEY_TABLE
 *   .
 * <li> <b>Changed items</b>
 *   -
 *   -
 *   .
 * <li> <b>Removed items</b>
 *   - ::CREATOR_STACK_DISCOVERY_CACHE
 *   - ::CREATOR_STACK_APS_INDIRECT_BINDING_TABLE
 *   .
 * </ul>
 * HIDDEN -->
 */
