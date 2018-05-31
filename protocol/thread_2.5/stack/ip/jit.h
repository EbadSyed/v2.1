// File: jit.h
//
// Description: Manage "just-in-time" (JIT) messages sent to our sleepy end
// device children.
//
// Copyright 2014 by Silicon Laboratories. All rights reserved.             *80*

#ifndef JIT_H
#define JIT_H

// Hold header and send it to all sleepy children that poll.
void emSetSleepyBroadcast(PacketHeader header);

// Returns a stack JIT message (sleepy broadcast or "alarm") for immediate
// transmission.
PacketHeader emMakeStackJitMessage(void);

// Check whether header is a JIT message that we submitted.
bool emJitTransmitComplete(PacketHeader header, EmberStatus status);

extern Buffer emSleepyBroadcastHeader;

#endif // JIT_H
