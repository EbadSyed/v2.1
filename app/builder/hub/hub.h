// This file is generated by Simplicity Studio.  Please do not edit manually.
//
//

// Enclosing macro to prevent multiple inclusion
#ifndef SILABS_APP_HUB_H
#define SILABS_APP_HUB_H


/**** Included Header Section ****/

/**** ZCL Section ****/
#define ZA_PROMPT "hub"
#define ZCL_USING_BASIC_CLUSTER_CLIENT
#define ZCL_USING_BASIC_CLUSTER_SERVER
#define ZCL_USING_POWER_CONFIG_CLUSTER_CLIENT
#define ZCL_USING_IDENTIFY_CLUSTER_CLIENT
#define ZCL_USING_IDENTIFY_CLUSTER_SERVER
#define ZCL_USING_GROUPS_CLUSTER_CLIENT
#define ZCL_USING_SCENES_CLUSTER_CLIENT
#define ZCL_USING_ON_OFF_CLUSTER_CLIENT
#define ZCL_USING_ON_OFF_CLUSTER_SERVER
#define ZCL_USING_ON_OFF_SWITCH_CONFIG_CLUSTER_CLIENT
#define ZCL_USING_LEVEL_CONTROL_CLUSTER_CLIENT
#define ZCL_USING_LEVEL_CONTROL_CLUSTER_SERVER
#define ZCL_USING_TIME_CLUSTER_SERVER
#define ZCL_USING_OTA_BOOTLOAD_CLUSTER_SERVER
#define ZCL_USING_POWER_PROFILE_CLUSTER_CLIENT
#define ZCL_USING_POLL_CONTROL_CLUSTER_CLIENT
#define ZCL_USING_GREEN_POWER_CLUSTER_CLIENT
#define ZCL_USING_GREEN_POWER_CLUSTER_SERVER
#define ZCL_USING_SHADE_CONFIG_CLUSTER_CLIENT
#define ZCL_USING_DOOR_LOCK_CLUSTER_CLIENT
#define ZCL_USING_WINDOW_COVERING_CLUSTER_CLIENT
#define ZCL_USING_PUMP_CONFIG_CONTROL_CLUSTER_CLIENT
#define ZCL_USING_THERMOSTAT_CLUSTER_CLIENT
#define ZCL_USING_FAN_CONTROL_CLUSTER_CLIENT
#define ZCL_USING_COLOR_CONTROL_CLUSTER_CLIENT
#define ZCL_USING_COLOR_CONTROL_CLUSTER_SERVER
#define ZCL_USING_ILLUM_MEASUREMENT_CLUSTER_CLIENT
#define ZCL_USING_TEMP_MEASUREMENT_CLUSTER_CLIENT
#define ZCL_USING_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_CLIENT
#define ZCL_USING_OCCUPANCY_SENSING_CLUSTER_CLIENT
#define ZCL_USING_IAS_ZONE_CLUSTER_CLIENT
#define ZCL_USING_SIMPLE_METERING_CLUSTER_CLIENT
#define ZCL_USING_METER_IDENTIFICATION_CLUSTER_CLIENT
#define ZCL_USING_APPLIANCE_STATISTICS_CLUSTER_CLIENT
#define ZCL_USING_ELECTRICAL_MEASUREMENT_CLUSTER_CLIENT
#define ZCL_USING_DIAGNOSTICS_CLUSTER_CLIENT
#define ZCL_USING_OTA_CONFIGURATION_CLUSTER_CLIENT
#define ZCL_USING_MFGLIB_CLUSTER_CLIENT
/**** Optional Attributes ****/
#define ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE 
#define ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE 
#define ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_REMAINING_TIME_ATTRIBUTE 
#define ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_COLOR_TEMPERATURE_ATTRIBUTE 
#define ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_COLOR_MODE_ATTRIBUTE 
#define ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_NUMBER_OF_PRIMARIES_ATTRIBUTE 
#define ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_ENHANCED_COLOR_MODE_ATTRIBUTE 
#define ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_COLOR_CAPABILITIES_ATTRIBUTE 
#define ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MIN_ATTRIBUTE 
#define ZCL_USING_COLOR_CONTROL_CLUSTER_COLOR_CONTROL_COLOR_TEMP_PHYSICAL_MAX_ATTRIBUTE 
#define EMBER_AF_MANUFACTURER_CODE 0x1002
#define EMBER_AF_DEFAULT_RESPONSE_POLICY_CONDITIONAL

/**** Cluster endpoint counts ****/
#define EMBER_AF_BASIC_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_BASIC_CLUSTER_SERVER_ENDPOINT_COUNT (1)
#define EMBER_AF_POWER_CONFIG_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_IDENTIFY_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_IDENTIFY_CLUSTER_SERVER_ENDPOINT_COUNT (1)
#define EMBER_AF_GROUPS_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_SCENES_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_ON_OFF_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_ON_OFF_CLUSTER_SERVER_ENDPOINT_COUNT (1)
#define EMBER_AF_ON_OFF_SWITCH_CONFIG_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_LEVEL_CONTROL_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_LEVEL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT (1)
#define EMBER_AF_TIME_CLUSTER_SERVER_ENDPOINT_COUNT (1)
#define EMBER_AF_OTA_BOOTLOAD_CLUSTER_SERVER_ENDPOINT_COUNT (1)
#define EMBER_AF_POWER_PROFILE_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_POLL_CONTROL_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_GREEN_POWER_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_GREEN_POWER_CLUSTER_SERVER_ENDPOINT_COUNT (1)
#define EMBER_AF_SHADE_CONFIG_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_DOOR_LOCK_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_WINDOW_COVERING_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_PUMP_CONFIG_CONTROL_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_THERMOSTAT_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_FAN_CONTROL_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_COLOR_CONTROL_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT (1)
#define EMBER_AF_ILLUM_MEASUREMENT_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_TEMP_MEASUREMENT_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_OCCUPANCY_SENSING_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_IAS_ZONE_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_SIMPLE_METERING_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_METER_IDENTIFICATION_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_APPLIANCE_STATISTICS_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_ELECTRICAL_MEASUREMENT_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_DIAGNOSTICS_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_OTA_CONFIGURATION_CLUSTER_CLIENT_ENDPOINT_COUNT (1)
#define EMBER_AF_MFGLIB_CLUSTER_CLIENT_ENDPOINT_COUNT (1)

/**** CLI Section ****/
#define EMBER_AF_GENERATE_CLI
#define EMBER_AF_ENABLE_CUSTOM_COMMANDS
#define EMBER_COMMAND_INTEPRETER_HAS_DESCRIPTION_FIELD

/**** Security Section ****/
#define EMBER_AF_HAS_SECURITY_PROFILE_Z3

/**** Network Section ****/
#define EMBER_SUPPORTED_NETWORKS (1)
#define EMBER_AF_NETWORK_INDEX_PRIMARY (0)
#define EMBER_AF_DEFAULT_NETWORK_INDEX EMBER_AF_NETWORK_INDEX_PRIMARY
#define EMBER_AF_HAS_COORDINATOR_NETWORK
#define EMBER_AF_HAS_ROUTER_NETWORK
#define EMBER_AF_HAS_RX_ON_WHEN_IDLE_NETWORK
#define EMBER_AF_TX_POWER_MODE EMBER_TX_POWER_MODE_USE_TOKEN
#define EMBER_AF_ENABLE_TX_ZDO

/**** HAL Section ****/
#define ZA_CLI_FULL

/**** Callback Section ****/
#define EMBER_CALLBACK_BASIC_CLUSTER_RESET_TO_FACTORY_DEFAULTS
#define EMBER_CALLBACK_COLOR_CONTROL_CLUSTER_STOP_MOVE_STEP
#define EMBER_CALLBACK_COLOR_CONTROL_CLUSTER_MOVE_TO_COLOR
#define EMBER_CALLBACK_COLOR_CONTROL_CLUSTER_MOVE_COLOR
#define EMBER_CALLBACK_COLOR_CONTROL_CLUSTER_STEP_COLOR
#define EMBER_CALLBACK_COLOR_CONTROL_CLUSTER_MOVE_TO_COLOR_TEMPERATURE
#define EMBER_CALLBACK_COLOR_CONTROL_CLUSTER_MOVE_COLOR_TEMPERATURE
#define EMBER_CALLBACK_COLOR_CONTROL_CLUSTER_STEP_COLOR_TEMPERATUE
#define EMBER_CALLBACK_COLOR_CONTROL_CLUSTER_MOVE_TO_HUE_AND_SATURATION
#define EMBER_CALLBACK_COLOR_CONTROL_CLUSTER_MOVE_HUE
#define EMBER_CALLBACK_COLOR_CONTROL_CLUSTER_MOVE_SATURATION
#define EMBER_CALLBACK_COLOR_CONTROL_CLUSTER_MOVE_TO_HUE
#define EMBER_CALLBACK_COLOR_CONTROL_CLUSTER_MOVE_TO_SATURATION
#define EMBER_CALLBACK_COLOR_CONTROL_CLUSTER_STEP_HUE
#define EMBER_CALLBACK_COLOR_CONTROL_CLUSTER_STEP_SATURATION
#define EMBER_CALLBACK_INCOMING_ROUTE_ERROR_HANDLER
#define EMBER_APPLICATION_HAS_INCOMING_ROUTE_ERROR_HANDLER
#define EMBER_CALLBACK_EZSP_INCOMING_ROUTE_ERROR_HANDLER
#define EZSP_APPLICATION_HAS_INCOMING_ROUTE_ERROR_HANDLER
#define EMBER_CALLBACK_GET_SOURCE_ROUTE_OVERHEAD
#define EMBER_CALLBACK_SET_SOURCE_ROUTE_OVERHEAD
#define EMBER_CALLBACK_COUNTER_HANDLER
#define EMBER_APPLICATION_HAS_COUNTER_HANDLER
#define EMBER_CALLBACK_EZSP_COUNTER_ROLLOVER_HANDLER
#define EZSP_APPLICATION_HAS_COUNTER_ROLLOVER_HANDLER
#define EMBER_CALLBACK_TRUST_CENTER_JOIN
#define EMBER_CALLBACK_IDENTIFY_CLUSTER_IDENTIFY_QUERY_RESPONSE
#define EMBER_CALLBACK_MAIN_START
#define EMBER_CALLBACK_READ_ATTRIBUTES_RESPONSE
#define EMBER_CALLBACK_REPORT_ATTRIBUTES_RESPONSE
#define EMBER_CALLBACK_CONFIGURE_REPORTING_RESPONSE
#define EMBER_CALLBACK_READ_REPORTING_CONFIGURATION_RESPONSE
#define EMBER_CALLBACK_MSG_SENT
#define EMBER_CALLBACK_PRE_MSG
#define EMBER_CALLBACK_PRE_MESSAGE_SEND
#define EMBER_CALLBACK_GREEN_POWER_CLUSTER_GP_NOTIFICATION_RESPONSE
#define EMBER_CALLBACK_GREEN_POWER_CLUSTER_GP_PAIRING
#define EMBER_CALLBACK_GREEN_POWER_CLUSTER_GP_PROXY_COMMISSIONING_MODE
#define EMBER_CALLBACK_GREEN_POWER_CLUSTER_GP_RESPONSE
#define EMBER_CALLBACK_GREEN_POWER_CLUSTER_GP_SINK_TABLE_RESPONSE
#define EMBER_CALLBACK_GREEN_POWER_CLUSTER_GP_PROXY_TABLE_REQUEST
#define EMBER_CALLBACK_IAS_ZONE_CLUSTER_IAS_ZONE_CLUSTER_CLIENT_INIT
#define EMBER_CALLBACK_IAS_ZONE_CLUSTER_ZONE_ENROLL_REQUEST
#define EMBER_CALLBACK_IAS_ZONE_CLUSTER_ZONE_STATUS_CHANGE_NOTIFICATION
#define EMBER_CALLBACK_IDENTIFY_CLUSTER_IDENTIFY_CLUSTER_SERVER_INIT
#define EMBER_CALLBACK_IDENTIFY_CLUSTER_IDENTIFY_CLUSTER_SERVER_TICK
#define EMBER_CALLBACK_IDENTIFY_CLUSTER_IDENTIFY_CLUSTER_SERVER_ATTRIBUTE_CHANGED
#define EMBER_CALLBACK_IDENTIFY_CLUSTER_IDENTIFY
#define EMBER_CALLBACK_IDENTIFY_CLUSTER_IDENTIFY_QUERY
#define EMBER_CALLBACK_LEVEL_CONTROL_CLUSTER_LEVEL_CONTROL_CLUSTER_SERVER_TICK
#define EMBER_CALLBACK_LEVEL_CONTROL_CLUSTER_MOVE_TO_LEVEL
#define EMBER_CALLBACK_LEVEL_CONTROL_CLUSTER_MOVE_TO_LEVEL_WITH_ON_OFF
#define EMBER_CALLBACK_LEVEL_CONTROL_CLUSTER_MOVE
#define EMBER_CALLBACK_LEVEL_CONTROL_CLUSTER_MOVE_WITH_ON_OFF
#define EMBER_CALLBACK_LEVEL_CONTROL_CLUSTER_STEP
#define EMBER_CALLBACK_LEVEL_CONTROL_CLUSTER_STEP_WITH_ON_OFF
#define EMBER_CALLBACK_LEVEL_CONTROL_CLUSTER_STOP
#define EMBER_CALLBACK_LEVEL_CONTROL_CLUSTER_STOP_WITH_ON_OFF
#define EMBER_CALLBACK_ON_OFF_CLUSTER_ON_OFF_CLUSTER_LEVEL_CONTROL_EFFECT
#define EMBER_CALLBACK_UNUSED_PAN_ID_FOUND
#define EMBER_CALLBACK_SCAN_ERROR
#define EMBER_CALLBACK_FIND_UNUSED_PAN_ID_AND_FORM
#define EMBER_CALLBACK_START_SEARCH_FOR_JOINABLE_NETWORK
#define EMBER_CALLBACK_GET_FORM_AND_JOIN_EXTENDED_PAN_ID
#define EMBER_CALLBACK_SET_FORM_AND_JOIN_EXTENDED_PAN_ID
#define EMBER_CALLBACK_ON_OFF_CLUSTER_OFF
#define EMBER_CALLBACK_ON_OFF_CLUSTER_ON
#define EMBER_CALLBACK_ON_OFF_CLUSTER_TOGGLE
#define EMBER_CALLBACK_ON_OFF_CLUSTER_ON_OFF_CLUSTER_SET_VALUE
#define EMBER_CALLBACK_OTA_BOOTLOAD_CLUSTER_OTA_BOOTLOAD_CLUSTER_SERVER_INIT
#define EMBER_CALLBACK_OTA_BOOTLOAD_CLUSTER_OTA_BOOTLOAD_CLUSTER_SERVER_TICK
#define EMBER_CALLBACK_OTA_SERVER_INCOMING_MESSAGE_RAW
#define EMBER_CALLBACK_OTA_SERVER_SEND_IMAGE_NOTIFY
#define EMBER_CALLBACK_OTA_SERVER_QUERY
#define EMBER_CALLBACK_OTA_SERVER_BLOCK_SIZE
#define EMBER_CALLBACK_OTA_SERVER_UPGRADE_END_REQUEST
#define EMBER_CALLBACK_OTA_PAGE_REQUEST_SERVER_POLICY
#define EMBER_CALLBACK_OTA_STORAGE_INIT
#define EMBER_CALLBACK_OTA_STORAGE_GET_COUNT
#define EMBER_CALLBACK_OTA_STORAGE_SEARCH
#define EMBER_CALLBACK_OTA_STORAGE_ITERATOR_FIRST
#define EMBER_CALLBACK_OTA_STORAGE_ITERATOR_NEXT
#define EMBER_CALLBACK_OTA_STORAGE_CLEAR_TEMP_DATA
#define EMBER_CALLBACK_OTA_STORAGE_WRITE_TEMP_DATA
#define EMBER_CALLBACK_OTA_STORAGE_GET_FULL_HEADER
#define EMBER_CALLBACK_OTA_STORAGE_GET_TOTAL_IMAGE_SIZE
#define EMBER_CALLBACK_OTA_STORAGE_READ_IMAGE_DATA
#define EMBER_CALLBACK_OTA_STORAGE_CHECK_TEMP_DATA
#define EMBER_CALLBACK_OTA_STORAGE_FINISH_DOWNLOAD
#define EMBER_CALLBACK_OTA_STORAGE_DRIVER_PREPARE_TO_RESUME_DOWNLOAD
#define EMBER_CALLBACK_POLL_CONTROL_CLUSTER_CHECK_IN
#define EMBER_CALLBACK_CONFIGURE_REPORTING_COMMAND
#define EMBER_CALLBACK_READ_REPORTING_CONFIGURATION_COMMAND
#define EMBER_CALLBACK_CLEAR_REPORT_TABLE
#define EMBER_CALLBACK_REPORTING_ATTRIBUTE_CHANGE
#define EMBER_CALLBACK_ENERGY_SCAN_RESULT
#define EMBER_CALLBACK_SCAN_COMPLETE
#define EMBER_CALLBACK_NETWORK_FOUND
#define EMBER_CALLBACK_SIMPLE_METERING_CLUSTER_GET_PROFILE_RESPONSE
#define EMBER_CALLBACK_SIMPLE_METERING_CLUSTER_REQUEST_MIRROR
#define EMBER_CALLBACK_SIMPLE_METERING_CLUSTER_REMOVE_MIRROR
#define EMBER_CALLBACK_SIMPLE_METERING_CLUSTER_REQUEST_FAST_POLL_MODE_RESPONSE
#define EMBER_CALLBACK_SIMPLE_METERING_CLUSTER_SIMPLE_METERING_CLUSTER_CLIENT_DEFAULT_RESPONSE
#define EMBER_CALLBACK_SIMPLE_METERING_CLUSTER_SUPPLY_STATUS_RESPONSE
#define EMBER_CALLBACK_CLUSTER_SECURITY_CUSTOM
#define EMBER_CALLBACK_TIME_CLUSTER_TIME_CLUSTER_SERVER_INIT
#define EMBER_CALLBACK_TIME_CLUSTER_TIME_CLUSTER_SERVER_TICK
/**** Debug printing section ****/

// Global switch
#define EMBER_AF_PRINT_ENABLE
// Individual areas
#define EMBER_AF_PRINT_CORE 0x0001
#define EMBER_AF_PRINT_APP 0x0002
#define EMBER_AF_PRINT_ATTRIBUTES 0x0004
#define EMBER_AF_PRINT_OTA_BOOTLOAD_CLUSTER 0x0008
#define EMBER_AF_PRINT_BITS { 0x0F }
#define EMBER_AF_PRINT_NAMES { \
  "Core",\
  "Application",\
  "Attributes",\
  "Over the Air Bootloading",\
  NULL\
}
#define EMBER_AF_PRINT_NAME_NUMBER 4


#define EMBER_AF_SUPPORT_COMMAND_DISCOVERY


// Generated plugin macros

// Use this macro to check if cJSON plugin is included
#define EMBER_AF_PLUGIN_CJSON

// Use this macro to check if Paho MQTT plugin is included
#define EMBER_AF_PLUGIN____________

// Use this macro to check if Linked List plugin is included
#define EMBER_AF_PLUGIN_LINKED_LIST

// Use this macro to check if Gateway MQTT Transport plugin is included
#define EMBER_AF_PLUGIN_TRANSPORT_MQTT
// User options for plugin Gateway MQTT Transport
#define EMBER_AF_PLUGIN_TRANSPORT_MQTT_BROKER_ADDRESS "tcp://localhost:1883"
#define EMBER_AF_PLUGIN_TRANSPORT_MQTT_BROKER_ADDRESS_LENGTH (20)
#define EMBER_AF_PLUGIN_TRANSPORT_MQTT_QOS QO_S2_EXACTLY_ONCE
#define EMBER_AF_PLUGIN_TRANSPORT_MQTT_CLIENT_ID_PREFIX "gw"
#define EMBER_AF_PLUGIN_TRANSPORT_MQTT_CLIENT_ID_PREFIX_LENGTH (2)

// Use this macro to check if Address Table plugin is included
#define EMBER_AF_PLUGIN_ADDRESS_TABLE
// User options for plugin Address Table
#define EMBER_AF_PLUGIN_ADDRESS_TABLE_SIZE 2
#define EMBER_AF_PLUGIN_ADDRESS_TABLE_TRUST_CENTER_CACHE_SIZE 2

// Use this macro to check if Basic Server Cluster plugin is included
#define EMBER_AF_PLUGIN_BASIC

// Use this macro to check if Color Control Cluster Server plugin is included
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER
// User options for plugin Color Control Cluster Server
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_XY
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_TEMP
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HSV
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_MIN_REPORT_INTERVAL 1
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_MAX_REPORT_INTERVAL 65535
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_COLOR_XY_CHANGE 1
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_COLOR_TEMP_CHANGE 1
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_HUE_CHANGE 1
#define EMBER_AF_PLUGIN_COLOR_CONTROL_SERVER_SATURATION_CHANGE 1

// Use this macro to check if Command Relay plugin is included
#define EMBER_AF_PLUGIN_COMMAND_RELAY
// User options for plugin Command Relay
#define EMBER_AF_PLUGIN_COMMAND_RELAY_RELAY_TABLE_SIZE 200

// Use this macro to check if Concentrator Support plugin is included
#define EMBER_AF_PLUGIN_CONCENTRATOR
#define EMBER_APPLICATION_HAS_SOURCE_ROUTING
#define EZSP_APPLICATION_HAS_ROUTE_RECORD_HANDLER
// User options for plugin Concentrator Support
#define EMBER_AF_PLUGIN_CONCENTRATOR_CONCENTRATOR_TYPE HIGH_RAM_CONCENTRATOR
#define EMBER_SOURCE_ROUTE_TABLE_SIZE 250
#define EZSP_HOST_SOURCE_ROUTE_TABLE_SIZE 250
#define EMBER_AF_PLUGIN_CONCENTRATOR_MIN_TIME_BETWEEN_BROADCASTS_SECONDS 5
#define EMBER_AF_PLUGIN_CONCENTRATOR_MAX_TIME_BETWEEN_BROADCASTS_SECONDS 60
#define EMBER_AF_PLUGIN_CONCENTRATOR_ROUTE_ERROR_THRESHOLD 3
#define EMBER_AF_PLUGIN_CONCENTRATOR_DELIVERY_FAILURE_THRESHOLD 3
#define EMBER_AF_PLUGIN_CONCENTRATOR_MAX_HOPS 0
#define EMBER_AF_PLUGIN_CONCENTRATOR_NCP_SUPPORT
#define EMBER_AF_PLUGIN_CONCENTRATOR_DEFAULT_ROUTER_BEHAVIOR FULL

// Use this macro to check if Counters plugin is included
#define EMBER_AF_PLUGIN_COUNTERS
// User options for plugin Counters

// Use this macro to check if Device Table plugin is included
#define EMBER_AF_PLUGIN_DEVICE_TABLE

// Use this macro to check if EZ-Mode Commissioning plugin is included
#define EMBER_AF_PLUGIN_EZMODE_COMMISSIONING
// User options for plugin EZ-Mode Commissioning
#define EMBER_AF_PLUGIN_EZMODE_COMMISSIONING_IDENTIFY_TIMEOUT 180

// Use this macro to check if Form and Join Library plugin is included
#define EMBER_AF_PLUGIN_FORM_AND_JOIN

// Use this macro to check if Green Power Client plugin is included
#define EMBER_AF_PLUGIN_GREEN_POWER_CLIENT
#define EZSP_APPLICATION_HAS_GPEP_INCOMING_MESSAGE_HANDLER
#define EZSP_APPLICATION_HAS_DGP_SENT_HANDLER
// User options for plugin Green Power Client
#define EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_GPP_COMMISSIONING_WINDOW 160
#define EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_GPP_DUPLICATE_TIMEOUT_SEC 160
#define EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_ADDR_ENTRIES 3
#define EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR 4

// Use this macro to check if Green Power Common plugin is included
#define EMBER_AF_PLUGIN_GREEN_POWER_COMMON

// Use this macro to check if IAS Zone Client plugin is included
#define EMBER_AF_PLUGIN_IAS_ZONE_CLIENT
// User options for plugin IAS Zone Client
#define EMBER_AF_PLUGIN_IAS_ZONE_CLIENT_MAX_DEVICES 40

// Use this macro to check if Identify Cluster plugin is included
#define EMBER_AF_PLUGIN_IDENTIFY

// Use this macro to check if Identify Feedback plugin is included
#define EMBER_AF_PLUGIN_IDENTIFY_FEEDBACK
// User options for plugin Identify Feedback
#define EMBER_AF_PLUGIN_IDENTIFY_FEEDBACK_LED_FEEDBACK

// Use this macro to check if Level Control Server Cluster plugin is included
#define EMBER_AF_PLUGIN_LEVEL_CONTROL
// User options for plugin Level Control Server Cluster
#define EMBER_AF_PLUGIN_LEVEL_CONTROL_MAXIMUM_LEVEL 255
#define EMBER_AF_PLUGIN_LEVEL_CONTROL_MINIMUM_LEVEL 0
#define EMBER_AF_PLUGIN_LEVEL_CONTROL_RATE 0

// Use this macro to check if Network Creator plugin is included
#define EMBER_AF_PLUGIN_NETWORK_CREATOR
// User options for plugin Network Creator
#define EMBER_AF_PLUGIN_NETWORK_CREATOR_SCAN_DURATION 4
#define EMBER_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_MASK 0x02108800
#define EMBER_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_BEACONS_THRESHOLD 20
#define EMBER_AF_PLUGIN_NETWORK_CREATOR_RADIO_POWER -2

// Use this macro to check if Network Creator Security plugin is included
#define EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY
// User options for plugin Network Creator Security
#define EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_NETWORK_OPEN_TIME_S 120
#define EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_TRUST_CENTER_SUPPORT
#define EMBER_AF_PLUGIN_NETWORK_CREATOR_SECURITY_ALLOW_HA_DEVICES_TO_STAY

// Use this macro to check if Network Find plugin is included
#define EMBER_AF_PLUGIN_NETWORK_FIND
#define EMBER_AF_DISABLE_FORM_AND_JOIN_TICK
// User options for plugin Network Find
#define EMBER_AF_PLUGIN_NETWORK_FIND_CHANNEL_MASK 0x0318C800
#define EMBER_AF_PLUGIN_NETWORK_FIND_RADIO_TX_POWER 3
#define EMBER_AF_PLUGIN_NETWORK_FIND_EXTENDED_PAN_ID { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
#define EMBER_AF_PLUGIN_NETWORK_FIND_DURATION 5
#define EMBER_AF_PLUGIN_NETWORK_FIND_JOINABLE_SCAN_TIMEOUT_MINUTES 1

// Use this macro to check if On/Off Server Cluster plugin is included
#define EMBER_AF_PLUGIN_ON_OFF

// Use this macro to check if OTA Bootload Cluster Common Code plugin is included
#define EMBER_AF_PLUGIN_OTA_COMMON

// Use this macro to check if OTA Bootload Cluster Server plugin is included
#define EMBER_AF_PLUGIN_OTA_SERVER
// User options for plugin OTA Bootload Cluster Server
#define EMBER_AF_PLUGIN_OTA_SERVER_MIN_BLOCK_REQUEST_SUPPORT

// Use this macro to check if OTA Bootload Cluster Server Policy plugin is included
#define EMBER_AF_PLUGIN_OTA_SERVER_POLICY

// Use this macro to check if OTA Bootload Cluster Storage Common Code plugin is included
#define EMBER_AF_PLUGIN_OTA_STORAGE_COMMON

// Use this macro to check if OTA POSIX Filesystem Storage Module plugin is included
#define EMBER_AF_PLUGIN_OTA_STORAGE_POSIX_FILESYSTEM

// Use this macro to check if Poll Control Client Cluster plugin is included
#define EMBER_AF_PLUGIN_POLL_CONTROL_CLIENT
// User options for plugin Poll Control Client Cluster
#define EMBER_AF_PLUGIN_POLL_CONTROL_CLIENT_DEFAULT_FAST_POLL_TIMEOUT 32

// Use this macro to check if Reporting plugin is included
#define EMBER_AF_PLUGIN_REPORTING
// User options for plugin Reporting
#define EMBER_AF_PLUGIN_REPORTING_TABLE_SIZE 10
#define EMBER_AF_PLUGIN_REPORTING_ENABLE_GROUP_BOUND_REPORTS

// Use this macro to check if Scan Dispatch plugin is included
#define EMBER_AF_PLUGIN_SCAN_DISPATCH
// User options for plugin Scan Dispatch
#define EMBER_AF_PLUGIN_SCAN_DISPATCH_SCAN_QUEUE_SIZE 10

// Use this macro to check if Simple Main plugin is included
#define EMBER_AF_PLUGIN_SIMPLE_MAIN

// Use this macro to check if Simple Metering Client plugin is included
#define EMBER_AF_PLUGIN_SIMPLE_METERING_CLIENT
// User options for plugin Simple Metering Client
#define EMBER_AF_PLUGIN_SIMPLE_METERING_CLIENT_NUMBER_OF_INTERVALS_SUPPORTED 4

// Use this macro to check if Stack Diagnostics plugin is included
#define EMBER_AF_PLUGIN_STACK_DIAGNOSTICS

// Use this macro to check if Test Harness plugin is included
#define EMBER_AF_PLUGIN_TEST_HARNESS
// User options for plugin Test Harness

// Use this macro to check if Time Server Cluster plugin is included
#define EMBER_AF_PLUGIN_TIME_SERVER
// User options for plugin Time Server Cluster

// Use this macro to check if Trust Center Backup plugin is included
#define EMBER_AF_PLUGIN_TRUST_CENTER_BACKUP
// User options for plugin Trust Center Backup
#define EMBER_AF_PLUGIN_TRUST_CENTER_BACKUP_MAX_CLI_BACKUP_SIZE 6

// Use this macro to check if Trust Center Network Key Update Broadcast plugin is included
#define EMBER_AF_PLUGIN_TRUST_CENTER_NWK_KEY_UPDATE_BROADCAST

// Use this macro to check if Trust Center Network Key Update Periodic plugin is included
#define EMBER_AF_PLUGIN_TRUST_CENTER_NWK_KEY_UPDATE_PERIODIC
// User options for plugin Trust Center Network Key Update Periodic
#define EMBER_AF_PLUGIN_TRUST_CENTER_NWK_KEY_UPDATE_PERIODIC_KEY_UPDATE_PERIOD 2
#define EMBER_AF_PLUGIN_TRUST_CENTER_NWK_KEY_UPDATE_PERIODIC_KEY_UPDATE_UNITS DAYS

// Use this macro to check if Trust Center Network Key Update Unicast plugin is included
#define EMBER_AF_PLUGIN_TRUST_CENTER_NWK_KEY_UPDATE_UNICAST

// Use this macro to check if EZSP Common plugin is included
#define EMBER_AF_PLUGIN_EZSP

// Use this macro to check if EZSP UART plugin is included
#define EMBER_AF_PLUGIN_EZSP_UART

// Use this macro to check if File Descriptor Dispatch plugin is included
#define EMBER_AF_PLUGIN_FILE_DESCRIPTOR_DISPATCH

// Use this macro to check if Gateway Support plugin is included
#define EMBER_AF_PLUGIN_GATEWAY
// User options for plugin Gateway Support
#define EMBER_AF_PLUGIN_GATEWAY_MAX_FDS 10
#define EMBER_AF_PLUGIN_GATEWAY_TCP_PORT_OFFSET 4900
#define EMBER_AF_PLUGIN_GATEWAY_MAX_WAIT_FOR_EVENT_TIMEOUT_MS 0xFFFFFFFF

// Use this macro to check if Gateway Relay Mqtt plugin is included
#define EMBER_AF_PLUGIN_GATEWAY_RELAY_MQTT

// Use this macro to check if NCP Configuration plugin is included
#define EMBER_AF_PLUGIN_NCP_CONFIGURATION
// User options for plugin NCP Configuration
#define EMBER_BINDING_TABLE_SIZE 16
#define EMBER_MAX_END_DEVICE_CHILDREN 32
#define EMBER_END_DEVICE_POLL_TIMEOUT 5
#define EMBER_END_DEVICE_POLL_TIMEOUT_SHIFT 6
#define EMBER_KEY_TABLE_SIZE 0
#define EMBER_ZLL_GROUP_ADDRESSES 0
#define EMBER_ZLL_RSSI_THRESHOLD -128
#define EMBER_TRANSIENT_KEY_TIMEOUT_S 120

// Use this macro to check if EZSP Secure Protocol Stub plugin is included
#define EMBER_AF_PLUGIN_SECURE_EZSP_STUB

// Use this macro to check if Unix Library plugin is included
#define EMBER_AF_PLUGIN_UNIX_LIBRARY
// User options for plugin Unix Library

// Use this macro to check if Unix Printf plugin is included
#define EMBER_AF_PLUGIN_UNIX_PRINTF


// Generated API headers

// API cjson from cJSON plugin
#define EMBER_AF_API_CJSON "util/third_party/cjson/cJSON.h"

// API linked-list from Linked List plugin
#define EMBER_AF_API_LINKED_LIST "util/plugin/plugin-common/linked-list/linked-list.h"

// API transport-mqtt from Gateway MQTT Transport plugin
#define EMBER_AF_API_TRANSPORT_MQTT "util/plugin/plugin-common/transport-mqtt/transport-mqtt.h"

// API command-relay from Command Relay plugin
#define EMBER_AF_API_COMMAND_RELAY "protocol/zigbee_6.1/app/framework/plugin/command-relay/command-relay.h"

// API device-table from Device Table plugin
#define EMBER_AF_API_DEVICE_TABLE "protocol/zigbee_6.1/app/framework/plugin/device-table/device-table.h"

// API network-creator from Network Creator plugin
#define EMBER_AF_API_NETWORK_CREATOR "protocol/zigbee_6.1/app/framework/plugin/network-creator/network-creator.h"

// API network-creator-security from Network Creator Security plugin
#define EMBER_AF_API_NETWORK_CREATOR_SECURITY "protocol/zigbee_6.1/app/framework/plugin/network-creator-security/network-creator-security.h"

// API scan-dispatch from Scan Dispatch plugin
#define EMBER_AF_API_SCAN_DISPATCH "protocol/zigbee_6.1/app/framework/plugin/scan-dispatch/scan-dispatch.h"

// API ezsp-protocol from EZSP Common plugin
#define EMBER_AF_API_EZSP_PROTOCOL "protocol/zigbee_6.1/app/util/ezsp/ezsp-protocol.h"

// API ezsp from EZSP Common plugin
#define EMBER_AF_API_EZSP "protocol/zigbee_6.1/app/util/ezsp/ezsp.h"

// API ezsp-secure from EZSP Secure Protocol Stub plugin
#define EMBER_AF_API_EZSP_SECURE "protocol/zigbee_6.1/app/util/ezsp/secure-ezsp-protocol.h"

// API crc from Unix Library plugin
#define EMBER_AF_API_CRC "platform/base/hal/micro/crc.h"

// API endian from Unix Library plugin
#define EMBER_AF_API_ENDIAN "platform/base/hal/micro/endian.h"

// API hal from Unix Library plugin
#define EMBER_AF_API_HAL "platform/base/hal/hal.h"

// API random from Unix Library plugin
#define EMBER_AF_API_RANDOM "platform/base/hal/micro/random.h"

// API system-timer from Unix Library plugin
#define EMBER_AF_API_SYSTEM_TIMER "platform/base/hal/micro/system-timer.h"


// Custom macros
#ifdef APP_SERIAL
#undef APP_SERIAL
#endif
#define APP_SERIAL 1

#ifdef EMBER_ASSERT_SERIAL_PORT
#undef EMBER_ASSERT_SERIAL_PORT
#endif
#define EMBER_ASSERT_SERIAL_PORT 1

#ifdef EMBER_AF_BAUD_RATE
#undef EMBER_AF_BAUD_RATE
#endif
#define EMBER_AF_BAUD_RATE 19200

#ifdef EMBER_AF_SERIAL_PORT_INIT
#undef EMBER_AF_SERIAL_PORT_INIT
#endif
#define EMBER_AF_SERIAL_PORT_INIT() \
  do { \
    emberSerialInit(1, BAUD_19200, PARITY_NONE, 1); \
  } while (0)



#endif // SILABS_APP_HUB_H
