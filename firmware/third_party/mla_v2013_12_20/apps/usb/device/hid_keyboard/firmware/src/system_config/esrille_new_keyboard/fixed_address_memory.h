/*
 * Copyright 2014-2017 Esrille Inc.
 *
 * This file is a modified version of fixed_address_memory.h provided by
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
 *******************************************************************/

#ifndef FIXED_MEMORY_ADDRESS_H
#define FIXED_MEMORY_ADDRESS_H

#define FIXED_ADDRESS_MEMORY

#define KEYBOARD_INPUT_REPORT_DATA_BUFFER_ADDRESS_TAG   @0x500
#define KEYBOARD_OUTPUT_REPORT_DATA_BUFFER_ADDRESS_TAG  @0x508

#define MOUSE_REPORT_DATA_BUFFER_ADDRESS                0x50A

#define APP_VERSION_ADDRESS     0x1826  // The application image firmware version number address
#define APP_VERSION_VALUE       0x0022  // BCD

#define BOARD_REV_ADDRESS       0x17FE
#define BOARD_REV_VALUE         (*(const unsigned int*) BOARD_REV_ADDRESS)

#define APP_MACHINE_VALUE       0x4550  // PIC18F4550

#endif //FIXED_MEMORY_ADDRESS
