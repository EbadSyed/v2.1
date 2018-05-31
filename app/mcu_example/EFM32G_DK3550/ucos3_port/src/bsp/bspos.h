/***************************************************************************//**
 * @file
 * @brief uC/OS-III example - Board Support Package (BSP) header
 * @version 5.3.5
 *******************************************************************************
 * # License
 * <b>Copyright 2015 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#ifndef  __BSP_PRESENT_H
#define  __BSP_PRESENT_H

/*******************************************************************************
 *****************************   INCLUDE FILES   *******************************
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 **************************   FUNCTION PROTOTYPES   ****************************
 ******************************************************************************/
void BSPOS_Init(void);
CPU_INT32U OS_CPU_SysTickClkFreq (void);

#ifdef __cplusplus
}
#endif

#endif /* end of __BSP_PRESENT_H */