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
* File : ex_common_clk_set_sntpc.c
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

#if defined(RTOS_MODULE_COMMON_CLK_AVAIL) &&    \
    defined(RTOS_MODULE_NET_AVAIL)        &&    \
    defined(RTOS_MODULE_NET_SNTP_CLIENT_AVAIL)

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/clk.h>
#include  <rtos/common/include/lib_def.h>
#include  <rtos/common/include/rtos_utils.h>
#include  <rtos/net/include/sntp_client.h>
#include  <rtos/kernel/include/os.h>


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
*                                       Ex_CommonClkSetSNTPc()
*
* Description : Provides example on how to use the Clock sub-module to set the time using SNTPc.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : (1) This examples assumes that SNTPc has previously been initialized successfully.
*********************************************************************************************************
*/

void  Ex_CommonClkSetSNTPc (void)
{
    SNTP_PKT       pkt;
    SNTP_TS        ts;
    CLK_DATE_TIME  date_time;
    CLK_DATE_TIME  date_time_from_convert_ts;
    CLK_DATE_TIME  date_time_from_convert_ntp;
    CLK_TS_SEC     ts_sec;
    CLK_TS_SEC     ts_sec_ntp_from_clk;
    CPU_BOOLEAN    ok;
    CPU_CHAR       clk_str_buf[CLK_STR_FMT_DAY_MONTH_DD_HH_MM_SS_YYYY_LEN];
    CPU_INT08U     retry_ctr = 0;
    RTOS_ERR       err;


    for (retry_ctr = 0; retry_ctr < 3; retry_ctr++) {
        RTOS_ERR  err_local;


        SNTPc_ReqRemoteTime("0.pool.ntp.org", &pkt, &err);      /* Send SNTP request.                                   */
        if (err.Code == RTOS_ERR_NONE) {
            break;
        } else {
            OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_HMSM_NON_STRICT, &err_local);
        }
    }

    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    ts = SNTPc_GetRemoteTime(&pkt, &err);                       /* Retrieve current time.                               */
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    ok = Clk_SetTS_NTP(ts.Sec);
    APP_RTOS_ASSERT_DBG((ok == DEF_OK), ;);

    ok = Clk_GetTS_NTP(&ts_sec_ntp_from_clk);
    APP_RTOS_ASSERT_DBG((ok == DEF_OK), ;);

    ts_sec = Clk_GetTS();
    ok = Clk_TS_ToDateTime(ts_sec, 0, &date_time_from_convert_ts);
    APP_RTOS_ASSERT_DBG((ok == DEF_OK), ;);

    ok = Clk_TS_NTP_ToDateTime(ts.Sec, 0, &date_time_from_convert_ntp);
    APP_RTOS_ASSERT_DBG((ok == DEF_OK), ;);

    ok = Clk_GetDateTime(&date_time);
    APP_RTOS_ASSERT_DBG((ok == DEF_OK), ;);

    EX_TRACE("ts.sec = %u | ts_sec_ntp_from_clk = %u\r\n", ts.Sec, ts_sec_ntp_from_clk);

    ok = Clk_DateTimeToStr(&date_time, CLK_STR_FMT_DAY_MONTH_DD_HH_MM_SS_YYYY, &clk_str_buf[0u], CLK_STR_FMT_DAY_MONTH_DD_HH_MM_SS_YYYY_LEN);
    APP_RTOS_ASSERT_DBG((ok == DEF_OK), ;);

    EX_TRACE("Clk: We are %s.\r\n", (CPU_CHAR *)&clk_str_buf);
}


#endif /* RTOS_MODULE_COMMON_CLK_AVAIL */
