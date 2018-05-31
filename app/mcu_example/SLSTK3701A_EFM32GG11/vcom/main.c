/***************************************************************************//**
 * @file
 * @brief VCOM example for SLSTK3701A starter kit
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

#include <stdio.h>
#include "em_device.h"
#include "em_chip.h"
#include "retargetserial.h"

/*
 * Note! You can set compile time define -DRETARGET_USART4 to build this
 * example to use USART4 instead of default LEUART0. You can also define
 * -DRETARGET_VCOM to enable the virtual COM port using the USB cable.
 * See retargetserialconfig.h for details.
 */

/** Input buffer size */
#define ECHOBUFSIZE    80
/** Input buffer */
static char echoBuffer[ECHOBUFSIZE];

/***************************************************************************//**
 * @brief  Main function
 ******************************************************************************/
int main(void)
{
  int c;
  int index;

  /* Chip errata */
  CHIP_Init();

  /* Initialize LEUART/USART and map LF to CRLF */
  RETARGET_SerialInit();
  RETARGET_SerialCrLf(1);

#if defined(RETARGET_USART4)
  printf("\nEFM32 Giant Gecko 11 USART4 example\n");
#else
  printf("\nEFM32 Giant Gecko 11 LEUART0 example\n");
#endif

#if defined(RETARGET_VCOM)
  printf("Virtual COM port enabled.\n");
#endif

  for (index = 0; index < ECHOBUFSIZE; index++) {
    echoBuffer[index] = (char) 'a' + index;
  }

  /* Retrieve characters, print local echo and full line back */
  index = 0;
  while (1) {
    /* Retrieve new character */
    c = getchar();
    if (c > 0) {
      /* Output character - most terminals use CRLF */
      if (c == '\r') {
        echoBuffer[index] = '\0';
        /* Output entire line */
        printf("\n%s\n", echoBuffer);
        index = 0;
      } else {
        /* Filter non-printable characters */
        if ((c < ' ') || (c > '~')) {
          continue;
        }

        if (index < ECHOBUFSIZE) {
          /* Enter into buffer */
          echoBuffer[index] = c;
          index++;
        } else {
          /* Ignore character, buffer is full */
        }

        /* Local echo */
        putchar(c);
      }
    }
  }
}