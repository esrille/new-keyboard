/*********************************************************************
 * FileName:        usb.h
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18
 * Compiler:        C18 v3.46+ or XC8 v1.21+
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the "Company") for its PIC(R) Microcontroller is intended and
 * supplied to you, the Company's customer, for use solely and
 * exclusively on Microchip PIC Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

 ********************************************************************/
#ifndef USB_H
#define USB_H

/*
 * usb.h provides a centralize way to include all files
 * required by Microchip USB Firmware.
 *
 * The order of inclusion is important.
 * Dependency conflicts are resolved by the correct ordering.
 */

#include "typedefs.h"
#include "usb_config.h"
#include "usb_device.h"
#include "HardwareProfile.h"

#if defined(USB_USE_HID)                // See usb_config.h
#include "usb_device_hid.h"
#endif


//These callback functions belong in your main.c (or equivalent) file.  The USB
//stack will call these callback functions in response to specific USB bus events,
//such as entry into USB suspend mode, exit from USB suspend mode, and upon
//receiving the "set configuration" control tranfer request, which marks the end
//of the USB enumeration sequence and the start of normal application run mode (and
//where application related variables and endpoints may need to get (re)-initialized.
void USBCBSuspend(void);
void USBCBWakeFromSuspend(void);
void USBCBInitEP(uint8_t ConfigurationIndex);
void USBCBCheckOtherReq(void);


//API renaming wrapper functions
#define HIDTxHandleBusy(a)   {mHIDTxIsBusy()}
#define HIDRxHandleBusy(a)   {mHIDRxIsBusy()}

#endif //USB_H
