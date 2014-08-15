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
#include <system.h>
#include <buttons.h>

// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Constants
// *****************************************************************************
// *****************************************************************************

static unsigned char trisA = 0x00;
static unsigned char trisE = 0x03;
static unsigned char portD = 0xFC;

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

    if (3 <= BOARD_REV_VALUE) {
        trisA = 0x01;
        trisE = 0x07;
    }
    if (4 <= BOARD_REV_VALUE)
        portD = 0xF3;
    TRISA = trisA;
    TRISE = 0x00;
    result = (~PORTD & portD) || (~PORTB & 0x3F);
    TRISA = 0x3F;
    TRISE = trisE;
    return result;
}


/*******************************************************************************
 End of File
*/
