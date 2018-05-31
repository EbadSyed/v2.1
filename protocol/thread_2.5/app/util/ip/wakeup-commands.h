// File: wakeup-commands.h
//
// Description: Low duty cycle wakeup commands for the IP stack.
//
// Copyright 2013 by Ember Corporation. All rights reserved.                *80*

#ifndef WAKEUP_COMMANDS_H
#define WAKEUP_COMMANDS_H

void listenForWakeupCommand(void);
void activeWakeupCommand(void);
void startWakeupCommand(void);
void abortWakeupCommand(void);
void configWakeupCommand(void);
void wakeupStateCommand(void);
void setWakeupSequenceCommand(void);
void setLegacyFilterParamsCommand(void);

#endif // WAKEUP_COMMANDS_H
