/***************************************************************************//**
 * @file spi-host.c
 * @brief EZSP SPI Host user interface functions
 *
 * This includes command option parsing, trace and counter functions.
 *
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/

// For getopt(3), snprintf(3), etc.
#define _GNU_SOURCE

#include PLATFORM_HEADER
#include <string.h>
#ifndef WIN32
  #include <unistd.h>
  #include <time.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "stack/include/ember-types.h"
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/ezsp-host/ezsp-host-common.h"
#include "app/ezsp-host/spi/spi-host.h"
#include "app/ezsp-host/ezsp-host-priv.h"
#include "app/ezsp-host/ezsp-host-io.h"
#include "app/ezsp-host/ezsp-host-queues.h"
#include "app/ezsp-host/ezsp-host-ui.h"
#include "app/util/gateway/backchannel.h"
#include "app/ezsp-host/spi/spi-host-ui.h"

//------------------------------------------------------------------------------
// Preprocessor definitions

#define ERR_LEN               128   // max length error message

#define txControl (txBuffer[0])     // more descriptive aliases
#define rxControl (rxBuffer[0])

static const char options[] = "ht:";

bool ezspInternalProcessCommandOptions(int argc, char *argv[], char *errStr)
{
  int c;
  uint8_t trace;
  int optionCount = 0;

  while (true) {
    c = getopt(argc, argv, options);
    if (c == -1) {
      if (optind < argc ) {
        snprintf(errStr, ERR_LEN, "Invalid option %s.\n", argv[optind]);
      }
      break;
    }

    optionCount++;

    switch (c) {
      case 'h':
      case '?':
        snprintf(errStr, ERR_LEN, "\n");
        break;
      case 't':
        if (sscanf(optarg, "%hhu", &trace) != 1) {
          snprintf(errStr, ERR_LEN, "Invalid trace flag value %s.\n", optarg);
        } else {
          spiWriteConfig(traceFlags, trace);
        }
        break;
      default:
        assert(1);
        break;
    }   // end of switch (c)
  } //end while
  return true;
}
