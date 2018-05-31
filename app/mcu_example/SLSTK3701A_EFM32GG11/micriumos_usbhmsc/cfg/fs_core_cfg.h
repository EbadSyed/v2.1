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
*                                       FILE SYSTEM CONFIGURATION
*
*                                      CONFIGURATION TEMPLATE FILE
*
* Filename      : fs_core_cfg.h
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

#ifndef  _FS_CORE_CFG_H_
#define  _FS_CORE_CFG_H_


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
*                                      FILE SYSTEM CONFIGURATION
*********************************************************************************************************
*********************************************************************************************************
*/

#define  FS_CORE_CFG_FAT_EN                                 DEF_ENABLED

#define  FS_CORE_CFG_POSIX_EN                               DEF_ENABLED

#define  FS_CORE_CFG_DIR_EN                                 DEF_ENABLED

#define  FS_CORE_CFG_FILE_BUF_EN                            DEF_ENABLED

#define  FS_CORE_CFG_FILE_LOCK_EN                           DEF_ENABLED

#define  FS_CORE_CFG_PARTITION_EN                           DEF_ENABLED

#define  FS_CORE_CFG_TASK_WORKING_DIR_EN                    DEF_ENABLED

#define  FS_CORE_CFG_UTF8_EN                                DEF_ENABLED

#define  FS_CORE_CFG_THREAD_SAFETY_EN                       DEF_ENABLED

#define  FS_CORE_CFG_ORDERED_WR_EN                          DEF_ENABLED

#define  FS_CORE_CFG_FILE_COPY_EN                           DEF_ENABLED

#define  FS_CORE_CFG_RD_ONLY_EN                             DEF_DISABLED

#define  FS_CORE_CFG_POSIX_PUTCHAR                          putchar

#define  FS_CORE_CFG_MAX_VOL_NAME_LEN                       20u


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   FILE SYSTEM DEBUG CONFIGURATION
*********************************************************************************************************
*********************************************************************************************************
*/

#define  FS_CORE_CFG_DBG_MEM_CLR_EN                         DEF_DISABLED


/*
*********************************************************************************************************
*********************************************************************************************************
*                             FILE SYSTEM COUNTER MANAGEMENT CONFIGURATION
*********************************************************************************************************
*********************************************************************************************************
*/

#define  FS_CORE_CFG_CTR_STAT_EN                            DEF_DISABLED

#define  FS_CORE_CFG_CTR_ERR_EN                             DEF_DISABLED


/*
*********************************************************************************************************
*********************************************************************************************************
*                                    FILE SYSTEM FAT CONFIGURATION
*********************************************************************************************************
*********************************************************************************************************
*/

#define  FS_FAT_CFG_LFN_EN                                  DEF_ENABLED

#define  FS_FAT_CFG_FAT12_EN                                DEF_ENABLED

#define  FS_FAT_CFG_FAT16_EN                                DEF_ENABLED

#define  FS_FAT_CFG_FAT32_EN                                DEF_ENABLED

#define  FS_FAT_CFG_JOURNAL_EN                              DEF_ENABLED


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif                                                          /* End of fs_core_cfg.h module include.                 */
