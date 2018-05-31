/*
 * File: dns-client.h
 * Description: DNS client
 *
 * Copyright 2016 Silicon Laboratories, Inc.                                *80*
 */

#ifndef DNS_CLIENT_H
#define DNS_CLIENT_H

#define DNS_PORT 53

void emDnsClientIncomingMessageHandler(PacketHeader header,
                                       Ipv6Header *ipHeader);

#endif // DNS_CLIENT_H
