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
*                                          COMMON CLK EXAMPLE
*
* File : ex_common_clk_set_manual.c
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

#ifdef RTOS_MODULE_COMMON_CLK_AVAIL

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/clk.h>
#include  <rtos/common/include/lib_def.h>
#include  <rtos/common/include/rtos_utils.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       Ex_CommonClkSetManual()
*
* Description : Provides example on how to use the Clock sub-module to manually set the time.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_CommonClkSetManual (void)
{
    CLK_DATE_TIME  date_time_set;
    CPU_BOOLEAN    is_ok;


                                                                /* Obtain date and time data from user-input or ... */
    is_ok = Clk_DateTimeMake(&date_time_set,                    /* RTC, use it to set clk's internal date and time. */
                             2000u,
                             CLK_MONTH_JAN,
                             01u,
                             7u,
                             8u,
                             9u,
                             CLK_TZ_SEC_FROM_UTC_GET(0));
    APP_RTOS_ASSERT_CRITICAL(is_ok == DEF_OK, ;);


    is_ok = Clk_SetDateTime(&date_time_set);
    APP_RTOS_ASSERT_CRITICAL(is_ok == DEF_OK, ;);
}


#endif /* RTOS_MODULE_COMMON_CLK_AVAIL */
