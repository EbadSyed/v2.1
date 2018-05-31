/**************************************************************************//**
 * @file app_lcd_logo.h
 * @brief Wireless Whiteboard's Silicon Labs Logo
 * WGM110 and GLIB demo for the SLSTK3401A running on uC/OS-III.
 * This header file contains the Silicon Labs logo pixel values.
 * @version 5.3.5
 ******************************************************************************
 * # License * <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silicon Labs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#ifndef APP_LCD_LOGO_H
#define APP_LCD_LOGO_H

/// Silicon Labs Logo 128x128 pixels.
const uint8_t APP_LCD_LOGO_PixValues[] =
{
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf3,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xf1,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,0xfe,0xff,0x7f,0x00,0x00,
  0xfe,0xff,0xff,0xc7,0xff,0xff,0xff,0xff,0xff,0xff,0xe1,0xff,0xff,0x01,0x00,
  0x00,0xc0,0xff,0xff,0x0f,0xff,0xff,0xff,0xff,0xff,0x3f,0xfc,0xff,0x3f,0x00,
  0x00,0x00,0x00,0xff,0xff,0x0f,0xfc,0xff,0xff,0xff,0xff,0x87,0xff,0xff,0x07,
  0x00,0x00,0x00,0x00,0xfe,0xff,0x1f,0xf8,0xff,0xff,0xff,0xff,0xe0,0xff,0xff,
  0x01,0x00,0x00,0x00,0x00,0xfc,0xff,0x1f,0xf0,0xff,0xff,0xff,0x1f,0xf8,0xff,
  0x7f,0x00,0x00,0x00,0x00,0x00,0xfc,0xff,0x1f,0xe0,0xff,0xff,0xff,0x03,0xff,
  0xff,0x1f,0x00,0x00,0x00,0x00,0x00,0xfc,0xff,0x0f,0xc0,0xff,0xff,0xff,0x80,
  0xff,0xff,0x0f,0x00,0x00,0x00,0x00,0x00,0xfc,0xff,0x07,0x80,0xff,0xff,0x3f,
  0xe0,0xff,0xff,0x03,0x00,0x00,0x00,0x00,0x00,0xfe,0xff,0x03,0x80,0xff,0xff,
  0x07,0xf8,0xff,0xff,0x01,0x00,0x00,0x00,0x00,0x80,0xff,0xff,0x00,0x80,0xff,
  0xff,0x01,0xfc,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0x1f,0x00,0x80,
  0xff,0x7f,0x00,0xfe,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x80,0xff,0x3f,0x00,0xff,0xff,0x7f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x80,0xff,0x0f,0x80,0xff,0xff,0x7f,0x00,0x00,0x80,0xff,0x3f,0x00,0x00,
  0x00,0x00,0x80,0xff,0x03,0x80,0xff,0xff,0x7f,0x00,0xe0,0xff,0xff,0xff,0x7f,
  0x00,0x00,0x00,0xc0,0xff,0x01,0xc0,0xff,0xff,0x7f,0xc0,0xff,0xff,0xff,0xff,
  0xff,0x0f,0x00,0x00,0xc0,0xff,0x00,0xc0,0xff,0xff,0xff,0xe0,0xff,0xff,0xff,
  0xff,0xff,0x7f,0x00,0x00,0xe0,0x3f,0x00,0x80,0xff,0xff,0xff,0x01,0xfe,0xff,
  0xff,0xff,0xff,0xff,0x01,0x00,0xe0,0x1f,0x00,0x80,0xff,0xff,0xff,0x07,0x00,
  0x80,0xff,0xff,0xff,0xff,0x03,0x00,0xf0,0x0f,0x00,0x00,0xff,0xff,0xff,0x3f,
  0x00,0x00,0xc0,0xff,0xff,0xff,0x07,0x00,0xf8,0x07,0x00,0x00,0xfe,0xff,0xff,
  0xff,0x07,0x00,0x00,0xff,0xff,0xff,0x0f,0x00,0xfc,0x07,0x00,0x00,0xf8,0xff,
  0xff,0xff,0xff,0xff,0x07,0xfc,0xff,0xff,0x0f,0x00,0xfe,0x03,0x00,0x00,0xe0,
  0xff,0xff,0xff,0xff,0xff,0x3f,0xf8,0xff,0xff,0x0f,0x80,0xff,0x01,0x00,0x00,
  0x00,0xfc,0xff,0xff,0xff,0xff,0x00,0xf0,0xff,0xff,0x0f,0xc0,0xff,0x01,0x00,
  0x00,0x00,0x80,0xff,0xff,0xff,0x00,0x00,0xf0,0xff,0xff,0x0f,0xf0,0xff,0x01,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0xff,0xff,0x07,0xf8,0xff,
  0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0xff,0xff,0x03,0xfe,
  0xff,0x01,0x00,0x00,0x1f,0x00,0x00,0x00,0x00,0x00,0x00,0xf0,0xff,0xff,0x81,
  0xff,0xff,0x01,0x00,0xc0,0xff,0x03,0x00,0x00,0x00,0x00,0x00,0xf8,0xff,0xff,
  0xe0,0xff,0xff,0x01,0x00,0xf0,0xff,0x07,0x00,0x00,0x00,0x00,0x00,0xfc,0xff,
  0x3f,0xf8,0xff,0xff,0x01,0x00,0xfc,0xff,0x01,0x00,0x00,0x00,0x00,0x00,0xff,
  0xff,0x1f,0xfe,0xff,0xff,0x03,0x00,0xfe,0xff,0x01,0x00,0x00,0x00,0x00,0x80,
  0xff,0xff,0x87,0xff,0xff,0xff,0x03,0x00,0xff,0xff,0x01,0x00,0x00,0x00,0x00,
  0xe0,0xff,0xff,0xf1,0xff,0xff,0xff,0x07,0x80,0xff,0xff,0x01,0x00,0x00,0x00,
  0x00,0xf8,0xff,0x7f,0xfe,0xff,0xff,0xff,0x0f,0x80,0xff,0xff,0x07,0x00,0x00,
  0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x1f,0xc0,0xff,0xff,0x0f,0x00,
  0x00,0x00,0xe0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f,0x80,0xff,0xff,0x7f,
  0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x01,0xff,0xff,
  0xff,0x0f,0x00,0xfc,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x07,0xfe,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x3f,
  0xf8,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xc3,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0x03,0x3f,0x9f,0x7f,0x7e,0xf0,0x07,0x3e,0xcf,0xff,
  0x9f,0xff,0xe3,0x0f,0xf8,0xc0,0x01,0x3e,0x9f,0x7f,0x3e,0xc0,0x03,0x3c,0xcf,
  0xff,0x9f,0xff,0xe3,0x0f,0x70,0x80,0x79,0x3e,0x9f,0x7f,0x3e,0xcf,0xf3,0x3c,
  0xce,0xff,0x9f,0xff,0xe3,0xcf,0x73,0x9e,0x79,0x3e,0x9f,0x7f,0x3e,0xcf,0xf3,
  0x3c,0xce,0xff,0x9f,0xff,0xc3,0xcf,0x73,0x9e,0xf9,0x3f,0x9f,0x7f,0x3e,0xff,
  0xf3,0x3c,0xcc,0xff,0x9f,0xff,0xc9,0xcf,0x73,0xfe,0xe1,0x3f,0x9f,0x7f,0x3e,
  0xff,0xf3,0x3c,0xcc,0xff,0x9f,0xff,0xc9,0xcf,0x73,0xf8,0x83,0x3f,0x9f,0x7f,
  0x3e,0xff,0xf3,0x3c,0xc8,0xff,0x9f,0xff,0x89,0x0f,0xf0,0xe0,0x07,0x3e,0x9f,
  0x7f,0x3e,0xff,0xf3,0x3c,0xc9,0xff,0x9f,0xff,0x88,0x0f,0xf8,0x81,0x3f,0x3e,
  0x9f,0x7f,0x3e,0xff,0xf3,0x3c,0xc1,0xff,0x9f,0xff,0x9c,0xcf,0xf3,0x8f,0x7f,
  0x3e,0x9f,0x7f,0x3e,0xff,0xf3,0x3c,0xc3,0xff,0x9f,0xff,0x80,0xcf,0xf3,0x9f,
  0x79,0x3e,0x9f,0x7f,0x3e,0xcf,0xf3,0x3c,0xc3,0xff,0x9f,0xff,0x00,0xcf,0x73,
  0x9e,0x79,0x3e,0x9f,0x7f,0x3e,0xcf,0xf3,0x3c,0xc7,0xff,0x9f,0x7f,0x1c,0xcf,
  0x73,0x9e,0x01,0x3e,0x1f,0x70,0x3e,0xc0,0x03,0x3c,0xc7,0xff,0x1f,0x70,0x3e,
  0x0f,0x70,0x80,0x03,0x3f,0x1f,0x70,0x7e,0xe0,0x07,0x3e,0xcf,0xff,0x1f,0x30,
  0x3e,0x0e,0xf8,0xc0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

#endif  // APP_LCD_LOGO_H
