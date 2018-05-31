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
* File : ex_common_lib_mem_mem_seg_compile.c
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

#define  EX_LIB_MEM_SEG_COMPILE_SIZE                    2048u


/*
*********************************************************************************************************
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

static  CPU_INT08U  ExMemSegData[EX_LIB_MEM_SEG_COMPILE_SIZE];
static  MEM_SEG     ExMemSeg = MEM_SEG_INIT("Example compile-time memory segment",
                                            (CPU_ADDR)&ExMemSegData[0u],
                                             EX_LIB_MEM_SEG_COMPILE_SIZE,
                                             LIB_MEM_PADDING_ALIGN_NONE);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      Ex_CommonLibMemSegCompile()
*
* Description : Provide example of Mem_SegReg() function.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : The memory segment created at compile-time need to be register, to let LIB Mem know of
*               its existence.
*********************************************************************************************************
*/

void  Ex_CommonLibMemSegCompile (void)
{
    RTOS_ERR  err;


                                                                /* Memory segment created at compile-time and ...       */
    Mem_SegReg(&ExMemSeg,                                       /* registered a run-time.                               */
               &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
}
