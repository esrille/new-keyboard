/*
 * Copyright 2016 Esrille Inc.
 *
 * This file is a modified version of main.c provided by
 * Microchip Technology, Inc. for using Esrille New Keyboard.
 * See the file NOTICE and the Software License Agreement below for the
 * License.
 */
/*********************************************************************
 * FileName:        usb_config.h
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18
 * Compiler:        C18 3.45+ or XC8 v1.21+
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
 *
 ********************************************************************/

#ifndef USBCFG_H
#define USBCFG_H

//----------------------------------------------------------------------------------------------------------
//User configurable options
//----------------------------------------------------------------------------------------------------------

//When defined/enabled, this option allows the boot up code to check an I/O pin
//(as defined by the sw2() macro in HardwareProfile.h), and if logic low, execution
//stays within the bootloader, allowing the user to update the firmware.  If this
//option is not enabled, then the only method of entering the bootloader will
//be from the application firmware project, by executing a goto 0x001C operation.
//Enabling the I/O pin to enter the bootloader is recommended, since it is more
//robust/recoverable (compared to software only entry into the bootloader), in
//the event of a failed erase/program/verify operation, or an otherwise corrupted
//application firmware image is loaded.
#define ENABLE_IO_PIN_CHECK_BOOTLOADER_ENTRY  //Uncomment if you wish to enable I/O pin entry method into bootloader mode
                                              //Make sure proper sw2() macro definition is provided in HardwareProfile.h

//Option to allow blinking of LED to show USB bus status.  May be optionally
//commented out to save code space (and/or if there are no LEDs available on
//the actual target application board).  If this option is uncommented, you must
//provide the proper LED pin definitions in the HardwareProfile.h file.
#define ENABLE_USB_LED_BLINK_STATUS


//USB VBUS sensing and USB Bus/Self power sensing options.
//---------------------------------------------------------
//#define USE_SELF_POWER_SENSE_IO   //Leave commented if device is bus powered only (or self powered only, uncomment for some types of dual powered devices)
#define USE_USB_BUS_SENSE_IO      //If the device is self powered, this needs to uncommented if making a fully compliant USB design




//----------------------------------------------------------------------------------------------------------
//Other semi-configurable settings that tell the USB stack how to operate (but usually don't need changing)
//----------------------------------------------------------------------------------------------------------
#define MAX_EP_NUMBER           1   // EP0 and EP1 are the only EPs used in this application
#define MAX_NUM_INT             1   // For tracking Alternate Setting - make sure this matches the number of interfaces implemented in the device
#define EP0_BUFF_SIZE           8   // Valid Options: 8, 16, 32, or 64 bytes.
                                    // There is little advantage in using
                                    // more than 8 bytes on EP0 IN/OUT in most cases.
#define USB_MAX_NUM_CONFIG_DSC  1   // Number of configurations that this firmware implements
//#define ENABLE_CONTROL_TRANSFERS_WITH_OUT_DATA_STAGE    //Commented out to save code size, since this bootloader firmware doesn't use OUT control transfers with data stage


#define CONFIG_DESC_TOTAL_LEN    41     //Make sure this matches the size of your configuration descriptor + all subordinate
                                        //descriptors returned by the get descriptor(configuration) request

/* Parameter definitions are defined in usb_device.h */
#define MODE_PP                 _PPBM1  //This code is only written to support _PPBM1 mode only (ping pong on EP0 OUT buffer only).  Do not change.
#define UCFG_VAL                _PUEN|_TRINT|_FS|MODE_PP

//Device class and endpoint definitions
#define USB_USE_HID

/* HID */
#define HID_INTF_ID             0x00
#define HID_UEP                 UEP1
#define HID_BD_OUT              ep1Bo
#define HID_INT_OUT_EP_SIZE     64
#define HID_BD_IN               ep1Bi
#define HID_INT_IN_EP_SIZE      64
#define HID_NUM_OF_DSC          1       //Just the Report descriptor (no physical descriptor present)
#define HID_RPT01_SIZE          29      //Make sure this matches the size of your HID report descriptor




#endif //USBCFG_H
