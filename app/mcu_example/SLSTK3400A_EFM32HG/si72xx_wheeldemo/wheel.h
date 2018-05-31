/***************************************************************************//**
 * @file wheel.h
 * @brief Hall Effect Wheel Demo for SLSTK3400A_EFM32HG
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

#ifndef WHEEL_H_
#define WHEEL_H_

#define SAMPLES_TO_AVG                   5

#define SI72XXEXP_PORT              gpioPortC
#define SI72XXEXP_U1_PIN            1
#define SI72XXEXP_U2_PIN            0

#define SI72XXPS_OUT_PORT           gpioPortD
#define SI72XXPS_OUT_PIN            4

typedef enum _demoNo{
  menuScreen,
  expAnglePosition,
  expRevolutionCounter,
  psI2cFieldData,
  psI2cTempData,
  psSwitchLatch,
  psAnalogOut,
  psPwmOut,
  psSentOut,
} DemoNo_t;

void gpioEnablePushButton1(void);
void gpioDisablePushButton1(void);

#endif /* WHEEL_H_ */
