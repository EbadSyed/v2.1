/***************************************************************************//**
 * Copyright 2017 Silicon Laboratories, Inc.
 *
 ******************************************************************************/

#include "hal-config.h"
#include "em_device.h"
#include "em_assert.h"
#include "em_gpio.h"
#include "em_bus.h"
#include "em_cmu.h"

#if HAL_LNA_ENABLE
#if (BSP_LNA_TXRX_CHANNEL + 1) != BSP_LNA_SLEEP_CHANNEL
  #error "LNA_SLEEP channel must immediately follow LNA_TXRX channel"
#endif

#if BSP_LNA_SLEEP_CHANNEL >= PRS_CHAN_COUNT
  #error "LNA_SLEEP channel number higher than number of PRS channels"
#endif
#endif // HAL_LNA_ENABLE

/***************************************************************************//**
 * Initialize LNA for xGM1xP modules.
 ******************************************************************************/
void module_initLna(void)
{
#if HAL_LNA_ENABLE
  volatile uint32_t * routeRegister;

  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_PRS, true);

  // Initialize GPIO
  GPIO_PinModeSet(BSP_LNA_TXRX_PORT,
                  BSP_LNA_TXRX_PIN,
                  gpioModePushPullAlternate,
                  0);
  GPIO_PinModeSet(BSP_LNA_SLEEP_PORT,
                  BSP_LNA_SLEEP_PIN,
                  gpioModePushPullAlternate,
                  0);

  // Setup TX/RX and SLEEP PRS signal sources
  PRS->CH[BSP_LNA_TXRX_CHANNEL].CTRL = PRS_RAC_LNAEN;
  PRS->CH[BSP_LNA_SLEEP_CHANNEL].CTRL = PRS_RAC_PAEN | PRS_CH_CTRL_ORPREV;

  // Configure TX/RX PRS output to selected channel and location
  if (BSP_LNA_TXRX_CHANNEL < 4) {
    routeRegister = &PRS->ROUTELOC0;
  } else if (BSP_LNA_TXRX_CHANNEL < 8) {
    routeRegister = &PRS->ROUTELOC1;
  } else if (BSP_LNA_TXRX_CHANNEL < 12) {
    routeRegister = &PRS->ROUTELOC2;
  } else {
    EFM_ASSERT(0);
    return; // error
  }

  BUS_RegMaskedWrite(routeRegister,
                     0xFF << ((BSP_LNA_TXRX_CHANNEL % 4) * 8),
                     BSP_LNA_TXRX_LOC << ((BSP_LNA_TXRX_CHANNEL % 4) * 8));

  // Configure SLEEP PRS output to selected channel and location
  if (BSP_LNA_SLEEP_CHANNEL < 4) {
    routeRegister = &PRS->ROUTELOC0;
  } else if (BSP_LNA_SLEEP_CHANNEL < 8) {
    routeRegister = &PRS->ROUTELOC1;
  } else if (BSP_LNA_SLEEP_CHANNEL < 12) {
    routeRegister = &PRS->ROUTELOC2;
  } else {
    EFM_ASSERT(0);
    return; // error
  }

  BUS_RegMaskedWrite(routeRegister,
                     0xFF << ((BSP_LNA_SLEEP_CHANNEL % 4) * 8),
                     BSP_LNA_SLEEP_LOC << ((BSP_LNA_SLEEP_CHANNEL % 4) * 8));

  // Enable PRS output
  BUS_RegMaskedSet(&PRS->ROUTEPEN,
                   (1 << BSP_LNA_TXRX_CHANNEL) | (1 << BSP_LNA_SLEEP_CHANNEL));
#endif
}
