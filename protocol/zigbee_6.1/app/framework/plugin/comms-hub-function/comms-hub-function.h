// *******************************************************************
// * comms-hub-function.h
// *
// *
// * Copyright 2014 by Silicon Laboratories, Inc. All rights reserved.      *80*
// *******************************************************************

// Printing macros for plugin: Comms Hub Function
#define emberAfPluginCommsHubFunctionPrint(...)    emberAfAppPrint(__VA_ARGS__)
#define emberAfPluginCommsHubFunctionPrintln(...)  emberAfAppPrintln(__VA_ARGS__)
#define emberAfPluginCommsHubFunctionDebugExec(x)  emberAfAppDebugExec(x)
#define emberAfPluginCommsHubFunctionPrintBuffer(buffer, len, withSpace) emberAfAppPrintBuffer(buffer, len, withSpace)

typedef enum {
  EMBER_AF_CHF_STATUS_SUCCESS                 = 0x00,
  EMBER_AF_CHF_STATUS_TOO_MANY_PEND_MESSAGES  = 0xFA,
  EMBER_AF_CHF_STATUS_FNF_ATTR_FAILURE        = 0xFB,
  EMBER_AF_CHF_STATUS_NO_MIRROR               = 0xFC,
  EMBER_AF_CHF_STATUS_TUNNEL_FAILURE          = 0xFD,
  EMBER_AF_CHF_STATUS_NO_ACCESS               = 0xFE,
  EMBER_AF_CHF_STATUS_SEND_TIMEOUT            = 0xFF,
} EmberAfPluginCommsHubFunctionStatus;

/**
 * @brief Pass message to be tunneled over the HAN using either
 * a sleepy buffer system (GSME) or direct to the device
 * by initiating a tunnel (ESME, HCALCS, PPMID, TYPE2).
 *
 * This function can be used to transfer data to a device on the HAN.
 *
 * @param destinationDeviceId The EUI64 of the destination device.
 * @param length The length in octets of the data.
 * @param payload Buffer(memory location at WAN Message Handler) containing the
 *  raw octets of the message(GBCS Message)
 * @param messageCode The GBCS Message Code for the data that is being sent.
 * @return
 * ::EMBER_AF_CHF_STATUS_SUCCESS data was sent or has been queue to be sent
 * ::EMBER_AF_CHF_STATUS_NO_ACCESS No entry in the GBCS Device Log for the specified device
 * ::EMBER_AF_CHF_STATUS_NO_MIRROR Mirror endpoint for given device has not been configured
 * ::EMBER_AF_CHF_STATUS_FNF_ATTR_FAILURE Unable to read or write the functional notification flags attribute
 * ::EMBER_AF_CHF_STATUS_TOO_MANY_PEND_MESSAGES There are too many messages currently pending to be delivered
 * ::EMBER_AF_CHF_STATUS_TUNNEL_FAILURE tunnel cannot be created to non sleepy devices
 */
EmberAfPluginCommsHubFunctionStatus emberAfPluginCommsHubFunctionSend(EmberEUI64 destinationDeviceId,
                                                                      uint16_t length,
                                                                      uint8_t *payload,
                                                                      uint16_t messageCode);

/**
 * @brief Tunnel Accept
 *
 * This callback is called by the tunnel manager when a tunnel is requested. The
 * given device identifier should be checked against the Device Log to verify
 * whether tunnels from the device should be accepted or not.
 *
 * @param deviceId Identifier of the device from which a tunnel is requested
 * @return true is the tunnel should be allowed, false otherwise
 */
bool emAfPluginCommsHubFunctionTunnelAcceptCallback(EmberEUI64 deviceId);

/**
 * @brief Tunnel Data Received
 *
 * This callback is called by the tunnel manager when data is received over a tunnel.
 * It is responsible for the implementation of the GET, GET_RESPONSE, PUT
 * protocol used when communicating with a sleepy device.
 *
 * @param senderDeviceId Identifier of the device from which the data was received
 * @param length The length of the data received
 * @param payload The data received
 */
void emAfPluginCommsHubFunctionTunnelDataReceivedCallback(EmberEUI64 senderDeviceId,
                                                          uint16_t length,
                                                          uint8_t *payload);

/**
 * @brief Set the default remote part message timeout
 *
 * This function can be used to set the default timeout for all messages
 * destined for a sleepy device.  If the device does not retrieve the message
 * before this time then it will be discarded and a ::EMBER_AF_CHF_STATUS_SEND_TIMEOUT
 * error will be return in emberAfPluginCommsHubFunctionSendCallback().
 *
 * @param timeout timeout in seconds
 */
void emAfPluginCommsHubFunctionSetDefaultTimeout(uint32_t timeout);
