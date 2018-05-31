/*
*********************************************************************************************************
*                                              Micrium OS
*                                                Common
*
*                             (c) Copyright 2017; Silicon Laboratories Inc.
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
*                                       COMMON BASIC OPERATIONS
*
* File : common.c
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos_description.h>

#include  <rtos/cpu/include/cpu.h>

#include  <rtos/common/include/common.h>
#include  <rtos/common/include/rtos_path.h>
#include  <rtos/common/include/lib_math.h>
#include  <rtos/common/include/lib_mem.h>
#include  <rtos/common/include/platform_mgr.h>

#if (RTOS_CFG_LOG_EN == DEF_ENABLED)
#include  <rtos/common/include/logging.h>
#endif

#include  <rtos/common/source/rtos/rtos_utils_priv.h>
#include  <rtos/common/source/platform_mgr/platform_mgr_priv.h>

#if defined(RTOS_MODULE_COMMON_CLK_AVAIL)
#include  <common_cfg.h>
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#define  LOG_DFLT_CH                       (COMMON)
#define  RTOS_MODULE_CUR                    RTOS_CFG_MODULE_COMMON

#if (RTOS_CFG_LOG_EN == DEF_ENABLED)
#define  COMMON_INIT_CFG_DFLT               {                                   \
                                                .CommonMemSegPtr = DEF_NULL,    \
                                                .LoggingCfg =                   \
                                                {                               \
                                                    .AsyncBufSize = 512u        \
                                                },                              \
                                                .LoggingMemSegPtr = DEF_NULL    \
                                            }
#else
#define  COMMON_INIT_CFG_DFLT               {                                   \
                                                .CommonMemSegPtr = DEF_NULL     \
                                            }
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

#if (RTOS_CFG_EXTERNALIZE_OPTIONAL_CFG_EN == DEF_DISABLED)
static  CPU_BOOLEAN      Common_IsInit      = DEF_NO;

const   COMMON_INIT_CFG  Common_InitCfgDflt = COMMON_INIT_CFG_DFLT;
static  COMMON_INIT_CFG  Common_InitCfg     = COMMON_INIT_CFG_DFLT;
#else
extern  const  COMMON_INIT_CFG  Common_InitCfg;
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       Common_ConfigureLogging()
*
* Description : Configure the properties of the logging module.
*
* Argument(s) : p_cfg   Pointer to the structure containing the new logging parameters.
*
* Return(s)   : None.
*
* Note(s)     : (1) This function is optional. If it is called, it must be called before Common_Init().
*                   If it is not called, default values will be used to initialize the module.
*********************************************************************************************************
*/

#if ((RTOS_CFG_LOG_EN                      == DEF_ENABLED) && \
     (RTOS_CFG_EXTERNALIZE_OPTIONAL_CFG_EN == DEF_DISABLED))
void  Common_ConfigureLogging (COMMON_CFG_LOGGING  *p_cfg)
{
    RTOS_ASSERT_DBG((Common_IsInit == DEF_FALSE), RTOS_ERR_ALREADY_INIT, ;);

    RTOS_ASSERT_DBG((p_cfg != DEF_NULL), RTOS_ERR_NULL_PTR, ;);


    Common_InitCfg.LoggingCfg = *p_cfg;
}
#endif


/*
*********************************************************************************************************
*                                    Common_ConfigureMemSegLogging()
*
* Description : Configure the memory segment that will be used to allocate internal data needed by the
*               logging module instead of the default memory segment.
*
* Argument(s) : p_mem_seg   Pointer to the memory segment from which the internal data will be allocated.
*                           If DEF_NULL, the internal data will be allocated from the global Heap.
*
* Return(s)   : None.
*
* Note(s)     : (1) This function is optional. If it is called, it must be called before Common_Init().
*                   If it is not called, default values will be used to initialize the module.
*********************************************************************************************************
*/

#if ((RTOS_CFG_LOG_EN                      == DEF_ENABLED) && \
     (RTOS_CFG_EXTERNALIZE_OPTIONAL_CFG_EN == DEF_DISABLED))
void  Common_ConfigureMemSegLogging (MEM_SEG  *p_mem_seg)
{
    RTOS_ASSERT_DBG((Common_IsInit == DEF_FALSE), RTOS_ERR_ALREADY_INIT, ;);


    Common_InitCfg.LoggingMemSegPtr = p_mem_seg;
}
#endif


/*
*********************************************************************************************************
*                                       Common_ConfigureMemSeg()
*
* Description : Configure the memory segment that will be used to allocate internal data needed by Common
*               modules instead of the default memory segment.
*
* Argument(s) : p_mem_seg   Pointer to the memory segment from which the internal data will be allocated.
*                           If DEF_NULL, the internal data will be allocated from the global Heap.
*
* Return(s)   : None.
*
* Note(s)     : (1) This function is optional. If it is called, it must be called before Common_Init().
*                   If it is not called, default values will be used to initialize the module.
*********************************************************************************************************
*/

#if (RTOS_CFG_EXTERNALIZE_OPTIONAL_CFG_EN == DEF_DISABLED)
void  Common_ConfigureMemSeg (MEM_SEG  *p_mem_seg)
{
    RTOS_ASSERT_DBG((Common_IsInit == DEF_FALSE), RTOS_ERR_ALREADY_INIT, ;);


    Common_InitCfg.CommonMemSegPtr = p_mem_seg;
}
#endif


/*
*********************************************************************************************************
*                                            Common_Init()
*
* Description : Initializes all the common modules (Lib Mem, Lib Math, Logging, KAL, platform manager) in
*               the correct order with the specified configurations.
*
* Argument(s) : p_err   Pointer to the variable that will receive one of the following error code(s) from
*                       this function:
*
*                           RTOS_ERR_NONE
*                           RTOS_ERR_BLK_ALLOC_CALLBACK
*                           RTOS_ERR_SEG_OVF
*
* Return(s)   : None.
*
* Note(s)     : (1) The functions Common_Configure...() can be used to configure more specific properties
*                   of the Common module, when RTOS_CFG_EXTERNALIZE_OPTIONAL_CFG_EN is set to
*                   DEF_DISABLED. If set to DEF_ENABLED, the structure Common_InitCfg needs to be
*                   declared and filled by the application to configure these specific properties for the
*                   module.
*********************************************************************************************************
*/

void  Common_Init (RTOS_ERR  *p_err)
{
#if (RTOS_CFG_EXTERNALIZE_OPTIONAL_CFG_EN == DEF_DISABLED)
    Common_IsInit = DEF_YES;
#endif


    RTOS_ASSERT_DBG_ERR_PTR_VALIDATE(p_err, ;);
    RTOS_ERR_SET(*p_err, RTOS_ERR_NONE);

    Mem_Init();
    Math_Init();


#if (RTOS_CFG_LOG_EN == DEF_ENABLED)
    Log_Init((COMMON_CFG_LOGGING *)(&(Common_InitCfg.LoggingCfg)),
             p_err);
    if (RTOS_ERR_CODE_GET(*p_err) != RTOS_ERR_NONE) {
        return;
    }
#endif

#if ((defined(RTOS_MODULE_COMMON_CLK_AVAIL) && (CLK_CFG_EXT_EN == DEF_DISABLED) && (CLK_CFG_SIGNAL_EN == DEF_DISABLED)) || \
     defined(RTOS_MODULE_COMMON_SHELL_AVAIL) || \
     defined(RTOS_MODULE_FS_AVAIL)           || \
     defined(RTOS_MODULE_USB_HOST_AVAIL)     || \
     defined(RTOS_MODULE_USB_DEV_AVAIL)      || \
     defined(RTOS_MODULE_IO_AVAIL)           || \
     defined(RTOS_MODULE_NET_AVAIL))
    KAL_Init(p_err);
    if (RTOS_ERR_CODE_GET(*p_err) != RTOS_ERR_NONE) {
        return;
    }
#endif

    PlatformMgrInit(p_err);
    if (RTOS_ERR_CODE_GET(*p_err) != RTOS_ERR_NONE) {
        return;
    }
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           PRIVATE FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                     Common_MemSegLoggingPtrGet()
*
* Description : Obtain the memory segment that must be used for logging.
*
* Argument(s) : None.
*
* Return(s)   : Pointer to memory segment that loggin must use.
*
* Note(s)     : None.
*********************************************************************************************************
*/

#if (RTOS_CFG_LOG_EN == DEF_ENABLED)
MEM_SEG  *Common_MemSegLoggingPtrGet(void)
{
    return (Common_InitCfg.LoggingMemSegPtr);
}
#endif


/*
*********************************************************************************************************
*                                         Common_MemSegPtrGet()
*
* Description : Obtain the memory segment that must be used by Common modules.
*
* Argument(s) : None.
*
* Return(s)   : Pointer to memory segment that Common modules must use.
*
* Note(s)     : None.
*********************************************************************************************************
*/

MEM_SEG  *Common_MemSegPtrGet(void)
{
    return (Common_InitCfg.CommonMemSegPtr);
}
