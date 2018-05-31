/** @file app/ip-ncp/spi-link-adapter.c
 * @brief Adapts IP-NCP UART API to the spi-protocol2 EZSP-esque API
 *
 * <!-- Copyright 2013 by Silicon Laboratories.          All rights reserved.-->
 */

#include PLATFORM_HEADER
#include "stack/include/ember-types.h"
#include "stack/core/ember-stack.h"
#include "hal/hal.h"
#include "stack/framework/ip-packet-header.h"
#include "stack/ip/ip-header.h"
#include "stack/ip/6lowpan-header.h"
#include "app/util/serial/command-interpreter2.h"
#include "uart-link-protocol.h"
#include "hal/micro/uart-link.h"
#include "hal/micro/cortexm3/spi-protocol2.h" // TODO make this be micro-agnostic

void halInternalPowerUpUart(void)
{
  // TODO: Power up the SPI.
  //  assert(false);
}

void halInternalPowerDownUart(void)
{
  // TODO: Power down the SPI.
  //  assert(false);
}
