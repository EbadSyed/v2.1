/**
 * @file udp.h
 * @brief Simple UDP API
 *
 * <!--Copyright 2013 by Silicon Labs. All rights reserved.              *80*-->
 */

#ifndef UDP_H
#define UDP_H

/**
 * @addtogroup udp
 *
 * See udp.h for source code.
 * @{
 */

/** @brief  This function sets up a listener for UDP messages for a given address.
 *
 * @param port    A port to bind the UDP address to
 * @param address The IPv6 address listened to.
 *
 * @return EMBER_SUCCESS    if successful
 *         EMBER_TABLE_FULL if we failed to set up a listener
 *         EMBER_ERR_FATAL  other fatal failure
 */
EmberStatus emberUdpListen(uint16_t port, const uint8_t *address);

/** @brief  This function sends a UDP message.
 *
 * @param destination     IPv6 destination address
 * @param sourcePort      UDP source port
 * @param destinationPort UDP destination port
 * @param payload         UDP transport payload
 * @param payloadLength   Payload length
 *
 * @return EMBER_SUCCESS    if successful
 *         EMBER_NO_BUFFERS if failed to allocate a buffer
 *         EMBER_ERR_FATAL  other fatal failure
 */
EmberStatus emberSendUdp(const uint8_t *destination,
                         uint16_t sourcePort,
                         uint16_t destinationPort,
                         uint8_t *payload,
                         uint16_t payloadLength);

/** @brief  An application callback for an incoming UDP message.
 *
 * @param destination     IPv6 destination address
 * @param source          IPv6 source address
 * @param localPort       UDP source port
 * @param remotePort      UDP destination port
 * @param payload         UDP transport payload
 * @param payloadLength   payload length
 */
void emberUdpHandler(const uint8_t *destination,
                     const uint8_t *source,
                     uint16_t localPort,
                     uint16_t remotePort,
                     const uint8_t *payload,
                     uint16_t payloadLength);

/** @brief  An application callback for an incoming UDP multicast.
 *
 * @param destination     IPv6 destination address
 * @param source          IPv6 source address
 * @param localPort       UDP source port
 * @param remotePort      UDP destination port
 * @param payload         UDP transport payload
 * @param payloadLength   payload length
 */
void emberUdpMulticastHandler(const uint8_t *destination,
                              const uint8_t *source,
                              uint16_t localPort,
                              uint16_t remotePort,
                              const uint8_t *payload,
                              uint16_t payloadLength);
/** @} // END addtogroup
 */

#endif // UDP_H
