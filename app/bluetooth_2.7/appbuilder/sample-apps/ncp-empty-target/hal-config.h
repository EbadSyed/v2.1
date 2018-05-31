#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

#include "board_features.h"
#include "hal-config-board.h"
#include "hal-config-app-common.h"

#define HAL_UARTNCP_BAUD_RATE             (115200)
#define HAL_UARTNCP_FLOW_CONTROL          (HAL_USART_FLOW_CONTROL_NONE)

#ifndef HAL_VCOM_ENABLE
#define HAL_VCOM_ENABLE                   (1)
#endif
#define HAL_I2CSENSOR_ENABLE              (0)
#define HAL_SPIDISPLAY_ENABLE             (0)

#endif
