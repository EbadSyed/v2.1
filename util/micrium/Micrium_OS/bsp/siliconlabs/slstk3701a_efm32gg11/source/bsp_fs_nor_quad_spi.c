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
*                                     NOR QUAD SPI CONTROLLER BSP
*
*                                            Silicon Labs
*                                        SLSTK3701A-EFM32GG11
*
* File : bsp_fs_nor_quad_spi.c
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

#if (defined(RTOS_MODULE_FS_AVAIL) && defined(RTOS_MODULE_FS_STORAGE_NOR_AVAIL))


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/


#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/lib_def.h>
#include  <rtos/common/include/lib_mem.h>

#include  <rtos/drivers/fs/include/fs_nor_quad_spi_drv.h>
#include  <rtos/drivers/fs/include/fs_nor_quad_spi_drv_silabs_efm32gg11.h>
#include  <rtos/fs/include/fs_nor_quad_spi.h>
#include  <rtos/fs/include/fs_nor.h>

#include  <em_cmu.h>
#include  <em_gpio.h>
#include  <em_qspi.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                        LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_FS_NOR_QuadSPI_Init      (FS_NOR_QUAD_SPI_ISR_HANDLE_FNCT   isr_fnct,
                                                   void                             *p_drv_data);

static  CPU_BOOLEAN  BSP_FS_NOR_QuadSPI_ClkCfg    (void);

static  CPU_BOOLEAN  BSP_FS_NOR_QuadSPI_IO_Cfg    (void);

static  CPU_INT32U   BSP_FS_NOR_QuadSPI_ClkFreqGet(void);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

static const FS_NOR_QUAD_SPI_BSP_API  QuadSPI_Ctrlr_BSP_API = {
    .Init       = BSP_FS_NOR_QuadSPI_Init,
    .ClkCfg     = BSP_FS_NOR_QuadSPI_ClkCfg,
    .IO_Cfg     = BSP_FS_NOR_QuadSPI_IO_Cfg,
    .IntCfg     = DEF_NULL,
    .ChipSelEn  = DEF_NULL,
    .ChipSelDis = DEF_NULL,
    .ClkFreqGet = BSP_FS_NOR_QuadSPI_ClkFreqGet
};


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

const FS_NOR_QUAD_SPI_CTRLR_INFO  BSP_FS_NOR_QuadSPI_HwInfo = {
    .DrvApiPtr            = (FS_NOR_QUAD_SPI_DRV_API *)&FS_NOR_QuadSpiDrvAPI_Silabs_EFM32GG11,
    .BspApiPtr            = (FS_NOR_QUAD_SPI_BSP_API *)&QuadSPI_Ctrlr_BSP_API,
    .BaseAddr             = 0x4001C400,
    .AlignReq             = sizeof(CPU_ALIGN),
    .FlashMemMapStartAddr = FS_QSPI_SYSTEM_SPACE_START_ADDR,
    .BusWidth             = FS_NOR_SERIAL_BUS_WIDTH_SINGLE_IO
};


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      BSP_FS_NOR_QuadSPI_Init()
*
* Description : Initializes internal resources needed by the NOR QSPI BSP.
*
* Argument(s) : isr_fnct    Interrupt service routine to call when an interruption from the QSPI
*                           controller occurs.
*
*               p_drv_data  Pointer to QSPI controller driver private data. This must be passed to the
*                           interrupt service routine 'isr_fnct'.
*
* Return(s)   : DEF_OK,   if successful.
*               DEF_FAIL, otherwise.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_FS_NOR_QuadSPI_Init (FS_NOR_QUAD_SPI_ISR_HANDLE_FNCT   isr_fnct,
                                              void                             *p_drv_data)
{
    PP_UNUSED_PARAM(isr_fnct);                                  /* QSPI driver does not use interrupt for NOR xfers.    */
    PP_UNUSED_PARAM(p_drv_data);

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                      BSP_FS_NOR_QuadSPI_ClkCfg()
*
* Description : Initializes clock(s) needed by the QSPI controller.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if successful.
*               DEF_FAIL, otherwise.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_FS_NOR_QuadSPI_ClkCfg (void)
{
    CMU_ClockEnable(cmuClock_GPIO,  true);                      /* Enable the GPIO clock Peripheral.                    */
    CMU_ClockEnable(cmuClock_QSPI0, true);                      /* Enable the QSPI clock Peripheral.                    */

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                      BSP_FS_NOR_QuadSPI_IO_Cfg()
*
* Description : Initializes Input/Output needed by the QSPI controller.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if successful.
*               DEF_FAIL, otherwise.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_FS_NOR_QuadSPI_IO_Cfg (void)
{
    GPIO_PinModeSet(gpioPortG,
                    9,                                          /* MX25R Chip Select (CS#): pin 9.                      */
                    gpioModePushPull,                           /* MX25R Input.                                         */
                    DEF_OFF);                                   /* Initial setting: line is low = chip NOT selected.    */
    GPIO_PinModeSet(gpioPortG,
                    0,                                          /* MX25R Clock Input (SCLK): pin 0.                     */
                    gpioModePushPull,                           /* MX25R Input.                                         */
                    DEF_OFF);                                   /* Initial setting: line is low.                        */
    GPIO_PinModeSet(gpioPortG,
                    1,                                          /* MX25R Serial Data Input 0 (SIO0): pin 1.             */
                    gpioModePushPull,                           /* MX25R Input/Output.                                  */
                    DEF_OFF);                                   /* Initial setting: line is low.                        */
    GPIO_PinModeSet(gpioPortG,
                    2,                                          /* MX25R Serial Data Input 1 (SIO1): pin 2.             */
                    gpioModePushPull,                           /* MX25R Input/Output.                                  */
                    DEF_OFF);                                   /* Initial setting: line is low.                        */
    GPIO_PinModeSet(gpioPortG,
                    3,                                          /* MX25R Serial Data Input 2 (SIO2): pin 3.             */
                    gpioModePushPull,                           /* MX25R Input/Output.                                  */
                    DEF_OFF);                                   /* Initial setting: line is low.                        */
    GPIO_PinModeSet(gpioPortG,
                    4,                                          /* MX25R Serial Data Input 3 (SIO3): pin 4.             */
                    gpioModePushPull,                           /* MX25R Input/Output.                                  */
                    DEF_OFF);                                   /* Initial setting: line is low.                        */

    GPIO_PinModeSet(gpioPortG,
                    13,                                         /* Board QSPI power enable: pin 13.                     */
                    gpioModePushPull,
                    DEF_ON);                                    /* Initial setting: line is high = power enabled.       */

                                                                /* Cfg I/O Routing Location and Pin in QSPI controller. */
    QSPI0->ROUTELOC0 = QSPI_ROUTELOC0_QSPILOC_LOC2;             /* Location 2 used on EFM32G11 STK.                     */
                                                                /* Enable all required pins.                            */
    QSPI0->ROUTEPEN  = (QSPI_ROUTEPEN_SCLKPEN |
                        QSPI_ROUTEPEN_CS0PEN  |
                        QSPI_ROUTEPEN_DQ0PEN  |
                        QSPI_ROUTEPEN_DQ1PEN  |
                        QSPI_ROUTEPEN_DQ2PEN  |
                        QSPI_ROUTEPEN_DQ3PEN);

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                    BSP_FS_NOR_QuadSPI_ClkFreqGet()
*
* Description : Get input clock frequency of Quad SPI controller.
*
* Argument(s) : none.
*
* Return(s)   : Input clock frequency, in hertz.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

static CPU_INT32U  BSP_FS_NOR_QuadSPI_ClkFreqGet (void)
{
    CPU_INT32U  clk_freq;


                                                                /* Get input clock of QSPI controller.                  */
    clk_freq = CMU_ClockFreqGet(cmuClock_QSPI0);

    return (clk_freq);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif  /* RTOS_MODULE_FS_AVAIL && RTOS_MODULE_FS_STORAGE_NOR_AVAIL*/
