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
*                                     NAND MEMORY CONTROLLER BSP
*
*                                              Silicon Labs
*                                            EFM32GG-STK3700a
*
* File : bsp_fs_nand.c
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

#if (defined(RTOS_MODULE_FS_AVAIL) && defined(RTOS_MODULE_FS_STORAGE_NAND_AVAIL))


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/lib_def.h>

#include  <rtos/drivers/fs/include/fs_nand_ctrlr_drv.h>
#include  <rtos/fs/include/fs_nand_ctrlr_gen.h>
#include  <rtos/fs/include/fs_nand_ctrlr_gen_ext_soft_ecc.h>
         
                                                                /* Third Party Library Includes                         */
#include  <em_cmu.h>
#include  <em_gpio.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            LOCAL DEFINES
*
* Note(s) : (1) The NAND256W3A Chip is located at EBI Bank 0, with Base Address of 0x80000000u.
*********************************************************************************************************
*********************************************************************************************************
*/

#define  RTOS_MODULE_CUR                        RTOS_CFG_MODULE_BSP

#define  BSP_EBI_BASE_ADDR                      EBI_MEM_BASE


/*
*********************************************************************************************************
*                                          PORT & PIN DEFINES
*********************************************************************************************************
*/

                                                                /* ------------- 'ENABLE' PORT/PIN DEFINES ------------ */
#define  BSP_NAND_ALE_PORT                      gpioPortC       /* Port C, Pin  1 - Address Latch Enable.               */
#define  BSP_NAND_ALE_PIN                       1u

#define  BSP_NAND_CLE_PORT                      gpioPortC       /* Port C, Pin  2 - Command Latch Enable.               */
#define  BSP_NAND_CLE_PIN                       2u

#define  BSP_NAND_WP_PORT                       gpioPortD       /* Port D, Pin 13 - Write Protect.                      */
#define  BSP_NAND_WP_PIN                        13u

#define  BSP_NAND_CE_PORT                       gpioPortD       /* Port D, Pin 14 - Chip Enable.                        */
#define  BSP_NAND_CE_PIN                        14u

#define  BSP_NAND_RB_PORT                       gpioPortD       /* Port D, Pin 15 - Ready / Busy.                       */
#define  BSP_NAND_RB_PIN                        15u

#define  BSP_NAND_WE_PORT                       gpioPortF       /* Port F, Pin  8 - Write Enable.                       */
#define  BSP_NAND_WE_PIN                        8u

#define  BSP_NAND_RE_PORT                       gpioPortF       /* Port F, Pin  9 - Read Enable.                        */
#define  BSP_NAND_RE_PIN                        9u

#define  BSP_NAND_PE_PORT                       gpioPortB       /* Port B, Pin 15 - Power Enable.                       */
#define  BSP_NAND_PE_PIN                        15u

                                                                /* --------------- I/O PORT/PIN DEFINES --------------- */
#define  BSP_NAND_IOn_PORT                      gpioPortE       /* Port E - Port for All I/O Pins.                      */

#define  BSP_NAND_IO0_PIN                       8u              /* Pin  8 - I/O Pin 0. EBI_AD0.                         */
#define  BSP_NAND_IO1_PIN                       9u              /* Pin  9 - I/O Pin 1. EBI_AD1.                         */
#define  BSP_NAND_IO2_PIN                       10u             /* Pin 10 - I/O Pin 2. EBI_AD2.                         */
#define  BSP_NAND_IO3_PIN                       11u             /* Pin 11 - I/O Pin 3. EBI_AD3.                         */
#define  BSP_NAND_IO4_PIN                       12u             /* Pin 12 - I/O Pin 4. EBI_AD4.                         */
#define  BSP_NAND_IO5_PIN                       13u             /* Pin 13 - I/O Pin 5. EBI_AD5.                         */
#define  BSP_NAND_IO6_PIN                       14u             /* Pin 14 - I/O Pin 6. EBI_AD6.                         */
#define  BSP_NAND_IO7_PIN                       15u             /* Pin 15 - I/O Pin 7. EBI_AD7.                         */


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
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_FS_NAND_Init      (FS_NAND_CTRLR_DRV_ISR_HANDLE_FNCT   isr_fnct,
                                            FS_NAND_CTRLR_DRV                  *p_drv);

static  CPU_BOOLEAN  BSP_FS_NAND_ClkCfg    (void);

static  CPU_BOOLEAN  BSP_FS_NAND_IO_Cfg    (void);

static  CPU_BOOLEAN  BSP_FS_NAND_ChipSelEn (CPU_INT16U                          part_slave_id);

static  CPU_BOOLEAN  BSP_FS_NAND_ChipSelDis(CPU_INT16U                          part_slave_id);

static  CPU_BOOLEAN  BSP_FS_NAND_IsChipRdy (void);


/*
*********************************************************************************************************
*                                         INTERFACE STRUCTURE
*********************************************************************************************************
*/
                                                                /* ---------------- NAND BSP API STRUCT --------------- */
static const  FS_NAND_CTRLR_BSP_API  BSP_FS_NAND_BSP_API = {
    .Init       = BSP_FS_NAND_Init,
    .ClkCfg     = BSP_FS_NAND_ClkCfg,
    .IO_Cfg     = BSP_FS_NAND_IO_Cfg,
    .IntCfg     = DEF_NULL,
    .ChipSelEn  = BSP_FS_NAND_ChipSelEn,
    .ChipSelDis = BSP_FS_NAND_ChipSelDis,
    .IsChipRdy  = BSP_FS_NAND_IsChipRdy
};

                                                                /* ------------ CONTROLLER SPECIFIC CONFIG ------------ */
                                                                /* Ctrl Info : Soft-ECC Used, Hamming(Hsaio) ECC ref.   */
static  const  FS_NAND_CTRLR_GEN_HW_INFO  BSP_FS_NAND_CtrlrGen_HwInfo = FS_NAND_CTRLR_GEN_HW_INFO_INIT(&FS_NAND_CtrlrGen_SoftEcc_Hamming_HwInfo,
                                                                                                       &FS_NAND_CtrlDrvAPI_Silabs_EFM32GG_EBI,
                                                                                                       &BSP_FS_NAND_BSP_API,
                                                                                                       BSP_EBI_BASE_ADDR,
                                                                                                       sizeof(CPU_ALIGN));

                                                                /* --------------- CHIP SPECIFIC CONFIG --------------- */
                                                                /* Chip Info : 'STATIC' Configuration Specifications.   */
static  const  FS_NAND_PART_PARAM         BSP_FS_NAND_PartParam_Cfg = {
        2048u,                                                  /* BlkCnt           : Total Number of Blocks            */
        32u,                                                    /* PgPerBlk         : Number of Pages per Block         */
        512u,                                                   /* PgSize           : Size (in Octets) of each Page     */
        16u,                                                    /* SpareSize        : Size (Octets) of Spare Area Per Pg*/
        DEF_NO,                                                 /* SupportsRndPgPgm : Supports Random Page Programming  */
        3u,                                                     /* NbrPgmPerPg      : Nbr of Program Operations Per Pg  */
        8u,                                                     /* BusWidth         : Bus Width of NAND Device          */
        1u,                                                     /* ECC_NbrCorrBits  : 1-Bit Correction, 2-Bit Detection */
        1u,                                                     /* ECC_CodewordSize : Size (in Bytes) of _NbrCorrBits.  */
        DEFECT_SPARE_B_6_W_1_PG_1_OR_2,                         /* DefectMarkType   : 6th Byte in Spare of 1st Pg != FFh*/
        40u,                                                    /* MaxBadBlkCnt     : Max Number of Bad Blocks in Device*/
        100000u                                                 /* MaxBlkErase      : Max Number of Erase OPs per Blk   */
    };

                                                                /* Chip Info : Free Spare Map for Micron - NAND256W3A.  */
static  const  FS_NAND_FREE_SPARE_DATA    BSP_FS_NAND_FreeSpareMapTbl[] =  {{  1u,  15u},
                                                                            { -1 , -1 }};

                                                                /* Chip Info : NAND Memory Chip Config Structure.       */
static  const  FS_NAND_PART_HW_INFO       BSP_FS_NAND_Part_HwInfo = FS_NAND_PART_HW_INFO_INIT(&BSP_FS_NAND_PartParam_Cfg,
                                                                                              BSP_FS_NAND_FreeSpareMapTbl,
                                                                                              0u);


                                                                /* ------------- RTOS-FS SPECIFIC HW INFO ------------- */
                                                                /* HW Info : NAND Controller + Chip Specific Config(s). */
        const  FS_NAND_HW_INFO            BSP_FS_NAND_HwInfo = FS_NAND_HW_INFO_INIT(&BSP_FS_NAND_CtrlrGen_HwInfo,
                                                                                    &BSP_FS_NAND_Part_HwInfo);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           BSP_FS_NAND_Init()
*
* Description : Initializes internal resources needed by the NAND BSP.
*
* Argument(s) : isr_fnct    Interrupt service routine to call when an interruption from the NAND memory
*                           controller occurs.
*
*               p_drv       Pointer to NAND memory controller driver data structure. This must be passed
*                           to the interrupt service routine 'isr_fnct'.
*
* Return(s)   : DEF_OK,   if successful.
*               DEF_FAIL, otherwise.
*
* Note(s)     : (1) This function will be called EVERY time the device is opened.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_FS_NAND_Init  (FS_NAND_CTRLR_DRV_ISR_HANDLE_FNCT   isr_fnct,
                                        FS_NAND_CTRLR_DRV                  *p_drv)
{
    PP_UNUSED_PARAM(isr_fnct);                                  /* EBI driver does not use interrupt for NAND xfers.    */
    PP_UNUSED_PARAM(p_drv);

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                         BSP_FS_NAND_ClkCfg()
*
* Description : Initializes clock(s) needed by the NAND memory controller.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if successful.
*               DEF_FAIL, otherwise.
*
* Note(s)     : (1) This function will be called EVERY time the device is opened.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_FS_NAND_ClkCfg (void)
{
                                                                /* ------------------- CLOCK ENABLE ------------------- */
    CMU_ClockEnable(cmuClock_GPIO, DEF_ON);                     /* Clock Enable : Enable the GPIO Clock Peripheral.     */
    CMU_ClockEnable(cmuClock_EBI,  DEF_ON);                     /*              : Enable the EBI  Clock Peripheral.     */


    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                         BSP_FS_NAND_IO_Cfg()
*
* Description : Initializes Input/Output needed by the NAND memory controller.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if successful.
*               DEF_FAIL, otherwise.
*
* Note(s)     : (1) This function will be called EVERY time the device is opened.
*
*               (2) The NAND Chip found on the EFM32GG-STK3700a board is the Numonyx (now Micron)
*                   "NAND256W3A", which has an 8-bit bus width. For 16-bit bus width, the pin
*                   configuration must be modified.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_FS_NAND_IO_Cfg (void)
{
                                                                /* ----------------- PIN CONFIGURATION ---------------- */
                                                                /* See Note #2.                                         */
    GPIO_PinModeSet(BSP_NAND_ALE_PORT,                          /* ALE Pin : Address Latch Enable Pin. Port C, Pin 1.   */
                    BSP_NAND_ALE_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_OFF);                                   /*         : Initial Setting for Output, Line is Low.   */

    GPIO_PinModeSet(BSP_NAND_CLE_PORT,                          /* CLE Pin : Command Latch Enable Pin . Port C, Pin 2.  */
                    BSP_NAND_CLE_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_OFF);                                   /*         : Initial Setting for Output, Line is Low.   */

    GPIO_PinModeSet(BSP_NAND_WP_PORT,                           /* WP Pin  : Write Protect Pin. Port D, Pin 13.         */
                    BSP_NAND_WP_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_ON);                                    /*         : Active Low Write-Protect, Initially 'OFF'. */

    GPIO_PinModeSet(BSP_NAND_CE_PORT,                           /* CE Pin  : Chip Enable Pin. Port D, Pin 14.           */
                    BSP_NAND_CE_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_ON);                                    /*         : Active Low Chip-Enable, Initially 'OFF'.   */

    GPIO_PinModeSet(BSP_NAND_RB_PORT,                           /* R/B Pin : Ready / Busy Pin. Port D, Pint 15.         */
                    BSP_NAND_RB_PIN,
                    gpioModeInput,                              /*         : Input Mode Set.                            */
                    DEF_OFF);                                   /*         : Input Pin, Not Needed.                     */

    GPIO_PinModeSet(BSP_NAND_IOn_PORT,                          /* I/O Pin : I/O Pin 0 - Port E, Pin  8. EBI_AD0.       */
                    BSP_NAND_IO0_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_OFF);                                   /*         : Initial Setting for Output, Line is Low.   */

    GPIO_PinModeSet(BSP_NAND_IOn_PORT,                          /* I/O Pin : I/O Pin 1 - Port E, Pin  9. EBI_AD1.       */
                    BSP_NAND_IO1_PIN,                           /*         : Same as Pin Above.                         */
                    gpioModePushPull,
                    DEF_OFF);

    GPIO_PinModeSet(BSP_NAND_IOn_PORT,                          /* I/O Pin : I/O Pin 2 - Port E, Pin 10. EBI_AD2.       */
                    BSP_NAND_IO2_PIN,                           /*         : Same as Pin Above.                         */
                    gpioModePushPull,
                    DEF_OFF);

    GPIO_PinModeSet(BSP_NAND_IOn_PORT,                          /* I/O Pin : I/O Pin 3 - Port E, Pin 11. EBI_AD3.       */
                    BSP_NAND_IO3_PIN,                           /*         : Same as Pin Above.                         */
                    gpioModePushPull,
                    DEF_OFF);

    GPIO_PinModeSet(BSP_NAND_IOn_PORT,                          /* I/O Pin : I/O Pin 4 - Port E, Pin 12. EBI_AD4.       */
                    BSP_NAND_IO4_PIN,                           /*         : Same as Pin Above.                         */
                    gpioModePushPull,
                    DEF_OFF);

    GPIO_PinModeSet(BSP_NAND_IOn_PORT,                          /* I/O Pin : I/O Pin 5 - Port E, Pin 13. EBI_AD5.       */
                    BSP_NAND_IO5_PIN,                           /*         : Same as Pin Above.                         */
                    gpioModePushPull,
                    DEF_OFF);

    GPIO_PinModeSet(BSP_NAND_IOn_PORT,                          /* I/O Pin : I/O Pin 6 - Port E, Pin 14. EBI_AD6.       */
                    BSP_NAND_IO6_PIN,                           /*         : Same as Pin Above.                         */
                    gpioModePushPull,
                    DEF_OFF);

    GPIO_PinModeSet(BSP_NAND_IOn_PORT,                          /* I/O Pin : I/O Pin 7 - Port E, Pin 15. EBI_AD7.       */
                    BSP_NAND_IO7_PIN,                           /*         : Same as Pin Above.                         */
                    gpioModePushPull,
                    DEF_OFF);

    GPIO_PinModeSet(BSP_NAND_WE_PORT,                           /* WE Pin  : Write Enable Pin. Port F, Pin 8.           */
                    BSP_NAND_WE_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_ON);                                    /*         : Initial Setting for Output, Line is High.  */

    GPIO_PinModeSet(BSP_NAND_RE_PORT,                           /* RE Pin  : Read Enable Pin. Port F, Pin 9.            */
                    BSP_NAND_RE_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_ON);                                    /*         : Initial Setting for Output, Line is High.  */

    GPIO_PinModeSet(BSP_NAND_PE_PORT,                           /* PE Pin  : Power Enable Pin. Port B, Pin 15.          */
                    BSP_NAND_PE_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_ON);                                    /*         : Initial Setting for Output, Chip is 'ON'.  */

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                        BSP_FS_NAND_ChipSelEn()
*
* Description : Selects the NAND flash device using the Chip Enable signal.
*
* Argument(s) : part_slave_id   slave ID of the NAND chip in case parallel bus shared with other external
*                               devices. See Note #1.
*
* Return(s)   : DEF_OK,   if successful.
*               DEF_FAIL, otherwise.
*
* Note(s)     : (1) If the NAND flash chip is the only device connected to the parallel bus, the slave
*                   ID can be ignored.
*
*               (2) The NAND flash is the only external parallel device connected to the EFM32GG MCU EBI
*                   peripheral on the EFM32GG-STK3700a board. Thus the slave ID can be ignored.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_FS_NAND_ChipSelEn (CPU_INT16U  part_slave_id)
{
    PP_UNUSED_PARAM(part_slave_id);                             /* See Note #2.                                         */

    GPIO_PinOutClear(BSP_NAND_CE_PORT, BSP_NAND_CE_PIN);        /* Clr 'Chip Enable' Pin (Active Low - Port D, Pin 14). */

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                       BSP_FS_NAND_ChipSelDis()
*
* Description : Unselects the NAND flash device using the Chip Enable signal.
*
* Argument(s) : part_slave_id   slave ID of the NAND chip in case parallel bus shared with other external
*                               devices. See Note #1.
*
* Return(s)   : DEF_OK,   if successful.
*               DEF_FAIL, otherwise.
*
* Note(s)     : (1) If the NAND flash chip is the only device connected to the parallel bus, the slave
*                   ID can be ignored.
*
*               (2) The NAND flash is the only external parallel device connected to the EFM32GG MCU EBI
*                   peripheral on the EFM32GG-STK3700a board. Thus the slave ID can be ignored.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_FS_NAND_ChipSelDis (CPU_INT16U  part_slave_id)
{
    PP_UNUSED_PARAM(part_slave_id);                             /* See Note #2.                                         */

    GPIO_PinOutSet(BSP_NAND_CE_PORT, BSP_NAND_CE_PIN);          /* Set 'Chip Enable' Pin (Active Low - Port D, Pin 14). */

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                        BSP_FS_NAND_IsChipRdy()
*
* Description : Determines if the NAND flash device is ready using the Ready/Busy signal.
*
* Argument(s) : none.
*
* Return(s)   : DEF_YES, if chip is ready.
*               DEF_NO,  otherwise.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_FS_NAND_IsChipRdy (void)
{
    CPU_BOOLEAN  rdy = DEF_NO;


                                                                /* Test Ready/Busy output of NAND chip.                 */
    if (DEF_BIT_IS_CLR(GPIO->P[BSP_NAND_RB_PORT].DIN, (1u << BSP_NAND_RB_PIN)) == DEF_YES) {
        rdy = DEF_NO;                                           /* Ready/Busy low: rd, pgm or erase op in progress.     */
    } else {
        rdy = DEF_YES;                                          /* Ready/Busy high: rd, pgm or erase op complete.       */
    }

    return (rdy);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif  /* RTOS_MODULE_FS_AVAIL && RTOS_MODULE_FS_STORAGE_NAND_AVAIL*/

