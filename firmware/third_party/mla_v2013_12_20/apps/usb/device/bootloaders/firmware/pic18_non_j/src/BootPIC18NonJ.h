/*
 * Copyright 2014 Esrille Inc.
 *
 * This file is a modified version of BootPIC18NonJ.h provided by
 * Microchip Technology, Inc. for using Esrille New Keyboard.
 * See the Software License Agreement below for the License.
 */

/*********************************************************************
 *
 *   Microchip USB HID Bootloader v1.01 for PIC18F and PIC18LF versions of:
 *	 PIC18F4553/4458/2553/2458
 *	 PIC18F4550/4455/2550/2455
 *	 PIC18F4450/2450
 *	 PIC18F14K50/13K50
 *
 *********************************************************************
 * FileName:        BootPIC18NonJ.h
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18Fxxxxx (with no J in the part number) USB microcontrollers
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
 * 1.0				06/03/2008	Original
 * 2.9j             06/11/13    Added some definitions. 
 ********************************************************************/
#ifndef BOOTPIC18NONJ_H
#define BOOTPIC18NONJ_H

/** P U B L I C  P R O T O T Y P E S *****************************************/
void UserInit(void);
void ProcessIO(void);
void ClearWatchdog(void);
void DisableUSBandExecuteLongDelay(void);


//Vector remapping/absolute address constants
#define REMAPPED_APPLICATION_RESET_VECTOR       0x1800
//#define REMAPPED_APPLICATION_HIGH_ISR_VECTOR    0x1808        //See VectorRemap.asm
//#define REMAPPED_APPLICATION_LOW_ISR_VECTOR     0x1818        //See VectorRemap.asm
#define BOOTLOADER_ABSOLUTE_ENTRY_ADDRESS       0x001C  //Execute a "goto 0x001C" inline assembly instruction, if you want to enter the bootloader mode from the application via software

#define APP_SIGNATURE_ADDRESS                   0x1806  //0x1006 and 0x1007 contains the "signature" WORD, indicating successful erase/program/verify operation
#define APP_SIGNATURE_VALUE                     0x600D  //leet "GOOD", implying that the erase/program was a success and the bootloader intentionally programmed the APP_SIGNATURE_ADDRESS with this value
#define APP_VERSION_ADDRESS                     0x1816  //0x1016 and 0x1017 should contain the application image firmware version number

//
// Esrille New Keyboard
//
#define BOARD_REV_ADDRESS                       0x17FE
#define BOARD_REV_VALUE                         0x0001

#endif //BOOTPIC18NONJ_H
