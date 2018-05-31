/*
*********************************************************************************************************
*                                             EXAMPLE CODE
*********************************************************************************************************
* Licensing:
*   The licensor of this EXAMPLE CODE is Silicon Laboratories Inc.
*
*   Silicon Laboratories Inc. grants you a personal, worldwide, royalty-free, fully paid-up license to
*   use, copy, modify and distribute the EXAMPLE CODE software, or portions thereof, in any of your
*   products.
*
*   Your use of this EXAMPLE CODE is at your own risk. This EXAMPLE CODE does not come with any
*   warranties, and the licensor disclaims all implied warranties concerning performance, accuracy,
*   non-infringement, merchantability and fitness for your application.
*
*   The EXAMPLE CODE is provided "AS IS" and does not come with any support.
*
*   You can find user manuals, API references, release notes and more at: https://doc.micrium.com
*
*   You can contact us at: https://www.micrium.com
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      EXAMPLE NETWORK INITIALISATION
*
* File : ex_network.c
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                     DEPENDENCIES & AVAIL CHECK(S)
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos_description.h>

#if (defined(RTOS_MODULE_NET_AVAIL))


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <ex_description.h>

#include  "core_init/ex_net_core_init.h"

#if (defined(RTOS_MODULE_NET_HTTP_CLIENT_AVAIL) && defined(EX_HTTP_CLIENT_INIT_AVAIL))
#include  "http/client/ex_http_client.h"
#endif
#if (defined(RTOS_MODULE_NET_HTTP_SERVER_AVAIL) && defined(EX_HTTP_SERVER_INIT_AVAIL))
#include  "http/server/ex_http_server.h"
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


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

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

void  Ex_NetworkInit (void)
{
                                                                /* -------------- INITIALIZE CORE MODULE -------------- */
    Ex_Net_CoreInit();

                                                                /* ---------- INITIALIZE HTTP CLIENT MODULE ----------- */
#if (defined(RTOS_MODULE_NET_HTTP_CLIENT_AVAIL) && \
     defined(EX_HTTP_CLIENT_INIT_AVAIL))
    Ex_HTTP_Client_Init();
#endif

                                                                /* ---------- INITIALIZE HTTP SERVER MODULE ----------- */
#if (defined(RTOS_MODULE_NET_HTTP_SERVER_AVAIL) && \
     defined(EX_HTTP_SERVER_INIT_AVAIL))
    Ex_HTTP_Server_Init();
#endif

                                                                /* ---------- INITIALIZE MQTT CLIENT MODULE ----------- */
#if (defined(RTOS_MODULE_NET_MQTT_CLIENT_AVAIL) && \
     defined(EX_MQTT_CLIENT_INIT_AVAIL))
    Ex_MQTT_Client_Init();
#endif

                                                                /* --------- INITIALIZE TELNET SERVER MODULE ---------- */
#if (defined(RTOS_MODULE_NET_TELNET_SERVER_AVAIL) && \
     defined(EX_TELNET_SERVER_INIT_AVAIL))
    Ex_TELNET_Server_Init();
#endif

                                                                /* ------------- INITIALIZE IPERF MODULE -------------- */
#if (defined(RTOS_MODULE_NET_IPERF_AVAIL) && \
     defined(EX_IPERF_INIT_AVAIL))
    Ex_IPerf_Init();
#endif

                                                                /* ---------- INITIALIZE SNTP CLIENT MODULE ----------- */
#if (defined(RTOS_MODULE_NET_SNTP_CLIENT_AVAIL) && \
     defined(EX_SNTP_CLIENT_INIT_AVAIL))
    Ex_SNTP_Client_Init();
#endif

                                                                /* ---------- INITIALIZE SMTP CLIENT MODULE ----------- */
#if (defined(RTOS_MODULE_NET_SMTP_CLIENT_AVAIL) && \
     defined(EX_SMTP_CLIENT_INIT_AVAIL))
    Ex_SMTP_Client_Init();
#endif
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif  /* RTOS_MODULE_NET_AVAIL */

