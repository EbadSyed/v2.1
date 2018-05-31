// Copyright 2017 Silicon Laboratories, Inc.

#ifndef __CONNECTION_MANAGER_JIB_H__
#define __CONNECTION_MANAGER_JIB_H__

#include CONFIGURATION_HEADER

/** @brief Get the fixed joining key
 *
 * This function will be called whenever the in band commissioning fixed joining
 * key is needed by the connection manager.  The key will be set by this function
 * using the joinKey parameter, and the size will be relayed by the return value.
 * A return value of 0 indicates that the key has not been set, which will cause
 * the connection manager to halt its connection attempt.
 *
 * @param joinKey A pointer to the fixed joining key
 */
uint8_t emberConnectionManagerJibGetJoinKeyCallback(uint8_t **joinKey);

#endif //__CONNECTION_MANAGER_JIB_H__
