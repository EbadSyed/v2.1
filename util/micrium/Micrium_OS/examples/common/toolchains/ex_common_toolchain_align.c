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
*                                       COMMON TOOLCHAIN EXAMPLE
*
* File : ex_common_toolchain_align.c
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
#include  <rtos/common/include/toolchains.h>                    /* Include file needed by operation.                    */
#include  <rtos/cpu/include/cpu.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

PP_ALIGN(CPU_INT32U  GlobalAlignedVariable, 64);                /* Variable can be declared globally.                   */


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      Ex_CommonToolchainAlign()
*
* Description : Provides example on how to use the PP_ALIGN macros.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_CommonToolchainAlign (void)
{
    PP_ALIGN(CPU_INT32U  local_aligned_variable, 8);            /* Variable can be declared local to a function.        */


    GlobalAlignedVariable  = 0u;                                /* Use variable(s) without any difference.              */
    local_aligned_variable = 4u;

    /* [...] */

    PP_UNUSED_PARAM(local_aligned_variable);
    return;
}
