// File: tmsp-enum.h
//
// *** Generated file. Do not edit! ***
//
// Description: Enumerations for TMSP.
//
// Copyright 2017 Silicon Laboratories, Inc.                                *80*

#ifndef __TMSP_ENUM_H__
#define __TMSP_ENUM_H__

#define TMSP_VERSION 0x0E00
//------------------------------------------------------------------------------
typedef uint8_t TmspMfglibActivities;

enum {
  TMSP_MFGLIB_TONE                              = 0x00,
  TMSP_MFGLIB_STREAM                            = 0x01
};
#define TMSP_MFGLIB_ACTIVITIES_MAX  0x01

//------------------------------------------------------------------------------
typedef uint8_t TmspMfglibValues;

enum {
  TMSP_MFGLIB_CHANNEL                           = 0x00,
  TMSP_MFGLIB_POWER                             = 0x01,
  TMSP_MFGLIB_POWER_MODE                        = 0x02,
  TMSP_MFGLIB_SYN_OFFSET                        = 0x03,
  TMSP_MFGLIB_OPTIONS                           = 0x04
};
#define TMSP_MFGLIB_VALUES_MAX  0x04

//------------------------------------------------------------------------------
typedef uint8_t SetLargeValueId;

enum {
  SET_ND_DATA                                   = 0x00,
  SET_LOCAL_NETWORK_DATA                        = 0x01
};
#define SET_LARGE_VALUE_ID_MAX  0x01

//------------------------------------------------------------------------------
// Frame IDs

enum {
// Core

  EMBER_RESET_MICRO_COMMAND_IDENTIFIER                                 = 0x6900,
  EMBER_RESET_NETWORK_STATE_COMMAND_IDENTIFIER                         = 0x6901,
  EMBER_INIT_HOST_COMMAND_IDENTIFIER                                   = 0x6902,
  EMBER_STATE_COMMAND_IDENTIFIER                                       = 0x6903,
  EMBER_GET_VERSIONS_COMMAND_IDENTIFIER                                = 0x6904,
  EMBER_FORM_NETWORK_COMMAND_IDENTIFIER                                = 0x6905,
  EMBER_JOIN_NETWORK_COMMAND_IDENTIFIER                                = 0x6906,
  EMBER_RESUME_NETWORK_COMMAND_IDENTIFIER                              = 0x6907,
  EMBER_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER                           = 0x6908,
  EMBER_HOST_JOIN_CLIENT_COMPLETE_COMMAND_IDENTIFIER                   = 0x690C,
  EMBER_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER                     = 0x690D,
  EMBER_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER                  = 0x690F,
  EMBER_START_SCAN_COMMAND_IDENTIFIER                                  = 0x6913,
  EMBER_STOP_SCAN_COMMAND_IDENTIFIER                                   = 0x6915,
  EMBER_GET_RIP_ENTRY_COMMAND_IDENTIFIER                               = 0x6917,
  EMBER_GET_MULTICAST_TABLE_COMMAND_IDENTIFIER                         = 0x6918,
  EMBER_GET_COUNTER_COMMAND_IDENTIFIER                                 = 0x6919,
  EMBER_CLEAR_COUNTERS_COMMAND_IDENTIFIER                              = 0x691A,
  EMBER_SET_TX_POWER_MODE_COMMAND_IDENTIFIER                           = 0x691C,
  EMBER_GET_TX_POWER_MODE_COMMAND_IDENTIFIER                           = 0x691D,
  EMBER_FF_WAKEUP_COMMAND_IDENTIFIER                                   = 0x6924,
  EMBER_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER                           = 0x6925,
  EMBER_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER                           = 0x6926,
  EMBER_GET_RADIO_POWER_COMMAND_IDENTIFIER                             = 0x6927,
  EMBER_SET_RADIO_POWER_COMMAND_IDENTIFIER                             = 0x6928,
  EMBER_ECHO_COMMAND_IDENTIFIER                                        = 0x692A,
  EMBER_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER                           = 0x6944,
  EMBER_GET_INDEXED_TOKEN_COMMAND_IDENTIFIER                           = 0x6945,
  EMBER_POLL_FOR_DATA_COMMAND_IDENTIFIER                               = 0x6946,
  EMBER_DEEP_SLEEP_COMMAND_IDENTIFIER                                  = 0x6947,
  EMBER_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER                         = 0x6948,
  EMBER_OK_TO_NAP_COMMAND_IDENTIFIER                                   = 0x6949,
  EMBER_PING_COMMAND_IDENTIFIER                                        = 0x694F,
  EMBER_JOIN_COMMISSIONED_COMMAND_IDENTIFIER                           = 0x6951,
  EMBER_COMMISSION_NETWORK_COMMAND_IDENTIFIER                          = 0x6952,
  EMBER_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER                        = 0x6956,
  EMBER_HOST_TO_NCP_NO_OP_COMMAND_IDENTIFIER                           = 0x6958,
  EMBER_GET_GLOBAL_ADDRESSES_COMMAND_IDENTIFIER                        = 0x6959,
  EMBER_GET_DHCP_CLIENTS_COMMAND_IDENTIFIER                            = 0x6962,
  EMBER_ADD_STEERING_EUI64_COMMAND_IDENTIFIER                          = 0x6965,
  EMBER_BECOME_COMMISSIONER_COMMAND_IDENTIFIER                         = 0x6966,
  EMBER_GET_COMMISSIONER_COMMAND_IDENTIFIER                            = 0x6967,
  EMBER_SEND_STEERING_DATA_COMMAND_IDENTIFIER                          = 0x6968,
  EMBER_STOP_COMMISSIONING_COMMAND_IDENTIFIER                          = 0x6969,
  EMBER_SET_JOIN_KEY_COMMAND_IDENTIFIER                                = 0x696A,
  EMBER_SET_JOINING_MODE_COMMAND_IDENTIFIER                            = 0x696B,
  EMBER_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER                            = 0x696E,
  EMBER_GET_GLOBAL_PREFIXES_COMMAND_IDENTIFIER                         = 0x696F,
  EMBER_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER                       = 0x6970,
  EMBER_SET_EUI64_COMMAND_IDENTIFIER                                   = 0x6971,
  EMBER_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER                       = 0x6972,
  EMBER_CUSTOM_HOST_TO_NCP_MESSAGE_COMMAND_IDENTIFIER                  = 0x6973,
  EMBER_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER                        = 0x6977,
  EMBER_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER                         = 0x6978,
  EMBER_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER               = 0x6979,
  EMBER_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER                    = 0x697a,
  EMBER_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER                   = 0x697b,
  EMBER_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER                        = 0x697c,
  EMBER_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER              = 0x697d,
  EMBER_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER                = 0x697e,
  EMBER_GET_MFG_TOKEN_COMMAND_IDENTIFIER                               = 0x697f,
  EMBER_SET_MFG_TOKEN_COMMAND_IDENTIFIER                               = 0x6980,
  EMBER_ENABLE_HOST_DTLS_CLIENT_COMMAND_IDENTIFIER                     = 0x6981,
  EMBER_GET_CTUNE_COMMAND_IDENTIFIER                                   = 0x6982,
  EMBER_SET_CTUNE_COMMAND_IDENTIFIER                                   = 0x6983,
  EMBER_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER                          = 0x6987,
  EMBER_GET_PTA_ENABLE_COMMAND_IDENTIFIER                              = 0x6988,
  EMBER_SET_PTA_ENABLE_COMMAND_IDENTIFIER                              = 0x6989,
  EMBER_GET_ANTENNA_MODE_COMMAND_IDENTIFIER                            = 0x698A,
  EMBER_SET_ANTENNA_MODE_COMMAND_IDENTIFIER                            = 0x698B,
  EMBER_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER                    = 0x698C,
  EMBER_GET_PTA_OPTIONS_COMMAND_IDENTIFIER                             = 0x698D,
  EMBER_SET_PTA_OPTIONS_COMMAND_IDENTIFIER                             = 0x698E,
  EMBER_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER                        = 0x698F,
  EMBER_NOTE_EXTERNAL_COMMISSIONER_COMMAND_IDENTIFIER                  = 0x6990,
  EMBER_NCP_SET_ND_DATA_COMMAND_IDENTIFIER                             = 0x6991,
  EMBER_NCP_SET_LARGE_DATA_COMMAND_IDENTIFIER                          = 0x6992,
  CB_RESET_MICRO_COMMAND_IDENTIFIER                                    = 0x6300,
  CB_STATE_COMMAND_IDENTIFIER                                          = 0x6301,
  CB_GET_VERSIONS_COMMAND_IDENTIFIER                                   = 0x6302,
  CB_GET_RIP_ENTRY_COMMAND_IDENTIFIER                                  = 0x6303,
  CB_INIT_COMMAND_IDENTIFIER                                           = 0x6304,
  CB_GET_COUNTER_COMMAND_IDENTIFIER                                    = 0x6305,
  CB_SET_SECURITY_PARAMETERS_COMMAND_IDENTIFIER                        = 0x6306,
  CB_SWITCH_TO_NEXT_NETWORK_KEY_COMMAND_IDENTIFIER                     = 0x6308,
  CB_FORM_NETWORK_COMMAND_IDENTIFIER                                   = 0x630A,
  CB_JOIN_NETWORK_COMMAND_IDENTIFIER                                   = 0x630B,
  CB_RESUME_NETWORK_COMMAND_IDENTIFIER                                 = 0x630C,
  CB_ATTACH_TO_NETWORK_COMMAND_IDENTIFIER                              = 0x6365,
  CB_ENERGY_SCAN_COMMAND_IDENTIFIER                                    = 0x6310,
  CB_ACTIVE_SCAN_COMMAND_IDENTIFIER                                    = 0x6311,
  CB_SCAN_COMMAND_IDENTIFIER                                           = 0x6312,
  CB_SET_ADDRESS_COMMAND_IDENTIFIER                                    = 0x6313,
  CB_SET_DRIVER_ADDRESS_COMMAND_IDENTIFIER                             = 0x6314,
  CB_START_HOST_JOIN_CLIENT_COMMAND_IDENTIFIER                         = 0x6315,
  CB_SET_TX_POWER_MODE_COMMAND_IDENTIFIER                              = 0x6316,
  CB_GET_TX_POWER_MODE_COMMAND_IDENTIFIER                              = 0x6317,
  CB_SET_NETWORK_KEYS_COMMAND_IDENTIFIER                               = 0x6318,
  CB_GET_MULTICAST_ENTRY_COMMAND_IDENTIFIER                            = 0x631E,
  CB_GET_CCA_THRESHOLD_COMMAND_IDENTIFIER                              = 0x6321,
  CB_SET_CCA_THRESHOLD_COMMAND_IDENTIFIER                              = 0x6322,
  CB_GET_RADIO_POWER_COMMAND_IDENTIFIER                                = 0x6323,
  CB_SET_RADIO_POWER_COMMAND_IDENTIFIER                                = 0x6324,
  CB_ECHO_COMMAND_IDENTIFIER                                           = 0x6326,
  CB_ASSERT_INFO_COMMAND_IDENTIFIER                                    = 0x632D,
  CB_CONFIGURE_GATEWAY_COMMAND_IDENTIFIER                              = 0x6340,
  CB_GET_CHANNEL_CAL_DATA_TOKEN_COMMAND_IDENTIFIER                     = 0x6341,
  CB_POLL_FOR_DATA_COMMAND_IDENTIFIER                                  = 0x6342,
  CB_DEEP_SLEEP_COMMAND_IDENTIFIER                                     = 0x6343,
  CB_STACK_POLL_FOR_DATA_COMMAND_IDENTIFIER                            = 0x6344,
  CB_OK_TO_NAP_COMMAND_IDENTIFIER                                      = 0x6345,
  CB_DEEP_SLEEP_COMPLETE_COMMAND_IDENTIFIER                            = 0x6346,
  CB_RESET_NETWORK_STATE_COMMAND_IDENTIFIER                            = 0x634C,
  CB_EXTERNAL_ROUTE_CHANGE_COMMAND_IDENTIFIER                          = 0x6350,
  CB_DHCP_SERVER_CHANGE_COMMAND_IDENTIFIER                             = 0x6351,
  CB_NCP_NETWORK_DATA_CHANGE_COMMAND_IDENTIFIER                        = 0x6389,
  CB_NCP_GET_NETWORK_DATA_COMMAND_IDENTIFIER                           = 0x6388,
  CB_ADDRESS_CONFIGURATION_CHANGE_COMMAND_IDENTIFIER                   = 0x6352,
  CB_REQUEST_DHCP_ADDRESS_COMMAND_IDENTIFIER                           = 0x6353,
  CB_COMMISSION_NETWORK_COMMAND_IDENTIFIER                             = 0x6355,
  CB_GET_GLOBAL_ADDRESS_COMMAND_IDENTIFIER                             = 0x6356,
  CB_GET_DHCP_CLIENT_COMMAND_IDENTIFIER                                = 0x6358,
  CB_BECOME_COMMISSIONER_COMMAND_IDENTIFIER                            = 0x6359,
  CB_SEND_STEERING_DATA_COMMAND_IDENTIFIER                             = 0x635A,
  CB_CHANGE_NODE_TYPE_COMMAND_IDENTIFIER                               = 0x635D,
  CB_GET_GLOBAL_PREFIX_COMMAND_IDENTIFIER                              = 0x635E,
  CB_RESIGN_GLOBAL_ADDRESS_COMMAND_IDENTIFIER                          = 0x635F,
  CB_SLAAC_SERVER_CHANGE_COMMAND_IDENTIFIER                            = 0x6360,
  CB_REQUEST_SLAAC_ADDRESS_COMMAND_IDENTIFIER                          = 0x6361,
  CB_CUSTOM_NCP_TO_HOST_MESSAGE_COMMAND_IDENTIFIER                     = 0x6362,
  CB_NCP_TO_HOST_NO_OP_COMMAND_IDENTIFIER                              = 0x6363,
  CB_COMMISSIONER_STATUS_COMMAND_IDENTIFIER                            = 0x6364,
  CB_LEADER_DATA_COMMAND_IDENTIFIER                                    = 0x6366,
  CB_GET_NETWORK_DATA_TLV_COMMAND_IDENTIFIER                           = 0x6367,
  CB_GET_ROUTING_LOCATOR_COMMAND_IDENTIFIER                            = 0x6368,
  CB_SET_RANDOMIZE_MAC_EXTENDED_ID_COMMAND_IDENTIFIER                  = 0x6369,
  CB_CONFIGURE_EXTERNAL_ROUTE_COMMAND_IDENTIFIER                       = 0x636a,
  CB_ALLOW_NATIVE_COMMISSIONER_COMMAND_IDENTIFIER                      = 0x6370,
  CB_SET_COMMISSIONER_KEY_COMMAND_IDENTIFIER                           = 0x6371,
  CB_SET_COMM_PROXY_APP_PARAMETERS_COMMAND_IDENTIFIER                  = 0x6372,
  CB_SET_COMM_PROXY_APP_SECURITY_COMMAND_IDENTIFIER                    = 0x6373,
  CB_SET_COMM_PROXY_APP_ADDRESS_COMMAND_IDENTIFIER                     = 0x6374,
  CB_NETWORK_STATUS_COMMAND_IDENTIFIER                                 = 0x6375,
  CB_GET_STANDALONE_BOOTLOADER_INFO_COMMAND_IDENTIFIER                 = 0x6376,
  CB_LAUNCH_STANDALONE_BOOTLOADER_COMMAND_IDENTIFIER                   = 0x6377,
  CB_GET_MFG_TOKEN_COMMAND_IDENTIFIER                                  = 0x6378,
  CB_SET_MFG_TOKEN_COMMAND_IDENTIFIER                                  = 0x6379,
  CB_HOST_STATE_COMMAND_IDENTIFIER                                     = 0x637A,
  CB_GET_CTUNE_COMMAND_IDENTIFIER                                      = 0x637B,
  CB_SET_CTUNE_COMMAND_IDENTIFIER                                      = 0x637C,
  CB_SET_RADIO_HOLD_OFF_COMMAND_IDENTIFIER                             = 0x637F,
  CB_GET_PTA_ENABLE_COMMAND_IDENTIFIER                                 = 0x6380,
  CB_SET_PTA_ENABLE_COMMAND_IDENTIFIER                                 = 0x6381,
  CB_GET_ANTENNA_MODE_COMMAND_IDENTIFIER                               = 0x6382,
  CB_SET_ANTENNA_MODE_COMMAND_IDENTIFIER                               = 0x6383,
  CB_RADIO_GET_RANDOM_NUMBERS_COMMAND_IDENTIFIER                       = 0x6384,
  CB_GET_PTA_OPTIONS_COMMAND_IDENTIFIER                                = 0x6385,
  CB_SET_PTA_OPTIONS_COMMAND_IDENTIFIER                                = 0x6386,
  CB_SET_JOIN_KEY_COMMAND_IDENTIFIER                                   = 0x638A,
  CB_SET_ND_DATA_COMMAND_IDENTIFIER                                    = 0x638b,
  CB_SET_LOCAL_NETWORK_DATA_COMMAND_IDENTIFIER                         = 0x638c,
  CB_SET_COMM_PROXY_APP_PSKC_COMMAND_IDENTIFIER                        = 0x638d,
  CB_SET_PSKC_COMMAND_IDENTIFIER                                       = 0x638f,

// MFGLIB

  EMBER_MFGLIB_START_COMMAND_IDENTIFIER                                = 0x6938,
  EMBER_MFGLIB_END_COMMAND_IDENTIFIER                                  = 0x6939,
  EMBER_MFGLIB_START_ACTIVITY_COMMAND_IDENTIFIER                       = 0x693A,
  EMBER_MFGLIB_STOP_ACTIVITY_COMMAND_IDENTIFIER                        = 0x693B,
  EMBER_MFGLIB_SEND_PACKET_COMMAND_IDENTIFIER                          = 0x693C,
  EMBER_MFGLIB_SET_COMMAND_IDENTIFIER                                  = 0x693D,
  EMBER_MFGLIB_GET_COMMAND_IDENTIFIER                                  = 0x693E,
  EMBER_MFGLIB_TEST_CONT_MOD_CAL_COMMAND_IDENTIFIER                    = 0x693F,
  CB_MFGLIB_START_TEST_COMMAND_IDENTIFIER                              = 0x632F,
  CB_MFGLIB_RX_COMMAND_IDENTIFIER                                      = 0x6330,
  CB_MFGLIB_END_TEST_COMMAND_IDENTIFIER                                = 0x6331,
  CB_MFGLIB_START_COMMAND_IDENTIFIER                                   = 0x6332,
  CB_MFGLIB_STOP_COMMAND_IDENTIFIER                                    = 0x6333,
  CB_MFGLIB_SEND_PACKET_EVENT_COMMAND_IDENTIFIER                       = 0x6334,
  CB_MFGLIB_SET_COMMAND_IDENTIFIER                                     = 0x6335,
  CB_MFGLIB_GET_CHANNEL_COMMAND_IDENTIFIER                             = 0x6338,
  CB_MFGLIB_GET_POWER_COMMAND_IDENTIFIER                               = 0x6339,
  CB_MFGLIB_GET_POWER_MODE_COMMAND_IDENTIFIER                          = 0x633A,
  CB_MFGLIB_GET_SYN_OFFSET_COMMAND_IDENTIFIER                          = 0x633B,
  CB_MFGLIB_GET_OPTIONS_COMMAND_IDENTIFIER                             = 0x638E,

// EMBER_TEST

  EMBER_CONFIG_UART_COMMAND_IDENTIFIER                                 = 0x6929,
  EMBER_RESET_NCP_ASH_COMMAND_IDENTIFIER                               = 0x692B,
  EMBER_RESET_IP_DRIVER_ASH_COMMAND_IDENTIFIER                         = 0x692C,
  EMBER_START_UART_STORM_COMMAND_IDENTIFIER                            = 0x692F,
  EMBER_STOP_UART_STORM_COMMAND_IDENTIFIER                             = 0x6930,
  EMBER_SEND_DONE_COMMAND_IDENTIFIER                                   = 0x694A,
  CB_CONFIG_UART_COMMAND_IDENTIFIER                                    = 0x6325,
  CB_RESET_NCP_ASH_COMMAND_IDENTIFIER                                  = 0x6327,
  CB_START_UART_STORM_COMMAND_IDENTIFIER                               = 0x6328,
  CB_STOP_UART_STORM_COMMAND_IDENTIFIER                                = 0x6329,
  CB_SEND_DONE_COMMAND_IDENTIFIER                                      = 0x6347,

// QA_THREAD_TEST

  EMBER_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER                        = 0x6910,
  EMBER_RESET_NCP_GPIO_COMMAND_IDENTIFIER                              = 0x692D,
  EMBER_ENABLE_RESET_NCP_GPIO_COMMAND_IDENTIFIER                       = 0x692E,
  EMBER_FORCE_ASSERT_COMMAND_IDENTIFIER                                = 0x6937,
  EMBER_GET_NODE_STATUS_COMMAND_IDENTIFIER                             = 0x694C,
  EMBER_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER                            = 0x6953,
  EMBER_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER                         = 0x6954,
  EMBER_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER                         = 0x6955,
  EMBER_START_UART_SPEED_TEST_COMMAND_IDENTIFIER                       = 0x6957,
  EMBER_NCP_UDP_STORM_COMMAND_IDENTIFIER                               = 0x696D,
  CB_GET_NETWORK_KEY_INFO_COMMAND_IDENTIFIER                           = 0x6309,
  CB_GET_NODE_STATUS_COMMAND_IDENTIFIER                                = 0x6349,
  CB_ADD_ADDRESS_DATA_COMMAND_IDENTIFIER                               = 0x634D,
  CB_CLEAR_ADDRESS_CACHE_COMMAND_IDENTIFIER                            = 0x634E,
  CB_LOOKUP_ADDRESS_DATA_COMMAND_IDENTIFIER                            = 0x634F,
  CB_UART_SPEED_TEST_COMMAND_IDENTIFIER                                = 0x6354,
  CB_NCP_UDP_STORM_COMMAND_IDENTIFIER                                  = 0x635B,
  CB_NCP_UDP_STORM_COMPLETE_COMMAND_IDENTIFIER                         = 0x635C,

// APP_USES_SOFTWARE_FLOW_CONTROL

  EMBER_START_XON_XOFF_TEST_COMMAND_IDENTIFIER                         = 0x694D,
};

#endif // __TMSP_ENUM_H__
