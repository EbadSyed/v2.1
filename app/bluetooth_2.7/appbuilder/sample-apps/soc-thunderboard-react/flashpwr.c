#include "em_device.h"
#include "em_usart.h"
#include "em_cmu.h"
#include "em_gpio.h"

#include "flashpwr.h"

#define MX25_PORT_MOSI         gpioPortC
#define MX25_PIN_MOSI          6
#define MX25_PORT_MISO         gpioPortC
#define MX25_PIN_MISO          7
#define MX25_PORT_SCLK         gpioPortC
#define MX25_PIN_SCLK          8
#define MX25_PORT_CS           gpioPortC
#define MX25_PIN_CS            9

#define MX25_USART             USART1
#define MX25_USART_CLK         cmuClock_USART1

#define MX25_RXLOC USART_ROUTELOC0_RXLOC_LOC11
#define MX25_TXLOC USART_ROUTELOC0_TXLOC_LOC11
#define MX25_CLKLOC USART_ROUTELOC0_CLKLOC_LOC11

static void msDelay(uint32_t delay)
{
  volatile uint32_t cnt;

  cnt = CMU_ClockFreqGet(cmuClock_HF) * delay / 1000;
  cnt /= 7;   /* 7 cycles per loop count */

  while (cnt--) ;

  return;
}

void flashpwrInit(void)
{
  USART_InitSync_TypeDef init = USART_INITSYNC_DEFAULT;

  /* Enable clocks */
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(MX25_USART_CLK, true);

  /* Configure GPIO pins */
  GPIO_PinModeSet(MX25_PORT_MOSI, MX25_PIN_MOSI, gpioModePushPull, 1);
  GPIO_PinModeSet(MX25_PORT_MISO, MX25_PIN_MISO, gpioModeInput, 0);
  GPIO_PinModeSet(MX25_PORT_SCLK, MX25_PIN_SCLK, gpioModePushPull, 1);
  GPIO_PinModeSet(MX25_PORT_CS, MX25_PIN_CS, gpioModePushPull, 1);

  /* Configure USART */
  init.msbf = true;
  USART_InitSync(MX25_USART, &init);

  MX25_USART->ROUTELOC0 = (MX25_RXLOC | MX25_TXLOC | MX25_CLKLOC);
  MX25_USART->ROUTEPEN  = (USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_TXPEN | USART_ROUTEPEN_CLKPEN);

  /* Wait for flash to power up */
  msDelay(10);

  /* Set EM4 pin retention so chip select stays high if we enter EM4 */
  EMU->EM4CTRL |= EMU_EM4CTRL_EM4IORETMODE_EM4EXIT;

  return;
}

void flashpwrDeepPowerDown(void)
{
  /* Assert chip select ( low ) */
  GPIO_PinOutClear(MX25_PORT_CS, MX25_PIN_CS);

  /* Send DP command ( 0xB9 ) */
  USART_SpiTransfer(MX25_USART, 0xB9);

  /* Deassert chip select ( high ) */
  GPIO_PinOutSet(MX25_PORT_CS, MX25_PIN_CS);

  /* We must disable SPI communication */
  USART_Reset(MX25_USART);

  return;
}
