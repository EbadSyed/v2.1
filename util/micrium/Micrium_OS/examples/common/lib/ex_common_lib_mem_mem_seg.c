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
* File : ex_common_lib_mem_mem_seg.c
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
#define  EX_MEM_SEG_HW_DATA_SIZE                         128u


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

static  MEM_SEG     ExMemSeg;
static  CPU_INT08U  ExMemSegData[EX_MEM_SEG_DATA_SIZE];         /* Could specify/use particular location for this data. */

static  MEM_SEG     ExMemSegHW;
static  CPU_INT08U  ExMemSegHW_Data[EX_MEM_SEG_HW_DATA_SIZE];   /* Could specify/use particular location for this data. */


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          Ex_CommonLibMemSeg()
*
* Description : Provide examples of MemSegCreate() and several MemSegAlloc...() functions.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_CommonLibMemSeg (void)
{
    RTOS_ERR     err;
    CPU_INT32U  *p_data;
    CPU_REG32   *p_reg;
    CPU_INT08U  *p_buf;
    CPU_INT08U   ix;


                                                                /* Create regular, general-purpose, memory segment.     */
    Mem_SegCreate("Example memory segment",                     /* Name of the memory segment, for debug purposes.      */
                  &ExMemSeg,                                    /* Pointer to MEM_SEG structure.                        */
                  (CPU_ADDR)&(ExMemSegData[0u]),                /* Pointer to data start location.                      */
                  EX_MEM_SEG_DATA_SIZE,                         /* Segment size.                                        */
                  LIB_MEM_PADDING_ALIGN_NONE,                   /* No padding alignment required.                       */
                  &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);


                                                                /* Create memory segment with padding. Could be to ...  */
                                                                /* align on cache lines or to respect constraints  ...  */
                                                                /* from dedicated memory, etc.                          */
    Mem_SegCreate("Hardware example memory segment",            /* Name of the memory segment, for debug purposes.      */
                  &ExMemSegHW,                                  /* Pointer to MEM_SEG structure.                        */
                  (CPU_ADDR)&(ExMemSegHW_Data[0u]),             /* Pointer to data start location.                      */
                   EX_MEM_SEG_HW_DATA_SIZE,                     /* Segment size.                                        */
                   32u,                                         /* 32-bytes padding for each allocated blocks.          */
                  &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);


                                                                /* Allocate block from general-purpose mem seg.         */
                                                                /* Name of the block, for debugging purposes.           */
    p_data = (CPU_INT32U *)Mem_SegAlloc("Example block from example memory segment",
                                        &ExMemSeg,              /* Pointer to MEM_SEG from which to allocate.           */
                                         sizeof(CPU_INT32U),    /* Size of the block to allocate.                       */
                                        &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    *p_data = 0xDEADBEEF;                                       /* Fill with dummy data.                                */

    EX_TRACE("Address: %p\n", p_data);


                                                                /* Alloc block for CPU_INT32U, aligned on 32-bytes, ... */
                                                                /* from the general-purpose memory segment.             */
                                                                /* Name of the block, for debugging purposes.           */
    p_reg = (CPU_INT32U *)Mem_SegAllocExt("Aligned example block from example memory segment",
                                          &ExMemSeg,            /* Pointer to MEM_SEG from which to allocate.           */
                                           sizeof(CPU_INT32U),  /* Size of the block to allocate.                       */
                                           32u,                 /* Alignment for the block to allocate.                 */
                                                                /* Pointer to optional variable indicating how many ... */
                                           DEF_NULL,            /* bytes are missing, if allocation failed.             */
                                          &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    *p_reg = 0xDEADBEEF;                                         /* Fill with dummy data.                                */

    EX_TRACE("Address: %p\n", p_reg);


                                                                /* Alloc block for CPU_INT32U, aligned on 16-bytes  ... */
                                                                /* and the padding specified when creating the segment. */
                                                                /* So if allocated block does not end of a 32-bytes ... */
                                                                /* boundary, the rest will be padded.                   */
                                                                /* Name of the block, for debugging purposes.           */
    p_buf = (CPU_INT08U *)Mem_SegAllocHW("Aligned and padded example block from example memory segment",
                                         &ExMemSegHW,           /* Pointer to MEM_SEG from which to allocate.           */
                                         sizeof(CPU_INT08U) * 9u,/* Size of the block to allocate.                      */
                                         16u,                   /* Alignment for the block to allocate.                 */
                                                                /* Pointer to optional variable indicating how many ... */
                                         DEF_NULL,              /* bytes are missing, if allocation failed.             */
                                         &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    for (ix = 0u; ix < 9u; ++ix) {                              /* Fill with dummy data.                                */
        p_buf[ix] = ix;
    }

    EX_TRACE("Address: %p\n", p_buf);
}
