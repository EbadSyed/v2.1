#include COMMON_HEADER
#include "gatt_db.h"
#include "crc16.h"
#include "mpsi_ble_transport_server.h"

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

// MPSI control command parameter values
typedef enum {
  mpsiControl_BeginTransaction = 1u,
  mpsiControl_CommitWithCRC = 2u
} MpsiControlValues_t;

// MPSI control command parameter values
typedef enum {
  mpsiEcode_NoError = 0x00,
  mpsiControlEcode_UnknownControlCommand  = 0x01,
  mpsiControlEcode_TransactionBeginError  = 0x01,

  mpsiDataEcode_ControlIsNotSet       = 0x11,
  mpsiDataEcode_TooMuchDataSent       = 0x12,
  mpsiDataEcode_TooMuchDataRequested  = 0x13,
  mpsiDataEcode_CrcError              = 0x14,

  mpsiOtaDfuEcode_DataReceiveCallbackError = 0x21,
} MpsiEcodes_t;

// Remote address of Switched Multi-protocol Device
// Be aware, it's LSB
bd_addr smpd_remoteAddress = { { 0x48, 0xab, 0x07, 0x57, 0x0b, 0x00 } };

// Server booted flag
bool server_booted = false;

// MPSI operation states
typedef enum {
  mpsiState_NoState = 0u,
  mpsiState_SendReadResponse = 1u,
  mpsiState_LastReadResponse = 2u,
} MpsiOperationState_t;

MpsiOperationState_t mpsiOpState = mpsiState_NoState;

// SorfTimer enum extension for BLE API
typedef enum {
  BleSoftTimer_RunForever = 0u,
  BleSoftTimer_OneShot  = 1u,
} BleSoftTimerFunction_t;

// Security flag enum extension for BLE API
typedef enum {
  BleSecurityFlag_BondRequiresMitm          = 0x01,
  BleSecurityFlag_EnryptionRequiresBonding  = 0x02,
  BleSecurityFlag_SecureConnectionOnly      = 0x04,
  BleSecurityFlag_NeedToConfirmBonding      = 0x08,
} BleSecurityFlags_t;

#define BLE_SOFT_TIMER_SEC 32768
#define BLE_SOFT_TIMER_MIN (60 * BLE_SOFT_TIMER_SEC)

#define BLE_OPCODE_GATT_WRITE_REQUEST 18
#define BLE_OPCODE_GATT_WRITE_COMMAND 82

#define SOFT_TIMER_ID_ACCEPT_ANY_CONNECTION 0x10

#define SOFT_TIMER_ADVERTISE_DEFAULT_TIMEOUT 10

uint8_t   mpsiInput[MPSI_DATA_MAX_MESSAGE_LEN];
uint16_t  mpsiInputCntr = 0;
bool      mpsiInputInProgress = false;

uint8_t   mpsiOutput[MPSI_DATA_MAX_MESSAGE_LEN];
uint16_t  mpsiOutputCntr = 0;
uint16_t  mpsiOutputActMsgLen = 0;
uint8_t   mpsiControlOut[MPSI_CTRL_MAX_COMMAND_LEN];

// Remote client connection properties
typedef struct {
  bool isConnected;
  uint8_t connection;
#if defined(APP_ENABLE_AUTHENTICATION)
  bool isBonded;
  bool isBondable;
#endif
} ServerRemoteConnection_t;

ServerRemoteConnection_t server_remoteClient = {
  false, 0x00,
#if defined(APP_ENABLE_AUTHENTICATION)
  false, false,
#endif
};

// OTA DFU block size
#define SWITCHED_MULTIPROTOCOL_OTA_DFU_START        0x00
#define SWITCHED_MULTIPROTOCOL_OTA_DFU_COMMIT       0x03

uint32_t ota_data_len;

void sendBleTestData(uint8_t dataNum)
{
  // Notify client that we have something for it.
  uint16_t i;
  struct gecko_msg_gatt_server_send_characteristic_notification_rsp_t *ret;

  mpsiTransportPrint("Sending message is requested.\n");

  // Fill output buffer before sending out indication.
  for (i = 0; i < MPSI_DATA_MAX_MESSAGE_LEN; i++) {
    mpsiOutput[i] = (dataNum) ? i : (MPSI_DATA_MAX_MESSAGE_LEN - i);
  }

  // Set message length
  mpsiOutputActMsgLen = 259;

  // Do not start sending data if too much is requested.
  if (MPSI_DATA_MAX_MESSAGE_LEN < mpsiOutputActMsgLen) {
    mpsiTransportPrint("Error: MPSI_DATA_MAX_MESSAGE_LEN:%d < mpsiOutputActMsgLen:%d", MPSI_DATA_MAX_MESSAGE_LEN, mpsiOutputActMsgLen);
  } else {
    // Set payload length field
    mpsiOutput[MPSI_MESSAGE_PAYLOAD_LEN_LOC] = mpsiOutputActMsgLen - 4;

    // Clear output byte counter
    mpsiOutputCntr = 0;

/*****  MPSI Output Control  *****/
    // Send indication that we have something to read.
    mpsiControlOut[MPSI_CTRL_CMD] = mpsiControl_BeginTransaction;
    ret = gecko_cmd_gatt_server_send_characteristic_notification(0xFF, gattdb_mpsi_control_out, MPSI_CTRL_MAX_COMMAND_LEN, mpsiControlOut);

    if (ret->result) {
      mpsiTransportPrint("Error while sending gattdb_mpsi_control_out notification! Errno: 0x%x\n", ret->result);
    } else {
      mpsiTransportPrint("Output control *begin* notification sent successfully!\n");
    }
  }
}

int8_t bleSendMpsiTransportLongMessage(uint8_t *buffer, uint16_t len)
{
  // Notify client that we have something for it.
  struct gecko_msg_gatt_server_send_characteristic_notification_rsp_t *ret;

  mpsiTransportPrint("Sending MPSI message is requested. Len: %d\n", len);

  // Copy buffer to send before sending out indication.
  memcpy(mpsiOutput, buffer, len);

  // Store act message length to use it later.
  mpsiOutputActMsgLen = len;

  // Do not start sending data if too much is requested.
  if (MPSI_DATA_MAX_MESSAGE_LEN < mpsiOutputActMsgLen) {
    mpsiTransportPrint("Error: MPSI_DATA_MAX_MESSAGE_LEN:%d < mpsiOutputActMsgLen:%d",
                       MPSI_DATA_MAX_MESSAGE_LEN,
                       mpsiOutputActMsgLen);
    return MPSI_TRANSPORT_ERROR;
  } else {
    // Clear output byte counter
    mpsiOutputCntr = 0;

/*****  MPSI Output Control  *****/
    // Send indication that we have something to read.
    mpsiControlOut[MPSI_CTRL_CMD] = mpsiControl_BeginTransaction;
    ret = gecko_cmd_gatt_server_send_characteristic_notification(0xFF,
                                                                 gattdb_mpsi_control_out,
                                                                 MPSI_CTRL_MAX_COMMAND_LEN,
                                                                 mpsiControlOut);
    if (ret->result) {
      mpsiTransportPrint("Error while sending gattdb_mpsi_control_out notification! Errno: 0x%x\n", ret->result);
      return MPSI_TRANSPORT_ERROR;
    } else {
      mpsiTransportPrint("Output control *begin* notification sent successfully!\n");
    }
  }

  return MPSI_TRANSPORT_SUCCESS;
}

void bleStartAdvertisement(void)
{
  // Start general advertising and enable connections.
  gecko_cmd_le_gap_set_mode(le_gap_general_discoverable, le_gap_undirected_connectable);
  mpsiTransportPrint("Server is discoverable/connectable.\n");
}

void bleStopAdvertisement(void)
{
  // Start general advertising and enable connections.
  gecko_cmd_le_gap_set_mode(le_gap_non_discoverable, le_gap_non_connectable);
  mpsiTransportPrint("Server is not discoverable/connectable.\n");
}

void bleDisconnect(void)
{
  gecko_cmd_endpoint_close(server_remoteClient.connection);
}

#if defined(APP_ENABLE_AUTHENTICATION)
void bleSetTimeoutForAcceptingAllConnections(uint8_t timeoutMin)
{
  mpsiTransportPrint("Server is bondable.\n");
  mpsiTransportPrint("Accepting non-bonded connections for %d minute(s).\n", timeoutMin);
  // Start soft timer with timeout.
  gecko_cmd_hardware_set_soft_timer(BLE_SOFT_TIMER_MIN * timeoutMin,
                                    SOFT_TIMER_ID_ACCEPT_ANY_CONNECTION,
                                    BleSoftTimer_OneShot);
  gecko_cmd_sm_set_bondable_mode(true);
  server_remoteClient.isBondable = true;
}
#endif

void mpsiTransportHandleEvents(struct gecko_cmd_packet *evt)
{
  // Do not try to process NULL event.
  if (NULL == evt) {
    return;
  }

  // Do not handle any events until system is booted up properly.
  if ((BGLIB_MSG_ID(evt->header) != gecko_evt_system_boot_id)
      && !server_booted) {
#if defined(DEBUG)
    printf("Event: 0x%04x\n", BGLIB_MSG_ID(evt->header));
#endif
#if defined(BLE_NCP_HOST)
    usleep(50000);
#endif // BLE_NCP_HOST
    return;
  }

  // Handle events
  switch (BGLIB_MSG_ID(evt->header)) {
/************************
 *  Soft Timer
 ***********************/
    case gecko_evt_hardware_soft_timer_id:
      if (SOFT_TIMER_ID_ACCEPT_ANY_CONNECTION == evt->data.evt_hardware_soft_timer.handle) {
        // Do not accept new bondings.
        gecko_cmd_sm_set_bondable_mode(false);
        server_remoteClient.isBondable = false;
      }
      break;

/************************
 *  Boot
 ***********************/

    // This boot event is generated when the system boots up after reset.
    // Here the system is set to configure security parameters if set and to
    // start advertising immediately after boot procedure.
    case gecko_evt_system_boot_id: {
      struct gecko_msg_system_get_bt_address_rsp_t *rsp_bt_addr;
      uint16_t devId;
      char devName[APP_DEVNAME_LEN + 1];
      char temp[6];

      printf("\n");
      mpsiTransportPrint("Server booted!\n");
      server_booted = true;

      // Read out local address
      rsp_bt_addr = gecko_cmd_system_get_bt_address();
      mpsiTransportPrint("Local address:%02x:%02x:%02x:%02x:%02x:%02x\n",
                         rsp_bt_addr->address.addr[5],
                         rsp_bt_addr->address.addr[4],
                         rsp_bt_addr->address.addr[3],
                         rsp_bt_addr->address.addr[2],
                         rsp_bt_addr->address.addr[1],
                         rsp_bt_addr->address.addr[0]);

      // Create and set the device name based on the 16-bit device ID
      devId = *((uint16*)(rsp_bt_addr->address.addr));
      snprintf(temp, 6, "%05u", devId);
      snprintf(devName, APP_DEVNAME_LEN + 1, APP_DEVNAME, &temp[1]);
      gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name,
                                                  0,
                                                  strlen(devName),
                                                  (uint8_t *)devName);

      // Set advertising parameters.
      gecko_cmd_le_gap_set_adv_parameters(160, 160, 7);

#if defined(APP_REMOVE_ALL_BONDING_AT_STARTUP)
      gecko_cmd_sm_delete_bondings();
      mpsiTransportPrint("HEADS UP! Removing all bondings!\n");
#endif // APP_REMOVE_ALL_BONDING_AT_STARTUP

#if defined(APP_ENABLE_AUTHENTICATION)
      // Configure security manager
      gecko_cmd_sm_configure((BleSecurityFlag_BondRequiresMitm
                              | BleSecurityFlag_EnryptionRequiresBonding
                              | BleSecurityFlag_NeedToConfirmBonding),
                             sm_io_capability_displayyesno);

      mpsiTransportPrint("Server: Security Manger is configured.\n");

      // Set maximum number of bondings, also new bonding are dropped if max is reached.
      gecko_cmd_sm_store_bonding_configuration(APP_MAX_NUMBER_OF_BONDINGS, (uint8_t)false);
      mpsiTransportPrint("Server: Set max number of bondings: %d.\n", APP_MAX_NUMBER_OF_BONDINGS);

      // Set default timeout for accepting non-bonded connections.
      bleSetTimeoutForAcceptingAllConnections(SOFT_TIMER_ADVERTISE_DEFAULT_TIMEOUT);
#endif
      // Start advertisement
      bleStartAdvertisement();
      break;
    }

/************************
 * Connection
 ***********************/
    case gecko_evt_le_connection_opened_id:
      mpsiTransportPrint("Connection opened: %d\n", (int16_t)evt->data.evt_le_connection_opened.connection);

      // Do not allow new connections if one already exists
      if (server_remoteClient.isConnected) {
        mpsiTransportPrint("Connection dropped: %d. New connection is not allowed!\n",
                           (int16_t)evt->data.evt_le_connection_opened.connection);
        gecko_cmd_endpoint_close((int16_t)evt->data.evt_le_connection_opened.connection);
      } else {
        server_remoteClient.connection = (int16_t)evt->data.evt_le_connection_opened.connection;
        server_remoteClient.isConnected = true;

        if (0xFF == evt->data.evt_le_connection_opened.bonding) {
          mpsiTransportPrint("Remote device is not bonded.\n");
#if defined(APP_ENABLE_AUTHENTICATION)
          // Drop connection if non-bonded connection is not allowed.
          if (!server_remoteClient.isBondable) {
            mpsiTransportPrint("Connection dropped: %d. Non-bonded connection is not allowed!\n",
                               (int16_t)evt->data.evt_le_connection_opened.connection);
            gecko_cmd_endpoint_close(server_remoteClient.connection);
          }
#endif
        } else {
          mpsiTransportPrint("Server: Already bonded to remote device!\n");
#if defined(APP_ENABLE_AUTHENTICATION)
          server_remoteClient.isBonded = true;
#endif
        }
      }
      // Restart advertisement
      bleStartAdvertisement();
      break;

    case gecko_evt_le_connection_closed_id:
      // Close connection properly if this was the active connection
      if (server_remoteClient.connection == evt->data.evt_le_connection_closed.connection) {
        server_remoteClient.isConnected = false;
#if defined(APP_ENABLE_AUTHENTICATION)
        server_remoteClient.isBonded = false;
#endif
      }

      mpsiTransportPrint("Connection closed!\n  - Connection: %d\n  - Reason: 0x%04x\n",
                         evt->data.evt_le_connection_closed.connection,
                         evt->data.evt_le_connection_closed.reason);

      // Restart advertisement
      bleStartAdvertisement();
      break;

#if defined(APP_ENABLE_AUTHENTICATION)
/************************
 *  Passkey
 ***********************/
    case gecko_evt_sm_passkey_display_id:
      mpsiTransportPrint("Server: Displaying passkey: %6d\n", evt->data.evt_sm_passkey_display.passkey);
      break;

    case gecko_evt_sm_passkey_request_id:
      mpsiTransportPrint("Server: Requesting passkey...\n");
      gecko_cmd_sm_enter_passkey(evt->data.evt_sm_passkey_request.connection, 42u);
      break;

    case gecko_evt_sm_confirm_passkey_id:
#if defined(BLE_TRANSPORT_AUTO_SECURITY_CONNECTION)
      mpsiTransportPrint("Server: Auto confirm passkey: %d\n", evt->data.evt_sm_confirm_passkey.passkey);
      gecko_cmd_sm_passkey_confirm(evt->data.evt_sm_confirm_bonding.connection, 1u);
#else // BLE_TRANSPORT_AUTO_SECURITY_CONNECTION
      bleMpsiTransportConfirmPasskey(evt);
#endif // BLE_TRANSPORT_AUTO_SECURITY_CONNECTION
      break;

/************************
 *   Bonding
 ***********************/
    case gecko_evt_sm_confirm_bonding_id:
#if defined(BLE_TRANSPORT_AUTO_SECURITY_CONNECTION)
      mpsiTransportPrint("Server: Auto confirming bonding...\n");
      gecko_cmd_sm_bonding_confirm(evt->data.evt_sm_confirm_bonding.connection, 1u);
#else // BLE_TRANSPORT_AUTO_SECURITY_CONNECTION
      bleMpsiTransportConfirmBonding(evt);
#endif // BLE_TRANSPORT_AUTO_SECURITY_CONNECTION
      break;

    case gecko_evt_sm_bonded_id:
      mpsiTransportPrint("Server: Bonded! Data: %d\n", evt->data.evt_sm_bonded.bonding);
      server_remoteClient.isBonded = true;
      break;

    case gecko_evt_sm_bonding_failed_id:
      server_remoteClient.isBonded = false;
#if defined(BLE_NCP_HOST)
      ble_errorExit("Error, bonding failed. Errno: 0x%x\n Exiting...", evt->data.evt_sm_bonding_failed.reason);
#else
      mpsiTransportPrint("Error, bonding failed. Errno: 0x%x\n", evt->data.evt_sm_bonding_failed.reason);
#endif
      break;
#endif // APP_ENABLE_AUTHENTICATION

/***************************
 *   User Read Request
 **************************/
    case gecko_evt_gatt_server_user_read_request_id: {
      uint32_t connection = evt->data.evt_gatt_server_user_read_request.connection;
      uint32_t characteristic = evt->data.evt_gatt_server_user_read_request.characteristic;
      struct gecko_msg_gatt_server_send_user_read_response_rsp_t *ret;

#if defined(APP_ENABLE_AUTHENTICATION)
      // Only allow any operation if connected client is boneded
      if (server_remoteClient.isBonded) {
#endif

/*****  MPSI Output Message  *****/
      if (gattdb_mpsi_message_out == characteristic) {
        uint16_t len = (mpsiOutputActMsgLen - mpsiOutputCntr > (36))
                       ? (36)
                       : (mpsiOutputActMsgLen - mpsiOutputCntr);

        // Check if data would overflow of output buffer
        if (MPSI_DATA_MAX_MESSAGE_LEN < mpsiOutputCntr + len) {
          mpsiTransportPrint("Error: Attempt to send too much data! This should not happen...\n");
        } else {
          ret =
            gecko_cmd_gatt_server_send_user_read_response(connection,
                                                          characteristic,
                                                          mpsiEcode_NoError,
                                                          len,
                                                          (uint8_t*) &mpsiOutput[mpsiOutputCntr]);
          mpsiOpState = mpsiState_SendReadResponse;

          if (ret->result) {
            mpsiTransportPrint("Error while sending gattdb_mpsi_data_out! Errno: 0x%x\n", ret->result);
            mpsiTransportPrint("Connection: %d\nCharacteristic: %d\nLen: %d...", connection, characteristic, len);
          } else {
            mpsiOutputCntr += len;
            mpsiTransportPrint("Output data sent successfully! Len: %d\n", mpsiOutputCntr);

            // Check if all available data sent or overflow.
            if (mpsiOutputActMsgLen == mpsiOutputCntr) {
              // This occurs if all output data is sent to the client,
              // so that the end of transaction can be indicated.
              struct gecko_msg_gatt_server_send_characteristic_notification_rsp_t *ret;
              // CRC of MPSI message (CRC16-CCITT - seed: 0xFFFF).
              uint16_t mpsiCrc = crc16Stream(mpsiOutput, mpsiOutputActMsgLen, CRC16_START);

              mpsiTransportPrint("Last read request is served, issue to send indication.\n");

              mpsiControlOut[MPSI_CTRL_CMD] = mpsiControl_CommitWithCRC;

              // Store CRC value in LSB order.
              mpsiControlOut[MPSI_CTRL_PARAM]   = UINT16_TO_BYTE0(mpsiCrc);
              mpsiControlOut[MPSI_CTRL_PARAM + 1] = UINT16_TO_BYTE1(mpsiCrc);

              ret = gecko_cmd_gatt_server_send_characteristic_notification(0xFF, gattdb_mpsi_control_out, MPSI_CTRL_MAX_COMMAND_LEN, mpsiControlOut);

              if (ret->result) {
                mpsiTransportPrint("Error while sending gattdb_mpsi_control_out notification! Errno: 0x%x\n", ret->result);
              } else {
                mpsiTransportPrint("Output control *commit* notification sent successfully!\n");
              }

              mpsiTransportPrint("ProcComp. OK\n");
              mpsiOpState = mpsiState_NoState;
            } else if (mpsiOutputActMsgLen < mpsiOutputCntr) {
              // Thats too bad, something went wrong, shouldn't be here.
              mpsiTransportPrint("Error: mpsiOutputActMsgLen: %d < mpsiOutputCntr: %d\n", mpsiOutputActMsgLen, mpsiOutputCntr);
            }
          }
        }
      }
#if defined(APP_ENABLE_AUTHENTICATION)
    } else {
        mpsiTransportPrint("Operation is not allowed! Client is not bonded!\n");
    }
#endif
      break;
    }

/***************************
 *  User Write Request
 **************************/
    case  gecko_evt_gatt_server_user_write_request_id: {
      uint32_t  connection      = evt->data.evt_gatt_server_user_write_request.connection;
      uint32_t  characteristic  = evt->data.evt_gatt_server_user_write_request.characteristic;
      uint8_t   *charValueData  = evt->data.evt_gatt_server_user_write_request.value.data;
      struct    gecko_msg_gatt_server_send_user_write_response_rsp_t *ret;

#if defined(APP_ENABLE_AUTHENTICATION)
      // Only allow any operation if connected client is boneded
      if (server_remoteClient.isBonded) {
#endif

// Do not process OTA-DFU related messages for NCP host application.
#if !defined(BLE_NCP_HOST)
      uint8_t writeRequestAttOpcode = evt->data.evt_gatt_server_user_write_request.att_opcode;
      uint8_t charLen = evt->data.evt_gatt_server_user_write_request.value.len;

/*****  OTA Control  *****/
      if (characteristic == gattdb_ota_control) {
        MpsiEcodes_t responseVal = mpsiEcode_NoError;
        uint8_t otaDfuCommand = charValueData[0];
        uint8_t *otaDfuCommandVal = (uint8_t*)&charValueData[1];
        uint8_t otaDfuCommandValLen = (0 == charLen) ? (charLen) : (charLen - 1);

        switch (otaDfuCommand) {
          case SWITCHED_MULTIPROTOCOL_OTA_DFU_START:
            mpsiTransportPrint("OTA-DFU transaction started!\n");
            ota_data_len = 0;
            if (MPSI_TRANSPORT_SUCCESS
                != bleMpsiTransportOtaDfuTransactionBegin(otaDfuCommandValLen, otaDfuCommandVal)) {
              responseVal = mpsiControlEcode_TransactionBeginError;
            }
            break;

          case SWITCHED_MULTIPROTOCOL_OTA_DFU_COMMIT:
            mpsiTransportPrint("OTA-DFU transaction finished!\n");
            bleMpsiTransportOtaDfuTransactionFinish(ota_data_len);
            break;

          default:
            responseVal = mpsiControlEcode_UnknownControlCommand;
            mpsiTransportPrint("OTA-DFU ERROR: Unknown parameter!\n");
            break;
        }

        ret = gecko_cmd_gatt_server_send_user_write_response(connection, characteristic, responseVal);

        if (ret->result) {
          mpsiTransportPrint("Error while sending write response for OTA Control ! Errno: 0x%x\n", ret->result);
        } else {
          mpsiTransportPrint("Write response for OTA Control is sent successfully!\n");
        }

/*****  OTA Data  *****/
      } else if (characteristic == gattdb_ota_data) {
        MpsiEcodes_t responseVal = mpsiEcode_NoError;

        ota_data_len += charLen;
        mpsiTransportPrint("OTA-DFU data - Total: %d - length: %d\n", ota_data_len, charLen);

        if (MPSI_TRANSPORT_SUCCESS != bleMpsiTransportOtaDfuDataReceived(charLen, charValueData)) {
          responseVal = mpsiOtaDfuEcode_DataReceiveCallbackError;
        }

        // Send back response in case of data was sent as a write request
        if (BLE_OPCODE_GATT_WRITE_REQUEST == writeRequestAttOpcode) {
          ret = gecko_cmd_gatt_server_send_user_write_response(connection, characteristic, responseVal);

          if (ret->result) {
            mpsiTransportPrint("Error while sending write response for OTA Data! Errno: 0x%x\n", ret->result);
          } else {
            //mpsiTransportPrint("Write response for OTA Data  is sent successfully!\n");
          }
        }
      } else
#endif // !defined(BLE_NCP_HOST)

/*****  MPSI Input Control - Begin Transaction  *****/
      if (characteristic == gattdb_mpsi_control_in) { // MPSI control input
        uint16_t i;

        if (mpsiControl_BeginTransaction == charValueData[MPSI_CTRL_CMD]) {
          // Restart MPSI counter.
          mpsiInputCntr = 0;

          // Clear buffer.
          memset(mpsiInput, '\0', MPSI_DATA_MAX_MESSAGE_LEN);

          // Enable MPSI input.
          mpsiInputInProgress = true;

          ret = gecko_cmd_gatt_server_send_user_write_response(connection, characteristic, mpsiEcode_NoError);

          if (ret->result) {
            mpsiTransportPrint("Error while sending write response for mpsiControl_BeginTransaction! Errno: 0x%x\n", ret->result);
          } else {
            mpsiTransportPrint("Write response for mpsiControl_BeginTransaction is sent successfully!\n");
          }

/*****  MPSI Input Control - Commit Transaction  *****/
        } else if (mpsiControl_CommitWithCRC == charValueData[MPSI_CTRL_CMD]) {
          uint16_t mpsiCrc;
          bool mpsiCrcOk = false;

          // Disable MPSI input.
          mpsiInputInProgress = false;

          // CRC of MPSI message in LSD format (CRC16-CCITT - seed: 0xFFFF).
          mpsiCrc = BYTES_TO_UINT16((uint16_t)charValueData[MPSI_CTRL_PARAM], (uint16_t)charValueData[MPSI_CTRL_PARAM + 1]);

          // Print out received MPSI message
          mpsiTransportPrint("MPSI message arrived - len: %d, CRC: 0x%04x, msg:", mpsiInputCntr, mpsiCrc);
          for (i = 0; i < mpsiInputCntr; i++) {
            printf(" 0x%02x", mpsiInput[i]);
          }
          printf("\n");

          // Calculate and test CRC of the input message
          if (mpsiCrc == crc16Stream(mpsiInput, mpsiInputCntr, CRC16_START)) {
            mpsiCrcOk = true;
            ret = gecko_cmd_gatt_server_send_user_write_response(connection, characteristic, mpsiEcode_NoError);
          } else {
            bleMpsiTransportLongMessageReceptionCrcFail();
            ret = gecko_cmd_gatt_server_send_user_write_response(connection, characteristic, mpsiDataEcode_CrcError);
          }

          if (ret->result) {
            mpsiTransportPrint("Error while sending write response for *mpsiControl_CommitWithCRC*! Errno: 0x%x\n", ret->result);
          } else {
            mpsiTransportPrint("Write response for *mpsiControl_CommitWithCRC* is sent successfully!\n");

            if (mpsiCrcOk) {
              bleMpsiTransportLongMessageReceived(mpsiInput, mpsiInputCntr);
            }
          }
        } else {
          mpsiTransportPrint("Error, unknown MPSI control message command!\n");
          // Send back error for the client as well.
          gecko_cmd_gatt_server_send_user_write_response(connection, characteristic, mpsiControlEcode_UnknownControlCommand);
        }

/*****  MPSI Input Message  *****/
      } else if (characteristic == gattdb_mpsi_message_in) { // MPSI message input
        // If input is in progress write new values to the buffer.
        if (mpsiInputInProgress) {
          uint16_t len = evt->data.evt_gatt_server_user_write_request.value.len;

          // Prevent input buffer overflow
          if (MPSI_DATA_MAX_MESSAGE_LEN < mpsiInputCntr + len) {
            gecko_cmd_gatt_server_send_user_write_response(connection, characteristic, mpsiDataEcode_TooMuchDataSent);
            mpsiTransportPrint("Error: too much data is received from client. Sending back mpsiDataEcode_TooMuchDataSent");
          } else {
            memcpy( (uint8_t*)&mpsiInput[mpsiInputCntr], charValueData, len);
            mpsiInputCntr += len;

            gecko_cmd_gatt_server_send_user_write_response(connection, characteristic, mpsiEcode_NoError);
            mpsiTransportPrint("Data is received from client successfully. Len: %d\n", mpsiInputCntr);
          }
        } else {
          mpsiTransportPrint("Error! Control value wasn't set before changing characteristic.\n");
          // Send back error for the client as well.
          gecko_cmd_gatt_server_send_user_write_response(connection, characteristic, mpsiDataEcode_ControlIsNotSet);
        }
      } else {
        mpsiTransportPrint("Characteristic write request received: %d\n", characteristic);
      }

#if defined(APP_ENABLE_AUTHENTICATION)
    } else {
        mpsiTransportPrint("Operation is not allowed! Client is not bonded!\n");
    }
#endif

      break;
    }

/************************
 *  Attribute Value
 ***********************/
    // Value of attribute changed from the local database by remote GATT client
    case gecko_evt_gatt_server_attribute_value_id:

#if defined(APP_ENABLE_AUTHENTICATION)
      // Only allow any operation if connected client is boneded
      if (server_remoteClient.isBonded) {
#endif

#if !defined(BLE_NCP_HOST)
/*****  OTA Data  *****/
      if (evt->data.evt_gatt_server_attribute_value.attribute == gattdb_ota_data) {
        uint8_t *charValueData = evt->data.evt_gatt_server_attribute_value.value.data;

        uint8_t len = evt->data.evt_gatt_server_user_write_request.value.len;

        ota_data_len += len;
        mpsiTransportPrint("OTA-DFU attr. data - Total: %d - length: %d\n", ota_data_len, len);

        if (MPSI_TRANSPORT_SUCCESS != bleMpsiTransportOtaDfuDataReceived(len, charValueData)) {
          mpsiTransportPrint("OTA-DFU callback error!\n");
        }
      } else
#endif // !defined(BLE_NCP_HOST)
      {
        mpsiTransportPrint("Attribute value change: att_opcode: %d",
                           evt->data.evt_gatt_server_attribute_value.att_opcode);
      }

#if defined(APP_ENABLE_AUTHENTICATION)
  } else {
    mpsiTransportPrint("Operation is not allowed! Client is not bonded!\n");
  }
#endif

      break;

    default:
      break;
  }
}
