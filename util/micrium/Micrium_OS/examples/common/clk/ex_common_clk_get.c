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
* File : ex_common_clk_get.c
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
*                                            LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

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
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           Ex_CommonClkGet()
*
* Description : Provides example on how to use the Clock sub-module of Common to get time.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_CommonClkGet (void)
{
    CLK_DATE_TIME         date_time_set;
    CLK_DATE_TIME         date_time_get;
    CLK_DAY               day_of_wk;
    CLK_DAY               day_of_yr;
    CPU_BOOLEAN           is_ok;
    volatile  CPU_INT32U  i = 0u;


    is_ok = Clk_DateTimeMake(&date_time_set,
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

    day_of_wk = Clk_GetDayOfWk(2000u,
                               CLK_MONTH_JAN,
                               01u);
    EX_TRACE("Day of week for Jan 1st 2000 is %u.\r\n", day_of_wk);

    day_of_yr = Clk_GetDayOfYr(2000u,
                               CLK_MONTH_AUG,
                               01u);
    EX_TRACE("Day of year for Aug 1st 2000 is %u.\r\n", day_of_yr);

    while (i < 100000000u) {                                     /* Let some time pass.                                  */
        i++;
    }

    is_ok = Clk_GetDateTime(&date_time_get);
    APP_RTOS_ASSERT_CRITICAL(is_ok == DEF_OK, ;);

    if ((date_time_get.Hr  == date_time_set.Hr) &&
        (date_time_get.Min == date_time_set.Min) &&
        (date_time_get.Sec == date_time_set.Sec)) {
        EX_TRACE("Clk did not see any time pass. Try increasing the previous delay and/or see if the signaling to Clk is done correctly, either in Clk task or via Clk_SignalClk() calls.\r\n");
    } else {
        EX_TRACE("Time set was: %uh%02u, %02u seconds. Time is now: %uh%02u, %02u seconds.",
                date_time_set.Hr, date_time_set.Min, date_time_set.Sec,
                date_time_get.Hr, date_time_get.Min, date_time_get.Sec);
    }
}


#endif /* RTOS_MODULE_COMMON_CLK_AVAIL */
