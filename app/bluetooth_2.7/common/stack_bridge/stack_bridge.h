/**************************************************************************//**
 * \file   stack_bridge.h
 * \brief  Infrastructure to integrate corss-stack codes to BLE SDK
 ******************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ******************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *****************************************************************************/

#ifndef __STACK_BRIDGE__
#define __STACK_BRIDGE__

#define MEMCOPY(dest, src, len)       memcpy(dest, src, len)
#define MEMSET(dest, val, len)        memset(dest, val, len)
#define MEMCOMPARE(data1, data2, len) memcmp(data1, data2, len)

#define HIGH_BYTE(n)  UINT16_TO_BYTE1(n);
#define LOW_BYTE(n)   UINT16_TO_BYTE0(n);

#define BYTE_0(n)     UINT32_TO_BYTE0(n)
#define BYTE_1(n)     UINT32_TO_BYTE1(n)
#define BYTE_2(n)     UINT32_TO_BYTE2(n)
#define BYTE_3(n)     UINT32_TO_BYTE3(n)

#endif // __STACK_BRIDGE__
