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
*                                             IO-SD BSP
*
*                                             Template
*
* File : bsp_sd.c
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                     DEPENDENCIES & AVAIL CHECK(S)
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/common/include/rtos_path.h>
#include  <rtos_description.h>

#if  defined(RTOS_MODULE_IO_SD_AVAIL)


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/cpu/include/cpu.h>

#include  <rtos/io/include/sd.h>
#include  <rtos/io/include/sd_card.h>
#include  <rtos/drivers/io/include/sd_drv.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#define  RTOS_MODULE_CUR                RTOS_CFG_MODULE_BSP


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
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

static  SD_CARD_DRV                    *BSP_SD_SDHC_DrvPtr;
static  SD_CARD_CTRLR_ISR_HANDLE_FNCT   BSP_SD_SDHC_ISR_Fnct;


/*
*********************************************************************************************************
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_SD_SDHC_Init           (SD_CARD_CTRLR_ISR_HANDLE_FNCT   isr_fnct,
                                                 SD_CARD_DRV                    *p_ser_drv);

static  CPU_BOOLEAN  BSP_SD_SDHC_ClkEn          (void);

static  CPU_BOOLEAN  BSP_SD_SDHC_IO_Cfg         (void);

static  CPU_BOOLEAN  BSP_SD_SDHC_IntCfg         (void);

static  CPU_BOOLEAN  BSP_SD_SDHC_PwrCfg         (void);

static  CPU_BOOLEAN  BSP_SD_SDHC_Start          (void);

static  CPU_BOOLEAN  BSP_SD_SDHC_Stop           (void);

static  void         BSP_SD_SDHC_ISR_Handler    (void);

static  CPU_INT32U   BSP_SD_SDHC_ClkFreqGet     (void);

static  CPU_BOOLEAN  BSP_SD_SDHC_SignalVoltSet  (SD_CARD_BUS_SIGNAL_VOLT         volt);

static  void         BSP_SD_SDHC_CapabilitiesGet(SD_HOST_CAPABILITIES           *p_capabilities);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*********************************************************************************************************
*/

                                                                /* ---------------- BSP API STRUCTURE ----------------- */
static  const  SD_CARD_CTRLR_BSP_API  BSP_SD_SDHC_BSP_API = {
    .Init            = BSP_SD_SDHC_Init,
    .ClkEn           = BSP_SD_SDHC_ClkEn,
    .IO_Cfg          = BSP_SD_SDHC_IO_Cfg,
    .IntCfg          = BSP_SD_SDHC_IntCfg,
    .PwrCfg          = BSP_SD_SDHC_PwrCfg,
    .Start           = BSP_SD_SDHC_Start,
    .Stop            = BSP_SD_SDHC_Stop,
    .ClkFreqGet      = BSP_SD_SDHC_ClkFreqGet,
    .SignalVoltSet   = BSP_SD_SDHC_SignalVoltSet,
    .CapabilitiesGet = BSP_SD_SDHC_CapabilitiesGet,
    .BSP_API_ExtPtr  = DEF_NULL
};


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           GLOBAL CONSTANTS
*********************************************************************************************************
*********************************************************************************************************
*/

                                                                /* ---------------- DRIVER INFORMATION ---------------- */
const  SD_CARD_CTRLR_DRV_INFO  BSP_SD_SDHC_BSP_DrvInfo = {
    .BSP_API_Ptr            = &BSP_SD_SDHC_BSP_API,
    .DrvAPI_Ptr             =  DEF_NULL,                        /* TODO: Insert ptr to appropriate driver API structure.*/
    .HW_Info.BaseAddr       =  0x00000000u,
    .HW_Info.CardSignalVolt =  SD_CARD_BUS_SIGNAL_VOLT_AUTO,
    .HW_Info.InfoExtPtr     =  DEF_NULL
};


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          FUNCTION DEFINITIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/**
*********************************************************************************************************
*                                          BSP_SD_SDHC_Init()
*
* @brief    Initializes BSP.
*
* @param    isr_fnct        ISR handler function.
*
* @param    p_sd_card_drv   Pointer to sd card driver data.
*
* @return   DEF_OK, if successful,
*           DEF_FAIL, otherwise.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_SD_SDHC_Init (SD_CARD_CTRLR_ISR_HANDLE_FNCT   isr_fnct,
                                       SD_CARD_DRV                    *p_ser_drv)
{
    BSP_SD_SDHC_ISR_Fnct = isr_fnct;
    BSP_SD_SDHC_DrvPtr   = p_ser_drv;

    return (DEF_OK);
}


/**
*********************************************************************************************************
*                                          BSP_SD_SDHC_ClkEn()
*
* @brief    Enables SD controller clock.
*
* @return   DEF_OK, if successful,
*           DEF_FAIL, otherwise.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_SD_SDHC_ClkEn (void)
{
    return (DEF_OK);
}


/**
*********************************************************************************************************
*                                         BSP_SD_SDHC_IO_Cfg()
*
* @brief    Configures I/O.
*
* @return   DEF_OK, if successful,
*           DEF_FAIL, otherwise.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_SD_SDHC_IO_Cfg (void)
{
    return (DEF_OK);
}


/**
*********************************************************************************************************
*                                         BSP_SD_SDHC_IntCfg()
*
* @brief    Configures interruption.
*
* @return   DEF_OK, if successful,
*           DEF_FAIL, otherwise.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_SD_SDHC_IntCfg (void)
{
    return (DEF_OK);
}


/**
*********************************************************************************************************
*                                         BSP_SD_SDHC_PwrCfg()
*
* @brief    Configures controller power.
*
* @return   DEF_OK, if successful,
*           DEF_FAIL, otherwise.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_SD_SDHC_PwrCfg (void)
{
    return (DEF_OK);
}


/**
*********************************************************************************************************
*                                          BSP_SD_SDHC_Start()
*
* @brief    Starts controller.
*
* @return   DEF_OK, if successful,
*           DEF_FAIL, otherwise.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_SD_SDHC_Start (void)
{
    return (DEF_OK);
}


/**
*********************************************************************************************************
*                                          BSP_SD_SDHC_Stop()
*
* @brief    Stops controller.
*
* @return   DEF_OK, if successful,
*           DEF_FAIL, otherwise.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_SD_SDHC_Stop (void)
{
    return (DEF_OK);
}


/**
*********************************************************************************************************
*                                       BSP_SD_SDHC_ClkFreqGet()
*
* @brief    Gets SD clock frequency.
*
* @return   Clock frequency, in hertz.
*********************************************************************************************************
*/

static  CPU_INT32U  BSP_SD_SDHC_ClkFreqGet (void)
{
    return (0u);
}


/**
*********************************************************************************************************
*                                    BSP_SD_SDHC_SignalVoltSet()
*
* @brief    Sets the voltage level for the signaling pins.
*
* @param    volt  Voltate level to set.
*
* @return   DEF_OK, if successful,
*           DEF_FAIL, otherwise.
*
* @note     (1) Some Host Controller may need additional operations by the BSP to be able
                to change the signaling level voltage.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_SD_SDHC_SignalVoltSet (SD_CARD_BUS_SIGNAL_VOLT  volt)
{
    return (DEF_OK);
}


/**
*********************************************************************************************************
*                                    BSP_SD_SDHC_CapabilitiesGet()
*
* @brief    Gets capabilities of SD controller.
*
* @param    p_capabilities      Structure describing the capabilities of the controller.
*
* @note     (1) The capabilities are normally determined by the SD controller driver. However, in some
*               occasions, the BSP may need to override them.
*********************************************************************************************************
*/

static  void  BSP_SD_SDHC_CapabilitiesGet (SD_HOST_CAPABILITIES  *p_capabilities)
{
    PP_UNUSED_PARAM(p_capabilities);

    /* TODO: Modify p_capabilities if needed.   */
}


/**
*********************************************************************************************************
*                                       BSP_SD_SDHC_ISR_Handler()
*
* @brief    Interrupt Service Routine Handler.
*********************************************************************************************************
*/

static  void  BSP_SD_SDHC_ISR_Handler (void)
{
    BSP_SD_SDHC_ISR_Fnct(BSP_SD_SDHC_DrvPtr);
}

#endif  /* RTOS_MODULE_IO_SD_AVAIL */
