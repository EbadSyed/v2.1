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

#include  <bsp/include/bsp.h>

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/common.h>
#include  <rtos/kernel/include/os.h>

#include  <rtos/common/include/lib_def.h>
#include  <rtos/common/include/rtos_utils.h>
#include  <rtos/common/include/toolchains.h>

#include  <ex_description.h>

#include  "common/common/ex_common.h"

#if (defined(RTOS_MODULE_IO_AVAIL))
#include  <rtos/io/include/io.h>
#endif

#if (defined(RTOS_MODULE_IO_SERIAL_SPI_AVAIL) && defined(EX_IO_SPI_INIT_AVAIL))
#include  "io/ex_spi_init.h"
#endif

#if (defined(RTOS_MODULE_IO_SD_AVAIL) && defined(EX_IO_SD_INIT_AVAIL))
#include  "io/ex_sd_init.h"
#endif

#if (defined(RTOS_MODULE_FS_AVAIL) && defined(EX_FS_INIT_AVAIL))
#include  "fs/ex_fs.h"
#endif

#if (defined(RTOS_MODULE_USB_DEV_AVAIL) && defined(EX_USBD_CORE_INIT_AVAIL))
#include  "usb/device/ex_usbd.h"
#endif

#if (defined(RTOS_MODULE_USB_HOST_AVAIL) && defined(EX_USBH_CORE_INIT_AVAIL))
#include  "usb/host/ex_usbh.h"
#endif

#if (defined(RTOS_MODULE_NET_AVAIL) && defined(EX_NETWORK_INIT_AVAIL))
#include  "net/ex_network_init.h"
#include  "net/core_init/ex_net_core_init.h"
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#define  EX_MAIN_START_TASK_PRIO              21u
#define  EX_MAIN_START_TASK_STK_SIZE         768u


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


    OSInit(&err);                                               /* Initialize the Kernel.                               */
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), 1);


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
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), 1);


    OSStart(&err);                                              /* Start the kernel.                                    */
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), 1);


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
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);
#endif

#if (OS_CFG_TRACE_EN == DEF_ENABLED)
    OS_TRACE_INIT();                                            /* Initialize the Kernel events trace recorder.         */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();                                /* Initialize interrupts disabled measurement.          */
#endif

    Ex_CommonInit();                                            /* Call common module initialization example.           */

    BSP_PeriphInit();                                           /* Initialize the BSP. It is expected that the BSP ...  */
                                                                /* ... will register all the hardware controller to ... */
                                                                /* ... the platform manager at this moment.             */

#if (defined(RTOS_MODULE_IO_AVAIL))
    IO_Init(&err);                                              /* Initialize common IO module(s).                      */
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
#endif

#if (defined(RTOS_MODULE_IO_SERIAL_SPI_AVAIL) && defined(EX_IO_SPI_INIT_AVAIL))
    Ex_SPI_Init();                                              /* Call SPI module initialization example.              */

    Ex_SPI_Start();                                             /* Call SPI module start example.                       */
#endif

#if (defined(RTOS_MODULE_IO_SD_AVAIL) && defined(EX_IO_SD_INIT_AVAIL))
    Ex_SD_Init();                                               /* Call SD module initialization example.               */

    Ex_SD_Start();                                              /* Call SD module start example.                        */
#endif

#if (defined(RTOS_MODULE_FS_AVAIL) && defined(EX_FS_INIT_AVAIL))
    Ex_FS_Init();                                               /* Call File System initialization example.             */
#endif

#if (defined(RTOS_MODULE_USB_DEV_AVAIL) && defined(EX_USBD_CORE_INIT_AVAIL))
    Ex_USBD_Init();                                             /* Call USB device module initialization example.       */

    Ex_USBD_Start();                                            /* Call USB device controller start.                    */
#endif

#if (defined(RTOS_MODULE_USB_HOST_AVAIL) && defined(EX_USBH_CORE_INIT_AVAIL))
    Ex_USBH_Init();                                             /* Call USB host module initialization example.         */

    Ex_USBH_Start();                                            /* Call USB host controller start.                      */
#endif

#if (defined(RTOS_MODULE_NET_AVAIL) && defined(EX_NETWORK_INIT_AVAIL))
    Ex_NetworkInit();                                           /* Call Network module initialization example.          */

    Ex_Net_CoreStartIF();                                       /* Call network interface start example.                */
#endif


    /* TODO : Add application code or other example calls here. */


    while (DEF_ON) {
                                                                /* Delay Start Task execution for                       */
        OSTimeDly( 5000,                                        /*   5000 OS Ticks                                      */
                   OS_OPT_TIME_PERIODIC,                        /*   from now.                                          */
                  &err);
                                                                /*   Check error code.                                  */
        APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);
    }
}
