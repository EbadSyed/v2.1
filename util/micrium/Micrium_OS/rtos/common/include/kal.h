/*
*********************************************************************************************************
*                                              Micrium OS
*                                                Common
*
*                             (c) Copyright 2013; Silicon Laboratories Inc.
*                                        https://www.micrium.com
*
*********************************************************************************************************
* Licensing:
*           YOUR USE OF THIS SOFTWARE IS SUBJECT TO THE TERMS OF A MICRIUM SOFTWARE LICENSE.
*   If you are not willing to accept the terms of an appropriate Micrium Software License, you must not
*   download or use this software for any reason.
*   Information about Micrium software licensing is available at https://www.micrium.com/licensing/
*   It is your obligation to select an appropriate license based on your intended use of the Micrium OS.
*   Unless you have executed a Micrium Commercial License, your use of the Micrium OS is limited to
*   evaluation, educational or personal non-commercial uses. The Micrium OS may not be redistributed or
*   disclosed to any third party without the written consent of Silicon Laboratories Inc.
*********************************************************************************************************
* Documentation:
*   You can find user manuals, API references, release notes and more at: https://doc.micrium.com
*********************************************************************************************************
* Technical Support:
*   Support is available for commercially licensed users of Micrium's software. For additional
*   information on support, you can contact info@micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                   KERNEL ABSTRACTION LAYER (KAL)
*
* File : kal.h
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  _KAL_H_
#define  _KAL_H_


/*
*********************************************************************************************************
*********************************************************************************************************
*                                     DEPENDENCIES & AVAIL CHECK(S)
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos_description.h>

#if ((!defined(RTOS_MODULE_KERNEL_OS2_AVAIL)) && \
     (!defined(RTOS_MODULE_KERNEL_OS3_AVAIL)) && \
     (!defined(RTOS_MODULE_KERNEL_AVAIL)))
#error Micrium OS requires either uC/OS-II, uC/OS-III or the Micrium OS Kernel. Make sure one of them is \
       part of your project and that either RTOS_MODULE_KERNEL_OS2_AVAIL or RTOS_MODULE_KERNEL_OS3_AVAIL \
       is defined in rtos_description.h.
#else


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/common/include/rtos_err.h>
#include  <rtos/common/include/lib_mem.h>

#if (defined(RTOS_MODULE_KERNEL_OS2_AVAIL))
#include  <ucos_ii.h>
#elif (defined(RTOS_MODULE_KERNEL_OS3_AVAIL))
#include  <os.h>
#elif (defined(RTOS_MODULE_KERNEL_AVAIL))
#include  <rtos/kernel/include/os.h>
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                              DATA TYPES
*********************************************************************************************************
*********************************************************************************************************
*/

typedef  OS_PRIO  KAL_TASK_PRIO;


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif  /* ((!defined(RTOS_MODULE_KERNEL_OS2_AVAIL)) && (!defined(RTOS_MODULE_KERNEL_OS3_AVAIL))) */


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif                                                          /* End of kal module include.                           */
