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
*                                       EXAMPLE HTTP SERVER
*
* File : ex_http_server.h
*********************************************************************************************************
*/

#include  <rtos/cpu/include/cpu.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  EX_HTTP_SERVER_H
#define  EX_HTTP_SERVER_H


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

extern  CPU_CHAR  Ex_HTTPs_Name[];


/*
*********************************************************************************************************
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

#if  defined(RTOS_MODULE_NET_AVAIL)             && \
     defined(RTOS_MODULE_NET_HTTP_SERVER_AVAIL)

void  Ex_HTTP_Server_Init                   (void);

void  Ex_HTTP_Server_InstanceCreateStaticFS (void);

void  Ex_HTTP_Server_InstanceCreateNoFS     (void);

void  Ex_HTTP_Server_InstanceCreateSecure   (void);

#ifdef  RTOS_MODULE_FS_AVAIL
void  Ex_HTTP_Server_InstanceCreateBasic    (void);
#endif

void  Ex_HTTP_Server_InstanceCreateREST     (void);

void  Ex_HTTP_Server_InstanceCreateCtrlLayer(void);

#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif
