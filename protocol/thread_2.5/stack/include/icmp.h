/**
 * @file icmp.h
 * @brief Simple ICMP API
 *
 * <!--Copyright 2013 by Silicon Labs. All rights reserved.              *80*-->
 */

#ifndef ICMP_H
#define ICMP_H

/**
 * @addtogroup icmp
 *
 * See icmp.h for source code.
 * @{
 */

/** @brief  This function sets up a listener for ICMP messages for the given address.
 *
 * @param address The IPv6 address, which is listened to.
 *
 * @return EMBER_SUCCESS    if successful
 *         EMBER_TABLE_FULL if failed to set up a listener
 *         EMBER_ERR_FATAL  other fatal failure
 */
EmberStatus emberIcmpListen(const uint8_t *address);

/** @brief  This function sends an ICMP ECHO REQUEST message.
 *
 * @param destination     IPv6 destination address
 * @param id              IPv6 unique ID
 * @param sequence        IPv6 unique sequence
 * @param length          Payload length
 * @param hopLimit        IPv6 hop limit
 *
 * @return Returns true if the ICMP echo request was succesfully
 * submitted to the IP stack and false otherwise.
 */
bool emberIpPing(uint8_t *destination,
                 uint16_t id,
                 uint16_t sequence,
                 uint16_t length,
                 uint8_t hopLimit);

/** @brief  An application callback for an incoming ICMP message.
 *
 * @param ipHeader        Pointer to an IPv6 buffer
 */
void emberIncomingIcmpHandler(Ipv6Header *ipHeader);

/** @} // END addtogroup
 */

#endif // ICMP_H
