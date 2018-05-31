#ifndef __EMBER_H__
#define __EMBER_H__

#include "ember-types.h"
#include "error.h"
#include "network-management.h"
#if (defined (EMBER_TEST)        \
  || defined(QA_THREAD_TEST)     \
  || defined(EMBER_WAKEUP_STACK) \
  || defined(RTOS))
  #include "wakeup.h"
#endif

// APIs
#include "byte-utilities.h"
#include "child.h"
#include "coap.h"
#include "coap-diagnostic.h"
#include "dtls.h"
#include "ember-debug.h"
#include "event.h"
#include "icmp.h"
#include "mfglib.h"
#include "network-data-tlv.h"
#include "stack-info.h"
#include "tcp.h"
#include "udp.h"
#include "udp-peer.h"

// config
#include "config/config.h"

// tmsp
#include "app/tmsp/tmsp-enum.h"
#include "app/tmsp/tmsp-mfglib.h"

extern const EmberVersion emberVersion;

#endif // __EMBER_H__
