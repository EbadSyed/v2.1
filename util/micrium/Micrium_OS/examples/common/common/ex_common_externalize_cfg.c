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
* File : ex_common_externalize_cfg.c
*********************************************************************************************************
* Note(s) : (1) This example intends to provide a template for externalized configuration structures.
*               The values in these structures will be used as configuration values for the stacks and no
*               <Module>_Configure...() functions will be available to change these afterwards.
*               Please refer to Micrium's documentation for more details on the methods that can be used
*               to configure the stacks.
*
*           (2) This example requires RTOS_CFG_EXTERNALIZE_OPTIONAL_CFG_EN to be set to DEF_DISABLED to
*               work correctly. This will otherwise casue symbols to be duplicated.
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
*                                           GLOBAL CONSTANTS
*********************************************************************************************************
*********************************************************************************************************
*/

#if (RTOS_CFG_EXTERNALIZE_OPTIONAL_CFG_EN == DEF_ENABLED)

#if (RTOS_CFG_LOG_EN == DEF_ENABLED)
const  COMMON_INIT_CFG  Common_InitCfg =
{
    .CommonMemSegPtr = DEF_NULL,
    .LoggingCfg =
    {
        .AsyncBufSize = 512u
    },
    .LoggingMemSegPtr = DEF_NULL
};
#else
const  COMMON_INIT_CFG  Common_InitCfg =
{
    .CommonMemSegPtr = DEF_NULL,
};
#endif

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL
const  SHELL_INIT_CFG  Shell_InitCfg =
{
    .CfgCmdUsage =
    {
        .CmdTblItemNbrInit = 10u,
        .CmdTblItemNbrMax  = 10u,
        .CmdArgNbrMax      = 10u,
        .CmdNameLenMax     = 10u
    },
    .MemSegPtr = DEF_NULL
};
#endif

#ifdef  RTOS_MODULE_COMMON_CLK_AVAIL
#if ((CLK_CFG_EXT_EN    == DEF_DISABLED) && \
     (CLK_CFG_SIGNAL_EN == DEF_DISABLED))
const  CLK_INIT_CFG  Clk_InitCfg =
{
    .StkSizeElements = 128u,
    .StkPtr          = DEF_NULL,
    .MemSegPtr       = DEF_NULL
};
#else
const  CLK_INIT_CFG  Clk_InitCfg =
{
    .MemSegPtr = DEF_NULL
};
#endif
#endif

const  AUTH_INIT_CFG  Auth_InitCfg =
{
    .RootUserCfg =
    {
        .RootUserName = "admin",
        .RootUserPwd  = "admin"
    },
    .ResourceCfg =
    {
        .NbUserMax  =  4u,
        .NameLenMax = 10u,
        .PwdLenMax  = 10u
    },
    .MemSegPtr = DEF_NULL
};

#endif  /* RTOS_CFG_EXTERNALIZE_OPTIONAL_CFG_EN */
