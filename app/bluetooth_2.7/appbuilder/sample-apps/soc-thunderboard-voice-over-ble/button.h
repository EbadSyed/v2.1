/***************************************************************************//**
 * @file button.h
 * @brief Button definitions
 *******************************************************************************
 * # License
 * <b>Copyright 2016 Silicon Laboratories, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silicon Labs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/
#ifndef BUTTON_H_
#define BUTTON_H_

/***************************************************************************//**
 * @addtogroup Button
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @defgroup Button_External_Messages Button External Messages
 * @{
 * @brief Button external message macro definitions
 ******************************************************************************/
#define EXTSIG_BUTTON_SW1_RELEASED (1 << 0)     /**< External signal - SW1 button released */
#define EXTSIG_BUTTON_SW2_RELEASED (1 << 1)     /**< External signal - SW2 button released */

/** @} {end defgroup Button_External_Messages} */

/***************************************************************************//**
 * @defgroup Button_Functions Button Functions
 * @{
 * @brief Button functions
 ******************************************************************************/
void BUTTON_Init();

/** @} {end defgroup Button_Functions}*/

/** @} {end addtogroup Button} */

#endif /* BUTTON_H_ */
