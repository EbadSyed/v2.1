/*
*********************************************************************************************************
*                            UC/PROBE TERMINAL WINDOW OPERATING SYSTEM LAYER
*                                         Micrium OS Kernel
*
*                             (c) Copyright 2017; Micrium, Inc.; Weston, FL
*                    All rights reserved.  Protected by international copyright laws.
*
*
* File    : PROBE_TERM_OS.H
* By      : JPB
* Version : V1.00.00
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <rtos/cpu/include/cpu.h>


/*
*********************************************************************************************************
*                                        CRITICAL SECTIONS
*********************************************************************************************************
*/

#define  PROBE_CPU_SR_ALLOC()          CPU_SR_ALLOC()
#define  PROBE_CPU_CRITICAL_ENTER()    CPU_CRITICAL_ENTER()
#define  PROBE_CPU_CRITICAL_EXIT()     CPU_CRITICAL_EXIT()