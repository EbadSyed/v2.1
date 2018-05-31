/*
 * File: rip.h
 * Description: Routing Information Protocol, RFC 1058
 * http://tools.ietf.org/html/rfc1058
 * Author(s): Matteo Paris, matteo@ember.com
 *
 * Copyright 2012 by Ember Corporation. All rights reserved.                *80*
 */

#ifndef RIP_H
#define RIP_H

/*
   Notes

   This discussion assumes you've read http://tools.ietf.org/html/rfc1058.

   We assume the RIP network has at most 32 routers.  The address assignment,
   over-the-air message format, and internal implementation details all depend
   on this assumption.

   A RIP router has an id between 0 and 63, inclusive.  Its two byte short
   mac id is formed by shifting its RIP id 10 bytes to the left.

   The maximum RIP metric value is 16.

   We use a value of 0 (OTA and internally) to mean infinity (no route),
   rather than setting infinity to one more than the max metric value as is
   customary.  We do this to save bits over the air (0 is normally unused).

   A major design goal is to conserve bandwidth.  RIP update messages and
   link quality information are combined in a single message, currently sent
   as an MLE advertisement with a new TLV type.  We want to keep these
   updates short, but certainly no larger than one 15.4 packet.  An over
   the air entry for each destination consists of a single byte: two bits
   for outgoing quality, two bits for incoming quality, and 4 bits for the RIP
   metric.

   An incoming or outgoing quality of 0 means no link, either because the nodes
   cannot hear each other, or because they can but only too poorly to use.
   The other values for link cost correspond to poor (1), medium (2), and
   good (3) quality links.  As usual the outgoing link quality is obtained from
   the neighbor's incoming link quality and echoed back.  (Note: we are not
   using the OTA outgoing cost, so if we don't come up with a use for it
   we should probably remove it.)

   Currently we advertise all 32 route entries over the air at all times, so
   there is no need for garbage collection of infinite metric routes, or
   for a route change flag.

   Incoming link quality for a neighbor is derived from RSSI measurements on
   packets received from that neighbor.  Currently we assume a static noise
   floor of -95 dBm (this could be a problem), and use this to convert to
   a nonnegative link margin, which we store internally as a one byte rolling
   average.  Based on empirical data from the EM357, link margin is mapped
   to incoming link cost as follows:

    quality 3: >20 dBm
    quality 2: >10 dBm
    quality 1: >2  dBm
    quality 0: <= 2 dBm

   The total link cost (generally referred to simply as link cost) for a
   neighbor is derived from the incoming and outgoing link quality, with an
   added penalty for elderly links described below. See the linkCost[][]
   table in rip-neighbor.c for the mapping.  The total link cost is added
   to a neighbor's route cost to determine our route cost.

   An age timer is incremented every 30 seconds for neighbors that have been
   heard from.  It is reset to 0 upon receipt of an update from the neighbor.
   Incoming link cost is penalized in steps as neighbors get older.
   At RIP_MAX_AGE, the link quality fields are set to 0, indicating the node
   is no longer a neighbor.

   Todo

   Should we reset the age upon receipt of any packet from the neighbor?
   (Probably not... then the age would no longer truly serve as an age for
   the route, because a link's age could keep getting reset even though we
   hadn't actually heard an update for a long time, and that update might
   have indicated that the neighbor's route to a destination had gotten
   worse or gone away, but we wouldn't know.)

   Currently we store our metric to each destination, which is obtained by
   summing the next hop's metric to the destination with our link cost to the
   next hop.  Instead, we could simply store the next hop's metric to the
   destination, and perform the sum ourselves when needed (essentially only
   when we send out an update).  This would make it so that changes in link
   cost would be immediately reflected in all the metric values that used
   that link as a next hop, rather than waiting for the next update across that
   link (which may never come if the link is aging out).  This would speed up
   switching to a better route without any additional overhead.

   I'm considering keeping a backup next hop.  Currently if our next
   hop to a destination fails, we have to wait for an update from another
   neighbor to fix the route.  A backup would allow us to immediately fail over.
   We'd have to store 5 bits for the backup next hop, and 4 bits for the metric
   via that next hop, so about 36 bytes of RAM.  It would also add a little flash
   and some complexity.  In order to minimize loops, our metric via the backup
   cannot exceed our metric via the primary.  We would continue to advertise
   only the primary metric in updates.  Empirical study should help determine
   whether this feature is worth implementing.

   To help avoid loops, the RIP RFC uses split horizon with poison reverse,
   in which the update you send to a given neighbor advertises an infinite cost
   to any destination for which that neighbor is your next hop.  We can't do
   it that way because we broadcast a single update to all of our neighbors.
   However we need to do something since the situation of two nodes temporarily
   choosing each other as next hops occurs easily, which I have observerd.
   One option is to include our next hop to each destination in each update
   message.  This would increase the update payload from 32 bytes to 52 or 64
   bytes, depending on whether we respect byte boundaries.  Ouch.
   The reactive approach is to detect and fix loops when they happen.
   It saves protocol overhead at the cost of data reliability. But with a backup
   next hop, the reliability hit would be mitigated.  There is a latency hit
   on the data packet that detects the loop, but it is small and rare, nothing
   to be worried about.

   I think I need to be more careful about initializing the next hop to a
   special value when it is not valid (essentially, when the metric is 0).
   Currently it is initialized to 0, and after that it is not cleaned up
   when routes go away.  This shouldn't matter because it is not used for
   anything if the metric is 0, but it is a potential cause for confusion.

   We probably want a minimum link margin below which the link gets an
   incoming cost of zero, and below which we do not initiate an MLE
   handshake.  There is an annoying phenomenon where MLE handshakes are
   initiated on a bad link, and fail.  We might need to keep a separate
   timer which throttles back the MLE requests, and for neighbors that
   have recently failed, only allows a new request to go out if the link
   margin has improved substantially.

   Loops greater than two hops.

   Mac duplicate detection bug?

 */

// Routers use the top six bits of the short macs ids.
#define RIP_ID_MASK 0x3FF
#define RIP_ID_SHIFT 10
#define RIP_MAX_ROUTER_ID 63
#define ASSIGNED_RIP_ID_MASK_SIZE 8

#define RIP_MAX_METRIC 15
#define RIP_INFINITY 0

// The Thread spec says to use a Trickle timer with Imin = 1 second and
// Imax = 32 seconds.  Our Trickle delay in milliseconds is:
//   (1 << trickleExponent) + expRandom(trickleExponent);
// so our minimum exponent is 9
//   (1 << 9) + (random() & ((1 << 9) - 1)) => 512 ... 1023 milliseconds
// and our maximum is 14
//   (1 << 14) + (random() & ((1 << 14) - 1)) => 16 ... 32 seconds
#define MIN_TRICKLE_EXP 9
#define MAX_TRICKLE_EXP 14

// Don't go back to the minimum unless we have slowed down a bit.
// This avoids clogging the channel when there is a lot of churn.
// Note that if trickleExponent is 13, it means that the last ad
// we scheduled used an exponent of 12, ie, 4-8 seconds.
#define MIN_TRICKLE_EXP_TO_HASTEN 13

// +1 to get the fixed part of the interval plus the maximum random part
#define MLE_MAX_ADVERTISEMENT_INTERVAL_MS (1UL << (MAX_TRICKLE_EXP + 1))

// Aging event fires every 15 seconds.  The max age is 7, so neighbors
// age out after 105 to 120 seconds (7 or 8 ticks, depending on how
// soon the event fires after the link goes down).  Spec says 100
// seconds, so we are close.  Max unreachability is 6, so
// unreachability happens after 90 to 105 seconds.  Spec says 90, so
// again we are close without going early.
#define RIP_AGE_INTERVAL_MS 15000
#define MAX_NEIGHBOR_AGE 7
#define RIP_AGE_PENALTY_1 5       // Subtract 1 from incoming link quality
// Set this high enough so that it never kicks in.  Increasing the
// link quality by two increases the link cost enough to make two-hop
// loops look attractive.
#define RIP_AGE_PENALTY_2 100     // Subtract 2 from incoming link quality
#define INFINITE_COST_TIMEOUT 6   // 90s

#define RIP_MIN_TRIGGER_JITTER_MS 0
#define RIP_TRIGGER_JITTER_EXP_MS 10  // 1 second

extern uint8_t emRipTableSize;
extern RipEntry *emRipTable;
extern uint8_t emRipIdSequenceNumber;
extern uint8_t emRipTableCount;
uint8_t emRipRouterCount(void);

#define emRipIdToShortMacId(ripId) ((ripId) << RIP_ID_SHIFT)
#define emShortMacIdToRipId(shortMacId) ((shortMacId) >> RIP_ID_SHIFT)
#define emRipIsRouterShortMacId(shortMacId) (((shortMacId) & RIP_ID_MASK) == 0)
bool emIdIsMyChild(EmberNodeId shortMacId);
uint8_t emLocalRipId(void);
bool emIsRouterIpAddress(const uint8_t *address);

// Incoming and outgoing costs are stored in bits 4-5 and 6-7 of the metric
// field, respectively.
#define RIP_METRIC_MASK 0x0F
#define RIP_INCOMING_QUALITY_SHIFT 4
#define RIP_INCOMING_QUALITY_MASK  0x30
#define RIP_OUTGOING_QUALITY_SHIFT 6
#define RIP_OUTGOING_QUALITY_MASK  0xC0
#define RIP_MAX_LINK_QUALITY 3
#define emRipMetric(index) (emRipTable[(index)].metric & RIP_METRIC_MASK)
// On the leader, this marks router IDs that are unassigned but
// cannot yet be reassigned to another node.
#define RIP_RECENTLY_UNASSIGNED_ROUTER_METRIC 1
// This is the value used for the route byte in the RIP TLV corresponding
// to this router.  It is not a valid route byte since you can't have a rip
// metric of 1 without having a link.
#define RIP_TLV_SELF_PLACEHOLDER 1

// RIP table flags field.  Not sent over the air.
// NOTE: BIT 6 (0x40) IS CURRENTLY UNUSED
#define NEIGHBOR_AGE_MASK         0x07
#define UNREACHABILITY_TIMER_MASK 0x38
#define UNREACHABILITY_TIMER_SHIFT 3
#define MLE_LINK_ESTABLISHED      0x80
#define emNeighborAge(index) (emRipTable[(index)].flags & NEIGHBOR_AGE_MASK)
#define emUnreachabilityTimer(index) \
  ((emRipTable[(index)].flags & UNREACHABILITY_TIMER_MASK) >> UNREACHABILITY_TIMER_SHIFT)

void emRipInit(void);
void emStartRip(void);
void emStopRip(void);
void emSetRipIdSequenceNumber(uint8_t seq);
void emProcessRipMessage(PacketHeader header, Ipv6Header *ipHeader);
uint16_t emRipLookupNextHop(uint16_t shortMacId);
uint8_t emRipLinkCost(uint8_t ripId);
uint8_t emIncomingLinkQuality(uint8_t index);
uint8_t emOutgoingLinkQuality(uint8_t index);
void emUpdateIncomingLinkQuality(uint8_t index);
uint8_t emRssiToLinkMargin(int8_t rssi);
uint8_t emLinkMarginToQuality(uint8_t linkMargin);
int8_t emLinkQualityToRssi(uint8_t linkQuality);
void emConnectivityBucketCounts(uint8_t *buckets);
uint8_t emGetRoute(uint8_t destIndex, uint8_t *routeCostResult);
uint8_t emGetRouteCost(uint16_t shortMacId);
void emSetOutgoingLinkQuality(uint8_t index, uint8_t quality);
void emMaybeUpdateRipMetric(uint8_t destId,
                            uint8_t newNextHop,
                            uint8_t newNextHopMetric);
void emInitializeLeaderRipId(void);
bool emMakeAssignedRipIdMask(uint8_t *mask);
void emMarkAssignedRipIds(uint8_t sequence, const uint8_t *mask);
bool emIsAssignedId(EmberNodeId id, const uint8_t *neighborData);
bool emIsAssignedInMask(const uint8_t *mask, uint8_t index);
void emHastenRipAdvertisement(void);
void emMaybeHastenRipAdvertisement(uint8_t index, uint8_t oldRouteCost);

// Table management (found in rip-neighbor.c)
uint8_t emRouterIndex(uint8_t ripId);
uint8_t emInsertRipEntry(uint8_t index, uint8_t ripId);
void emMaybeClearRoute(uint16_t destId, uint16_t nextHop);
void emClearRoute(uint8_t index);
void emClearNeighbor(uint8_t index, bool preserveMultihopRoute);
void emRemoveRipEntry(uint8_t index);
uint8_t emLeaderIndex(void);
uint8_t emLeaderCost(void);
uint8_t emRouterOrLurkerIndexByLongId(const uint8_t *longId, bool isLurker);

#define emLurkerIndexByLongId(id) emRouterOrLurkerIndexByLongId(id, true)
#define emRouterIndexByLongId(id) emRouterOrLurkerIndexByLongId(id, false)

//------------------------------------------------------------------------------
// Id Assignment

uint8_t emAssignRipId(uint8_t requestedRipId,
                      const uint8_t *longId,
                      AddressManagementStatus status);
bool emUnassignRipId(const uint8_t *longId);

// debug -- print the RIP table
void emPrintRipTable(uint8_t port);

void emRipNeighborInit(void);
void emResetRipTable(bool keepParent);

// debug -- if true then ignore router ID requests in solicit messages,
// and assign router IDs in order
extern bool emAssignRouterIdsInOrder;

#endif // RIP_H
