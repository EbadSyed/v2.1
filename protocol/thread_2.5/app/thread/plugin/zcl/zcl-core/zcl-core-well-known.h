// Copyright 2016 Silicon Laboratories, Inc.

#ifndef ZCL_CORE_WELL_KNOWN_H
#define ZCL_CORE_WELL_KNOWN_H

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include "zcl-core.h"

/**
 * @addtogroup ZCLIP
 *
 * @{
 */

/**
 * @addtogroup ZCLIP_discovery Discovery
 *
 * See zcl-core-well-known.h for source code.
 * @{
 */

/** Defines possible request modes. */
typedef enum {
  /** Discovery request is allowed a single query. */
  EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY = 0,
  /** Discovery request is allowed multiple queries. */
  EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY = 1,
  /** Maximum discovery request mode. */
  EMBER_ZCL_DISCOVERY_REQUEST_MODE_MAX = 2
} EmberZclDiscoveryRequestMode;

/**************************************************************************//**
 * Initialization for sending Discovery command.
 *****************************************************************************/
void emberZclDiscInit(void);

/**************************************************************************//**
 * This function sets mode to create a query.
 *
 * @param      mode  EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY - single query
 *                   EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY - multiple queries
 *
 * @return     True if mode was set or false otherwise.
 *
 * Under EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY mode, appending one query string
 * automatically triggers the Discovery command to be broadcast.
 * Under EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY mode, appended query strings
 * is accumulated. The accumulated query string will not be broadcast until
 * emberZclDiscSend() is called.
 *****************************************************************************/
bool emberZclDiscSetMode(EmberZclDiscoveryRequestMode mode);

/**************************************************************************//**
 * This function broadcasts a GET using the Discovery request string.
 *
 * @param      responseHandler  The response handler
 *
 * @return     True if the message was sent or false otherwise.
 *
 *****************************************************************************/
bool emberZclDiscSend(EmberCoapResponseHandler responseHandler);

/**************************************************************************//**
 * This function appends a cluster ID query to the discovery request string.
 *
 * @param      clusterSpec      A structure for cluster ID / role / manufacture code
 * @param      responseHandler  The response handler
 *
 * @return     in EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY mode:
 *               True if the command was sent or false otherwise.
 *             in EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY mode:
 *               True if the command was appended or false otherwise.
 *****************************************************************************/
bool emberZclDiscByClusterId(const EmberZclClusterSpec_t *clusterSpec,
                             EmberCoapResponseHandler responseHandler);

/**************************************************************************//**
 * This function appends an endpoint query to the Discovery request string.
 *
 * @param      endpointId       The endpoint identifier
 * @param      deviceId         The device identifier
 * @param      responseHandler  The response handler
 *
 * @return     in EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY mode:
 *               True if the command was sent or false otherwise.
 *             in EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY mode:
 *               True if the command was appended or false otherwise.
 *****************************************************************************/
bool emberZclDiscByEndpoint(EmberZclEndpointId_t endpointId,
                            EmberZclDeviceId_t deviceId,
                            EmberCoapResponseHandler responseHandler);

/**************************************************************************//**
 * This function appends a UID query to the discovery request string.
 *
 * @param      uid              The uid
 * @param      uidBits          The uid bits
 * @param      responseHandler  The response handler
 *
 * @return     in EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY mode:
 *               True if the command was sent or false otherwise.
 *             in EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY mode:
 *               True if the command was appended or false otherwise.
 *****************************************************************************/
bool emberZclDiscByUid(const EmberZclUid_t *uid,
                       uint16_t uidBits,
                       EmberCoapResponseHandler responseHandler);

/**************************************************************************//**
 * This function appends a cluster revision query to the discovery request string.
 *
 * @param      version          The version
 * @param      responseHandler  The response handler
 *
 * @return     in EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY mode:
 *               True if the command was sent or false otherwise.
 *             in EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY mode:
 *               True if the command was appended or false otherwise.
 *****************************************************************************/
bool emberZclDiscByClusterRev(EmberZclClusterRevision_t version,
                              EmberCoapResponseHandler responseHandler);

/**************************************************************************//**
 * This function appends a device ID query to the discovery request string.
 *
 * @param      deviceId         The device identifier
 * @param      responseHandler  The response handler
 *
 * @return     in EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY mode:
 *               True if the command was sent or false otherwise.
 *             in EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY mode:
 *               True if the command was appended or false otherwise.
 *****************************************************************************/
bool emberZclDiscByDeviceId(EmberZclDeviceId_t deviceId,
                            EmberCoapResponseHandler responseHandler);

/**************************************************************************//**
 * This function appends a resource version query to the discovery request string.
 *
 * @param      version          The version
 * @param      responseHandler  The response handler
 *
 * @return     in EMBER_ZCL_DISCOVERY_REQUEST_SINGLE_QUERY mode:
 *               True if the command was sent or false otherwise.
 *             in EMBER_ZCL_DISCOVERY_REQUEST_MULTIPLE_QUERY mode:
 *               True if the command was appended or false otherwise.
 *****************************************************************************/
bool emberZclDiscByResourceVersion(EmberZclClusterRevision_t version,
                                   EmberCoapResponseHandler responseHandler);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef enum {
  EM_ZCL_DISC_URI_STRING_APPEND_REQUEST,
  EM_ZCL_DISC_URI_STRING_APPEND_RESPONSE,
} EmZclDiscUriStringAppendMode;

extern EmZclUriPath emZclWellKnownUriPaths[];

#define EM_ZCL_URI_QUERY_UID                                "ep="
#define EM_ZCL_URI_QUERY_UID_SHA_256                        "ni:///sha-256;"
#define EM_ZCL_URI_QUERY_UID_SHA_256_PREFIX                 EM_ZCL_URI_QUERY_UID EM_ZCL_URI_QUERY_UID_SHA_256
#define EM_ZCL_URI_QUERY_PREFIX_CLUSTER_ID                  "rt=urn:zcl"
#define EM_ZCL_URI_QUERY_PREFIX_VERSION                     "if=urn:zcl:"
#define EM_ZCL_URI_QUERY_PROTOCOL_REVISION_FORMAT           "if=urn:zcl:v%x"
#define EM_ZCL_URI_QUERY_CLUSTER_REVISION_FORMAT            "if=urn:zcl:c.v%x"
#define EM_ZCL_URI_QUERY_PREFIX_DEVICE_TYPE_AND_ENDPOINT    "ze=urn:zcl:"
#define EM_ZCL_URI_QUERY_POSTFIX_DEVICE_ID                  "d."
#define EM_ZCL_URI_QUERY_CLUSTER_ID_LEN                     (3)
#define EM_ZCL_URI_QUERY_DOT                                '.'
#define EM_ZCL_URI_QUERY_WILDCARD                           '*'
#define EM_ZCL_URI_QUERY_VERSION_KEY                        "c.v"
#define EM_ZCL_URI_WELL_KNOWN                               ".well-known"
#define EM_ZCL_URI_CORE                                     "core"
#define EM_ZCL_URI_WELL_KNOWN_CORE                          ".well-known/core"
#define EM_ZCL_URI_RESPONSE_DELIMITER                       ";"
#define EM_ZCL_URI_WELL_KNOWN_CORE_PAYLOAD                  "if=urn:zcl:v0;rt=urn:zcl"

uint16_t emZclUriAppendUriPath(char *finger,
                               char *endOfBuffer,
                               EmberZclEndpointId_t endpointId,
                               const EmberZclClusterSpec_t *clusterSpec);
bool emZclUriBreak(char *finger);
bool emZclDiscByUidString(const uint8_t *uidString, EmberCoapResponseHandler responseHandler);
bool emZclDiscSetAccept(EmberCoapContentFormatType accept);
#endif // #ifndef DOXYGEN_SHOULD_SKIP_THIS

/** @} end addtogroup ZCLIP_discovery */
/** @} end addtogroup ZCLIP */

#endif // #ifndef ZCL_CORE_WELL_KNOWN_H
