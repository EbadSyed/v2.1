// *******************************************************************
// * mfg-lib-plugin.h
// *
// *
// * Copyright 2015 by Silicon Laboratories. All rights reserved.           *80*
// *******************************************************************

#ifndef SILABS_MANUFACTURING_LIBRARY_CLI_PLUGIN_H
#define SILABS_MANUFACTURING_LIBRARY_CLI_PLUGIN_H

/** @brief Returns a true if the manufacturing library is currently running.
 *
 * Function to return whether the manufacturing library is currently running.
 * Code that initiates scan/join behavoir should not do so if the manufacturing
 * library is currently running as this will cause a conflict and may result
 * in a fatal error.
 *
 * @return A ::bool value that is true if the manufacturing library is
 * running, and false if it is not.
 */
bool emberAfMfglibRunning(void);

/** @brief Returns a true if the manufacturing library token has been set.
 *
 * Function to return whether the manufacturing library token has currently
 * been set.  Reference designs are programmed to initiate off scan/join
 * behavoir as soon as the device has been powered up.  Certain sleepy devices,
 * such as security sensors, may also use the UART for manufacturing, which
 * becomes inactive during normal operation.  Setting this token will allow
 * the device to stay awake or hold off on normal joining behavior for a few
 * seconds to allow manufacturing mode to be enabled.  The last step in the
 * manufacturing process would be to disable this token.
 *
 * Note:  this token is disabled by default.  If you wish to enable it by
 * default in your application, you must edit the file
 * app/framework/plugin/mfg-lib/mfg-lib-tokens.h.
 *
 * @return A ::bool value that is true if the manufacturing library token
 * has been set and false if it has not been set.
 */
bool emberAfMfglibEnabled(void);

#endif
