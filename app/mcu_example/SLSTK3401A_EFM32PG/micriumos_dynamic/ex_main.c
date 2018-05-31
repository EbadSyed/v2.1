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

#include  "bsp_os.h"
#include  "bsp_cpu.h"
#include  "bsp.h"

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/common.h>
#include  <rtos/kernel/include/os.h>

#include  <rtos/common/include/lib_def.h>
#include  <rtos/common/include/rtos_utils.h>
#include  <rtos/common/include/toolchains.h>
#include  <rtos/common/include/rtos_prio.h>

#include  "common/common/ex_common.h"
#include  "em_gpio.h"
#include  "retargetserial.h"
#include  "sleep.h"
#include  <stdio.h>

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

static void Ex_MainStartTask (void  *p_arg);
static void idleHook(void);
static void setupHooks(void);

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
int main(void)
{
    RTOS_ERR  err;
    /* Setup a 1024 Hz tick instead of the default 1000 Hz. This 
     * improves accuracy when using dynamic tick which runs of 
     * the RTCC at 32768 Hz. */
    OS_TASK_CFG tickTaskCfg = {
      .StkBasePtr = DEF_NULL,
      .StkSize    = 256u,
      .Prio       = KERNEL_TICK_TASK_PRIO_DFLT,
      .RateHz     = 1024u
    };

    BSP_CPUInit();                                              /* Initialize CPU and make all interrupts Kernel Aware. */
    BSP_SystemInit();                                           /* Initialize System.                                   */
    
    RETARGET_SerialInit();
    RETARGET_SerialCrLf(1);

    OS_TRACE_INIT();
    OS_ConfigureTickTask(&tickTaskCfg);
    OSInit(&err);                                               /* Initialize the Kernel.                               */
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

    BSP_TickInit();                                             /* Initialize Kernel tick source.                       */
    setupHooks();

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

#if (OS_CFG_STAT_TASK_EN == DEF_ENABLED)
    OSStatTaskCPUUsageInit(&err);                               /* Initialize CPU Usage.                                */
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();                                /* Initialize interrupts disabled measurement.          */
#endif

    Ex_CommonInit();                                            /* Call common module initialization example.           */
    BSP_OS_Init();                                              /* Initialize the BSP. It is expected that the BSP ...  */
                                                                /* ... will register all the hardware controller to ... */
                                                                /* ... the platform manager at this moment.             */

    BSP_LedsInit();
    while (DEF_ON) {
        BSP_LedToggle(1);
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_PERIODIC, &err);
    }
}

/***************************************************************************//**
 * @brief
 *   This is the idle hook. 
 *
 * @detail
 *   This will be called by the Micrium OS idle task when there is no other 
 *   task ready to run. We just enter the lowest possible energy mode.
 ******************************************************************************/
static void idleHook(void)
{
  /* Put MCU in the lowest sleep mode available, usually EM2 */
  SLEEP_Sleep();
}

/***************************************************************************//**
 * @brief
 *   Setup the Micrium OS hooks. We are only using the idle task hook in this 
 *   example. See the Mcirium OS documentation for more information on the 
 *   other available hooks.
 ******************************************************************************/
static void setupHooks(void)
{
    CPU_SR_ALLOC();

    CPU_CRITICAL_ENTER();
    /* Don't allow EM3, since we use LF clocks. */
    SLEEP_SleepBlockBegin(sleepEM3);
    OS_AppIdleTaskHookPtr = idleHook;
    CPU_CRITICAL_EXIT();
}
