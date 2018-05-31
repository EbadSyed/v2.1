/*
*********************************************************************************************************
*                                              Micrium OS
*                                                Common
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
*                                            CUSTOM LIBRARY
*
* File : lib_utils.h
*********************************************************************************************************
* Note(s) : (1) This file is intended to regroup LIB capabilities that depends on CPU elements.
*               'lib_def.h' should be used if CPU is not needed.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  _LIB_UTILS_H_
#define  _LIB_UTILS_H_


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/cpu/include/cpu_def.h>

#include  <rtos/common/include/rtos_path.h>
#include  <cpu_cfg.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                                DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             BIT MACRO'S
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              DEF_BIT()
*
* Description : Create bit mask with single, specified bit set.
*
* Argument(s) : bit         Bit number of bit to set.
*
* Return(s)   : Bit mask with single, specified bit set.
*
* Note(s)     : (1) 'bit' SHOULD be a non-negative integer.
*
*               (2) 'bit' values that overflow the target CPU &/or compiler environment (e.g. negative
*                   or greater-than-CPU-data-size values) MAY generate compiler warnings &/or errors.
*********************************************************************************************************
*/

#define  DEF_BIT(bit)                                                   (1u << (bit))


/*
*********************************************************************************************************
*                                             DEF_BITxx()
*
* Description : Create bit mask of specified bit size with single, specified bit set.
*
* Argument(s) : bit         Bit number of bit to set.
*
* Return(s)   : Bit mask with single, specified bit set.
*
* Note(s)     : (1) 'bit' SHOULD be a non-negative integer.
*
*               (2) 'bit' values that overflow the target CPU &/or compiler environment (e.g. negative
*                   or greater-than-CPU-data-size values) MAY generate compiler warnings &/or errors.
*
*                   To avoid overflowing any target CPU &/or compiler's integer data type, unsigned
*                   bit constant '1' is cast to specified integer data type size.
*
*               (3) Ideally, DEF_BITxx() macro's should be named DEF_BIT_xx(); however, these names already
*                   previously-released for bit constant #define's (see 'STANDARD DEFIN   BIT DEFINES').
*********************************************************************************************************
*/

#define  DEF_BIT08(bit)                        ((CPU_INT08U)((CPU_INT08U)1u  << (bit)))

#define  DEF_BIT16(bit)                        ((CPU_INT16U)((CPU_INT16U)1u  << (bit)))

#define  DEF_BIT32(bit)                        ((CPU_INT32U)((CPU_INT32U)1u  << (bit)))

#define  DEF_BIT64(bit)                        ((CPU_INT64U)((CPU_INT64U)1u  << (bit)))


/*
*********************************************************************************************************
*                                           DEF_BIT_MASK()
*
* Description : Shift a bit mask.
*
* Argument(s) : bit_mask    Bit mask to shift.
*
*               bit_shift   Number of bit positions to left-shift bit mask.
*
* Return(s)   : Shifted bit mask.
*
* Note(s)     : (1) 'bit_mask'  SHOULD be an unsigned    integer.
*
*                   'bit_shift' SHOULD be a non-negative integer.
*
*               (2) 'bit_shift' values that overflow the target CPU &/or compiler environment (e.g. negative
*                   or greater-than-CPU-data-size values) MAY generate compiler warnings &/or errors.
*********************************************************************************************************
*/

#define  DEF_BIT_MASK(bit_mask, bit_shift)                                     ((bit_mask) << (bit_shift))


/*
*********************************************************************************************************
*                                          DEF_BIT_MASK_xx()
*
* Description : Shift a bit mask of specified bit size.
*
* Argument(s) : bit_mask    Bit mask to shift.
*
*               bit_shift   Number of bit positions to left-shift bit mask.
*
* Return(s)   : Shifted bit mask.
*
* Note(s)     : (1) 'bit_mask'  SHOULD be an unsigned    integer.
*
*               (2) 'bit_shift' SHOULD be a non-negative integer.
*
*               (3) 'bit_shift' values that overflow the target CPU &/or compiler environment (e.g. negative
*                   or greater-than-CPU-data-size values) MAY generate compiler warnings &/or errors.
*********************************************************************************************************
*/

#define  DEF_BIT_MASK_08(bit_mask, bit_shift)         ((CPU_INT08U)((CPU_INT08U)(bit_mask) << (bit_shift)))

#define  DEF_BIT_MASK_16(bit_mask, bit_shift)         ((CPU_INT16U)((CPU_INT16U)(bit_mask) << (bit_shift)))

#define  DEF_BIT_MASK_32(bit_mask, bit_shift)         ((CPU_INT32U)((CPU_INT32U)(bit_mask) << (bit_shift)))

#define  DEF_BIT_MASK_64(bit_mask, bit_shift)         ((CPU_INT64U)((CPU_INT64U)(bit_mask) << (bit_shift)))


/*
*********************************************************************************************************
*                                           DEF_BIT_FIELD()
*
* Description : Create & shift a contiguous bit field.
*
* Argument(s) : bit_field   Number of contiguous bits to set in the bit field.
*
*               bit_shift   Number of bit positions   to left-shift bit field.
*
* Return(s)   : Shifted bit field.
*
* Note(s)     : (1) 'bit_field' & 'bit_shift' SHOULD be non-negative integers.
*
*               (2) 'bit_field'/'bit_shift' values that overflow the target CPU &/or compiler
*                   environment (e.g. negative or greater-than-CPU-data-size values) MAY generate
*                   compiler warnings &/or errors.
*
*                   (a) To avoid overflowing any target CPU &/or compiler's integer data type, unsigned
*                       bit constant '1' is suffixed with 'L'ong integer modifier.
*
*                       This may still be insufficient for CPUs &/or compilers that support 'long long'
*                       integer data types, in which case 'LL' integer modifier should be suffixed.
*                       However, since almost all 16- & 32-bit CPUs & compilers support 'long' integer
*                       data types but many may NOT support 'long long' integer data types, only 'long'
*                       integer data types & modifiers are supported.
*********************************************************************************************************
*/

#define  DEF_BIT_FIELD(bit_field, bit_shift)                                 ((((bit_field) >= DEF_INT_CPU_NBR_BITS) ? (DEF_INT_CPU_U_MAX_VAL)     \
                                                                                                                     : (DEF_BIT(bit_field) - 1uL)) \
                                                                                                                            << (bit_shift))

/*
*********************************************************************************************************
*                                         DEF_BIT_FIELD_xx()
*
* Description : Create & shift a contiguous bit field of specified bit size.
*
* Argument(s) : bit_field   Number of contiguous bits to set in the bit field.
*
*               bit_shift   Number of bit positions   to left-shift bit field.
*
* Return(s)   : Shifted bit field.
*
* Note(s)     : (1) 'bit_field' & 'bit_shift' SHOULD be non-negative integers.
*
*               (2) 'bit_field'/'bit_shift' values that overflow the target CPU &/or compiler
*                   environment (e.g. negative or greater-than-CPU-data-size values) MAY generate
*                   compiler warnings &/or errors.
*
*                   To avoid overflowing any target CPU &/or compiler's integer data type, unsigned
*                   bit constant '1' is cast to specified integer data type size.
*********************************************************************************************************
*/

#define  DEF_BIT_FIELD_08(bit_field, bit_shift)     ((CPU_INT08U)((((CPU_INT08U)(bit_field) >= (CPU_INT08U)DEF_INT_08_NBR_BITS) ? (CPU_INT08U)(DEF_INT_08U_MAX_VAL)                    \
                                                                                                                                : (CPU_INT08U)(DEF_BIT08(bit_field) - (CPU_INT08U)1u)) \
                                                                                                                                                     << (bit_shift)))

#define  DEF_BIT_FIELD_16(bit_field, bit_shift)     ((CPU_INT16U)((((CPU_INT16U)(bit_field) >= (CPU_INT16U)DEF_INT_16_NBR_BITS) ? (CPU_INT16U)(DEF_INT_16U_MAX_VAL)                    \
                                                                                                                                : (CPU_INT16U)(DEF_BIT16(bit_field) - (CPU_INT16U)1u)) \
                                                                                                                                                     << (bit_shift)))

#define  DEF_BIT_FIELD_32(bit_field, bit_shift)     ((CPU_INT32U)((((CPU_INT32U)(bit_field) >= (CPU_INT32U)DEF_INT_32_NBR_BITS) ? (CPU_INT32U)(DEF_INT_32U_MAX_VAL)                    \
                                                                                                                                : (CPU_INT32U)(DEF_BIT32(bit_field) - (CPU_INT32U)1u)) \
                                                                                                                                                     << (bit_shift)))

#define  DEF_BIT_FIELD_64(bit_field, bit_shift)     ((CPU_INT64U)((((CPU_INT64U)(bit_field) >= (CPU_INT64U)DEF_INT_64_NBR_BITS) ? (CPU_INT64U)(DEF_INT_64U_MAX_VAL)                    \
                                                                                                                                : (CPU_INT64U)(DEF_BIT64(bit_field) - (CPU_INT64U)1u)) \
                                                                                                                                                     << (bit_shift)))


/*
*********************************************************************************************************
*                                            DEF_BIT_SET()
*
* Description : Set specified bit(s) in a value.
*
* Argument(s) : val         Value to modify by setting specified bit(s).
*
*               mask        Mask of bits to set.
*
* Return(s)   : Modified value with specified bit(s) set.
*
* Note(s)     : (1) 'val' & 'mask' SHOULD be unsigned integers.
*********************************************************************************************************
*/

#define  DEF_BIT_SET(val, mask)                        ((val) = ((val) | (mask)))


/*
*********************************************************************************************************
*                                          DEF_BIT_SET_xx()
*
* Description : Set specified bit(s) in a value of specified bit size.
*
* Argument(s) : val         Value to modify by setting specified bit(s).
*
*               mask        Mask of bits to set.
*
* Return(s)   : Modified value with specified bit(s) set.
*
* Note(s)     : (1) 'val' & 'mask' SHOULD be unsigned integers.
*
*               (2) These macros are deprecated and should be replaced by the DEF_BIT_SET macro.
*********************************************************************************************************
*/

#define  DEF_BIT_SET_08(val, mask)                     DEF_BIT_SET((val), (mask))

#define  DEF_BIT_SET_16(val, mask)                     DEF_BIT_SET((val), (mask))

#define  DEF_BIT_SET_32(val, mask)                     DEF_BIT_SET((val), (mask))

#define  DEF_BIT_SET_64(val, mask)                     DEF_BIT_SET((val), (mask))


/*
*********************************************************************************************************
*                                          DEF_BIT_CLR_xx()
*
* Description : Clear specified bit(s) in a value of specified bit size.
*
* Argument(s) : val         Value to modify by clearing specified bit(s).
*
*               mask        Mask of bits to clear.
*
* Return(s)   : Modified value with specified bit(s) clear.
*
* Note(s)     : (1) 'val' & 'mask' SHOULD be unsigned integers.
*********************************************************************************************************
*/

#define  DEF_BIT_CLR_08(val, mask)                     ((val) = ((val) & ((CPU_INT08U)(~((unsigned int)(mask))))))

#if (CPU_CFG_DATA_SIZE > CPU_WORD_SIZE_16)
#define  DEF_BIT_CLR_16(val, mask)                     ((val) = ((val) & ((CPU_INT16U)(~((unsigned int)(mask))))))
#else
#define  DEF_BIT_CLR_16(val, mask)                     ((val) = ((val) & ((CPU_INT16U)(~((CPU_INT16U)(mask))))))
#endif

#if (CPU_CFG_DATA_SIZE > CPU_WORD_SIZE_32)
#define  DEF_BIT_CLR_32(val, mask)                     ((val) = ((val) & ((CPU_INT32U)(~((unsigned int)(mask))))))
#else
#define  DEF_BIT_CLR_32(val, mask)                     ((val) = ((val) & ((CPU_INT32U)(~((CPU_INT32U)(mask))))))
#endif

#define  DEF_BIT_CLR_64(val, mask)                     ((val) = ((val) & ((CPU_INT64U)(~((CPU_INT64U)(mask))))))


/*
*********************************************************************************************************
*                                            DEF_BIT_CLR()
*
* Description : Clear specified bit(s) in a value.
*
* Argument(s) : val         Value to modify by clearing specified bit(s).
*
*               mask        Mask of bits to clear.
*
* Return(s)   : Modified value with specified bit(s) clear.
*
* Note(s)     : (1) 'val' & 'mask' SHOULD be unsigned integers.
*********************************************************************************************************
*/

#if     (CPU_CFG_DATA_SIZE_MAX == CPU_WORD_SIZE_08)

#define  DEF_BIT_CLR(val, mask)                 ((sizeof(val) == CPU_WORD_SIZE_08) ? DEF_BIT_CLR_08((val), (mask)) : 0)


#elif   (CPU_CFG_DATA_SIZE_MAX == CPU_WORD_SIZE_16)

#define  DEF_BIT_CLR(val, mask)                 ((sizeof(val) == CPU_WORD_SIZE_08) ? DEF_BIT_CLR_08((val), (mask)) :   \
                                                ((sizeof(val) == CPU_WORD_SIZE_16) ? DEF_BIT_CLR_16((val), (mask)) : 0))


#elif   (CPU_CFG_DATA_SIZE_MAX == CPU_WORD_SIZE_32)

#define  DEF_BIT_CLR(val, mask)                 ((sizeof(val) == CPU_WORD_SIZE_08) ? DEF_BIT_CLR_08((val), (mask)) :    \
                                                ((sizeof(val) == CPU_WORD_SIZE_16) ? DEF_BIT_CLR_16((val), (mask)) :    \
                                                ((sizeof(val) == CPU_WORD_SIZE_32) ? DEF_BIT_CLR_32((val), (mask)) : 0)))


#elif   (CPU_CFG_DATA_SIZE_MAX == CPU_WORD_SIZE_64)

#define  DEF_BIT_CLR(val, mask)                 ((sizeof(val) == CPU_WORD_SIZE_08) ? DEF_BIT_CLR_08((val), (mask)) :     \
                                                ((sizeof(val) == CPU_WORD_SIZE_16) ? DEF_BIT_CLR_16((val), (mask)) :     \
                                                ((sizeof(val) == CPU_WORD_SIZE_32) ? DEF_BIT_CLR_32((val), (mask)) :     \
                                                ((sizeof(val) == CPU_WORD_SIZE_64) ? DEF_BIT_CLR_64((val), (mask)) : 0))))

#else

#error  CPU_CFG_DATA_SIZE_MAX illegally #defined in '[arch]_cpu_port.h'. \
		[See '[arch]_cpu_port.h CONFIGURATION ERRORS']

#endif


/*
*********************************************************************************************************
*                                            DEF_BIT_TOGGLE()
*
* Description : Toggles specified bit(s) in a value.
*
* Argument(s) : val         Value to modify by toggling specified bit(s).
*
*               mask        Mask of bits to toggle.
*
* Return(s)   : Modified value with specified bit(s) toggled.
*
* Note(s)     : (1) 'val' & 'mask' SHOULD be unsigned integers.
*********************************************************************************************************
*/

#define  DEF_BIT_TOGGLE(val, mask)                      ((val) ^= (mask))


/*
*********************************************************************************************************
*                                           DEF_BIT_FIELD_RD()
*
* Description : Reads a 'val' field, masked and shifted, given by mask 'field_mask'.
*
* Argument(s) : val         Value to read from.
*
*               field_mask  Mask of field to read. See note #1, #2 and #3.
*
* Return(s)   : Field value, masked and right-shifted to bit position 0.
*
* Note(s)     : (1) 'field_mask' argument must NOT be 0.
*
*               (2) 'field_mask' argument must contain a mask with contiguous set bits.
*
*               (3) 'val' & 'field_mask' SHOULD be unsigned integers.
*********************************************************************************************************
*/

#define  DEF_BIT_FIELD_RD(val, field_mask)              (((val) & (field_mask)) / ((field_mask) & ((~(field_mask)) + 1u)))


/*
*********************************************************************************************************
*                                          DEF_BIT_FIELD_ENC()
*
* Description : Encodes given 'field_val' at position given by mask 'field_mask'.
*
* Argument(s) : field_val   Value to encode.
*
*               field_mask  Mask of field to read. See note #1 and #2.
*
* Return(s)   : Field value, masked and left-shifted to field position.
*
* Note(s)     : (1) 'field_mask' argument must contain a mask with contiguous set bits.
*
*               (2) 'field_val' & 'field_mask' SHOULD be unsigned integers.
*********************************************************************************************************
*/

#define  DEF_BIT_FIELD_ENC(field_val, field_mask)       (((field_val) * ((field_mask) & ((~(field_mask)) + 1u))) & (field_mask))


/*
*********************************************************************************************************
*                                           DEF_BIT_FIELD_WR()
*
* Description : Writes 'field_val' field at position given by mask 'field_mask' in variable 'var'.
*
* Argument(s) : var         Variable to write field to. See note #2.
*
*               field_val   Desired value for field. See note #2.
*
*               field_mask  Mask of field to write to. See note #1 and #2.
*
* Return(s)   : None.
*
* Note(s)     : (1) 'field_mask' argument must contain a mask with contiguous set bits.
*
*               (2) 'var', 'field_val' & 'field_mask' SHOULD be unsigned integers.
*********************************************************************************************************
*/

#define  DEF_BIT_FIELD_WR(var, field_val, field_mask)   (var) = (((var) & ~(field_mask)) | DEF_BIT_FIELD_ENC((field_val), (field_mask)))


/*
*********************************************************************************************************
*                                          DEF_BIT_IS_SET()
*
* Description : Determine if specified bit(s) in a value are set.
*
* Argument(s) : val         Value to check for specified bit(s) set.
*
*               mask        Mask of bits to check if set (see Note #2).
*
* Return(s)   : DEF_YES, if ALL specified bit(s) are     set in value.
*
*               DEF_NO,  if ALL specified bit(s) are NOT set in value.
*
* Note(s)     : (1) 'val' & 'mask' SHOULD be unsigned integers.
*
*               (2) NULL 'mask' allowed; returns 'DEF_NO' since NO mask bits specified.
*********************************************************************************************************
*/

#define  DEF_BIT_IS_SET(val, mask)                    (((((val) & (mask)) == (mask)) && \
                                                         ((mask)          !=  0u))    ? (DEF_YES) : (DEF_NO))


/*
*********************************************************************************************************
*                                          DEF_BIT_IS_CLR()
*
* Description : Determine if specified bit(s) in a value are clear.
*
* Argument(s) : val         Value to check for specified bit(s) clear.
*
*               mask        Mask of bits to check if clear (see Note #2).
*
* Return(s)   : DEF_YES, if ALL specified bit(s) are     clear in value.
*
*               DEF_NO,  if ALL specified bit(s) are NOT clear in value.
*
* Note(s)     : (1) 'val' & 'mask' SHOULD be unsigned integers.
*
*               (2) NULL 'mask' allowed; returns 'DEF_NO' since NO mask bits specified.
*********************************************************************************************************
*/

#define  DEF_BIT_IS_CLR(val, mask)                    (((((val) & (mask)) ==  0u)  && \
                                                         ((mask)          !=  0u))  ? (DEF_YES) : (DEF_NO))


/*
*********************************************************************************************************
*                                        DEF_BIT_IS_SET_ANY()
*
* Description : Determine if any specified bit(s) in a value are set.
*
* Argument(s) : val         Value to check for specified bit(s) set.
*
*               mask        Mask of bits to check if set (see Note #2).
*
* Return(s)   : DEF_YES, if ANY specified bit(s) are     set in value.
*
*               DEF_NO,  if ALL specified bit(s) are NOT set in value.
*
* Note(s)     : (1) 'val' & 'mask' SHOULD be unsigned integers.
*
*               (2) NULL 'mask' allowed; returns 'DEF_NO' since NO mask bits specified.
*********************************************************************************************************
*/

#define  DEF_BIT_IS_SET_ANY(val, mask)               ((((val) & (mask)) ==  0u)     ? (DEF_NO ) : (DEF_YES))


/*
*********************************************************************************************************
*                                        DEF_BIT_IS_CLR_ANY()
*
* Description : Determine if any specified bit(s) in a value are clear.
*
* Argument(s) : val         Value to check for specified bit(s) clear.
*
*               mask        Mask of bits to check if clear (see Note #2).
*
* Return(s)   : DEF_YES, if ANY specified bit(s) are     clear in value.
*
*               DEF_NO,  if ALL specified bit(s) are NOT clear in value.
*
* Note(s)     : (1) 'val' & 'mask' SHOULD be unsigned integers.
*
*               (2) NULL 'mask' allowed; returns 'DEF_NO' since NO mask bits specified.
*********************************************************************************************************
*/

#define  DEF_BIT_IS_CLR_ANY(val, mask)               ((((val) & (mask)) == (mask))  ? (DEF_NO ) : (DEF_YES))


/*
*********************************************************************************************************
*                                         DEF_GET_U_MAX_VAL()
*
* Description : Get the maximum unsigned value that can be represented in an unsigned integer variable
*                   of the same data type size as an object.
*
* Argument(s) : obj         Object or data type to return maximum unsigned value (see Note #1).
*
* Return(s)   : Maximum unsigned integer value that can be represented by the object, if NO error(s).
*
*               0,                                                                    otherwise.
*
* Note(s)     : (1) 'obj' SHOULD be an integer object or data type but COULD also be a character or
*                   pointer object or data type.
*********************************************************************************************************
*/

#if     (CPU_CFG_DATA_SIZE_MAX == CPU_WORD_SIZE_08)

#define  DEF_GET_U_MAX_VAL(obj)                 ((sizeof(obj) == CPU_WORD_SIZE_08) ? DEF_INT_08U_MAX_VAL : 0)


#elif   (CPU_CFG_DATA_SIZE_MAX == CPU_WORD_SIZE_16)

#define  DEF_GET_U_MAX_VAL(obj)                 ((sizeof(obj) == CPU_WORD_SIZE_08) ? DEF_INT_08U_MAX_VAL :   \
                                                ((sizeof(obj) == CPU_WORD_SIZE_16) ? DEF_INT_16U_MAX_VAL : 0))


#elif   (CPU_CFG_DATA_SIZE_MAX == CPU_WORD_SIZE_32)

#define  DEF_GET_U_MAX_VAL(obj)                 ((sizeof(obj) == CPU_WORD_SIZE_08) ? DEF_INT_08U_MAX_VAL :    \
                                                ((sizeof(obj) == CPU_WORD_SIZE_16) ? DEF_INT_16U_MAX_VAL :    \
                                                ((sizeof(obj) == CPU_WORD_SIZE_32) ? DEF_INT_32U_MAX_VAL : 0)))


#elif   (CPU_CFG_DATA_SIZE_MAX == CPU_WORD_SIZE_64)

#define  DEF_GET_U_MAX_VAL(obj)                 ((sizeof(obj) == CPU_WORD_SIZE_08) ? DEF_INT_08U_MAX_VAL :     \
                                                ((sizeof(obj) == CPU_WORD_SIZE_16) ? DEF_INT_16U_MAX_VAL :     \
                                                ((sizeof(obj) == CPU_WORD_SIZE_32) ? DEF_INT_32U_MAX_VAL :     \
                                                ((sizeof(obj) == CPU_WORD_SIZE_64) ? DEF_INT_64U_MAX_VAL : 0))))

#else

#error  CPU_CFG_DATA_SIZE_MAX illegally #defined in '[arch]_cpu_port.h'. \
		[See '[arch]_cpu_port.h CONFIGURATION ERRORS']

#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif                                                          /* End of lib utils module include.                     */
