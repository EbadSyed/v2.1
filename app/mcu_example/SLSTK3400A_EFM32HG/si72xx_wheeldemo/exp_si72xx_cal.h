/***************************************************************************//**
 * @file exp_si72xx_cal.h
 * @brief Code for Si72xx-EXP calibration
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

#ifndef EXP_SI72XX_CALIBRATION_H_
#define EXP_SI72XX_CALIBRATION_H_

void CAL_runAngleCalibration(void);
int16_t CAL_getValue (int device, int index);

#endif /* EXP_SI72XX_CALIBRATION_H_ */
