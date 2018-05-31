// Copyright 2015 Silicon Laboratories, Inc.

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_BUFFER_MANAGEMENT
#include EMBER_AF_API_EVENT_QUEUE
#include EMBER_AF_API_HAL
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE

#define MIN_LEVEL EMBER_AF_PLUGIN_LEVEL_CONTROL_SERVER_MINIMUM_LEVEL
#define MAX_LEVEL EMBER_AF_PLUGIN_LEVEL_CONTROL_SERVER_MAXIMUM_LEVEL

#define BOUND_MIN(value)     ((value) < MIN_LEVEL ? MIN_LEVEL : (value))
#define BOUND_MAX(value)     ((value) < MAX_LEVEL ? (value) : MAX_LEVEL)
#define BOUND_MIN_MAX(value) BOUND_MIN(BOUND_MAX(value))

#define MIN_DELAY_MS 1

int abs(int I);
static void moveToLevelHandler(const EmberZclCommandContext_t *context,
                               const EmberZclClusterLevelControlServerCommandMoveToLevelRequest_t *request,
                               bool withOnOff);
static void moveHandler(const EmberZclCommandContext_t *context,
                        const EmberZclClusterLevelControlServerCommandMoveRequest_t *request,
                        bool withOnOff);
static void stepHandler(const EmberZclCommandContext_t *context,
                        const EmberZclClusterLevelControlServerCommandStepRequest_t *request,
                        bool withOnOff);
static void stopHandler(const EmberZclCommandContext_t *context,
                        const EmberZclClusterLevelControlServerCommandStopRequest_t *request,
                        bool withOnOff);
static bool getOnOff(EmberZclEndpointId_t endpointId);
static void setOnOff(EmberZclEndpointId_t endpointId, bool onOff);
static uint8_t getCurrentLevel(EmberZclEndpointId_t endpointId);
static void setCurrentLevel(EmberZclEndpointId_t endpointId,
                            uint8_t currentLevel);
static uint8_t getOnLevelOrCurrentLevel(EmberZclEndpointId_t endpointId);

typedef struct {
  uint32_t delayMs;
  EmberZclEndpointId_t endpointId;
  uint8_t targetLevel;
  uint8_t postTransitionLevel;
  bool increasing;
  bool withOnOff;
} State;

typedef struct {
  Event event;
  State state;
} LevelControlEvent;

extern EventQueue emAppEventQueue;
static void eventHandler(LevelControlEvent *event);
static void eventMarker(LevelControlEvent *event);
static EventActions actions = {
  &emAppEventQueue,
  (void (*)(struct Event_s *))eventHandler,
  (void (*)(struct Event_s *))eventMarker,
  "level control server"
};

static LevelControlEvent *cancel(State *state);
static EmberZclStatus_t schedule(State *state);

void emberZclLevelControlServerSetOnOff(EmberZclEndpointId_t endpointId,
                                        bool value)
{
  State state = {
    .endpointId = endpointId,
    .increasing = value,
    .withOnOff = true,
  };
  if (value) {
    // Light is being turned on
    state.targetLevel = getOnLevelOrCurrentLevel(endpointId);
    state.postTransitionLevel = state.targetLevel;
    if (getOnOff(endpointId) == 0) {
      // Only smoothly transition from off to on if the light is off
      setCurrentLevel(endpointId, MIN_LEVEL);
      setOnOff(endpointId, (state.targetLevel != MIN_LEVEL));
    }
  } else {
    // Light is being turned off
    state.targetLevel = MIN_LEVEL;
    state.postTransitionLevel = getOnLevelOrCurrentLevel(endpointId);
  }

  if (getCurrentLevel(endpointId) == state.targetLevel) {
    cancel(&state);
  } else {
    state.delayMs = MIN_DELAY_MS; // TODO: Support OnOffTransitionTime.
    schedule(&state);
  }
}

void emberZclClusterLevelControlServerCommandMoveToLevelRequestHandler(const EmberZclCommandContext_t *context,
                                                                       const EmberZclClusterLevelControlServerCommandMoveToLevelRequest_t *request)
{
  moveToLevelHandler(context, request, false); // without on/off
}

void emberZclClusterLevelControlServerCommandMoveRequestHandler(const EmberZclCommandContext_t *context,
                                                                const EmberZclClusterLevelControlServerCommandMoveRequest_t *request)
{
  moveHandler(context, request, false); // without on/off
}

void emberZclClusterLevelControlServerCommandStepRequestHandler(const EmberZclCommandContext_t *context,
                                                                const EmberZclClusterLevelControlServerCommandStepRequest_t *request)
{
  stepHandler(context, request, false); // without on/off
}

void emberZclClusterLevelControlServerCommandStopRequestHandler(const EmberZclCommandContext_t *context,
                                                                const EmberZclClusterLevelControlServerCommandStopRequest_t *request)
{
  stopHandler(context, request, false); // without on/off
}

void emberZclClusterLevelControlServerCommandMoveToLevelWithOnOffRequestHandler(const EmberZclCommandContext_t *context,
                                                                                const EmberZclClusterLevelControlServerCommandMoveToLevelWithOnOffRequest_t *request)
{
  moveToLevelHandler(context,
                     (const EmberZclClusterLevelControlServerCommandMoveToLevelRequest_t *)request,
                     true); // with on/off
}

void emberZclClusterLevelControlServerCommandMoveWithOnOffRequestHandler(const EmberZclCommandContext_t *context,
                                                                         const EmberZclClusterLevelControlServerCommandMoveWithOnOffRequest_t *request)
{
  moveHandler(context,
              (const EmberZclClusterLevelControlServerCommandMoveRequest_t *)request,
              true); // with on/off
}

void emberZclClusterLevelControlServerCommandStepWithOnOffRequestHandler(const EmberZclCommandContext_t *context,
                                                                         const EmberZclClusterLevelControlServerCommandStepWithOnOffRequest_t *request)
{
  stepHandler(context,
              (const EmberZclClusterLevelControlServerCommandStepRequest_t *)request,
              true); // with on/off
}

void emberZclClusterLevelControlServerCommandStopWithOnOffRequestHandler(const EmberZclCommandContext_t *context,
                                                                         const EmberZclClusterLevelControlServerCommandStopWithOnOffRequest_t *request)
{
  stopHandler(context,
              (const EmberZclClusterLevelControlServerCommandStopRequest_t *)request,
              true); // with on/off
}

static void moveToLevelHandler(const EmberZclCommandContext_t *context,
                               const EmberZclClusterLevelControlServerCommandMoveToLevelRequest_t *request,
                               bool withOnOff)
{
  emberAfCorePrintln("RX: MoveToLevel%s", (withOnOff ? "WithOnOff" : ""));

  // TODO: New ZCL (07-5123-05, section 3.10.2.2) requirement that breaks
  // existing HA1.2 (05-3520-29, section 7.1.3).
  //if (!withOnOff && !getOnOff(context->endpointId)) {
  //  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  //  return;
  //}

  uint8_t currentLevel = getCurrentLevel(context->endpointId);
  uint8_t level = BOUND_MIN_MAX(request->level);
  State state = {
    .endpointId = context->endpointId,
    .targetLevel = level,
    .postTransitionLevel = level,
    .increasing = (currentLevel <= level),
    .withOnOff = withOnOff,
  };

  if (state.increasing && state.withOnOff) {
    setOnOff(context->endpointId, (state.targetLevel != MIN_LEVEL));
  }

  EmberZclStatus_t status;
  if (currentLevel == state.targetLevel) {
    cancel(&state);
    status = EMBER_ZCL_STATUS_SUCCESS;
  } else {
    uint8_t stepSize = abs(currentLevel - request->level);
    state.delayMs = (request->transitionTime == 0xFFFF
                     ? MIN_DELAY_MS // TODO: Support OnOffTransitionTime.
                     : (request->transitionTime
                        * MILLISECOND_TICKS_PER_DECISECOND
                        / stepSize));
    status = schedule(&state);
  }
  emberZclSendDefaultResponse(context, status);
}

static void moveHandler(const EmberZclCommandContext_t *context,
                        const EmberZclClusterLevelControlServerCommandMoveRequest_t *request,
                        bool withOnOff)
{
  emberAfCorePrintln("RX: Move%s", (withOnOff ? "WithOnOff" : ""));

  uint8_t level;
  switch (request->moveMode) {
    case 0: // up
      level = MAX_LEVEL;
      break;
    case 1: // down
      level = MIN_LEVEL;
      break;
    default:
      emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_INVALID_FIELD);
      return;
  }

  // TODO: New ZCL (07-5123-05, section 3.10.2.2) requirement that breaks
  // existing HA1.2 (05-3520-29, section 7.1.3).
  //if (!withOnOff && !getOnOff(context->endpointId)) {
  //  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
  //  return;
  //}

  uint8_t currentLevel = getCurrentLevel(context->endpointId);
  State state = {
    .endpointId = context->endpointId,
    .targetLevel = level,
    .postTransitionLevel = level,
    .increasing = (currentLevel <= level),
    .withOnOff = withOnOff,
  };

  if (state.increasing && state.withOnOff) {
    setOnOff(context->endpointId, (state.targetLevel != MIN_LEVEL));
  }

  EmberZclStatus_t status;
  if (currentLevel == state.targetLevel) {
    cancel(&state);
    status = EMBER_ZCL_STATUS_SUCCESS;
  } else {
    state.delayMs = (request->rate == 0xFF
                     ? MIN_DELAY_MS // TODO: Support DefaultMoveRate.
                     : MILLISECOND_TICKS_PER_SECOND / request->rate);
    status = schedule(&state);
  }
  emberZclSendDefaultResponse(context, status);
}

static void stepHandler(const EmberZclCommandContext_t *context,
                        const EmberZclClusterLevelControlServerCommandStepRequest_t *request,
                        bool withOnOff)
{
  emberAfCorePrintln("RX: Step%s", (withOnOff ? "WithOnOff" : ""));

  uint8_t currentLevel = getCurrentLevel(context->endpointId);
  uint8_t level;
  switch (request->stepMode) {
    case 0: // up
      level = BOUND_MAX(currentLevel + request->stepSize);
      break;
    case 1: // down
      level = BOUND_MIN(currentLevel - request->stepSize);
      break;
    default:
      emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_INVALID_FIELD);
      return;
  }

  // TODO: New ZCL (07-5123-05, section 3.10.2.2) requirement that breaks
  // existing HA1.2 (05-3520-29, section 7.1.3).
  //if (!withOnOff && !getOnOff(context->endpointId)) {
  //  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_INVALID_FIELD);
  //  return;
  //}

  State state = {
    .endpointId = context->endpointId,
    .targetLevel = level,
    .postTransitionLevel = level,
    .increasing = (currentLevel <= level),
    .withOnOff = withOnOff,
  };

  if (state.increasing && state.withOnOff) {
    setOnOff(context->endpointId, (state.targetLevel != MIN_LEVEL));
  }

  EmberZclStatus_t status;
  if (currentLevel == state.targetLevel) {
    cancel(&state);
    status = EMBER_ZCL_STATUS_SUCCESS;
  } else {
    state.delayMs = (request->transitionTime == 0xFFFF
                     ? MIN_DELAY_MS
                     : (request->transitionTime
                        * MILLISECOND_TICKS_PER_DECISECOND
                        / request->stepSize));
    status = schedule(&state);
  }
  emberZclSendDefaultResponse(context, status);
}

static void stopHandler(const EmberZclCommandContext_t *context,
                        const EmberZclClusterLevelControlServerCommandStopRequest_t *request,
                        bool withOnOff)
{
  emberAfCorePrintln("RX: Stop%s", (withOnOff ? "WithOnOff" : ""));

  // TODO: New ZCL (07-5123-05, section 3.10.2.2) requirement that breaks
  // existing HA1.2 (05-3520-29, section 7.1.3).
  //if (withOnOff || getOnOff(context->endpointId)) {
  State state = {
    .endpointId = context->endpointId,
  };
  cancel(&state);
  //}
  emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_SUCCESS);
}

static bool getOnOff(EmberZclEndpointId_t endpointId)
{
  bool onOff;
  if (emberZclReadAttribute(endpointId,
                            &emberZclClusterOnOffServerSpec,
                            EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                            &onOff,
                            sizeof(onOff))
      == EMBER_ZCL_STATUS_SUCCESS) {
    // Returns true if the light is on, false if the light is off
    return onOff;
  } else {
    // If emberZclReadAttribute fails to return a success status,
    // false is returned as a default to represent the off state
    return false;
  }
}

static void setOnOff(EmberZclEndpointId_t endpointId, bool onOff)
{
  emberZclWriteAttribute(endpointId,
                         &emberZclClusterOnOffServerSpec,
                         EMBER_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF,
                         &onOff,
                         sizeof(onOff));
}

static uint8_t getCurrentLevel(EmberZclEndpointId_t endpointId)
{
  uint8_t currentLevel;
  if (emberZclReadAttribute(endpointId,
                            &emberZclClusterLevelControlServerSpec,
                            EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL,
                            &currentLevel,
                            sizeof(currentLevel))
      == EMBER_ZCL_STATUS_SUCCESS) {
    return currentLevel;
  } else {
    return 0;
  }
}

static void setCurrentLevel(EmberZclEndpointId_t endpointId,
                            uint8_t currentLevel)
{
  emberZclWriteAttribute(endpointId,
                         &emberZclClusterLevelControlServerSpec,
                         EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_CURRENT_LEVEL,
                         &currentLevel,
                         sizeof(currentLevel));
}

static uint8_t getOnLevelOrCurrentLevel(EmberZclEndpointId_t endpointId)
{
  uint8_t onLevel;
  if ((emberZclReadAttribute(endpointId,
                             &emberZclClusterLevelControlServerSpec,
                             EMBER_ZCL_CLUSTER_LEVEL_CONTROL_SERVER_ATTRIBUTE_ON_LEVEL,
                             &onLevel,
                             sizeof(onLevel))
       == EMBER_ZCL_STATUS_SUCCESS)
      && onLevel != 0xFF) {
    return onLevel;
  } else {
    return getCurrentLevel(endpointId);
  }
}

static void eventHandler(LevelControlEvent *event)
{
  uint8_t currentLevel = (getCurrentLevel(event->state.endpointId)
                          + (event->state.increasing ? +1 : -1));
  setCurrentLevel(event->state.endpointId, currentLevel);
  if (currentLevel == event->state.targetLevel) {
    if (event->state.withOnOff) {
      setOnOff(event->state.endpointId, (currentLevel != MIN_LEVEL));
    }
    if (currentLevel != event->state.postTransitionLevel) {
      setCurrentLevel(event->state.endpointId,
                      event->state.postTransitionLevel);
    }
  } else {
    emberEventSetDelayMs((Event *)event, event->state.delayMs);
  }
}

static void eventMarker(LevelControlEvent *event)
{
}

static bool predicate(LevelControlEvent *event,
                      EmberZclEndpointId_t *endpointId)
{
  return (*endpointId == event->state.endpointId);
}

static LevelControlEvent *cancel(State *state)
{
  return (LevelControlEvent *)emberFindEvent(actions.queue,
                                             &actions,
                                             (EventPredicate)predicate,
                                             &state->endpointId);
}

static EmberZclStatus_t schedule(State *state)
{
  LevelControlEvent *event = cancel(state);
  if (event == NULL) {
    Buffer buffer = emAllocateBuffer(sizeof(LevelControlEvent));
    if (buffer == NULL_BUFFER) {
      return EMBER_ZCL_STATUS_FAILURE;
    }
    event = (LevelControlEvent *)emGetBufferPointer(buffer);
  }
  event->event.actions = &actions;
  event->event.next = NULL;
  event->state = *state;
  emberEventSetDelayMs((Event *)event, event->state.delayMs);
  return EMBER_ZCL_STATUS_SUCCESS;
}
