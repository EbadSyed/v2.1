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
* File : bsp_fs_nand_ctrlr_gen.c
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

#include  <rtos/fs/include/fs_nand.h>
#include  <rtos/fs/include/fs_nand_ctrlr_gen.h>
#include  <rtos/fs/include/fs_nand_ctrlr_gen_ext_soft_ecc.h>
         
                                                                /* Third Party Library Includes                         */
#include  <em_cmu.h>
#include  <em_ebi.h>
#include  <em_gpio.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#define  RTOS_MODULE_CUR                        RTOS_CFG_MODULE_BSP


/*
*********************************************************************************************************
*                                          PORT & PIN DEFINES
*********************************************************************************************************
*/
                                                                /* ------------- 'ENABLE' PORT/PIN DEFINES ------------ */
#define  NAND_BSP_ALE_PORT                      gpioPortC       /* Port C, Pin  1 - Address Latch Enable.               */
#define  NAND_BSP_ALE_PIN                       1u

#define  NAND_BSP_CLE_PORT                      gpioPortC       /* Port C, Pin  2 - Command Latch Enable.               */
#define  NAND_BSP_CLE_PIN                       2u

#define  NAND_BSP_WP_PORT                       gpioPortD       /* Port D, Pin 13 - Write Protect.                      */
#define  NAND_BSP_WP_PIN                        13u

#define  NAND_BSP_CE_PORT                       gpioPortD       /* Port D, Pin 14 - Chip Enable.                        */
#define  NAND_BSP_CE_PIN                        14u

#define  NAND_BSP_RB_PORT                       gpioPortD       /* Port D, Pin 15 - Ready / Busy.                       */
#define  NAND_BSP_RB_PIN                        15u

#define  NAND_BSP_WE_PORT                       gpioPortF       /* Port F, Pin  8 - Write Enable.                       */
#define  NAND_BSP_WE_PIN                        8u

#define  NAND_BSP_RE_PORT                       gpioPortF       /* Port F, Pin  9 - Read Enable.                        */
#define  NAND_BSP_RE_PIN                        9u

#define  NAND_BSP_PE_PORT                       gpioPortB       /* Port B, Pin 15 - Power Enable.                       */
#define  NAND_BSP_PE_PIN                        15u

                                                                /* --------------- I/O PORT/PIN DEFINES --------------- */
#define  NAND_BSP_IOn_PORT                      gpioPortE       /* Port E - Port for All I/O Pins.                      */

#define  NAND_BSP_IO0_PIN                       8u              /* Pin  8 - I/O Pin 0. EBI_AD0.                         */
#define  NAND_BSP_IO1_PIN                       9u              /* Pin  9 - I/O Pin 1. EBI_AD1.                         */
#define  NAND_BSP_IO2_PIN                       10u             /* Pin 10 - I/O Pin 2. EBI_AD2.                         */
#define  NAND_BSP_IO3_PIN                       11u             /* Pin 11 - I/O Pin 3. EBI_AD3.                         */
#define  NAND_BSP_IO4_PIN                       12u             /* Pin 12 - I/O Pin 4. EBI_AD4.                         */
#define  NAND_BSP_IO5_PIN                       13u             /* Pin 13 - I/O Pin 5. EBI_AD5.                         */
#define  NAND_BSP_IO6_PIN                       14u             /* Pin 14 - I/O Pin 6. EBI_AD6.                         */
#define  NAND_BSP_IO7_PIN                       15u             /* Pin 15 - I/O Pin 7. EBI_AD7.                         */


/*
*********************************************************************************************************
*                                        MISCELLANEOUS DEFINES
*********************************************************************************************************
*/
                                                                /* -------- MEMORY-MAPPED REG SELECTION DEFINES ------- */
#define  NAND_BSP_ALE_SELECT                    DEF_BIT_24      /* Address Latch Bit is found on 24th Bit of Data Reg.  */
#define  NAND_BSP_CLE_SELECT                    DEF_BIT_25      /* Command Latch Bit is found on 25th Bit of Data Reg.  */

                                                                /* ------------------ TIMEOUT DEFINES ----------------- */
#define  NAND_BSP_WAIT_TIMEOUT_VAL              0xFFFFFFFFu     /* Timeout Error Value for "Wait While Busy" Function.  */


/*
*********************************************************************************************************
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/
                                                                /* ---------------- NAND FLASH DEFINES ---------------- */
static  CPU_INT32U  NAND_Flash_BaseAddr = EBI_MEM_BASE;         /* Default Base Address of Memory-Mapped NAND is BANK0  */


/*
*********************************************************************************************************
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  void  BSP_FS_NAND_Open         (RTOS_ERR      *p_err);

static  void  BSP_FS_NAND_Close        (void);

static  void  BSP_FS_NAND_ChipSelEn    (void);

static  void  BSP_FS_NAND_ChipSelDis   (void);

static  void  BSP_FS_NAND_CmdWr        (CPU_INT08U    *p_cmd,
                                        CPU_SIZE_T     cnt,
                                        RTOS_ERR      *p_err);

static  void  BSP_FS_NAND_AddrWr       (CPU_INT08U    *p_addr,
                                        CPU_SIZE_T     cnt,
                                        RTOS_ERR      *p_err);

static  void  BSP_FS_NAND_DataWr       (void          *p_src,
                                        CPU_SIZE_T     cnt,
                                        CPU_INT08U     bus_width,
                                        RTOS_ERR      *p_err);

static  void  BSP_FS_NAND_DataRd       (void          *p_dest,
                                        CPU_SIZE_T     cnt,
                                        CPU_INT08U     bus_width,
                                        RTOS_ERR      *p_err);

static  void  BSP_FS_NAND_WaitWhileBusy(void          *poll_fnct_arg,
                                        CPU_BOOLEAN  (*poll_fnct)(void  *p_arg),
                                        CPU_INT32U     to_us,
                                        RTOS_ERR      *p_err);


/*
*********************************************************************************************************
*                                         INTERFACE STRUCTURE
*********************************************************************************************************
*/
                                                                /* ---------------- NAND BSP API STRUCT --------------- */
static const  FS_NAND_CTRLR_GEN_BSP_API  BSP_FS_NAND_BSP_API = {
    .Open          = BSP_FS_NAND_Open,
    .Close         = BSP_FS_NAND_Close,
    .ChipSelEn     = BSP_FS_NAND_ChipSelEn,
    .ChipSelDis    = BSP_FS_NAND_ChipSelDis,
    .CmdWr         = BSP_FS_NAND_CmdWr,
    .AddrWr        = BSP_FS_NAND_AddrWr,
    .DataWr        = BSP_FS_NAND_DataWr,
    .DataRd        = BSP_FS_NAND_DataRd,
    .WaitWhileBusy = BSP_FS_NAND_WaitWhileBusy
};

                                                                /* ------------ CONTROLLER SPECIFIC CONFIG ------------ */
                                                                /* Ctrl Info : Soft-ECC Used, Hamming(Hsaio) ECC ref.   */
static  const  FS_NAND_CTRLR_GEN_HW_INFO  BSP_FS_NAND_CtrlrGen_HwInfo = FS_NAND_CTRLR_GEN_HW_INFO_INIT(&FS_NAND_CtrlrGen_SoftEcc_Hamming_HwInfo,
                                                                                                       &BSP_FS_NAND_BSP_API,
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
                                                                                               BSP_FS_NAND_FreeSpareMapTbl);


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
*                                           BSP_FS_NAND_Open()
*
* Description : Open (initialize) bus for NAND.
*
* Argument(s) : p_err   Pointer to variable that will receive the return error code from this function :
*
*                           RTOS_ERR_NONE            No error.
*
* Return(s)   : None.
*
* Note(s)     : (1) This function will be called EVERY time the device is opened.
*
*               (2) The NAND Chip found on this Board is the Numonyx (now Micron) "NAND256W3A", which
*                   has an 8-Bit Bus Width. For 16-Bit Bus Width, the Pin Configuration must be modified.
*
*                   (a) The NAND256W3A Chip is located at EBI Bank 0, with Base Address of 0x80000000u.
*********************************************************************************************************
*/

static  void  BSP_FS_NAND_Open (RTOS_ERR  *p_err)
{
    EBI_Init_TypeDef  EBI_NandCfg;
    
                                                                /* ------------------- CLOCK ENABLE ------------------- */
    CMU_ClockEnable(cmuClock_GPIO, DEF_ON);                     /* Clock Enable : Enable the GPIO Clock Peripheral.     */
    CMU_ClockEnable(cmuClock_EBI,  DEF_ON);                     /*              : Enable the EBI  Clock Peripheral.     */

                                                                /* ----------------- PIN CONFIGURATION ---------------- */
    GPIO_PinModeSet(NAND_BSP_ALE_PORT,                          /* ALE Pin : Address Latch Enable Pin. Port C, Pin 1.   */
                    NAND_BSP_ALE_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_OFF);                                   /*         : Initial Setting for Output, Line is Low.   */
  
    GPIO_PinModeSet(NAND_BSP_CLE_PORT,                          /* CLE Pin : Command Latch Enable Pin . Port C, Pin 2.  */
                    NAND_BSP_CLE_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_OFF);                                   /*         : Initial Setting for Output, Line is Low.   */
    
    GPIO_PinModeSet(NAND_BSP_WP_PORT,                           /* WP Pin  : Write Protect Pin. Port D, Pin 13.         */
                    NAND_BSP_WP_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_ON);                                    /*         : Active Low Write-Protect, Initially 'OFF'. */
    
    GPIO_PinModeSet(NAND_BSP_CE_PORT,                           /* CE Pin  : Chip Enable Pin. Port D, Pin 14.           */
                    NAND_BSP_CE_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_ON);                                    /*         : Active Low Chip-Enable, Initially 'OFF'.   */
    
    GPIO_PinModeSet(NAND_BSP_RB_PORT,                           /* R/B Pin : Ready / Busy Pin. Port D, Pint 15.         */
                    NAND_BSP_RB_PIN,
                    gpioModeInput,                              /*         : Input Mode Set.                            */
                    DEF_OFF);                                   /*         : Input Pin, Not Needed.                     */

    GPIO_PinModeSet(NAND_BSP_IOn_PORT,                          /* I/O Pin : I/O Pin 0 - Port E, Pin  8. EBI_AD0.       */
                    NAND_BSP_IO0_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_OFF);                                   /*         : Initial Setting for Output, Line is Low.   */
    
    GPIO_PinModeSet(NAND_BSP_IOn_PORT,                          /* I/O Pin : I/O Pin 1 - Port E, Pin  9. EBI_AD1.       */
                    NAND_BSP_IO1_PIN,                           /*         : Same as Pin Above.                         */
                    gpioModePushPull,
                    DEF_OFF);
    
    GPIO_PinModeSet(NAND_BSP_IOn_PORT,                          /* I/O Pin : I/O Pin 2 - Port E, Pin 10. EBI_AD2.       */
                    NAND_BSP_IO2_PIN,                           /*         : Same as Pin Above.                         */
                    gpioModePushPull,
                    DEF_OFF);
    
    GPIO_PinModeSet(NAND_BSP_IOn_PORT,                          /* I/O Pin : I/O Pin 3 - Port E, Pin 11. EBI_AD3.       */
                    NAND_BSP_IO3_PIN,                           /*         : Same as Pin Above.                         */
                    gpioModePushPull,
                    DEF_OFF);
    
    GPIO_PinModeSet(NAND_BSP_IOn_PORT,                          /* I/O Pin : I/O Pin 4 - Port E, Pin 12. EBI_AD4.       */
                    NAND_BSP_IO4_PIN,                           /*         : Same as Pin Above.                         */
                    gpioModePushPull,
                    DEF_OFF);
    
    GPIO_PinModeSet(NAND_BSP_IOn_PORT,                          /* I/O Pin : I/O Pin 5 - Port E, Pin 13. EBI_AD5.       */
                    NAND_BSP_IO5_PIN,                           /*         : Same as Pin Above.                         */
                    gpioModePushPull,
                    DEF_OFF);
    
    GPIO_PinModeSet(NAND_BSP_IOn_PORT,                          /* I/O Pin : I/O Pin 6 - Port E, Pin 14. EBI_AD6.       */
                    NAND_BSP_IO6_PIN,                           /*         : Same as Pin Above.                         */
                    gpioModePushPull,
                    DEF_OFF);
    
    GPIO_PinModeSet(NAND_BSP_IOn_PORT,                          /* I/O Pin : I/O Pin 7 - Port E, Pin 15. EBI_AD7.       */
                    NAND_BSP_IO7_PIN,                           /*         : Same as Pin Above.                         */
                    gpioModePushPull,
                    DEF_OFF);
    
    GPIO_PinModeSet(NAND_BSP_WE_PORT,                           /* WE Pin  : Write Enable Pin. Port F, Pin 8.           */
                    NAND_BSP_WE_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_ON);                                    /*         : Initial Setting for Output, Line is High.  */
    
    GPIO_PinModeSet(NAND_BSP_RE_PORT,                           /* RE Pin  : Read Enable Pin. Port F, Pin 9.            */
                    NAND_BSP_RE_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_ON);                                    /*         : Initial Setting for Output, Line is High.  */
    
    GPIO_PinModeSet(NAND_BSP_PE_PORT,                           /* PE Pin  : Power Enable Pin. Port B, Pin 15.          */
                    NAND_BSP_PE_PIN,
                    gpioModePushPull,                           /*         : Push Pull Mode Set (Output).               */
                    DEF_ON);                                    /*         : Initial Setting for Output, Chip is 'ON'.  */
    
                                                                /* ----------------- EBI (NAND) CONFIG ---------------- */
    EBI_NandCfg.mode               = ebiModeD8A8;               /* EBI Cfg : 8-bit Address + 8-bit Data Mode.           */
    EBI_NandCfg.ardyPolarity       = ebiActiveLow;              /*         : Address Ready     Pin Polarity - Active Low*/
    EBI_NandCfg.alePolarity        = ebiActiveLow;              /*         : Addr Latch Enable Pin Polarity - Active Low*/
    EBI_NandCfg.wePolarity         = ebiActiveLow;              /*         : Write Enable      Pin Polarity - Active Low*/
    EBI_NandCfg.rePolarity         = ebiActiveLow;              /*         : Read  Enable      Pin Polarity - Active Low*/
    EBI_NandCfg.csPolarity         = ebiActiveLow;              /*         : Chip Select       Pin Polarity - Active Low*/
    EBI_NandCfg.blPolarity         = ebiActiveLow;              /*         : Byte Lane         Pin Polarity - Active Low*/
    EBI_NandCfg.blEnable           = DEF_FALSE;                 /*         : Byte Lane Support  - Enable/Disabled.      */
    EBI_NandCfg.noIdle             = DEF_TRUE;                  /*         : Idle State "Insert" Between xfer - En/Dis. */
    EBI_NandCfg.ardyEnable         = DEF_FALSE;                 /*         : Address Ready Support - Disable, Use GPIO. */
    EBI_NandCfg.ardyDisableTimeout = DEF_TRUE;                  /*         : Turn Off 32-Cycle Timeout Ability for ARDY.*/
    EBI_NandCfg.banks              = EBI_BANK0;                 /*         : Address Bank 0 Select.                     */
    EBI_NandCfg.csLines            = 0u;                        /*         : Chip Select Line 0 - Selected.             */
    EBI_NandCfg.addrSetupCycles    = 0u;                        /*         : Cycles Delayed after ALE is Set.           */
    EBI_NandCfg.addrHoldCycles     = 0u;                        /*         : Cycles Addr is Driven onto ADDRDAT B4 ALE. */
    EBI_NandCfg.addrHalfALE        = DEF_FALSE;                 /*         : Disable Half-Cycle ALE Strobe in Addr Cycle*/
    EBI_NandCfg.readSetupCycles    = 0u;                        /*         : Cycles for Addr setup before RE is Set.    */
    EBI_NandCfg.readStrobeCycles   = 2u;                        /*         : Cycles for RE to be Held Active.           */
    EBI_NandCfg.readHoldCycles     = 1u;                        /*         : Cycles for CS to be Active after RE is Clr.*/
    EBI_NandCfg.readPageMode       = DEF_FALSE;                 /*         : Disable Page Mode Reads.                   */
    EBI_NandCfg.readPrefetch       = DEF_FALSE;                 /*         : Disable Prefetching from Seq Address.      */
    EBI_NandCfg.readHalfRE         = DEF_FALSE;                 /*         : Disable Half-Cycle duration of RE Strobe.  */
    EBI_NandCfg.writeSetupCycles   = 0u;                        /*         : Cycles for Addr Setup before WE is Set.    */
    EBI_NandCfg.writeStrobeCycles  = 2u;                        /*         : Cycles for WE to be Held Active.           */
    EBI_NandCfg.writeHoldCycles    = 1u;                        /*         : Cycles for CS to be Active after WE is Clr.*/
    EBI_NandCfg.writeBufferDisable = DEF_FALSE;                 /*         : Enable the Write Buffer.                   */
    EBI_NandCfg.writeHalfWE        = DEF_FALSE;                 /*         : Disable Half-Cycle duration of WE Strobe.  */
    EBI_NandCfg.aLow               = ebiALowA24;                /*         : Lower Addr Pin Limit - Addr 24 and Higher. */
    EBI_NandCfg.aHigh              = ebiAHighA26;               /*         : High  Addr Pin Limit - Addr 26 and Higher. */
    EBI_NandCfg.location           = ebiLocation1;              /*         : Use Pin Location #1.                       */
    EBI_NandCfg.enable             = DEF_TRUE;                  /*         : EBI should be Enabled after Configuration. */

    EBI_Init(&EBI_NandCfg);                                     /* Initialize the EBI Peripheral based on Config.       */
    
    NAND_Flash_BaseAddr = EBI_BankAddress(EBI_NandCfg.banks);   /* Base Addr of Memory-Mapped NAND Device (See Note #2a)*/
    
    EBI->CMD      &= ~EBI_CMD_ECCSTART;                         /* Disable MCU-Side ECC Generation.                     */
    EBI->NANDCTRL  = (EBI_NANDCTRL_BANKSEL_BANK0 |              /* Memory Bank 0 is Connected to NAND Flash Device.     */
                      EBI_NANDCTRL_EN);                         /* Enable NAND Flash Control for the Memory Bank.       */
    
    RTOS_ERR_SET(*p_err, RTOS_ERR_NONE);                        /* Set Error Return to _ERR_NONE, everything OK.        */
}


/*
*********************************************************************************************************
*                                        BSP_FS_NAND_Close()
*
* Description : Close (uninitialize) bus for NAND.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : (1) This function will be called EVERY time the device is closed.
*********************************************************************************************************
*/

static  void  BSP_FS_NAND_Close (void)
{
                                                                /* &&&& Optional uninit.                                */
}


/*
*********************************************************************************************************
*                                        BSP_FS_NAND_ChipSelEn()
*
* Description : Enable NAND chip select/enable.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : (1) If you are sharing the bus/hardware with other software, this function MUST get an
*                   exclusive resource lock and configure any peripheral that could have been setup
*                   differently by the other software.
*********************************************************************************************************
*/

static  void  BSP_FS_NAND_ChipSelEn (void)
{
                                                                /* En chip sel & optionally acquire lock.               */
    GPIO_PinOutClear(NAND_BSP_CE_PORT, NAND_BSP_CE_PIN);        /* Clr 'Chip Enable' Pin (Active Low - Port D, Pin 14). */
}


/*
*********************************************************************************************************
*                                      BSP_FS_NAND_ChipSelDis()
*
* Description : Disable NAND chip select/enable.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : (1) If you are sharing the bus/hardware with other software, this function MUST release
*                   the exclusive resource lock obtained in function BSP_FS_NAND_ChipSelEn().
*********************************************************************************************************
*/

static  void  BSP_FS_NAND_ChipSelDis (void)
{
                                                                /* Dis chip sel & optionally release lock.              */
    GPIO_PinOutSet(NAND_BSP_CE_PORT, NAND_BSP_CE_PIN);          /* Set 'Chip Enable' Pin (Active Low - Port D, Pin 14). */
}


/*
*********************************************************************************************************
*                                          BSP_FS_NAND_CmdWr()
*
* Description : Write command to NAND.
*
* Argument(s) : p_cmd   Pointer to buffer that holds command.
*
*               cnt     Number of octets to write.
*
*               p_err   Pointer to variable that will receive the return error code from this function :
*
*                           RTOS_ERR_NONE            No error.
*
* Return(s)   : None.
*
* Note(s)     : (1) This function must send 'cnt' octets on the data bus of the NAND. The pins of this
*                   bus are usually labeled IO0_0 - IO7_0 or DQ0_0 - DQ7_0 in the datasheet of the
*                   NAND flash, as specified by the ONFI 3.0 specification. The CLE (Command Latch Enable)
*                   pin must also be set high to indicate the data is an address.
*********************************************************************************************************
*/

static  void  BSP_FS_NAND_CmdWr (CPU_INT08U  *p_cmd,
                                 CPU_SIZE_T   cnt,
                                 RTOS_ERR    *p_err)
{
    CPU_INT32U  *p_nand_cmd;
    
                                                                /* Memory-Mapped Address w/ CLE Bit to Write to Cmd Reg.*/
    p_nand_cmd = (CPU_INT32U *)(NAND_Flash_BaseAddr + NAND_BSP_CLE_SELECT);
    
    Mem_Copy((void *)p_nand_cmd, (void *)p_cmd, cnt);           /* Write Command to NAND Mem Location, with needed 'cnt'*/

    RTOS_ERR_SET(*p_err, RTOS_ERR_NONE);                        /* Set Error Return to _ERR_NONE, everything OK.        */
}


/*
*********************************************************************************************************
*                                          BSP_FS_NAND_AddrWr()
*
* Description : Write address to NAND.
*
* Argument(s) : p_addr  Pointer to buffer that holds address.
*
*               cnt     Number of octets to write (size of the address).
*
*               p_err   Pointer to variable that will receive the return error code from this function :
*
*                           RTOS_ERR_NONE            No error.
*
* Return(s)   : None.
*
* Note(s)     : (1) This function must send 'cnt' octets on the data bus of the NAND. The pins of this
*                   bus are usually labeled IO0_0 - IO7_0 or DQ0_0 - DQ7_0 in the datasheet of the
*                   NAND flash, as specified by the ONFI 3.0 specification. The ALE (Address Latch Enable)
*                   pin must also be set high to indicate the data is an address.
*********************************************************************************************************
*/

static void  BSP_FS_NAND_AddrWr (CPU_INT08U  *p_addr,
                                 CPU_SIZE_T   cnt,
                                 RTOS_ERR    *p_err)
{
    CPU_INT32U  *p_nand_addr;

                                                                /* Memory-Mapped Address w/ ALE Bit to Write to Addr Reg*/
    p_nand_addr = (CPU_INT32U *)(NAND_Flash_BaseAddr + NAND_BSP_ALE_SELECT);
    
    Mem_Copy((void *)p_nand_addr, (void *)p_addr, cnt);         /* Write Address to NAND Mem Location, with needed 'cnt'*/
    
    RTOS_ERR_SET(*p_err, RTOS_ERR_NONE);                        /* Set Error Return to _ERR_NONE, everything OK.        */
}


/*
*********************************************************************************************************
*                                          BSP_FS_NAND_DataWr()
*
* Description : Write data to NAND.
*
* Argument(s) : p_src       Pointer to source memory buffer.
*
*               cnt         Number of octets to write.
*
*               bus_width   Parallel bus width. Usually 8 or 16 bits.
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                           RTOS_ERR_NONE            No error.
*
* Return(s)   : None.
*
* Note(s)     : (1) This function must send 'cnt' octets on the data bus of the NAND. The pins of this
*                   bus are usually labeled IO0_0 - IO7_0 or DQ0_0 - DQ7_0 in the datasheet of the
*                   NAND flash, as specified by the ONFI 3.0 specification. The CLE (Command Latch Enable)
*                   and ALE (Address Latch Enabled) pins must both be set low.
*
*               (2) When calling this function, the generic controller layer has already sent the proper
*                   address and command to the NAND Flash. The purpose of this function is only to write
*                   data at a specific address.
*********************************************************************************************************
*/

static  void  BSP_FS_NAND_DataWr (void        *p_src,
                                  CPU_SIZE_T   cnt,
                                  CPU_INT08U   bus_width,
                                  RTOS_ERR    *p_err)
{
    CPU_INT32U  *p_nand_data;


    PP_UNUSED_PARAM(bus_width);                                 /* NAND BSP Pre-Configured to 8-Bit Bus Width.          */
    
    p_nand_data = (CPU_INT32U *)NAND_Flash_BaseAddr;            /* Memory-Mapped Address w/ no Bits to Write to Data Reg*/

    Mem_Copy((void *)p_nand_data, p_src, cnt);                  /* Write Source Data to NAND Mem Location, w/ 'cnt'     */
    
    RTOS_ERR_SET(*p_err, RTOS_ERR_NONE);                        /* Set Error Return to _ERR_NONE, everything OK.        */
}


/*
*********************************************************************************************************
*                                          BSP_FS_NAND_DataRd()
*
* Description : Read data from NAND.
*
* Argument(s) : p_dest      Pointer to destination memory buffer.
*
*               cnt         Number of octets to read.
*
*               bus_width   Parallel bus width. Usually 8 or 16 bits.
*
*               p_err       Pointer to variable that will receive the return error code from this function :
*
*                           RTOS_ERR_NONE            No error.
*
* Return(s)   : None.
*
* Note(s)     : (1) This function must read 'cnt' octets on the data bus of the NAND. The pins of this
*                   bus are usually labeled IO0_0 - IO7_0 or DQ0_0 - DQ7_0 in the datasheet of the
*                   NAND flash, as specified by the ONFI 3.0 specification.
*
*               (2) When calling this function, the generic controller layer has already sent the proper
*                   address and command to the NAND Flash. The purpose of this function is only to read
*                   data at a specific address.
*********************************************************************************************************
*/

static  void  BSP_FS_NAND_DataRd (void        *p_dest,
                                  CPU_SIZE_T   cnt,
                                  CPU_INT08U   bus_width,
                                  RTOS_ERR    *p_err)
{
    CPU_INT32U  *p_nand_data;


    PP_UNUSED_PARAM(bus_width);                                 /* NAND BSP Pre-Configured to 8-Bit Bus Width.          */
    
    p_nand_data = (CPU_INT32U *)NAND_Flash_BaseAddr;            /* Memory-Mapped Addr w/ no Bits to Read from Data Reg. */

    Mem_Copy(p_dest, (void *)p_nand_data, cnt);                 /* Read Data from NAND Mem Location to Destination.     */
    
    RTOS_ERR_SET(*p_err, RTOS_ERR_NONE);                        /* Set Error Return to _ERR_NONE, everything OK.        */
}


/*
*********************************************************************************************************
*                                      BSP_FS_NAND_WaitWhileBusy()
*
* Description : Wait while NAND is busy.
*
* Argument(s) : poll_fnct_arg   Pointer to argument that must be passed to the poll_fnct, if needed.
*
*               poll_fnct       Pointer to function to poll, if there is no hardware ready/busy signal.
*
*               to_us           Timeout, in microseconds.
*
*               p_err           Pointer to variable that will receive the return error code from this function :
*
*                                   RTOS_ERR_NONE            No error.
*                                   RTOS_ERR_TIMEOUT         NAND did not respond in time.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  BSP_FS_NAND_WaitWhileBusy (void          *poll_fnct_arg,
                                         CPU_BOOLEAN  (*poll_fnct)(void  *p_arg),
                                         CPU_INT32U     to_us,
                                         RTOS_ERR      *p_err)
{
    CPU_INT32U  timeout_val;
    
    
    PP_UNUSED_PARAM(poll_fnct_arg);
    PP_UNUSED_PARAM(poll_fnct);
	PP_UNUSED_PARAM(to_us);
    
    timeout_val = NAND_BSP_WAIT_TIMEOUT_VAL;                    /* Init Var(s).                                         */
    
                                                                /* Wait until the EBI AHB Trasaction is Done. Used ...  */
                                                                /* ... when 'Write Buffer' is Enabled.                  */
    while ((DEF_BIT_IS_SET(EBI->STATUS, EBI_STATUS_AHBACT) == DEF_YES) &&
           (timeout_val                                     > 0u     )) {
        timeout_val--;
    }
    
    if (timeout_val == 0u) {
        RTOS_ERR_SET(*p_err, RTOS_ERR_TIMEOUT);                 /* Timeout Occurred, EBI AHB Transaction didn't finish. */
        return;
    }
    
    timeout_val = NAND_BSP_WAIT_TIMEOUT_VAL;                    /* Reset the Timeout Value.                             */
    
                                                                /* Wait until the Read/Busy Pin becomes 'HIGH', NAND ...*/
                                                                /* ... has Finished Transaction.                        */
    while ((DEF_BIT_IS_CLR(GPIO->P[NAND_BSP_RB_PORT].DIN, (1u << NAND_BSP_RB_PIN)) == DEF_YES) &&
           (timeout_val                                                             > 0u     )) {
        timeout_val--;
    }
    
    if (timeout_val == 0u) {
        RTOS_ERR_SET(*p_err, RTOS_ERR_TIMEOUT);                 /* Timeout Occurred, EBI AHB Transaction didn't finish. */
        return;
    }
    
    RTOS_ERR_SET(*p_err, RTOS_ERR_NONE);                        /* Set Error Return to _ERR_NONE, everything OK.        */
}


#endif
