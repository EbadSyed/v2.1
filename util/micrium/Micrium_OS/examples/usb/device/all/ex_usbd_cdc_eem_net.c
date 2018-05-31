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
*                                         USB DEVICE EXAMPLE
*
*                                  USB CDC Ethernet Emulation Model
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

#if (defined(RTOS_MODULE_USB_DEV_EEM_AVAIL) && defined(RTOS_MODULE_NET_IF_ETHER_AVAIL))


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

#include  <rtos/usb/include/device/usbd_core.h>
#include  <rtos/usb/include/device/usbd_cdc_eem.h>

#include  <rtos/net/include/net.h>
#include  <rtos/net/include/net_if.h>

#include  <rtos/drivers/net/include/net_drv_ether.h>

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
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
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
*                                       Ex_USBD_CDC_EEM_Init()
*
* Description : Adds a CDC-EEM interface to the device and creates an ethernet interface that it
*               associates to the CDC EEM instance. This example requires uC-RTOS-Net to be part of the
*               project.
*
* Argument(s) : dev_nbr         Device number.
*
*               config_nbr_fs   Full-Speed configuration number.
*
*               config_nbr_hs   High-Speed configuration number.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_USBD_CDC_EEM_Init (CPU_INT08U  dev_nbr,
                            CPU_INT08U  config_nbr_fs,
                            CPU_INT08U  config_nbr_hs)
{
    CPU_INT08U                      cdc_eem_nbr;
    USBD_CDC_EEM_QTY_CFG            cfg_eem;
    NET_IF_NBR                      if_nbr;
    CPU_CHAR                       *p_name;
    NET_IF_ETHER_CFG                ether_cfg;
    USBD_CDC_EEM_NET_IF_ETHER_CFG   cdc_eem_ether_cfg;
    RTOS_ERR                        err;


                                                                /* -------- INITIALIZE CDC EEM CLASS INSTANCE --------- */
    cfg_eem.ClassInstanceQty = 1;
    cfg_eem.ConfigQty        = 2;

    USBD_CDC_EEM_Init(&cfg_eem,                                 /* Pointer to the configuration structure.              */
                      &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* ---------- CREATE CDC EEM CLASS INSTANCE ----------- */

    Mem_Copy((CPU_CHAR *)&cdc_eem_ether_cfg.HW_AddrStr,
             (CPU_CHAR *)"00:AB:CD:EF:80:07",
                          NET_IF_802x_ADDR_SIZE_STR);

    cdc_eem_ether_cfg.RxBufQty      = 3u;
    cdc_eem_ether_cfg.TxBufQty      = 1u;
    cdc_eem_ether_cfg.TxBufSmallLen = 60u;
    cdc_eem_ether_cfg.TxBufSmallQty = 3u;

    cdc_eem_nbr = USBD_CDC_EEM_NetIF_Reg (&p_name,              /* String identifier given by the platform manager.     */
                                          &cdc_eem_ether_cfg,   /* Variable than hold buffer size configuration.        */
                                          &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* ---------- ADD CLASS INSTANCE TO CONFIGS ----------- */
    USBD_CDC_EEM_ConfigAdd(cdc_eem_nbr,
                           dev_nbr,                             /* Device number.                                       */
                           config_nbr_fs,                       /* Full-speed configuration number.                     */
                          "USB device CDC EEM example",         /* Name of the interface.                               */
                          &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    if (config_nbr_hs != USBD_CONFIG_NBR_NONE) {                /* Add instance to HS config if available.              */
        USBD_CDC_EEM_ConfigAdd(cdc_eem_nbr,                     /* Class instance number.                               */
                               dev_nbr,                         /* Device number.                                       */
                               config_nbr_hs,                   /* High-speed configuration number.                     */
                              "USB device CDC EEM example",     /* Name of the interface.                               */
                              &err);
        APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    }


    if_nbr = NetIF_Ether_Add(p_name,                            /* String identifier for the Ethernet interface.        */
                             DEF_NULL,                          /* Buffer configuration: no overwrite.                  */
                             DEF_NULL,                          /* Memory segment.                                      */
                            &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* --------- START AN ETHERNET TYPE INTERFACE --------- */

    ether_cfg.HW_AddrStr                 = "00:AB:CD:EF:80:07";
    ether_cfg.IPv4.Static.Addr           = "192.168.0.10";
    ether_cfg.IPv4.Static.Mask           = "255.255.255.0";
    ether_cfg.IPv4.Static.Gateway        = "192.168.0.1";
    ether_cfg.IPv4.DHCPc.En              =  DEF_NO;
    ether_cfg.IPv4.DHCPc.OnCompleteHook  =  DEF_NULL;
    ether_cfg.IPv6.Static.Addr           =  DEF_NULL;
    ether_cfg.IPv6.Static.PrefixLen      =  0u;
    ether_cfg.IPv6.Static.DAD_En         =  DEF_NO;
    ether_cfg.IPv6.AutoCfg.En            =  DEF_YES;
    ether_cfg.IPv6.AutoCfg.DAD_En        =  DEF_YES;

    NetIF_Ether_Start(if_nbr,                                   /* Interface number.                                    */
                     &ether_cfg,                                /* Ethernet address configuration structure.            */
                     &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif  /* RTOS_MODULE_USB_DEV_EEM_AVAIL && RTOS_MODULE_NET_IF_ETHER_AVAIL */
