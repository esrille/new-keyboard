/*
 * Copyright 2023 Esrille Inc.
 *
 * This file is a modified version of app_device_mouse.c provided by
 * Microchip Technology, Inc. for using Esrille New Keyboard.
 * See the file NOTICE for copying permission.
 */

/********************************************************************
 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the "Company") for its PIC(R) Microcontroller is intended and
 supplied to you, the Company's customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
********************************************************************/

/** INCLUDES *******************************************************/
#include <system.h>

#include <stdint.h>

#include <usb/usb.h>
#include <usb/usb_device.h>
#include <usb/usb_device_hid.h>

#include <app_led_usb_status.h>
#include <app_device_cc.h>
#include <usb_config.h>

#include "Keyboard.h"

#define HID_USAGE_CONSUMER_MUTE                 0x00E2
#define HID_USAGE_CONSUMER_VOLUME_INCREMENT     0x00E9
#define HID_USAGE_CONSUMER_VOLUME_DECREMENT     0x00EA

/*******************************************************************************
 * HID Report Descriptor - this describes the data format of the reports that
 * are sent between the host and the device.
 *
 * In this example there are only one INPUT report.  This report descriptor can
 * be generated using the HID descriptor tool available at www.usb.org.
 ******************************************************************************/
const struct{uint8_t report[HID_RPT02_SIZE];}hid_rpt02=
{
    {
        0x05, 0x0C,         /* Usage Page (Consumer Devices)    */
        0x09, 0x01,         /* Usage (Consumer Control)         */
        0xA1, 0x01,         /* Collection (Application)         */
        0x15, 0x00,         /*   Logical Minimum (0)            */
        0x26, 0xFF, 0x03,   /*   Logical Maximum (0x03FF)       */
        0x19, 0x00,         /*   Usage Minimum (0)              */
        0x2A, 0xFF, 0x03,   /*   Usage Maximum (0x03FF)         */
        0x95, 0x01,         /*   Report Count (1)               */
        0x75, 0x10,         /*   Report Size (16)               */
        0x81, 0x00,         /*   Input (Data,Ary,Abs)           */
        0xC0                /* End Collection                   */
    }
};

/*******************************************************************************
 * Report Data Types - These typedefs will mirror the reports defined in the
 * HID report descriptor so that the application has an easy way to see what
 * the report will look like as well as access/modify each of the members of the
 * report.
 ******************************************************************************/

/* INPUT report - this structure will represent the only INPUT report in the HID
 * descriptor.
 */
typedef struct __attribute__((packed))
{
    uint16_t cc;
} CC_REPORT;

/** VARIABLES ******************************************************/
/* Some processors have a limited range of RAM addresses where the USB module
 * is able to access.  The following section is for those devices.  This section
 * assigns the buffers that need to be used by the USB module into those
 * specific areas.
 */
#if defined(FIXED_ADDRESS_MEMORY)
    #if defined(COMPILER_MPLAB_C18)
        #pragma udata CC_REPORT_DATA_BUFFER=CC_REPORT_DATA_BUFFER_ADDRESS
            static CC_REPORT ccReport;
        #pragma udata
    #elif defined(__XC8)
        static CC_REPORT ccReport @ CC_REPORT_DATA_BUFFER_ADDRESS;
    #endif
#else
    static CC_REPORT ccReport;
#endif

typedef struct
{
    USB_HANDLE lastINTransmission;
} CC;

static CC cc;

/*********************************************************************
* Function: void APP_DeviceConsumerInitialize(void);
*
* Overview: Initializes the demo code
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceConsumerInitialize(void)
{
    /* initialize the handles to invalid so we know they aren't being used. */
    cc.lastINTransmission = NULL;

    //enable the HID endpoint
    USBEnableEndpoint(HID_CC_EP,USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
}//end UserInit

/*********************************************************************
* Function: void APP_DeviceConsumerTasks(void);
*
* Overview: Keeps the demo running.
*
* PreCondition: The demo should have been initialized and started via
*   the APP_DeviceMouseInitialize() and APP_DeviceMouseStart() demos
*   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/

static void shiftReport(uint8_t* report, uint8_t i)
{
    memmove(report + i, report + i + 1, 7 - i);
    report[7] = 0;
}

void APP_DeviceConsumerTasks(uint8_t* report)
{
    static uint16_t prev = 0x400;
    uint16_t code = 0;

    for (int8_t i = 2; i < 8; ++i) {
        switch (report[i]) {
        case KEY_MUTE:
            code = HID_USAGE_CONSUMER_MUTE;
            shiftReport(report, i);
            --i;
            break;
        case KEY_VOLUME_DOWN:
            code = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
            shiftReport(report, i);
            --i;
            break;
        case KEY_VOLUME_UP:
            code = HID_USAGE_CONSUMER_VOLUME_INCREMENT;
            shiftReport(report, i);
            --i;
            break;
        default:
            break;
        }
    }
    if (prev != code && HIDTxHandleBusy(cc.lastINTransmission) == false) {
        prev = code;
        ccReport.cc = code;
        cc.lastINTransmission = HIDTxPacket(HID_CC_EP, (uint8_t*) &ccReport, sizeof ccReport);
    }
}//end ProcessIO
