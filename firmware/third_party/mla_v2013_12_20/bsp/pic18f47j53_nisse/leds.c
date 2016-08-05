/*
 * Copyright 2014-2016 Esrille Inc.
 *
 * This file is a modified version of leds.c provided by
 * Microchip Technology, Inc. for using Esrille New Keyboard.
 * See the file NOTICE for copying permission.
 */

/*******************************************************************************
  LED demo board abstraction layer for PICDEM FS USB demo board.

  Company:
    Microchip Technology Inc.

  File Name:
    leds.c

  Summary:
    LED demo board abstraction layer for PICDEM FS USB demo board.

  Description:
    LED demo board abstraction layer for PICDEM FS USB demo board.
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

// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Constants
// *****************************************************************************
// *****************************************************************************

static unsigned char led1Bit = 1u << 0;
static unsigned char led2Bit = 1u << 1;
static unsigned char led3Bit = 1u << 2;

static volatile unsigned char* led1Port = &LATC;
static volatile unsigned char* led2Port = &LATC;
static volatile unsigned char* led3Port = &LATC;


// *****************************************************************************
// *****************************************************************************
// Section: Macros or Functions
// *****************************************************************************
// *****************************************************************************

void LED_Initialize(void)
{
    LED_On(LED_D1);
    LED_On(LED_D2);
    LED_On(LED_D3);
}

static void LED_Set(LED led)
{
    switch (led) {
    case LED_D1:
        *led1Port |= led1Bit;
        break;
    case LED_D2:
        *led2Port |= led2Bit;
        break;
    case LED_D3:
        *led3Port |= led3Bit;
        break;
    default:
        break;
    }
}

static void LED_Clear(LED led)
{
    switch (led) {
    case LED_D1:
        *led1Port &= ~led1Bit;
        break;
    case LED_D2:
        *led2Port &= ~led2Bit;
        break;
    case LED_D3:
        *led3Port &= ~led3Bit;
        break;
    default:
        break;
    }
}

/*********************************************************************
* Function: void LED_On(LED led);
*
* Overview: Turns requested LED on
*
* PreCondition: LED configured via LED_Configure()
*
* Input: LED led - enumeration of the LEDs available in this
*        demo.  They should be meaningful names and not the names of
*        the LEDs on the silkscreen on the board (as the demo code may
*        be ported to other boards).
*         i.e. - LED_On(LED_CONNECTION_DETECTED);
*
* Output: none
*
********************************************************************/
void LED_On(LED led)
{
    LED_Set(led);
}

/*********************************************************************
* Function: void LED_Off(LED led);
*
* Overview: Turns requested LED off
*
* PreCondition: LED configured via LEDConfigure()
*
* Input: LED led - enumeration of the LEDs available in this
*        demo.  They should be meaningful names and not the names of
*        the LEDs on the silkscreen on the board (as the demo code may
*        be ported to other boards).
*         i.e. - LED_Off(LED_CONNECTION_DETECTED);
*
* Output: none
*
********************************************************************/
void LED_Off(LED led)
{
    LED_Clear(led);
}


/*******************************************************************************
 End of File
*/
