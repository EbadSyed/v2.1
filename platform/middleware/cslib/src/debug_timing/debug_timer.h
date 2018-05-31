/**************************************************************************//**
 * Copyright 2016 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#ifndef __DEBUG_TIMER_H__
#define __DEBUG_TIMER_H__
void debugTimer_setup(void);
void debugTimer_start();
uint32_t debugTimer_getTime();
void debugTimer_stop();

#endif // __DEBUG_TIMER_H__
