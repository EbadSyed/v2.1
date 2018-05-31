// Copyright 2007 - 2012 by Ember Corporation. All rights reserved.
//
//

#include "../../include/af.h"
#include "../../util/common.h"
#include "door-lock-server.h"

static EmberAfDoorLockUser userTable[DOOR_LOCK_USER_TABLE_SIZE];
static EmberAfDoorLockScheduleEntry schedule[DOOR_LOCK_SCHEDULE_TABLE_SIZE];

static void printPin(uint8_t pinLen, uint8_t *PIN);
static void printUserTable(void);
static void printScheduleTable(void);
static void printSuccessOrFailure(bool success);
static bool checkForSufficientSpace(uint8_t spaceReq, uint8_t spaceAvail);
static bool verifyPin(uint8_t* PIN, uint8_t* userId);

static void printScheduleTable(void)
{
  uint8_t i;
  emberAfDoorLockClusterPrintln("id uid dm strth strtm stph stpm");
  for (i = 0; i < DOOR_LOCK_SCHEDULE_TABLE_SIZE; i++ ) {
    EmberAfDoorLockScheduleEntry *dlse = &schedule[i];
    emberAfDoorLockClusterPrintln("%x %x  %x %4x   %4x   %4x  %4x",
                                  i, dlse->userID, dlse->daysMask, dlse->startHour, dlse->stopHour,
                                  dlse->stopMinute);
  }
}

static void printUserTable(void)
{
  uint8_t i;
  emberAfDoorLockClusterPrintln("id st ty pl PIN");
  for (i = 0; i < DOOR_LOCK_USER_TABLE_SIZE; i++) {
    EmberAfDoorLockUser *user = &userTable[i];
    emberAfDoorLockClusterPrint("%x %x %x %x ", i, user->status, user->type, user->pinLength);
    printPin(user->pinLength, user->pin);
    emberAfDoorLockClusterPrintln("");
  }
}

static void printPin(uint8_t pinLen, uint8_t* PIN)
{
  uint8_t i;
  if (PIN != NULL) {
    emberAfDoorLockClusterPrint("(%x)", pinLen);
    for (i = 0; i < pinLen; i++) {
      emberAfDoorLockClusterPrint(" %c", PIN[i]);
    }
  }
}

static void printSuccessOrFailure(bool success)
{
  if (success) {
    emberAfDoorLockClusterPrintln("SUCCESS!");
  } else {
    emberAfDoorLockClusterPrintln("FAILURE!");
  }
}

/**
 * This function checks to see if a pin is required and, if it is
 * and a pin is provided, it validates the pin against those known in
 * the user table.
 */
static bool verifyPin(uint8_t* PIN, uint8_t* userId)
{
  bool pinRequired = false;
  EmberStatus status;
  uint8_t i;

  status = emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT,
                                      ZCL_DOOR_LOCK_CLUSTER_ID,
                                      ZCL_REQUIRE_PIN_FOR_RF_OPERATION_ATTRIBUTE_ID,
                                      (uint8_t *) &pinRequired,
                                      sizeof(pinRequired));
  if (EMBER_SUCCESS != status || !pinRequired) {
    return true;
  } else if (PIN == NULL) {
    return false;
  }

  for (i = 0; i < DOOR_LOCK_USER_TABLE_SIZE; i++) {
    EmberAfDoorLockUser *user = &userTable[i];
    if (user->pinLength != emberAfStringLength(PIN)) {
      continue;
    }
    if (0 == MEMCOMPARE(&(user->pin), (&PIN[1]), emberAfStringLength(PIN))) {
      *userId = i;
      return true;
    }
  }
  return false;
}

static bool checkForSufficientSpace(uint8_t spaceReq, uint8_t spaceAvail)
{
  if (spaceReq > spaceAvail) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INSUFFICIENT_SPACE);
    return false;
  }
  return true;
}

/** @brief Lock Door
 *
 *
 * @param PIN   Ver.: since ha-1.2-11-5474-19
 */
bool emberAfDoorLockClusterLockDoorCallback(uint8_t* PIN)
{
  uint8_t userId = 0;
  bool pinVerified = verifyPin(PIN, &userId);
  bool doorLocked = false;
  uint8_t lockStateLocked = 0x01;
  uint16_t rfOperationEventMask = 0xffff; //will send events by default

  emberAfDoorLockClusterPrint("LOCK DOOR ");
  printSuccessOrFailure(pinVerified);

  if (pinVerified) {
    doorLocked =
      emberAfPluginDoorLockServerActivateDoorLockCallback(true); //lock door
  }

  if (doorLocked) {
    emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT,
                                ZCL_DOOR_LOCK_CLUSTER_ID,
                                ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                &lockStateLocked,
                                ZCL_INT8U_ATTRIBUTE_TYPE);
  }

  //send response
  emberAfFillCommandDoorLockClusterLockDoorResponse(doorLocked
                                                    ? EMBER_ZCL_STATUS_SUCCESS
                                                    : EMBER_ZCL_STATUS_FAILURE);
  emberAfSendResponse();

  //check if we should send event notification
  emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT,
                             ZCL_DOOR_LOCK_CLUSTER_ID,
                             ZCL_RF_OPERATION_EVENT_MASK_ATTRIBUTE_ID,
                             (uint8_t*)&rfOperationEventMask,
                             sizeof(rfOperationEventMask));

  // Possibly send operation event
  if (doorLocked) {
    if (rfOperationEventMask & BIT(1) && (PIN != NULL)) {
      emberAfFillCommandDoorLockClusterOperationEventNotification(0x01, 0x01, userId, PIN, 0x00, PIN);
    }
  } else {
    if (rfOperationEventMask & BIT(3) && (PIN != NULL)) {
      emberAfFillCommandDoorLockClusterOperationEventNotification(0x01, 0x03, userId, PIN, 0x00, PIN);
    }
  }
  emberAfSendCommandUnicastToBindings();

  return true;
}

/** @brief Unlock Door
 *
 *
 * @param PIN   Ver.: since ha-1.2-11-5474-19
 */
bool emberAfDoorLockClusterUnlockDoorCallback(uint8_t* PIN)
{
  uint8_t userId = 0;
  bool pinVerified = verifyPin(PIN, &userId);
  bool doorUnlocked = false;
  uint8_t lockStateUnlocked = 0x02;
  uint16_t rfOperationEventMask = 0xffff; //sends event by default
  emberAfDoorLockClusterPrint("UNLOCK DOOR ");
  printSuccessOrFailure(pinVerified);

  if (pinVerified) {
    doorUnlocked =
      emberAfPluginDoorLockServerActivateDoorLockCallback(false); //unlock door
  }
  if (doorUnlocked) {
    emberAfWriteServerAttribute(DOOR_LOCK_SERVER_ENDPOINT,
                                ZCL_DOOR_LOCK_CLUSTER_ID,
                                ZCL_LOCK_STATE_ATTRIBUTE_ID,
                                &lockStateUnlocked,
                                ZCL_INT8U_ATTRIBUTE_TYPE);
  }

  emberAfFillCommandDoorLockClusterUnlockDoorResponse(doorUnlocked
                                                      ? EMBER_ZCL_STATUS_SUCCESS
                                                      : EMBER_ZCL_STATUS_FAILURE);
  emberAfSendResponse();

  //get bitmask so we can check if we should send event notification
  emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT,
                             ZCL_DOOR_LOCK_CLUSTER_ID,
                             ZCL_RF_OPERATION_EVENT_MASK_ATTRIBUTE_ID,
                             (uint8_t*)&rfOperationEventMask,
                             sizeof(rfOperationEventMask));

  //send operation event
  if (doorUnlocked) {
    if (rfOperationEventMask & BIT(2) && (PIN != NULL)) {
      emberAfFillCommandDoorLockClusterOperationEventNotification(0x01, 0x02, userId, PIN, 0x00, PIN);
    }
  } else {
    if (rfOperationEventMask & BIT(5) && (PIN != NULL)) {
      emberAfFillCommandDoorLockClusterOperationEventNotification(0x01, 0x05, userId, PIN, 0x00, PIN);
    }
  }
  emberAfSendCommandUnicastToBindings();

  return true;
}

/** @brief Set P I N
 *
 *
 * @param userID   Ver.: always
 * @param userStatus   Ver.: always
 * @param userType   Ver.: always
 * @param PIN   Ver.: always
 */
bool emberAfDoorLockClusterSetPinCallback(uint16_t userID,
                                          uint8_t userStatus,
                                          uint8_t userType,
                                          uint8_t* PIN)
{
  uint8_t i, status = 0x00;
  uint16_t rfProgrammingEventMask = 0xffff; //send event by default
  emberAfDoorLockClusterPrintln("***RX SET PIN***");
  emberAfDoorLockClusterFlush();

  if (!checkForSufficientSpace(userID, DOOR_LOCK_USER_TABLE_SIZE)
      || !checkForSufficientSpace(PIN[0], DOOR_LOCK_MAX_PIN_LENGTH)) {
    status = 0x01;     //general failure
  }

  if (!status) {
    EmberAfDoorLockUser *user = &userTable[userID];
    user->status = userStatus;
    user->type = userType;
    user->pinLength = PIN[0];
    for (i = 0; i < PIN[0]; i++) {
      //first digit in PIN is length because it is a ZigBee String
      user->pin[i] = PIN[i + 1];
    }
    printUserTable();
  }

  //send response
  emberAfFillCommandDoorLockClusterSetPinResponse(status);
  emberAfSendResponse();

  //get bitmask so we can check if we should send event notification
  emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT,
                             ZCL_DOOR_LOCK_CLUSTER_ID,
                             ZCL_RF_PROGRAMMING_EVENT_MASK_ATTRIBUTE_ID,
                             (uint8_t*)&rfProgrammingEventMask,
                             sizeof(rfProgrammingEventMask));

  if ((rfProgrammingEventMask & BIT(1)) && !status && (PIN != NULL)) {
    emberAfFillCommandDoorLockClusterProgrammingEventNotification(0x01, 0x02, userID, PIN, 0x00, 0x00, 0x00, PIN);
    emberAfSendCommandUnicastToBindings();
  } else if ((rfProgrammingEventMask & BIT(0)) && status && (PIN != NULL)) {
    emberAfFillCommandDoorLockClusterProgrammingEventNotification(0x01, 0x00, userID, PIN, 0x00, 0x00, 0x00, PIN);
    emberAfSendCommandUnicastToBindings();
  }

  return true;
}

/** @brief Get P I N
 *
 *
 * @param userID   Ver.: always
 */
bool emberAfDoorLockClusterGetPinCallback(uint16_t userID)
{
  uint8_t i, userStatus, userType, codeLength = 0;
  uint8_t PIN[8];
  PIN[0] = 0x00; //init PIN to zero length string

  if (checkForSufficientSpace(userID, DOOR_LOCK_USER_TABLE_SIZE)) {
    EmberAfDoorLockUser *user = &userTable[userID];
    userStatus = user->status;
    userType = user->type;
    codeLength = user->pinLength;
    PIN[0] = codeLength;
    for (i = 0; i < codeLength; i++) {
      //first digit in PIN is length because it is a ZigBee String
      PIN[i + 1] = user->pin[i];
    }

    emberAfFillCommandDoorLockClusterGetPinResponse(userID,
                                                    userStatus,
                                                    userType,
                                                    &PIN);
    emberAfSendResponse();
  }

  return true;
}

/** @brief Clear P I N
 *
 *
 * @param userID   Ver.: always
 */
bool emberAfDoorLockClusterClearPinCallback(uint16_t userID)
{
  uint8_t status = 0x01;
  uint8_t userPin = 0x00; //zero length string
  uint16_t rfProgrammingEventMask = 0xffff; //event sent by default
  if (checkForSufficientSpace(userID, DOOR_LOCK_USER_TABLE_SIZE)) {
    EmberAfDoorLockUser *user = &userTable[userID];
    MEMSET(user, 0, sizeof(EmberAfDoorLockUser));
    status = 0x00;
  }
  emberAfFillCommandDoorLockClusterClearPinResponse(status);
  emberAfSendResponse();

  //get bitmask so we can check if we should send event notification
  emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT,
                             ZCL_DOOR_LOCK_CLUSTER_ID,
                             ZCL_RF_PROGRAMMING_EVENT_MASK_ATTRIBUTE_ID,
                             (uint8_t*)&rfProgrammingEventMask,
                             sizeof(rfProgrammingEventMask));

  if ((rfProgrammingEventMask & BIT(2)) && !status) {
    emberAfFillCommandDoorLockClusterProgrammingEventNotification(0x01, 0x03, userID, &userPin, 0x00, 0x00, 0x00, &userPin);
    emberAfSendCommandUnicastToBindings();
  } else if ((rfProgrammingEventMask & BIT(0)) && status) {
    emberAfFillCommandDoorLockClusterProgrammingEventNotification(0x01, 0x00, userID, &userPin, 0x00, 0x00, 0x00, &userPin);
    emberAfSendCommandUnicastToBindings();
  }

  return true;
}

/** @brief Clear All P I Ns
 *
 *
 */
bool emberAfDoorLockClusterClearAllPinsCallback(void)
{
  uint8_t i;
  for (i = 0; i < DOOR_LOCK_USER_TABLE_SIZE; i++) {
    EmberAfDoorLockUser *user = &userTable[i];
    MEMSET(user, 0, sizeof(EmberAfDoorLockUser));
  }
  emberAfFillCommandDoorLockClusterClearAllPinsResponse(0x00);
  emberAfSendResponse();
  return true;
}

/** @brief Set Weekday Schedule
 *
 *
 * @param scheduleID   Ver.: always
 * @param userID   Ver.: always
 * @param daysMask   Ver.: always
 * @param startHour   Ver.: always
 * @param startMinute   Ver.: always
 * @param stopHour   Ver.: always
 * @param stopMinute   Ver.: always
 */
bool emberAfDoorLockClusterSetWeekdayScheduleCallback(uint8_t scheduleID,
                                                      uint16_t userID,
                                                      uint8_t daysMask,
                                                      uint8_t startHour,
                                                      uint8_t startMinute,
                                                      uint8_t stopHour,
                                                      uint8_t stopMinute)
{
  uint8_t status = 0x00;
  uint8_t userPin = 0x00;
  uint16_t rfProgrammingEventMask = 0xffff; //event sent by default
  if (!checkForSufficientSpace(scheduleID, DOOR_LOCK_SCHEDULE_TABLE_SIZE)
      || !checkForSufficientSpace(userID, DOOR_LOCK_USER_TABLE_SIZE)) {
    status = 0x01;
  }
  if (!status) {
    EmberAfDoorLockScheduleEntry *dlse = &schedule[scheduleID];
    dlse->userID = userID;
    dlse->daysMask = daysMask;
    dlse->startHour = startHour;
    dlse->startMinute = startMinute;
    dlse->stopHour = stopHour;
    dlse->stopMinute = stopMinute;
    emberAfDoorLockClusterPrintln("***RX SET WEEKDAY SCHEDULE***");
    printScheduleTable();
  }
  emberAfFillCommandDoorLockClusterSetWeekdayScheduleResponse(status);
  emberAfSendResponse();

  //get bitmask so we can check if we should send event notification
  emberAfReadServerAttribute(DOOR_LOCK_SERVER_ENDPOINT,
                             ZCL_DOOR_LOCK_CLUSTER_ID,
                             ZCL_RF_PROGRAMMING_EVENT_MASK_ATTRIBUTE_ID,
                             (uint8_t*)&rfProgrammingEventMask,
                             sizeof(rfProgrammingEventMask));

  if (rfProgrammingEventMask & BIT(0)) {
    emberAfFillCommandDoorLockClusterProgrammingEventNotification(0x01, 0x00, userID, &userPin, 0x00, 0x00, 0x00, &userPin);
    emberAfSendCommandUnicastToBindings();
  }

  return true;
}

/** @brief Get Weekday Schedule
 *
 *
 * @param scheduleID   Ver.: always
 * 2@param userID   Ver.: always
 */
bool emberAfDoorLockClusterGetWeekdayScheduleCallback(uint8_t scheduleID,
                                                      uint16_t userID)
{
  uint8_t status = 0x00;
  if (!checkForSufficientSpace(scheduleID, DOOR_LOCK_SCHEDULE_TABLE_SIZE)
      || !checkForSufficientSpace(userID, DOOR_LOCK_USER_TABLE_SIZE)) {
    status = 0x01;
  }
  if (!status) {
    EmberAfDoorLockScheduleEntry *dlse = &schedule[scheduleID];
    emberAfFillCommandDoorLockClusterGetWeekdayScheduleResponse(scheduleID,
                                                                userID,
                                                                status,
                                                                dlse->daysMask,
                                                                dlse->startHour,
                                                                dlse->startMinute,
                                                                dlse->stopHour,
                                                                dlse->stopMinute);
    emberAfSendResponse();
  }
  return true;
}
