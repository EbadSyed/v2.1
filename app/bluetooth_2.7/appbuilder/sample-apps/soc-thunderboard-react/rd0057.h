/***********************************************************************************************//**
 * \file   rd0057.h
 * \brief  RD-0057 ThunderBoard-React Board Handling
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef RD0057_H
#define RD0057_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "i2cspm.h"

/***************************************************************************************************
 * Buttons and LED definitions
 **************************************************************************************************/

#define NO_OF_BUTTONS  2
#define NO_OF_LEDS     2
#define LED_BLUE       0
#define LED_GREEN      1

/***************************************************************************************************
 * Sensor GPIO definitions
 **************************************************************************************************/

// LED pin definitions
#define LED0_PORT                 gpioPortA
#define LED0_PIN                  2
#define LED1_PORT                 gpioPortF
#define LED1_PIN                  6

// Button pin definitions
#define BUTTON0_PORT              gpioPortA
#define BUTTON0_PIN               3
#define BUTTON1_PORT              gpioPortF
#define BUTTON1_PIN               7

// MPU-6500 accelerometer/gyro interrupt pin definitions
#define ACCGYRO_INT_PORT          gpioPortF
#define ACCGYRO_INT_PIN           4

// Si1133 Ambient Light/UV sensor interrupt pin definitions
#define ALUV_INT_PORT             gpioPortF
#define ALUV_INT_PIN              5

// Hall effect sensor pin definitions
#define HALLSENSOR_OUT_PORT       gpioPortF
#define HALLSENSOR_OUT_PIN        3
#define HALLSENSOR_ENABLE_PORT    gpioPortB
#define HALLSENSOR_ENABLE_PIN     11

/***************************************************************************************************
 * Clock and Energy Management
 **************************************************************************************************/

/***************************************************************************************************
 * Global Variables
 **************************************************************************************************/

extern I2CSPM_Init_TypeDef i2cInit;

/***************************************************************************************************
 * Function Declarations
 **************************************************************************************************/

/**********************************************************************************************//**
 * @brief
 *   Function to initialize the board and put all sensors in their lowest power
 *  mode. This function is run by bookkeeping at startup.
 *  return None
 *************************************************************************************************/
void boardInit(void);

/**********************************************************************************************//**
 * @brief
 *   Function called to register a callback that will be called whenever a
 *  button is pushed or released. Note that this callback will be called from
 *  interrupt context, hence it should return quickly.
 *  param[in]  buttonCallback Pointer to callback function
 *  return None
 *************************************************************************************************/
void boardRegisterButtonCallback(void (*buttonCallback)(uint8_t, bool));

/**********************************************************************************************//**
 * @brief
 *   Function is called by the interrupt handler when a button state has
 *  changed. This function will in turn again call any registered callbacks.
 *  param[in]  buttonNo Number of button that has changed.
 *  return None
 *************************************************************************************************/
void boardButtonInterrupt(uint8_t buttonNo);

/**********************************************************************************************//**
 * @brief
 *   Function to check the state of a button
 *  param[in]  buttonNo Number of button to check.
 *  return State of button
 *************************************************************************************************/
uint8_t boardButtonGet(uint8_t buttonNo);

/**********************************************************************************************//**
 * @brief
 *   Function to turn on LED
 *  param[in]  ledNo Number of LED to turn on.
 *  return None
 *************************************************************************************************/
void boardLedOn(uint8_t ledNo);

/**********************************************************************************************//**
 * @brief
 *   Function to turn off LED
 *  param[in]  ledNo Number of LED to turn off.
 *  return None
 *************************************************************************************************/
void boardLedOff(uint8_t ledNo);

/**********************************************************************************************//**
 * @brief
 *   Function to toggle LED
 *  param[in]  ledNo Number of LED to toggle.
 *  return None
 *************************************************************************************************/
void boardLedToggle(uint8_t ledNo);

/**********************************************************************************************//**
 * @brief
 *   Function to check the state of a LED
 *  param[in]  buttonNo Number of LED to check.
 *  return State of LED
 *************************************************************************************************/
uint8_t boardLedGet(uint8_t ledNo);

/**********************************************************************************************//**
 * @brief
 *   Function to turn on and off the Hall sensor
 *  param[in]  enable Set to enable hall sensor, clear to disable.
 *  return None
 *************************************************************************************************/
void boardHallEnable(bool enable);

#ifdef __cplusplus
};
#endif

#endif /* RD0057_H */
