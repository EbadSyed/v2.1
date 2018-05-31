// -----------------------------------------------------------------------------
/// @file ember-hal-config.h
/// @brief HAL configuration legacy ember support header
///
/// @section License
/// <b>(C) Copyright 2017 Silicon Laboratories, http://www.silabs.com</b>
///
/// This file is licensed under the Silabs License Agreement. See the file
/// "Silabs_License_Agreement.txt" for details. Before using this software for
/// any purpose, you must agree to the terms of that agreement.
///
// -----------------------------------------------------------------------------
#ifndef EMBER_HAL_CONFIG_H
#define EMBER_HAL_CONFIG_H
/* BUTTON */
// Support legacy BUTTONn parameter
  #ifdef BSP_BUTTON0_PIN
  #define BUTTON0 BSP_BUTTON0_PIN
  #endif
  #ifdef BSP_BUTTON1_PIN
  #define BUTTON1 BSP_BUTTON1_PIN
  #endif
  #ifdef BSP_BUTTON2_PIN
  #define BUTTON2 BSP_BUTTON2_PIN
  #endif
  #ifdef BSP_BUTTON3_PIN
  #define BUTTON3 BSP_BUTTON3_PIN
  #endif
  #ifdef BSP_BUTTON4_PIN
  #define BUTTON4 BSP_BUTTON4_PIN
  #endif
  #ifdef BSP_BUTTON5_PIN
  #define BUTTON5 BSP_BUTTON5_PIN
  #endif
  #ifdef BSP_BUTTON6_PIN
  #define BUTTON6 BSP_BUTTON6_PIN
  #endif
  #ifdef BSP_BUTTON7_PIN
  #define BUTTON7 BSP_BUTTON7_PIN
  #endif

/* SERIAL */
// Override legacy APP_SERIAL define with BSP_SERIAL_APP_PORT
  #ifdef BSP_SERIAL_APP_PORT
  #undef APP_SERIAL
  #define APP_SERIAL BSP_SERIAL_APP_PORT
  #else // support alternatively APP_SERIAL or BSP_SERIAL_APP_PORT
  #ifdef APP_SERIAL
    #define BSP_SERIAL_APP_PORT APP_SERIAL
  #endif
  #endif
  #define EMBER_AF_SERIAL_PORT_INIT() emberSerialInit(BSP_SERIAL_APP_PORT, HAL_SERIAL_APP_BAUD_RATE, PARITY_NONE, 1);
  #if defined(EMBER_ASSERT_SERIAL_PORT) && defined (BSP_SERIAL_ASSERT_PORT)
  #undef EMBER_ASSERT_SERIAL_PORT
  #endif
  #ifdef BSP_SERIAL_ASSERT_PORT
  #define EMBER_ASSERT_SERIAL_PORT BSP_SERIAL_ASSERT_PORT
  #endif

/* ANTDIV */
  #if HAL_ANTDIV_ENABLE
  #ifdef BSP_ANTDIV_SEL_PORT
    #define ANTENNA_SELECT_GPIO   ((BSP_ANTDIV_SEL_PORT << 4) | BSP_ANTDIV_SEL_PIN)
  #endif
  #ifdef BSP_ANTDIV_NSEL_PORT
    #define ANTENNA_nSELECT_GPIO  ((BSP_ANTDIV_NSEL_PORT << 4) | BSP_ANTDIV_NSEL_PIN)
  #endif
  #endif

/* COEX */
  #if HAL_COEX_ENABLE
  #define ENABLE_PTA 1
  #ifdef BSP_COEX_REQ_PORT
    #define PTA_REQ_GPIO      ((BSP_COEX_REQ_PORT << 4) | BSP_COEX_REQ_PIN)
  #endif
  #ifdef BSP_COEX_GNT_PORT
    #define PTA_GNT_GPIO      ((BSP_COEX_GNT_PORT << 4) | BSP_COEX_GNT_PIN)
  #endif
  #ifdef BSP_COEX_PRI_PORT
    #define PTA_PRI_GPIO      ((BSP_COEX_PRI_PORT << 4) | BSP_COEX_PRI_PIN)
  #endif
  #ifdef BSP_COEX_REQ_ASSERT_LEVEL
    #define PTA_REQ_ASSERTED  BSP_COEX_REQ_ASSERT_LEVEL
  #endif
  #ifdef BSP_COEX_GNT_ASSERT_LEVEL
    #define PTA_GNT_ASSERTED  BSP_COEX_GNT_ASSERT_LEVEL
  #endif
  #ifdef BSP_COEX_PRI_ASSERT_LEVEL
    #define PTA_PRI_ASSERTED  BSP_COEX_PRI_ASSERT_LEVEL
  #endif
  #ifdef BSP_COEX_RHO_PORT
    #define RADIO_HOLDOFF 1
    #define RHO_GPIO ((BSP_COEX_RHO_PORT << 4) | BSP_COEX_RHO_PIN)
  #endif
  #ifdef BSP_COEX_RHO_ASSERT_LEVEL
    #define RHO_ASSERTED BSP_COEX_RHO_ASSERT_LEVEL
  #endif
  #if HAL_COEX_REQ_SHARED
    #define PTA_REQ_SHARED  HAL_COEX_REQ_SHARED
  #endif
  #if HAL_COEX_PRI_SHARED
    #define PTA_PRI_SHARED  HAL_COEX_PRI_SHARED
  #endif
  #if HAL_COEX_RETRYRX_ENABLE
    #define PTA_RX_RETRY_REQ HAL_COEX_RETRYRX_ENABLE
  #endif
  #if HAL_COEX_TX_ABORT
    #define ENABLE_PTA_OPT_ABORT_TX HAL_COEX_TX_ABORT
  #endif
  #ifdef HAL_COEX_RETRYRX_TIMEOUT
    #define PTA_RX_RETRY_TIMEOUT_MS HAL_COEX_RETRYRX_TIMEOUT
  #endif
  #if HAL_COEX_RETRYRX_HIPRI
    #define ENABLE_PTA_OPT_RX_RETRY_HI_PRI HAL_COEX_RETRYRX_HIPRI
  #endif
  #if HAL_COEX_TX_HIPRI
    #define ENABLE_PTA_OPT_TX_HI_PRI HAL_COEX_TX_HIPRI
  #endif
  #if HAL_COEX_RX_HIPRI
    #define ENABLE_PTA_OPT_RX_HI_PRI HAL_COEX_RX_HIPRI
  #endif
  #if HAL_COEX_ACKHOLDOFF
    #define ENABLE_PTA_OPT_ACK_HOLDOFF HAL_COEX_ACKHOLDOFF
  #endif
  #ifdef HAL_COEX_REQ_BACKOFF
  #define PTA_REQ_MAX_BACKOFF_MASK HAL_COEX_REQ_BACKOFF
  #else
  #define PTA_REQ_MAX_BACKOFF_MASK 0
  #endif
  #endif //HAL_COEX_ENABLE

/* EXTDEV/EZRADIOPRO */
  #if HAL_EZRADIOPRO_ENABLE
    #define HAL_EXTDEV_ENABLE    1
    #ifndef HAL_EZRADIOPRO_FREQ
      #define HAL_EZRADIOPRO_FREQ 1000000
    #endif
    #define BSP_EXTDEV_CS_PIN    BSP_EZRADIOPRO_CS_PIN
    #define BSP_EXTDEV_CS_PORT   BSP_EZRADIOPRO_CS_PORT
    #define BSP_EXTDEV_INT_PIN   BSP_EZRADIOPRO_INT_PIN
    #define BSP_EXTDEV_INT_PORT  BSP_EZRADIOPRO_INT_PORT
    #define BSP_EXTDEV_SDN_PIN   BSP_EZRADIOPRO_SDN_PIN
    #define BSP_EXTDEV_SDN_PORT  BSP_EZRADIOPRO_SDN_PORT
  #endif //HAL_EZRADIOPRO_ENABLE

#endif //EMBER_HAL_CONFIG_H
