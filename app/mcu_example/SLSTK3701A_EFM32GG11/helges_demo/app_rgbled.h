/**************************************************************************//**
* @file app_rgbled.h
* @brief Helper functions for PWM control of RGB LEDs
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

#ifndef APP_RGBLED_H
#define APP_RGBLED_H

typedef struct {
  uint32_t red;
  uint32_t green;
  uint32_t blue;
} ColorMix;

typedef struct {
  int16_t led0Color;
  int16_t led1Color;
  int8_t led0Power;
  int8_t led1Power;
  uint8_t curSetting;
} RGBLED_Settings_t;

#define RGBLED_SETTINGS_DEFAULT \
  {                             \
    330,                        \
    90,                         \
    6,                          \
    6,                          \
    0,                          \
  }

void setupRGBLed(void);
void setLedSettings(RGBLED_Settings_t * ledSettings);
void getColorMix(int16_t wheelpos, ColorMix * inColor, uint32_t gain);

#endif /* APP_RGBLED_H */
