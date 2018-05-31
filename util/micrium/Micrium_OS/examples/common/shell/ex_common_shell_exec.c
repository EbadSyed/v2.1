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
*                                         COMMON SHELL EXAMPLE
*
* File : ex_common_shell_exec.c
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

#ifdef  RTOS_MODULE_COMMON_SHELL_AVAIL

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/shell.h>
#include  <rtos/common/include/lib_def.h>
#include  <rtos/common/include/rtos_utils.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  CPU_INT16S  Ex_CommonShellExecOutFnct (CPU_CHAR    *p_buf,
                                               CPU_INT16U   buf_len,
                                               void        *p_opt);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          LOCAL CONSTANTS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         Ex_CommonShellExec()
*
* Description : Provides example on how to use the Shell sub-module of Common to execute a command. The
*               'help' command will list every command currently available in the Shell sub-module.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_CommonShellExec (void)
{
    CPU_INT16S       ret_val;
    SHELL_CMD_PARAM  cmd_param;
    CPU_CHAR         cmd_buf[25u];
    RTOS_ERR         err;


    Str_Copy(cmd_buf, "help");                                  /* String passed to Shell_Exec must not be const.       */

                                                                /* Call Shell_Exec() with:                              */
    ret_val = Shell_Exec(cmd_buf,                               /*      string containing the cmd to exec,              */
                         Ex_CommonShellExecOutFnct,             /*      function to use to output,                      */
                         &cmd_param,                            /*      a cmd param structure that could be used by cmd.*/
                         &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    APP_RTOS_ASSERT_CRITICAL(ret_val == SHELL_EXEC_ERR_NONE, ;);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      Ex_CommonShellExecOutFnct()
*
* Description : Output data as requested by the Shell sub-module during the Shell_Exec() call.
*
* Argument(s) : p_buf       Pointer to buffer containing the data to output.
*
*               buf_len     Length of the buffer.
*
*               p_opt       Pointer to optional options parameter, unused in this case.
*
* Return(s)   : Number of bytes outputted.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  CPU_INT16S  Ex_CommonShellExecOutFnct (CPU_CHAR    *p_buf,
                                               CPU_INT16U   buf_len,
                                               void        *p_opt)
{
    CPU_INT16U  tx_len = buf_len;


    PP_UNUSED_PARAM(p_opt);                                     /* Supplemental options not used in this command.       */

    while (tx_len != 0u) {
        putchar(*p_buf);                                        /* Any putchar-like function could be used, here.       */
        tx_len--;
        p_buf++;
    }

    return (buf_len);                                           /* Return nbr of tx'd characters.                       */
}

#endif  /* RTOS_MODULE_COMMON_SHELL_AVAIL */
