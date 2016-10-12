/*
 * Copyright 2014-2016 Esrille Inc.
 *
 * This file is a modified version of boot_18fxxjxx.h provided by
 * Microchip Technology, Inc. for using Esrille New Keyboard.
 * See the file NOTICE and the Software License Agreement below for the
 * License.
 */
/*********************************************************************
 *
 * Microchip USB C18 Firmware -  HID Bootloader for PIC18FXXJXX USB Devices
 *
 *********************************************************************
 * FileName:        boot_18fxxjxx.h
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18FxxJxx USB microcontrollers
 * Compiler:        C18 3.46+ or XC8 v1.21+
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
 * File version     Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 1.0              06/03/2008  Original
 * 2.9j             06/11/13    Added some definitions.
 ********************************************************************/
#ifndef BOOT18FXXJXX_H
#define BOOT18FXXJXX_H

/** P U B L I C  P R O T O T Y P E S *****************************************/
void UserInit(void);
void ProcessIO(void);
void ClearWatchdog(void);
void DisableUSBandExecuteLongDelay(void);


//Vector remapping/absolute address constants
#define REMAPPED_APPLICATION_RESET_VECTOR       0x2000
//#define REMAPPED_APPLICATION_HIGH_ISR_VECTOR    0x2008        //See VectorRemap.asm
//#define REMAPPED_APPLICATION_LOW_ISR_VECTOR     0x2018        //See VectorRemap.asm
#define BOOTLOADER_ABSOLUTE_ENTRY_ADDRESS       0x001C  //Execute a "goto 0x001C" inline assembly instruction, if you want to enter the bootloader mode from the application via software

#define APP_SIGNATURE_ADDRESS                   0x2006  //0x2006 and 0x2007 contains the "signature" WORD, indicating successful erase/program/verify operation
#define APP_SIGNATURE_VALUE                     0x600D  //leet "GOOD", implying that the erase/program was a success and the bootloader intentionally programmed the APP_SIGNATURE_ADDRESS with this value
#define APP_VERSION_ADDRESS                     0x1F7F8 //0x1F7F8 and 0x1F7F9 should contain the application image firmware version number

//
// Esrille New Keyboard
//
#define APP_MACHINE_ADDRESS                     0x2004  // The application image machine type number address
#define APP_MACHINE_VALUE                       0x4753  // PIC18F47J53

// The first 16 bytes of RAM is reserved for passing configuration
// information from the HID bootloader to the application firmware.
#define BOOT_FLAGS_ADDRESS                      0x0000  // in RAM
#define BOOT_WITH_ESC                           0x0001
#define BOOT_WITH_APP                           0x0002

#define BOARD_REV_ADDRESS                       0x1FFE
#define BOARD_REV_VALUE                         ESRILLE_NEW_KEYBOARD

#endif //BOOT18FXXJXX_H
