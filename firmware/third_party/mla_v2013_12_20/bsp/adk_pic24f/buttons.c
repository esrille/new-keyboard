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

#include <xc.h>
#include <stdbool.h>
#include <buttons.h>

/*** Button Definitions *********************************************/
#define BUTTON_BTN2_PORT  PORTFbits.RF13
#define BUTTON_BTN3_PORT  PORTFbits.RF12
#define BUTTON_BTN4_PORT  PORTBbits.RB11
#define BUTTON_BTN5_PORT  PORTAbits.RA1

#define BUTTON_BTN2_TRIS  TRISFbits.TRISF13
#define BUTTON_BTN3_TRIS  TRISFbits.TRISF12
#define BUTTON_BTN4_TRIS  TRISBbits.TRISB11
#define BUTTON_BTN5_TRIS  TRISAbits.TRISA1

#define BUTTON_PRESSED      0
#define BUTTON_NOT_PRESSED  1

#define PIN_INPUT           1
#define PIN_OUTPUT          0
/*********************************************************************
 * Function: bool BUTTON_IsPressed(BUTTON button);
 *
 * Overview: Returns the current state of the requested button
 *
 * PreCondition: button configured via BUTTON_SetConfiguration()
 *
 * Input: BUTTON button - enumeration of the buttons available in
 *        this demo.  They should be meaningful names and not the names
 *        of the buttons on the silkscreen on the board (as the demo
 *        code may be ported to other boards).
 *         i.e. - ButtonIsPressed(BUTTON_SEND_MESSAGE);
 *
 * Output: true if pressed; false if not pressed.
 *
 ********************************************************************/
bool BUTTON_IsPressed ( BUTTON button )
{
    switch (button)
    {
        case BUTTON_BTN2:
            return ( ( BUTTON_BTN2_PORT == BUTTON_PRESSED ) ? true : false ) ;

        case BUTTON_BTN3:
            return ( ( BUTTON_BTN3_PORT == BUTTON_PRESSED ) ? true : false ) ;

        case BUTTON_BTN4:
            return ( ( BUTTON_BTN4_PORT == BUTTON_PRESSED ) ? true : false ) ;

        case BUTTON_BTN5:
            return ( ( BUTTON_BTN5_PORT == BUTTON_PRESSED ) ? true : false ) ;

        case BUTTON_NONE:
            return false ;
    }

    return false ;
}
/*********************************************************************
 * Function: void BUTTON_Enable(BUTTON button);
 *
 * Overview: Returns the current state of the requested button
 *
 * PreCondition: button configured via BUTTON_SetConfiguration()
 *
 * Input: BUTTON button - enumeration of the buttons available in
 *        this demo.  They should be meaningful names and not the names
 *        of the buttons on the silkscreen on the board (as the demo
 *        code may be ported to other boards).
 *         i.e. - ButtonIsPressed(BUTTON_SEND_MESSAGE);
 *
 * Output: None
 *
 ********************************************************************/
void BUTTON_Enable ( BUTTON button )
{
    switch (button)
    {
        case BUTTON_BTN2:
            BUTTON_BTN2_TRIS = PIN_INPUT ;
            break ;

        case BUTTON_BTN3:
            BUTTON_BTN3_TRIS = PIN_INPUT ;
            break ;

        case BUTTON_BTN4:
            AD1PCFGLbits.PCFG11 = 1;
            BUTTON_BTN4_TRIS = PIN_INPUT ;
            break ;

        case BUTTON_BTN5:
            BUTTON_BTN5_TRIS = PIN_INPUT ;
            break ;

        case BUTTON_NONE:
            break ;
    }
}
