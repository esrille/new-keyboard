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

#include <xc.h>
#include <lcd.h>
#include <stdint.h>

/* Private Definitions ***********************************************/
// Define a fast instruction execution time in terms of loop time
// typically > 43us
#define LCD_F_INSTR         1000

// Define a slow instruction execution time in terms of loop time
// typically > 1.35ms
#define LCD_S_INSTR         3000

// Define the startup time for the LCD in terms of loop time
// typically > 30ms
#define LCD_STARTUP         20000

#define LCD_MAX_COLUMN      16

#define CS1_BASE_ADDRESS    0x00020000ul
#define CS2_BASE_ADDRESS    0x000A0000ul

#define LCD_SendData(data) { ADDR1 = data; LCD_Wait(LCD_F_INSTR); }
#define LCD_SendCommand(command, delay) { ADDR0 = command; LCD_Wait(delay); }
#define LCD_COMMAND_CLEAR_SCREEN        0x01
#define LCD_COMMAND_RETURN_HOME         0x02
#define LCD_COMMAND_ENTER_DATA_MODE     0x06
#define LCD_COMMAND_CURSOR_OFF          0x0C
#define LCD_COMMAND_CURSOR_ON           0x0F
#define LCD_COMMAND_MOVE_CURSOR_LEFT    0x10
#define LCD_COMMAND_MOVE_CURSOR_RIGHT   0x14
#define LCD_COMMAND_SET_MODE_8_BIT      0x38
#define LCD_COMMAND_ROW_0_HOME          0x80
#define LCD_COMMAND_ROW_1_HOME          0xC0

/* Private Functions *************************************************/
static void LCD_CarriageReturn ( void ) ;
static void LCD_ShiftCursorLeft ( void ) ;
static void LCD_ShiftCursorRight ( void ) ;
static void LCD_ShiftCursorUp ( void ) ;
static void LCD_ShiftCursorDown ( void ) ;
static void LCD_Wait ( unsigned int ) ;

/* Private variables ************************************************/
static uint8_t row ;
static uint8_t column ;
static __eds__ unsigned int __attribute__ ( ( noload , section ( "epmp_cs1" ) , address ( CS1_BASE_ADDRESS ) ) ) ADDR0 __attribute__ ( ( space ( eds ) ) ) ;
static __eds__ unsigned int __attribute__ ( ( noload , section ( "epmp_cs1" ) , address ( CS1_BASE_ADDRESS ) ) ) ADDR1 __attribute__ ( ( space ( eds ) ) ) ;
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
 * Output: true if initialized, false otherwise
 *
 ********************************************************************/
bool LCD_Initialize ( void )
{
    PMCON1bits.ADRMUX = 0 ;      // address is not multiplexed
    PMCON1bits.MODE = 3 ;        // master mode
    PMCON1bits.CSF = 0 ;         // PMCS1 pin used for chip select 1,
    // PMCS2 pin used for chip select 2
    PMCON1bits.ALP = 0 ;         //
    PMCON1bits.ALMODE = 0 ;      // "smart" address strobes are not used
    PMCON1bits.BUSKEEP = 0 ;     // bus keeper is not used

    // CS1 start address
    PMCS1BS = ( CS1_BASE_ADDRESS >> 8 ) ;

    PMCS1CFbits.CSDIS = 0 ;      // enable CS
    PMCS1CFbits.CSP = 1 ;        // CS1 polarity
    PMCS1CFbits.BEP = 1 ;        // byte enable polarity
    PMCS1CFbits.WRSP = 1 ;       // write strobe polarity
    PMCS1CFbits.RDSP = 1 ;        // read strobe polarity
    PMCS1CFbits.CSPTEN = 1 ;     // enable CS port
    PMCS1CFbits.SM = 0 ;         // read and write strobes on separate lines
    PMCS1CFbits.PTSZ = 2 ;       // data bus width is 16-bit
    PMCS1MDbits.ACKM = 0 ;       // PMACK is not used

    // The device timing parameters. Set the proper timing
    // according to the device used (the timing macros are defined in the hardware profile)
    PMCS1MDbits.DWAITB = 0b11 ;     // access time 3 1/4 Tcy
    PMCS1MDbits.DWAITM = 0b1111 ;
    PMCS1MDbits.DWAITE = 0b11 ;

    PMCON2bits.RADDR = 0 ;       // don't care since CS2 is not be used
    PMCON4 = 0x0001 ;            // PMA0 - PMA15 address lines are enabled

    PMCON3bits.PTWREN = 1 ;      // enable write strobe port
    PMCON3bits.PTRDEN = 1 ;      // enable read strobe port
    PMCON3bits.PTBE0EN = 1 ;     // enable byte enable port
    PMCON3bits.PTBE1EN = 1 ;     // enable byte enable port
    PMCON3bits.AWAITM = 0b11 ;      // set address latch pulses width to 3 1/2 Tcy
    PMCON3bits.AWAITE = 1 ;      // set address hold time to 1 1/4 Tcy

    PMCON1bits.PMPEN = 1 ;       // enable the module

    LCD_Wait ( LCD_STARTUP ) ;
    LCD_Wait ( LCD_STARTUP ) ;

    LCD_SendCommand ( LCD_COMMAND_SET_MODE_8_BIT ,     LCD_F_INSTR + LCD_STARTUP ) ;
    LCD_SendCommand ( LCD_COMMAND_CURSOR_OFF ,         LCD_F_INSTR ) ;
    LCD_SendCommand ( LCD_COMMAND_ENTER_DATA_MODE ,    LCD_S_INSTR ) ;

    LCD_ClearScreen ( ) ;

    return true ;
}
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
void LCD_PutString ( char* inputString , uint16_t length )
{
    while (length--)
    {
        switch (*inputString)
        {
            case 0x00:
                return ;

            default:
                LCD_PutChar ( *inputString++ ) ;
                break ;
        }
    }
}
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
void LCD_PutChar ( char inputCharacter )
{
    switch (inputCharacter)
    {
        case '\r':
            LCD_CarriageReturn ( ) ;
            break ;

        case '\n':
            if (row == 0)
            {
                LCD_ShiftCursorDown ( ) ;
            }
            else
            {
                LCD_ShiftCursorUp ( ) ;
            }
            break ;

        case '\b':
            LCD_ShiftCursorLeft ( ) ;
            LCD_PutChar ( ' ' ) ;
            LCD_ShiftCursorLeft ( ) ;
            break ;

        default:
            LCD_SendData ( inputCharacter ) ;
            column++ ;

            if (column == LCD_MAX_COLUMN)
            {
                column = 0 ;
                if (row == 0)
                {
                    LCD_SendCommand ( LCD_COMMAND_ROW_1_HOME , LCD_S_INSTR ) ;
                    row = 1 ;
                }
                else
                {
                    LCD_SendCommand ( LCD_COMMAND_ROW_0_HOME , LCD_S_INSTR ) ;
                    row = 0 ;
                }
            }
            break ;
    }
}
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
void LCD_ClearScreen ( void )
{
    LCD_SendCommand ( LCD_COMMAND_CLEAR_SCREEN , LCD_S_INSTR ) ;
    LCD_SendCommand ( LCD_COMMAND_RETURN_HOME ,  LCD_S_INSTR ) ;

    row = 0 ;
    column = 0 ;
}


/*******************************************************************/
/*******************************************************************/
/* Private Functions ***********************************************/
/*******************************************************************/
/*******************************************************************/
/*********************************************************************
 * Function: static void LCD_CarriageReturn(void)
 *
 * Overview: Handles a carriage return
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_CarriageReturn ( void )
{
    if (row == 0)
    {
        LCD_SendCommand ( LCD_COMMAND_ROW_0_HOME , LCD_S_INSTR ) ;
    }
    else
    {
        LCD_SendCommand ( LCD_COMMAND_ROW_1_HOME , LCD_S_INSTR ) ;
    }
    column = 0 ;
}
/*********************************************************************
 * Function: static void LCD_ShiftCursorLeft(void)
 *
 * Overview: Shifts cursor left one spot (wrapping if required)
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_ShiftCursorLeft ( void )
{
    uint8_t i ;

    if (column == 0)
    {
        if (row == 0)
        {
            LCD_SendCommand ( LCD_COMMAND_ROW_1_HOME , LCD_S_INSTR ) ;
            row = 1 ;
        }
        else
        {
            LCD_SendCommand ( LCD_COMMAND_ROW_0_HOME , LCD_S_INSTR ) ;
            row = 0 ;
        }

        //Now shift to the end of the row
        for (i = 0 ; i < ( LCD_MAX_COLUMN - 1 ) ; i++)
        {
            LCD_ShiftCursorRight ( ) ;
        }
    }
    else
    {
        column-- ;
        LCD_SendCommand ( LCD_COMMAND_MOVE_CURSOR_LEFT , LCD_F_INSTR ) ;
    }
}
/*********************************************************************
 * Function: static void LCD_ShiftCursorRight(void)
 *
 * Overview: Shifts cursor right one spot (wrapping if required)
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_ShiftCursorRight ( void )
{
    LCD_SendCommand ( LCD_COMMAND_MOVE_CURSOR_RIGHT , LCD_F_INSTR ) ;
    column++ ;

    if (column == LCD_MAX_COLUMN)
    {
        column = 0 ;
        if (row == 0)
        {
            LCD_SendCommand ( LCD_COMMAND_ROW_1_HOME , LCD_S_INSTR ) ;
            row = 1 ;
        }
        else
        {
            LCD_SendCommand ( LCD_COMMAND_ROW_0_HOME , LCD_S_INSTR ) ;
            row = 0 ;
        }
    }
}
/*********************************************************************
 * Function: static void LCD_ShiftCursorUp(void)
 *
 * Overview: Shifts cursor up one spot (wrapping if required)
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_ShiftCursorUp ( void )
{
    uint8_t i ;

    for (i = 0 ; i < LCD_MAX_COLUMN ; i++)
    {
        LCD_ShiftCursorLeft ( ) ;
    }
}
/*********************************************************************
 * Function: static void LCD_ShiftCursorDown(void)
 *
 * Overview: Shifts cursor down one spot (wrapping if required)
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_ShiftCursorDown ( void )
{
    uint8_t i ;

    for (i = 0 ; i < LCD_MAX_COLUMN ; i++)
    {
        LCD_ShiftCursorRight ( ) ;
    }
}
/*********************************************************************
 * Function: static void LCD_Wait(unsigned int B)
 *
 * Overview: A crude wait function that just cycle burns
 *
 * PreCondition: None
 *
 * Input: unsigned int - artibrary delay time based on loop counts.
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_Wait ( unsigned int delay )
{
    while (delay)
    {
        delay-- ;
    }
}
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
void LCD_CursorEnable ( bool enable )
{
    if (enable == true)
    {
        LCD_SendCommand ( LCD_COMMAND_CURSOR_ON , LCD_S_INSTR ) ;
    }
    else
    {
        LCD_SendCommand ( LCD_COMMAND_CURSOR_OFF , LCD_S_INSTR ) ;
    }
}

