/***************************************************************************//**
 * @file descriptors.h
 * @brief USB descriptor prototypes for HID keyboard example project.
 * @version 5.3.5
 *******************************************************************************
 * # License
 * <b>Copyright 2015 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/
#ifndef __SILICON_LABS_DESCRIPTORS_H__
#define __SILICON_LABS_DESCRIPTORS_H__

#include "em_usb.h"
#include "hidkbd.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const HIDKBD_KeyReport_t             USBDESC_reportTable[15];
extern const HIDKBD_KeyReport_t             USBDESC_noKeyReport;
extern const void *                         USBDESC_HidDescriptor;

extern const USB_DeviceDescriptor_TypeDef   USBDESC_deviceDesc;
extern const uint8_t                        USBDESC_configDesc[];
extern const void * const                   USBDESC_strings[4];
extern const uint8_t                        USBDESC_bufferingMultiplier[];

#ifdef __cplusplus
}
#endif

#endif /* __SILICON_LABS_DESCRIPTORS_H__ */
