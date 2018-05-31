/**************************************************************************//**
 * @file coexistence.h
 * @brief This file contains the radio coexistence interface.
 * @copyright Copyright 2017 Silicon Laboratories, Inc. www.silabs.com
 *****************************************************************************/

#ifndef __COEXISTENCE_H__
#define __COEXISTENCE_H__

// Include standard type headers to help define structures
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Radio Coexistence Structures
// -----------------------------------------------------------------------------
/**
 * @addtogroup Radio Coexistence
 * @{
 */

/**
 * @typedef COEX_GpioHandle_t
 * @brief User provided reference to a GPIO.
 */
typedef const void *COEX_GpioHandle_t;

/**
 * @enum COEX_GpioOptions_t
 * @brief Configuration options for an individual GPIO.
 */
typedef enum COEX_GpioOptions {
  /** GPIO asserted by default */
  COEX_GPIO_OPTION_DEFAULT_ASSERTED = (1u << 0),

  /** Trigger interrupt on GPIO asserted */
  COEX_GPIO_OPTION_INT_ASSERTED = (1u << 1),

  /** Trigger interrupt on GPIO deasserted */
  COEX_GPIO_OPTION_INT_DEASSERTED = (1u << 2),

  /** GPIO is shared **/
  COEX_GPIO_OPTION_SHARED = (1u << 3),

  /** GPIO is an output **/
  COEX_GPIO_OPTION_OUTPUT = (1u << 4)
} COEX_GpioOptions_t;

/**
 * @struct COEX_GpioConfig_t
 * @brief Configuration structure for an individual GPIO.
 */
typedef struct COEX_GpioConfig{
  /** Function called when GPIO is toggled **/
  void (*cb)(void);

  /** GPIO configuration options **/
  COEX_GpioOptions_t options;
} COEX_GpioConfig_t;

/**
 * @enum COEX_Events_t
 * @brief COEX events bit mask.
 */
typedef enum COEX_Events {
  /** Radio coexistence has been disabled */
  COEX_EVENT_COEX_DISABLED = (1u << 0),
  /** Request coexistence has been enabled */
  COEX_EVENT_COEX_ENABLED = (1u << 1),
  /** Request GPIO has been released */
  COEX_EVENT_REQUEST_RELEASED = (1u << 2),
  /** Request GPIO has been denied */
  COEX_EVENT_REQUEST_DENIED = (1u << 3),
  /** Grant GPIO has been released */
  COEX_EVENT_GRANT_RELEASED = (1u << 4),
  /** Priority GPIO is asserted */
  COEX_EVENT_PRIORITY_ASSERTED = (1u << 5),
  /** The radio is allowed to transmit */
  COEX_EVENT_HOLDOFF_DISABLED = (1u << 6),
  /** The radio is not allowed to transmit */
  COEX_EVENT_HOLDOFF_ENABLED = (1u << 7),
  /** The last transmit was aborted due to GRANT loss */
  COEX_EVENT_TX_ABORTED = (1u << 8)
} COEX_Events_t;

/**
 * @enum COEX_Options_t
 * @brief COEX configuration options.
 */
typedef enum COEX_Options {
  /** Maximum random backoff time(0 to 15 microseconds) before asserting request */
  COEX_OPTION_MAX_REQ_BACKOFF_MASK = 0x0F,
  /** Request GPIO is shared */
  COEX_OPTION_REQ_SHARED = (1u << 4),
  /** Pulse request when RHO GPIO is released */
  COEX_OPTION_PULSE_REQ_ON_RHO_RELEASE = (1u << 5),
  /** Pulse request when GNT GPIO is released */
  COEX_OPTION_PULSE_REQ_ON_GNT_RELEASE = (1u << 6),
  /** Abort any TX in progress if GNT GPIO is released */
  COEX_OPTION_TX_ABORT = (1u << 7),
  /** Priority GPIO is shared */
  COEX_OPTION_PRI_SHARED = (1u << 8),
  /** Radio Holdoff is enabled */
  COEX_OPTION_RHO_ENABLED = (1u << 9),
  /** Coexistence is enabled */
  COEX_OPTION_COEX_ENABLED = (1u << 10)
} COEX_Options_t;

/**
 * @enum COEX_Req_t
 * @brief COEX request radio enumeration.
 */
typedef enum COEX_Req {
  /** Disable request. */
  COEX_REQ_OFF = 0U,
  /** Assert request. */
  COEX_REQ_ON = (1u << 0),
  /** Request is hi-pri. */
  COEX_REQ_HIPRI = (1u << 1),
  /** Force assertion immediately. */
  COEX_REQ_FORCE = (1u << 2),
  /** Callback when REQUEST asserted */
  COEX_REQCB_REQUESTED = (1u << 3),
  /** Callback when GRANT asserted */
  COEX_REQCB_GRANTED = (1u << 4),
  /** Callback when GRANT negated */
  COEX_REQCB_NEGATED = (1u << 5),
  /** Callback when REQUEST removed */
  COEX_REQCB_OFF = (1u << 6)
} COEX_Req_t;

/**
 * @typedef COEX_ReqCb_t
 * @brief User provided callbacks for radio coexistence (COEX)
 *        REQUEST and/or GRANT events.
 */
typedef void (*COEX_ReqCb_t)(COEX_Req_t coexStatus);

/**
 * @struct COEX_ReqState_t
 * @brief User provided callbacks for radio coexistence (COEX)
 *
 * This structure must be allocated in application global read-write memory
 * that persists for the duration of the COEX request. It cannot be allocated
 * in read-only memory or on the call stack.
 */
typedef struct COEX_ReqState {
  struct COEX_ReqState *next;
  volatile COEX_Req_t coexReq;
  COEX_ReqCb_t cb;
} COEX_ReqState_t;

/**
 * User provided random wait callback.
 *
 * @param delayMaskUs Value to mask random delay with.
 *
 * Wait a random period of time(0-0xFFFF microseconds)
 */
typedef void (*COEX_RandomDelayCallback_t)(uint16_t randomDelayMaskUs);

/**
 * User provided radio callback for radio coexistence (COEX).
 *
 * @param[in] events A bit mask of COEX events.
 *
 * See the \ref COEX_Events_t documentation for the list of COEX events.
 */
typedef void (*COEX_RadioCallback_t)(COEX_Events_t events);

/**
 * @struct COEX_HalCallbacks_t
 * @brief User provided HAL callbacks for radio coexistence (COEX)
 */
typedef struct COEX_HalCallbacks {
  /**
   * Set/clear the logical output of a GPIO.
   *
   * @param[in] gpioHandle A GPIO instance handle.
   * @param[in] enabled
   *  true - set the GPIO if it is active high, clear otherwise
   *  false - clear the GPIO if it is active high, set otherwise
   *
   */
  void (*setGpio)(COEX_GpioHandle_t gpioHandle, bool enabled);
  /**
   * Set/clear GPIO interrupt flag.
   *
   * @param[in] gpioHandle A GPIO instance handle.
   * @param[in] enabled
   *  true - set the GPIO interrupt flag
   *  false - clear the GPIO interrupt flag
   * @note The return value should be inverted if the GPIO is active low.
   */
  void (*setGpioFlag)(COEX_GpioHandle_t gpioHandle, bool enabled);
  /**
   * Enable/disable a GPIO interrupt.
   *
   * @param[in] gpioHandle A GPIO instance handle.
   * @param[in] enabled
   *  true - enable the GPIO interrupt
   *  false - disable the GPIO interrupt
   * @param[out] wasAsserted pointer to asserted flag.
   *  This flag is cleared before an interrupt is enabled.
   *  This can be used to prevent missing GPIO toggles.
   */
  void (*enableGpioInt)(COEX_GpioHandle_t gpioHandle,
                        bool enabled,
                        bool *wasAsserted);
  /**
   * Configure a GPIO.
   *
   * @param[in] gpioHandle A GPIO instance handle.
   * @param[in] config A pointer to GPIO configuration struct
   */
  void (*configGpio)(COEX_GpioHandle_t gpioHandle, COEX_GpioConfig_t *config);
  /**
   * Get the current logical output state of a GPIO.
   *
   * @param[in] gpioHandle A GPIO instance handle.
   *
   * @return The current output state of GPIO.
   *
   * @note The return value should be inverted if the GPIO is active low.
   */
  bool (*isGpioOutSet)(COEX_GpioHandle_t gpioHandle);
  /**
   * Get the current logical input state(asserted/deasserted) of a GPIO.
   *
   * @param[in] gpioHandle A GPIO instance handle.
   *
   * @return The current logical input state(asserted/deasserted) of GPIO.
   *
   * @note The return value should be inverted if the GPIO is active low.
   */
  bool (*isGpioInSet)(COEX_GpioHandle_t gpioHandle);
} COEX_HalCallbacks_t;

/**
 * Request permission to transmit from COEX master.
 *
 * @param[in] reqState Pointer to /ref COEX_ReqState_t structure.
 *                     This structure should be zero initialized before it's first use.
 * @param[in] coexReq This parameter is either ON, OFF, PRIORITY or FORCED. PRIORITY AND FORCED can be
 *                    combined with ON and OFF.
 * @param[in] cb Callback fired when REQUEST is asserted.
 *
 * @return This function returns the true if request was set, false otherwise.
 */
bool COEX_SetRequest(COEX_ReqState_t *reqState,
                     COEX_Req_t coexReq,
                     COEX_ReqCb_t cb);

/**
 * Configure the COEX request GPIO.
 *
 * @param[in] gpioHandle A GPIO instance handle.
 * @return This function returns true if the request GPIO
 *  was successfully configured, false otherwise.
 *
 * The request GPIO is asserted when the radio
 * needs to transmit.  The COEX master will assert
 * the grant GPIO if the radio can transmit.
 *
 * @note Pass NULL to disable the request GPIO.
 */
bool COEX_ConfigRequest(COEX_GpioHandle_t gpioHandle);

/**
 * Configure the COEX grant GPIO.
 *
 * @param[in] gpioHandle A GPIO instance handle.
 * @return This function returns true if the grant GPIO
 *  was successfully configure, false otherwise.
 *
 * The grant GPIO is asserted by the COEX master
 * when a request is granted.  The radio will not
 * transmit until unless the grant GPIO is asserted.
 *
 * @note Pass NULL to disable the grant GPIO.
 */
bool COEX_ConfigGrant(COEX_GpioHandle_t gpioHandle);

/**
 * Configure the COEX priority GPIO.
 *
 * @param[in] gpioHandle A GPIO instance handle.
 * @return This function returns true if the priority GPIO
 *  was successfully configure, false otherwise.
 *
 * This GPIO is asserted when a high priority
 * transmission is needed.
 *
 * @note Pass NULL to disable the priority GPIO.
 */
bool COEX_ConfigPriority(COEX_GpioHandle_t gpioHandle);

/**
 * Configure the radio hold off GPIO.
 *
 * @param[in] gpioHandle A GPIO instance handle.
 * @return This function returns true if the radio hold off GPIO
 *  was successfully configure, false otherwise.
 *
 * The radio will not transmit if the radio hold off GPIO
 * is asserted.
 *
 * @note Pass NULL to disable the radio hold off GPIO.
 */
bool COEX_ConfigRadioHoldOff(COEX_GpioHandle_t gpioHandle);

/**
 * Set the COEX configuration options
 *
 * @param[in] options New COEX configuration options.
 */
bool COEX_SetOptions(COEX_Options_t options);

/**
 * Get the current COEX configuration options
 *
 * @return This function returns current COEX configuration
 *         set by \ref COEX_SetOptions.
 */
COEX_Options_t COEX_GetOptions(void);

/**
 * Check if COEX is enabled.
 *
 * @return True if COEX is enabled, false otherwise.
 */
bool COEX_IsEnabled(void);

/**
 * Update COEX grant state.
 *
 * This function should be called to reassess the COEX grant GPIO
 * before transmitting.
 * This is needed if the COEX grant GPIO is preempted by radio interrupts.
 */
void COEX_UpdateGrant(void);

/**
 * Set COEX HAL callback function pointers.
 *
 * @param[in] callbacks
 *   Pointer to struct of HAL callback function pointers.
 *
 * @note This function does not create a local copy of callbacks.
 *   Callbacks should be allocated in persistent memory and not the call stack.
 */
void COEX_SetHalCallbacks(const COEX_HalCallbacks_t * callbacks);

/**
 * Set COEX radio callback function pointers.
 *
 * @param[in] callbacks
 *   Pointer to struct of radio callback function pointers.
 *
 * @note This function does not create a local copy of callbacks.
 *   Callbacks should be allocated in persistent memory and not the call stack.
 */
void COEX_SetRadioCallback(COEX_RadioCallback_t callback);

/**
 * Set COEX random delay callback function pointers.
 *
 * @param[in] callbacks
 *   Pointer to struct of radio callback function pointers.
 *
 * @note This function does not create a local copy of callbacks.
 *   Callbacks should be allocated in persistent memory and not the call stack.
 */
void COEX_SetRandomDelayCallback(COEX_RandomDelayCallback_t callback);

/**
 * Notify COEX of radio power state.
 *
 * @param[in] powerUp radio is powered up if true; radio is powered down if false.
 */
void setCoexPowerState(bool powerUp);
/**
 * @}
 * end of COEX_API
 */

#endif  // __COEXISTENCE_H__
