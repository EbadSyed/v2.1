#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

#include "board_features.h"
#include "hal-config-board.h"

#if defined(FEATURE_IOEXPANDER)
#include "hal-config-ioexp.h"
#endif

#if defined(FEATURE_LNA)
#include "hal-config-lna.h"
#endif

#ifdef BSP_CLK_LFXO_CTUNE
#undef BSP_CLK_LFXO_CTUNE
#endif
#define BSP_CLK_LFXO_CTUNE                            (32)

#define HAL_EXTFLASH_FREQUENCY                        (1000000)

#define HAL_PA_ENABLE                                 (1)
#define HAL_PA_RAMP                                   (10)
#define HAL_PA_2P4_LOWPOWER                           (0)
#define HAL_PA_POWER                                  (252)
#define HAL_PA_CURVE_HEADER                            "pa_curves_efr32.h"
#ifdef FEATURE_PA_HIGH_POWER
#define HAL_PA_VOLTAGE                                (3300)
#else // FEATURE_PA_HIGH_POWER
#define HAL_PA_VOLTAGE                                (1800)
#endif // FEATURE_PA_HIGH_POWER

#define HAL_PTI_ENABLE                                (1)
#define HAL_PTI_MODE                                  (HAL_PTI_MODE_UART)
#define HAL_PTI_BAUD_RATE                             (1600000)

#define HAL_VCOM_ENABLE                   (0)
#define HAL_I2CSENSOR_ENABLE              (0)
#define HAL_SPIDISPLAY_ENABLE             (0)

#ifdef  FEATURE_EXP_HEADER_USART3

#define BSP_EXP_USART           USART3

#define BSP_EXP_USART_CTS_PIN   BSP_USART3_CTS_PIN
#define BSP_EXP_USART_CTS_PORT  BSP_USART3_CTS_PORT
#define BSP_EXP_USART_CTS_LOC   BSP_USART3_CTS_LOC

#define BSP_EXP_USART_RTS_PIN   BSP_USART3_RTS_PIN
#define BSP_EXP_USART_RTS_PORT  BSP_USART3_RTS_PORT
#define BSP_EXP_USART_RTS_LOC   BSP_USART3_RTS_LOC

#define BSP_EXP_USART_RX_PIN    BSP_USART3_RX_PIN
#define BSP_EXP_USART_RX_PORT   BSP_USART3_RX_PORT
#define BSP_EXP_USART_RX_LOC    BSP_USART3_RX_LOC

#define BSP_EXP_USART_TX_PIN    BSP_USART3_TX_PIN
#define BSP_EXP_USART_TX_PORT   BSP_USART3_TX_PORT
#define BSP_EXP_USART_TX_LOC    BSP_USART3_TX_LOC

#else

#define BSP_EXP_USART           USART0

#define BSP_EXP_USART_CTS_PIN   BSP_USART0_CTS_PIN
#define BSP_EXP_USART_CTS_PORT  BSP_USART0_CTS_PORT
#define BSP_EXP_USART_CTS_LOC   BSP_USART0_CTS_LOC

#define BSP_EXP_USART_RTS_PIN   BSP_USART0_RTS_PIN
#define BSP_EXP_USART_RTS_PORT  BSP_USART0_RTS_PORT
#define BSP_EXP_USART_RTS_LOC   BSP_USART0_RTS_LOC

#define BSP_EXP_USART_RX_PIN    BSP_USART0_RX_PIN
#define BSP_EXP_USART_RX_PORT   BSP_USART0_RX_PORT
#define BSP_EXP_USART_RX_LOC    BSP_USART0_RX_LOC

#define BSP_EXP_USART_TX_PIN    BSP_USART0_TX_PIN
#define BSP_EXP_USART_TX_PORT   BSP_USART0_TX_PORT
#define BSP_EXP_USART_TX_LOC    BSP_USART0_TX_LOC

#endif // FEATURE_EXP_HEADER_USART3

#endif
