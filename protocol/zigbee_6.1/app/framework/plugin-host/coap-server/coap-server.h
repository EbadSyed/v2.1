// Copyright 2017 Silicon Laboratories, Inc.                                *80*

#ifndef SILABS_COAP_SERVER_H
#define SILABS_COAP_SERVER_H

#include "include/coap/coap.h"

/** @brief API to send message using COAP.
 *
 * Need a method for sending unsolicited messages to the server.
 *
 * @param *uri Pointer to the URI string.
 *
 * @param *payload Pointer to the payload (if any).  NULL indicates no payload.
 *
 * @param length Number of bytes in the payload.  0 indicates no payload.
 *
 * @param device Numver of the device from which the message was received.
 *
 * @param method.  COAP method to use to send the message.
 *
 */
void emberAfPluginCoapServerSendMessage(uint8_t *uri,
                                        uint8_t *payload,
                                        uint16_t length,
                                        uint16_t device,
                                        uint8_t method);

/** @brief Creates a listener port based on the device table index.
 *
 * When a new device joins the gateway, it will be placed into the device table
 * at a unique index point.  This method will open up the appropriate COAP
 * port on which to listen for traffic.
 *
 * @param deviceIndex Index of the device to create.
 *
 */
void emberAfPluginCoapServerCreateDevice(uint16_t deviceIndex);

/** @brief Remove a listener port based on the device table index.
 *
 * When a device is removed from the device table, we need to also close the
 * corresponding COAP port.
 *
 * @param deviceIndex Index of the device to remove.
 *
 */
void emberAfPluginCoapServerRemoveDevice(uint16_t deviceIndex);

/** @brief Copy the response string into the data in pointer.
 *
 * Gateway relay coap plugin may generate a response string.  If so, the coap
 * server will need to obtain it using this api.
 *
 * @param *serverString String of the IP address of the server.
 *
 * @param serverPort Port of the server to which we will send responses.
 *
 */
void emberAfPluginCoapServerSetServerNameAndPort(uint8_t *serverString,
                                                 uint16_t serverPort);

/** @brief Macro to set the maximum number of devices that can connect.  Note:
 * this will be specific to the implementation hardware.
 */
 #define PLUGIN_COAP_SERVER_MAX_PORTS 20

/** @brief COAP Response code for message received and undrestood.
 */
#define COAP_RESPONSE_OK                  200

/** @brief COAP Response code for resource created.
 */
#define COAP_RESPONSE_CREATED             201

/** @brief COAP Response code for message was valid.
 */
#define COAP_RESPONSE_VALID               203

/** @brief COAP Response code stating response message has data.
 */
#define COAP_RESPONSE_DATA                205

/** @brief COAP Response code for a bad request.
 */
#define COAP_RESPONSE_BAD_REQUEST         400

/** @brief COAP Response code stating the resource was not found.
 */
#define COAP_RESPONSE_NOT_FOUND           404

/** @brief COAP Response code stating resource can not be accessed with this
 *  method
 */
#define COAP_RESPONSE_METHOD_NOT_ALLOWED  405

/** @brief COAP Response code for an internal server error.
 */
#define COAP_RESPONSE_INTERNAL_ERROR      500

/** @brief COAP Response code stating the resource was not yet implemented.
 */
#define COAP_RESPONSE_NOT_IMPLEMENTED     501

/** @brief COAP Response code for unavailable service.
 */
#define COAP_RESPONSE_UNAVAILABLE         503

/** @brief COAP Response code for gateway timeout error.
 */
#define COAP_RESPONSE_TIMEOUT             504

#endif
