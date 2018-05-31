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
* File : ex_common_lib_mem_mem_seg_info.c
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
#define  EX_LIB_MEM_SEG_INFO_ALIGNMENT                     8u


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


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        Ex_CommonLibMemSegInfo()
*
* Description : Provide example of MemSegCreate() and Mem_SegRemSizeGet() functions.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_CommonLibMemSegInfo (void)
{
    RTOS_ERR       err;
    MEM_SEG_INFO   info;
    CPU_SIZE_T     rem_size;
    CPU_INT32U    *p_alloc_data;


                                                                /* Create regular, general-purpose, memory segment.     */
    Mem_SegCreate("Example memory segment",                     /* Name of the memory segment, for debug purposes.      */
                  &ExMemSeg,                                    /* Pointer to MEM_SEG structure.                        */
                  (CPU_ADDR)&(ExMemSegData[0u]),                /* Pointer to data start location.                      */
                   EX_MEM_SEG_DATA_SIZE,                        /* Segment size.                                        */
                   LIB_MEM_PADDING_ALIGN_NONE,                  /* No padding alignment required.                       */
                  &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Allocate block from general-purpose mem seg.         */
                                                                /* name of the block, for debugging purposes,           */
    p_alloc_data = (CPU_INT32U *)Mem_SegAlloc("Example block from example memory segment",
                                              &ExMemSeg,        /* pointer to MEM_SEG from which to allocate,           */
                                                                /* size of the block to allocate.                       */
                                               sizeof(CPU_INT32U),
                                              &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

   *p_alloc_data = 0xDEADBEEF;                                  /* Fill with dummy data.                                */

    rem_size = Mem_SegRemSizeGet(&ExMemSeg,
                                  EX_LIB_MEM_SEG_INFO_ALIGNMENT,/* Alignment in bytes.                                  */
                                 &info,
                                 &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    EX_TRACE("Base address: %d\nTotal size: %d\nUsed size: %d\n", info.AddrBase, info.TotalSize, info.UsedSize);

    EX_TRACE("Resize: %d\n", rem_size);
}
