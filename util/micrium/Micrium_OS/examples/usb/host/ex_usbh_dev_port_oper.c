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
*                                  Port operation example application
*********************************************************************************************************
* This example shows how to perform USB devices operations like suspend, resume and reset.
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

#if (defined(RTOS_MODULE_USB_HOST_AVAIL))


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/rtos_utils.h>
#include  <rtos/common/include/rtos_err.h>

#include  <rtos/usb/include/host/usbh_core.h>
#include  <rtos/usb/include/host/usbh_core_hub.h>

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
*                                               LOGGING
*
* Note(s) : (1) This example outputs information to the console via the function printf() via a macro
*               called EX_TRACE(). This can be modified or disabled if printf() is not supported.
*********************************************************************************************************
*/

#ifndef EX_TRACE
#include  <stdio.h>
#define  EX_TRACE(...)                                      printf(__VA_ARGS__)
#endif


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

static  void  Ex_USBH_DevPortOperCmpl (USBH_DEV_HANDLE     hub_dev_handle,
                                       USBH_HUB_PORT_REQ   port_req,
                                       CPU_INT08U          port_nbr,
                                       void               *p_arg,
                                       RTOS_ERR            err);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      Ex_USBH_DevSuspendExec()
*
* Description : Suspends a USB device.
*
* Argument(s) : dev_handle      Device handle to suspend.
*
* Return(s)   : None.
*
* Note(s)     : (1) USB Host hub class cannot suspend a USB device using the device handle. It can only
*                   suspend a port on a USB hub. Hence, you must first retrieve the hub handle and the
*                   port number on which the device is connected.
*
*               (2) Once you return from the function, it does not mean the the operation is completed.
*                   The function Ex_USBH_DevPortOperCmpl() will be called one the operation is completed.
*********************************************************************************************************
*/

void  Ex_USBH_DevSuspendExec (USBH_DEV_HANDLE  dev_handle)
{
    CPU_INT08U       port_nbr;
    USBH_DEV_HANDLE  hub_dev_handle;
    RTOS_ERR         err;


                                                                /* Get hub handle on which dev connected. See note (1). */
    hub_dev_handle = USBH_DevHubHandleGet(dev_handle, &err);

                                                                /* Get port nbr on which dev connected. See note (1).   */
    port_nbr = USBH_DevPortNbrGet(dev_handle, &err);

                                                                /* Submit a request to suspend port. See note (2).      */
    USBH_HUB_PortSuspendReq(hub_dev_handle,
                            port_nbr,
                            Ex_USBH_DevPortOperCmpl,            /* Function called when operation completed.            */
                            DEF_NULL,                           /* Pointer to data to be passed to callback function.   */
                           &err);
}


/*
*********************************************************************************************************
*                                       Ex_USBH_DevResumeExec()
*
* Description : Resume a USB device.
*
* Argument(s) : p_cfg       Pointer to RTOS configurations.
*
* Return(s)   : None.
*
* Note(s)     : (1) USB Host hub class cannot resume a USB device using the device handle. It can only
*                   resume a port on a USB hub. Hence, you must first retrieve the hub handle and the
*                   port number on which the device is connected.
*
*               (2) Once you return from the function, it does not mean the the operation is completed.
*                   The function Ex_USBH_DevPortOperCmpl() will be called one the operation is completed.
*********************************************************************************************************
*/

void  Ex_USBH_DevResumeExec (USBH_DEV_HANDLE  dev_handle)
{
    CPU_INT08U       port_nbr;
    USBH_DEV_HANDLE  hub_dev_handle;
    RTOS_ERR         err;


                                                                /* Get hub handle on which dev connected. See note (1). */
    port_nbr = USBH_DevPortNbrGet(dev_handle, &err);

                                                                /* Get port nbr on which dev connected. See note (1).   */
    hub_dev_handle = USBH_DevHubHandleGet(dev_handle, &err);

                                                                /* Submit a request to resume port. See note (2).       */
    USBH_HUB_PortResumeReq(hub_dev_handle,
                           port_nbr,
                           Ex_USBH_DevPortOperCmpl,             /* Function called when operation completed.            */
                           DEF_NULL,                            /* Pointer to data to be passed to callback function.   */
                          &err);
}


/*
*********************************************************************************************************
*                                       Ex_USBH_DevResetExec()
*
* Description : Reset a USB device.
*
* Argument(s) : p_cfg       Pointer to RTOS configurations.
*
* Return(s)   : None.
*
* Note(s)     : (1) USB Host hub class cannot reset a USB device using the device handle. It can only
*                   reset a port on a USB hub. Hence, you must first retrieve the hub handle and the
*                   port number on which the device is connected.
*
*               (2) Once you return from the function, it does not mean the the operation is completed.
*                   The function Ex_USBH_DevPortOperCmpl() will be called one the operation is completed.
*********************************************************************************************************
*/

void  Ex_USBH_DevResetExec (USBH_DEV_HANDLE  dev_handle)
{
    CPU_INT08U       port_nbr;
    USBH_DEV_HANDLE  hub_dev_handle;
    RTOS_ERR         err;


                                                                /* Get hub handle on which dev connected. See note (1). */
    port_nbr = USBH_DevPortNbrGet(dev_handle, &err);

                                                                /* Get port nbr on which dev connected. See note (1).   */
    hub_dev_handle = USBH_DevHubHandleGet(dev_handle, &err);

                                                                /* Submit a request to reset port. See note (2).        */
    USBH_HUB_PortResetReq(hub_dev_handle,
                          port_nbr,
                          Ex_USBH_DevPortOperCmpl,              /* Function called when operation completed.            */
                          DEF_NULL,                             /* Pointer to data to be passed to callback function.   */
                         &err);
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
*                                      Ex_USBH_DevPortOperCmpl()
*
* Description : Hub port operation completed.
*
* Argument(s) : hub_dev_handle      Handle on HUB device.
*
*               port_req            Port request that was performed.
*
*                                       USBH_HUB_PORT_REQ_SUSPEND
*                                       USBH_HUB_PORT_REQ_RESUME
*                                       USBH_HUB_PORT_REQ_RESET
*
*               port_nbr            Port number on which the request was performed.
*
*               p_arg               Pointer to argument passed to function USBH_HUB_PortXXXXReq().
*
*               err                 Error associated to operation.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  Ex_USBH_DevPortOperCmpl (USBH_DEV_HANDLE     hub_dev_handle,
                                       USBH_HUB_PORT_REQ   port_req,
                                       CPU_INT08U          port_nbr,
                                       void               *p_arg,
                                       RTOS_ERR            err)
{
    PP_UNUSED_PARAM(hub_dev_handle);
    PP_UNUSED_PARAM(port_req);
    PP_UNUSED_PARAM(port_nbr);
    PP_UNUSED_PARAM(p_arg);

    if (err.Code == RTOS_ERR_NONE) {
        EX_TRACE("USB Host port oper example: Port operation completed\r\n");
    } else {
        EX_TRACE("USB Host port oper example: Port operation FAILED\r\n");
    }
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif  /* RTOS_MODULE_USB_HOST_AVAIL */
