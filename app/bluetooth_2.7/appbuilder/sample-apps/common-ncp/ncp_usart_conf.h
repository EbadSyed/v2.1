/***********************************************************************************************//**
 * \file   ncp_usart_conf.h
 * \brief  Silabs Network Co-Processor (NCP) library USART driver configuration
 *         This library allows customers create applications work in NCP mode.
 ***************************************************************************************************
 * <b> (C) Copyright 2017 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#ifndef NCP_UART_CONF_H_
#define NCP_UART_CONF_H_

#include "hal-config.h"

#define NCP_USART_STOPBITS    usartStopbits1  // Number of stop bits
#define NCP_USART_PARITY      usartNoParity   // Parity configuration
#define NCP_USART_OVS         usartOVS16      // Oversampling mode
#define NCP_USART_MVDIS       0               // Majority Vote Disable for 16x, 8x

// USART flow control enabled
#define NCP_USART_FLOW_CONTROL_ENABLED

#if defined(NCP_USART_FLOW_CONTROL_ENABLED)
  #define NCP_USART_FLOW_CONTROL_TYPE    uartdrvFlowControlHwUart
#else
  #define NCP_USART_FLOW_CONTROL_TYPE    uartdrvFlowControlNone
#endif

// Define if NCP sleep functionality is requested
//#define NCP_DEEP_SLEEP_ENABLED

#if defined(NCP_DEEP_SLEEP_ENABLED)
  #define NCP_WAKEUP_PORT
  #define NCP_WAKEUP_PIN
  #define NCP_WAKEUP_POLARITY
#endif

// Define if NCP wakeup functionality is requested
//#define NCP_HOST_WAKEUP_ENABLED

#if defined(NCP_HOST_WAKEUP_ENABLED)
  #define NCP_HOST_WAKEUP_PORT
  #define NCP_HOST_WAKEUP_PIN
  #define NCP_HOST_WAKEUP_POLARITY
#endif

#endif /* NCP_UART_CONF_H_ */
