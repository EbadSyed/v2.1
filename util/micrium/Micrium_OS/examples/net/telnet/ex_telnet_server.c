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
*                                                EXAMPLE
*
*                                             TELNET SERVER
*
* File : ex_telnet_server.c
*********************************************************************************************************
* Note(s) : (1) For additional details on the features available with Micrium OS Telnet Server,
*               the API, the installation, etc. Refer to the Micrium OS Telnet Server documentation
*               available at https://doc.micrium.com.
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

#if (defined(RTOS_MODULE_NET_TELNET_SERVER_AVAIL))


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/rtos_err.h>
#include  <rtos/common/include/rtos_utils.h>
#include  <rtos/common/include/auth.h>
#include  <rtos/net/include/telnet_server.h>

#include  <rtos/common/include/rtos_prio.h>

#include  <ex_description.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  EX_TELNET_USERNAME
#define  EX_TELNET_USERNAME         "DUT"
#endif

#ifndef  EX_TELNET_PASSWORD
#define  EX_TELNET_PASSWORD         "micrium"
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/


#if defined(EX_TELNET_SERVER_SRV_TASK_STK_SIZE) ||  \
    defined(EX_TELNET_SERVER_SRV_TASK_PRIO)

#ifndef  EX_TELNET_SERVER_SRV_TASK_STK_SIZE
#define  EX_TELNET_SERVER_SRV_TASK_STK_SIZE         TELNET_SERVER_TASK_CFG_STK_SIZE_ELEMENTS_DFLT
#endif

#ifndef EX_TELNET_SERVER_SRV_TASK_PRIO
#define  EX_TELNET_SERVER_SRV_TASK_PRIO             TELNET_SERVER_SRV_TASK_PRIO_DFLT
#endif

RTOS_TASK_CFG  Ex_TELNET_Server_SrvTaskCfg = {
    .Prio            = EX_TELNET_SERVER_SRV_TASK_PRIO,
    .StkSizeElements = EX_TELNET_SERVER_SRV_TASK_STK_SIZE,
    .StkPtr          = DEF_NULL
};

RTOS_TASK_CFG   *Ex_TELNET_Server_SrvTaskCfgPtr = &Ex_TELNET_Server_SrvTaskCfg;

#else
RTOS_TASK_CFG   *Ex_TELNET_Server_SrvTaskCfgPtr = DEF_NULL;
#endif



#if defined(EX_TELNET_SERVER_SESSION_TASK_STK_SIZE) ||  \
    defined(EX_TELNET_SERVER_SESSION_TASK_PRIO)

#ifndef  EX_TELNET_SERVER_SESSION_TASK_STK_SIZE
#define  EX_TELNET_SERVER_SESSION_TASK_STK_SIZE         TELNET_SERVER_SESSION_TASK_CFG_STK_SIZE_ELEMENTS_DFLT
#endif

#ifndef EX_TELNET_SERVER_SESSION_TASK_PRIO
#define  EX_TELNET_SERVER_SESSION_TASK_PRIO             TELNET_SERVER_SESSION_TASK_PRIO_DFLT
#endif

RTOS_TASK_CFG  Ex_TELNET_Server_SessionTaskCfg = {
    .Prio            = EX_TELNET_SERVER_SESSION_TASK_PRIO,
    .StkSizeElements = EX_TELNET_SERVER_SESSION_TASK_STK_SIZE,
    .StkPtr          = DEF_NULL
};

RTOS_TASK_CFG   *Ex_TELNET_Server_SessionTaskCfgPtr = &Ex_TELNET_Server_SessionTaskCfg;

#else
RTOS_TASK_CFG   *Ex_TELNET_Server_SessionTaskCfgPtr = DEF_NULL;
#endif



/*
*********************************************************************************************************
*                                       Ex_TELNET_Server_Init()
*
* Description : Example of initialization of the Telnet server.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_TELNET_Server_Init (void)
{
    RTOS_ERR  err;


    TELNETs_Init(&err);                                         /* Initialize the Telnet server.                        */
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);
}


/*
*********************************************************************************************************
*                                   Ex_TELNET_Server_InstanceCreate()
*
* Description : Initializes and starts a default telnet server instance.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_TELNET_Server_InstanceCreate (void)
{
    TELNETs_INSTANCE  *p_instance;
    RTOS_ERR           err;


    Auth_CreateUser(EX_TELNET_USERNAME, EX_TELNET_PASSWORD, &err);
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);


    p_instance = TELNETs_InstanceInit(DEF_NULL,
                                      Ex_TELNET_Server_SrvTaskCfgPtr,
                                      Ex_TELNET_Server_SessionTaskCfgPtr,
                                     &err);
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);


    TELNETs_InstanceStart(p_instance, &err);
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif /* RTOS_MODULE_NET_TELNET_SERVER_AVAIL */

