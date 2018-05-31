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
*                                         FILE SYSTEM EXAMPLE
*
* File : ex_fs_blk_dev_rd_wr.c
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

#if (defined(RTOS_MODULE_FS_AVAIL))


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <ex_description.h>

#include  "ex_fs_blk_dev_rd_wr.h"
#include  "ex_fs_utils.h"
#include  "ex_fs.h"

#include  <rtos/common/include/rtos_utils.h>

#include  <rtos/fs/include/fs_blk_dev.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#define  EX_CFG_FS_BLK_DEV_ZONE_QTY_MAX         5u              /* Entire memory size split in N zones.                 */


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
#define  EX_TRACE(...)              printf(__VA_ARGS__)
#endif


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


/*
*********************************************************************************************************
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  void  Ex_FS_BlkDevRdWr_Exec(FS_MEDIA_HANDLE   media_handle,
                                    RTOS_ERR         *p_err);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         Ex_FS_BlkDevRdWr()
*
* Description : Perform block device read/write example.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : none.
*
*********************************************************************************************************
*/

void  Ex_FS_BlkDevRdWr (void)
{
    FS_MEDIA_HANDLE  media_handle;
    RTOS_ERR         err;


    media_handle = FSMedia_Get(EX_CFG_FS_ACTIVE_MEDIA_NAME);    /* Get media handle.                                    */
    APP_RTOS_ASSERT_CRITICAL(!FS_MEDIA_HANDLE_IS_NULL(media_handle), ;);
    EX_TRACE("FS Example: Block Device rd/wr on '%s'...", EX_CFG_FS_ACTIVE_MEDIA_NAME);

                                                                /* Execute block device read/write example.             */
    Ex_FS_BlkDevRdWr_Exec(media_handle,
                          &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    EX_TRACE("OK\n\r");
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
*                                        Ex_FS_BlkDevRdWr_Exec()
*
* Description : This example will do the following main steps (see Note #1):
*
*                   (a) Write some known data in the N first sectors.
*                   (b) Read data back from the N first sectors.
*                   (c) Verify data read.
*
* Argument(s) : media_handle    Media handle.
*
*               p_err           Pointer to variable that will receive the return error code from this
*                               function.
*
* Return(s)   : none.
*
* Note(s)     : (1) **WARNING** You should run this example with extra care. The Storage layer is NOT
*                   aware of the file system formatting on the media. The example writes in the first
*                   media sectors. Thus any file system formatting information contained in some sectors
*                   may be corrupted. The directories and files may also be corrupted. If the media was
*                   formatted and the example is executed, you must re-format the media so that other
*                   examples (e.g. file read/write, file multi-descriptors, etc.) can be run.
*
*********************************************************************************************************
*/

void  Ex_FS_BlkDevRdWr_Exec (FS_MEDIA_HANDLE   media_handle,
                             RTOS_ERR         *p_err)
{
    CPU_INT08U         *p_buf;
    FS_BLK_DEV_HANDLE   blk_dev_handle;
    FS_LB_QTY           lb_cnt;
    FS_LB_SIZE          lb_size;
    CPU_INT32U          lb_ix;
    CPU_INT32U          lb_stop;
    CPU_INT08U          pattern = 0u;
    CPU_BOOLEAN         result;
    MEM_SEG_INFO        seg_info;
    CPU_SIZE_T          seg_rem_size;
    RTOS_ERR            local_err;


    RTOS_ERR_SET(local_err, RTOS_ERR_NONE);
                                                                /* ------------------ INITIALIZATION ------------------ */
    blk_dev_handle = FSBlkDev_Open(media_handle, p_err);        /* Open a block device for the given media.             */
    if (p_err->Code != RTOS_ERR_NONE) {
        EX_TRACE("Ex_FS_BlkDevRdWr(): Error opening block device w/ err %d\r\n", p_err->Code);
        return;
    }

    lb_size = FSBlkDev_LbSizeGet(blk_dev_handle, p_err);        /* Get logical block size of block device.              */
    if (p_err->Code != RTOS_ERR_NONE) {
        EX_TRACE("Ex_FS_BlkDevRdWr(): Error getting block device logical block size w/ err %d\r\n", p_err->Code);
        goto end_blk_dev;
    }
                                                                /* Ensure enough space to allocate internal resources.  */
    seg_rem_size = Mem_SegRemSizeGet(&Ex_FS_MemSeg, sizeof(CPU_ALIGN), &seg_info, &local_err);
    APP_RTOS_ASSERT_CRITICAL((local_err.Code == RTOS_ERR_NONE), ;);
    APP_RTOS_ASSERT_CRITICAL((seg_rem_size > lb_size), ;);

                                                                /* Allocate internal buffer.                            */
    p_buf = (CPU_INT08U *)Mem_SegAlloc("Ex - FS blk dev rd-wr buf",
                                       &Ex_FS_MemSeg,
                                       lb_size,
                                       p_err);
    if (p_err->Code != RTOS_ERR_NONE) {
        EX_TRACE("Ex_FS_BlkDevRdWr(): Error allocating buffer w/ err %d\r\n", p_err->Code);
        goto end_blk_dev;
    }
                                                                /* ---------------- EXAMPLE EXECUTION ----------------- */
    lb_cnt = FSBlkDev_LbCntGet(blk_dev_handle, p_err);          /* Get logical blocks count of block device.            */
    if (p_err->Code != RTOS_ERR_NONE) {
        EX_TRACE("Ex_FS_BlkDevRdWr(): Error getting block device logical block count w/ err %d\r\n", p_err->Code);
        goto end_blk_dev;
    }

    lb_stop = DEF_MIN(lb_cnt, 100u);                            /* Write/Read up to 100 logical blocks.                 */

    for (lb_ix = 0u; lb_ix < lb_stop; lb_ix++) {                /* Write blocks in first zone of media. Any...          */
                                                                /* ...formatting info from first sectors is lost.       */

        Ex_FS_BufFill(p_buf, lb_size, pattern);                 /* Write buffer with known pattern.                     */
                                                                /* Write 1 block of data.                               */
        FSBlkDev_Wr(blk_dev_handle,
                    (void *)p_buf,
                    lb_ix,                                      /* Logical block number to start writing to.            */
                    1u,                                         /* Number of logical blocks to write.                   */
                    p_err);
        if (p_err->Code != RTOS_ERR_NONE) {
            EX_TRACE("Ex_FS_BlkDevRdWr(): Error writing to device w/ err %d\r\n", p_err->Code);
            goto end_blk_dev;
        }
        pattern++;                                              /* Different pattern in each block of data.             */
    }

    pattern = 0u;
    for (lb_ix = 0u; lb_ix < lb_stop; lb_ix++) {
        Mem_Clr(p_buf, lb_size);

        FSBlkDev_Rd(blk_dev_handle,                             /* Read 1 block of data.                               */
                    (void *)p_buf,
                    lb_ix,                                      /* Logical block number to start reading from.         */
                    1u,                                         /* Number of logical blocks to read.                   */
                    p_err);
        if (p_err->Code != RTOS_ERR_NONE) {
            EX_TRACE("Ex_FS_BlkDevRdWr(): Error reading to device w/ err %d\r\n", p_err->Code);
            goto end_blk_dev;
        }
                                                                /* Verify data.                                         */
        result = Ex_FS_BufValidate(p_buf, lb_size, pattern);
        if (!result) {
            EX_TRACE("Ex_FS_BlkDevRdWr(): data read from device mismatch\r\n");
            p_err->Code = RTOS_ERR_FAIL;
            goto end_blk_dev;
        }
        pattern++;
    }
                                                                /* --------------------- CLOSING ---------------------- */
end_blk_dev:
    FSBlkDev_Close(blk_dev_handle, &local_err);                 /* Close the block device.                              */
    APP_RTOS_ASSERT_CRITICAL(local_err.Code == RTOS_ERR_NONE, ;);

    Mem_SegClr(&Ex_FS_MemSeg, &local_err);                      /* Clear memory segment for next example.               */
    APP_RTOS_ASSERT_CRITICAL(local_err.Code == RTOS_ERR_NONE, ;);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif  /* RTOS_MODULE_FS_AVAIL */


