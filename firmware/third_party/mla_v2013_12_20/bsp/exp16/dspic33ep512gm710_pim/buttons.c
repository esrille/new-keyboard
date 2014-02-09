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

#warning "Overriding compiler v1.11"
/*** Button Definitions *********************************************/
//      S1  is MCLR button
#define S3_PORT  PORTDbits.RD6
#define S6_PORT  PORTCbits.RC9
#define S5_PORT  PORTFbits.RF7       //Overlaps with D10
#define S4_PORT  PORTDbits.RD13

#define S3_TRIS  TRISDbits.TRISD6
#define S6_TRIS  TRISCbits.TRISC9
#define S5_TRIS  TRISFbits.TRISF7
#define S4_TRIS  TRISDbits.TRISD13

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
 * Output: TRUE if pressed; FALSE if not pressed.
 *
 ********************************************************************/
bool BUTTON_IsPressed ( BUTTON button )
{
    switch (button)
    {
        case BUTTON_S3:
            return ( ( S3_PORT == BUTTON_PRESSED ) ? true : false ) ;
            break ;
        case BUTTON_S6:
            return ( ( S6_PORT == BUTTON_PRESSED ) ? true : false ) ;
            break ;
        case BUTTON_S5:
            __asm__ ( "BTST 0x0E52, #0x07" ) ;
            if (SRbits.Z == 1)
                return true ;
            else if (SRbits.Z == 0)
                return false ;
            break ;
        case BUTTON_S4:
            return ( ( S4_PORT == BUTTON_PRESSED ) ? true : false ) ;
            break ;
        case BUTTON_DISABLED:
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
        case BUTTON_S3:
            S3_TRIS = PIN_INPUT ;

            break ;

        case BUTTON_S6:
            S6_TRIS = PIN_INPUT ;

            break ;

        case BUTTON_S5:
            __asm__ ( "BSET 0x0E50, #0x07" ) ;
            break ;

        case BUTTON_S4:
            S4_TRIS = PIN_INPUT ;
            break ;

        case BUTTON_DISABLED:
            break ;
    }
}
