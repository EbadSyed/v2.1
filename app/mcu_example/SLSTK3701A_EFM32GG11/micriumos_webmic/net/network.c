/**************************************************************************//**
 * @file network.c
 * @brief Initializes the network
 * @version 5.3.5
 ******************************************************************************
 * # License
 * <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
// Dependencies and avail check(s)

#include  <rtos_description.h>

#if (defined(RTOS_MODULE_NET_AVAIL))

// -----------------------------------------------------------------------------
// Include files

#include  "../description.h"

#include  "core/net_core.h"

#if (defined(RTOS_MODULE_NET_HTTP_CLIENT_AVAIL) && defined(EX_HTTP_CLIENT_INIT_AVAIL))
#include  "http/client/http_client.h"
#endif

#if (defined(RTOS_MODULE_NET_HTTP_SERVER_AVAIL) && defined(EX_HTTP_SERVER_INIT_AVAIL))
#include  "http/server/http_server.h"
#endif

#if (defined(RTOS_MODULE_NET_MQTT_CLIENT_AVAIL) && defined(EX_MQTT_CLIENT_INIT_AVAIL))
#include  "mqtt/ex_mqtt_client.h"
#endif

#if (defined(RTOS_MODULE_NET_TELNET_SERVER_AVAIL) && defined(EX_TELNET_SERVER_INIT_AVAIL))
#include  "telnet/ex_telnet_server.h"
#endif

#if (defined(RTOS_MODULE_NET_IPERF_AVAIL) && defined(EX_IPERF_INIT_AVAIL))
#include  "iperf/ex_iperf.h"
#endif

#if (defined(RTOS_MODULE_NET_SNTP_CLIENT_AVAIL) && defined(EX_SNTP_CLIENT_INIT_AVAIL))
#include  "sntp/ex_sntp_client.h"
#endif

#if (defined(RTOS_MODULE_NET_SMTP_CLIENT_AVAIL) && defined(EX_SMTP_CLIENT_INIT_AVAIL))
#include  "smtp/ex_smtp_client.h"
#endif

// -----------------------------------------------------------------------------
// Global functions

/*
 *********************************************************************************************************
 *                                           Ex_NetworkInit()
 *
 * Description : Initialize the Network Module including the core module and all the network applications
 *               selected.
 *
 * Argument(s) : None.
 *
 * Return(s)   : None.
 *
 * Note(s)     : None.
 *********************************************************************************************************
 */
void  Ex_NetworkInit(void)
{
  Ex_Net_CoreInit(); // Initialize core module

  // Initialize client module
#if (defined(RTOS_MODULE_NET_HTTP_CLIENT_AVAIL) \
  && defined(EX_HTTP_CLIENT_INIT_AVAIL))
  Ex_HTTP_Client_Init();
#endif

  // Initialize http server module
#if (defined(RTOS_MODULE_NET_HTTP_SERVER_AVAIL) \
  && defined(EX_HTTP_SERVER_INIT_AVAIL))
  Ex_HTTP_Server_Init();
#endif

  // Initialize MQTT client module
#if (defined(RTOS_MODULE_NET_MQTT_CLIENT_AVAIL) \
  && defined(EX_MQTT_CLIENT_INIT_AVAIL))
  Ex_MQTT_Client_Init();
#endif

  // Initialize telnet server module
#if (defined(RTOS_MODULE_NET_TELNET_SERVER_AVAIL) \
  && defined(EX_TELNET_SERVER_INIT_AVAIL))
  Ex_TELNET_Server_Init();
#endif

  // Initialize IPERF module
#if (defined(RTOS_MODULE_NET_IPERF_AVAIL) \
  && defined(EX_IPERF_INIT_AVAIL))
  Ex_IPerf_Init();
#endif

  // Initialize SNTP client module
#if (defined(RTOS_MODULE_NET_SNTP_CLIENT_AVAIL) \
  && defined(EX_SNTP_CLIENT_INIT_AVAIL))
  Ex_SNTP_Client_Init();
#endif

  // Initialize SMTP client module
#if (defined(RTOS_MODULE_NET_SMTP_CLIENT_AVAIL) \
  && defined(EX_SMTP_CLIENT_INIT_AVAIL))
  Ex_SMTP_Client_Init();
#endif
}

// -----------------------------------------------------------------------------
// Dependencies and avail check(s)

#endif // RTOS_MODULE_NET_AVAIL
