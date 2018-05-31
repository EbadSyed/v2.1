/**
 * child.h
 *
 * Copyright 2014 by Silicon Laboratories. All rights reserved.            *80*
 */

#ifndef CHILD_H
#define CHILD_H

/** @brief This function converts a child index to a node ID.
 *
 * @param childIndex  The index.
 *
 * @return The node ID of the child or ::EMBER_NULL_NODE_ID  a
 *         child isn't at the childIndex specified.
 */
EmberNodeId emberChildId(uint8_t childIndex);

/** @brief This function converts a node ID to a child index.
 *
 * @param childId  The node ID of the child.
 *
 * @return The child index or 0xFF if the node ID does not belong to a child.
 */
uint8_t emberChildIndex(EmberNodeId childId);

/** @brief This function sets a flag to indicate a message is pending for a child.
 * Next time that the child polls, it will be informed that it has
 * a pending message. The message is sent from emberPollHandler, which is called
 * when the child requests the data.
 *
 * @param childId  The ID of the child that just polled for data.
 *
 * @return An ::EmberStatus value.
 * - ::EMBER_SUCCESS - The next time that the child polls, it will be informed
 *     that it has pending data.
 * - ::EMBER_NOT_JOINED - The child identified by childId is not our child
 *    (it is not in the PAN).
 */
EmberStatus emberSetMessageFlag(EmberNodeId childId);

/** @brief This function clears a flag to indicate that no more messages are available for a
 * child. Next time the child polls, it will be informed that it does not
 * have any pending messages.
 *
 * @param childId  The ID of the child that no longer has pending messages.
 *
 * @return An ::EmberStatus value.
 * - ::EMBER_SUCCESS - The next time that the child polls, it will be informed
 *     that it does not have any pending messages.
 * - ::EMBER_NOT_JOINED - The child identified by childId is not our child
 *     (it is not in the PAN).
 */
EmberStatus emberClearMessageFlag(EmberNodeId childId);

/** @brief A callback that allows the application to send a message in
 * response to a poll from a child.
 *
 * This function is called when a child polls,
 * provided that the pending message flag is set for that child
 * (see ::emberSetMessageFlag(). The message should be sent to the child
 * using ::emberSendUnicast() with the ::EMBER_APS_OPTION_POLL_RESPONSE option.
 *
 * If the application includes ::emberPollHanlder(), it must
 * define EMBER_APPLICATION_HAS_POLL_HANDLER in its CONFIGURATION_HEADER.
 *
 * @param childId           The ID of the child that is requesting data.
 *
 * @param transmitExpected  true if the child is expecting an application-
 *     supplied data message.  false otherwise.
 *
 */
void emberPollHandler(EmberNodeId childId, bool transmitExpected);

#endif // CHILD_H
