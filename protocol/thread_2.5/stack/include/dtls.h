/**
 * @file dtls.h
 * @brief DTLS API for dotdot
 *
 * Note:
 * If using an mbed TLS library, your stack must define EMBER_MBEDTLS_STACK.
 *
 * <!--Copyright 2017 by Silicon Labs. All rights reserved.              *80*-->
 */

#ifndef DTLS_H
#define DTLS_H

/**
 * @addtogroup dtls
 *
 * See dtls.h for source code.
 * @{
 */

/** @brief Set a device certificate to be used to create a certificate
 *         based secure session on the application.  The expected
 *         arguments are DER encoded X.509 certificates.
 *         If this succeeds, ::emberSetDtlsDeviceCertificateReturn
 *         should return 0.
 *
 * @param certAuthority  the certificate authority
 * @param deviceCert     the certificate
 */
void emberSetDtlsDeviceCertificate(const CertificateAuthority **certAuthority,
                                   const DeviceCertificate *deviceCert);

/** @brief Provides the result of a call to emberSetDtlsDeviceCertificate().
 *
 * @param result
 * - ::0      The certificate was set successfully.
 * - ::result error code
 *            - an EmberStatus value if using Silicon Labs TLS
 *            - an mbed TLS error code if using mbed TLS library
 *            (see mbedtls:include/mbedtls/ssl.h)
 */
void emberSetDtlsDeviceCertificateReturn(uint32_t result);

/** @brief Set a key to be used to create a PSK based secure session
 *         on the application.  The maximum length of the key is 32
 *         bytes.
 *
 *         Note: Up to 32 pre-shared keys can be stored.
 *
 *         If this succeeds, ::emberSetDtlsPresharedKeyReturn
 *         will return EMBER_SUCCESS.  Otherwise, a failure status is
 *         indicated.
 *
 * @param key           the pre-shared key
 * @param keyLength     length
 * @param remoteAddress IPv6 address of peer
 */
void emberSetDtlsPresharedKey(const uint8_t *key,
                              uint8_t keyLength,
                              const EmberIpv6Address *remoteAddress);

/** @brief Provides the result of a call to emberSetDtlsPresharedKey().
 *
 * @param result
 * - ::status An EmberStatus value
 */
void emberSetDtlsPresharedKeyReturn(EmberStatus status);

/** @brief Define the various modes of a DTLS connection.
 *
 * Note: Please configure either the CERT or PSK modes, as the
 * public key option is currently unavailable.
 */
// Set by the application
#define EMBER_DTLS_MODE_CERT    0x01
#define EMBER_DTLS_MODE_PSK     0x02
#define EMBER_DTLS_MODE_PKEY    0x04

typedef uint8_t EmberDtlsMode;

/** @brief Establish a DTLS connection with a peer on the Thread network.
 *         When established, this session can be used to send secure CoAP
 *         data.  The device requesting the connection acts as a DTLS
 *         client.
 *
 *         (For DotDot applications, the local port and remote port are
 *         both ::EMBER_COAP_SECURE_PORT)
 *
 * @param dtlsMode      DTLS connection mode (see EMBER_DTLS_MODE_* above)
 * @param remoteAddress IPv6 address of the server
 * @param localPort     local port
 * @param remotePort    remote port
 */
void emberOpenDtlsConnection(EmberDtlsMode dtlsMode,
                             const EmberIpv6Address *remoteAddress,
                             uint16_t localPort,
                             uint16_t remotePort);

/** @brief Provides the result of a call to emberOpenDtlsConnection().
 *
 * @param result error code
 *        - an EmberStatus value if using Silicon Labs TLS
 *        - an mbed TLS error code if using mbed TLS library
 *        (see mbedtls:include/mbedtls/ssl.h)
 * @param remoteAddress IPv6 address of the server
 * @param localPort     local port
 * @param remotePort    remote port
 */
void emberOpenDtlsConnectionReturn(uint32_t result,
                                   const EmberIpv6Address *remoteAddress,
                                   uint16_t localPort,
                                   uint16_t remotePort);

/** @brief Indicates to the application that a secure connection was
 *         successfully established.
 *
 * @param flags         1 = server, 0 = client (possibly other info later)
 * @param sessionId     sessionId used for secure CoAP transport
 * @param localAddress  local IPv6 address
 * @param remoteAddress remote IPv6 address
 * @param localPort     local port
 * @param remotePort    remote port
 */
void emberDtlsSecureSessionEstablished(uint8_t flags,
                                       uint8_t sessionId,
                                       const EmberIpv6Address *localAddress,
                                       const EmberIpv6Address *remoteAddress,
                                       uint16_t localPort,
                                       uint16_t remotePort);

/** @brief Request the session ID given connection parameters.
 *
 * @param remoteAddress remote IPv6 address
 * @param localPort     local port
 * @param remotePort    remote port
 */
void emberGetSecureDtlsSessionId(const EmberIpv6Address *remoteAddress,
                                 uint16_t localPort,
                                 uint16_t remotePort);

/** @brief Provides the result of a call to emberGetSecureDtlsSessionId().
 *
 * @param sessionId     sessionId used for secure CoAP transport
 * @param remoteAddress remote IPv6 address
 * @param localPort     local port
 * @param remotePort    remote port
 */
void emberGetSecureDtlsSessionIdReturn(uint8_t sessionId,
                                       const EmberIpv6Address *remoteAddress,
                                       uint16_t localPort,
                                       uint16_t remotePort);

/** @brief Close a currently active secure session on the application.
 *         When successful, emberCloseDtlsConnectionReturn should be
 *         called on both ends of the connection with ::EMBER_SUCCESS.
 *
 * @param sessionId     sessionId used for secure CoAP transport.
 *
 */
void emberCloseDtlsConnection(uint8_t sessionId);

/** @brief Provides the result of a call to emberCloseDtlsConnection(), or
 *         indicates that the connection was closed on the other end.
 *
 * @param sessionId     sessionId used for secure CoAP transport.
 * @param status
 * - ::EMBER_SUCCESS      - Successfully closed the connection
 * - ::EMBER_INVALID_CALL - Invalid session ID
 * - ::EMBER_ERR_FATAL    - Fatal error closing the connection
 */
void emberCloseDtlsConnectionReturn(uint8_t sessionId, EmberStatus status);

/** @brief  Public DTLS transmit handler to be set in emberCoapSend.
 *          The secure payload is delivered via emberProcessCoap on the other
 *          end, with a matching session ID in the transmitHandlerData of
 *          its CoapRequestInfo.
 *          See ::emberProcessCoap (stack/include/coap.h)
 *
 * @param payload              CoAP payload to be sent securely
 * @param payloadLength        payload length
 * @param localAddress         local IPv6 address
 * @param localPort            local port
 * @param remoteAddress        remote IPv6 address
 * @param remotePort           remote port
 * @param transmitHandlerData  session ID of the secure connection
 *                             (see ::emberDtlsSecureSessionEstablished
 *                              or ::emberGetSecureDtlsSessionId above)
 *
 */
bool emberDtlsTransmitHandler(const uint8_t *payload,
                              uint16_t payloadLength,
                              const EmberIpv6Address *localAddress,
                              uint16_t localPort,
                              const EmberIpv6Address *remoteAddress,
                              uint16_t remotePort,
                              void *transmitHandlerData);

/** @} // END addtogroup
 */

#endif // DTLS_H
