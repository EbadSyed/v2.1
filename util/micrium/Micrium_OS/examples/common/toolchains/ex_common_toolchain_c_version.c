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
* File : ex_common_toolchain_c_version.c
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
*                                    Ex_CommonToolchainC_Version()
*
* Description : Provides example on how to use the C versions macros.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_CommonToolchainC_Version (void)
{
#ifdef  PP_C_STD_VERSION_C89_PRESENT
    /* This part of the code will only be executed if the C standard version used to compile is at least C89. */
    EX_TRACE("Compiling with at least C89 standard version.\r\n");
#endif

#ifdef  PP_C_STD_VERSION_C90_PRESENT
    /* This part of the code will only be executed if the C standard version used to compile is at least C90. */
    EX_TRACE("Compiling with at least C90 standard version.\r\n");
#endif

#ifdef  PP_C_STD_VERSION_C94_PRESENT
    /* This part of the code will only be executed if the C standard version used to compile is at least C94. */
    EX_TRACE("Compiling with at least C94 standard version.\r\n");
#endif

#ifdef  PP_C_STD_VERSION_C99_PRESENT
    /* This part of the code will only be executed if the C standard version used to compile is at least C99. */
    EX_TRACE("Compiling with at least C99 standard version.\r\n");
#endif


#if (PP_C_STD_VERSION == PP_C_STD_VERSION_C89)
    /* This part of the code will only be executed if the C standard version used to compile is the C89 version. */
    EX_TRACE("Compiling with the C89 standard version.\r\n");
#endif

#if (PP_C_STD_VERSION == PP_C_STD_VERSION_C90)
    /* This part of the code will only be executed if the C standard version used to compile is the C90 version. */
    EX_TRACE("Compiling with the C90 standard version.\r\n");
#endif

#if (PP_C_STD_VERSION == PP_C_STD_VERSION_C94)
    /* This part of the code will only be executed if the C standard version used to compile is the C94 version. */
    EX_TRACE("Compiling with the C94 standard version.\r\n");
#endif

#if (PP_C_STD_VERSION == PP_C_STD_VERSION_C99)
    /* This part of the code will only be executed if the C standard version used to compile is the C99 version. */
    EX_TRACE("Compiling with the C99 standard version.\r\n");
#endif
}
