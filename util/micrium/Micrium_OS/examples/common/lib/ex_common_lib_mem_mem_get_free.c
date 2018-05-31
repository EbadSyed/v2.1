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
*                                       COMMON LIB MEM EXAMPLE
*
* File : ex_common_lib_mem_mem_get_free.c
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos_description.h>
#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/lib_mem.h>
#include  <rtos/common/include/rtos_utils.h>
#include  <rtos/common/include/rtos_err.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#define  EX_MEM_SEG_DATA_SIZE                            512u


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
#define  EX_TRACE(...)                                  printf(__VA_ARGS__)
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

static  MEM_SEG       ExMemSeg;
static  CPU_INT08U    ExMemSegData[EX_MEM_SEG_DATA_SIZE];       /* Could specify/use particular location for this data. */
static  MEM_DYN_POOL  ExMemPool;


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      Ex_CommonLibMemMemGetFree()
*
* Description : Provide examples of Mem_DynPoolCreate(), Mem_DynPoolBlkGet() and Mem_DynPoolBlkFree()
*               functions.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_CommonLibMemMemGetFree (void)
{
    RTOS_ERR     err;
    CPU_INT64U  *p_data_get;


                                                                /* Create regular, general-purpose, memory segment.     */
    Mem_SegCreate("Example memory segment",                     /* Name of the memory segment, for debug purposes.      */
                  &ExMemSeg,                                    /* Pointer to MEM_SEG structure.                        */
                  (CPU_ADDR)&(ExMemSegData[0u]),                /* Pointer to data start location.                      */
                   EX_MEM_SEG_DATA_SIZE,                        /* Segment size.                                        */
                   LIB_MEM_PADDING_ALIGN_NONE,                  /* No padding alignment required.                       */
                  &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Create a dynamic memory pool.                        */
    Mem_DynPoolCreate("Example dynamic pool",                   /* Name of the block, for debugging purposes.           */
                      &ExMemPool,                               /* Pointer to the pool data.                            */
                      &ExMemSeg,                                /* Pointer to MEM_SEG from which to allocate.           */
                       sizeof(CPU_INT64U),                      /* Size of memory block to allocate from pool.          */
                       sizeof(CPU_ALIGN),                       /* Allignment of memory blocks.                         */
                       1u,                                      /* Number of elements to be allocated from the pool.    */
                       LIB_MEM_BLK_QTY_UNLIMITED,               /* Maximum elements to be allocated from the pool.      */
                      &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Pointer to the pool data.                            */
    p_data_get = (CPU_INT64U *)Mem_DynPoolBlkGet(&ExMemPool, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    *p_data_get = 0xDEADBEEFFFFF0000;                           /* Fill with dummy data.                                */

    EX_TRACE("Address: %p Value: %llX", p_data_get, *p_data_get);

    Mem_DynPoolBlkFree(&ExMemPool,                              /* Pointer to the pool data.                            */
                       (void *)p_data_get,                      /* Pointer to first byte of memory block.               */
                       &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Pointer to the pool data.                            */
    p_data_get = (CPU_INT64U *)Mem_DynPoolBlkGet(&ExMemPool, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    EX_TRACE("Address: %p Value: %llX", p_data_get, *p_data_get);
}
