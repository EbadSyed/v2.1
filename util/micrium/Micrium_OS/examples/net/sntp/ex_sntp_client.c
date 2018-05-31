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
*                                                EXAMPLE
*
*                                  SNTP CLIENT APPLICATION FUNCTIONS FILE
*
* File : ex_sntp_client.c
*********************************************************************************************************
* Note(s) : (1) For additional details on the features available with Micrium OS SNTP Client,
*               the API, the installation, etc. Refer to the Micrium OS SNTP Client documentation
*               available at https://doc.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                     DEPENDENCIES & AVAIL CHECK(S)
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos_description.h>

#if (defined(RTOS_MODULE_NET_SNTP_CLIENT_AVAIL))


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <ex_description.h>

#include  <rtos/common/include/rtos_path.h>
#include  <rtos/common/include/rtos_utils.h>

#include  <rtos/net/include/net_app.h>
#include  <rtos/net/include/sntp_client.h>


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
#define  EX_TRACE(...)                                      printf(__VA_ARGS__)
#endif


/*
*********************************************************************************************************
*                                            Ex_SNTP_Client_Init()
*
* Description : Initialize the Micrium OS SNTP Client module for the example application.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_SNTP_Client_Init (void)
{
    RTOS_ERR  err;

                                                                /* ------------- INITIALIZE CLIENT SUITE -------------- */
    SNTPc_Init(&err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
}


/*
*********************************************************************************************************
*                                    Ex_SNTP_Client_CurTimeDisplay()
*
* Description : Displays current time.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : (1) The timestamp returned by SNTPc_GetRemoteTime() represents the quantity of seconds
*                   since January 1, 1900. It also gives the fractions of second.
*********************************************************************************************************
*/

void  Ex_SNTP_Client_CurTimeDisplay (void)
{
    CPU_INT08U  sec;
    CPU_INT08U  min;
    CPU_INT08U  hour;
    CPU_INT16U  day;
    CPU_INT16U  year;
    CPU_INT16U  qty_leap_years;
    CPU_INT32U  temp;
    SNTP_PKT    pkt;
    SNTP_TS     ts;
    RTOS_ERR    err;


    SNTPc_ReqRemoteTime("0.pool.ntp.org",                       /* Send SNTP request.                                   */
                        &pkt,
                        &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    ts = SNTPc_GetRemoteTime(&pkt, &err);                       /* Retrieve current time.                               */
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    sec  = ts.Sec % 60u;                                        /* ts.Sec represents qty of seconds since Jan 1, 1900.  */
    temp = ts.Sec / 60u;

    min   = temp % 60u;
    temp /= 60u;

    hour  = temp % 24u;
    temp /= 24u;

                                                                /* Assume all years have 365 days for now.              */
    year            =  1900u + (temp / 365u);
    qty_leap_years  = (year / 4u) - (year / 100u) + (year / 400u);
    qty_leap_years -=  460u;                                    /* 460 leap years between year 0 and 1900.              */
    day             =  temp % 365u;

    if (((year % 4u   == 0u) &&
         (year % 100u != 0u)) ||
        ( year % 400u == 0u)) {
        qty_leap_years--;                                       /* Est year is leap, do not consider in date calc.      */
    }

    if (day < qty_leap_years) {                                 /* Date is in year before estimated year.               */
        CPU_INT16U  new_yr_qty_days;


        year            -=  1u;
        new_yr_qty_days  = (((year % 4u == 0u) && (year % 100u != 0u)) || (year % 400u == 0u)) ? 366u : 365u;
        day              =  new_yr_qty_days - (qty_leap_years - day);
    } else {                                                    /* Date is in estimated year.                           */
        day -= qty_leap_years;
    }

    day++;                                                      /* Convert day starting from 0 to starting from 1.      */

    EX_TRACE("Day %u of year %u. Time: %u:%u:%u(UTC)\r\n", day, year, hour, min, sec);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif /* RTOS_MODULE_NET_SNTP_CLIENT_AVAIL */

