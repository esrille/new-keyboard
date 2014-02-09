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

#include <stdbool.h>
#include <print_ram_buffer.h>
#include <stdint.h>
#include <string.h>

/* User build options *********************************************/
#ifndef PRINT_BUFFER_SIZE
    #define PRINT_BUFFER_SIZE 32
#endif

/* Variables ******************************************************/
static char printBuffer[PRINT_BUFFER_SIZE];
static char* current;

/*********************************************************************
* Function: bool PRINT_Initialize()
*
* Overview: Initializes the print operation
*
* PreCondition: none
*
* Input: None
*
* Output: true if successful, false if not configured or doesn?t
*         exist for this board.
*
********************************************************************/
bool PRINT_Initialize()
{
    PRINT_ClearScreen();

    return true;
}

/*********************************************************************
* Function: void PRINT_String(char* string, uint16_t length)
*
* Overview: Prints a string until a null terminator is reached or the
*           specified string length is printed.
*
* PreCondition: none
*
* Input: char* string - the string to print.
*        uint16_t length - the length of the string.
*
* Output: None
*
********************************************************************/
void PRINT_String(char* string, uint16_t length)
{
    while(*string != 0)
    {
        if(length == 0)
        {
            return;
        }

        PRINT_Char(*string++);
        length--;
    }
}

/*********************************************************************
* Function: void PRINT_Char(char charToPrint)
*
* Overview: Prints a character
*
* PreCondition: none
*
* Input: char charToPrint - the character to print
*
* Output: None
*
********************************************************************/
void PRINT_Char(char charToPrint)
{
    *current = charToPrint;
    
    if(current == (&printBuffer[PRINT_BUFFER_SIZE - 1]))
    {
        current = printBuffer;
    }
    else
    {
        current++;
    }
}

/*********************************************************************
* Function: void PRINT_ClearStreen()
*
* Overview: Clears the screen, if possible
*
* PreCondition: none
*
* Input: None
*
* Output: None
*
********************************************************************/
void PRINT_ClearScreen(void)
{
    memset(printBuffer, 0, sizeof(printBuffer));
    current = printBuffer;
}


