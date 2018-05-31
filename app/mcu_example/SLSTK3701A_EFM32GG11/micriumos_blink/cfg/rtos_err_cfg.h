/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*********************************************************************************************************
* Licensing terms:
*   This file is provided as an example on how to use Micrium products. It has not necessarily been
*   tested under every possible condition and is only offered as a reference, without any guarantee.
*
*   Please feel free to use any application code labeled as 'EXAMPLE CODE' in your application products.
*   Example code may be used as is, in whole or in part, or may be used as a reference only. This file
*   can be modified as required.
*
*   You can find user manuals, API references, release notes and more at: https://doc.micrium.com
*
*   You can contact us at: http://www.micrium.com
*
*   Please help us continue to provide the Embedded community with the finest software available.
*
*   Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        RTOS_ERR CONFIGURATION
*
*                                      CONFIGURATION TEMPLATE FILE
*
* Filename      : rtos_err_cfg.h
* Programmer(s) : Micrium
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  _RTOS_ERR_CFG_H_
#define  _RTOS_ERR_CFG_H_


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/common/include/lib_def.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          INCLUDE FILES NOTE
*
* Note(s) : (1) No files including rtos_err.h must be included by this file. This could lead to circular
*               inclusion problems.
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                               DEFINES
*
* Note(s) : (1) RTOS_ERR_CFG_LEGACY_EN is provided to allow usage of existing code without needing to
*               modify anything regarding error codes. New applications should either completely remove
*               this configuration define  or set it to DEF_DISABLED, to allow usage of extended errors
*               and other advantages such as unified error codes.
*
*           (2) RTOS_ERR_CFG_EXT_EN allows to configure whether the RTOS_ERR type contains only an error
*               code (DEF_DISABLED) or contains more debug information (DEF_ENABLED). If set to
*               DEF_ENABLED, a string containing the file name and line number where the error has been
*               set and also the function name, if compiling in C99, will be included in the RTOS_ERR
*               type. Setting this configuration to DEF_ENABLED may have an impact on performance and
*               resource usage, it is recommended to set it to DEF_DISABLED once development is
*               complete. This feature cannot be used if RTOS_ERR_CFG_LEGACY_EN is set to DEF_ENABLED.
*
*           (3) RTOS_ERR_CFG_STR_EN allows to have strings associated with each error code, in order to
*               print them. If set to DEF_DISABLED, the error code enum value will be outputted instead.
*               For example, if set to DEF_ENABLED, it would be possible to print RTOS_ERR_NONE or
*               RTOS_ERR_INVALID_ARG as a string instead of printing the numerical value associated,
*               which would be 0 for RTOS_ERR_NONE and higher than 0 for RTOS_ERR_INVALID_ARG.
*********************************************************************************************************
*********************************************************************************************************
*/

#define  RTOS_ERR_CFG_LEGACY_EN                             DEF_DISABLED

#define  RTOS_ERR_CFG_EXT_EN                                DEF_ENABLED

#define  RTOS_ERR_CFG_STR_EN                                DEF_ENABLED


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif                                                          /* End of rtos_err_cfg.h module include.                */
