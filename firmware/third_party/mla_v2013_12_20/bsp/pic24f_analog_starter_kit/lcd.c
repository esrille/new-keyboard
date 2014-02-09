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
#include <stdbool.h>
#include <string.h>
#include <timer_1ms.h>

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

#define LCD_CURSOR_BLINK_COUNT      75

typedef struct
{
    uint8_t c[5]; ///< Column Data
    uint8_t w;    ///< Width
} CHARACTER;

typedef struct
{
    char data[6];

    struct
    {
        uint8_t position;
        bool enabled;
        uint8_t blinkCount;
        bool visible;
    } cursor;

    bool touched;
} LCD;

typedef union __attribute__((packed))
{
    uint8_t byte;
    struct __attribute__((packed))
    {
        uint8_t bit0 :1;
        uint8_t bit1 :1;
        uint8_t bit2 :1;
        uint8_t bit3 :1;
        uint8_t bit4 :1;
        uint8_t bit5 :1;
        uint8_t bit6 :1;
        uint8_t bit7 :1;
    } bits;
} BIT_ACCESS;

#define	X0Y0  _S8C0
#define	X0Y1  _S8C1
#define	X0Y2  _S8C2
#define	X0Y3  _S8C3
#define	X0Y4  _S8C4
#define	X0Y5  _S8C5
#define	X0Y6  _S8C6
#define Dollar	  _S8C7
              //
#define	X1Y0   _S9C0
#define	X1Y1   _S9C1
#define	X1Y2   _S9C2
#define	X1Y3   _S9C3
#define	X1Y4   _S9C4
#define	X1Y5   _S9C5
#define	X1Y6   _S9C6
#define WHEEL1	  _S9C7
              ////
#define	X2Y0 	  _S12C0
#define	X2Y1 	  _S12C1
#define	X2Y2 	  _S12C2
#define	X2Y3 	  _S12C3
#define	X2Y4 	  _S12C4
#define	X2Y5 	  _S12C5
#define	X2Y6 	  _S12C6
#define WHEEL2	  _S12C7

#define	X3Y0 	  _S13C0
#define	X3Y1 	  _S13C1
#define	X3Y2 	  _S13C2
#define	X3Y3 	  _S13C3
#define	X3Y4 	  _S13C4
#define	X3Y5 	  _S13C5
#define	X3Y6 	  _S13C6
#define WHEEL3	  _S13C7

#define	X4Y0 	  _S14C0
#define	X4Y1 	  _S14C1
#define	X4Y2 	  _S14C2
#define	X4Y3 	  _S14C3
#define	X4Y4 	  _S14C4
#define	X4Y5 	  _S14C5
#define	X4Y6 	  _S14C6
#define WHEEL4	  _S14C7

#define	X5Y0 	  _S15C0
#define	X5Y1 	  _S15C1
#define	X5Y2 	  _S15C2
#define	X5Y3 	  _S15C3
#define	X5Y4 	  _S15C4
#define	X5Y5 	  _S15C5
#define	X5Y6 	  _S15C6
#define WHEEL5	  _S15C7

#define	X6Y0 	  _S16C0
#define	X6Y1 	  _S16C1
#define	X6Y2 	  _S16C2
#define	X6Y3 	  _S16C3
#define	X6Y4 	  _S16C4
#define	X6Y5 	  _S16C5
#define	X6Y6 	  _S16C6
#define WHEEL6	  _S16C7

#define	X7Y0 	  _S25C0
#define	X7Y1 	  _S25C1
#define	X7Y2 	  _S25C2
#define	X7Y3 	  _S25C3
#define	X7Y4 	  _S25C4
#define	X7Y5 	  _S25C5
#define	X7Y6 	  _S25C6
#define WHEEL7	  _S25C7

#define	X8Y0 	  _S26C0
#define	X8Y1 	  _S26C1
#define	X8Y2 	  _S26C2
#define	X8Y3 	  _S26C3
#define	X8Y4 	  _S26C4
#define	X8Y5 	  _S26C5
#define	X8Y6 	  _S26C6
#define WHEEL8	  _S26C7

#define	X9Y0 	  _S28C0
#define	X9Y1 	  _S28C1
#define	X9Y2 	  _S28C2
#define	X9Y3 	  _S28C3
#define	X9Y4 	  _S28C4
#define	X9Y5 	  _S28C5
#define	X9Y6 	  _S28C6
#define WHEEL9	  _S28C7

#define	X10Y0 	  _S29C0
#define	X10Y1 	  _S29C1
#define	X10Y2 	  _S29C2
#define	X10Y3 	  _S29C3
#define	X10Y4 	  _S29C4
#define	X10Y5 	  _S29C5
#define	X10Y6 	  _S29C6
#define WHEEL10	  _S29C7

#define	X11Y0 	 _S31C0
#define	X11Y1 	 _S31C1
#define	X11Y2 	 _S31C2
#define	X11Y3 	 _S31C3
#define	X11Y4 	 _S31C4
#define	X11Y5 	 _S31C5
#define	X11Y6 	 _S31C6
#define SUN 	 _S31C7

#define	X12Y0 	  _S62C0
#define	X12Y1 	  _S62C1
#define	X12Y2 	  _S62C2
#define	X12Y3 	  _S62C3
#define	X12Y4 	  _S62C4
#define	X12Y5 	  _S62C5
#define	X12Y6 	  _S62C6
#define PAN 	  _S62C7

#define	X13Y0 	  _S61C0
#define	X13Y1 	  _S61C1
#define	X13Y2 	  _S61C2
#define	X13Y3 	  _S61C3
#define	X13Y4 	  _S61C4
#define	X13Y5 	  _S61C5
#define	X13Y6 	  _S61C6
#define WIFI 	  _S61C7

#define	X14Y0 	  _S60C0
#define	X14Y1 	  _S60C1
#define	X14Y2 	  _S60C2
#define	X14Y3 	  _S60C3
#define	X14Y4 	  _S60C4
#define	X14Y5 	  _S60C5
#define	X14Y6 	  _S60C6
#define ZIG 	  _S60C7

#define	X15Y0 	  _S59C0
#define	X15Y1 	  _S59C1
#define	X15Y2 	  _S59C2
#define	X15Y3 	  _S59C3
#define	X15Y4 	  _S59C4
#define	X15Y5 	  _S59C5
#define	X15Y6 	  _S59C6
#define CLOUD 	  _S59C7

#define	X16Y0 	  _S57C0
#define	X16Y1 	  _S57C1
#define	X16Y2 	  _S57C2
#define	X16Y3 	  _S57C3
#define	X16Y4 	  _S57C4
#define	X16Y5 	  _S57C5
#define	X16Y6 	  _S57C6
#define HEART 	  _S57C7

#define	X17Y0 	  _S56C0
#define	X17Y1 	  _S56C1
#define	X17Y2 	  _S56C2
#define	X17Y3 	  _S56C3
#define	X17Y4 	  _S56C4
#define	X17Y5 	  _S56C5
#define	X17Y6 	  _S56C6
#define STRIP 	  _S56C7

#define	X18Y0 	  _S55C0
#define	X18Y1 	  _S55C1
#define	X18Y2 	  _S55C2
#define	X18Y3 	  _S55C3
#define	X18Y4 	  _S55C4
#define	X18Y5 	  _S55C5
#define	X18Y6 	  _S55C6
#define BAT1 	  _S55C7

#define	X19Y0	  _S34C0
#define	X19Y1	  _S34C1
#define	X19Y2	  _S34C2
#define	X19Y3	  _S34C3
#define	X19Y4	  _S34C4
#define	X19Y5	  _S34C5
#define	X19Y6	  _S34C6
#define BAT2 	  _S34C7

#define	X20Y0	  _S37C0
#define	X20Y1	  _S37C1
#define	X20Y2	  _S37C2
#define	X20Y3	  _S37C3
#define	X20Y4	  _S37C4
#define	X20Y5	  _S37C5
#define	X20Y6	  _S37C6
#define BAT3 	  _S37C7

#define	X21Y0	  _S38C0
#define	X21Y1	  _S38C1
#define	X21Y2	  _S38C2
#define	X21Y3	  _S38C3
#define	X21Y4	  _S38C4
#define	X21Y5	  _S38C5
#define	X21Y6	  _S38C6
#define BAT4 	  _S38C7

#define	X22Y0	  _S54C0
#define	X22Y1	  _S54C1
#define	X22Y2	  _S54C2
#define	X22Y3	  _S54C3
#define	X22Y4	  _S54C4
#define	X22Y5	  _S54C5
#define	X22Y6	  _S54C6
#define DROP 	  _S54C7

#define	X23Y0	  _S53C0
#define	X23Y1	  _S53C1
#define	X23Y2	  _S53C2
#define	X23Y3	  _S53C3
#define	X23Y4	  _S53C4
#define	X23Y5	  _S53C5
#define	X23Y6	  _S53C6
#define THERMO 	  _S53C7

#define	X24Y0	  _S52C0
#define	X24Y1	  _S52C1
#define	X24Y2	  _S52C2
#define	X24Y3	  _S52C3
#define	X24Y4	  _S52C4
#define	X24Y5	  _S52C5
#define	X24Y6	  _S52C6
#define CEL 	  _S52C7

#define	X25Y0	  _S51C0
#define	X25Y1	  _S51C1
#define	X25Y2	  _S51C2
#define	X25Y3	  _S51C3
#define	X25Y4	  _S51C4
#define	X25Y5	  _S51C5
#define	X25Y6	  _S51C6
#define FAREN 	  _S51C7

#define	X26Y0	  _S48C0
#define	X26Y1	  _S48C1
#define	X26Y2	  _S48C2
#define	X26Y3	  _S48C3
#define	X26Y4	  _S48C4
#define	X26Y5	  _S48C5
#define	X26Y6	  _S48C6
#define MICROCHIP _S48C7

#define	X27Y0	  _S49C0
#define	X27Y1	  _S49C1
#define	X27Y2	  _S49C2
#define	X27Y3	  _S49C3
#define	X27Y4	  _S49C4
#define	X27Y5	  _S49C5
#define	X27Y6	  _S49C6
#define CENT      _S49C7

#define	X28Y0	  _S46C0
#define	X28Y1	  _S46C1
#define	X28Y2	  _S46C2
#define	X28Y3	  _S46C3
#define	X28Y4	  _S46C4
#define	X28Y5	  _S46C5
#define	X28Y6	  _S46C6
#define RMB       _S46C7

#define	X29Y0	  _S39C0
#define	X29Y1	  _S39C1
#define	X29Y2	  _S39C2
#define	X29Y3	  _S39C3
#define	X29Y4	  _S39C4
#define	X29Y5	  _S39C5
#define	X29Y6	  _S39C6
#define	X36Y0     _S39C7

#define	X30Y0	  _S40C0
#define	X30Y1	  _S40C1
#define	X30Y2	  _S40C2
#define	X30Y3	  _S40C3
#define	X30Y4	  _S40C4
#define	X30Y5	  _S40C5
#define	X30Y6	  _S40C6
#define	X36Y1     _S40C7

#define	X31Y0	  _S41C0
#define	X31Y1	  _S41C1
#define	X31Y2	  _S41C2
#define	X31Y3	  _S41C3
#define	X31Y4	  _S41C4
#define	X31Y5	  _S41C5
#define	X31Y6	  _S41C6
#define	X36Y2     _S41C7

#define	X32Y0	  _S42C0
#define	X32Y1	  _S42C1
#define	X32Y2	  _S42C2
#define	X32Y3	  _S42C3
#define	X32Y4	  _S42C4
#define	X32Y5	  _S42C5
#define	X32Y6	  _S42C6
#define	X36Y3     _S42C7

#define	X33Y0	  _S43C0
#define	X33Y1	  _S43C1
#define	X33Y2	  _S43C2
#define	X33Y3	  _S43C3
#define	X33Y4	  _S43C4
#define	X33Y5	  _S43C5
#define	X33Y6	  _S43C6
#define	X36Y4     _S43C7

#define	X34Y0	  _S50C0
#define	X34Y1	  _S50C1
#define	X34Y2	  _S50C2
#define	X34Y3	  _S50C3
#define	X34Y4	  _S50C4
#define	X34Y5	  _S50C5
#define	X34Y6	  _S50C6
#define	X36Y5     _S50C7

#define	X35Y0	  _S45C0
#define	X35Y1	  _S45C1
#define	X35Y2	  _S45C2
#define	X35Y3	  _S45C3
#define	X35Y4	  _S45C4
#define	X35Y5	  _S45C5
#define	X35Y6	  _S45C6
#define	X36Y6     _S45C7

/* Private Functions *************************************************/
static void LCD_ShiftCursorLeft ( void ) ;
static void LCD_Update( void );

/* Private variables ************************************************/
static const CHARACTER charMap[]= {
    {{0x00, 0x00, 0x00, 0x00, 0x00}, .w= 0x03},      /*' '  0x20 */
    {{0x5f, 0x00, 0x00, 0x00, 0x00}, .w= 0x01},      /* !   0x21 */
    {{0x07, 0x00, 0x07, 0x00, 0x00}, .w= 0x03},      /* "   0x22 */
    {{0x14, 0x7f, 0x14, 0x7f, 0x14}, .w= 0x05},      /* #   0x23 */
    {{0x24, 0x2a, 0x7f, 0x2a, 0x12}, .w= 0x05},      /* $   0x24 */
    {{0x23, 0x13, 0x08, 0x64, 0x62}, .w= 0x05},      /* %   0x25 */
    {{0x36, 0x49, 0x55, 0x22, 0x50}, .w= 0x05},      /* &   0x26 */
    {{0x03, 0x00, 0x00, 0x00, 0x00}, .w= 0x01},      /* '   0x27 */
    {{0x1c, 0x22, 0x41, 0x00, 0x00}, .w= 0x03},      /* (   0x28 */
    {{0x41, 0x22, 0x1c, 0x00, 0x00}, .w= 0x03},      /* )   0x29 */
    {{0x14, 0x08, 0x3e, 0x08, 0x14}, .w= 0x05},      /* *   0x2a */
    {{0x08, 0x08, 0x3e, 0x08, 0x08}, .w= 0x05},      /* +   0x2b */
    {{0x50, 0x30, 0x00, 0x00, 0x00}, .w= 0x02},      /* ,   0x2c */
    {{0x08, 0x08, 0x08, 0x08, 0x08}, .w= 0x03},      /* -   0x2d */
    {{0x40, 0x00, 0x00, 0x00, 0x00}, .w= 0x01},      /* .   0x2e */
    {{0x20, 0x10, 0x08, 0x04, 0x02}, .w= 0x05},      /* /   0x2f */
    {{0x3e, 0x51, 0x49, 0x45, 0x3e}, .w= 0x05},      /* 0   0x30 */
    {{0x00, 0x42, 0x7f, 0x40, 0x00}, .w= 0x05},      /* 1   0x31 */
    {{0x42, 0x61, 0x51, 0x49, 0x46}, .w= 0x05},      /* 2   0x32 */
    {{0x21, 0x41, 0x45, 0x4b, 0x31}, .w= 0x05},      /* 3   0x33 */
    {{0x18, 0x14, 0x12, 0x7f, 0x10}, .w= 0x05},      /* 4   0x34 */
    {{0x27, 0x45, 0x45, 0x45, 0x39}, .w= 0x05},      /* 5   0x35 */
    {{0x3c, 0x4a, 0x49, 0x49, 0x30}, .w= 0x05},      /* 6   0x36 */
    {{0x03, 0x01, 0x71, 0x09, 0x07}, .w= 0x05},      /* 7   0x37 */
    {{0x36, 0x49, 0x49, 0x49, 0x36}, .w= 0x05},      /* 8   0x38 */
    {{0x06, 0x49, 0x49, 0x29, 0x1e}, .w= 0x05},      /* 9   0x39 */
    {{0x36, 0x36, 0x00, 0x00, 0x00}, .w= 0x01},      /* :   0x3a */
    {{0x56, 0x36, 0x00, 0x00, 0x00}, .w= 0x02},      /* ;   0x3b */
    {{0x08, 0x14, 0x22, 0x41, 0x00}, .w= 0x04},      /* <   0x3c */
    {{0x14, 0x14, 0x14, 0x14, 0x14}, .w= 0x05},      /* =   0x3d */
    {{0x41, 0x22, 0x14, 0x08, 0x00}, .w= 0x04},      /* >   0x3e */
    {{0x02, 0x01, 0x51, 0x09, 0x06}, .w= 0x05},      /* ?   0x3f */
    {{0x32, 0x49, 0x79, 0x41, 0x3e}, .w= 0x05},      /* @   0x40 */
    {{0x7e, 0x11, 0x11, 0x11, 0x7e}, .w= 0x05},      /* A   0x41 */
    {{0x7f, 0x49, 0x49, 0x49, 0x36}, .w= 0x05},      /* B   0x42 */
    {{0x3e, 0x41, 0x41, 0x41, 0x22}, .w= 0x05},      /* C   0x43 */
    {{0x7f, 0x41, 0x41, 0x22, 0x1c}, .w= 0x05},      /* D   0x44 */
    {{0x7f, 0x49, 0x49, 0x49, 0x41}, .w= 0x05},      /* E   0x45 */
    {{0x7f, 0x09, 0x09, 0x09, 0x01}, .w= 0x05},      /* F   0x46 */
    {{0x3e, 0x41, 0x49, 0x49, 0x7a}, .w= 0x05},      /* G   0x47 */
    {{0x7f, 0x08, 0x08, 0x08, 0x7f}, .w= 0x05},      /* H   0x48 */
    {{0x41, 0x7f, 0x41, 0x00, 0x00}, .w= 0x03},      /* I   0x49 */
    {{0x20, 0x40, 0x41, 0x3f, 0x01}, .w= 0x05},      /* J   0x4a */
    {{0x7f, 0x08, 0x14, 0x22, 0x41}, .w= 0x05},      /* K   0x4b */
    {{0x7f, 0x40, 0x40, 0x40, 0x40}, .w= 0x03},      /* L   0x4c */
    {{0x7f, 0x02, 0x0c, 0x02, 0x7f}, .w= 0x05},      /* M   0x4d */
    {{0x7f, 0x04, 0x08, 0x10, 0x7f}, .w= 0x05},      /* N   0x4e */
    {{0x3e, 0x41, 0x41, 0x41, 0x3e}, .w= 0x05},      /* O   0x4f */
    {{0x7f, 0x09, 0x09, 0x09, 0x06}, .w= 0x05},      /* P   0x50 */
    {{0x3e, 0x41, 0x51, 0x21, 0x5e}, .w= 0x05},      /* Q   0x51 */
    {{0x7f, 0x09, 0x19, 0x29, 0x46}, .w= 0x05},      /* R   0x52 */
    {{0x46, 0x49, 0x49, 0x49, 0x31}, .w= 0x05},      /* S   0x53 */
    {{0x01, 0x01, 0x7f, 0x01, 0x01}, .w= 0x05},      /* T   0x54 */
    {{0x3f, 0x40, 0x40, 0x40, 0x3f}, .w= 0x05},      /* U   0x55 */
    {{0x1f, 0x20, 0x40, 0x20, 0x1f}, .w= 0x05},      /* V   0x56 */
    {{0x3f, 0x40, 0x30, 0x40, 0x3f}, .w= 0x05},      /* W   0x57 */
    {{0x63, 0x14, 0x08, 0x14, 0x63}, .w= 0x05},      /* X   0x58 */
    {{0x07, 0x08, 0x70, 0x08, 0x07}, .w= 0x05},      /* Y   0x59 */
    {{0x61, 0x51, 0x49, 0x45, 0x43}, .w= 0x05},      /* Z   0x5a */
    {{0x7f, 0x41, 0x41, 0x00, 0x00}, .w= 0x03},      /* [   0x5b */
    {{0x15, 0x16, 0x7c, 0x16, 0x15}, .w= 0x05},      /* y   0x5c */
    {{0x41, 0x41, 0x7f, 0x00, 0x00}, .w= 0x03},      /* ]   0x5d */
    {{0x04, 0x02, 0x01, 0x02, 0x04}, .w= 0x05},      /* ^   0x5e */
    {{0x40, 0x40, 0x40, 0x40, 0x40}, .w= 0x05},      /* _   0x5f */
    {{0x01, 0x02, 0x04, 0x00, 0x00}, .w= 0x03},      /* `   0x60 */
    {{0x20, 0x54, 0x54, 0x54, 0x78}, .w= 0x05},      /* a   0x61 */
    {{0x7f, 0x48, 0x44, 0x44, 0x38}, .w= 0x05},      /* b   0x62 */
    {{0x38, 0x44, 0x44, 0x44, 0x20}, .w= 0x05},      /* c   0x63 */
    {{0x38, 0x44, 0x44, 0x48, 0x7f}, .w= 0x05},      /* d   0x64 */
    {{0x38, 0x54, 0x54, 0x18, 0x00}, .w= 0x04},      /* e   0x65 */
    {{0x08, 0x7e, 0x09, 0x01, 0x02}, .w= 0x05},      /* f   0x66 */
    {{0x0c, 0x52, 0x52, 0x52, 0x3e}, .w= 0x05},      /* g   0x67 */
    {{0x7f, 0x08, 0x04, 0x04, 0x78}, .w= 0x05},      /* h   0x68 */
    {{0x44, 0x7d, 0x40, 0x00, 0x00}, .w= 0x03},      /* i   0x69 */
    {{0x20, 0x40, 0x44, 0x3d, 0x00}, .w= 0x04},      /* j   0x6a */
    {{0x7f, 0x10, 0x28, 0x44, 0x00}, .w= 0x04},      /* k   0x6b */
    {{0x41, 0x7f, 0x40, 0x00, 0x00}, .w= 0x03},      /* l   0x6c */
    {{0x7c, 0x04, 0x18, 0x04, 0x78}, .w= 0x05},      /* m   0xed */
    {{0x7c, 0x08, 0x04, 0x04, 0x78}, .w= 0x05},      /* n   0x6e */
    {{0x38, 0x44, 0x44, 0x44, 0x38}, .w= 0x05},      /* o   0x6f */
    {{0x7c, 0x14, 0x14, 0x14, 0x08}, .w= 0x05},      /* p   0x70 */
    {{0x08, 0x14, 0x14, 0x18, 0x7c}, .w= 0x05},      /* q   0x71 */
    {{0x7c, 0x08, 0x04, 0x04, 0x08}, .w= 0x05},      /* r   0x72 */
    {{0x48, 0x54, 0x54, 0x54, 0x20}, .w= 0x05},      /* s   0x73 */
    {{0x04, 0x3f, 0x44, 0x20, 0x00}, .w= 0x04},      /* t   0x74 */
    {{0x3c, 0x40, 0x40, 0x20, 0x7c}, .w= 0x05},      /* u   0x75 */
    {{0x1c, 0x20, 0x40, 0x20, 0x1c}, .w= 0x05},      /* v   0x76 */
    {{0x3c, 0x40, 0x30, 0x40, 0x3c}, .w= 0x05},      /* w   0x77 */
    {{0x44, 0x28, 0x10, 0x28, 0x44}, .w= 0x05},      /* x   0x78 */
    {{0x0c, 0x50, 0x50, 0x50, 0x3c}, .w= 0x05},      /* y   0x79 */
    {{0x44, 0x64, 0x54, 0x4c, 0x44}, .w= 0x05},      /* z   0x7a */
    {{0x08, 0x36, 0x41, 0x00, 0x00}, .w= 0x03},      /* {   0x7b */
    {{0x7f, 0x00, 0x00, 0x00, 0x00}, .w= 0x01},      /* |   0x7c */
    {{0x41, 0x36, 0x08, 0x00, 0x00}, .w= 0x03},      /* }   0x7d */
    {{0x7F, 0x3e, 0x1c, 0x08, 0x00}, .w= 0x04},      /* ->  0x7e */
    {{0x08, 0x1c, 0x3e, 0x7f, 0x00}, .w= 0x04},      /* <-  0x7f */
    {{0x63, 0x55, 0x49, 0x41, 0x41}, .w= 0x05},      /*     0x80 */ // SIGMA
    {{0x70, 0x4c, 0x43, 0x4c, 0x70}, .w= 0x05},      /*     0x81 */ // Delta
    {{0x06, 0x09, 0x09, 0x06, 0x00}, .w= 0x04},      /*     0x82 */ // DEGREE
    {{0x31, 0x29, 0x25, 0x23, 0x00}, .w= 0x04},      /*     0x83 */ // HIGHZ
    {{0x62, 0x52, 0x4a, 0x46, 0x00}, .w= 0x04},      /*     0x84 */ // LOWZ
    {{0x00, 0x00, 0x00, 0x00, 0x00}, .w= 0x01},      /*     0x85 */ // space 3
    {{0x08, 0x04, 0x3c, 0x44, 0x22}, .w= 0x05},      /*     0x86 */ // Tau
    {{0x00, 0x00, 0x00, 0x00, 0x00}, .w= 0x04},      /*     0x87 */ // space 4
    {{0x00, 0x00, 0x00, 0x00, 0x00}, .w= 0x05},      /*     0x87 */ // space 5
    {{0x70, 0x4c, 0x43, 0x4c, 0x70}, .w= 0x05}};

static LCD lcd;

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
    // Initialize LCD: no charge pump, 8 common drivers
    LCDPSbits.WFT = 0; // Type A waveform
    LCDPSbits.LP = 2; // LCD Prescaller 1:3
    LCDCONbits.LMUX = 0x07; // 8 commons, 1/3 Bias
    LCDCONbits.CS = 1; // Clock is LPRC
    LCDREFbits.VLCD1PE = 0; // Enable internal bias
    LCDREFbits.VLCD2PE = 0;
    LCDREFbits.VLCD3PE = 0;
    LCDREFbits.LRLAP = 0x03; // ladder in High-Power Interval A (transition)
    LCDREFbits.LRLBP = 0x03; // ladder in High-Power Interval B (steady state, for higher contrast ratio))
    LCDREFbits.LRLAT = 0x03; // Internal LCD reference ladder is in A Power mode for 3 clocks and B Power mode for 13 clocks
    LCDREFbits.LCDIRE = 1; // Internal Reference Enable
    LCDREFbits.LCDCST = 2; // Contrast is 2/7ths of maximum
    LCDCONbits.LCDEN = 1; // enable LCD module

    LCD_ClearScreen ( ) ;

    LCDSE0 = 0b1111001100000000;
    LCDSE1 = 0b1011011000000001;
    LCDSE2 = 0b0110111111100100;
    LCDSE3 = 0b0111101111111111;

    lcd.cursor.enabled = false;

    TIMER_CancelTick(&LCD_Update);
    TIMER_RequestTick(&LCD_Update, (uint32_t)5);

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
    /* if the string is longer than we can print, and we know that we are going
     * to wrap the display anyway, reset the LCD to the start and just print
     * the first characters of the string rather than the wrapped around end
     * of the string. */
    if(length > (sizeof(lcd.data) - lcd.cursor.position) )
    {
        lcd.cursor.position = 0;
    }

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

        /* If we have wrapped around the end of the display, let's stop there
         * for this screen. */
        if(lcd.cursor.position == 0)
        {
            return;
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
    if( (inputCharacter >= ' ') &&
        (inputCharacter <= 0x86) )
    {
        /* The character is in the displayable character range.  Load the
         * character to be printed. */
        lcd.data[lcd.cursor.position++] = inputCharacter;
        lcd.touched = true;

        if(lcd.cursor.position >= sizeof(lcd.data))
        {
            lcd.cursor.position = 0;
        }
    }
    else
    {
        switch (inputCharacter)
        {
            case '\r':
                lcd.cursor.position = 0;
                break ;

            case '\n':
                break ;

            case '\b':
                LCD_ShiftCursorLeft();
                LCD_PutChar ( ' ' ) ;
                LCD_ShiftCursorLeft();
                break ;
        }
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
    uint8_t i;
    
    for(i=0; i < sizeof(lcd.data); i++)
    {
        lcd.data[i] = ' ';
    }

    lcd.cursor.position = 0;
    lcd.touched = true;
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
    lcd.cursor.enabled = enable;
    lcd.cursor.visible = true;
    lcd.cursor.blinkCount = 0;
}


/*******************************************************************/
/*******************************************************************/
/* Private Functions ***********************************************/
/*******************************************************************/
/*******************************************************************/

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
    if(lcd.cursor.position == 0)
    {
        lcd.cursor.position = sizeof(lcd.data);
    }

    lcd.cursor.position--;
}

static void LCD_Update( void )
{
    uint8_t i;
    BIT_ACCESS data[5];

    lcd.cursor.blinkCount++;

    for(i=0; i<sizeof(lcd.data); i++)
    {
        memcpy(data, &charMap[lcd.data[i] - ' '], sizeof(charMap[0].c));
        
        if( (i == lcd.cursor.position) &&
            (lcd.cursor.enabled == true) 
          )
        {
            if(lcd.cursor.blinkCount >= LCD_CURSOR_BLINK_COUNT)
            {
                lcd.cursor.blinkCount = 0;
                lcd.cursor.visible ^= 1;
                lcd.touched = true;
            }

            if(lcd.cursor.visible == true)
            {
                memset(data, 0xFF, sizeof(data));
            }
        }

        if(lcd.touched == true)
        {
            switch(i)
            {
                case 0:
                    X0Y0 = data[0].bits.bit0;
                    X0Y1 = data[0].bits.bit1;
                    X0Y2 = data[0].bits.bit2;
                    X0Y3 = data[0].bits.bit3;
                    X0Y4 = data[0].bits.bit4;
                    X0Y5 = data[0].bits.bit5;
                    X0Y6 = data[0].bits.bit6;

                    X1Y0 = data[1].bits.bit0;
                    X1Y1 = data[1].bits.bit1;
                    X1Y2 = data[1].bits.bit2;
                    X1Y3 = data[1].bits.bit3;
                    X1Y4 = data[1].bits.bit4;
                    X1Y5 = data[1].bits.bit5;
                    X1Y6 = data[1].bits.bit6;

                    X2Y0 = data[2].bits.bit0;
                    X2Y1 = data[2].bits.bit1;
                    X2Y2 = data[2].bits.bit2;
                    X2Y3 = data[2].bits.bit3;
                    X2Y4 = data[2].bits.bit4;
                    X2Y5 = data[2].bits.bit5;
                    X2Y6 = data[2].bits.bit6;

                    X3Y0 = data[3].bits.bit0;
                    X3Y1 = data[3].bits.bit1;
                    X3Y2 = data[3].bits.bit2;
                    X3Y3 = data[3].bits.bit3;
                    X3Y4 = data[3].bits.bit4;
                    X3Y5 = data[3].bits.bit5;
                    X3Y6 = data[3].bits.bit6;

                    X4Y0 = data[4].bits.bit0;
                    X4Y1 = data[4].bits.bit1;
                    X4Y2 = data[4].bits.bit2;
                    X4Y3 = data[4].bits.bit3;
                    X4Y4 = data[4].bits.bit4;
                    X4Y5 = data[4].bits.bit5;
                    X4Y6 = data[4].bits.bit6;
                    break;

                case 1:
                    X6Y0 = data[0].bits.bit0;
                    X6Y1 = data[0].bits.bit1;
                    X6Y2 = data[0].bits.bit2;
                    X6Y3 = data[0].bits.bit3;
                    X6Y4 = data[0].bits.bit4;
                    X6Y5 = data[0].bits.bit5;
                    X6Y6 = data[0].bits.bit6;

                    X7Y0 = data[1].bits.bit0;
                    X7Y1 = data[1].bits.bit1;
                    X7Y2 = data[1].bits.bit2;
                    X7Y3 = data[1].bits.bit3;
                    X7Y4 = data[1].bits.bit4;
                    X7Y5 = data[1].bits.bit5;
                    X7Y6 = data[1].bits.bit6;

                    X8Y0 = data[2].bits.bit0;
                    X8Y1 = data[2].bits.bit1;
                    X8Y2 = data[2].bits.bit2;
                    X8Y3 = data[2].bits.bit3;
                    X8Y4 = data[2].bits.bit4;
                    X8Y5 = data[2].bits.bit5;
                    X8Y6 = data[2].bits.bit6;

                    X9Y0 = data[3].bits.bit0;
                    X9Y1 = data[3].bits.bit1;
                    X9Y2 = data[3].bits.bit2;
                    X9Y3 = data[3].bits.bit3;
                    X9Y4 = data[3].bits.bit4;
                    X9Y5 = data[3].bits.bit5;
                    X9Y6 = data[3].bits.bit6;

                    X10Y0 = data[4].bits.bit0;
                    X10Y1 = data[4].bits.bit1;
                    X10Y2 = data[4].bits.bit2;
                    X10Y3 = data[4].bits.bit3;
                    X10Y4 = data[4].bits.bit4;
                    X10Y5 = data[4].bits.bit5;
                    X10Y6 = data[4].bits.bit6;
                    break;

                case 2:
                    X12Y0 = data[0].bits.bit0;
                    X12Y1 = data[0].bits.bit1;
                    X12Y2 = data[0].bits.bit2;
                    X12Y3 = data[0].bits.bit3;
                    X12Y4 = data[0].bits.bit4;
                    X12Y5 = data[0].bits.bit5;
                    X12Y6 = data[0].bits.bit6;

                    X13Y0 = data[1].bits.bit0;
                    X13Y1 = data[1].bits.bit1;
                    X13Y2 = data[1].bits.bit2;
                    X13Y3 = data[1].bits.bit3;
                    X13Y4 = data[1].bits.bit4;
                    X13Y5 = data[1].bits.bit5;
                    X13Y6 = data[1].bits.bit6;

                    X14Y0 = data[2].bits.bit0;
                    X14Y1 = data[2].bits.bit1;
                    X14Y2 = data[2].bits.bit2;
                    X14Y3 = data[2].bits.bit3;
                    X14Y4 = data[2].bits.bit4;
                    X14Y5 = data[2].bits.bit5;
                    X14Y6 = data[2].bits.bit6;

                    X15Y0 = data[3].bits.bit0;
                    X15Y1 = data[3].bits.bit1;
                    X15Y2 = data[3].bits.bit2;
                    X15Y3 = data[3].bits.bit3;
                    X15Y4 = data[3].bits.bit4;
                    X15Y5 = data[3].bits.bit5;
                    X15Y6 = data[3].bits.bit6;

                    X16Y0 = data[4].bits.bit0;
                    X16Y1 = data[4].bits.bit1;
                    X16Y2 = data[4].bits.bit2;
                    X16Y3 = data[4].bits.bit3;
                    X16Y4 = data[4].bits.bit4;
                    X16Y5 = data[4].bits.bit5;
                    X16Y6 = data[4].bits.bit6;
                    break;

                case 3:
                    X18Y0 = data[0].bits.bit0;
                    X18Y1 = data[0].bits.bit1;
                    X18Y2 = data[0].bits.bit2;
                    X18Y3 = data[0].bits.bit3;
                    X18Y4 = data[0].bits.bit4;
                    X18Y5 = data[0].bits.bit5;
                    X18Y6 = data[0].bits.bit6;

                    X19Y0 = data[1].bits.bit0;
                    X19Y1 = data[1].bits.bit1;
                    X19Y2 = data[1].bits.bit2;
                    X19Y3 = data[1].bits.bit3;
                    X19Y4 = data[1].bits.bit4;
                    X19Y5 = data[1].bits.bit5;
                    X19Y6 = data[1].bits.bit6;

                    X20Y0 = data[2].bits.bit0;
                    X20Y1 = data[2].bits.bit1;
                    X20Y2 = data[2].bits.bit2;
                    X20Y3 = data[2].bits.bit3;
                    X20Y4 = data[2].bits.bit4;
                    X20Y5 = data[2].bits.bit5;
                    X20Y6 = data[2].bits.bit6;

                    X21Y0 = data[3].bits.bit0;
                    X21Y1 = data[3].bits.bit1;
                    X21Y2 = data[3].bits.bit2;
                    X21Y3 = data[3].bits.bit3;
                    X21Y4 = data[3].bits.bit4;
                    X21Y5 = data[3].bits.bit5;
                    X21Y6 = data[3].bits.bit6;

                    X22Y0 = data[4].bits.bit0;
                    X22Y1 = data[4].bits.bit1;
                    X22Y2 = data[4].bits.bit2;
                    X22Y3 = data[4].bits.bit3;
                    X22Y4 = data[4].bits.bit4;
                    X22Y5 = data[4].bits.bit5;
                    X22Y6 = data[4].bits.bit6;
                    break;

                case 4:
                    X24Y0 = data[0].bits.bit0;
                    X24Y1 = data[0].bits.bit1;
                    X24Y2 = data[0].bits.bit2;
                    X24Y3 = data[0].bits.bit3;
                    X24Y4 = data[0].bits.bit4;
                    X24Y5 = data[0].bits.bit5;
                    X24Y6 = data[0].bits.bit6;

                    X25Y0 = data[1].bits.bit0;
                    X25Y1 = data[1].bits.bit1;
                    X25Y2 = data[1].bits.bit2;
                    X25Y3 = data[1].bits.bit3;
                    X25Y4 = data[1].bits.bit4;
                    X25Y5 = data[1].bits.bit5;
                    X25Y6 = data[1].bits.bit6;

                    X26Y0 = data[2].bits.bit0;
                    X26Y1 = data[2].bits.bit1;
                    X26Y2 = data[2].bits.bit2;
                    X26Y3 = data[2].bits.bit3;
                    X26Y4 = data[2].bits.bit4;
                    X26Y5 = data[2].bits.bit5;
                    X26Y6 = data[2].bits.bit6;

                    X27Y0 = data[3].bits.bit0;
                    X27Y1 = data[3].bits.bit1;
                    X27Y2 = data[3].bits.bit2;
                    X27Y3 = data[3].bits.bit3;
                    X27Y4 = data[3].bits.bit4;
                    X27Y5 = data[3].bits.bit5;
                    X27Y6 = data[3].bits.bit6;

                    X28Y0 = data[4].bits.bit0;
                    X28Y1 = data[4].bits.bit1;
                    X28Y2 = data[4].bits.bit2;
                    X28Y3 = data[4].bits.bit3;
                    X28Y4 = data[4].bits.bit4;
                    X28Y5 = data[4].bits.bit5;
                    X28Y6 = data[4].bits.bit6;
                    break;

                case 5:
                    X30Y0 = data[0].bits.bit0;
                    X30Y1 = data[0].bits.bit1;
                    X30Y2 = data[0].bits.bit2;
                    X30Y3 = data[0].bits.bit3;
                    X30Y4 = data[0].bits.bit4;
                    X30Y5 = data[0].bits.bit5;
                    X30Y6 = data[0].bits.bit6;

                    X31Y0 = data[1].bits.bit0;
                    X31Y1 = data[1].bits.bit1;
                    X31Y2 = data[1].bits.bit2;
                    X31Y3 = data[1].bits.bit3;
                    X31Y4 = data[1].bits.bit4;
                    X31Y5 = data[1].bits.bit5;
                    X31Y6 = data[1].bits.bit6;

                    X32Y0 = data[2].bits.bit0;
                    X32Y1 = data[2].bits.bit1;
                    X32Y2 = data[2].bits.bit2;
                    X32Y3 = data[2].bits.bit3;
                    X32Y4 = data[2].bits.bit4;
                    X32Y5 = data[2].bits.bit5;
                    X32Y6 = data[2].bits.bit6;

                    X33Y0 = data[3].bits.bit0;
                    X33Y1 = data[3].bits.bit1;
                    X33Y2 = data[3].bits.bit2;
                    X33Y3 = data[3].bits.bit3;
                    X33Y4 = data[3].bits.bit4;
                    X33Y5 = data[3].bits.bit5;
                    X33Y6 = data[3].bits.bit6;

                    X34Y0 = data[4].bits.bit0;
                    X34Y1 = data[4].bits.bit1;
                    X34Y2 = data[4].bits.bit2;
                    X34Y3 = data[4].bits.bit3;
                    X34Y4 = data[4].bits.bit4;
                    X34Y5 = data[4].bits.bit5;
                    X34Y6 = data[4].bits.bit6;
                    break;
            }
        }
    }


    lcd.touched = false;
}