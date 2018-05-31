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
*                                         KERNEL MUTEX EXAMPLE
*
* File : ex_kernel_mutex.c
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/common.h>
#include  <rtos/kernel/include/os.h>

#include  <rtos/common/include/lib_def.h>
#include  <rtos/common/include/rtos_utils.h>
#include  <rtos/common/include/toolchains.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                  EXAMPLE CONFIGURATION REQUIREMENTS
*********************************************************************************************************
*********************************************************************************************************
*/

#if (OS_CFG_MUTEX_EN != DEF_ENABLED)
#error "This example requires OS_CFG_MUTEX_EN to be set to DEF_ENABLED"
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               LOGGING
*
* Note(s) : (1) This example outputs information to the console via the function printf() via a macro
*               called EX_TRACE(). This can be modified or disabled if printf() is not supported.
*********************************************************************************************************
*/

#ifndef  EX_TRACE
#include  <stdio.h>
#define  EX_TRACE(...)                                      printf(__VA_ARGS__)
#endif


/*
*********************************************************************************************************
*                                         TASK'S CONFIGURATION
*********************************************************************************************************
*/

                                                                /* Pend Task Configuration.                             */
#define  EX_KERNEL_MUTEX_PEND_TASK_STK_SIZE              128u
#define  EX_KERNEL_MUTEX_PEND_TASK_PRIO                   22u


/*
*********************************************************************************************************
*                                        EXAMPLE MUTEX SETTINGS
*********************************************************************************************************
*/

                                                                /* Number of pend/posts before deletion.                */
#define  EX_KERNEL_MUTEX_MAX_COUNT                         1u


/*
*********************************************************************************************************
*********************************************************************************************************
*                                        LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

                                                                /* Pend Task Data.                                      */
static  CPU_STK     Ex_KernelMutexPendTaskStk[EX_KERNEL_MUTEX_PEND_TASK_STK_SIZE];
static  OS_TCB      Ex_KernelMutexPendTaskTCB;

                                                                /* Mutex.                                               */
static  OS_MUTEX    Ex_KernelMutexObj;

                                                                /* Example execution counter.                           */
static  CPU_INT08U  Ex_Done;


/*
*********************************************************************************************************
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  void  Ex_KernelMutexPendTask(void  *p_arg);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          LOCAL CONSTANTS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           Ex_KernelMutex()
*
* Description : Illustrates the usage of kernel mutexes.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Notes       : None.
*********************************************************************************************************
*/

void  Ex_KernelMutex (void)
{
    RTOS_ERR    err;
    OS_OBJ_QTY  qty;


                                                                /* Initialize Example.                                  */
    Ex_Done = 0u;

                                                                /* Create Mutex.                                        */
    OSMutexCreate(&Ex_KernelMutexObj,                           /*   Pointer to the example mutex.                      */
                  "Ex Kernel Mutex",                            /*   Name used for debugging.                           */
                  &err);
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);

    OSTaskCreate(&Ex_KernelMutexPendTaskTCB,                    /* Create Pend Task.                                    */
                 "Ex Kernel Mutex Pend Task",
                  Ex_KernelMutexPendTask,
                  0,
                  EX_KERNEL_MUTEX_PEND_TASK_PRIO,
                 &Ex_KernelMutexPendTaskStk[0],
                 (EX_KERNEL_MUTEX_PEND_TASK_STK_SIZE / 10u),
                  EX_KERNEL_MUTEX_PEND_TASK_STK_SIZE,
                  0,
                  0,
                  0,
                 (OS_OPT_TASK_STK_CLR),
                 &err);
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);

                                                                /* Ex_KernelMutexPendTask will execute.                 */

                                                                /* Delay Task execution for                             */
    OSTimeDly( 500,                                             /*   500 OS Ticks                                       */
               OS_OPT_TIME_DLY,                                 /*   from now.                                          */
              &err);

                                                                /* Ex_KernelMutexPendTask has blocked, execution        */
                                                                /* returns here.                                        */

                                                                /* Try to acquire mutex, the Pend Task will resume.     */
    OSMutexPend(&Ex_KernelMutexObj,                             /*   Pointer to the example mutex.                      */
                 0,                                             /*   Wait for infinity.                                 */
                 OS_OPT_PEND_BLOCKING,                          /*   Task will block.                                   */
                 DEF_NULL,                                      /*   Timestamp is not used.                             */
                &err);
    if (err.Code == RTOS_ERR_NONE) {                            /*   Check error code.                                  */

                                                                /* Ex_KernelMutexPendTask released the mutex,           */
                                                                /* Ex_KernelMutex now has it.                           */
        Ex_Done = 1u;

        EX_TRACE("Example Kernel Mutex: Ex_KernelMutex got the mutex, now releasing it.\r\n");

                                                                /* Release mutex.                                       */
        OSMutexPost(&Ex_KernelMutexObj,                         /*   Pointer to the example mutex.                      */
                     OS_OPT_POST_1,                             /*   Only wake up highest-priority task.                */
                    &err);
                                                                /*   Check error code.                                  */
        APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);
    } else {
        EX_TRACE("Example Kernel Mutex: unable to pend on mutex. OSMutexPend() returned with %i\r\n",
                  err.Code);
        RTOS_ASSERT_CRITICAL_FAILED_END_CALL(0u);
    }

                                                                /* Delete Mutex.                                        */
    qty = OSMutexDel(&Ex_KernelMutexObj,                        /*   Pointer to the example evnt flag group.            */
                      OS_OPT_DEL_NO_PEND,                       /*   Option: delete if 0 tasks are pending.             */
                     &err);
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);
    PP_UNUSED_PARAM(qty);
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
*                                        Ex_KernelMutexPendTask()
*
* Description : This task pends on the mutex.
*
* Argument(s) : p_arg
*
* Return(s)   : None.
*
* Notes       : None.
*********************************************************************************************************
*/

static  void  Ex_KernelMutexPendTask (void  *p_arg)
{
    RTOS_ERR  err;


                                                                /* Prevent compiler warning.                            */
    PP_UNUSED_PARAM(p_arg);

                                                                /* Task body.                                           */
    while (DEF_ON) {
        if (Ex_Done < EX_KERNEL_MUTEX_MAX_COUNT) {
                                                                /* Acquire mutex.                                       */
            OSMutexPend(&Ex_KernelMutexObj,                     /*   Pointer to the example mutex.                      */
                         0,                                     /*   Wait for infinity.                                 */
                         OS_OPT_PEND_BLOCKING,                  /*   Task will block.                                   */
                         DEF_NULL,                              /*   Timestamp is not used.                             */
                        &err);
            if (err.Code == RTOS_ERR_NONE) {                    /*   Check error code.                                  */
                EX_TRACE("Example Kernel Mutex: Ex_KernelMutexPendTask got the mutex, now releasing it.\r\n");

                                                                /* Ex_KernelMutex will resume execution.                */

                                                                /* Delay Task execution for                             */
                OSTimeDly( 500,                                 /*   500 OS Ticks                                       */
                           OS_OPT_TIME_DLY,                     /*   from now.                                          */
                          &err);
                                                                /*   Check error code.                                  */
                APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);

                                                                /* Ex_KernelMutex will resume execution.                */

                                                                /* Release mutex.                                       */
                OSMutexPost(&Ex_KernelMutexObj,                 /*   Pointer to the example mutex.                      */
                             OS_OPT_POST_1,                     /*   Only wake up highest-priority task.                */
                            &err);

                                                                /*   Check error code.                                  */
                APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);
            } else {
                EX_TRACE("Example Kernel Mutex: unable to pend on mutex. OSMutexPend() returned with %i\r\n",
                          err.Code);
                RTOS_ASSERT_CRITICAL_FAILED_END_CALL(;);
            }
        }
                                                                /* Delay Task execution for                             */
        OSTimeDly( 5000,                                        /*   5000 OS Ticks                                      */
                   OS_OPT_TIME_DLY,                             /*   from now.                                          */
                  &err);
                                                                /*   Check error code.                                  */
        APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);
    }
}
