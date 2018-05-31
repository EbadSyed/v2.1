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
*                                              TEMPLATE
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
#include  <rtos/fs/include/fs_nor_quad_spi.h>
#include  <rtos/fs/include/fs_nor.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#define  BSP_FS_NOR_QUAD_SPI_REG_BASE_ADDR      0x00000000  /* TODO: Add the QSPI controller's base address corresponding to your MCU. */


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

static  CPU_BOOLEAN  BSP_FS_NOR_QuadSPI_IntCfg    (void);

static  CPU_BOOLEAN  BSP_FS_NOR_QuadSPI_ChipSelEn (CPU_INT16U                        part_slave_id);

static  CPU_BOOLEAN  BSP_FS_NOR_QuadSPI_ChipSelDis(CPU_INT16U                        part_slave_id);

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
    .IntCfg     = BSP_FS_NOR_QuadSPI_IntCfg,
    .ChipSelEn  = BSP_FS_NOR_QuadSPI_ChipSelEn,
    .ChipSelDis = BSP_FS_NOR_QuadSPI_ChipSelDis,
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
    .DrvApiPtr            = (FS_NOR_QUAD_SPI_DRV_API *)DEF_NULL,    /* TODO place here the proper Quad SPI driver API defined in fs_nor_quad_spi_drv.h. */
    .BspApiPtr            = (FS_NOR_QUAD_SPI_BSP_API *)&QuadSPI_Ctrlr_BSP_API,
    .BaseAddr             = BSP_FS_NOR_QUAD_SPI_REG_BASE_ADDR,
    .AlignReq             = sizeof(CPU_ALIGN),
    .FlashMemMapStartAddr = 0x00000000u,
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
    PP_UNUSED_PARAM(isr_fnct);
    PP_UNUSED_PARAM(p_drv_data);

    /* TODO: create/allocate any software resources required by your BSP. */

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
    /* TODO: perform hardware configuration related to the QSPI controller's clock.*/

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
    /* TODO: perform hardware configuration related to the QSPI controller's I/Os. */

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                         BSP_FS_NOR_QuadSPI_IntCfg()
*
* Description : Initializes interrupts needed by the QSPI controller.
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

static  CPU_BOOLEAN  BSP_FS_NOR_QuadSPI_IntCfg (void)
{
    /* TODO: perform hardware configuration related to the QSPI controller's interrupts. */

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                        BSP_FS_NOR_QuadSPI_ChipSelEn()
*
* Description : Selects the NOR flash device using the Chip Select signal (see Note #1).
*
* Argument(s) : part_slave_id   slave ID of the NOR chip in case serial bus shared with other external
*                               devices (see Note #2).
*
* Return(s)   : DEF_OK,   if successful.
*               DEF_FAIL, otherwise.
*
* Note(s)     : (1) This function may be ignored if the QSPI controller allows to control directly the
*                   Chip Select from one of its registers.
*
*               (2) If the NOR flash chip is the only device connected to the serial bus, the slave
*                   ID can be ignored.
*
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_FS_NOR_QuadSPI_ChipSelEn (CPU_INT16U  part_slave_id)
{
    PP_UNUSED_PARAM(part_slave_id);                             /* See Note #2.                                         */

    /* TODO: add code to select the NOR flash device using the Chip Select signal. Usually an I/O register allows to do it. */

    return (DEF_OK);
}


/*
*********************************************************************************************************
*                                       BSP_FS_NOR_QuadSPI_ChipSelDis()
*
* Description : Unselects the NAND flash device using the Chip Select signal (see Note #1).
*
* Argument(s) : part_slave_id   slave ID of the NOR chip in case serial bus shared with other external
*                               devices (see Note #2).
*
* Return(s)   : DEF_OK,   if successful.
*               DEF_FAIL, otherwise.
*
* Note(s)     : (1) This function may be ignored if the QSPI controller allows to control directly the
*                   Chip Select from one of its registers.
*
*               (2) If the NOR flash chip is the only device connected to the serial bus, the slave
*                   ID can be ignored.
*
*********************************************************************************************************
*/

static  CPU_BOOLEAN  BSP_FS_NOR_QuadSPI_ChipSelDis (CPU_INT16U  part_slave_id)
{
    PP_UNUSED_PARAM(part_slave_id);                             /* See Note #2.                                         */

    /* TODO: add code to unselect the NOR flash device using the Chip Select signal. Usually an I/O register allows to do it. */

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
    /* TODO: Retrieve input clock frequency.*/

    return (0u);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif  /* RTOS_MODULE_FS_AVAIL && RTOS_MODULE_FS_STORAGE_NOR_AVAIL*/
