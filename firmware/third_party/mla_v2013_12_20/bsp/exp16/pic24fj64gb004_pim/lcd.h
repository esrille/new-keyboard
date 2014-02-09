/******************************************************************************
Software License Agreement

The software supplied herewith by Microchip Technology Incorporated
(the "Company") for its PIC(R) Microcontroller is intended and
supplied to you, the Company's customer, for use solely and
exclusively on Microchip PICmicro Microcontroller products. The
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
*******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * NOTE: For the LCD to work with this PIM, S1 must be either set to the PGX2
 * position or the appplication must be programmed in RUN mode and not DEBUG
 * mode.  PGX1 overlaps the LCD pins and in DEBUG mode, the PGX1 pins will be
 * used by the debugger.
 *
 * Also note that PGX2 is overlapped with D+ and D- so for USB applications,
 * DEBUG mode also can't be used as this will prevent the USB module from
 * gaining control over the D+ and D- lines required for communication.
 *
 * Thus for USB applications that want to use the LCD, debugging with this PIM
 * is not possible.  The PIM can only be programmed in the normal RUN mode.
 ******************************************************************************/

/*********************************************************************
* Function: bool LCD_Initialize(void);
*
* Overview: Initializes the LCD screen.  Can take several hundred
*           milliseconds.
*
* PreCondition: none
*
* Input: None
*
* Output: true if successful, false otherwise
*
********************************************************************/
bool LCD_Initialize(void);

/*********************************************************************
* Function: void LCD_PutString(char* inputString, uint16_t length);
*
* Overview: Puts a string on the LCD screen.  Unsupported characters will be
*           discarded.  May block or throw away characters is LCD is not ready
*           or buffer space is not available.  Will terminate when either a
*           null terminator character (0x00) is reached or the length number
*           of characters is printed, which ever comes first.
*
* PreCondition: already initialized via LCD_Initialize()
*
* Input: char* - string to print
*        uint16_t - length of string to print
*
* Output: None
*
********************************************************************/
void LCD_PutString(char* inputString, uint16_t length);

/*********************************************************************
* Function: void LCD_PutChar(char);
*
* Overview: Puts a character on the LCD screen.  Unsupported characters will be
*           discarded.  May block or throw away characters is LCD is not ready
*           or buffer space is not available.
*
* PreCondition: already initialized via LCD_Initialize()
*
* Input: char - character to print
*
* Output: None
*
********************************************************************/
void LCD_PutChar(char);

/*********************************************************************
* Function: void LCD_ClearScreen(void);
*
* Overview: Clears the screen, if possible.
*
* PreCondition: already initialized via LCD_Initialize()
*
* Input: None
*
* Output: None
*
********************************************************************/
void LCD_ClearScreen(void);

/*********************************************************************
* Function: void LCD_CursorEnable(bool enable)
*
* Overview: Enables/disables the cursor
*
* PreCondition: None
*
* Input: bool - specifies if the cursor should be on or off
*
* Output: None
*
********************************************************************/
void LCD_CursorEnable(bool enable);