/***************************************************************************//**
 * @file displayconfigapp.h
 * @brief Display application specific configuration file.
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

#ifndef __DISPLAYCONFIGAPP_H__
#define __DISPLAYCONFIGAPP_H__

// Include TEXTDISPLAY support because this example prints text to the display.
#define INCLUDE_TEXTDISPLAY_SUPPORT

// Enable/disable video terminal escape sequences.
#define INCLUDE_VIDEO_TERMINAL_ESCAPE_SEQUENCE_SUPPORT

// Select one of the fonts listed below:
//   TEXTDISPLAY_FONT_6x8
//   TEXTDISPLAY_FONT_8x8
#define TEXTDISPLAY_FONT_8x8

// Enable or disable scroll mode on the text display.
#define RETARGETTEXTDISPLAY_SCROLL_MODE  (false)

// Enable or disable adding Carriage Return (CR) to Line Feed (LF) characters
// on the text display.
#define RETARGETTEXTDISPLAY_LINE_FEED_MODE  (true)

// Use LFRCO
#undef  PAL_RTCC_CLOCK_LFXO
#define  PAL_RTCC_CLOCK_LFRCO

// The DISPLAY driver Platform Abstraction Layer (PAL) uses the RTCC to time and
// toggle the EXTCOMIN pin of the Sharp memory LCD per default. However, the LC
// sense example wants to use the RTCC to keep track of time, i.e. generate
// interrupt every second. Therefore the LC sense example implements a function
// (rtcIntCallbackRegister)) that enables the PAL to register the callback
// function that needs to be called in order to toggle the EXTCOMIN pin.
//   I.e we need to undefine the INCLUDE_PAL_GPIO_PIN_AUTO_TOGGLE (which is
//   defined by default and uses the RTCC) and we need to define the
//   PAL_TIMER_REPEAT_FUNCTION. */
#undef INCLUDE_PAL_GPIO_PIN_AUTO_TOGGLE
#if 1
#define PAL_TIMER_REPEAT_FUNCTION  (rtcIntCallbackRegister)
extern int rtcIntCallbackRegister (void(*pFunction)(void*),
                                   void* argument,
                                   unsigned int frequency);
#endif

#endif /* __DISPLAYCONFIGAPP_H__ */
