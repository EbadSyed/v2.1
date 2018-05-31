#ifndef HAL_CONFIG_LNA_H
#define HAL_CONFIG_LNA_H

// $[LNA]
#define HAL_LNA_ENABLE                    (1)

#define BSP_LNA_SLEEP_PIN                 (11)
#define BSP_LNA_SLEEP_PORT                (gpioPortD)
#define BSP_LNA_SLEEP_LOC                 (13)

#define BSP_LNA_TXRX_PIN                  (10)
#define BSP_LNA_TXRX_PORT                 (gpioPortD)
#define BSP_LNA_TXRX_LOC                  (0)

#define BSP_LNA_SLEEP_CHANNEL             (6)
#define BSP_LNA_TXRX_CHANNEL              (5)
// [LNA]$

#endif /* HAL_CONFIG_LNA_H */
