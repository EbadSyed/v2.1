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
*                                           IO-SD SDHC BSP
*
*                                            Silicon Labs
*                                              STK3701
*
* File : bsp_sd_sdhc.c
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

#include  <em_cmu.h>
#include  <em_gpio.h>
#include  "em_device.h"


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#define  RTOS_MODULE_CUR                RTOS_CFG_MODULE_BSP

#define  BSP_CM4_PERIPH_INT_ID_BASE     16u

/*
*********************************************************************************************************
*                                             SD ROUTE
*********************************************************************************************************
*/

#define  BSP_SD_ROUTE_APP_HDR         0u                        /* Route #0: SD route through application header.       */
#define  BSP_SD_ROUTE_SD_CARD_SLOT    1u                        /* Route #1: SD route through SD card slot.             */

                                                                /* TODO change this define to select SD route.          */
#define  BSP_SD_ROUTE_SEL             BSP_SD_ROUTE_SD_CARD_SLOT


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
                                                 SD_CARD_DRV                    *p_sd_card_drv);

static  CPU_BOOLEAN  BSP_SD_SDHC_ClkEn          (void);

static  CPU_BOOLEAN  BSP_SD_SDHC_IO_Cfg         (void);

static  CPU_BOOLEAN  BSP_SD_SDHC_IntCfg         (void);

static  CPU_BOOLEAN  BSP_SD_SDHC_PwrCfg         (void);

static  CPU_BOOLEAN  BSP_SD_SDHC_Start          (void);

static  CPU_BOOLEAN  BSP_SD_SDHC_Stop           (void);

static  void         BSP_SD_SDHC_ISR_Handler    (void);

static  CPU_INT32U   BSP_SD_SDHC_ClkFreqGet     (void);

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
    .SignalVoltSet   = DEF_NULL,
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
    .DrvAPI_Ptr             = &SDHC_Drv_API_Arasan,
    .HW_Info.BaseAddr       =  0x400F1000u,
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
                                       SD_CARD_DRV                    *p_sd_card_drv)
{
    BSP_SD_SDHC_ISR_Fnct = isr_fnct;
    BSP_SD_SDHC_DrvPtr   = p_sd_card_drv;

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
    CMU_OscillatorEnable(cmuOsc_HFRCO, true, true);
    CMU_ClockEnable(cmuClock_SDIOREF, true);

    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
    CMU_OscillatorEnable(cmuOsc_AUXHFRCO, true, true);
    CMU_OscillatorEnable(cmuOsc_USHFRCO, true, true);

    CMU_ClockEnable(cmuClock_GPIO, true);

    CMU->HFBUSCLKEN0 |= CMU_HFBUSCLKEN0_SDIO;

    DEF_BIT_FIELD_WR(CMU->SDIOCTRL, CMU_SDIOCTRL_SDIOCLKSEL_HFRCO, _CMU_SDIOCTRL_SDIOCLKSEL_MASK);

    DEF_BIT_CLR(CMU->SDIOCTRL, CMU_SDIOCTRL_SDIOCLKDIS);

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
    CMU_ClockEnable(cmuClock_GPIO, true );

                                                                /* Enable SD power.                                     */
    GPIO_PinModeSet(gpioPortE, 7u, gpioModePushPull, 1);
    GPIO_PinOutSet(gpioPortE, 7u);

#if (BSP_SD_ROUTE_SEL == BSP_SD_ROUTE_APP_HDR)                  /* For Application Header SD route.                     */
    GPIO_PinModeSet(gpioPortE, 8u,  gpioModePushPull, 1u);
    GPIO_PinModeSet(gpioPortE, 9u,  gpioModePushPull, 1u);
    GPIO_PinModeSet(gpioPortE, 10u, gpioModePushPull, 1u);
    GPIO_PinModeSet(gpioPortE, 11u, gpioModePushPull, 1u);

    GPIO_PinModeSet(gpioPortE, 12u, gpioModePushPull, 0u);
    GPIO_PinModeSet(gpioPortE, 13u, gpioModePushPull, 1u);

                                                                /* Set route location to 0.                             */
    SDIO->ROUTELOC0 = 0u;
    SDIO->ROUTELOC1 = 0u;
#else                                                           /* For SD card slot SD route.                           */
    GPIO_PinModeSet(gpioPortA, 3u, gpioModePushPull, 1u);
    GPIO_PinModeSet(gpioPortA, 2u, gpioModePushPull, 1u);
    GPIO_PinModeSet(gpioPortA, 1u, gpioModePushPull, 1u);
    GPIO_PinModeSet(gpioPortA, 0u, gpioModePushPull, 1u);

    GPIO_PinModeSet(gpioPortE, 15u, gpioModePushPull, 0u);
    GPIO_PinModeSet(gpioPortE, 14u, gpioModePushPull, 1u);

                                                                /* Set route location to 1.                             */
    SDIO->ROUTELOC0 = DEF_BIT_24 | DEF_BIT_16 | DEF_BIT_08 | DEF_BIT_00;
    SDIO->ROUTELOC1 = 1u;
#endif


                                                                /* Enable SD pins.                                      */
    SDIO->ROUTEPEN = DEF_BIT_00 | DEF_BIT_01 | DEF_BIT_02 | DEF_BIT_03 | DEF_BIT_04 | DEF_BIT_05;

    SDIO->CTRL = ( 0 << _SDIO_CTRL_ITAPDLYEN_SHIFT)
               | ( 0 << _SDIO_CTRL_ITAPDLYSEL_SHIFT)
               | ( 0 << _SDIO_CTRL_ITAPCHGWIN_SHIFT)
               | ( 1 << _SDIO_CTRL_OTAPDLYEN_SHIFT)
               | ( 8 << _SDIO_CTRL_OTAPDLYSEL_SHIFT);

    SDIO->CFG0 = (0x20 << _SDIO_CFG0_TUNINGCNT_SHIFT)
               | (0x30 << _SDIO_CFG0_TOUTCLKFREQ_SHIFT)
               | (1 << _SDIO_CFG0_TOUTCLKUNIT_SHIFT)
               | (0xD0 << _SDIO_CFG0_BASECLKFREQ_SHIFT)
               | (SDIO_CFG0_MAXBLKLEN_1024B)
               | (1 << _SDIO_CFG0_C8BITSUP_SHIFT)
               | (1 << _SDIO_CFG0_CADMA2SUP_SHIFT)
               | (1 << _SDIO_CFG0_CHSSUP_SHIFT)
               | (1 << _SDIO_CFG0_CSDMASUP_SHIFT)
               | (1 << _SDIO_CFG0_CSUSPRESSUP_SHIFT)
               | (1 << _SDIO_CFG0_C3P3VSUP_SHIFT)
               | (1 << _SDIO_CFG0_C3P0VSUP_SHIFT)
               | (1 << _SDIO_CFG0_C1P8VSUP_SHIFT);

    SDIO->CFG1 = (0 << _SDIO_CFG1_ASYNCINTRSUP_SHIFT)
#ifdef eMMC_CARD
               | (SDIO_CFG1_SLOTTYPE_RMSDSLOT)
#else
                | (SDIO_CFG1_SLOTTYPE_EMSDSLOT)
#endif
               | (1 << _SDIO_CFG1_CSDR50SUP_SHIFT)
               | (1 << _SDIO_CFG1_CSDR104SUP_SHIFT)
               | (1 << _SDIO_CFG1_CDDR50SUP_SHIFT)
               | (1 << _SDIO_CFG1_CDRVASUP_SHIFT)
               | (1 << _SDIO_CFG1_CDRVCSUP_SHIFT)
               | (1 << _SDIO_CFG1_CDRVDSUP_SHIFT)
               | (1 << _SDIO_CFG1_RETUNTMRCTL_SHIFT)
               | (1 << _SDIO_CFG1_TUNSDR50_SHIFT)
               | (0 << _SDIO_CFG1_RETUNMODES_SHIFT)
               | (1 << _SDIO_CFG1_SPISUP_SHIFT)
               | (1 << _SDIO_CFG1_ASYNCWKUPEN_SHIFT);

    SDIO->CFGPRESETVAL0 = (0 << _SDIO_CFGPRESETVAL0_INITSDCLKFREQ_SHIFT)
                        | (0 << _SDIO_CFGPRESETVAL0_INITCLKGENEN_SHIFT)
                        | (0 << _SDIO_CFGPRESETVAL0_INITDRVST_SHIFT)
                        | (0x4 << _SDIO_CFGPRESETVAL0_DSPSDCLKFREQ_SHIFT)
                        | (0 << _SDIO_CFGPRESETVAL0_DSPCLKGENEN_SHIFT)
                        | (0x3 << _SDIO_CFGPRESETVAL0_DSPDRVST_SHIFT);

    SDIO->CFGPRESETVAL1 = (2 << _SDIO_CFGPRESETVAL1_HSPSDCLKFREQ_SHIFT)
                        | (0 << _SDIO_CFGPRESETVAL1_HSPCLKGENEN_SHIFT)
                        | (2 << _SDIO_CFGPRESETVAL1_HSPDRVST_SHIFT)
                        | (4 << _SDIO_CFGPRESETVAL1_SDR12SDCLKFREQ_SHIFT)
                        | (0 << _SDIO_CFGPRESETVAL1_SDR12CLKGENEN_SHIFT)
                        | (1 << _SDIO_CFGPRESETVAL1_SDR12DRVST_SHIFT);

    SDIO->CFGPRESETVAL2 = (2 << _SDIO_CFGPRESETVAL2_SDR25SDCLKFREQ_SHIFT)
                        | (0 << _SDIO_CFGPRESETVAL2_SDR25CLKGENEN_SHIFT)
                        | (0 << _SDIO_CFGPRESETVAL2_SDR25DRVST_SHIFT)
                        | (1 << _SDIO_CFGPRESETVAL2_SDR50SDCLKFREQ_SHIFT)
                        | (0 << _SDIO_CFGPRESETVAL2_SDR50CLKGENEN_SHIFT)
                        | (1 << _SDIO_CFGPRESETVAL2_SDR50DRVST_SHIFT);

    SDIO->CFGPRESETVAL3 = (0 << _SDIO_CFGPRESETVAL3_SDR104SDCLKFREQ_SHIFT)
                        | (0 << _SDIO_CFGPRESETVAL3_SDR104CLKGENEN_SHIFT)
                        | (2 << _SDIO_CFGPRESETVAL3_SDR104DRVST_SHIFT)
                        | (2 << _SDIO_CFGPRESETVAL3_DDR50SDCLKFREQ_SHIFT)
                        | (0 << _SDIO_CFGPRESETVAL3_DDR50CLKGENEN_SHIFT)
                        | (3 << _SDIO_CFGPRESETVAL3_DDR50DRVST_SHIFT);

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
    CPU_INT_SRC_HANDLER_SET_KA(SDIO_IRQn + BSP_CM4_PERIPH_INT_ID_BASE,
                               BSP_SD_SDHC_ISR_Handler);

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
    GPIO_PinModeSet(gpioPortB, 10u, gpioModeInput, 0);

    CPU_IntSrcEn(SDIO_IRQn + BSP_CM4_PERIPH_INT_ID_BASE);

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
    CPU_IntSrcDis(SDIO_IRQn + BSP_CM4_PERIPH_INT_ID_BASE);

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
    CMU_HFRCOFreq_TypeDef  freq;


    freq = CMU_HFRCOFreqGet();

    return ((CPU_INT32U)freq);
}


/**
*********************************************************************************************************
*                                    BSP_SD_SDHC_CapabilitiesGet()
*
* @brief    Allows to update the Host Controller capabilities.
*
* @param    p_capabilities  Pointer to the Host Controller capabilities variable.
*
* @note     (1) This Host Controller does not support the voltage switch to 1.8V for signaling.
*********************************************************************************************************
*/

static  void  BSP_SD_SDHC_CapabilitiesGet (SD_HOST_CAPABILITIES *p_capabilities)
{
                                                                /* See Note #1.                                         */
    DEF_BIT_CLR(p_capabilities->Capabilities, SD_CAP_BUS_SIGNAL_VOLT_1_8);
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