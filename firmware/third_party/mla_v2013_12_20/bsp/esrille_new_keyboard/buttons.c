/*
 * Copyright 2014 Esrille Inc.
 *
 * This file is a modified version of buttons.c provided by
 * Microchip Technology, Inc. for using Esrille New Keyboard.
 * See the Software License Agreement below for the License.
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
#include <stdbool.h>
#include <xc.h>
#include <buttons.h>

// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Constants
// *****************************************************************************
// *****************************************************************************
//      S1       MCLR reset
#define S2_PORT  PORTBbits.RB4      //AN11
#define S3_PORT  PORTBbits.RB5      

#define S2_TRIS  TRISBbits.TRISB4
#define S3_TRIS  TRISBbits.TRISB5

#define BUTTON_PRESSED      0
#define BUTTON_NOT_PRESSED  1

#define PIN_INPUT           1
#define PIN_OUTPUT          0

#define PIN_DIGITAL         1
#define PIN_ANALOG          0


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
    LATA &= 0xC0;
    LATE &= 0xFC;
    TRISA = 0x00;
    TRISE = 0x00;
    return (~PORTD & 0xFC) || (~PORTB & 0x3F);
}


/*******************************************************************************
 End of File
*/
