/*
 * File: ecc-mbedtls-util.c
 * Description: Utility functions for ECC calls into mbed TLS.
 *
 * Copyright 2017 by Silicon Laboratories. All rights reserved.             *80*
 */

#include "stack/core/ember-stack.h"
#include "phy/phy.h"         // emRadioGetRandomNumbers()
#include "hal/hal.h"
#include "hal/micro/micro.h" // for watchdog routines.

int emGetRandomBytes(void *token, unsigned char *result, size_t resultLength)
{
  uint16_t extra;

  assert(emRadioGetRandomNumbers((uint16_t *) result, resultLength >> 1));
  if (resultLength & 0x01) {
    assert(emRadioGetRandomNumbers(&extra, 1));
    result[resultLength - 1] = LOW_BYTE(extra);
  }
  return 0;
}

void emSetWatchdog(bool on)
{
#if (defined (PHY_EM3XX))
  if (on) {
    if (!halInternalWatchDogEnabled()) {
      halInternalEnableWatchDog();
    }
  } else {
    if (halInternalWatchDogEnabled()) {
      halInternalDisableWatchDog(MICRO_DISABLE_WATCH_DOG_KEY);
    }
  }
#endif
}
