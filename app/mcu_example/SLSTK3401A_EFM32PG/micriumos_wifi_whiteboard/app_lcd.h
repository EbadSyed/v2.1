/**************************************************************************//**
 * @file app_lcd.h
 * @brief Wireless Whiteboard's LCD Application Header File
 * WGM110 and GLIB demo for the SLSTK3401A running on MicOS.
 * This module besides initializing the LCD, SPI and the GLIB, it also
 * implements a series of predefined screens to display on the LCD.
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

#ifndef APP_LCD_H
#define APP_LCD_H

// -----------------------------------------------------------------------------
// Externs

#ifdef APP_LCD_MODULE
#define APP_LCD_MODULE_EXT
#else
#define APP_LCD_MODULE_EXT extern
#endif

// -----------------------------------------------------------------------------
// Constants

// GLIB Font size
#define APP_LCD_FONT_WIDTH  (appLcdGlibContext.font.fontWidth + appLcdGlibContext.font.charSpacing)
#define APP_LCD_FONT_HEIGHT (appLcdGlibContext.font.fontHeight)

// GLIB Center of display
#define APP_LCD_CENTER_X (appLcdGlibContext.pDisplayGeometry->xSize / 2)
#define APP_LCD_CENTER_Y (appLcdGlibContext.pDisplayGeometry->ySize / 2)

// GLIB Min and Max coordinates
#define APP_LCD_MAX_X (appLcdGlibContext.pDisplayGeometry->xSize - 1)
#define APP_LCD_MAX_Y (appLcdGlibContext.pDisplayGeometry->ySize - 1)

#define APP_LCD_MIN_X 0
#define APP_LCD_MIN_Y 0

// GLIB Whiteboard's image dimensions in pixels
#define APP_LCD_IMG_WIDTH  128
#define APP_LCD_IMG_HEIGHT 128

// Whiteboard image sprite's total number of bytes (i.e. 2048).
#define APP_LCD_SPRITE_SIZE      ((APP_LCD_IMG_WIDTH * APP_LCD_IMG_HEIGHT) / 8)
// Whiteboard image sprite string's total number of chars.
#define APP_LCD_SPRITE_STR_SIZE   (APP_LCD_SPRITE_SIZE)

// -----------------------------------------------------------------------------
// Data Types

typedef enum
{
  APP_LCD_SCREEN_NONE,
  APP_LCD_SCREEN_CONN_AP,
  APP_LCD_SCREEN_CONN_SERVER,
  APP_LCD_SCREEN_NETWORK_INFO,
  APP_LCD_SCREEN_WHITEBOARD
} APP_LCD_Screen_t;

// -----------------------------------------------------------------------------
// Global Variables

/// Latest whiteboard's image pixel values.
APP_LCD_MODULE_EXT uint8_t APP_LCD_PixValues[APP_LCD_SPRITE_SIZE];

// -----------------------------------------------------------------------------
// Function Prototypes

void APP_LCD_Init(void);
void APP_LCD_ShowScreen(APP_LCD_Screen_t screen);

#endif  // APP_LCD_H
