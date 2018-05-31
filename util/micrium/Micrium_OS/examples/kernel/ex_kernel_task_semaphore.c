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
*                                     KERNEL TASK SEMAPHORE EXAMPLE
*
* File : ex_kernel_task_semaphore.c
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
#include  <rtos/kernel/include/os.h>
#include  <rtos/common/include/common.h>
#include  <rtos/common/include/lib_def.h>
#include  <rtos/common/include/rtos_utils.h>
#include  <rtos/common/include/toolchains.h>
#include  "ex_kernel_task_semaphore.h"


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

#define  EX_KERNEL_RESOURCE_MANAGEMENT_TASK_STK_SIZE     128u
#define  EX_KERNEL_RESOURCE_MANAGEMENT_TASK_PRIO          22u

#define  EX_KERNEL_MANIPULATION_TASK_STK_SIZE            128u
#define  EX_KERNEL_MANIPULATION_TASK_PRIO                 23u


/*
*********************************************************************************************************
*********************************************************************************************************
*                                                DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#define  EX_KERNEL_POST_MAX_LOOP                           3u


/*
*********************************************************************************************************
*********************************************************************************************************
*                                        LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

                                                                /* Resource Management Task Data.                       */
static  CPU_STK  Ex_KernelResourceManagementTaskStk[EX_KERNEL_RESOURCE_MANAGEMENT_TASK_STK_SIZE];
static  OS_TCB   Ex_KernelResourceManagementTaskTCB;

                                                                /* Manipulation Task Data.                              */
static  CPU_STK  Ex_KernelManipulationTaskStk[EX_KERNEL_MANIPULATION_TASK_STK_SIZE];
static  OS_TCB   Ex_KernelManipulationTaskTCB;


/*
*********************************************************************************************************
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  void  Ex_KernelResourceManagementTask(void  *p_arg);
static  void  Ex_KernelManipulationTask      (void  *p_arg);


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
*                                        Ex_KernelTaskSemaphore()
*
* Description : Illustrates the usage of kernel task semaphore functions.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Notes       : None.
*********************************************************************************************************
*/

void  Ex_KernelTaskSemaphore (void)
{
    RTOS_ERR  err;


    OSTaskCreate(&Ex_KernelResourceManagementTaskTCB,           /* Create Resource Management Task.                     */
                 "Ex Kernel Resource Management Task",          /*   Name of the task                                   */
                  Ex_KernelResourceManagementTask,              /*   Pointer to the task implementation                 */
                  DEF_NULL,                                     /*   Task argument                                      */
                  EX_KERNEL_RESOURCE_MANAGEMENT_TASK_PRIO,      /*   Task priority                                      */
                 &Ex_KernelResourceManagementTaskStk[0],        /*   Pointer to the task stack                          */
                                                                /*   Stack limit                                        */
                 (EX_KERNEL_RESOURCE_MANAGEMENT_TASK_STK_SIZE / 10u),
                  EX_KERNEL_RESOURCE_MANAGEMENT_TASK_STK_SIZE,  /*   Stack size                                         */
                  0,                                            /*   The maximum number of messages in the task queue   */
                  0,                                            /*   Time slice in tick for round-robin                 */
                  DEF_NULL,                                     /*   Pointer to user-supplied external TCB data         */
                 (OS_OPT_TASK_STK_CLR),                         /*   Contains additional information about the task     */
                 &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);


    OSTaskSemSet(&Ex_KernelResourceManagementTaskTCB,           /* Pointer to a task                                    */
                  0,                                            /*   Set value of the semaphore                         */
                 &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);


    OSTaskCreate(&Ex_KernelManipulationTaskTCB,                 /* Create Manipulation Task.                            */
                 "Ex Kernel Manipulation Task",                 /*   Name of the task                                   */
                  Ex_KernelManipulationTask,                    /*   Pointer to the task implementation                 */
                  DEF_NULL,                                     /*   Task argument                                      */
                  EX_KERNEL_MANIPULATION_TASK_PRIO,             /*   Task priority                                      */
                 &Ex_KernelManipulationTaskStk[0],              /*   Pointer to the task stack                          */
                                                                /*   Stack limit                                        */
                 (EX_KERNEL_MANIPULATION_TASK_STK_SIZE / 10u),
                  EX_KERNEL_MANIPULATION_TASK_STK_SIZE,         /*   Stack size                                         */
                  0,                                            /*   The maximum number of messages in the task queue   */
                  0,                                            /*   Time slice in tick for round-robin                 */
                  DEF_NULL,                                     /*   Pointer to user-supplied external TCB data         */
                 (OS_OPT_TASK_STK_CLR),                         /*   Contains additional information about the task     */
                 &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Delay Task execution for                             */
    OSTimeDly( 3000,                                            /*   3000 OS Ticks                                      */
               OS_OPT_TIME_DLY,                                 /*   from now.                                          */
              &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
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
*                                   Ex_KernelResourceManagementTask()
*
* Description : This task simply pends on itself.
*
* Argument(s) : p_arg
*
* Return(s)   : None.
*
* Notes       : None.
*********************************************************************************************************
*/

static  void  Ex_KernelResourceManagementTask (void  *p_arg)
{
    RTOS_ERR  err;
    CPU_TS    timestamp;


    PP_UNUSED_PARAM(p_arg);                                     /* Prevent compiler warning.                            */

    while (DEF_ON) {

        EX_TRACE("Pend on task semaphore\r\n");
        OSTaskSemPend( 0,                                       /* Pend on task semaphore                               */
                       OS_OPT_PEND_BLOCKING,                    /*   Blocking pend                                      */
                      &timestamp,                               /*   Time at which the semaphore was posted             */
                      &err);
        if (err.Code == RTOS_ERR_ABORT) {
            EX_TRACE("Pend on task semaphore was aborted\r\n");
        } else {
                                                                /* Handle error(s).                                     */
        }
    }
}


/*
*********************************************************************************************************
*                                      Ex_KernelManipulationTask()
*
* Description : This task Suspend/Resume and delete Ex_KernelTaskManagementTask.
*
* Argument(s) : p_arg
*
* Return(s)   : None.
*
* Notes       : None.
*********************************************************************************************************
*/

static  void  Ex_KernelManipulationTask (void  *p_arg)
{
    RTOS_ERR    err;
    CPU_INT08U  post_counter;


    PP_UNUSED_PARAM(p_arg);                                     /* Prevent compiler warning.                            */

    post_counter = 0;
    while (DEF_ON) {

        if (post_counter < EX_KERNEL_POST_MAX_LOOP) {

            EX_TRACE("Post the task semaphore\r\n");

            OSTaskSemPost(&Ex_KernelResourceManagementTaskTCB,  /* Pointer to a task                                    */
                           OS_OPT_POST_NONE,                    /*   Post the task's semaphore                          */
                          &err);
            APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

            post_counter++;
        } else {

            EX_TRACE("Pend abort the task semaphore\r\n");
                                                                /* Pointer to a task                                    */
            OSTaskSemPendAbort(&Ex_KernelResourceManagementTaskTCB,
                                OS_OPT_POST_NONE,               /*   Abort the task's pend                              */
                               &err);
            APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

            break;
        }
    }


    OSTimeDlyHMSM( 0,                                           /* Delay task for 1 second                              */
                   0,
                   1,
                   0,
                   OS_OPT_TIME_HMSM_STRICT,
                  &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);


    EX_TRACE("Delete Ex_KernelResourceManagementTask task\r\n");
    OSTaskDel(&Ex_KernelResourceManagementTaskTCB,              /* Delete Resource Management Task                      */
              &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);


    EX_TRACE("Delete current task\r\n");
    OSTaskDel( DEF_NULL,                                        /* Delete self                                          */
              &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
}
