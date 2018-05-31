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
*                                     KERNEL MESSAGE QUEUE EXAMPLE
*
* File : ex_kernel_q.c
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

#if (OS_CFG_Q_EN != DEF_ENABLED)
#error "This example requires OS_CFG_Q_EN to be set to DEF_ENABLED"
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
#define  EX_KERNEL_Q_PEND_TASK_STK_SIZE                  128u
#define  EX_KERNEL_Q_PEND_TASK_PRIO                       22u


/*
*********************************************************************************************************
*                                          EXAMPLE Q SETTINGS
*********************************************************************************************************
*/

                                                                /* Number of pend/posts before deletion.                */
#define  EX_KERNEL_Q_MAX_COUNT                             1u
#define  EX_KERNEL_Q_MAX_MESSAGES                         10u


/*
*********************************************************************************************************
*                                            EXAMPLE Q TYPES
*********************************************************************************************************
*/

#define  EX_KERNEL_Q_TYPE_URGENT                           1u
#define  EX_KERNEL_Q_TYPE_NORMAL                           2u


/*
*********************************************************************************************************
*********************************************************************************************************
*                                              LOCAL TYPES
*********************************************************************************************************
*********************************************************************************************************
*/

typedef struct ex_q_msg {
    CPU_INT08U   type;
    CPU_INT08U  *p_msg;
} EX_Q_MSG;


/*
*********************************************************************************************************
*********************************************************************************************************
*                                        LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

                                                                /* Pend Task Data.                                      */
static  CPU_STK     Ex_KernelQPendTaskStk[EX_KERNEL_Q_PEND_TASK_STK_SIZE];
static  OS_TCB      Ex_KernelQPendTaskTCB;

                                                                /* Message Queue.                                       */
static  OS_Q        Ex_KernelQObj;

                                                                /* Example execution counter.                           */
static  CPU_INT08U  Ex_Done;


/*
*********************************************************************************************************
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  void  Ex_KernelQPendTask(void  *p_arg);


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
*                                           Ex_KernelQ()
*
* Description : Illustrates the usage of kernel message queues.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Notes       : None.
*********************************************************************************************************
*/

void  Ex_KernelQ (void)
{
    RTOS_ERR    err;
    OS_OBJ_QTY  qty;
    EX_Q_MSG    ex_msg1;
    EX_Q_MSG    ex_msg2;


                                                                /* Initialize Example.                                  */
    Ex_Done = 0u;

                                                                /* Create Message Queue.                                */
    OSQCreate(&Ex_KernelQObj,                                   /*   Pointer to the example message queue.              */
              "Ex Kernel Message Queue",                        /*   Name used for debugging.                           */
               EX_KERNEL_Q_MAX_MESSAGES,                        /*   Maximum number of messages in queue.               */
              &err);
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);

    OSTaskCreate(&Ex_KernelQPendTaskTCB,                        /* Create Pend Task.                                    */
                 "Ex Kernel Message Queue Pend Task",
                  Ex_KernelQPendTask,
                  0,
                  EX_KERNEL_Q_PEND_TASK_PRIO,
                 &Ex_KernelQPendTaskStk[0],
                 (EX_KERNEL_Q_PEND_TASK_STK_SIZE / 10u),
                  EX_KERNEL_Q_PEND_TASK_STK_SIZE,
                  0,
                  0,
                  0,
                 (OS_OPT_TASK_STK_CLR),
                 &err);
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);

                                                                /* Post two messages to the message queue.              */

    ex_msg1.type   = EX_KERNEL_Q_TYPE_URGENT;
    ex_msg1.p_msg  = (CPU_INT08U *)"Example Kernel Q Urgent Message";
                                                                /* Post Urgent Message.                                 */
    OSQPost(&Ex_KernelQObj,                                     /*   Pointer to the example message queue.              */
            (void *)&ex_msg1,                                   /*   The message is a pointer to the EX_Q_MSG.          */
            (OS_MSG_SIZE)sizeof(void *),                        /*   Size of the message is the size of a pointer.      */
             OS_OPT_POST_FIFO,                                  /*   Add message at the end of the queue.               */
            &err);
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);

    ex_msg2.type   = EX_KERNEL_Q_TYPE_NORMAL;
    ex_msg2.p_msg  = (CPU_INT08U *)"Example Kernel Q Normal Message";
                                                                /* Post Normal Message.                                 */
    OSQPost(&Ex_KernelQObj,                                     /*   Pointer to the example message queue.              */
            (void *)&ex_msg2,                                   /*   The message is a pointer to the EX_Q_MSG.          */
            (OS_MSG_SIZE)sizeof(void *),                        /*   Size of the message is the size of a pointer.      */
             OS_OPT_POST_FIFO,                                  /*   Add message at the end of the queue.               */
            &err);
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);

                                                                /* Ex_KernelQPendTask will execute.                     */

                                                                /* Delay Task execution for                             */
    OSTimeDly( 500,                                             /*   500 OS Ticks                                       */
               OS_OPT_TIME_DLY,                                 /*   from now.                                          */
              &err);

                                                                /* Ex_KernelQPendTask has blocked, execution            */
                                                                /* returns here.                                        */

                                                                /* Empty the message queue.                             */
    qty = OSQFlush(&Ex_KernelQObj,                              /*   Pointer to the example message queue.              */
                   &err);
    if (err.Code == RTOS_ERR_NONE) {                            /*   Check error code.                                  */
        if (qty == 1) {
            Ex_Done = 1u;
            EX_TRACE("Example Kernel Q: Ex_KernelQ successfully flushed the message queue.\r\n");
        } else {
            EX_TRACE("Example Kernel Q: wrong number of messages flushed. OSQFlush() flushed %i messages\r\n",
                      qty);
            RTOS_ASSERT_CRITICAL_FAILED_END_CALL(;);
        }
    } else {
        EX_TRACE("Example Kernel Q: error on flush. OSQFlush() returned with %i\r\n",
                  err.Code);
        RTOS_ASSERT_CRITICAL_FAILED_END_CALL(;);
    }

                                                                /* Delete Message Queue.                                */
    qty = OSQDel(&Ex_KernelQObj,                                /*   Pointer to the example message queue.              */
                  OS_OPT_DEL_NO_PEND,                           /*   Option: delete if 0 tasks are pending.             */
                 &err);
                                                                /*   Check error code.                                  */
    APP_RTOS_ASSERT_DBG((err.Code == RTOS_ERR_NONE), ;);
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
*                                        Ex_KernelQPendTask()
*
* Description : This task pends on the message queue.
*
* Argument(s) : p_arg
*
* Return(s)   : None.
*
* Notes       : None.
*********************************************************************************************************
*/

static  void  Ex_KernelQPendTask (void  *p_arg)
{
    RTOS_ERR      err;
    void         *p_raw_msg;
    OS_MSG_SIZE   msg_size;
    EX_Q_MSG     *p_msg;


                                                                /* Prevent compiler warning.                            */
    PP_UNUSED_PARAM(p_arg);

                                                                /* Task body.                                           */
    while (DEF_ON) {
        if (Ex_Done < EX_KERNEL_Q_MAX_COUNT) {
                                                                /* Get message from queue.                              */
            p_raw_msg = OSQPend(&Ex_KernelQObj,                 /*   Pointer to the example message queue.              */
                                 100,                           /*   Wait for 100 OS Ticks maximum.                     */
                                 OS_OPT_PEND_BLOCKING,          /*   Task will block.                                   */
                                &msg_size,                      /*   Will contain size of message in bytes.             */
                                 DEF_NULL,                      /*   Timestamp is not used.                             */
                                &err);
            if (err.Code == RTOS_ERR_NONE) {
                p_msg = (EX_Q_MSG *)p_raw_msg;
                EX_TRACE("Example Kernel Q: Pend task got message of type %i: '%s'.\r\n",
                         p_msg->type,
                         p_msg->p_msg);
            } else {
                EX_TRACE("Example Kernel Q: unable to pend on message queue. OSQPend() returned with %i\r\n",
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
