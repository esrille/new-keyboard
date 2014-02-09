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

#include <leds_3.h>
#include <oled.h>
#include <soft_start.h>
#include <stdbool.h>
#include <xc.h>

/* Local variables */
static bool LED1 = false;
static bool LED2 = false;
static bool LED3 = false;

static bool configured = false;

#define LED_CHARACTER_COUNT     4
#define OLED_CHARACTER_WIDTH    8
#define LED_SPACER              3
#define NUM_OLED_ROWS_PER_LED   5

#define LED1_COLUMN (((LED_CHARACTER_COUNT * OLED_CHARACTER_WIDTH) + LED_SPACER) * 0)
#define LED2_COLUMN (((LED_CHARACTER_COUNT * OLED_CHARACTER_WIDTH) + LED_SPACER) * 1)
#define LED3_COLUMN (((LED_CHARACTER_COUNT * OLED_CHARACTER_WIDTH) + LED_SPACER) * 2)

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
    uint8_t led_column;
    uint8_t rows;

    switch(led)
    {
        case LED_OLED1:
            LED1 = true;
            led_column = LED1_COLUMN;
            break;
        case LED_OLED2:
            LED2 = true;
            led_column = LED2_COLUMN;
            break;
        case LED_OLED3:
            LED3 = true;
            led_column = LED3_COLUMN;
            break;
			
		case LED_NONE:
            return;
			
        default:
            return;
    }

    //This pumps the soft start state machine in case it isn't already powered
    //  It also prevents the user fconst trying to write to the OLED before it is
    //  ready to use.
    if(configured == false)
    {
        return;
    }
    
    //Print a solid block for the LED being on
    for(rows = 0; rows < NUM_OLED_ROWS_PER_LED; rows++)
    {
        oledPutROMString((const unsigned char *)"\x85\x85\x85\x85", rows, led_column);
    }
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
    uint8_t led_column;
    uint8_t rows;

    switch(led)
    {
        case LED_OLED1:
            LED1 = false;
            led_column = LED1_COLUMN;
            break;
        case LED_OLED2:
            LED2 = false;
            led_column = LED2_COLUMN;
            break;
        case LED_OLED3:
            LED3 = false;
            led_column = LED3_COLUMN;
            break;
			
		case LED_NONE:
            return;
			
        default:
            return;
    }

    if(configured == false)
    {
        return;
    }

    //Print the top line of the empty box
    oledPutROMString((const unsigned char *)"\x8A\x89\x89\x8C", 0, led_column);

    //Print the middle of the box for the specified number of rows
    for(rows = 1; rows < (NUM_OLED_ROWS_PER_LED - 1); rows++)
    {
        oledPutROMString((const unsigned char *)"\x86\x84\x84\x87", rows, led_column);
    }

    //Print the bottom of the empty box
    oledPutROMString((const unsigned char *)"\x8B\x88\x88\x8D", rows, led_column);
}

/*********************************************************************
* Function: void LED_Toggle(LED led);
*
* Overview: Toggles the state of the requested LED
*
* PreCondition: LED configured via LEDConfigure()
*
* Input: LED led - enumeration of the LEDs available in this
*        demo.  They should be meaningful names and not the names of
*        the LEDs on the silkscreen on the board (as the demo code may
*        be ported to other boards).
*         i.e. - LED_Toggle(LED_CONNECTION_DETECTED);
*
* Output: none
*
********************************************************************/
void LED_Toggle(LED led)
{
    switch(led)
    {
        case LED_OLED1:
            if(LED1 == TRUE)
            {
                LED_Off(LED_OLED1);
            }
            else
            {
                LED_On(LED_OLED1);
            }
            break;
        case LED_OLED2:
            if(LED2 == TRUE)
            {
                LED_Off(LED_OLED2);
            }
            else
            {
                LED_On(LED_OLED2);
            }
            break;
        case LED_OLED3:
            if(LED3 == TRUE)
            {
                LED_Off(LED_OLED3);
            }
            else
            {
                LED_On(LED_OLED3);
            }
            break;
	    			
		case LED_NONE:
            return;
			
        default:
            return;
    }
}

/*********************************************************************
* Function: bool LED_Get(LED led);
*
* Overview: Returns the current state of the requested LED
*
* PreCondition: LED configured via LEDConfigure()
*
* Input: LED led - enumeration of the LEDs available in this
*        demo.  They should be meaningful names and not the names of
*        the LEDs on the silkscreen on the board (as the demo code may
*        be ported to other boards).
*         i.e. - LED_Get(LED_CONNECTION_DETECTED);
*
* Output: true if on, false if off
*
********************************************************************/
bool LED_Get(LED led)
{
    switch(led)
    {
        case LED_OLED1:
            return LED1;

        case LED_OLED2:
            return LED2;
			
        case LED_OLED3:
            return LED3;
	    			
		case LED_NONE:
            return false;
    }

    return false;
}

/*********************************************************************
* Function: void LED_Enable(LED led);
*
* Overview: Configures the LED for use by the other LED API
*
* PreCondition: none
*
* Input: LED led - enumeration of the LEDs available in this
*        demo.  They should be meaningful names and not the names of
*        the LEDs on the silkscreen on the board (as the demo code may
*        be ported to other boards).
*
* Output: none
*
********************************************************************/
void LED_Enable(LED led)
{
    if (AppPowerReady() == true)
    {
        if(configured == false)
        {
            ResetDevice();
            FillDisplay(0x00);

            //We need to make sure to set the current configuration before
            //  we do the LEDSet() calls, otherwise the LEDSet will not
            //  preform the operation because the current mode doesn't
            //  support the operation.
            configured = true;

            if(LED1 == true)
            {
                LED_On(LED_OLED1);
            }
            else
            {
                LED_Off(LED_OLED1);
            }

            if(LED2 == true)
            {
                LED_On(LED_OLED2);
            }
            else
            {
                LED_Off(LED_OLED2);
            }

            if(LED3 == true)
            {
                LED_On(LED_OLED3);
            }
            else
            {
                LED_Off(LED_OLED3);
            }
        }
    }
    else
    {
        configured = false;
    }
}



