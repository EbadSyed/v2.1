/*
*********************************************************************************************************
*                                              uC/Probe
*                                             Live Watch
*                                           Terminal Window
*
*                         (c) Copyright 2004-2013; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                             UC/PROBE TERMINAL WINDOW CONFIGURATION FILE
*
*                                              TEMPLATE
*
* File          : probe_term_cfg.h
* Version       : V1.00.00
* Programmer(s) : JPB
*
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) This uC/Probe terminal window configuration header file is protected from multiple
*               pre-processor inclusion through the use of the pre-processor macro definition
*               PROBE_TERM_CFG_MODULE_PRESENT.
*********************************************************************************************************
*/

#ifndef  PROBE_TERM_CFG_MODULE_PRESENT                          /* See Note #1.                                         */
#define  PROBE_TERM_CFG_MODULE_PRESENT


/*
*********************************************************************************************************
*                             UC/PROBE TERMINAL WINDOW GENERIC CONFIGURATION
*********************************************************************************************************
*/
                                                                /* ------------- COMMAND LINE INTERFACE --------------- */
#define  PROBE_TERM_CFG_CMD_EN                        1         /* Interface enable.                                    */
#define  PROBE_TERM_OS_CFG_CMD_RX_TASK_STK_SIZE     128         /* OS task stack size.                                  */
#define  PROBE_TERM_OS_CFG_CMD_RX_TASK_PRIO          10         /* OS task priority.                                    */
#define  PROBE_TERM_OS_CFG_CMD_TX_TASK_STK_SIZE     128         /* OS task stack size.                                  */
#define  PROBE_TERM_OS_CFG_CMD_TX_TASK_PRIO          11         /* OS task priority.                                    */
                                                                /* ----------------- TRACE INTERFACE ------------------ */
#define  PROBE_TERM_CFG_TRC_EN                        1         /* Interface enable.                                    */
#define  PROBE_TERM_OS_CFG_TRC_TASK_STK_SIZE        128         /* OS task stack size.                                  */
#define  PROBE_TERM_OS_CFG_TRC_TASK_PRIO             12         /* OS task priority.                                    */

#define  PROBE_TERM_CFG_BUF_SIZE                     64         /* Max size of the Rx and Tx message arrays.            */
#define  PROBE_TERM_CFG_Q_SIZE                       16         /* Max number of message arrays in the queues.          */

#define  PROBE_TERM_OS_CFG_TASK_DLY_MSEC            100         /* OS task delay in milliseconds to yield the CPU.      */


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif
