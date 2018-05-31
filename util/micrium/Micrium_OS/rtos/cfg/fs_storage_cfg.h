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
*                                   FILE SYSTEM STORAGE CONFIGURATION
*
*                                      CONFIGURATION TEMPLATE FILE
*
* Filename      : fs_storage_cfg.h
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  FS_STORAGE_CFG_H
#define  FS_STORAGE_CFG_H


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
*                                     STORAGE LAYER CONFIGURATION
*********************************************************************************************************
*********************************************************************************************************
*/

#define  FS_STORAGE_CFG_DBG_WR_VERIFY_EN                    DEF_DISABLED

#define  FS_STORAGE_CFG_CTR_STAT_EN                         DEF_DISABLED

#define  FS_STORAGE_CFG_CTR_ERR_EN                          DEF_DISABLED

#define  FS_STORAGE_CFG_MEDIA_POLL_TASK_EN                  DEF_ENABLED

#define  FS_STORAGE_CFG_RD_ONLY_EN                          DEF_DISABLED

#define  FS_STORAGE_CFG_CRC_OPTIMIZE_ASM_EN                 DEF_DISABLED


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          NAND CONFIGURATION
*********************************************************************************************************
*********************************************************************************************************
*/

#define  FS_NAND_CFG_AUTO_SYNC_EN                           DEF_ENABLED

#define  FS_NAND_CFG_UB_META_CACHE_EN                       DEF_ENABLED

#define  FS_NAND_CFG_DIRTY_MAP_CACHE_EN                     DEF_ENABLED

#define  FS_NAND_CFG_UB_TBL_SUBSET_SIZE                     1u

#define  FS_NAND_CFG_RSVD_AVAIL_BLK_CNT                     3u

#define  FS_NAND_CFG_MAX_RD_RETRIES                         10u

#define  FS_NAND_CFG_MAX_SUB_PCT                            30u

#define  FS_NAND_CFG_DUMP_SUPPORT_EN                        DEF_DISABLED


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          NOR CONFIGURATION
*********************************************************************************************************
*********************************************************************************************************
*/

#define  FS_NOR_CFG_WR_CHK_EN                               DEF_DISABLED


/*
*********************************************************************************************************
*********************************************************************************************************
*                                         SD SPI CONFIGURATION
*********************************************************************************************************
*********************************************************************************************************
*/

#define  FS_SD_SPI_CFG_CRC_EN                               DEF_ENABLED


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif                                                          /* End of fs_storage_cfg.h module include.              */