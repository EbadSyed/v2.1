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
*                                       COMMON LIB MEM EXAMPLE
*
* File : ex_common_lib_mem.h
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  _EX_COMMON_LIB_MEM_H_
#define  _EX_COMMON_LIB_MEM_H_

/*
*********************************************************************************************************
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

void  Ex_CommonLibMemSeg              (void);

void  Ex_CommonLibMemSegCreate        (void);

void  Ex_CommonLibMemDynPool          (void);

void  Ex_CommonLibMemDynPoolGetFree   (void);

void  Ex_CommonLibMemMemGetFree       (void);

void  Ex_CommonLibMemDynPoolPersistent(void);

void  Ex_CommonLibMemDynPoolHW        (void);

void  Ex_CommonLibMemSegCompile       (void);

void  Ex_CommonLibMemDynPoolRealloc   (void);

void  Ex_CommonLibMemSegInfo          (void);


#endif
