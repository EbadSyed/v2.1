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
*                                             EXAMPLE MAIN
*
* File : ex_main.c
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <bsp_os.h>
#include  "bsp.h"

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/common.h>
#include  <rtos/kernel/include/os.h>

#include  <rtos/common/include/lib_def.h>
#include  <rtos/common/include/rtos_utils.h>
#include  <rtos/common/include/toolchains.h>

#include  "common/common/ex_common.h"

#ifdef RTOS_MODULE_FS_AVAIL
#include  "fs/ex_fs.h"
#endif

#ifdef RTOS_MODULE_USB_DEV_AVAIL
#include  "usb/device/ex_usbd.h"
#endif

#ifdef RTOS_MODULE_USB_HOST_AVAIL
#include  "usb/host/ex_usbh.h"
#endif

#ifdef RTOS_MODULE_NET_AVAIL
#include  "net/ex_network.h"
#include  "net/core/ex_net_core.h"
#endif

#ifdef RTOS_MODULE_PROBE_SCOPE_AVAIL
#include  "probe/ex_probe_scope.h"
#endif

#ifdef RTOS_MODULE_PROBE_TERM_AVAIL
#include  "probe/ex_probe_term.h"
#endif

#include "retargetserial.h"
#include "stdio.h"

/*
*********************************************************************************************************
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#define  EX_MAIN_START_TASK_PRIO              21u
#define  EX_MAIN_START_TASK_STK_SIZE         512u


/*
*********************************************************************************************************
*********************************************************************************************************
*                                        LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

                                                                /* Start Task Stack.                                    */
static  CPU_STK  Ex_MainStartTaskStk[EX_MAIN_START_TASK_STK_SIZE];
                                                                /* Start Task TCB.                                      */
static  OS_TCB   Ex_MainStartTaskTCB;


/*
*********************************************************************************************************
*********************************************************************************************************
*                                       LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  void  Ex_MainStartTask (void  *p_arg);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C applications. It is assumed that your code will
*               call main() once you have performed all necessary initialization.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

int  main (void)
{
    RTOS_ERR  err;


    CPU_Init();                                                 /* Initialize CPU.                                      */
    BSP_SystemInit();                                           /* Initialize System.                                   */


    RETARGET_SerialInit();
    RETARGET_SerialCrLf(1);
    printf("\n\n*** MicriumOS USB-host Example. ***\n");

    OS_TRACE_INIT();
    OSInit(&err);                                               /* Initialize the Kernel.                               */
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);


    OSTaskCreate(&Ex_MainStartTaskTCB,                          /* Create the Start Task.                               */
                 "Ex Main Start Task",
                  Ex_MainStartTask,
                  DEF_NULL,
                  EX_MAIN_START_TASK_PRIO,
                 &Ex_MainStartTaskStk[0],
                 (EX_MAIN_START_TASK_STK_SIZE / 10u),
                  EX_MAIN_START_TASK_STK_SIZE,
                  0u,
                  0u,
                  DEF_NULL,
                 (OS_OPT_TASK_STK_CLR),
                 &err);
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);


    OSStart(&err);                                              /* Start the kernel.                                    */
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);


    return (1);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          Ex_MainStartTask()
*
* Description : This is the task that will be called by the Startup when all services are initializes
*               successfully.
*
* Argument(s) : p_arg   Argument passed from task creation. Unused, in this case.
*
* Return(s)   : None.
*
* Notes       : None.
*********************************************************************************************************
*/

static  void  Ex_MainStartTask (void  *p_arg)
{
    RTOS_ERR  err;


    PP_UNUSED_PARAM(p_arg);                                     /* Prevent compiler warning.                            */


    BSP_TickInit();                                             /* Initialize Kernel tick source.                       */


#if (OS_CFG_STAT_TASK_EN == DEF_ENABLED)
    OSStatTaskCPUUsageInit(&err);                               /* Initialize CPU Usage.                                */
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();                                /* Initialize interrupts disabled measurement.          */
#endif

    Ex_CommonInit();                                            /* Call common module initialization example.           */

    BSP_OS_Init();                                                 /* Initialize the BSP. It is expected that the BSP ...  */
                                                                /* ... will register all the hardware controller to ... */
                                                                /* ... the platform manager at this moment.             */

#ifdef RTOS_MODULE_FS_AVAIL
    Ex_FS_Init();                                               /* Call File System initialization example.             */
#endif

#ifdef RTOS_MODULE_USB_DEV_AVAIL
    Ex_USBD_Init();                                             /* Call USB device module initialization example.       */

    Ex_USBD_Start();                                            /* Call USB device controller start.                    */
#endif

#ifdef RTOS_MODULE_USB_HOST_AVAIL
    Ex_USBH_Init();                                             /* Call USB host module initialization example.         */

    Ex_USBH_Start();                                            /* Call USB host controller start.                      */
#endif

#ifdef RTOS_MODULE_NET_AVAIL
    Ex_NetworkInit();                                           /* Call Network module initialization example.          */

    Ex_Net_CoreStartIF();                                       /* Call network interface start example.                */
#endif

#ifdef RTOS_MODULE_PROBE_SCOPE_AVAIL
    Ex_ProbeScopeInit();                                        /* Call Probe Scope module initialization example.      */
#endif

#ifdef RTOS_MODULE_PROBE_TERM_AVAIL
    Ex_ProbeTermInit();                                         /* Call Probe Terminal module initialization example.   */
#endif


    /* TODO : Add application code or other example calls here. */


    while (DEF_ON) {
                                                                /* Delay Start Task execution for                       */
        OSTimeDly( 5000,                                        /*   5000 OS Ticks                                      */
                   OS_OPT_TIME_DLY,                             /*   from now.                                          */
                  &err);
                                                                /*   Check error code.                                  */
        APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
    }
}
