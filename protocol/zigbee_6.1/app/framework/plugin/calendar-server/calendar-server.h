// *******************************************************************
// * calendar-server.h
// *
// *
// * Copyright 2014 by Silicon Laboratories. All rights reserved.           *80*
// *******************************************************************

/**
 * @brief Publish an calendar.
 *
 * This function will locate the calendar in the calendar table at the specified
 * location and using the information from the calendar build and send a
 * PublishCalendar command.
 *
 * @param nodeId The destination nodeId
 * @param srcEndpoint The source endpoint
 * @param dstEndpoint The destination endpoint
 * @param calendarIndex The index in the calendar table.
 **/
void emberAfCalendarServerPublishCalendarMessage(EmberNodeId nodeId,
                                                 uint8_t srcEndpoint,
                                                 uint8_t dstEndpoint,
                                                 uint8_t calendarIndex);

/**
 * @brief Publish the day profiles of the specified day in the specified calendar.
 *
 * This function will locate the calendar in the calendar table at the specified
 * location and using the information from the calendar build and send a
 * PublishDayProfiles command.
 *
 * @param nodeId The destination nodeId
 * @param srcEndpoint The source endpoint
 * @param dstEndpoint The destination endpoint
 * @param calendarIndex The index in the calendar table.
 * @param dayIndex The index of the day in the calendar.
 **/
void emberAfCalendarServerPublishDayProfilesMessage(EmberNodeId nodeId,
                                                    uint8_t srcEndpoint,
                                                    uint8_t dstEndpoint,
                                                    uint8_t calendarIndex,
                                                    uint8_t dayIndex);

/**
 * @brief Publish the week profile of the specified week in the specified calendar.
 *
 * This function will locate the calendar in the calendar table at the specified
 * location and using the information from the calendar build and send a
 * PublishWeekProfile command.
 *
 * @param nodeId The destination nodeId
 * @param srcEndpoint The source endpoint
 * @param dstEndpoint The destination endpoint
 * @param calendarIndex The index in the calendar table.
 * @param weekIndex The index of the week in the calendar.
 **/
void emberAfCalendarServerPublishWeekProfileMessage(EmberNodeId nodeId,
                                                    uint8_t srcEndpoint,
                                                    uint8_t dstEndpoint,
                                                    uint8_t calendarIndex,
                                                    uint8_t weekIndex);

/**
 * @brief Publish the seasons in the specified calendar.
 *
 * This function will locate the calendar in the calendar table at the specified
 * location and using the information from the calendar build and send a
 * PublishSeasons command.
 *
 * @param nodeId The destination nodeId
 * @param srcEndpoint The source endpoint
 * @param dstEndpoint The destination endpoint
 * @param calendarIndex The index in the calendar table.
 **/
void emberAfCalendarServerPublishSeasonsMessage(EmberNodeId nodeId,
                                                uint8_t srcEndpoint,
                                                uint8_t dstEndpoint,
                                                uint8_t calendarIndex);

/**
 * @brief Publish the special days of the specified calendar.
 *
 * This function will locate the calendar in the calendar table at the specified
 * location and using the information from the calendar build and send a
 * PublishSpecialDays command.
 *
 * @param nodeId The destination nodeId
 * @param srcEndpoint The source endpoint
 * @param dstEndpoint The destination endpoint
 * @param calendarIndex The index in the calendar table.
 **/
void emberAfCalendarServerPublishSpecialDaysMessage(EmberNodeId nodeId,
                                                    uint8_t srcEndpoint,
                                                    uint8_t dstEndpoint,
                                                    uint8_t calendarIndex);

/**
 * @brief Publish the special days of the specified calendar.
 *
 * This function will locate the calendar in the calendar table at the specified
 * location and using the information from the calendar build and send a
 * CancelCalendar command. Note: it's is up to the caller to invalidate
 * the local copy of the calendar.
 *
 * @param nodeId The destination nodeId
 * @param srcEndpoint The source endpoint
 * @param dstEndpoint The destination endpoint
 * @param calendarIndex The index in the calendar table.
 **/
void emberAfCalendarServerCancelCalendarMessage(EmberNodeId nodeId,
                                                uint8_t srcEndpoint,
                                                uint8_t dstEndpoint,
                                                uint8_t calendarIndex);
