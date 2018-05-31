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
*                                           RTOS DESCRIPTION
*
*                                      CONFIGURATION TEMPLATE FILE
*
* Filename      : rtos_description.h
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

#ifndef  _RTOS_DESCRIPTION_H_
#define  _RTOS_DESCRIPTION_H_


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/common/include/rtos_opt_def.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                       ENVIRONMENT DESCRIPTION
*********************************************************************************************************
*********************************************************************************************************
*/

#define  RTOS_CPU_SEL                                       RTOS_CPU_SEL_ARM_CORTEX_M4

#define  RTOS_TOOLCHAIN_SEL                                 RTOS_TOOLCHAIN_AUTO

#define  RTOS_INT_CONTROLLER_SEL                            RTOS_INT_CONTROLLER_ARMV7_M


/*
*********************************************************************************************************
*********************************************************************************************************
*                                       RTOS MODULES DESCRIPTION
*********************************************************************************************************
*********************************************************************************************************
*/

#define  RTOS_MODULE_KERNEL_AVAIL

#define  RTOS_MODULE_USB_HOST_MSC_AVAIL

#define  RTOS_MODULE_COMMON_CLK_AVAIL

#define  RTOS_MODULE_FS_AVAIL
#define  RTOS_MODULE_FS_FAT_AVAIL
#define  RTOS_MODULE_FS_SCSI_AVAIL
#define  RTOS_MODULE_FS_STORAGE_SCSI_AVAIL
//#define  RTOS_MODULE_FS_STORAGE_RAM_DISK_AVAIL

#define  RTOS_MODULE_COMMON_AVAIL

#define  RTOS_MODULE_USB_HOST_AVAIL

#define  RTOS_MODULE_USB_HOST_PBHCI_AVAIL


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif                                                          /* End of rtos_description.h module include.            */
