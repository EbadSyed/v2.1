/**************************************************************************//**
 * @file graphics.c
 * Draws the graphics on the display
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

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include "display.h"
#include "dmd.h"
#include "em_device.h"
#include "glib.h"
#include "graphics.h"
#include "common_declarations.h"

#include <rtos/kernel/include/os.h>
#include <rtos/net/include/net_if.h>
#include <rtos/net/include/net_ipv4.h>
#include <rtos/net/include/net_ipv6.h>
#include <rtos/common/include/rtos_utils.h>

#define GLIB_FONT_WIDTH   (glibContext.font.fontWidth + glibContext.font.charSpacing)
#define GLIB_FONT_HEIGHT  (glibContext.font.fontHeight)
#define CENTER_X          (glibContext.pDisplayGeometry->xSize / 2)
#define CENTER_Y          (glibContext.pDisplayGeometry->ySize / 2)
#define MAX_X             (glibContext.pDisplayGeometry->xSize - 1)
#define MAX_Y             (glibContext.pDisplayGeometry->ySize - 1)
#define MIN_X             0
#define MIN_Y             0
#define MAX_STR_LEN       48

// -----------------------------------------------------------------------------
// Local variables

static GLIB_Context_t glibContext;

// -----------------------------------------------------------------------------
// Local function declarations
static void GRAPHICS_DrawString(const char * s, int y);
static bool GRAPHICS_DrawIPv4Status(void);
static void GRAPHICS_DrawTitle(void);
static void GRAPHICS_DrawMic(void);
//static void GRAPHICS_DrawAudioSignal(int16_t* leftSampleBuffer, 
//                                     int16_t* rightSampleBuffer)

// -----------------------------------------------------------------------------
// Global function definitions

/***************************************************************************//**
 * Initializes glib and DMD.
 ******************************************************************************/
void GRAPHICS_Init(void)
{
  EMSTATUS status;

  // Initialize the DMD module for the DISPLAY device driver.
  status = DMD_init(0);
  if (DMD_OK != status) {
    while (1) {
    }
  }

  status = GLIB_contextInit(&glibContext);
  if (GLIB_OK != status) {
    while (1) {
    }
  }
}

/***************************************************************************//**
 * Update the whole display with current status.
 ******************************************************************************/
void GRAPHICS_ShowStatus(void)

{
  GLIB_clear(&glibContext);

  GRAPHICS_DrawTitle();
  GRAPHICS_DrawIPv4Status();
  GRAPHICS_DrawMic();

  DMD_updateDisplay();
}

/***************************************************************************//**
 * Displays the amplitude (spectrum) of the sampled audio.
 *
 * @param leftSampleBuffer mic samples from left microphone
 * @param rightSampleBuffer mic samples from right microphone
 ******************************************************************************/
/*
void GRAPHICS_DrawAudioSignal(int16_t* leftSampleBuffer, int16_t* rightSampleBuffer)
{
  float max = 65536;
  int offset = MAX_Y / 2;
  int offsetR = MAX_Y / 4;
  int offsetL = offsetR + offset;

  GLIB_clear(&glibContext);
  GRAPHICS_DrawIPv4Status();

  glibContext.backgroundColor = White;
  glibContext.foregroundColor =  Black;

  for (int x = 0; x < MAX_X; x = x + 1) { //Displays only one mic in the centre of the display
    int16_t sample = leftSampleBuffer[x * 15];

    if (sample < -DC_CORRECTION) {
      sample = sample * 2;
      float num = (float)sample / max;
      float val = num * offset;
      GLIB_drawLineV(&glibContext, x, offset, offset + (uint16_t)val);
    } else {
      sample = sample * 2;
      float num = (float)sample / max;
      float val = num * offset;
      GLIB_drawLineV(&glibContext, x, offset - (uint16_t)val, offset);
    }
  }
  DMD_updateDisplay();
  return;

  for (int x = 0; x < MAX_X; x = x + 1) {
    int16_t sampleLeft = leftSampleBuffer[x * 15];
    int16_t sampleRight = rightSampleBuffer[x * 15];
    // Upper graph - right speaker
    if (sampleLeft < -DC_CORRECTION) { // Adjusting du to the audio samples' offset
      sampleLeft = sampleLeft * 2; // Adjusting sample values to fit display
      float num = (float)sampleLeft / max;
      float val = num * offsetR;
      GLIB_drawLineV(&glibContext, x, offsetL, (int)(offsetL + val));
    } else {
      sampleLeft = sampleLeft * 2;
      float num = (float)sampleLeft / max;
      float val = num * offsetR;
      GLIB_drawLineV(&glibContext, x, (int)(offsetL - val), offsetL);
    }
    // Lower graph - left speaker
    if (sampleRight < -DC_CORRECTION) { // Adjusting du to the audio samples' offset
      sampleRight = sampleRight * 2;
      float num = (float)sampleRight / max;
      float val = num * offsetR;
      GLIB_drawLineV(&glibContext, x, offsetR, (int)(offsetR + val));
    } else {
      sampleRight = sampleRight * 2;
      float num = (float)sampleRight / max;
      float val = num * offsetR;
      GLIB_drawLineV(&glibContext, x, (int)(offsetR - val), offsetR);
    }
  }
  DMD_updateDisplay();
}
*/

// -----------------------------------------------------------------------------
// Local function definitions

/***************************************************************************//**
 * Draw title
 ******************************************************************************/
static void GRAPHICS_DrawTitle(void)
{
  GLIB_setFont(&glibContext, (GLIB_Font_t *)&GLIB_FontNormal8x8);
  GRAPHICS_DrawString("Web Microphone", 0);
  GLIB_setFont(&glibContext, (GLIB_Font_t *)&GLIB_FontNarrow6x8);
  GRAPHICS_DrawString("Micrium OS Network", 10);
}

/***************************************************************************//**
 * Draw a string at a specific y coordinate
 ******************************************************************************/
static void GRAPHICS_DrawString(const char * s, int y)
{
  GLIB_drawString(&glibContext, s, strlen(s), 0, y, false);
}

/***************************************************************************//**
 * Draw Microphone
 ******************************************************************************/
static void GRAPHICS_DrawMic(void)
{
  GLIB_drawBitmap(&glibContext,
                  (MAX_X + 1 - MIC_BITMAP_WIDTH) / 2,
                   30,
                   MIC_BITMAP_WIDTH,
                   MIC_BITMAP_HEIGHT,
                   micBitmap);
}
/***************************************************************************//**
 * Draw IPv4 address
 ******************************************************************************/
static bool GRAPHICS_DrawIPv4Status(void)
{
  RTOS_ERR          err;
  NET_IF_NBR        ifNbr;
  NET_IPv4_ADDR     addrTable[4];
  NET_IP_ADDRS_QTY  addrTableSize = 4;
  CPU_BOOLEAN       ok;
  CPU_CHAR          addrString[NET_ASCII_LEN_MAX_ADDR_IPv4];

  ifNbr = NetIF_NbrGetFromName("eth0");
  ok = NetIPv4_GetAddrHost(ifNbr, addrTable, &addrTableSize, &err);

  if (!ok) {
    return DEF_OFF;
  }

  if (addrTableSize > 0) {
    NetASCII_IPv4_to_Str(addrTable[0], addrString, DEF_NO, &err);
    APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );
    GRAPHICS_DrawString(addrString, 20);
    if (addrTable[0] == 0) {
      return DEF_OFF;
    } else {
      return DEF_ON;
    }
  }
  return DEF_OFF;
}
