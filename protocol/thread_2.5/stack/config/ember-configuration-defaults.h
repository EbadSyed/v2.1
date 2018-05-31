/** @file ember-configuration-defaults.h
 * @brief User-configurable stack memory allocation defaults
 *
 * @note Application developers should \b not modify any portion
 * of this file. Doing so may cause mysterious bugs. Allocations should be
 * adjusted only by defining the appropriate macros in the application's
 * CONFIGURATION_HEADER.
 *
 * See @ref configuration for documentation.
 * <!-- Author(s): Lee Taylor, lee@ember.com -->
 * <!--Copyright 2005 by Ember Corporation. All rights reserved.         *80*-->
 */

//  Todo:
//  - explain how to use a configuration header
//  - the documentation of the custom handlers should
//    go in hal/ember-configuration.c, not here
//  - the stack profile documentation is out of date

/**
 * @addtogroup utilities
 *
 * All configurations have defaults, therefore many applications may not need
 * to do anything special.  However, you can override these defaults
 * by creating a CONFIGURATION_HEADER and within this header,
 * defining the appropriate macro to a different size.  For example, to
 * increase the child table size from 16 (the default) to 32:
 * @code
 * #define EMBER_CHILD_TABLE_SIZE 32
 * @endcode
 *
 * The convenience stubs provided in @c hal/ember-configuration.c can be overridden
 * by  defining the appropriate macro and providing the corresponding callback
 * function.  For example, an application with custom debug channel input must
 * implement @c emberDebugHandler() to process it.  Along with
 * the function definition, the application should provide the following
 * line in its CONFIGURATION_HEADER:
 * @code
 * #define EMBER_APPLICATION_HAS_DEBUG_HANDLER
 * @endcode
 *
 * See ember-configuration-defaults.h for source code.

 * @{
 */

#ifndef __EMBER_CONFIGURATION_DEFAULTS_H__
#define __EMBER_CONFIGURATION_DEFAULTS_H__

/** @brief If the application defined a configuration file, include it.
 */
#ifdef CONFIGURATION_HEADER
  #include CONFIGURATION_HEADER
#endif

/** @brief The default version name for an application.
 */
#ifndef EMBER_VERSION_NAME
  #define EMBER_VERSION_NAME "Thread"
#endif

/** @brief The minimum heap size allocated for an application.
 */
#ifndef EMBER_HEAP_SIZE
  #define EMBER_HEAP_SIZE 6000
#endif

/** @brief The default amount of heap allocated for the mbedtls malloc
 * library, if in use.
 */
#ifdef PHY_EM3XX
  #define EMBER_MALLOC_HEAP_SIZE_BYTES 16384
#else
  #define EMBER_MALLOC_HEAP_SIZE_BYTES 32768
#endif

/** @brief Settings to control if and where assert information will
 * be printed.
 *
 * The output can be suppressed by defining @c EMBER_ASSERT_OUTPUT_DISABLED.
 * The serial port to which the output is sent
 * can be changed by defining ::EMBER_ASSERT_SERIAL_PORT as the desired port.
 *
 * The default is to have assert output on and sent to serial port 1.
 */
#if !defined(EMBER_ASSERT_OUTPUT_DISABLED) \
  && !defined(EMBER_ASSERT_SERIAL_PORT)
  #define EMBER_ASSERT_SERIAL_PORT 1
#endif

/** @brief The maximum amount of time (in quarter seconds) that the MAC
 * will hold a message for indirect transmission to a child.
 *
 * The default is 30 quarter seconds (7.5 seconds).
 * The maximum value is 30000 quarter seconds (125 minutes).
 * Larger values will cause rollover confusion.
 */
#ifndef EMBER_INDIRECT_TRANSMISSION_TIMEOUT
  #define EMBER_INDIRECT_TRANSMISSION_TIMEOUT 30
#endif

/** @brief The size of the child table.  This include sleepy and powered
 * end device children, as well as router eligible end devices.
 *
 * Note:  We do not support greater than 32 children, so the maximum value
 * for this configuration setting is 32.
 */
#ifndef EMBER_CHILD_TABLE_SIZE
  #define EMBER_CHILD_TABLE_SIZE 16
#endif

#ifndef EMBER_RETRY_QUEUE_SIZE
  #define EMBER_RETRY_QUEUE_SIZE 8
#endif

/** @brief The security level used for security at the MAC and network
 * layers.  The supported values are 0 (no security) and 5 (payload is
 * encrypted and a four-byte MIC is used for authentication).
 */
#ifndef EMBER_SECURITY_LEVEL
  #define EMBER_SECURITY_LEVEL 5
#endif

#ifndef EMBER_SECURITY_TO_HOST
  #ifdef EMBER_APPLICATION_HAS_SECURITY_TO_HOST
    #define EMBER_SECURITY_TO_HOST true
  #else
    #define EMBER_SECURITY_TO_HOST false
  #endif
#endif

/** @brief The number of event tasks that can be tracked for the purpose of
 *  processor idling.  The Thread stack requires 1, an application and
 *  associated libraries may use additional tasks, though typically no more
 *  than 3 are needed for most applications.
 */
#ifndef EMBER_TASK_COUNT
 #define EMBER_TASK_COUNT (3)
#endif

/**
 * @brief The number of seconds after which the parent will time an
 * ::EMBER_SLEEPY_END_DEVICE out of its table if it has not heard a data
 * poll from it.
 *
 * The default is 240 seconds.  The maximum value is 2^32 - 1 (136 years).
 *
 * This value is determined by the child and communicated to the parent
 * via the MLE protocol.
 */
#ifndef EMBER_SLEEPY_CHILD_POLL_TIMEOUT
  #define EMBER_SLEEPY_CHILD_POLL_TIMEOUT 240  // 4 minutes
#endif

/** @brief The maximum amount of time that an ::EMBER_END_DEVICE can
 * wait between polls.
 *
 * The default is 240 seconds.
 *
 * If no poll is heard within this time, then the parent removes the
 * ::EMBER_END_DEVICE from its tables.
 */
#ifndef EMBER_END_DEVICE_POLL_TIMEOUT
  #define EMBER_END_DEVICE_POLL_TIMEOUT 240
#endif

/** @brief The number of packets received by an NCP before it decides
 * to send aggregated packet information to the host when running an
 * mfg send test.
 *
 * The default value is 50 packets.
 */
#ifndef EMBER_MFG_RX_NCP_TO_HOST_INTERVAL
  #define EMBER_MFG_RX_NCP_TO_HOST_INTERVAL 50
#endif

#ifndef EMBER_USE_DIRECT_IP_CALLBACK
  #define EMBER_USE_DIRECT_IP_CALLBACK false
#endif

#ifdef EMBER_WAKEUP_STACK
  #define RIP_MAX_LURKERS 32
#else
  #define RIP_MAX_LURKERS 0
#endif

/** @} END addtogroup */

#endif //__EMBER_CONFIGURATION_DEFAULTS_H__
