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
* File : ex_common_clk.h
*********************************************************************************************************
* Note(s) : (1) The following examples assume that the Common/Clk module has been initialized and that
*               the time is maintained by either Clk internal task or regular calls to Clk_SignalClk(),
*               depending on the configuration of Clk.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  _EX_COMMON_CLK_H_
#define  _EX_COMMON_CLK_H_


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos_description.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/


void  Ex_CommonClkSetManual (void);

void  Ex_CommonClkGet       (void);

#if defined(RTOS_MODULE_NET_AVAIL)        &&    \
    defined(RTOS_MODULE_NET_SNTP_CLIENT_AVAIL)
void  Ex_CommonClkSetSNTPc  (void);
#endif

#endif
