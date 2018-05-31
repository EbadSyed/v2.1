// Copyright 2016 Silicon Laboratories, Inc.                                *80*

#ifndef SILABS_BULB_UI_H
#define SILABS_BULB_UI_H

/** @brief Function to kick off the search for joinable networks.
 *
 * This funciton will kick off the bulb-ui search for joinable networks.  This
 * is not normally required, as the bulb-ui will do this automatically.
 * However, for some plugins, that interrupt the normal bulb-ui behavior,
 * such as the manufacturing library cluster server, they may need to kick off
 * a network search when their task is complete.
 *
 */
void emberAfPluginBulbUiInitiateNetworkSearch(void);

#endif
