// Copyright 2015 Silicon Laboratories, Inc.

#ifndef __ZCL_CORE_H__
#define __ZCL_CORE_H__

#include PLATFORM_HEADER
#include EMBER_AF_API_STACK
#include "zclip-struct.h"
#include "cbor.h"
#include "zcl-core-types.h"

#ifndef EMBER_SCRIPTED_TEST
  #include "thread-zclip.h"
#endif

/**
 * @addtogroup ZCLIP
 *
 * The ZCL Core plugin provides the necessary foundation of APIs to interface
 * with a ZCLIP-capable device.
 *
 * The functionality contained in this plugin provides basic ZCLIP features
 * including, but not limited to, the following:
 * - Attribute management
 * - Binding management
 * - Command handling and dispatching
 * - Endpoint management
 * - Group management
 * - Reporting configuration management
 * - Notification handling and dispatching
 * - Device discovery
 * - Application provisioning
 * - General ZCLIP utilities
 *
 * This plugin uses the Silicon Labs Constrained Application Protocol (CoAP)
 * implementation to communicate over the air with remote devices. More
 * information about the Silicon Labs CoAP implementation can be found in @ref
 * coap. This plugin also uses the Silicon Labs token system for storing
 * non-volatile ZCLIP data. More information about the Silicon Labs token
 * system can be found in @ref tokens.
 *
 * This plugin also provides a list of command line interface (CLI) commands
 * with which users can drive their applications. These CLI commands are
 * documented in the README.zclip file included in this release.
 *
 * See @ref zcl-core-callbacks for the ZCLIP application callback API.
 * @{
 */

// ----------------------------------------------------------------------------
// Addresses.

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern EmberZclUid_t emZclUid;
size_t emZclCacheGetEntryCount(void);
bool emZclCacheAdd(const EmberZclUid_t *key,
                   const EmberIpv6Address *value,
                   EmZclCacheIndex_t *index);
bool emZclCacheGet(const EmberZclUid_t *key,
                   EmberIpv6Address *value);
bool emZclCacheGetByIndex(EmZclCacheIndex_t index,
                          EmberZclUid_t *key,
                          EmberIpv6Address *value);
bool emZclCacheGetFirstKeyForValue(const EmberIpv6Address *value,
                                   EmberZclUid_t *key);
bool emZclCacheGetIndex(const EmberZclUid_t *key,
                        EmZclCacheIndex_t *index);
bool emZclCacheRemove(const EmberZclUid_t *key);
bool emZclCacheRemoveByIndex(EmZclCacheIndex_t index);
void emZclCacheRemoveAll(void);
size_t emZclCacheRemoveAllByValue(const EmberIpv6Address *value);
size_t emZclCacheRemoveAllByIpv6Prefix(const EmberIpv6Address *prefix,
                                       uint8_t prefixLengthInBits);
void emZclCacheScan(const void *criteria, EmZclCacheScanPredicate match);
#endif

// -----------------------------------------------------------------------------
// Endpoints.

/**
 * @addtogroup ZCLIP_endpoints Endpoints
 *
 * See zcl-core.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function finds the endpoint index for the specified endpoint identifier and
 * cluster specification.
 *
 * @param endpointId An endpoint identifier
 * @param clusterSpec A cluster specification or NULL
 * @return An endpoint index or @ref EMBER_ZCL_ENDPOINT_INDEX_NULL if no match
 *         is found.
 *
 * This function searches the endpoint table and returns the endpoint index for the entry
 * that matches the specified endpoint identifier and cluster specification.
 * If clusterSpec is NULL, match on endpointId only.
 *
 * @note The endpoint index is the zero-based relative position of an
 *       endpoint among the subset of endpoints that support the specified
 *       cluster. For example, an index of 3 refers to the fourth endpoint
 *       that supports the specified cluster. The value of endpoint index
 *       for a given endpoint identifier may be different for different
 *       clusters.
 *
 * @sa emberZclEndpointIndexToId()
 *****************************************************************************/
EmberZclEndpointIndex_t emberZclEndpointIdToIndex(EmberZclEndpointId_t endpointId,
                                                  const EmberZclClusterSpec_t *clusterSpec);

/**************************************************************************//**
 * This function finds the endpoint identifier for the specified endpoint index and
 * cluster specification.
 *
 * @param index An endpoint index
 * @param clusterSpec A cluster specification or NULL
 * @return An endpoint identifier or @ref EMBER_ZCL_ENDPOINT_NULL if no match
 *         is found.
 *
 * This function searches the endpoint table and returns the endpoint identifier for the entry
 * that matches the specified endpoint index and cluster specification.
 * If clusterSpec is NULL, match on index only.
 *
 * @note The endpoint index is the zero-based relative position of an
 *       endpoint among the subset of endpoints that support the specified
 *       cluster. For example, an index of 3 refers to the fourth endpoint
 *       that supports the specified cluster. The value of endpoint index
 *       for a given endpoint identifier may be different for different
 *       clusters.
 *
 * @sa emberZclEndpointIdToIndex()
 *****************************************************************************/
EmberZclEndpointId_t emberZclEndpointIndexToId(EmberZclEndpointIndex_t index,
                                               const EmberZclClusterSpec_t *clusterSpec);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern const EmZclEndpointEntry_t emZclEndpointTable[];
extern const size_t emZclEndpointCount;
const EmZclEndpointEntry_t *emZclFindEndpoint(EmberZclEndpointId_t endpointId);
bool emZclEndpointHasCluster(EmberZclEndpointId_t endpointId,
                             const EmberZclClusterSpec_t *clusterSpec);
EmberZclStatus_t emZclMultiEndpointDispatch(const EmZclContext_t *context,
                                            EmZclMultiEndpointHandler handler,
                                            CborState *state,
                                            void *data);
void emZclUriEndpointHandler(EmZclContext_t *context);
void emZclUriEndpointIdHandler(EmZclContext_t *context);
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Groups.

/**
 * @addtogroup ZCLIP_groups Groups
 *
 * See zcl-core.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function determines if an endpoint is a member of a group.
 *
 * @param endpointId An endpoint identifier
 * @param groupId A group identifier
 * @return `true` if the endpoint is a member of the group, `false` otherwise.
 *****************************************************************************/
bool emberZclIsEndpointInGroup(EmberZclEndpointId_t endpointId,
                               EmberZclGroupId_t groupId);

/**************************************************************************//**
 * This function gets a group name and its length.
 *
 * @param endpointId An endpoint identifier
 * @param groupId  A group identifier
 * @param groupName An array pointer which will contain the group name
 * @param groupNameLength A pointer which will contain the group name length
 * @return `true` if group name was retrieved successfully, `false` otherwise.
 *
 *****************************************************************************/
bool emberZclGetGroupName(EmberZclEndpointId_t endpointId,
                          EmberZclGroupId_t groupId,
                          uint8_t *groupName,
                          uint8_t *groupNameLength);

/**************************************************************************//**
 * This function adds an endpoint to a group.
 *
 * @param endpointId An endpoint identifier
 * @param groupId  A group identifier
 * @param groupName A pointer to an array containing name of group
 * @param groupNameLength Length of group name array (groupName is ignored if
 * this length is 0)
 * @return
 * - @ref EMBER_ZCL_STATUS_SUCCESS if the endpoint was added to the group
 * - @ref EMBER_ZCL_STATUS_DUPLICATE_EXISTS if the endpoint is already a member
 *   of the group
 * - @ref EMBER_ZCL_STATUS_INSUFFICIENT_SPACE if there is no capacity to store
 *   the endpoint/group association
 * - @ref EMBER_ZCL_STATUS_FAILURE if groupId is @ref EMBER_ZCL_GROUP_ALL_ENDPOINTS,
 *   or if groupId is not a value between @ref EMBER_ZCL_GROUP_MIN and
 *   @ref EMBER_ZCL_GROUP_MAX inclusive, or if groupNameLength is greater than
 *   @ref EMBER_ZCL_MAX_GROUP_NAME_LENGTH, or if groupNameLength is non-zero and
 *   groupName is NULL
 *****************************************************************************/
EmberZclStatus_t emberZclAddEndpointToGroup(EmberZclEndpointId_t endpointId,
                                            EmberZclGroupId_t groupId,
                                            const uint8_t *groupName,
                                            uint8_t groupNameLength);

/**************************************************************************//**
 * This function removes an endpoint from a group.
 *
 * @param endpointId An endpoint identifier
 * @param groupId A group identifier
 * @return
 * - @ref EMBER_ZCL_STATUS_SUCCESS if the endpoint was removed from the group
 * - @ref EMBER_ZCL_STATUS_NOT_FOUND if the endpoint is not a member of the group
 * - @ref EMBER_ZCL_STATUS_INVALID_FIELD if the groupId is not a value between
 *   @ref EMBER_ZCL_GROUP_MIN and @ref EMBER_ZCL_GROUP_MAX inclusive
 *****************************************************************************/
EmberZclStatus_t emberZclRemoveEndpointFromGroup(EmberZclEndpointId_t endpointId,
                                                 EmberZclGroupId_t groupId);

/**************************************************************************//**
 * This function removes an endpoint from all groups to which it belongs.
 *
 * @param endpointId An endpoint identifier
 * @return
 * - @ref EMBER_ZCL_STATUS_SUCCESS if the endpoint was removed from one
 *   or more groups
 * - @ref EMBER_ZCL_STATUS_NOT_FOUND if the endpoint is not a member of
 *   any group
 *   (other than the all-endpoints group)
 *****************************************************************************/
EmberZclStatus_t emberZclRemoveEndpointFromAllGroups(EmberZclEndpointId_t endpointId);

/**************************************************************************//**
 * This function removes a group.
 *
 * @param groupId A group identifier
 * @return
 * - @ref EMBER_ZCL_STATUS_SUCCESS if the group was removed
 * - @ref EMBER_ZCL_STATUS_NOT_FOUND if the group does not exist
 * - @ref EMBER_ZCL_STATUS_INVALID_FIELD if the groupId is not a value between
 *   @ref EMBER_ZCL_GROUP_MIN and @ref EMBER_ZCL_GROUP_MAX inclusive
 *****************************************************************************/
EmberZclStatus_t emberZclRemoveGroup(EmberZclGroupId_t groupId);

/**************************************************************************//**
 * This function removes all groups.
 *
 * @return
 * - @ref EMBER_ZCL_STATUS_SUCCESS if all groups were removed (other than the
 *   all-endpoints group)
 * - @ref EMBER_ZCL_STATUS_NOT_FOUND if no groups exist (other than the
 *   all-endpoints group)
 *****************************************************************************/
EmberZclStatus_t emberZclRemoveAllGroups(void);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
bool emZclHasGroup(EmberZclGroupId_t groupId);
void emZclUriGroupHandler(EmZclContext_t *context);
void emZclUriGroupIdHandler(EmZclContext_t *context);
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Management.

/**
 * @addtogroup ZCLIP_management Management
 *
 * See zcl-core.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function puts a device in EZ-Mode for a fixed-duration.
 *
 * @return
 * - @ref EMBER_ZCL_STATUS_SUCCESS if EZ-Mode started successfully
 * - @ref EMBER_ERR_FATAL if the multicast address is invalid
 * - @ref ::EmberStatus with failure reason otherwise
 *
 * Each time EZ-Mode is invoked, the device extends the window by the same
 * fixed duration. During the window, devices perform EZ-Mode finding and
 * binding with other devices also in EZ-Mode. Multicast messages
 * advertise capabilities of the device to other nodes in the network.
 * Unicast messages communicate binding targets to specific devices.
 * While the timer is active and not expired, including when the window is
 * extended due to subsequent invocations, the device listens on the EZ-Mode
 * multicast address and processes EZ-Mode requests.
 *****************************************************************************/
EmberStatus emberZclStartEzMode(void);

/**************************************************************************//**
 * This function stops EZ-Mode.
 *
 * The device ignores all EZ-Mode requests and stops listening on the EZ-Mode
 * multicast address.
 *****************************************************************************/
void emberZclStopEzMode(void);

/**************************************************************************//**
 * This function checks whether EZ-Mode is currently active.
 *
 * @return `true` if EZ-Mode is active, `false` otherwise.
 *****************************************************************************/
bool emberZclEzModeIsActive(void);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const EmZclCommandEntry_t *emZclManagementFindCommand(EmberZclCommandId_t commandId);
void emZclManagementHandler(EmZclContext_t *context);
void emZclManagementCommandHandler(EmZclContext_t *context);
void emZclManagementCommandIdHandler(EmZclContext_t *context);
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Clusters.

/**
 * @addtogroup ZCLIP_clusters Clusters
 *
 * See zcl-core.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function compares two cluster specifications.
 *
 * @param s1 A cluster specification to be compared
 * @param s2 A cluster specification to be compared
 * @return
 * - @ref `0` if both cluster specifications are equal
 * - @ref `-1` if `s1`'s ::EmberZclRole_t is not equal to `s2`'s ::EmberZclRole_t
 *   and `s1`'s role is ::EMBER_ZCL_ROLE_CLIENT
 * - @ref `1` if `s1`'s ::EmberZclRole_t is not equal to `s2`'s ::EmberZclRole_t
 *   and `s1`'s role is ::EMBER_ZCL_ROLE_SERVER
 * - @ref difference between ::EmberZclManufacturerCode_t of `s1` and
 *   `s2`, or difference between ::EmberZclClusterId_t of `s1` and `s2`
 *****************************************************************************/
int32_t emberZclCompareClusterSpec(const EmberZclClusterSpec_t *s1,
                                   const EmberZclClusterSpec_t *s2);

/**************************************************************************//**
 * This function compares two cluster specifications.
 *
 * @param s1 A cluster specification to be compared
 * @param s2 A cluster specification to be compared
 * @return `true` if both cluster specifications are equal, `false` otherwise.
 *****************************************************************************/
bool emberZclAreClusterSpecsEqual(const EmberZclClusterSpec_t *s1,
                                  const EmberZclClusterSpec_t *s2);

/**************************************************************************//**
 * This function reverses cluster specifications.
 *
 * @param s1 A cluster specification used for reversing
 * @param s2 A cluster specification to be reversed
 *
 * This function changes ::EmberZclRole_t of `s2` to be opposite of `s1`. It
 * also sets ::EmberZclManufacturerCode_t and ::EmberZclClusterId_t of `s2`
 * to be the same as `s1`.
 *****************************************************************************/
void emberZclReverseClusterSpec(const EmberZclClusterSpec_t *s1,
                                EmberZclClusterSpec_t *s2);

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Attributes.

/**
 * @addtogroup ZCLIP_attributes Attributes
 *
 * See zcl-core.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function resets all local attributes on given endpoint.
 *
 * @param endpointId An endpoint identifier
 *
 * @note ::emberZclPostAttributeChangeCallback is triggered after each attribute
 *       is changed.
 *****************************************************************************/
void emberZclResetAttributes(EmberZclEndpointId_t endpointId);

/**************************************************************************//**
 * This function reads the value of an attibute.
 *
 * @param endpointId An endpoint identifier of the attribute
 * @param clusterSpec A cluster specification of the attribute
 * @param attributeId An attribute identifier to read
 * @param buffer A buffer to read the attribute into
 * @param bufferLength Length of the read buffer
 * @return
 * - @ref EMBER_ZCL_STATUS_SUCCESS if the attribute was read successfully
 * - @ref EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE if the attribute is remote or
 *   if the attribute is not supported on a specified endpoint
 * - @ref EMBER_ZCL_STATUS_INSUFFICIENT_SPACE if not enough space is available in
 *   the passed buffer to store the attribute value
 * - @ref ::EmberStatus with failure reason otherwise
 *
 * @note ::emberZclReadExternalAttributeCallback is triggered if attribute
 *       is externally stored. If so, the result of that call is returned.
 *****************************************************************************/
EmberZclStatus_t emberZclReadAttribute(EmberZclEndpointId_t endpointId,
                                       const EmberZclClusterSpec_t *clusterSpec,
                                       EmberZclAttributeId_t attributeId,
                                       void *buffer,
                                       size_t bufferLength);

/**************************************************************************//**
 * This function writes the value of an attibute.
 *
 * @param endpointId An endpoint identifier of the attribute
 * @param clusterSpec A cluster specification of the attribute
 * @param attributeId An attribute identifier to write
 * @param buffer A buffer to write the attribute from
 * @param bufferLength Length of the write buffer
 * @return
 * - @ref EMBER_ZCL_STATUS_SUCCESS if the attribute was written successfully
 * - @ref EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE if the attribute is remote or
 *   if the attribute is not supported on a specified endpoint
 * - @ref EMBER_ZCL_STATUS_INSUFFICIENT_SPACE if not enough space is available in
 *   the attribute table to store the attribute value
 * - @ref EMBER_ZCL_STATUS_INVALID_VALUE if the attribute value is invalid
 * - @ref EMBER_ZCL_STATUS_FAILURE if ::emberZclPreAttributeChangeCallback
 *   returned `false`
 * - @ref ::EmberStatus with failure reason otherwise
 *
 * @note ::emberZclWriteExternalAttributeCallback is triggered if the attribute
 *       is externally stored. If so, the result of that call is returned.
 * @note ::emberZclPostAttributeChangeCallback is triggered after the attribute
 *       is successfully changed.
 *****************************************************************************/
EmberZclStatus_t emberZclWriteAttribute(EmberZclEndpointId_t endpointId,
                                        const EmberZclClusterSpec_t *clusterSpec,
                                        EmberZclAttributeId_t attributeId,
                                        const void *buffer,
                                        size_t bufferLength);

/**************************************************************************//**
 * This function notifies the core code that an externally stored attribute has changed.
 *
 * @param endpointId An endpoint identifier of the attribute
 * @param clusterSpec A cluster specification of the attribute
 * @param attributeId An attribute identifier that is changed
 * @param buffer A buffer to write the attribute from
 * @param bufferLength Length of the write buffer
 * @return
 * - @ref EMBER_ZCL_STATUS_SUCCESS if the function call was successful
 * - @ref EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE if the attribute is not
 *   external or if the attribute is not supported on a specified endpoint
 * - @ref ::EmberStatus with failure reason otherwise
 *
 * @note ::emberZclPostAttributeChangeCallback is triggered for a successful
 *       call to this function.
 *****************************************************************************/
EmberZclStatus_t emberZclExternalAttributeChanged(EmberZclEndpointId_t endpointId,
                                                  const EmberZclClusterSpec_t *clusterSpec,
                                                  EmberZclAttributeId_t attributeId,
                                                  const void *buffer,
                                                  size_t bufferLength);

/**************************************************************************//**
 * This function sends an attribute read command to a specified destination.
 *
 * @param destination A location to read the attribute from
 * @param clusterSpec A cluster specification of the attribute
 * @param attributeIds An array of attribute IDs to read
 * @param attributeIdsCount A total count of ::EmberZclAttributeId_t elements in a
 * passed array
 * @param handler callback that is triggered for a response
 * @return
 * - @ref EMBER_ZCL_STATUS_SUCCESS if function call was successful
 * - @ref ::EmberStatus with failure reason otherwise
 *
 * @sa EmberZclReadAttributeResponseHandler()
 *****************************************************************************/
EmberStatus emberZclSendAttributeRead(const EmberZclDestination_t *destination,
                                      const EmberZclClusterSpec_t *clusterSpec,
                                      const EmberZclAttributeId_t *attributeIds,
                                      size_t attributeIdsCount,
                                      const EmberZclReadAttributeResponseHandler handler);

/**************************************************************************//**
 * This function sends an attribute write command to a specified destination.
 *
 * @param destination A location to write the attribute to
 * @param clusterSpec A cluster specification of the attribute
 * @param attributeWriteData An array containing write data for attributes
 * @param attributeWriteDataCount A total count of ::EmberZclAttributeWriteData_t
 * elements in a passed array
 * @param handler A callback that is triggered for a response
 * @return
 * - @ref EMBER_ZCL_STATUS_SUCCESS if the function call was successful
 * - @ref ::EmberStatus with failure reason otherwise
 *
 * @sa EmberZclWriteAttributeResponseHandler()
 *****************************************************************************/
EmberStatus emberZclSendAttributeWrite(const EmberZclDestination_t *destination,
                                       const EmberZclClusterSpec_t *clusterSpec,
                                       const EmberZclAttributeWriteData_t *attributeWriteData,
                                       size_t attributeWriteDataCount,
                                       const EmberZclWriteAttributeResponseHandler handler);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern const uint8_t emZclAttributeDefaultMinMaxData[];
extern const size_t emZclAttributeDefaultMinMaxLookupTable[];
extern const EmZclAttributeEntry_t emZclAttributeTable[];
#define emZclIsAttributeLocal(attribute)                      \
  (READBITS((attribute)->mask, EM_ZCL_ATTRIBUTE_STORAGE_MASK) \
   != EM_ZCL_ATTRIBUTE_STORAGE_NONE)
#define emZclIsAttributeRemote(attribute)                     \
  (READBITS((attribute)->mask, EM_ZCL_ATTRIBUTE_STORAGE_MASK) \
   == EM_ZCL_ATTRIBUTE_STORAGE_NONE)
#define emZclIsAttributeExternal(attribute)                        \
  (READBITS((attribute)->mask, EM_ZCL_ATTRIBUTE_STORAGE_TYPE_MASK) \
   == EM_ZCL_ATTRIBUTE_STORAGE_TYPE_EXTERNAL)
#define emZclIsAttributeTokenized(attribute)                       \
  (READBITS((attribute)->mask, EM_ZCL_ATTRIBUTE_STORAGE_TYPE_MASK) \
   == EM_ZCL_ATTRIBUTE_STORAGE_TYPE_TOKEN)
#define emZclIsAttributeSingleton(attribute)                            \
  (READBITS((attribute)->mask, EM_ZCL_ATTRIBUTE_STORAGE_SINGLETON_MASK) \
   == EM_ZCL_ATTRIBUTE_STORAGE_SINGLETON_MASK)
#define emZclIsAttributeReadable(attribute)                      \
  (READBITS((attribute)->mask, EM_ZCL_ATTRIBUTE_ACCESS_READABLE) \
   == EM_ZCL_ATTRIBUTE_ACCESS_READABLE)
#define emZclIsAttributeWritable(attribute)                      \
  (READBITS((attribute)->mask, EM_ZCL_ATTRIBUTE_ACCESS_WRITABLE) \
   == EM_ZCL_ATTRIBUTE_ACCESS_WRITABLE)
#define emZclIsAttributeReportable(attribute)                      \
  (READBITS((attribute)->mask, EM_ZCL_ATTRIBUTE_ACCESS_REPORTABLE) \
   == EM_ZCL_ATTRIBUTE_ACCESS_REPORTABLE)
#define emZclIsAttributeBounded(attribute)                    \
  (READBITS((attribute)->mask, EM_ZCL_ATTRIBUTE_DATA_BOUNDED) \
   == EM_ZCL_ATTRIBUTE_DATA_BOUNDED)
#define emZclIsAttributeAnalog(attribute)                    \
  (READBITS((attribute)->mask, EM_ZCL_ATTRIBUTE_DATA_ANALOG) \
   == EM_ZCL_ATTRIBUTE_DATA_ANALOG)
const EmZclAttributeEntry_t *emZclFindAttribute(const EmberZclClusterSpec_t *clusterSpec,
                                                EmberZclAttributeId_t attributeId,
                                                bool includeRemote);
EmberZclStatus_t emZclReadAttributeEntry(EmberZclEndpointId_t endpointId,
                                         const EmZclAttributeEntry_t *attribute,
                                         void *buffer,
                                         size_t bufferLength);
EmberZclStatus_t emZclWriteAttributeEntry(EmberZclEndpointId_t endpointId,
                                          const EmZclAttributeEntry_t *attribute,
                                          const void *buffer,
                                          size_t bufferLength,
                                          bool enableUpdate);
size_t emZclAttributeSize(const EmZclAttributeEntry_t *attribute,
                          const void *data);
bool emZclReadEncodeAttributeKeyValue(CborState *state,
                                      EmberZclEndpointId_t endpointId,
                                      const EmZclAttributeEntry_t *attribute,
                                      void *buffer,
                                      size_t bufferLength);
bool emZclAttributeUriQueryFilterParse(EmZclContext_t *context,
                                       void *data,
                                       uint8_t depth);
bool emZclAttributeUriQueryUndividedParse(EmZclContext_t *context,
                                          void *data,
                                          uint8_t depth);
void emZclUriClusterAttributeHandler(EmZclContext_t *context);
void emZclUriClusterAttributeIdHandler(EmZclContext_t *context);
bool emZclGetAbsDifference(uint8_t *bufferA,
                           uint8_t *bufferB,
                           uint8_t *diffBuffer,
                           size_t size);
void emZclSignExtendAttributeBuffer(uint8_t *outBuffer,
                                    size_t outBufferLength,
                                    const uint8_t *inBuffer,
                                    size_t inBufferLength,
                                    uint8_t attributeType);
uint8_t emZclDirectBufferedZclipType(uint8_t zclipType);
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Bindings.

/**
 * @addtogroup ZCLIP_bindings Bindings
 *
 * See zcl-core.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function checks whether a specified binding exists.
 *
 * @param bindingId A binding identifier to check
 * @return
 * - @ref `true` if binding exists
 * - @ref `false` if binding does not exist
 *****************************************************************************/
bool emberZclHasBinding(EmberZclBindingId_t bindingId);

/**************************************************************************//**
 * This function gets a specified binding.
 *
 * @param bindingId A binding identifier of an entry to get
 * @param entry A binding entry to put the retrieved binding into
 * @return
 * - @ref `true` if binding was retrieved successfully
 * - @ref `false` if binding was not retrieved successfully
 *****************************************************************************/
bool emberZclGetBinding(EmberZclBindingId_t bindingId,
                        EmberZclBindingEntry_t *entry);

/**************************************************************************//**
 * This function sets a specified binding.
 *
 * @param bindingId A binding identifier of entry to set
 * @param entry A new entry to set the binding to
 * @return
 * - @ref `true` if binding was set successfully
 * - @ref `false` if binding was not set successfully
 *****************************************************************************/
bool emberZclSetBinding(EmberZclBindingId_t bindingId,
                        const EmberZclBindingEntry_t *entry);

/**************************************************************************//**
 * This function adds a given entry to the binding table.
 *
 * @param entry A binding entry to add to the table
 * @return
 * - @ref A binding identifier of entry if it was added successfully
 * - @ref EMBER_ZCL_BINDING_NULL if entry was not added successfully
 *
 * This function checks the binding table for duplicates. If a duplicate is
 * found, a binding identifier of the previous entry is used. Otherwise, a new one is
 * allocated. This function also validates contents of the given binding entry.
 *****************************************************************************/
EmberZclBindingId_t emberZclAddBinding(const EmberZclBindingEntry_t *entry);

/**************************************************************************//**
 * This function removes a specified entry from the binding table.
 *
 * @param bindingId A binding identifier of an entry to be removed from the table
 * @return
 * - @ref `true` if an entry was removed successfully
 * - @ref `false` if an entry was not removed successfully
 *****************************************************************************/
bool emberZclRemoveBinding(EmberZclBindingId_t bindingId);

/**************************************************************************//**
 * This function removes all entries from the binding table.
 *
 * @return
 * - @ref `true` if all entries were removed successfully
 * - @ref `false` if all entries were not removed successfully
 *****************************************************************************/
bool emberZclRemoveAllBindings(void);

/**************************************************************************//**
 * This function sends a command to a specified destination to add a binding.
 *
 * @param destination A location to send the command to
 * @param entry A binding entry to add to destination's binding table
 * @param handler A callback that is triggered for a response
 * @return
 * - @ref EMBER_ZCL_STATUS_SUCCESS if the function call was successful
 * - @ref ::EmberStatus with failure reason otherwise
 *
 * @sa EmberZclBindingResponseHandler()
 *****************************************************************************/
EmberStatus emberZclSendAddBinding(const EmberZclDestination_t *destination,
                                   const EmberZclBindingEntry_t *entry,
                                   const EmberZclBindingResponseHandler handler);

/**************************************************************************//**
 * This function sends a command to a specified destination to update a binding.
 *
 * @param destination A location to send a command to
 * @param entry A new binding entry to use for an update
 * @param bindingId A binding identifier to update in destination's binding table
 * @param handler A callback that is triggered for a response
 * @return
 * - @ref EMBER_ZCL_STATUS_SUCCESS if function call was successful
 * - @ref ::EmberStatus with failure reason otherwise
 *
 * @sa EmberZclBindingResponseHandler()
 *****************************************************************************/
EmberStatus emberZclSendUpdateBinding(const EmberZclDestination_t *destination,
                                      const EmberZclBindingEntry_t *entry,
                                      EmberZclBindingId_t bindingId,
                                      const EmberZclBindingResponseHandler handler);

/**************************************************************************//**
 * This function sends a command to a specified destination to remove a binding.
 *
 * @param destination location to send command to
 * @param clusterSpec cluster specification of binding entry to remove
 * @param bindingId Binding identifier to remove from destination's binding table
 * @param handler callback that is triggered for response
 * @return
 * - @ref EMBER_ZCL_STATUS_SUCCESS if function call was successful
 * - @ref ::EmberStatus with failure reason otherwise
 *
 * @sa EmberZclBindingResponseHandler()
 *****************************************************************************/
EmberStatus emberZclSendRemoveBinding(const EmberZclDestination_t *destination,
                                      const EmberZclClusterSpec_t *clusterSpec,
                                      EmberZclBindingId_t bindingId,
                                      const EmberZclBindingResponseHandler handler);

/**************************************************************************//**
 * This function gets destination from specified matching binding
 *
 * @param clusterSpec cluster specification of binding entry to remove
 * @param bindingIdx  index to start searching the binding table (index value
 * is incremented on return if a matching binding is found)
 * @param destination the destination of the matching binding entry
 * @return
 * - @ref `true` if matching binding was found
 * - @ref `false` if matching binding was not found
 *
 * @sa EmberZclGetDestinationFromBinding()
 *****************************************************************************/
bool emberZclGetDestinationFromBinding(const EmberZclClusterSpec_t *clusterSpec,
                                       EmberZclBindingId_t *bindingIdx,
                                       EmberZclDestination_t *destination);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
size_t emZclGetBindingCount(void);
bool emZclHasBinding(const EmZclContext_t *context,
                     EmberZclBindingId_t bindingId);
void emZclUriClusterBindingHandler(EmZclContext_t *context);
void emZclUriClusterBindingIdHandler(EmZclContext_t *context);
void emZclReadDestinationFromBinding(const EmberZclBindingEntry_t *binding,
                                     EmberZclDestination_t *destination);
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Commands.

/**
 * @addtogroup ZCLIP_commands Commands
 *
 * See zcl-core.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function sends a default response to a command.
 *
 * @param context A command context for the response
 * @param status A status to respond with
 * @return
 * - @ref EMBER_ZCL_STATUS_SUCCESS if response was sent successfully
 * - @ref ::EmberStatus with failure reason otherwise
 *****************************************************************************/
EmberStatus emberZclSendDefaultResponse(const EmberZclCommandContext_t *context,
                                        EmberZclStatus_t status);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern const EmZclCommandEntry_t emZclCommandTable[];
extern const size_t emZclCommandCount;
const EmZclCommandEntry_t *emZclFindCommand(const EmberZclClusterSpec_t *clusterSpec,
                                            EmberZclCommandId_t commandId);
EmberStatus emZclSendCommandRequest(const EmberZclDestination_t *destination,
                                    const EmberZclClusterSpec_t *clusterSpec,
                                    EmberZclCommandId_t commandId,
                                    const void *request,
                                    const ZclipStructSpec *requestSpec,
                                    const ZclipStructSpec *responseSpec,
                                    const EmZclResponseHandler handler);
EmberStatus emZclSendCommandResponse(const EmberZclCommandContext_t *context,
                                     const void *response,
                                     const ZclipStructSpec *responseSpec);
void emZclUriClusterCommandHandler(EmZclContext_t *context);
void emZclUriClusterCommandIdHandler(EmZclContext_t *context);
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Reporting.

/**
 * @addtogroup ZCLIP_reporting Reporting
 *
 * See zcl-core.h for source code.
 * @{
 */

#ifndef DOXYGEN_SHOULD_SKIP_THIS

bool emZclHasReportingConfiguration(EmberZclEndpointId_t endpointId,
                                    const EmberZclClusterSpec_t *clusterSpec,
                                    EmberZclReportingConfigurationId_t reportingConfigurationId);
void emZclUriClusterNotificationHandler(EmZclContext_t *context);
void emZclUriClusterReportingConfigurationHandler(EmZclContext_t *context);
void emZclUriClusterReportingConfigurationIdHandler(EmZclContext_t *context);

#endif

/** @brief This function performs a factory reset of the reporting configurations:-
 * 1. All entries in nv reporting configurations table are erased
 * 2. Default configurations for each endpoint/clusterSpec are restored
 * to their initial values and saved to nv.
 */
void emberZclReportingConfigurationsFactoryReset(EmberZclEndpointId_t endpointId);

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Utilities.

/**
 * @addtogroup ZCLIP_utilities Utilities
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function returns the length of the octet or character data in a given string.
 *
 * @param buffer string pointer
 * @return length of string
 *****************************************************************************/
uint8_t emberZclStringLength(const uint8_t *buffer);

/**************************************************************************//**
 * This function returns the size of a given string including overhead and data.
 *
 * @param buffer string pointer
 * @return size of string
 *****************************************************************************/
uint8_t emberZclStringSize(const uint8_t *buffer);

/**************************************************************************//**
 * This function returns the length of the octet or character data in a given long string.
 *
 * @param buffer string pointer
 * @return length of string
 *****************************************************************************/
uint16_t emberZclLongStringLength(const uint8_t *buffer);

/**************************************************************************//**
 * This function returns the size of a given long string including overhead and data.
 *
 * @param buffer string pointer
 * @return size of string
 *****************************************************************************/
uint16_t emberZclLongStringSize(const uint8_t *buffer);

#ifndef DOXYGEN_SHOULD_SKIP_THIS

const uint8_t *emZclGetMessageStatusName(EmberZclMessageStatus_t status);

// URI segment matching functions
bool emZclUriPathStringMatch       (EmZclContext_t *context, void *castString, uint8_t depth);
bool emZclUriQueryStringPrefixMatch(EmZclContext_t *context, void *castString, uint8_t depth);

// If a response handler is supplied it must call emZclCoapStatusHandler()
// with the status and response info.  This is so that the system can detect
// broken UID<->address associations.
EmberStatus emZclSend(const EmberZclCoapEndpoint_t *destination,
                      EmberCoapCode code,
                      const uint8_t *uri,
                      const uint8_t *payload,
                      uint16_t payloadLength,
                      EmberCoapResponseHandler handler,
                      void *applicationData,
                      uint16_t applicationDataLength);
void emZclCoapStatusHandler(EmberCoapStatus status, EmberCoapResponseInfo *info);
EmberStatus emZclRespondCborState(EmberCoapCode code,
                                  const CborState *state);
EmberStatus emZclRespondCborPayload(EmberCoapCode code,
                                    const uint8_t *payload,
                                    uint16_t payloadLength);
EmberStatus emZclRespondLinkFormatPayload(EmberCoapCode code,
                                          const uint8_t *payload,
                                          uint16_t payloadLength,
                                          EmberCoapContentFormatType contentFormatType);
EmberStatus emZclRespondWithStatus(EmberCoapCode code, EmberZclStatus_t status);
EmberStatus emZclRespond201Created(const uint8_t *locationPath);
EmberStatus emZclRespond202Deleted(void);
EmberStatus emZclRespond204Changed(void);
EmberStatus emZclRespond204ChangedCborState(const CborState *state);
EmberStatus emZclRespond205ContentCbor(const uint8_t *payload,
                                       uint16_t payloadLength);
EmberStatus emZclRespond205ContentCborState(const CborState *state);
EmberStatus emZclRespond205ContentLinkFormat(const uint8_t *payload,
                                             uint16_t payloadLength,
                                             EmberCoapContentFormatType contentFormatType);
EmberStatus emZclRespond400BadRequest(void);
EmberStatus emZclRespond400BadRequestCborState(const CborState *state);
EmberStatus emZclRespond404NotFound(void);
EmberStatus emZclRespond405MethodNotAllowed(void);
EmberStatus emZclRespond406NotAcceptable(void);
EmberStatus emZclRespond412PreconditionFailedCborState(const CborState *state);
EmberStatus emZclRespond413RequestEntityTooLarge(void);
EmberStatus emZclRespond415UnsupportedContentFormat(void);
EmberStatus emZclRespond500InternalServerError(void);

uint8_t emZclIntToHexString(uint32_t value, size_t size, uint8_t *result);
bool emZclHexStringToInt(const uint8_t *chars, size_t length, uintmax_t *result);
size_t emZclClusterToString(const EmberZclClusterSpec_t *clusterSpec,
                            uint8_t *result);
bool emZclStringToCluster(const uint8_t *chars,
                          size_t length,
                          EmberZclClusterSpec_t *clusterSpec);
uint16_t emZclParseUri(const uint8_t *payload, EmZclUriContext_t *context);

size_t emZclThingToUriPath(const EmberZclApplicationDestination_t *destination,
                           const EmberZclClusterSpec_t *clusterSpec,
                           char thing,
                           uint8_t *result);

size_t emZclDestinationToUri(const EmberZclDestination_t *destination,
                             uint8_t *result);
bool emZclUriToBindingEntry(const uint8_t *uri,
                            EmberZclBindingEntry_t *result,
                            bool includeCluster);
size_t emZclUidToString(const EmberZclUid_t *uid, uint16_t uidBits, uint8_t *result);
bool emZclStringToUid(const uint8_t *uid,
                      size_t length,
                      EmberZclUid_t *result,
                      uint16_t *resultBits);
bool emZclConvertBase64UrlToCode(const uint8_t *base64Url,
                                 uint16_t length,
                                 uint8_t *code);
size_t emZclUidToBase64Url(const EmberZclUid_t *uid,
                           uint16_t uidBits,
                           uint8_t *base64Url);
bool emZclBase64UrlToUid(const uint8_t *base64Url,
                         size_t length,
                         EmberZclUid_t *result,
                         uint16_t *resultBits);
bool emZclNiUriToUid(const uint8_t *uri, uint16_t uriLength, EmberZclUid_t *uid);

EmberZclStatus_t emZclCborValueReadStatusToEmberStatus(EmZclCoreCborValueReadStatus_t cborValueReadStatus);

#define emZclClusterToUriPath(address, clusterSpec, result) \
  emZclThingToUriPath(address, clusterSpec, ' ', result)
#define emZclAttributeToUriPath(address, clusterSpec, result) \
  emZclThingToUriPath(address, clusterSpec, 'a', result)
#define emZclBindingToUriPath(address, clusterSpec, result) \
  emZclThingToUriPath(address, clusterSpec, 'b', result)
#define emZclCommandToUriPath(address, clusterSpec, result) \
  emZclThingToUriPath(address, clusterSpec, 'c', result)
#define emZclNotificationToUriPath(address, clusterSpec, result) \
  emZclThingToUriPath(address, clusterSpec, 'n', result)
#define emZclReportingConfigurationToUriPath(address, clusterSpec, result) \
  emZclThingToUriPath(address, clusterSpec, 'r', result)

size_t emZclThingIdToUriPath(const EmberZclApplicationDestination_t *destination,
                             const EmberZclClusterSpec_t *clusterSpec,
                             char thing,
                             uintmax_t thingId,
                             size_t size,
                             uint8_t *result);
#define emZclAttributeIdToUriPath(address, clusterSpec, attributeId, result) \
  emZclThingIdToUriPath(address, clusterSpec, 'a', attributeId, sizeof(EmberZclAttributeId_t), result)
#define emZclBindingIdToUriPath(address, clusterSpec, bindingId, result) \
  emZclThingIdToUriPath(address, clusterSpec, 'b', bindingId, sizeof(EmberZclBindingId_t), result)
#define emZclCommandIdToUriPath(address, clusterSpec, commandId, result) \
  emZclThingIdToUriPath(address, clusterSpec, 'c', commandId, sizeof(EmberZclCommandId_t), result)
#define emZclReportingConfigurationIdToUriPath(address, clusterSpec, reportingConfigurationId, result) \
  emZclThingIdToUriPath(address, clusterSpec, 'r', reportingConfigurationId, sizeof(EmberZclReportingConfigurationId_t), result)

bool emZclGetMulticastAddress(EmberIpv6Address * dst);
#endif

/** @} end addtogroup */

// ----------------------------------------------------------------------------
// CLI.

#ifndef DOXYGEN_SHOULD_SKIP_THIS
bool emZclCliGetUidArgument(uint8_t index, EmberZclUid_t *uid);
void emZclCliResetCliState(void);
void emZclCliSetCurrentRequestCommand(const EmberZclClusterSpec_t *clusterSpec,
                                      EmberZclCommandId_t commandId,
                                      const ZclipStructSpec *structSpec,
                                      EmZclCliRequestCommandFunction function,
                                      const char *cliFormat);
#endif

/** @} end addtogroup */

#endif // __ZCL_CORE_H__
