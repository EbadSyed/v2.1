/*
************************************************************************************************************************
*                                                   Probe Data Types
*
*                                    (c) Copyright 2017; Micrium, Inc.; Weston, FL
*                           All rights reserved.  Protected by international copyright laws.
*
*
* File    : PROBE_TYPES.H
* By      : JPB
* Version : V1.00.00
************************************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                                MODULE
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  _PROBE_TYPES_H_
#define  _PROBE_TYPES_H_


/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <stdint.h>


/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

typedef  uint32_t                  PROBE_BOOLEAN;                             // 32-bit boolean or logical 
typedef  uint8_t                   PROBE_INT08U;                              //  8-bit unsigned integer   
typedef  int8_t                    PROBE_INT08S;                              //  8-bit   signed integer   
typedef  uint16_t                  PROBE_INT16U;                              // 16-bit unsigned integer   
typedef  int16_t                   PROBE_INT16S;                              // 16-bit   signed integer   
typedef  uint32_t                  PROBE_INT32U;                              // 32-bit unsigned integer   
typedef  int32_t                   PROBE_INT32S;                              // 32-bit   signed integer   
typedef  uint64_t                  PROBE_INT64U;                              // 64-bit unsigned integer   
typedef  int64_t                   PROBE_INT64S;                              // 64-bit   signed integer   

typedef  float                     PROBE_FP32;                                // 32-bit floating point     
typedef  double                    PROBE_FP64;                                // 64-bit floating point

typedef  char                      PROBE_CHAR;

#endif
