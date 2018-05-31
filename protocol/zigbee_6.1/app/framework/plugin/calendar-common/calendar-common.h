#define fieldLength(field) \
  (emberAfCurrentCommand()->bufLen - (field - emberAfCurrentCommand()->buffer));

#define EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_SCHEDULE_ENTRY 0xFFFF
#define EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_ID 0xFF
#define EMBER_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH 12
#define EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_INDEX 0xFF

#define SCHEDULE_ENTRY_SIZE (3)
typedef struct {
  uint16_t minutesFromMidnight;

  // the format of the actual data in the entry depends on the calendar type
  //   for calendar type 00 - 0x02, it is a rate switch time
  //     the data is a price tier enum (8-bit)
  //   for calendar type 0x03 it is friendly credit switch time
  //     the data is a bool (8-bit) meaning friendly credit enabled
  //   for calendar type 0x04 it is an auxilliary load switch time
  //     the data is a bitmap (8-bit)
  uint8_t data;
} EmberAfCalendarDayScheduleEntryStruct;

// Season start date (4-bytes) and week ID ref (1-byte)
#define SEASON_ENTRY_SIZE (5)

typedef struct {
  EmberAfCalendarDayScheduleEntryStruct scheduleEntries[EMBER_AF_PLUGIN_CALENDAR_COMMON_SCHEDULE_ENTRIES_MAX];
  uint8_t id;
  uint8_t numberOfScheduleEntries;
} EmberAfCalendarDayStruct;

// Special day date (4 bytes) and Day ID ref (1-byte)
#define SPECIAL_DAY_ENTRY_SIZE (5)
typedef struct {
  EmberAfDate startDate;
  uint8_t normalDayIndex;
  uint8_t flags;
} EmberAfCalendarSpecialDayStruct;

#define EMBER_AF_PLUGIN_CALENDAR_COMMON_MONDAY_INDEX (0)
#define EMBER_AF_PLUGIN_CALENDAR_COMMON_SUNDAY_INDEX (6)
#define EMBER_AF_DAYS_IN_THE_WEEK (7)

typedef struct {
  uint8_t normalDayIndexes[EMBER_AF_DAYS_IN_THE_WEEK];
  uint8_t id;
} EmberAfCalendarWeekStruct;

typedef struct {
  EmberAfDate startDate;
  uint8_t weekIndex;
} EmberAfCalendarSeasonStruct;

#define EMBER_AF_PLUGIN_CALENDAR_COMMON_INVALID_CALENDAR_ID 0xFFFFFFFF
#define EMBER_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_CALENDAR_ID 0x00000000
#define EMBER_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_PROVIDER_ID 0xFFFFFFFF
#define EMBER_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_ISSUER_ID 0xFFFFFFFF
#define EMBER_AF_PLUGIN_CALENDAR_COMMON_WILDCARD_CALENDAR_TYPE 0xFF

enum {
  EMBER_AF_PLUGIN_CALENDAR_COMMON_FLAGS_SENT = 0x01,
};

typedef struct {
  EmberAfCalendarWeekStruct weeks[EMBER_AF_PLUGIN_CALENDAR_COMMON_WEEK_PROFILE_MAX];
  EmberAfCalendarDayStruct normalDays[EMBER_AF_PLUGIN_CALENDAR_COMMON_DAY_PROFILE_MAX];
  EmberAfCalendarSpecialDayStruct specialDays[EMBER_AF_PLUGIN_CALENDAR_COMMON_SPECIAL_DAY_PROFILE_MAX];
  EmberAfCalendarSeasonStruct seasons[EMBER_AF_PLUGIN_CALENDAR_COMMON_SEASON_PROFILE_MAX];
  uint32_t providerId;
  uint32_t issuerEventId;
  uint32_t calendarId;
  uint32_t startTimeUtc;
  uint8_t name[EMBER_AF_PLUGIN_CALENDAR_MAX_CALENDAR_NAME_LENGTH + 1];
  uint8_t calendarType;
  uint8_t numberOfSeasons;
  uint8_t numberOfWeekProfiles;
  uint8_t numberOfDayProfiles;
  uint8_t numberOfSpecialDayProfiles;

  /* these "received" counter don't really belong here. it's here to help with
   * replaying TOM messages correctly. They'll serve as destination index
   * for the next publish command.
   */
  uint8_t numberOfReceivedSeasons;
  uint8_t numberOfReceivedWeekProfiles;
  uint8_t numberOfReceivedDayProfiles;
  uint8_t flags;
} EmberAfCalendarStruct;

extern EmberAfCalendarStruct calendars[];
#if defined(EMBER_AF_PLUGIN_GAS_PROXY_FUNCTION)
#define GBCS_TARIFF_SWITCHING_CALENDAR_ID 0xFFFFFFFF
#define GBCS_NON_DISABLEMENT_CALENDAR_ID  0xFFFFFFFE
extern uint32_t tariffSwitchingCalendarId;
extern uint32_t nonDisablementCalendarId;
#endif

uint8_t emberAfPluginCalendarCommonGetCalendarById(uint32_t calendarId,
                                                   uint32_t providerId);
uint32_t emberAfPluginCalendarCommonEndTimeUtc(const EmberAfCalendarStruct *calendar);
bool emberAfCalendarCommonSetCalInfo(uint8_t index,
                                     uint32_t providerId,
                                     uint32_t issuerEventId,
                                     uint32_t issuerCalendarId,
                                     uint32_t startTimeUtc,
                                     uint8_t calendarType,
                                     uint8_t *calendarName,
                                     uint8_t numberOfSeasons,
                                     uint8_t numberOfWeekProfiles,
                                     uint8_t numberOfDayProfiles);

/* @brief add new entry corresponding to PublishCalendar command.
 *
 * This function will try to handle the new entry in following method:
 *
 * 1) Try to apply new data to a matching existing entry.
 *    Fields such as providerId, issuerEventId, and startTime, will be used.
 * 3) Overwrite the oldest entry (one with smallest event id) with new info.
 *
 */
bool emberAfCalendarCommonAddCalInfo(uint32_t providerId,
                                     uint32_t issuerEventId,
                                     uint32_t issuerCalendarId,
                                     uint32_t startTimeUtc,
                                     uint8_t calendarType,
                                     uint8_t *calendarName,
                                     uint8_t numberOfSeasons,
                                     uint8_t numberOfWeekProfiles,
                                     uint8_t numberOfDayProfiles);
bool emberAfCalendarServerSetSeasonsInfo(uint8_t index,
                                         uint8_t seasonId,
                                         EmberAfDate startDate,
                                         uint8_t weekIndex);
bool emberAfCalendarServerAddSeasonsInfo(uint32_t issuerCalendarId,
                                         uint8_t * seasonsEntries,
                                         uint8_t seasonsEntriesLength,
                                         uint8_t * unknownWeekIdSeasonsMask);

bool emberAfCalendarCommonSetDayProfInfo(uint8_t index,
                                         uint8_t dayId,
                                         uint8_t entryId,
                                         uint16_t minutesFromMidnight,
                                         uint8_t data);
bool emberAfCalendarCommonAddDayProfInfo(uint32_t issuerCalendarId,
                                         uint8_t dayId,
                                         uint8_t * dayScheduleEntries,
                                         uint16_t dayScheduleEntriesLength);
bool emberAfCalendarServerSetWeekProfInfo(uint8_t index,
                                          uint8_t weekId,
                                          uint8_t dayIdRefMon,
                                          uint8_t dayIdRefTue,
                                          uint8_t dayIdRefWed,
                                          uint8_t dayIdRefThu,
                                          uint8_t dayIdRefFri,
                                          uint8_t dayIdRefSat,
                                          uint8_t dayIdRefSun);
bool emberAfCalendarServerAddWeekProfInfo(uint32_t issuerCalendarId,
                                          uint8_t weekId,
                                          uint8_t dayIdRefMon,
                                          uint8_t dayIdRefTue,
                                          uint8_t dayIdRefWed,
                                          uint8_t dayIdRefThu,
                                          uint8_t dayIdRefFri,
                                          uint8_t dayIdRefSat,
                                          uint8_t dayIdRefSun);

/* @brief Updating special days info of the specified calendar.
 *
 * This function assumes that the value of totalNumberOfSpecialDays will match
 * up with the info passed in between specialDaysEntries/specialDaysEntriesLength.
 *
 */
bool emberAfCalendarCommonAddSpecialDaysInfo(uint32_t issuerCalendarId,
                                             uint8_t totalNumberOfSpecialDays,
                                             uint8_t * specialDaysEntries,
                                             uint16_t specialDaysEntriesLength,
                                             uint8_t * unknownSpecialDaysMask);
