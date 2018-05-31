/**************************************************************************//**
* @file app_rtcc.h
* @brief Helper functions for timekeeping using the RTCC
* @version 5.3.5
******************************************************************************
* # License
* <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
*******************************************************************************
*
* This file is licensed under the Silabs License Agreement. See the file
* "Silabs_License_Agreement.txt" for details. Before using this software for
* any purpose, you must agree to the terms of that agreement.
*
******************************************************************************/

#ifndef APP_RTCC_H
#define APP_RTCC_H

// Function prototypes
void setupRTCC(uint32_t resetVal);

uint32_t millis(void);

void delay_ms(uint32_t ms);

#endif /* APP_RTCC_H */
