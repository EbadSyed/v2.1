#ifndef __MPSI_BLE_TRANSPORT__
#define __MPSI_BLE_TRANSPORT__

#include COMMON_HEADER

// Return values
#define   MPSI_TRANSPORT_SUCCESS               0
#define   MPSI_TRANSPORT_ERROR                -1

// Max length of any MPSI message
#define MPSI_DATA_MAX_MESSAGE_LEN 259

// Max length of any MPSI control command message
#define MPSI_CTRL_MAX_COMMAND_LEN 5
// Command location in an MPSI control message
#define MPSI_CTRL_CMD 0
// Parameter location in an MPSI control message
#define MPSI_CTRL_PARAM 1
// Payload location in an MPSI message
#define MPSI_MESSAGE_PAYLOAD_LEN_LOC 3

// Max length of an OTA message
#define OTA_DATA_MESSAGE_LEN 20
// Max length of an OTA control message
#define OTA_CTRL_MAX_COMMAND_LEN 5

void mpsiTransportHandleEvents(struct gecko_cmd_packet *evt);

#endif // __MPSI_BLE_TRANSPORT__
