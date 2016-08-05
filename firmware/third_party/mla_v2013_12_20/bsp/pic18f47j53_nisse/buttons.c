/*
 * Copyright 2014-2016 Esrille Inc.
 *
 * This file is a modified version of buttons.c provided by
 * Microchip Technology, Inc. for using Esrille New Keyboard.
 * See the file NOTICE for copying permission.
 */

/*******************************************************************************
  Demo board push button abstraction layer for PICDEM FS USB board.

  Company:
    Microchip Technology Inc.

  File Name:
    buttons.c

  Summary:
    Provides simple interface for pushbuttons on the PICDEM FS USB board.

  Description:
    Provides simple interface for pushbuttons on the PICDEM FS USB board.
*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <system.h>
#include <buttons.h>

// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Constants
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Macros or Functions
// *****************************************************************************
// *****************************************************************************

/*********************************************************************
* Function: bool BUTTON_IsPressed( );
*
* Overview: Returns the current state of the buttons
*
* PreCondition: button configured via BUTTON_SetConfiguration()
*
* Output: TRUE if any one of keys is pressed; FALSE if not pressed.
*
********************************************************************/
bool BUTTON_IsPressed()
{
    bool result;

    INTCON2bits.RBPU = 0;   // Enable RBPU
    TRISA = 0x00;
    TRISE = 0x80;           // Enable RDPU
    Nop(); Nop(); Nop(); Nop(); Nop();
    result = (~PORTD & 0xCE) || (~PORTB & 0xEF);
    TRISA = 0x2F;
    TRISE = 0x07;           // Disable RDPU
    INTCON2bits.RBPU = 1;   // Disable RBPU
    return result;
}


/*******************************************************************************
 End of File
*/
