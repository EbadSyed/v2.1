/**************************************************************************//**
* @file app_csen.h
* @brief Helper functions for capacitive touch using CSEN
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

#ifndef APP_CSEN_H
#define APP_CSEN_H

#define PAD_THRS 1500
#define PAD_LEVEL_0 { 0, 0, 0, 0, 0, 0 }
#define PAD_LEVEL_THRS { PAD_THRS, PAD_THRS, PAD_THRS, PAD_THRS, PAD_THRS, PAD_THRS }
#define APP_CSEN_NOISE_MARGIN 500

typedef struct {
  int32_t sliderPos;
  int32_t sliderPrevPos;
  int32_t sliderStartPos;
  int32_t sliderTravel;
  uint32_t eventStart;
  uint32_t eventDuration;
  uint32_t touchForce;
  bool eventActive;
} CSEN_Event_t;

#define CSEN_EVENT_DEFAULT \
  {                        \
    -1,                    \
    -1,                    \
    -1,                    \
    0,                     \
    0,                     \
    0,                     \
    0,                     \
    false,                 \
  }

// Function prototypes
void setupCSEN(void);

int32_t csenCalcPos(void);

void csenCheckScannedData(void);

CSEN_Event_t csenGetEvent(void);

#endif /* APP_CSEN_H */
