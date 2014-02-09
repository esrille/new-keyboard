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

#include <p24FJ256DA210.h>
#include <stdbool.h>
#include <buttons.h>

/*** Button Definitions *********************************************/
//      S1  is MCLR button
#define S1_PORT  PORTGbits.RG8
#define S2_PORT  PORTEbits.RE9
#define S3_PORT  PORTBbits.RB5
//      S4  //MCLR


#define S1_TRIS  TRISGbits.TRISG8
#define S2_TRIS  TRISEbits.TRISE9
#define S3_TRIS  TRISBbits.TRISB5

#define S1_ANSEL  ANSGbits.ANSG8
#define S2_ANSEL  ANSEbits.ANSE9
#define S3_ANSEL  ANSBbits.ANSB5

#define BUTTON_PRESSED      0
#define BUTTON_NOT_PRESSED  1

#define PIN_INPUT           1
#define PIN_OUTPUT          0

#define PIN_DIGITAL         0
#define PIN_ANALOG          1

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
bool BUTTON_IsPressed(BUTTON button)
{
    switch(button)
    {
        case BUTTON_S1:
            return ( (S1_PORT == BUTTON_PRESSED) ? true : false);

        case BUTTON_S2:
            return ( (S2_PORT == BUTTON_PRESSED) ? true : false);

        case BUTTON_S3:
            return ( (S3_PORT == BUTTON_PRESSED) ? true : false);

        case BUTTON_NONE:
            return false;
    }
    
    return false;
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
void BUTTON_Enable(BUTTON button)
{
    switch(button)
    {
        case BUTTON_S1:
            S1_TRIS = PIN_INPUT;
            S1_ANSEL = PIN_DIGITAL;
            break;

        case BUTTON_S2:
            S2_TRIS = PIN_INPUT;
            S2_ANSEL = PIN_DIGITAL;
            break;

        case BUTTON_S3:
            S3_TRIS = PIN_INPUT;
            S3_ANSEL = PIN_DIGITAL;
            break;

        case BUTTON_NONE:
            break;
    }
}
