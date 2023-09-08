/*
 * Copyright 2014-2023 Esrille Inc.
 *
 * This file is a modified version of usb_config.h provided by
 * Microchip Technology, Inc. for using Esrille New Keyboard.
 * See the Software License Agreement below for the License.
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

 THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *******************************************************************/

/*********************************************************************
 * Descriptor specific type definitions are defined in: usbd.h
 ********************************************************************/

#ifndef USBCFG_H
#define USBCFG_H

#include "usb/usb_ch9.h"

/** DEFINITIONS ****************************************************/
#define USB_EP0_BUFF_SIZE       8   // Valid Options: 8, 16, 32, or 64 bytes.
                                    // Using larger options take more SRAM, but
                                    // does not provide much advantage in most types
                                    // of applications.  Exceptions to this, are applications
                                    // that use EP0 IN or OUT for sending large amounts of
                                    // application related data.

#ifndef ENABLE_MOUSE
#define USB_MAX_NUM_INT         2
#define USB_MAX_EP_NUMBER       2
#else
#define USB_MAX_NUM_INT         3   //Set this number to match the maximum interface number used in the descriptors for this firmware project
#define USB_MAX_EP_NUMBER       3   //Set this number to match the maximum endpoint number used in the descriptors for this firmware project
#endif

//Make sure only one of the below "#define USB_PING_PONG_MODE"
//is uncommented.
//#define USB_PING_PONG_MODE USB_PING_PONG__NO_PING_PONG
#define USB_PING_PONG_MODE USB_PING_PONG__FULL_PING_PONG
//#define USB_PING_PONG_MODE USB_PING_PONG__EP0_OUT_ONLY
//#define USB_PING_PONG_MODE USB_PING_PONG__ALL_BUT_EP0		//NOTE: This mode is not supported in PIC18F4550 family rev A3 devices


//#define USB_POLLING
#define USB_INTERRUPT

/* Parameter definitions are defined in usb_device.h */
#define USB_PULLUP_OPTION USB_PULLUP_ENABLE
//#define USB_PULLUP_OPTION USB_PULLUP_DISABLED

#define USB_TRANSCEIVER_OPTION USB_INTERNAL_TRANSCEIVER
//External Transceiver support is not available on all product families.  Please
//  refer to the product family datasheet for more information if this feature
//  is available on the target processor.
//#define USB_TRANSCEIVER_OPTION USB_EXTERNAL_TRANSCEIVER

#define USB_SPEED_OPTION USB_FULL_SPEED
//#define USB_SPEED_OPTION USB_LOW_SPEED //(not valid option for PIC24F devices)

#define MY_VID 0x04D8
#define MY_PID 0xF550   // Esrille New Keyboard

//------------------------------------------------------------------------------------------------------------------
//Option to enable auto-arming of the status stage of control transfers, if no
//"progress" has been made for the USB_STATUS_STAGE_TIMEOUT value.
//If progress is made (any successful transactions completing on EP0 IN or OUT)
//the timeout counter gets reset to the USB_STATUS_STAGE_TIMEOUT value.
//
//During normal control transfer processing, the USB stack or the application
//firmware will call USBCtrlEPAllowStatusStage() as soon as the firmware is finished
//processing the control transfer.  Therefore, the status stage completes as
//quickly as is physically possible.  The USB_ENABLE_STATUS_STAGE_TIMEOUTS
//feature, and the USB_STATUS_STAGE_TIMEOUT value are only relevant, when:
//1.  The application uses the USBDeferStatusStage() API function, but never calls
//      USBCtrlEPAllowStatusStage().  Or:
//2.  The application uses host to device (OUT) control transfers with data stage,
//      and some abnormal error occurs, where the host might try to abort the control
//      transfer, before it has sent all of the data it claimed it was going to send.
//
//If the application firmware never uses the USBDeferStatusStage() API function,
//and it never uses host to device control transfers with data stage, then
//it is not required to enable the USB_ENABLE_STATUS_STAGE_TIMEOUTS feature.

#define USB_ENABLE_STATUS_STAGE_TIMEOUTS    //Comment this out to disable this feature.

//Section 9.2.6 of the USB 2.0 specifications indicate that:
//1.  Control transfers with no data stage: Status stage must complete within
//      50ms of the start of the control transfer.
//2.  Control transfers with (IN) data stage: Status stage must complete within
//      50ms of sending the last IN data packet in fulfillment of the data stage.
//3.  Control transfers with (OUT) data stage: No specific status stage timing
//      requirement.  However, the total time of the entire control transfer (ex:
//      including the OUT data stage and IN status stage) must not exceed 5 seconds.
//
//Therefore, if the USB_ENABLE_STATUS_STAGE_TIMEOUTS feature is used, it is suggested
//to set the USB_STATUS_STAGE_TIMEOUT value to timeout in less than 50ms.  If the
//USB_ENABLE_STATUS_STAGE_TIMEOUTS feature is not enabled, then the USB_STATUS_STAGE_TIMEOUT
//parameter is not relevant.

#define USB_STATUS_STAGE_TIMEOUT     (uint8_t)45    //Approximate timeout in milliseconds, except when
                                                    //USB_POLLING mode is used, and USBDeviceTasks() is called at < 1kHz
                                                    //In this special case, the timeout becomes approximately:
//Timeout(in milliseconds) = ((1000 * (USB_STATUS_STAGE_TIMEOUT - 1)) / (USBDeviceTasks() polling frequency in Hz))
//------------------------------------------------------------------------------------------------------------------

#define USB_SUPPORT_DEVICE

#define USB_NUM_STRING_DESCRIPTORS  3

/** DEVICE CLASS USAGE *********************************************/
#define USB_USE_HID

/** ENDPOINTS ALLOCATION *******************************************/

/* HID - Keyboard */
#define HID_INTF_ID                 0x00
#define HID_EP                      1
#define HID_INT_OUT_EP_SIZE         1
#define HID_INT_IN_EP_SIZE          8
#define HID_RPT01_SIZE              64
//#define USER_GET_REPORT_HANDLER USBHIDCBGetReportHandler
#define USER_SET_REPORT_HANDLER USBHIDCBSetReportHandler

/* HID - Consumer Control */
#define HID_CC_INTF_ID              0x01
#define HID_CC_EP                   2
#define HID_CC_INT_OUT_EP_SIZE      2
#define HID_CC_INT_IN_EP_SIZE       2
#define HID_RPT02_SIZE              23

/* HID - Mouse */
#define HID_MOUSE_INTF_ID           0x02
#define HID_MOUSE_EP                3
#define HID_MOUSE_INT_OUT_EP_SIZE   3
#define HID_MOUSE_INT_IN_EP_SIZE    3
#define HID_RPT03_SIZE              52

#define HID_NUM_OF_DSC              1

#ifndef ENABLE_MOUSE
#define HID_NUM_OF_INTF             2
#else
#define HID_NUM_OF_INTF             3
#endif

/** DEFINITIONS ****************************************************/

#endif //USBCFG_H
