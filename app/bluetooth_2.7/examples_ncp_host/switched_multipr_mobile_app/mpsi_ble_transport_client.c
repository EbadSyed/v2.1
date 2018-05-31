#include COMMON_HEADER
#include "crc16.h"
#include "mpsi_ble_transport_client.h"

// Error macro
#if defined(BLE_NCP_HOST)
#define ble_errorExit(...) do { mpsiTransportPrint(__VA_ARGS__); exit(EXIT_FAILURE); } while (0)
#else
#define ble_errorExit(...) do { mpsiTransportPrint(__VA_ARGS__); while (1) ; } while (0)
#endif

// Flush macros
#if defined(BLE_NCP_HOST)
#define ble_flush() fflush(stdout)
#else
// Do nothing if not on a host
#define ble_flush() (void)0
#endif

#define APP_ENABLE_AUTHENTICATION

#define CENTRAL_TYPE_SHORTENED_LOCAL_NAME 0x08
#define CENTRAL_TYPE_COMPLETE_LOCAL_NAME  0x09

// MPSI control command parameter values
typedef enum {
  mpsiControl_BeginTransaction = 1u,
  mpsiControl_CommitWithCRC = 2u
} MpsiControlValues_t;

// MPSI control command parameter values
typedef enum {
  mpsiEcode_NoError = 0x00,
  mpsiControlEcode_UnknownControlCommand  = 0x01,

  mpsiDataEcode_ControlIsNotSet     = 0x11,
  mpsiDataEcode_TooMuchDataSent   = 0x12,
  mpsiDataEcode_TooMuchDataRequested  = 0x13,
} MpsiEcodes_t;

// Remote server connection properties
typedef struct {
  bool isFound;
  bool isConnected;
  uint8_t connection;
  bd_addr address;
} ClientRemoteConnection_t;

ClientRemoteConnection_t client_remoteServer[serverNum_MaxNum] =  { { false, false, 0x00, { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } } },
                                                                    { false, false, 0x00, { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } } } };

// The server that we are connecting to
uint8_t activeServer;

// Client booted flag
bool client_booted = false;

// OTA (Over-the-Air firmware update) related BLE attributes
uint8_t client_otaServiceUuid[]   = { 0xf0, 0x19, 0x21, 0xb4, 0x47, 0x8f, 0xa4, 0xbf, 0xa1, 0x4f, 0x63, 0xfd, 0xee, 0xd6, 0x14, 0x1d };//1d14d6ee-fd63-4fa1-bfa4-8f47b42119f0
uint8_t client_otaControlUuid[]   = { 0x63, 0x60, 0x32, 0xe0, 0x37, 0x5e, 0xa4, 0x88, 0x53, 0x4e, 0x6d, 0xfb, 0x64, 0x35, 0xbf, 0xf7 };//f7bf3564-fb6d-4e53-88a4-5e37e0326063
uint8_t client_otaDataUuid[]      = { 0x53, 0xa1, 0x81, 0x1f, 0x58, 0x2c, 0xd0, 0xa5, 0x45, 0x40, 0xfc, 0x34, 0xf3, 0x27, 0x42, 0x98 };//984227f3-34fc-4045-a5d0-2c581f81a153

// MPSI (Multi-Protocol Stack Interface) related BLE attributes
uint8_t client_mpsiServiceUuid[]        = { 0x98, 0x55, 0xb6, 0x24, 0x19, 0x00, 0x9e, 0xa9, 0x88, 0x4b, 0x84, 0xbc, 0xda, 0x5f, 0x2f, 0xe5 };//e52f5fda-bc84-4b88-a99e-001924b65598
uint8_t client_mpsiOutputControlUuid[]  = { 0x54, 0xdf, 0xea, 0xfb, 0x20, 0xae, 0x5f, 0x81, 0xe9, 0x41, 0x77, 0x04, 0xa8, 0x7c, 0x06, 0x86 };//86067ca8-0477-41e9-815f-ae20fbeadf54
uint8_t client_mpsiOutputMessageUuid[]  = { 0xa8, 0x99, 0x44, 0x10, 0x08, 0x08, 0xa1, 0xae, 0x03, 0x4d, 0x34, 0x0b, 0xc1, 0x2e, 0x73, 0x98 };//98732ec1-0b34-4d03-aea1-0808104499a8
uint8_t client_mpsiInputControlUuid[]   = { 0x51, 0x38, 0xe7, 0x70, 0xf5, 0x94, 0x13, 0x92, 0x1d, 0x43, 0x59, 0x59, 0xd7, 0x82, 0xc0, 0xd5 };//d5c082d7-5959-431d-9213-94f570e73851
uint8_t client_mpsiInputMessageUuid[]   = { 0x05, 0x96, 0x32, 0xe5, 0x05, 0x55, 0x7e, 0xa0, 0xb2, 0x42, 0x0f, 0x61, 0x88, 0x2e, 0x53, 0x1e };//1e532e88-610f-42b2-a07e-5505e5329605

#define BLE_CUSTOM_SERVICE_SIZE         sizeof(client_otaServiceUuid)
#define BLE_CUSTOM_CHARACTERISTIC_SIZE  sizeof(client_mpsiOutputControlUuid)

// OTA related descriptors
uint32_t client_otaGattServiceHandle[serverNum_MaxNum];
uint16_t client_otaControlCharacteristic[serverNum_MaxNum];
uint16_t client_otaDataCharacteristic[serverNum_MaxNum];

// MPSI related descriptors
uint32_t client_mpsiGattServiceHandle[serverNum_MaxNum];
uint16_t client_mpsiOutputControlCharacteristic[serverNum_MaxNum];
uint16_t client_mpsiOutputMessageCharacteristic[serverNum_MaxNum];
uint16_t client_mpsiInputControlCharacteristic[serverNum_MaxNum];
uint16_t client_mpsiInputMessageCharacteristic[serverNum_MaxNum];

typedef enum {
  clientProcedureState_NoState          = 0x00,

  clientDiscoveryState_FindOtaService   = 0x01,
  clientDiscoveryState_FindMpsiService  = 0x02,

  clientDiscoveryState_FindOtaControlCharacteristic         = 0x11,
  clientDiscoveryState_FindOtaDataCharacteristic            = 0x12,
  clientDiscoveryState_FindMpsiInputControlCharacteristic   = 0x13,
  clientDiscoveryState_FindMpsiInputMessageCharacteristic   = 0x14,
  clientDiscoveryState_FindMpsiOutputControlCharacteristic  = 0x15,
  clientDiscoveryState_FindMpsiOutputMessageCharacteristic  = 0x16,

  clientConnectedState_EnableMpsiOutputControlIndication    = 0x21,

  clientConnectedState_SendMpsiControlBegin   = 0x31,
  clientConnectedState_SendMpsiMessage        = 0x32,
  clientConnectedState_SendMpsiControlCommit  = 0x33,
  clientConnectedState_WaitMpsiControlCommit  = 0x34,

  clientConnectedState_SendMpsiOutputControlBeginConfirmation   = 0x41,
  clientConnectedState_ReadMpsiOutputMessage                    = 0x42,
  clientConnectedState_WaitMpsiOutputMessage                    = 0x43,
  clientConnectedState_SendMpsiOutputControlCommitConfirmation  = 0x44,

  clientConnectedState_SendOtaControlBegin   = 0x51,
  clientConnectedState_SendOtaMessage        = 0x52,
  clientConnectedState_SendOtaControlCommit  = 0x53,
} ClientGattProcedureStates_t;

ClientGattProcedureStates_t client_gattProcedureState = clientProcedureState_NoState;

#define BLE_ATT_MTU_SIZE  247

// OTA DFU block size
#define SWITCHED_MULTIPROTOCOL_OTA_DATA_MAX_BLOCK_SIZE  (BLE_ATT_MTU_SIZE - 3)
#define SWITCHED_MULTIPROTOCOL_OTA_DFU_START            0x00
#define SWITCHED_MULTIPROTOCOL_OTA_DFU_COMMIT           0x03

uint8_t   smpd_otaServerInput[SWITCHED_MULTIPROTOCOL_OTA_DATA_MAX_BLOCK_SIZE];
uint32_t  smpd_otaServerInputRemainingBytes = 0;
FILE      *smpd_otaFile = NULL;
uint32_t  smpd_otaFileLen;
uint8_t   smpd_otaServerInputMaxLen = SWITCHED_MULTIPROTOCOL_OTA_DATA_MAX_BLOCK_SIZE;
uint8_t   smpd_otaServerWriteOperation = SWITCHED_MULTIPROTOCOL_OTA_WRITE_COMMAND;

uint8_t   smpd_mpsiServerInput[MPSI_DATA_MAX_MESSAGE_LEN];
uint16_t  smpd_mpsiServerInputCntr = 0;
uint8_t   smpd_mpsiServerControlInput[MPSI_CTRL_MAX_COMMAND_LEN];

uint8_t   smpd_mpsiServerOutput[MPSI_DATA_MAX_MESSAGE_LEN];
uint16_t  smpd_mpsiServerOutputCntr = 0;
uint16_t   smpd_mpsiServerInputActMsgLen = 0;
uint8_t   smpd_mpsiServerControlOutput[MPSI_CTRL_MAX_COMMAND_LEN];

// Security flag enum extension for BLE API
typedef enum {
  BleSecurityFlag_BondRequiresMitm          = 0x01,
  BleSecurityFlag_EnryptionRequiresBonding  = 0x02,
  BleSecurityFlag_SecureConnectionOnly      = 0x04,
  BleSecurityFlag_NeedToConfirmBonding      = 0x08,
} BleSecurityFlags_t;

/**
 * Discovers BLE service by UUID. Wrapper around cmd_gatt_discover_primary_services_by_uuid()
 * API function.
 */
static inline void discoverCustomBleService(uint8_t connection,
                                            uint8_t *serviceUuid)
{
  struct gecko_msg_gatt_discover_primary_services_by_uuid_rsp_t *rsp;
  rsp =
    gecko_cmd_gatt_discover_primary_services_by_uuid(connection,
                                                     BLE_CUSTOM_SERVICE_SIZE,
                                                     serviceUuid);
  if (rsp->result) {
    ble_errorExit("Error, service discover failed. Errno: 0x%x. Exiting...",
                  rsp->result);
  }
  mpsiTransportPrint("Discovering services... ");
  ble_flush();
}

/**
 * Discovers BLE characteristics by UUID. Wrapper around cmd_gatt_discover_primary_services_by_uuid()
 * API function.
 */
static inline void discoverCustomBleCharacteristic(uint8_t connection,
                                                   uint32_t serviceHandler,
                                                   uint8_t *characteristicUuid)
{
  struct gecko_msg_gatt_discover_characteristics_by_uuid_rsp_t *rsp;
  rsp = gecko_cmd_gatt_discover_characteristics_by_uuid(connection,
                                                        serviceHandler,
                                                        BLE_CUSTOM_CHARACTERISTIC_SIZE,
                                                        characteristicUuid);
  if (rsp->result) {
    ble_errorExit("Error, characteristics discover failed. Errno: 0x%x. Exiting...", rsp->result);
  }
  mpsiTransportPrint("Discovering characteristics... ");
  ble_flush();
}

/**
 * Enable or disable an indication. Wrapper around cmd_gatt_set_characteristic_notification()
 * API function.
 */
static inline void setIndication(uint8_t connection,
                                 uint16_t characteristicHandler)
{
  struct gecko_msg_gatt_set_characteristic_notification_rsp_t *rsp;
  rsp = gecko_cmd_gatt_set_characteristic_notification(connection,
                                                       characteristicHandler,
                                                       gatt_indication);
  if (rsp->result) {
    ble_errorExit("Error, set indication failed. Errno: 0x%x. Exiting...", rsp->result);
  }
  mpsiTransportPrint("Setting indication... ");
  ble_flush();
}

static inline void startDiscoveringServices(uint8_t connection)
{
  mpsiTransportPrint("Discovering services.\n");

  // Server is connected, now explore services
  discoverCustomBleService(connection, client_otaServiceUuid);
  client_gattProcedureState = clientDiscoveryState_FindOtaService;
}

void sendTestData(uint8_t server)
{
  // Notify client that we have something for it.
  uint16_t i;
  struct gecko_msg_gatt_write_characteristic_value_rsp_t *ret;

  if (clientProcedureState_NoState == client_gattProcedureState) {
/*****  OTA write-comamand  *****/
    mpsiTransportPrint("Sending MPSI message is requested.\n");

    activeServer = server;

    // Fill output buffer before sending out indication.
    for (i = 0; i < MPSI_DATA_MAX_MESSAGE_LEN; i++) {
      smpd_mpsiServerInput[i] = i;
    }

    // Set message length
    smpd_mpsiServerInputActMsgLen = 259;

    // Do not start sending data if too much is requested.
    if (MPSI_DATA_MAX_MESSAGE_LEN < smpd_mpsiServerInputActMsgLen) {
      mpsiTransportPrint("Error: MPSI_DATA_MAX_MESSAGE_LEN:%d < smpd_mpsiServerInputActMsgLen:%d",
                         MPSI_DATA_MAX_MESSAGE_LEN,
                         smpd_mpsiServerInputActMsgLen);
    } else {
      // Set payload length field
      smpd_mpsiServerInput[MPSI_MESSAGE_PAYLOAD_LEN_LOC] =
        smpd_mpsiServerInputActMsgLen - 4;

      // Clear output byte counter
      smpd_mpsiServerInputCntr = 0;

      // Send indication that we have something to read.
      smpd_mpsiServerControlInput[MPSI_CTRL_CMD] =
        mpsiControl_BeginTransaction;
      ret =
        gecko_cmd_gatt_write_characteristic_value(client_remoteServer[server].connection,
                                                  client_mpsiInputControlCharacteristic[server],
                                                  MPSI_CTRL_MAX_COMMAND_LEN,
                                                  smpd_mpsiServerControlInput);

      if (ret->result) {
        mpsiTransportPrint("Error while writing MPSI server output control! Errno: 0x%x\n",
                           ret->result);
      } else {
        mpsiTransportPrint("Writing server MPSI output control...\n");
        client_gattProcedureState =
          clientConnectedState_SendMpsiControlBegin;
      }
    }
  } else {
    mpsiTransportPrint("Error while trying to send write request! Other operation is in progress!\n");
  }
}

int8_t bleSendOtaFile(serverList_t server, uint8_t writeOperation, uint32_t slotId, FILE *fileToSend)
{
  struct gecko_msg_gatt_write_characteristic_value_rsp_t *ret;
  uint8_t otaMsgData[OTA_CTRL_MAX_COMMAND_LEN];

  if ( (SWITCHED_MULTIPROTOCOL_OTA_WRITE_COMMAND != writeOperation)
       && (SWITCHED_MULTIPROTOCOL_OTA_WRITE_REQUEST != writeOperation) ) {
    mpsiTransportPrint("Error, wronge write operation parameter!\n");
    return MPSI_TRANSPORT_ERROR;
  }

  smpd_otaServerWriteOperation = writeOperation;

  otaMsgData[0] = SWITCHED_MULTIPROTOCOL_OTA_DFU_START;
  otaMsgData[1] = UINT32_TO_BYTE0(slotId);
  otaMsgData[2] = UINT32_TO_BYTE1(slotId);
  otaMsgData[3] = UINT32_TO_BYTE2(slotId);
  otaMsgData[4] = UINT32_TO_BYTE3(slotId);

  if (clientProcedureState_NoState == client_gattProcedureState) {
    mpsiTransportPrint("Sending OTA message is requested.\n");
    activeServer = server;
    smpd_otaFile = fileToSend;

    ret =
      gecko_cmd_gatt_write_characteristic_value(client_remoteServer[server].connection,
                                                client_otaControlCharacteristic[server],
                                                COUNTOF(otaMsgData),
                                                otaMsgData);

    if (ret->result) {
      mpsiTransportPrint("Error while writing OTAserver output control! Errno: 0x%x\n",
                         ret->result);
      return MPSI_TRANSPORT_ERROR;
    } else {
      mpsiTransportPrint("Sending server OTA output control...\n");
      client_gattProcedureState =
        clientConnectedState_SendOtaControlBegin;
    }
  } else {
    mpsiTransportPrint("Error while trying to send write request! Other operation is in progress!\n");
    return MPSI_TRANSPORT_ERROR;
  }

  return MPSI_TRANSPORT_SUCCESS;
}

int8_t bleSendMpsiTransportLongMessage(serverList_t server, uint8_t *buffer, uint16_t len)
{
  // Notify client that we have something for it.
  struct gecko_msg_gatt_write_characteristic_value_rsp_t *ret;

  if (clientProcedureState_NoState == client_gattProcedureState) {
    activeServer = server;

/*****  MPSI data *****/
    mpsiTransportPrint("Sending MPSI message is requested.\n");

    // Copy buffer to send before sending out indication.
    memcpy(smpd_mpsiServerInput, buffer, len);

    // Set message length
    smpd_mpsiServerInputActMsgLen = len;

    // Do not start sending data if too much is requested.
    if (MPSI_DATA_MAX_MESSAGE_LEN < smpd_mpsiServerInputActMsgLen) {
      mpsiTransportPrint("Error: MPSI_DATA_MAX_MESSAGE_LEN:%d < smpd_mpsiServerInputActMsgLen:%d",
                         MPSI_DATA_MAX_MESSAGE_LEN,
                         smpd_mpsiServerInputActMsgLen);
      return MPSI_TRANSPORT_ERROR;
    } else {
      // Clear output byte counter
      smpd_mpsiServerInputCntr = 0;

      // Send indication that we have something to read.
      smpd_mpsiServerControlInput[MPSI_CTRL_CMD] =
        mpsiControl_BeginTransaction;
      ret =
        gecko_cmd_gatt_write_characteristic_value(client_remoteServer[server].connection,
                                                  client_mpsiInputControlCharacteristic[server],
                                                  MPSI_CTRL_MAX_COMMAND_LEN,
                                                  smpd_mpsiServerControlInput);
      if (ret->result) {
        mpsiTransportPrint("Error while writing MPSI server output control! Errno: 0x%x\n",
                           ret->result);
        return MPSI_TRANSPORT_ERROR;
      } else {
        mpsiTransportPrint("Writing server MPSI output control...\n");
        client_gattProcedureState =
          clientConnectedState_SendMpsiControlBegin;
      }
    }
  } else {
    mpsiTransportPrint("Error while trying to send write request! Other operation is in progress!\n");
    return MPSI_TRANSPORT_ERROR;
  }

  return MPSI_TRANSPORT_SUCCESS;
}

void bleConnect(serverList_t server)
{
  struct gecko_msg_le_gap_open_rsp_t *rsp;
  bd_addr *remoteAddress = &client_remoteServer[server].address;
  uint8_t *connection = &client_remoteServer[server].connection;

  activeServer = server;

  //move to connect state, connect to device address
  rsp = gecko_cmd_le_gap_open(*remoteAddress, le_gap_address_type_public);
  if (rsp->result) {
    ble_errorExit("Error, opening connection failed. Errno: 0x%x\n Exiting...", rsp->result);
  }
  *connection = rsp->connection;
  mpsiTransportPrint("Connecting to %02x:%02x:%02x:%02x:%02x:%02x, Connection: %d...",
                     remoteAddress->addr[5],
                     remoteAddress->addr[4],
                     remoteAddress->addr[3],
                     remoteAddress->addr[2],
                     remoteAddress->addr[1],
                     remoteAddress->addr[0],
                     *connection);
  ble_flush();
}

void mpsiTransportHandleEvents(struct gecko_cmd_packet *evt)
{
  // Do not try to process NULL event.
  if (NULL == evt) {
    return;
  }

  // Do not handle any events until system is booted up properly.
  if ((BGLIB_MSG_ID(evt->header) != gecko_evt_system_boot_id)
      && !client_booted) {
#if defined(DEBUG)
    printf("Event: 0x%04x\n", BGLIB_MSG_ID(evt->header));
#endif // DEBUG
    usleep(50000);
    return;
  }

  /* Handle events */
  switch (BGLIB_MSG_ID(evt->header)) {
/************************
      Boot
 *************************/

    /* This boot event is generated when the system boots up after reset.
     * Here the system is set to start advertising immediately after boot procedure. */
    case gecko_evt_system_boot_id: {
      struct gecko_msg_system_get_bt_address_rsp_t *rsp_bt_addr;
      struct gecko_msg_le_gap_set_conn_parameters_rsp_t *rsp_conn_param;
      struct gecko_msg_gatt_set_max_mtu_rsp_t *rsp_set_max_mtu;

      printf("\n");
      mpsiTransportPrint("Client booted!\n");
      client_booted = true;

      // Read out local address
      rsp_bt_addr = gecko_cmd_system_get_bt_address();
      mpsiTransportPrint("Local address:%02x:%02x:%02x:%02x:%02x:%02x\n",
                         rsp_bt_addr->address.addr[5],
                         rsp_bt_addr->address.addr[4],
                         rsp_bt_addr->address.addr[3],
                         rsp_bt_addr->address.addr[2],
                         rsp_bt_addr->address.addr[1],
                         rsp_bt_addr->address.addr[0]);

      //set default connection parameters
      rsp_conn_param = gecko_cmd_le_gap_set_conn_parameters(6, 6, 0, 300);
      if (rsp_conn_param->result) {
        ble_errorExit("Error, set connection parameters failed. Errno: 0x%x\n Exiting...", rsp_conn_param->result);
      }

      gecko_cmd_sm_delete_bondings();
      mpsiTransportPrint("HEADS UP! Removing all bondings!\n");

      // Change the MTU size to the max allowed by SMP OTA DFU limits.
      rsp_set_max_mtu = gecko_cmd_gatt_set_max_mtu(BLE_ATT_MTU_SIZE);
      if (rsp_set_max_mtu->result) {
        mpsiTransportPrint("Failed to setting MTU size! Error: %d\n", rsp_set_max_mtu->result);
      } else {
        mpsiTransportPrint("ATT MTU size is set to: %d\n", rsp_set_max_mtu->max_mtu);
      }

#ifdef APP_ENABLE_AUTHENTICATION
      // Configure security manager
      gecko_cmd_sm_configure((BleSecurityFlag_BondRequiresMitm
                              | BleSecurityFlag_EnryptionRequiresBonding
                              | BleSecurityFlag_NeedToConfirmBonding),
                             sm_io_capability_displayyesno);

      mpsiTransportPrint("Client: Security Manger is configured.\n");

      // Allow new bondings
      gecko_cmd_sm_set_bondable_mode(true);
      mpsiTransportPrint("Client is bondable.\n");
#endif

      bleStartDeviceDiscovery(1);
      break;
    }

/************************
 *  Connection
 ***********************/
    case gecko_evt_le_connection_closed_id:
      if (client_remoteServer[serverNum_Smpd].connection == evt->data.evt_le_connection_closed.connection) {
        client_remoteServer[serverNum_Smpd].connection   = 0x00;
        client_remoteServer[serverNum_Smpd].isConnected  = false;
        client_remoteServer[serverNum_Smpd].isFound      = false;
        mpsiTransportPrint("Connection closed!\n  - Joining Device: %d\n  - Reason: 0x%04x\n",
                           evt->data.evt_le_connection_closed.connection,
                           evt->data.evt_le_connection_closed.reason);
      } else if (client_remoteServer[serverNum_Tc].connection == evt->data.evt_le_connection_closed.connection) {
        client_remoteServer[serverNum_Tc].connection   = 0x00;
        client_remoteServer[serverNum_Tc].isConnected  = false;
        client_remoteServer[serverNum_Tc].isFound      = false;
        mpsiTransportPrint("Connection closed!\n  - Trust Center: %d\n  - Reason: 0x%04x\n",
                           evt->data.evt_le_connection_closed.connection,
                           evt->data.evt_le_connection_closed.reason);
      } else {
        mpsiTransportPrint("Connection closed!\n  - Unknown device: %d\n  - Reason: 0x%04x\n",
                           evt->data.evt_le_connection_closed.connection,
                           evt->data.evt_le_connection_closed.reason);
      }
      // Restart server discovery.
      bleStartDeviceDiscovery(1);
      break;

    case gecko_evt_le_gap_scan_response_id: {
      uint8_t i;
      uint8_t *scanResponseData = evt->data.evt_le_gap_scan_response.data.data;
      uint8_t ad_field_length, ad_field_type;

#if defined(DEBUG_BLE_SCANNING)
      mpsiTransportPrint("Scan response received:\n");
      mpsiTransportPrint("  - address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                         evt->data.evt_le_gap_scan_response.address.addr[5],
                         evt->data.evt_le_gap_scan_response.address.addr[4],
                         evt->data.evt_le_gap_scan_response.address.addr[3],
                         evt->data.evt_le_gap_scan_response.address.addr[2],
                         evt->data.evt_le_gap_scan_response.address.addr[1],
                         evt->data.evt_le_gap_scan_response.address.addr[0]);
      mpsiTransportPrint("  - address type: %d\n",
                         evt->data.evt_le_gap_scan_response.address_type);
      mpsiTransportPrint("  - bonding: %d\n",
                         evt->data.evt_le_gap_scan_response.bonding);

      mpsiTransportPrint("  - data len: %d\n", evt->data.evt_le_gap_scan_response.data.len);
      mpsiTransportPrint("  - data: ");
      for (i = 0; i < evt->data.evt_le_gap_scan_response.data.len; i++) {
        mpsiTransportPrint("%02x ", evt->data.evt_le_gap_scan_response.data.data[i]);
      }
      printf("\n");
#endif // DEBUG_BLE_SCANNING

      i = 0;

      // Step through advertisement data and interpret the fields inside
      while (i < evt->data.evt_le_gap_scan_response.data.len) {
        ad_field_length = scanResponseData[i];   // Read adv field length
        ad_field_type = scanResponseData[i + 1];   // Read adv field type

#if defined(DEBUG_BLE_SCANNING)
        mpsiTransportPrint("Field type: %d\n", ad_field_type);
#endif // DEBUG_BLE_SCANNING

        if ((ad_field_type == CENTRAL_TYPE_SHORTENED_LOCAL_NAME)
            || (ad_field_type == CENTRAL_TYPE_COMPLETE_LOCAL_NAME) ) {
#if defined(DEBUG_BLE_SCANNING)
          mpsiTransportPrint("  --> Local Name: 0x%2x 0x%2x 0x%2x 0x%2x\n", scanResponseData[i + 2], scanResponseData[i + 3], scanResponseData[i + 4], scanResponseData[i + 5]);
#endif // DEBUG_BLE_SCANNING

          // Check if Joining Device is already connected.
          if (!client_remoteServer[serverNum_Smpd].isConnected) {
            // Switched Multiprotocol Joining Device found.
            if ( (scanResponseData[i + 2] == 'S')
                 && (scanResponseData[i + 3] == 'L')
                 && (scanResponseData[i + 4] == 'J')
                 && (scanResponseData[i + 5] == 'D') ) {
              if (!client_remoteServer[serverNum_Smpd].isFound) {
                mpsiTransportPrint("Joining Device found!\n");
                client_remoteServer[serverNum_Smpd].isFound = true;
              }
              memcpy((char *)(&client_remoteServer[serverNum_Smpd].address), evt->data.evt_le_gap_scan_response.address.addr, 6);
            }
          }

          // Check if trust-centeer is already connected.
          if (!client_remoteServer[serverNum_Tc].isConnected) {
            // Switched Multiprotocol Trust-Center device found.
            if ( (scanResponseData[i + 2] == 'S')
                 && (scanResponseData[i + 3] == 'L')
                 && (scanResponseData[i + 4] == 'T')
                 && (scanResponseData[i + 5] == 'C') ) {
              if (!client_remoteServer[serverNum_Tc].isFound) {
                mpsiTransportPrint("Trust-center found!\n");
                client_remoteServer[serverNum_Tc].isFound = true;
              }
              memcpy((char *)(&client_remoteServer[serverNum_Tc].address), evt->data.evt_le_gap_scan_response.address.addr, 6);
            }
          }
        }

        // Increase i as many counts as needed.
        i += ad_field_length + 1;
      }

      break;
    }

    case gecko_evt_le_connection_opened_id: {
      printf("OK\n");
      ble_flush();

      client_remoteServer[activeServer].isConnected = true;
      client_remoteServer[activeServer].connection = evt->data.evt_le_connection_opened.connection;

      // Stop discovering new servers until services are discovered.
      bleStopDeviceDiscovery();

#ifdef APP_ENABLE_AUTHENTICATION
      // Check if already bonded, Move on if so, start bonding process otherwise.
      if (0xFF == evt->data.evt_le_connection_opened.bonding) {
        struct gecko_msg_sm_increase_security_rsp_t *rsp;

        rsp = gecko_cmd_sm_increase_security(evt->data.evt_le_connection_opened.connection);
        if (rsp->result) {
          ble_errorExit("Error, starting increasing security. Errno: 0x%x\n Exiting...", rsp->result);
        }
        mpsiTransportPrint("Increasing security...\n");
      } else {
        // Do  not disable bonding at this point as this client needs to connect
        // to multiple server devices.
        mpsiTransportPrint("Client: Already bonded to remote device!\n");
        startDiscoveringServices(client_remoteServer[activeServer].connection);
      }
      break;

/************************
 *    Bonding
 ***********************/
      case gecko_evt_sm_bonded_id:
        // Do  not disable bonding at this point as this client needs to connect
        // to multiple server devices.
        mpsiTransportPrint("Client: Bonded! Data: %d\n", evt->data.evt_sm_bonded.bonding);
#else
      // Do  not disable bonding at this point as this client needs to connect
      // to multiple server devices.
      if (0xFF == evt->data.evt_le_connection_opened.bonding) {
        mpsiTransportPrint("Remote device is not bonded.\n");
      } else {
        mpsiTransportPrint("Client: Already bonded to remote device!\n");
      }
#endif
        startDiscoveringServices(client_remoteServer[activeServer].connection);
        break;
    }

#ifdef APP_ENABLE_AUTHENTICATION
    case gecko_evt_sm_bonding_failed_id:
      ble_errorExit("Error, bonding failed. Errno: 0x%x\n Exiting...", evt->data.evt_sm_bonding_failed.reason);
      break;

    case gecko_evt_sm_confirm_bonding_id:
      mpsiTransportPrint("Client: Auto confirm new bonding...\n");
      gecko_cmd_sm_bonding_confirm(evt->data.evt_sm_confirm_bonding.connection, 1u);
      break;

/************************
 *    Passkey
 ***********************/
    case gecko_evt_sm_passkey_display_id:
      mpsiTransportPrint("Client: Displaying passkey: %6d", evt->data.evt_sm_passkey_display.passkey);
      break;

    case gecko_evt_sm_passkey_request_id:
      mpsiTransportPrint("Client: Requesting passkey...");
      gecko_cmd_sm_enter_passkey(evt->data.evt_sm_passkey_request.connection, 42u);
      break;

    case gecko_evt_sm_confirm_passkey_id:
      mpsiTransportPrint("Client: Auto confirm passkey: %d\n", evt->data.evt_sm_confirm_passkey.passkey);
      gecko_cmd_sm_passkey_confirm(evt->data.evt_sm_confirm_bonding.connection, 1u);
      break;
#endif

/************************
 *    Service
 ***********************/
    case gecko_evt_gatt_service_id: {
      if (clientDiscoveryState_FindOtaService == client_gattProcedureState) {
        // Store OTA service info
        client_otaGattServiceHandle[activeServer] = evt->data.evt_gatt_service.service;
        mpsiTransportPrint("OTA GATT Service found: %d", evt->data.evt_gatt_service.service);
      } else if (clientDiscoveryState_FindMpsiService == client_gattProcedureState) {
        // Store MPSI service info
        client_mpsiGattServiceHandle[activeServer] = evt->data.evt_gatt_service.service;
        mpsiTransportPrint("MPSI GATT Service found: %d", evt->data.evt_gatt_service.service);
      }
      break;
    }

/************************
 *     Characteristic
 ***********************/
    case gecko_evt_gatt_characteristic_id: {
      if (clientDiscoveryState_FindOtaControlCharacteristic == client_gattProcedureState) {
        // Store OTA control characteristic info
        client_otaControlCharacteristic[activeServer] = evt->data.evt_gatt_characteristic.characteristic;
        mpsiTransportPrint("OTA Control found: %d", evt->data.evt_gatt_characteristic.characteristic);
      } else if (clientDiscoveryState_FindOtaDataCharacteristic == client_gattProcedureState) {
        // Store OTA data characteristic info
        client_otaDataCharacteristic[activeServer] = evt->data.evt_gatt_characteristic.characteristic;
        mpsiTransportPrint("OTA Data found: %d", evt->data.evt_gatt_characteristic.characteristic);
      } else if (clientDiscoveryState_FindMpsiInputControlCharacteristic == client_gattProcedureState) {
        // Store MPSI input control characteristic info
        client_mpsiInputControlCharacteristic[activeServer] = evt->data.evt_gatt_characteristic.characteristic;
        mpsiTransportPrint("MPSI Input Control found: %d", evt->data.evt_gatt_characteristic.characteristic);
      } else if (clientDiscoveryState_FindMpsiInputMessageCharacteristic == client_gattProcedureState) {
        // Store MPSI input message characteristic info
        client_mpsiInputMessageCharacteristic[activeServer] = evt->data.evt_gatt_characteristic.characteristic;
        mpsiTransportPrint("MPSI Input Message found: %d", evt->data.evt_gatt_characteristic.characteristic);
      } else if (clientDiscoveryState_FindMpsiOutputControlCharacteristic == client_gattProcedureState) {
        // Store MPSI output control characteristic info
        client_mpsiOutputControlCharacteristic[activeServer] = evt->data.evt_gatt_characteristic.characteristic;
        mpsiTransportPrint("MPSI Output Control found: %d", evt->data.evt_gatt_characteristic.characteristic);
      } else if (clientDiscoveryState_FindMpsiOutputMessageCharacteristic == client_gattProcedureState) {
        // Store MPSI output message characteristic info
        client_mpsiOutputMessageCharacteristic[activeServer] = evt->data.evt_gatt_characteristic.characteristic;
        mpsiTransportPrint("MPSI Output Message found: %d", evt->data.evt_gatt_characteristic.characteristic);
      }
      break;
    }

/****************************
 *    Procedure complete
 ***************************/

    // This event is called after every GATT procedure
    // (except gatt_write_characteristic_value_without_response and gatt_send_characteristic_confirmation).
    // It is used to handle state changes after an operation.
    case gecko_evt_gatt_procedure_completed_id: {
      uint16_t procedureResult = evt->data.evt_gatt_procedure_completed.result;

      // Don't drop error for cases where errors are handled.
      if (clientConnectedState_WaitMpsiControlCommit != client_gattProcedureState) {
        // Test if procedure returned with error, drop error and return if so.
        if (procedureResult) {
          mpsiTransportPrint("\n\nProcedure Error!\nProcedure result: 0x%x\n", procedureResult);
          if (clientProcedureState_NoState != client_gattProcedureState) {
            mpsiTransportPrint("Procedure state: 0x%x\n", client_gattProcedureState);
          }
          // Exit for now.
          client_gattProcedureState = clientProcedureState_NoState;
          return;
        }
      }

      if (procedureResult) {
        printf(" ...Not OK\n");
      } else {
        printf(" ...OK\n");
      }

/*****  Found services *****/
      // Handle next operation according to the actual state.
      switch (client_gattProcedureState) {
        case clientDiscoveryState_FindOtaService:
          // Look for MPSI service
          discoverCustomBleService(client_remoteServer[activeServer].connection, client_mpsiServiceUuid);
          client_gattProcedureState = clientDiscoveryState_FindMpsiService;
          break;
        case clientDiscoveryState_FindMpsiService:
          // Look for OTA control characteristic
          discoverCustomBleCharacteristic(client_remoteServer[activeServer].connection, client_otaGattServiceHandle[activeServer], client_otaControlUuid);
          client_gattProcedureState = clientDiscoveryState_FindOtaControlCharacteristic;
          break;

/*****  Found characteristics *****/
        case clientDiscoveryState_FindOtaControlCharacteristic:
          // Look for OTA data characteristic
          discoverCustomBleCharacteristic(client_remoteServer[activeServer].connection, client_otaGattServiceHandle[activeServer], client_otaDataUuid);
          client_gattProcedureState = clientDiscoveryState_FindOtaDataCharacteristic;
          break;
        case clientDiscoveryState_FindOtaDataCharacteristic:
          // Look for MPSI input control characteristic
          discoverCustomBleCharacteristic(client_remoteServer[activeServer].connection, client_mpsiGattServiceHandle[activeServer], client_mpsiInputControlUuid);
          client_gattProcedureState = clientDiscoveryState_FindMpsiInputControlCharacteristic;
          break;
        case clientDiscoveryState_FindMpsiInputControlCharacteristic:
          // Look for MPSI input message characteristic
          discoverCustomBleCharacteristic(client_remoteServer[activeServer].connection, client_mpsiGattServiceHandle[activeServer], client_mpsiInputMessageUuid);
          client_gattProcedureState = clientDiscoveryState_FindMpsiInputMessageCharacteristic;
          break;
        case clientDiscoveryState_FindMpsiInputMessageCharacteristic:
          // Look for MPSI output control characteristic
          discoverCustomBleCharacteristic(client_remoteServer[activeServer].connection, client_mpsiGattServiceHandle[activeServer], client_mpsiOutputControlUuid);
          client_gattProcedureState = clientDiscoveryState_FindMpsiOutputControlCharacteristic;
          break;
        case clientDiscoveryState_FindMpsiOutputControlCharacteristic:
          // Look for MPSI output message characteristic
          discoverCustomBleCharacteristic(client_remoteServer[activeServer].connection, client_mpsiGattServiceHandle[activeServer], client_mpsiOutputMessageUuid);
          client_gattProcedureState = clientDiscoveryState_FindMpsiOutputMessageCharacteristic;
          break;
        case clientDiscoveryState_FindMpsiOutputMessageCharacteristic:
          // All characteristics are discovered successfully
          mpsiTransportPrint("All characteristics found successfully!\n");
          // Enable server notification for MPSI control output
          setIndication(client_remoteServer[activeServer].connection, client_mpsiOutputControlCharacteristic[activeServer]);
          client_gattProcedureState = clientConnectedState_EnableMpsiOutputControlIndication;
          break;
        case clientConnectedState_EnableMpsiOutputControlIndication:
          // Server notification enabled. Nothing more to do for now.
          printf("\n");
          client_gattProcedureState = clientProcedureState_NoState;

          // Restart server discovery if any of the servers is not connected yet.
          if (!client_remoteServer[serverNum_Smpd].isConnected
              || !client_remoteServer[serverNum_Tc].isConnected) {
            bleStartDeviceDiscovery(1);
          }

          break;

/*****  Sending MPSI Client --> Server Message *****/
        case clientConnectedState_SendMpsiControlBegin:
        case clientConnectedState_SendMpsiMessage: {
          // Control sent successfully. Start sending message.

          struct gecko_msg_gatt_write_characteristic_value_rsp_t *ret;

          // Check if remaining data is more than max payload length long
          // BGLIB_MSG_MAX_PAYLOAD - 4 = 124
          uint16_t len = (smpd_mpsiServerInputActMsgLen - smpd_mpsiServerInputCntr > (124) )
                         ? (124)
                         : (smpd_mpsiServerInputActMsgLen - smpd_mpsiServerInputCntr);

          // Check if data would overflow of server's input buffer
          if (MPSI_DATA_MAX_MESSAGE_LEN < smpd_mpsiServerInputCntr + len) {
            mpsiTransportPrint("Error: Attempt to send too much data! This should not happen...\n");
            client_gattProcedureState = clientProcedureState_NoState;
          } else {
            ret =
              gecko_cmd_gatt_write_characteristic_value(client_remoteServer[activeServer].connection,
                                                        client_mpsiInputMessageCharacteristic[activeServer],
                                                        len,
                                                        (uint8_t*) &smpd_mpsiServerInput[smpd_mpsiServerInputCntr]);
            if (ret->result) {
              mpsiTransportPrint("Error while attempting to send server's input message! Errno: 0x%x\n", ret->result);
              // Exit for now.
              client_gattProcedureState = clientProcedureState_NoState;
            } else {
              smpd_mpsiServerInputCntr += len;
              mpsiTransportPrint("Server's input data sent successfully! Len: %d\n", smpd_mpsiServerInputCntr);

              // Check if all available data sent or overflow.
              if (smpd_mpsiServerInputActMsgLen == smpd_mpsiServerInputCntr) {
                mpsiTransportPrint("Last write block is served, issue to send control and CRC.\n");
                client_gattProcedureState = clientConnectedState_SendMpsiControlCommit;
              } else if (smpd_mpsiServerInputActMsgLen < smpd_mpsiServerInputCntr) {
                // Thats too bad, something went wrong, shouldn't be here.
                mpsiTransportPrint("Error: smpd_mpsiServerInputActMsgLen: %d < smpd_mpsiServerInputCntr: %d\n", smpd_mpsiServerInputActMsgLen, smpd_mpsiServerInputCntr);
                // Exit for now.
                client_gattProcedureState = clientProcedureState_NoState;
              }
            }
          }

          break;
        }

/***** Sending OTA data (Client --> Server) *****/
        case clientConnectedState_SendOtaControlBegin:
          if (fseek(smpd_otaFile, 0L, SEEK_END)) {
            mpsiTransportPrint("File SEEK_END error!\n");
            client_gattProcedureState = clientProcedureState_NoState;
            break;
          }

          smpd_otaFileLen = smpd_otaServerInputRemainingBytes = ftell(smpd_otaFile);

          if (fseek(smpd_otaFile, 0L, SEEK_SET)) {
            mpsiTransportPrint("File SEEK_SET error!\n");
            client_gattProcedureState = clientProcedureState_NoState;
            break;
          }

          mpsiTransportPrint("OTA File bytes to send: %d\n", (uint32_t)smpd_otaServerInputRemainingBytes);

        // Intentionally falls through to next case.
        case clientConnectedState_SendOtaMessage: {
          // Control sent successfully. Start sending message.

          struct gecko_msg_gatt_write_characteristic_value_rsp_t *ret_writeRequest;
          struct gecko_msg_gatt_write_characteristic_value_without_response_rsp_t *ret_writeCommand;
          uint16_t len;

          // Send the whole image in a loop for now
          do {
            len =
              smpd_otaServerInputRemainingBytes > smpd_otaServerInputMaxLen
              ? smpd_otaServerInputMaxLen
              : smpd_otaServerInputRemainingBytes;

            // Read next block from file, drop error in case of error.
            if (fread(smpd_otaServerInput, 1, len, smpd_otaFile) != len) {
              mpsiTransportPrint("File read failure\n");
              client_gattProcedureState = clientProcedureState_NoState;
            } else {
              // Send data according to specified write operation.
              if (SWITCHED_MULTIPROTOCOL_OTA_WRITE_REQUEST == smpd_otaServerWriteOperation) {
                // Send data with the write request operation
                ret_writeRequest =
                  gecko_cmd_gatt_write_characteristic_value(client_remoteServer[activeServer].connection,
                                                            client_otaDataCharacteristic[activeServer],
                                                            len,
                                                            smpd_otaServerInput);
                if (ret_writeRequest->result) {
                  mpsiTransportPrint("Error while attempting to send server's OTA input message (write request) ! Errno: 0x%x\n",
                                     ret_writeRequest->result);
                  client_gattProcedureState = clientProcedureState_NoState;
                }
              } else {
                // Send data with the write command operation, repeate if failed to send.
                do {
                  gecko_peek_event();
                  ret_writeCommand =
                    gecko_cmd_gatt_write_characteristic_value_without_response(client_remoteServer[activeServer].connection,
                                                                               client_otaDataCharacteristic[activeServer],
                                                                               len,
                                                                               smpd_otaServerInput);
                } while ( (ret_writeCommand->result == bg_err_out_of_memory) || (ret_writeCommand->result == bg_err_wrong_state));

                if (ret_writeCommand->result) {
                  mpsiTransportPrint("Error while attempting to send server's OTA input message (write request) ! Errno: 0x%x\n",
                                     ret_writeCommand->result);
                  client_gattProcedureState = clientProcedureState_NoState;
                }
              }
            }

            // Test if any of the previous operaitons exited with error.
            if (clientProcedureState_NoState != client_gattProcedureState) {
              smpd_otaServerInputRemainingBytes -= len;
              mpsiTransportPrint("Server's OTA input data sent successfully! Len: %d\n",
                                 smpd_otaFileLen - smpd_otaServerInputRemainingBytes);

              // Check if all available data sent or overflow.
              if (0 == smpd_otaServerInputRemainingBytes) {
                mpsiTransportPrint("OTA: Last write block is served, issue to send control and CRC.\n");
                client_gattProcedureState =
                  clientConnectedState_SendOtaControlCommit;
              } else if (smpd_otaFileLen < smpd_otaServerInputRemainingBytes) {
                mpsiTransportPrint("Error: smpd_otaFileLen: %d < smpd_otaServerInputRemainingBytes: %d\n",
                                   smpd_otaFileLen,
                                   smpd_otaServerInputRemainingBytes);
                // Exit for now.
                client_gattProcedureState = clientProcedureState_NoState;
              }

              // Need to wait for procedure complete during the write request operation
              if (SWITCHED_MULTIPROTOCOL_OTA_WRITE_REQUEST == smpd_otaServerWriteOperation) {
                bool procCompl = false;
                struct gecko_cmd_packet *p;

                // Wait for procedure complete event to move on with sending next block.
                do {
                  p = gecko_peek_event();
                  if (p) {
                    if (BGLIB_MSG_ID(p->header) == gecko_evt_gatt_procedure_completed_id) {
                      procCompl = true;
                    }
                  }
                } while (!procCompl);
              }
            }
          } while ( (client_gattProcedureState != clientProcedureState_NoState)
                    && (client_gattProcedureState != clientConnectedState_SendOtaControlCommit));

          // Break if needs to exit, fall-through to next step if commit is requested
          if (client_gattProcedureState != clientConnectedState_SendOtaControlCommit) {
            break;
          }
        }

/***** Sending OTA control (Client --> Server) *****/
        case clientConnectedState_SendOtaControlCommit: {
          // This occurs if all output data is sent to the client,
          // so that the end of transaction can be indicated.
          struct gecko_msg_gatt_write_characteristic_value_rsp_t *ret;
          uint8_t otaData = SWITCHED_MULTIPROTOCOL_OTA_DFU_COMMIT;

          ret =
            gecko_cmd_gatt_write_characteristic_value(client_remoteServer[activeServer].connection,
                                                      client_otaControlCharacteristic[activeServer],
                                                      1,
                                                      &otaData);

          if (ret->result) {
            mpsiTransportPrint("Error while attempting to send OTA control commit to server! Errno: 0x%x\n", ret->result);
          } else {
            mpsiTransportPrint("Server's OTA input control *commit* notification sent successfully!\n");
          }

          // There's nothing more to do.
          client_gattProcedureState = clientProcedureState_NoState;

          break;
        }

/***** Sending MPSI Client --> Server Control *****/
        case clientConnectedState_SendMpsiControlCommit: {
          // This occurs if all output data is sent to the client,
          // so that the end of transaction can be indicated.
          struct gecko_msg_gatt_write_characteristic_value_rsp_t *ret;

          // CRC of MPSI message (CRC16-CCITT - seed: 0xFFFF).
          uint16_t mpsiCrc = crc16Stream(smpd_mpsiServerInput, smpd_mpsiServerInputActMsgLen, CRC16_START);

          smpd_mpsiServerControlInput[MPSI_CTRL_CMD] = mpsiControl_CommitWithCRC;

          // Store CRC value in LSB order.
          smpd_mpsiServerControlInput[MPSI_CTRL_PARAM]   = UINT16_TO_BYTE0(mpsiCrc);
          smpd_mpsiServerControlInput[MPSI_CTRL_PARAM + 1] = UINT16_TO_BYTE1(mpsiCrc);

          ret =
            gecko_cmd_gatt_write_characteristic_value(client_remoteServer[activeServer].connection,
                                                      client_mpsiInputControlCharacteristic[activeServer],
                                                      MPSI_CTRL_MAX_COMMAND_LEN,
                                                      smpd_mpsiServerControlInput);

          if (ret->result) {
            mpsiTransportPrint("Error while attempting to send control commit to server! Errno: 0x%x\n", ret->result);
          } else {
            mpsiTransportPrint("Server's input control *commit* notification sent successfully!\n");
          }

          // Wait until ACK received.
          client_gattProcedureState = clientConnectedState_WaitMpsiControlCommit;
          break;
        }

/***** Wait MPSI Client --> Server Control confirmation *****/
        case clientConnectedState_WaitMpsiControlCommit:
          if (procedureResult) {
            mpsiTransportPrint("Input control *commit* notification returned error: %d!\n", procedureResult);
            bleMpsiTransportLongMessageSendingCrcFail();
          } else {
            mpsiTransportPrint("Input control *commit* notification ACKed successfully by the server!\n");
            bleMpsiTransportLongMessageSent();
          }

          // There's nothing more to do.
          client_gattProcedureState = clientProcedureState_NoState;
          break;

/***** Read MPSI Server --> Client Message *****/
        case clientConnectedState_ReadMpsiOutputMessage: {
          // Start ask for server's outgoing data.
          struct gecko_msg_gatt_read_characteristic_value_rsp_t *ret;
          ret =
            gecko_cmd_gatt_read_characteristic_value(client_remoteServer[activeServer].connection,
                                                     client_mpsiOutputMessageCharacteristic[activeServer]);

          if (ret->result) {
            mpsiTransportPrint("Error while attempting to send characteristic value read request to server! Errno: 0x%x\n",
                               ret->result);
            client_gattProcedureState = clientProcedureState_NoState;
          } else {
            mpsiTransportPrint("Characteristic read request is sent successfully to server!\n");
            client_gattProcedureState = clientConnectedState_WaitMpsiOutputMessage;
          }

          break;
        }

/***** Wait MPSI Server --> Client Message *****/
        case clientConnectedState_WaitMpsiOutputMessage:
          mpsiTransportPrint("Wait MPSI Output read...");
          break;

/***** Wait MPSI Server --> Client Confirmation  *****/
        case clientConnectedState_SendMpsiOutputControlCommitConfirmation:
          // All set, we can go and rest for a while.
          client_gattProcedureState = clientProcedureState_NoState;
          break;

        case clientProcedureState_NoState:
        default:
          break;
      }
      break;
    }

/*****************************
 *     Characteristic value
 ****************************/
    case gecko_evt_gatt_characteristic_value_id: {
      uint8_t   connection      = evt->data.evt_gatt_characteristic_value.connection;
      uint16_t  characteristic  = evt->data.evt_gatt_characteristic_value.characteristic;
      uint8_t   attOpCode       = evt->data.evt_gatt_characteristic_value.att_opcode;
      uint8_t   *charValueData  = evt->data.evt_gatt_characteristic_value.value.data;

      // Operation mode for this event
      // 0 - Nothing to do
      // 1 - Proper indication is received from a connected device
      // 2 - Proper read response is received from a connected device
      uint8_t opMode = 0;

      // Select opmode and change active server according to the event parameters
      // This is used to track which is the active connection that we are operating on.
      if (client_remoteServer[serverNum_Smpd].connection == connection) {
        if ((client_mpsiOutputControlCharacteristic[serverNum_Smpd] == characteristic)
            && (gatt_handle_value_indication == attOpCode)) {
          opMode = 1;
          activeServer = serverNum_Smpd;
        } else if ((client_mpsiOutputMessageCharacteristic[serverNum_Smpd] == characteristic)
                   && (gatt_read_response == attOpCode)) {
          opMode = 2;
          activeServer = serverNum_Smpd;
        }
      } else if (client_remoteServer[serverNum_Tc].connection == connection) {
        if ((client_mpsiOutputControlCharacteristic[serverNum_Tc] == characteristic)
            && (gatt_handle_value_indication == attOpCode)) {
          opMode = 1;
          activeServer = serverNum_Tc;
        } else if ((client_mpsiOutputMessageCharacteristic[serverNum_Tc] == characteristic)
                   && (gatt_read_response == attOpCode)) {
          opMode = 2;
          activeServer = serverNum_Tc;
        }
      }

/*****  MPSI Server --> Client Control - Begin transaction *****/
      // Indication is received from Joining Device output control characteristic.
      if (1 == opMode) {
        struct gecko_msg_gatt_send_characteristic_confirmation_rsp_t *ret;

        // Check if MPSI transaction is just started.
        if (mpsiControl_BeginTransaction == charValueData[MPSI_CTRL_CMD]) {
          // Only accept starting transaction if no other procedure is still running.
          if (clientProcedureState_NoState == client_gattProcedureState) {
            // Restart MPSI counter.
            smpd_mpsiServerOutputCntr = 0;

            // Clear buffer.
            memset(smpd_mpsiServerOutput, '\0', MPSI_DATA_MAX_MESSAGE_LEN);

            client_gattProcedureState = clientConnectedState_ReadMpsiOutputMessage;

            ret = gecko_cmd_gatt_send_characteristic_confirmation(connection);

            if (ret->result) {
              mpsiTransportPrint("Error while sending characteristic confirmation for mpsiControl_BeginTransaction! Errno: 0x%x\n",
                                 ret->result);
            } else {
              mpsiTransportPrint("Characteristic confirmation for mpsiControl_BeginTransaction is sent successfully!\n");

              {
                // Start ask for server's outgoing data.
                struct gecko_msg_gatt_read_characteristic_value_rsp_t *ret;

                ret =
                  gecko_cmd_gatt_read_characteristic_value(client_remoteServer[activeServer].connection,
                                                           client_mpsiOutputMessageCharacteristic[activeServer]);

                if (ret->result) {
                  mpsiTransportPrint("Error while attempting to send characteristic value read request to server! Errno: 0x%x\n",
                                     ret->result);
                  client_gattProcedureState = clientProcedureState_NoState;
                } else {
                  mpsiTransportPrint("Characteristic read request is sent successfully to server!\n");
                  client_gattProcedureState = clientConnectedState_WaitMpsiOutputMessage;
                }
              }
            }
          } else {
            mpsiTransportPrint("Error! Cannot send characteristic confirmation, other procedure is still running. client_gattProcedureState: 0x%x",
                               client_gattProcedureState);
          }

/*****  MPSI Server --> Client Control - Commit transaction *****/
        } else if (mpsiControl_CommitWithCRC == charValueData[MPSI_CTRL_CMD]) {
          uint16_t mpsiCrc;
          uint16_t i;
          bool mpsiCrcOk = false;

          client_gattProcedureState = clientProcedureState_NoState;

          // CRC of MPSI message in LSD format (CRC16-CCITT - seed: 0xFFFF).
          mpsiCrc = BYTES_TO_UINT16((uint16_t)charValueData[MPSI_CTRL_PARAM], (uint16_t)charValueData[MPSI_CTRL_PARAM + 1]);

          // Print out received MPSI message
          mpsiTransportPrint("MPSI message arrived - len: %d, CRC: 0x%04x, msg:", smpd_mpsiServerOutputCntr, mpsiCrc);
          for (i = 0; i < smpd_mpsiServerOutputCntr; i++) {
            printf(" 0x%02x", smpd_mpsiServerOutput[i]);
          }
          printf("\n");

          // Calculate and test CRC of the input message
          if (mpsiCrc == crc16Stream(smpd_mpsiServerOutput, smpd_mpsiServerOutputCntr, CRC16_START)) {
            mpsiCrcOk = true;
          } else {
            bleMpsiTransportLongMessageReceptionCrcFail();
          }

          // Previous query must be repeated in this case by the client again.
          ret = gecko_cmd_gatt_send_characteristic_confirmation(connection);

          if (ret->result) {
            mpsiTransportPrint("Error while sending characteristic confirmation for mpsiControl_CommitWithCRC! Errno: 0x%x\n",
                               ret->result);
          } else {
            mpsiTransportPrint("Characteristic confirmation for mpsiControl_CommitWithCRC is sent successfully!\n");
            if (mpsiCrcOk) {
              bleMpsiTransportLongMessageReceived(smpd_mpsiServerOutput, smpd_mpsiServerOutputCntr);
            }
          }
        } else {
          mpsiTransportPrint("Error, unknown MPSI control message command!\n");
          client_gattProcedureState = clientProcedureState_NoState;

          // Send back confirmation (as no error is possible) to the client.
          gecko_cmd_gatt_send_characteristic_confirmation(connection);
        }

/*****  MPSI Server --> Client Data Message *****/
      } else if (2 == opMode) {
        uint16_t len = evt->data.evt_gatt_characteristic_value.value.len;

        // Prevent input buffer overflow
        if (MPSI_DATA_MAX_MESSAGE_LEN < smpd_mpsiServerOutputCntr + len) {
          // Client cannot send back error to the server with this procedure.
          mpsiTransportPrint("Error: too much data is received from the server. smpd_mpsiServerOutputCntr: 0x%x + len: 0x%x\n",
                             smpd_mpsiServerOutputCntr,
                             len);
        } else {
          uint16_t mpsiMaxActMsgLen;

          memcpy( (uint8_t*)&smpd_mpsiServerOutput[smpd_mpsiServerOutputCntr], charValueData, len);
          smpd_mpsiServerOutputCntr += len;
          mpsiTransportPrint("MPSI data read from server. Len: %d\n", smpd_mpsiServerOutputCntr);

          // Calculate the max length for this MPSI message
          mpsiMaxActMsgLen = (uint16_t)smpd_mpsiServerOutput[MPSI_MESSAGE_PAYLOAD_LEN_LOC] + MPSI_MESSAGE_PAYLOAD_LEN_LOC + 1;

          if (smpd_mpsiServerOutputCntr < mpsiMaxActMsgLen) {
            client_gattProcedureState = clientConnectedState_ReadMpsiOutputMessage;
          } else if (smpd_mpsiServerOutputCntr == mpsiMaxActMsgLen) {
            mpsiTransportPrint("All data was read from server successfully. Done.\n");
            client_gattProcedureState = clientProcedureState_NoState;
          } else {
            mpsiTransportPrint("That's too bad. Too much data was read from server...\n");
            client_gattProcedureState = clientProcedureState_NoState;
          }

          // Cannot send back response. See above.
        }
      }
      break;
    }

    default:
      break;
  }
}
