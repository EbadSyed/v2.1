/*
*********************************************************************************************************
*                                             EXAMPLE CODE
*********************************************************************************************************
* Licensing terms:
*   This file is provided as an example on how to use Micrium products. It has not necessarily been
*   tested under every possible condition and is only offered as a reference, without any guarantee.
*
*   Please feel free to use any application code labeled as 'EXAMPLE CODE' in your application products.
*   Example code may be used as is, in whole or in part, or may be used as a reference only. This file
*   can be modified as required.
*
*   You can find user manuals, API references, release notes and more at: https://doc.micrium.com
*
*   You can contact us at: http://www.micrium.com
*
*   Please help us continue to provide the Embedded community with the finest software available.
*
*   Your honesty is greatly appreciated.
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

#include  "ex_description.h"

#include  "core/ex_net_core.h"

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

