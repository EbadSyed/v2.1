// *******************************************************************
// * ias-zone-server.h
// *
// * This is the source for the plugin used to add an IAS Zone cluster server
// * to a project.  This source handles zone enrollment and storing of
// * attributes from a CIE device, and provides an API for different plugins to
// * post updated zone status values.
// *
// * Copyright 2015 Silicon Laboratories, Inc.                              *80*
// *******************************************************************
//-----------------------------------------------------------------------------
#ifndef SILABS_IAS_ZONE_SERVER_H
#define SILABS_IAS_ZONE_SERVER_H

#define EM_AF_UNKNOWN_ENDPOINT  0

/** @brief Update the zone status for an endpoint
 *
 * This function will update the zone status attribute of the specified endpoint
 * using the specified new zone status.  It will then notify the CIE of the
 * updated status.
 *
 * @param endpoint The endpoint whose zone status attribute is to be updated
 * @param newStatus The new status to write to the attribute
 * @param timeSinceStatusOccurredQs The amount of time (in quarter seconds) that
 *   has passed since the status change occurred
 *
 * @return EMBER_SUCCESS if the attribute update and notify succeeded, error
 * code otherewise
 */
EmberStatus emberAfPluginIasZoneServerUpdateZoneStatus(
  uint8_t endpoint,
  uint16_t newStatus,
  uint16_t  timeSinceStatusOccurredQs);

/** @brief Get the CIE assigned zone id of a given endpoint.
 *
 * This function will return the zone ID that was assigned to the given
 * endpoint by the CIE at time of enrollment.
 *
 * @param endpoint The endpoint whose ID is to be queried
 *
 * @return The zone ID assigned by the CIE at time of enrollment
 */
uint8_t emberAfPluginIasZoneServerGetZoneId(uint8_t endpoint);

/** @brief Determine the enrollment status of a given endpoint
 *
 * This function will return true or false depending on whether the specified
 * endpoint has undergone IAS Zone Enrollment.
 *
 * @param endpoint The endpoint whose enrollment status is to be queried
 *
 * @return true if enrolled, false otherwise
 */
bool emberAfIasZoneClusterAmIEnrolled(uint8_t endpoint);

#endif //__IAS_ZONE_SERVER_H__
