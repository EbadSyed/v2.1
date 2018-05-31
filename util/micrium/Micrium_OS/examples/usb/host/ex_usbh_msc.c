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
*                                          USB HOST EXAMPLE
*
*                                      Mass Storage Class (MSC)
*********************************************************************************************************
* The MSC class driver is meant to be used with devices such as flash drives, external hard drives, media
* card readers, CD-ROM readers, etc.
*
* This example requires the uC/OS-FS SCSI module.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                     DEPENDENCIES & AVAIL CHECK(S)
*********************************************************************************************************
*********************************************************************************************************
*/

#include <rtos_description.h>

#if (defined(RTOS_MODULE_USB_HOST_MSC_AVAIL))


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#define   EX_USBH_MSC_MODULE

#include  <rtos_description.h>
#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/rtos_utils.h>
#include  <rtos/common/include/rtos_err.h>
#include  <rtos/common/include/lib_mem.h>

#include  <rtos/usb/include/host/usbh_core.h>
#include  <rtos/usb/include/host/usbh_msc.h>

#ifdef RTOS_MODULE_FS_STORAGE_SCSI_AVAIL
#include  <rtos/fs/include/fs_scsi.h>
#endif

#include  <ex_description.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                        LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          LOCAL CONSTANTS
*********************************************************************************************************
*********************************************************************************************************
*/

#ifdef RTOS_MODULE_FS_STORAGE_SCSI_AVAIL
static  const  USBH_MSC_CMD_BLK_FNCTS  Ex_USBH_MSC_CmdBlkFncts = {.Conn             = FS_SCSI_LU_Conn,
                                                                  .Disconn          = FS_SCSI_LU_Disconn,
                                                                  .MaxRespBufLenGet = FS_SCSI_MaxRespBufLenGet};
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
*                                         Ex_USBH_HID_Init()
*
* Description : Example of initialization of the HID class driver.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_USBH_MSC_Init (void)
{
#ifdef RTOS_MODULE_FS_STORAGE_SCSI_AVAIL
    RTOS_ERR  err;


    USBH_MSC_Init(&Ex_USBH_MSC_CmdBlkFncts, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
#endif
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif  /* RTOS_MODULE_USB_HOST_MSC_AVAIL */
