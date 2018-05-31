/***************************************************************************//**
 * @file exp_si72xx.h
 * @brief Code for Si72xx-EXP demos
 * @version 5.3.5
 *******************************************************************************
 * # License
 * <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#ifndef EXP_SI72XX_H_
#define EXP_SI72XX_H_

uint32_t EXP_SI72XX_placeSensorsInSleepMode(void);
uint32_t EXP_SI72XX_placeSensorsInLatchMode(void);

void EXP_SI72XX_runAngleDemo(void);
void EXP_SI72XX_runRevolutionCounterDemo(void);
void EXP_SI72XX_disableRevolutionCounterDemo(void);
void EXP_SI72XX_runMenuScreen (void);

#endif /* EXP_SI72XX_H_ */
