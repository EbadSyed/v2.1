/*
*********************************************************************************************************
*                                              Micrium OS
*                                              CPU-Common
*
*                             (c) Copyright 2004; Silicon Laboratories Inc.
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
*                                  IAR ARM C STANDARD LIBRARY OVERRIDE
*
* File : arm_lib_mem_std_override.c
*********************************************************************************************************
* Note(s) : None.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <common_cfg.h>
#include  <rtos/common/include/lib_mem.h>

#ifdef LIB_MEM_COPY_IAR_OVERRIDE_EN

#include  <rtos_description.h>

#include  <ysizet.h>

#ifdef RTOS_MODULE_COMMON_CLK_AVAIL
#include  <time.h>
#include  <rtos/common/include/clk.h>
#include  <rtos/common/include/rtos_err.h>
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           __aeabi_memcpy4()
*
* Description : ANSI C memcpy but assume the pointers are 4-byte aligned.
*
* Argument(s) : dest    Pointer to the destination memory buffer.
*
*               src     Pointer to the source memory buffer.
*
*               n       Number of octets to copy.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

__nounwind __interwork __softfp __aapcs_core void __aeabi_memcpy4(void *dest, void const *src, unsigned int n)
{
    Mem_Copy(dest, src, n);
}


/*
*********************************************************************************************************
*                                           __aeabi_memcpy8()
*
* Description : ANSI C memcpy but assume the pointers are 8-byte aligned.
*
* Argument(s) : dest    Pointer to the destination memory buffer.
*
*               src     Pointer to the source memory buffer.
*
*               n       Number of octets to copy.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

__nounwind __interwork __softfp __aapcs_core void __aeabi_memcpy8(void *dest, void const *src, unsigned int n)
{
    Mem_Copy(dest, src, n);
}


/*
*********************************************************************************************************
*                                           __aeabi_memcpy()
*
* Description : ANSI C memcpy.
*
* Argument(s) : dest    Pointer to the destination memory buffer.
*
*               src     Pointer to the source memory buffer.
*
*               n       Number of octets to copy.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

__nounwind __interwork __softfp __aapcs_core void __aeabi_memcpy (void *dest, void const *src, unsigned int n)
{
    Mem_Copy(dest, src, n);
}


/*
*********************************************************************************************************
*                                           memmove()
*
* Description : ANSI C memmove.
*
* Argument(s) : _D      Pointer to the destination memory buffer.
*
*               _S      Pointer to the source memory buffer.
*
*               _N      Number of octets to move.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

__ATTRIBUTES void * memmove(void * _D, const void * _S, size_t _N)
{
    Mem_Move(_D, _S, _N);
    return (_D);
}


/*
*********************************************************************************************************
*                                           __time32()
*
* Description : Get the system time.
*
* Argument(s) : p   Pointer to the storage location for time.
*
* Return(s)   : Return the time as seconds.
*
* Note(s)     : None.
*********************************************************************************************************
*/
#ifdef RTOS_MODULE_COMMON_CLK_AVAIL
__ATTRIBUTES __time32_t __time32(__time32_t *p)
{
    __time32_t t;
#if (CLK_CFG_UNIX_EN == DEF_ENABLED)
    Clk_GetTS_Unix(&t);
    return (t);
#else
    RTOS_CRITICAL_FAIL_EXEC(RTOS_ERR_NOT_AVAIL, 0);
#endif
}
#endif

#endif  /* LIB_MEM_COPY_IAR_OVERRIDE_EN */
