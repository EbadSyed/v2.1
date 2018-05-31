/**
 * @file qa-thread-header.h
 * @brief  APIs for internal Thread testing only.
 *
 * This file contains API functions that are available on both host
 * and SOC platforms.
 *
 * Callback naming conventions:
 *   ...Handler()       // unilateral event, e.g. a beacon arriving
 *   ...Return()        // result of a call
 */

#ifndef THREAD_DEBUG_H
#define THREAD_DEBUG_H

/** @brief
 *  Get stored security key info.  If keyInUse is true, it returns the
 *  information of the current key, otherwise returns the other queued key
 *  (if it exists) in the cache.
 */
void emberGetNetworkKeyInfo(bool keyInUse);
void emberGetNetworkKeyInfoReturn(EmberStatus status,
                                  uint32_t sequence,
                                  uint8_t state);

/** @brief
 * Test command.  Retrieve the node's:
 * . rip id
 * . short id
 * . parent's rip id
 * . parent's short id
 * . network fragment identifier
 * . network frame counter
 */
void emberGetNodeStatus(void);
void emberGetNodeStatusReturn(EmberStatus status,
                              uint8_t ripId,
                              EmberNodeId nodeId,
                              uint8_t parentRipId,
                              EmberNodeId parentId,
                              const uint8_t *networkFragmentIdentifier,
                              uint32_t networkFrameCounter);

/** @brief
 * Test command. Set the drop and corruption rate for the UART.
 * Set the drop and corruption rate for the UART (test command)
 */
void emberConfigUart(uint8_t dropPercentage, uint8_t corruptPercentage);
void emberConfigUartReturn(void);

/** @brief
 * Test command. Reset the NCP's ASHv3 link.
 */
void emberResetNcpAsh(void);
void emberResetNcpAshReturn(void);

/** @brief
 * Test command. Reset the IP driver's ASHv3 link.
 */
void emberResetIpDriverAsh(void);

/** @brief
 * Test command. Reset the NCP using a host GPIO output.
 */
void emberResetNcpGpio(void);

/** @brief
 * Test command. Enable/disable NCP resets via host GPIO output.
 */
void emberEnableResetNcpGpio(uint8_t enable);

/** @brief
 * Test command. Start a data storm.
 */
void emberStartUartStorm(uint16_t rate);
void emberStartUartStormReturn(void);

/** @brief
 * Test command. Stop a data storm.
 */
void emberStopUartStorm(void);
void emberStopUartStormReturn(void);

/** @brief
 * Test command. Force an assert.
 */
void emberForceAssert(void);

/** @brief
 * Test command. Tell the NCP to respond with a done callback after
 * a specific number of milliseconds.
 */
void emberSendDone(uint32_t timeoutMs);
void emberSendDoneReturn(void);

/** @brief
 * Test command.  Add a manual address cache entry.
 */
void emberAddAddressData(const uint8_t *longId, uint16_t shortId);
void emberAddAddressDataReturn(uint16_t shortId);

/** @brief
 * Test command.  Clear the address cache.
 */
void emberClearAddressCache(void);
void emberClearAddressCacheReturn(void);

/** @brief
 * Test command.  Lookup shortId for a given eui64.
 */
void emberLookupAddressData(const uint8_t *longId);
void emberLookupAddressDataReturn(uint16_t shortId);

/** @brief
 *  (Internal) Get the multicast table.
 */
void emberGetMulticastTable(void);
void emberGetMulticastEntryReturn(uint8_t lastSequence,
                                  uint8_t windowBitmask,
                                  uint8_t dwellQs,
                                  const uint8_t *seed);

/** @brief
 * Test Command. Start a UART speed test.
 */
void emberStartUartSpeedTest(uint8_t payloadSize,
                             uint32_t timeout,
                             uint32_t intervalMs);
void emberUartSpeedTestReturn(uint32_t totalBytesSent,
                              uint32_t payloadBytesSent,
                              uint32_t timeout);

void emberSetWakeupSequenceNumber(uint16_t sequenceNumber);
void emberSetWakeupSequenceNumberReturn(void);

void emberNcpUdpStorm(uint8_t totalPackets,
                      const uint8_t *dest,
                      uint16_t payloadLength,
                      uint16_t txDelayMs);
void emberNcpUdpStormReturn(EmberStatus status);
void emberNcpUdpStormCompleteHandler(void);

/** @brief
 * Start XOn/XOff test.
 */
void emberStartXOnXOffTest(void);

/** @brief
 * Test Command. Enable or disable mac extended id randomization.
 */
void emberSetRandomizeMacExtendedId(bool value);
void emberSetRandomizeMacExtendedIdReturn(void);

#endif // THREAD_DEBUG_H
