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
*                                   EXAMPLE NETWORK CORE INITIALISATION
*                                        ETHERNET & WiFi INTERFACE
*
* File : ex_net_core_init.h
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  EX_NET_CORE_H
#define  EX_NET_CORE_H


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos_description.h>

#if (defined(RTOS_MODULE_NET_AVAIL))
#ifdef  RTOS_MODULE_NET_IF_ETHER_AVAIL
#include  <rtos/net/include/net_if_ether.h>
#endif
#ifdef  RTOS_MODULE_NET_IF_WIFI_AVAIL
#include  <rtos/net/include/net_if_wifi.h>
#endif
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

#if (defined(RTOS_MODULE_NET_AVAIL) && \
     defined(RTOS_MODULE_NET_IF_ETHER_AVAIL))
extern  NET_IF_ETHER_CFG  Ex_NetIF_CfgDflt_Ether;
#endif

#if (defined(RTOS_MODULE_NET_AVAIL) && \
     defined(RTOS_MODULE_NET_IF_WIFI_AVAIL))
extern  NET_IF_BUF_CFG    Ex_NetWiFi_IF_BufCfg;
extern  NET_IF_WIFI_CFG   Ex_NetIF_CfgDflt_WiFi;
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

void  Ex_Net_CoreInit       (void);

void  Ex_Net_CoreStartIF    (void);

void  Ex_Net_CoreStartEther (void);

void  Ex_Net_CoreStartWiFi  (void);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif  /* EX_NET_CORE_H */
