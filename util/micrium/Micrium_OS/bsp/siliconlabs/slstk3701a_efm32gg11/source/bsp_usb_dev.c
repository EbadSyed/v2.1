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
*                               USB BOARD SUPPORT PACKAGE (BSP) FUNCTIONS
*
*                                            SILICON LABS
*                                          EFM32GG-STK3700a
*                                          EFM32GG11-STK3701
*
* File : bsp_usb_dev.c
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#define    BSP_USB_MODULE

#include  <rtos/common/include/lib_utils.h>
#include  <rtos/common/include/rtos_path.h>
#include  <rtos/common/include/rtos_utils.h>
#include  <rtos_description.h>

#ifdef  RTOS_MODULE_USB_DEV_AVAIL

#include  <rtos/usb/include/device/usbd_core.h>
#include  <rtos/drivers/usb/include/usbd_drv.h>

#include  <em_cmu.h>
#include  <em_gpio.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#define  BSP_INT_OFFSET                      16u
#define  BSP_INT_ID_USB                      (BSP_INT_OFFSET + USB_IRQn)

/*
*********************************************************************************************************
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

                                                                /* ----------------- USB DEVICE FNCTS ----------------- */
static  void  BSP_USBD_EFM32_STK_Init    (USBD_DRV  *p_drv);

static  void  BSP_USBD_EFM32_STK_Conn    (void);

static  void  BSP_USBD_EFM32_STK_Disconn (void);

static  void  BSP_USBD_EFM32_IntHandler  (void);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           USB DEVICE CFGS
*********************************************************************************************************
*/

static  USBD_DRV  *BSP_USBD_EFM32_DrvPtr;                     /* USB device driver specific info.                     */


                                                                /* ---------- USB DEVICE ENDPOINTS INFO TBL ----------- */
static  USBD_DRV_EP_INFO  BSP_USBD_EFM32_EP_InfoTbl[] = {
    {USBD_EP_INFO_TYPE_CTRL                                                                            | USBD_EP_INFO_DIR_OUT, 0u,   64u},
    {USBD_EP_INFO_TYPE_CTRL                                                                            | USBD_EP_INFO_DIR_IN,  0u,   64u},
    {USBD_EP_INFO_TYPE_CTRL | USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_OUT, 1u,   64u},
    {USBD_EP_INFO_TYPE_CTRL | USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_IN,  1u,   64u},
    {USBD_EP_INFO_TYPE_CTRL | USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_OUT, 2u,   64u},
    {USBD_EP_INFO_TYPE_CTRL | USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_IN,  2u,   64u},
    {USBD_EP_INFO_TYPE_CTRL | USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_OUT, 3u,   64u},
    {USBD_EP_INFO_TYPE_CTRL | USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_IN,  3u,   64u},
    {USBD_EP_INFO_TYPE_CTRL | USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_OUT, 4u,   64u},
    {USBD_EP_INFO_TYPE_CTRL | USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_IN,  4u,   64u},
    {USBD_EP_INFO_TYPE_CTRL | USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_OUT, 5u,   64u},
    {USBD_EP_INFO_TYPE_CTRL | USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_IN,  5u,   64u},
    {DEF_BIT_NONE                                                                                                          ,   0u,    0u}
};

                                                                /* ---------- USB DEVICE DRIVER INFORMATION ----------- */
static  USBD_DRV_INFO  BSP_USBD_EFM32_DrvInfoPtr = {
#if defined(USBC_MEM_BASE)
    .BaseAddr   = USBC_MEM_BASE,
#else
    .BaseAddr   = USB_MEM_BASE,
#endif
    .MemAddr    = 0x00000000u,
    .MemSize    = 0u,
    .Spd        = USBD_DEV_SPD_FULL,
    .EP_InfoTbl = BSP_USBD_EFM32_EP_InfoTbl
};

                                                                /* ------------- USB DEVICE BSP STRUCTURE ------------- */
static  USBD_DRV_BSP_API   BSP_USBD_EFM32_STK_BSP_API_Ptr = {
    BSP_USBD_EFM32_STK_Init,
    BSP_USBD_EFM32_STK_Conn,
    BSP_USBD_EFM32_STK_Disconn
};

                                                                /* ----------- USB DEVICE HW INFO STRUCTURE ----------- */
const  USBD_DEV_CTRLR_HW_INFO  BSP_USBD_EFM32_HwInfo = {
    .DrvAPI_Ptr  = &USBD_DrvAPI_EFM32_OTG_FS,
    .DrvInfoPtr  = &BSP_USBD_EFM32_DrvInfoPtr,
    .BSP_API_Ptr = &BSP_USBD_EFM32_STK_BSP_API_Ptr
};


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      BSP_USBD_EFM32_STK_Init()
*
* Description : USB device controller board-specific initialization.
*
*                   (1) Enable    USB dev ctrl registers  and bus clock.
*                   (2) Configure USB dev ctrl interrupts.
*                   (3) Disable   USB dev transceiver Pull-up resistor in D+ line.
*                   (4) Disable   USB dev transceiver clock.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  BSP_USBD_EFM32_STK_Init (USBD_DRV  *p_drv)
{
    BSP_USBD_EFM32_DrvPtr = p_drv;

                                                                /* Enable USB clock                                     */
#if defined(USBC_MEM_BASE)
    // GG
    CMU->HFCORECLKEN0 |= CMU_HFCORECLKEN0_USB | CMU_HFCORECLKEN0_USBC;
    CMU_ClockSelectSet(cmuClock_USBC, cmuSelect_HFCLK);

    USB->ROUTE = USB_ROUTE_VBUSENPEN | USB_ROUTE_PHYPEN;

#else
    // GG11
    CMU_OscillatorEnable(cmuOsc_USHFRCO, true, true);
    
    CMU->HFBUSCLKEN0 |= CMU_HFBUSCLKEN0_USB;
    
    CMU->USBCTRL &= _CMU_USBCTRL_USBCLKSEL_MASK;
    CMU->USBCTRL |= CMU_USBCTRL_USBCLKSEL_USHFRCO;
    CMU->USBCTRL |= CMU_USBCTRL_USBCLKEN;
    
    USB->CTRL = USB_CTRL_LEMOSCCTRL_GATE
                | USB_CTRL_LEMIDLEEN
                | USB_CTRL_LEMPHYCTRL;
    
    USB->ROUTE = USB_ROUTE_VBUSENPEN | USB_ROUTE_PHYPEN;
#endif

                                                                /* Set handler interrupt request into interrupt channel */
    CPU_INT_SRC_HANDLER_SET_KA(BSP_INT_ID_USB,
                               BSP_USBD_EFM32_IntHandler);

    CPU_IntSrcDis(BSP_INT_ID_USB);
}


/*
*********************************************************************************************************
*                                      BSP_USBD_EFM32_STK_Conn()
*
* Description : Connect pull-up on DP.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  BSP_USBD_EFM32_STK_Conn (void)
{
    CPU_IntSrcEn(BSP_INT_ID_USB);                               /* Enable USB interrupt                                 */
}


/*
*********************************************************************************************************
*                                    BSP_USBD_EFM32_STK_Disconn()
*
* Description : Disconnect pull-up on DP.
*
* Argument(s) : none.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  BSP_USBD_EFM32_STK_Disconn (void)
{
    CPU_IntSrcDis(BSP_INT_ID_USB);                              /* Disable USB interrupt                                */
}


/*
*********************************************************************************************************
*                                     BSP_USBD_EFM32_IntHandler()
*
* Description : Provide p_drv pointer to the driver ISR Handler.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  BSP_USBD_EFM32_IntHandler (void)
{
    USBD_DRV  *p_drv;


    p_drv = BSP_USBD_EFM32_DrvPtr;
    p_drv->API_Ptr->ISR_Handler(p_drv);                         /* Call the USB Device driver ISR.                      */
}

#endif                                                          /* End of RTOS_MODULE_USB_DEV_AVAIL                     */
