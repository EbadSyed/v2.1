/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can find our product's user manual, API reference, release notes and
*               more information at https://doc.micrium.com.
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      APPLICATION CONFIGURATION
*
*                                            EXAMPLE CODE
*
*                                            SILICON LABS
*                                         EFM32PG_SLSTK3402A
*
* Filename : app_cfg.h
*********************************************************************************************************
*/

#ifndef  _APP_CFG_H_
#define  _APP_CFG_H_


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <stdarg.h>
#include  <stdio.h>


/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#define  APP_CFG_TASK_START_PRIO                  3u
#define  APP_CFG_TASK_WIFI_PRIO                   4u
#define  APP_CFG_TASK_LCD_PRIO                    5u


/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  APP_CFG_TASK_START_STK_SIZE             200u
#define  APP_CFG_TASK_WIFI_STK_SIZE              512u
#define  APP_CFG_TASK_LCD_STK_SIZE               256u


/*
*********************************************************************************************************
*                                       PRINTF CONFIGURATION
*********************************************************************************************************
*/

#define  APP_PRINTF_OFF                           0u
#define  APP_PRINTF_ON                            1u

#define  APP_PRINTF_CFG                      APP_PRINTF_OFF

#if (OS_CFG_TRACE_EN == DEF_ENABLED)
#define  APP_PRINTF_FX                  SEGGER_SYSVIEW_PrintfHost
#else
#define  APP_PRINTF_FX                          printf
#endif

#define  APP_PRINTF(x)     ((APP_PRINTF_CFG) ? (void)(APP_PRINTF_FX x) : (void)0)

#endif
