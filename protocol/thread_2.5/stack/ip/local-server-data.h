/*
 * File: local-server-data.h
 * Description: information about this node's servers
 *
 * Copyright 2013 by Silicon Laboratories. All rights reserved.             *80*
 */

#ifndef LOCAL_SERVER_DATA_H
#define LOCAL_SERVER_DATA_H

extern Buffer emLocalServerData;
extern Buffer emBorderRouterNdData;

// No point in allowing an individual node more data than can fit in
// the network data.
#define MAX_MY_NETWORK_DATA_SIZE MAX_NETWORK_DATA_SIZE

void emVerifyLocalServerData(void);
void emInitializeLeaderServerData(void);

void emLocalServerDataInit(void);

bool emHaveExternalRoute(const uint8_t *source, const uint8_t *dest);

void emResendNodeServerData(EmberNodeId oldNodeId);
void emRemoveChildServerData(EmberNodeId nodeId);

bool emHandleLocalServerDataPost(const uint8_t *uri,
                                 const uint8_t *payload,
                                 uint16_t payloadLength,
                                 const EmberCoapRequestInfo *info);

#endif // LOCAL_SERVER_DATA_H
