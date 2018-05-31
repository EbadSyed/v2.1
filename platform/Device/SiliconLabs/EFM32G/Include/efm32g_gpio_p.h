/**************************************************************************//**
 * @file efm32g_gpio_p.h
 * @brief EFM32G_GPIO_P register and bit field definitions
 * @version 5.3.5
 ******************************************************************************
 * # License
 * <b>Copyright 2017 Silicon Laboratories, Inc. www.silabs.com</b>
 ******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.@n
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.@n
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Laboratories, Inc.
 * has no obligation to support this Software. Silicon Laboratories, Inc. is
 * providing the Software "AS IS", with no express or implied warranties of any
 * kind, including, but not limited to, any implied warranties of
 * merchantability or fitness for any particular purpose or warranties against
 * infringement of any proprietary rights of a third party.
 *
 * Silicon Laboratories, Inc. will not be liable for any consequential,
 * incidental, or special damages, or any other relief, or for any claim by
 * any third party, arising from your use of this Software.
 *
 *****************************************************************************/

#if defined(__ICCARM__)
#pragma system_include       /* Treat file as system include file. */
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma clang system_header  /* Treat file as system include file. */
#endif

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @brief GPIO_P EFM32G GPIO P
 *****************************************************************************/
typedef struct {
  __IOM uint32_t CTRL;     /**< Port Control Register  */
  __IOM uint32_t MODEL;    /**< Port Pin Mode Low Register  */
  __IOM uint32_t MODEH;    /**< Port Pin Mode High Register  */
  __IOM uint32_t DOUT;     /**< Port Data Out Register  */
  __OM uint32_t  DOUTSET;  /**< Port Data Out Set Register  */
  __OM uint32_t  DOUTCLR;  /**< Port Data Out Clear Register  */
  __OM uint32_t  DOUTTGL;  /**< Port Data Out Toggle Register  */
  __IM uint32_t  DIN;      /**< Port Data In Register  */
  __IOM uint32_t PINLOCKN; /**< Port Unlocked Pins Register  */
} GPIO_P_TypeDef;

/** @} End of group Parts */