/*
 *********************************************************************************************************
 *                                            EXAMPLE CODE
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
 *                                         KERNEL CONFIGURATION
 *
 *                                      CONFIGURATION TEMPLATE FILE
 *
 * Filename      : os_cfg.h
 * Programmer(s) : Micrium
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                               MODULE
 *********************************************************************************************************
 *********************************************************************************************************
 */

#ifndef  _OS_CFG_H_
#define  _OS_CFG_H_

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                             INCLUDE FILES
 *********************************************************************************************************
 *********************************************************************************************************
 */

#include  <rtos/common/include/lib_def.h>

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                            MISCELLANEOUS
 *
 * Note(s) : (1) Configure OS_CFG_APP_HOOKS_EN to enable or disable Application-specific Hooks.
 *
 *           (2) Configure OS_CFG_COMPAT_INIT_EN to enable or disable old 'os_app_cfg.h'
 *               Application-specific configuration.
 *
 *           (3) Configure OS_CFG_DBG_EN to enable or disable debug helper code and variables.
 *
 *           (4) Configure OS_CFG_DYN_TICK_EN to enable or disable the Dynamic Tick feature.
 *
 *           (5) Configure OS_CFG_TS_EN to enable or disable Timestamping capabilities.
 *
 *           (6) Configure OS_CFG_PRIO_MAX to set the maximum number of Task Priorities (see OS_PRIO data
 *               type).
 *
 *           (7) Configure OS_CFG_SCHED_LOCK_TIME_MEAS_EN to enable or disable the Scheduler Lock time
 *               measurement code.
 *
 *           (8) Configure OS_CFG_SCHED_ROUND_ROBIN_EN to enable or disable the Round-Robin Scheduler.
 *
 *           (9) Configure OS_CFG_STK_SIZE_MIN to set the minimum allowable Task Stack size (in CPU_STK
 *               elements).
 *********************************************************************************************************
 *********************************************************************************************************
 */

#define  OS_CFG_APP_HOOKS_EN                                DEF_DISABLED

#define  OS_CFG_COMPAT_INIT_EN                              DEF_DISABLED

#define  OS_CFG_DBG_EN                                      DEF_ENABLED

#define  OS_CFG_DYN_TICK_EN                                 DEF_DISABLED

#define  OS_CFG_TS_EN                                       DEF_ENABLED

#define  OS_CFG_PRIO_MAX                                    64u

#define  OS_CFG_SCHED_LOCK_TIME_MEAS_EN                     DEF_DISABLED

#define  OS_CFG_SCHED_ROUND_ROBIN_EN                        DEF_ENABLED

#define  OS_CFG_STK_SIZE_MIN                                64u

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                             EVENT FLAGS
 *
 * Note(s) : (1) Configure OS_CFG_FLAG_EN to enable or disable the Event Flags synchronization construct.
 *
 *           (2) Configure OS_CFG_FLAG_DEL_EN to enable or disable the deletion of Event Flag objects.
 *
 *           (3) Configure OS_CFG_FLAG_MODE_CLR_EN to enable or disable the active-low mode of Event
 *               Flags.
 *
 *           (4) Configure OS_CFG_FLAG_PEND_ABORT_EN to enable or disable the pend abort feature of Event
 *               Flag objects.
 *********************************************************************************************************
 *********************************************************************************************************
 */

#define  OS_CFG_FLAG_EN                                     DEF_ENABLED

#define  OS_CFG_FLAG_DEL_EN                                 DEF_ENABLED

#define  OS_CFG_FLAG_MODE_CLR_EN                            DEF_DISABLED

#define  OS_CFG_FLAG_PEND_ABORT_EN                          DEF_DISABLED

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                          MEMORY MANAGEMENT
 *
 * Note(s) : (1) Configure OS_CFG_MEM_EN to enable or disable the built-in Memory Manager.
 *********************************************************************************************************
 *********************************************************************************************************
 */

#define  OS_CFG_MEM_EN                                      DEF_DISABLED

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                     MUTUAL EXCLUSION SEMAPHORES
 *
 * Note(s) : (1) Configure OS_CFG_MUTEX_EN to enable or disable the Mutual Exclusion (Mutex)
 *               synchronization construct.
 *
 *           (2) Configure OS_CFG_MUTEX_DEL_EN to enable or disable the deletion of Mutex objects.
 *
 *           (3) Configure OS_CFG_MUTEX_PEND_ABORT_EN to enable or disable the pend abort feature of
 *               Mutex objects.
 *********************************************************************************************************
 *********************************************************************************************************
 */

#define  OS_CFG_MUTEX_EN                                    DEF_ENABLED

#define  OS_CFG_MUTEX_DEL_EN                                DEF_ENABLED

#define  OS_CFG_MUTEX_PEND_ABORT_EN                         DEF_DISABLED

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                            MESSAGE QUEUES
 *
 * Note(s) : (1) Configure OS_CFG_Q_EN to enable or disable the Message Queue construct.
 *
 *           (2) Configure OS_CFG_Q_DEL_EN to enable or disable the deletion of Message Queues.
 *
 *           (3) Configure OS_CFG_Q_FLUSH_EN to enable or disable the Message Queue flush feature.
 *
 *           (4) Configure OS_CFG_Q_PEND_ABORT_EN to enable or disable the pend abort feature of Message
 *               Queues.
 *********************************************************************************************************
 *********************************************************************************************************
 */

#define  OS_CFG_Q_EN                                        DEF_ENABLED

#define  OS_CFG_Q_DEL_EN                                    DEF_ENABLED

#define  OS_CFG_Q_FLUSH_EN                                  DEF_ENABLED

#define  OS_CFG_Q_PEND_ABORT_EN                             DEF_ENABLED

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                              SEMAPHORES
 *
 * Note(s) : (1) Configure OS_CFG_SEM_EN to enable or disable the Semaphore synchronization construct.
 *
 *           (2) Configure OS_CFG_SEM_DEL_EN to enable or disable the deletion of Semaphore objects.
 *
 *           (3) Configure OS_CFG_SEM_PEND_ABORT_EN to enable or disable the pend abort feature of
 *               Semaphore objects.
 *
 *           (4) Configure OS_CFG_SEM_SET_EN to enable or disable the ability to explicitly set a
 *               Semaphore's value.
 *********************************************************************************************************
 *********************************************************************************************************
 */

#define  OS_CFG_SEM_EN                                      DEF_ENABLED

#define  OS_CFG_SEM_DEL_EN                                  DEF_ENABLED

#define  OS_CFG_SEM_PEND_ABORT_EN                           DEF_ENABLED

#define  OS_CFG_SEM_SET_EN                                  DEF_ENABLED

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                               MONITORS
 *
 * Note(s) : (1) Configure OS_CFG_MON_EN to enable or disable the Monitor (condition variable)
 *               synchronization construct.
 *
 *           (2) Configure OS_CFG_MON_DEL_EN to enable or disable the deletion of Monitor objects.
 *********************************************************************************************************
 *********************************************************************************************************
 */

#define  OS_CFG_MON_EN                                      DEF_ENABLED

#define  OS_CFG_MON_DEL_EN                                  DEF_ENABLED

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                           TASK MANAGEMENT
 *
 * Note(s) : (1) Configure OS_CFG_STAT_TASK_EN to enable or disable the Statistics gathering Task.
 *
 *           (2) Configure OS_CFG_STAT_TASK_STK_CHK_EN to enable or disable the stack overflow detection
 *               of the Statistics Task.
 *
 *           (3) Configure OS_CFG_TASK_DEL_EN to enable or disable the deletion of Tasks.
 *
 *           (4) Configure OS_CFG_TASK_IDLE_EN to enable or disable the Idle Task.
 *
 *           (5) Configure OS_CFG_TASK_PROFILE_EN to enable or disable Task profiling instrumentation.
 *
 *           (6) Configure OS_CFG_TASK_Q_EN to enable or disable built-in Task Message Queues.
 *
 *           (7) Configure OS_CFG_TASK_Q_PEND_ABORT_EN to enable or disable the pend abort feature of
 *               Task Message Queues.
 *
 *           (8) Configure OS_CFG_TASK_REG_TBL_SIZE to set the number of Task Registers.
 *
 *           (9) Configure OS_CFG_TASK_STK_REDZONE_EN to enable or disable the Redzone Stack Protection.
 *
 *           (10) Configure OS_CFG_TASK_STK_REDZONE_DEPTH to set the depth of the Redzone Stack
 *               Protection.
 *
 *           (11) Configure OS_CFG_TASK_SEM_PEND_ABORT_EN to enable or disable the pend abort feature of
 *               Task Semaphores.
 *
 *           (12) Configure OS_CFG_TASK_SUSPEND_EN to enable or disable the suspension of Tasks.
 *
 *           (13) Configure OS_CFG_TASK_TICK_EN to enable or disable the Tick Task.
 *********************************************************************************************************
 *********************************************************************************************************
 */

#define  OS_CFG_STAT_TASK_EN                                DEF_ENABLED

#define  OS_CFG_STAT_TASK_STK_CHK_EN                        DEF_ENABLED

#define  OS_CFG_TASK_DEL_EN                                 DEF_ENABLED

#define  OS_CFG_TASK_IDLE_EN                                DEF_ENABLED

#define  OS_CFG_TASK_PROFILE_EN                             DEF_ENABLED

#define  OS_CFG_TASK_Q_EN                                   DEF_ENABLED

#define  OS_CFG_TASK_Q_PEND_ABORT_EN                        DEF_DISABLED

#define  OS_CFG_TASK_REG_TBL_SIZE                           3u

#define  OS_CFG_TASK_STK_REDZONE_EN                         DEF_ENABLED

#define  OS_CFG_TASK_STK_REDZONE_DEPTH                      8u

#define  OS_CFG_TASK_SEM_PEND_ABORT_EN                      DEF_ENABLED

#define  OS_CFG_TASK_SUSPEND_EN                             DEF_ENABLED

#define  OS_CFG_TASK_TICK_EN                                DEF_ENABLED

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                    TASK LOCAL STORAGE MANAGEMENT
 *
 * Note(s) : (1) Configure OS_CFG_TLS_TBL_SIZE to set the number of Task Local Storage registers.
 *********************************************************************************************************
 *********************************************************************************************************
 */

#define  OS_CFG_TLS_TBL_SIZE                                0u

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                           TIME MANAGEMENT
 *
 * Note(s) : (1) Configure OS_CFG_TIME_DLY_HMSM_EN to enable or disable the human-readable (HH:MM:SS.mm)
 *               time delay feature.
 *
 *           (2) Configure OS_CFG_TIME_DLY_RESUME_EN to enable or disable the cancel delay feature.
 *********************************************************************************************************
 *********************************************************************************************************
 */

#define  OS_CFG_TIME_DLY_HMSM_EN                            DEF_ENABLED

#define  OS_CFG_TIME_DLY_RESUME_EN                          DEF_ENABLED

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                           TIMER MANAGEMENT
 *
 * Note(s) : (1) Configure OS_CFG_TMR_EN to enable or disable Software Timers.
 *
 *           (2) Configure OS_CFG_TMR_DEL_EN to enable or disable the deletion of Software Timers.
 *********************************************************************************************************
 *********************************************************************************************************
 */

#define  OS_CFG_TMR_EN                                      DEF_ENABLED

#define  OS_CFG_TMR_DEL_EN                                  DEF_ENABLED

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                           Trace Recorders
 *
 * Note(s) : (1) Configure OS_CFG_TRACE_EN to enable or disable the kernel events trace recorder.
 *********************************************************************************************************
 *********************************************************************************************************
 */

#define  OS_CFG_TRACE_EN                                    DEF_ENABLED

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                             MODULE END
 *********************************************************************************************************
 *********************************************************************************************************
 */

#endif /* End of os_cfg.h module include.                      */
