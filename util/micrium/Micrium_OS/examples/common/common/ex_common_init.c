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
*                                          COMMON INIT EXAMPLE
*
* File : ex_common_init.c
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
#include  <rtos/common/include/common.h>
#include  <rtos/common/include/auth.h>
#include  <rtos/common/include/rtos_err.h>
#include  <rtos/common/include/rtos_utils.h>

#include  <rtos_cfg.h>

#ifdef  RTOS_MODULE_COMMON_CLK_AVAIL
#include  <rtos/common/include/clk.h>
#endif

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
#include  <rtos/common/include/shell.h>
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           Ex_CommonInit()
*
* Description : Provides example on how to initialize the Common module and how to recover the default
*               configuration structure, if needed.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_CommonInit (void)
{
    RTOS_ERR  err;


    Common_Init(&err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
    Shell_Init(&err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
#endif

#ifdef  RTOS_MODULE_COMMON_CLK_AVAIL
    Clk_Init(&err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
#endif

    Auth_Init(&err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
}
