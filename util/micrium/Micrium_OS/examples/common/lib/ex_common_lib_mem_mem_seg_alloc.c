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
* File : ex_common_lib_mem_mem_seg_alloc.c
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
#define  EX_LIB_MEM_SEG_ALLOC_NUM_ELEMENT                  2u
#define  EX_LIB_MEM_SEG_ALLOC_MAX_ELEMENT                  6u


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
*                                        Ex_CommonLibMemDynPool()
*
* Description : Provide examples of MemSegCreate(), Mem_SegAllocExt() and Mem_DynPoolCreate() functions.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_CommonLibMemDynPool (void)
{
    RTOS_ERR     err;
    CPU_INT32U  *p_alloc_data;
    CPU_INT32U  *p_alloc_ext_data;
    CPU_SIZE_T   free_bytes;
    CPU_INT08U   i;


                                                                /* Create regular, general-purpose, memory segment.     */
    Mem_SegCreate("Example memory segment",                     /* Name of the memory segment, for debug purposes.      */
                  &ExMemSeg,                                    /* Pointer to MEM_SEG structure.                        */
                  (CPU_ADDR)&(ExMemSegData[0u]),                /* Pointer to data start location.                      */
                   EX_MEM_SEG_DATA_SIZE,                        /* Segment size.                                        */
                   LIB_MEM_PADDING_ALIGN_NONE,                  /* No padding alignment required.                       */
                  &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);


                                                                /* Allocate block from general-purpose mem seg.         */
                                                                /* Name of the block, for debugging purposes.           */
    p_alloc_data = (CPU_INT32U *)Mem_SegAlloc("Example block from example memory segment",
                                              &ExMemSeg,        /* Pointer to MEM_SEG from which to allocate.           */
                                               sizeof(CPU_INT32U), /* Size of the block to allocate.                    */
                                              &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    EX_TRACE("Address: %p\n", p_alloc_data);

    *p_alloc_data = 0xDEADBEEF;                                 /* Fill with dummy data.                            */

                                                                /* Allocate block from general-purpose mem seg.     */
                                                                /* Name of the block, for debugging purposes.       */
    p_alloc_ext_data = (CPU_INT32U *)Mem_SegAllocExt("Example block from example memory segment",
                                                                /* Pointer to MEM_SEG from which to allocate.       */
                                                      &ExMemSeg,
                                                                /* Size of the block to allocate.                   */
                                                       sizeof(CPU_INT32U) * 8u,
                                                                /* Aligment for memory block.                       */
                                                      (CPU_SIZE_T  ) 8u,
                                                                /* Free bytes missing for allocation.               */
                                                      &free_bytes,
                                                      &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    EX_TRACE("Address: %p\n", p_alloc_ext_data);

    for(i = 0; i < 8u; i ++){
       p_alloc_ext_data[i] = (CPU_INT32U)i;                     /* Fill with dummy data.                                */
    }

                                                                /* Create a dynamic memory pool.                        */
                                                                /* Name of the block, for debugging purposes.           */
    Mem_DynPoolCreate("Example dyn pool from example memory segment",
                      &ExMemPool,                               /* Pointer to the pool data.                            */
                      &ExMemSeg,                                /* Pointer to MEM_SEG from which to allocate.           */
                       sizeof(CPU_INT64U),                      /* Size of memory block to allocate from pool.          */
                       sizeof(CPU_ALIGN),                       /* Alignment of memory blocks.                          */
                       EX_LIB_MEM_SEG_ALLOC_NUM_ELEMENT,        /* Initial number of blocks to allocate.                */
                       EX_LIB_MEM_SEG_ALLOC_MAX_ELEMENT,        /* Maximum number of blocks that can be allocated.      */
                      &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
}
