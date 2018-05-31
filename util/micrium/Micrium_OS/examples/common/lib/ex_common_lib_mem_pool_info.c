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
* File : ex_common_lib_mem_pool_info.c
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
#define  EX_LIB_MEM_POOL_INFO_NUM_ELEMENT                  2u
#define  EX_LIB_MEM_POOL_INFO_MAX_ELEMENT                  6u

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
*                                      Ex_CommonLibMemDynPoolInfo()
*
* Description : Provide examples of MemSegCreate(), Mem_SegAllocExt() and Mem_DynPoolCreate()  functions.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_CommonLibMemDynPoolInfo (void)
{
    RTOS_ERR           err;
    MEM_DYN_POOL_INFO  info;
    CPU_SIZE_T         rem_blk_nbr;


                                                                /* Create regular, general-purpose, memory segment.     */
    Mem_SegCreate("Example memory segment",                     /* Name of the memory segment, for debug purposes.      */
                  &ExMemSeg,                                    /* Pointer to MEM_SEG structure.                        */
                  (CPU_ADDR)&(ExMemSegData[0u]),                /* Pointer to data start location.                      */
                   EX_MEM_SEG_DATA_SIZE,                        /* Segment size.                                        */
                   LIB_MEM_PADDING_ALIGN_NONE,                  /* No padding alignment required.                       */
                  &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Create a dynamic memory pool.                        */
                                                                /* Name of the block, for debugging purposes.           */
    Mem_DynPoolCreate("Example dyn pool from example memory segment",
                      &ExMemPool,                               /* Pointer to the pool data.                            */
                      &ExMemSeg,                                /* Pointer to MEM_SEG from which to allocate.           */
                       sizeof(CPU_INT64U),                      /* Size of memory block to allocate from pool.          */
                       sizeof(CPU_ALIGN),                       /* Alignment of memory blocks.                          */
                       EX_LIB_MEM_POOL_INFO_NUM_ELEMENT,        /* Number of elements to be allocated from the pool.    */
                       EX_LIB_MEM_POOL_INFO_MAX_ELEMENT,        /* Maximum elements to be allocated from the pool.      */
                      &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    rem_blk_nbr = Mem_DynPoolBlkNbrAvailGet(&ExMemPool,         /* Pointer to the pool data.                            */
                                       &info,                   /* Pointer to a MEM_DYN_POOL_INFO structure.            */
                                       &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    EX_TRACE("Resize: %d \nBlank quantity max: %d\nNumber of block available: %d\n",
              rem_blk_nbr,
              info.BlkQtyMax,
              info.BlkNbrAvailCnt);

                                                                /* Name of the block, for debugging purposes.           */
    Mem_DynPoolCreate("Example block from example memory segment",
                      &ExMemPool,                               /* Pointer to the pool data.                            */
                      &ExMemSeg,                                /* Pointer to MEM_SEG from which to allocate.           */
                       sizeof(CPU_INT64U),                      /* Size of memory block to allocate from pool.          */
                       sizeof(CPU_ALIGN),                       /* Alignment of memory blocks.                          */
                       EX_LIB_MEM_POOL_INFO_NUM_ELEMENT,        /* Number of elements to be allocated from the pool.    */
                       LIB_MEM_BLK_QTY_UNLIMITED,               /* Maximum elements to be allocated from the pool.      */
                      &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    rem_blk_nbr = Mem_DynPoolBlkNbrAvailGet(&ExMemPool,         /* Pointer to the pool data.                            */
                                            &info,              /* Pointer to a MEM_DYN_POOL_INFO structure.            */
                                            &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    EX_TRACE("Resize: %d \nBlank quantity max: %d\nNumber of block available: %d\n",
              rem_blk_nbr,
              info.BlkQtyMax,
              info.BlkNbrAvailCnt);
}
