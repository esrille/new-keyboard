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
********************************************************************/

/******** Include files **********************/
#include "oled.h"
#include <system.h>
#include <stdbool.h>
#include <stdint.h>

/******** Internal Definitions ****************/
#define	SH1101A_WRITE                       LATDbits.LATD4
#define	SH1101A_WRITE_TRIS                  TRISDbits.TRISD4
#define	SH1101A_READ                        LATDbits.LATD5
#define	SH1101A_READ_TRIS                   TRISDbits.TRISD5
#define	SH1101A_CHIP_SELECT                 LATDbits.LATD11
#define	SH1101A_CHIP_SELECT_TRIS            TRISDbits.TRISD11
#define	SH1101A_RESET                       LATDbits.LATD2
#define	SH1101A_RESET_TRIS                  TRISDbits.TRISD2
#define	SH1101A_DATA_COMMAND_SELECT         LATBbits.LATB15
#define	SH1101A_DATA_COMMAND_SELECT_TRIS    TRISBbits.TRISB15

#define SH1101A_DATA_SELECTED               1
#define SH1101A_COMMAND_SELECTED            0

#define SH1101A_WRITE_ENABLED               0
#define SH1101A_WRITE_DISABLED              1

#define SH1101A_READ_ENABLED                0
#define SH1101A_READ_DISABLED               1

#define SH1101A_CHIP_SELECT_ASSERT          0
#define SH1101A_CHIP_SELECT_DEASSERT        1

#define SH1101A_RESET_ASSERT                0
#define SH1101A_RESET_DEASSERT              1

#define CLIP_LEFT   2
#define CLIP_RIGHT  2

#define ROW_TOP     7
#define BOTTOM_ROW  0

typedef struct
{
    uint8_t row;
    uint8_t column;
} CURSOR;

typedef enum
{
    SET_COLUMN_ADDRESS_LOWER                            = 0b00000000,   //0x00
    SET_COLUMN_ADDRESS_UPPER                            = 0b00010000,   //0x10
    SET_DISPLAY_START_LINE                              = 0b01000000,   //0x40
    CONTRAST_CONTROL_MODE_SET                           = 0b10000001,   //0x81
    SET_SEGMENT_REMAP_RIGHT                             = 0b10100000,   //0xA0
    SET_SEGMENT_REMAP_LEFT                              = 0b10100001,   //0xA1
    SET_ENTIRE_DISPLAY_ON                               = 0b10100101,   //0xA5
    SET_ENTIRE_DISPLAY_OFF                              = 0b10100100,   //0xA4
    SET_NORMAL_DISPLAY                                  = 0b10100110,   //0xA6
    SET_REVERSE_DISPLAY                                 = 0b10100111,   //0xA7
    MULTIPLEX_RATION_MODE_SET                           = 0b10101000,   //0xA8
    DC_DC_CONTROL_MODE_SET                              = 0b10101101,   //0xAD
    DC_DC_MODE_SET_OFF                                  = 0b10001010,   //0x8A
    DC_DC_MODE_SET_ON                                   = 0b10001011,   //0x8B
    DISPLAY_ON                                          = 0b10101111,   //0xAF
    DISPLAY_OFF                                         = 0b10101110,   //0xAE
    SET_PAGE_ADDRESS                                    = 0b10110000,   //0xB0
    SET_COMMON_OUTPUT_SCAN_DIRECTION                    = 0b11000000,   //0xC0
    DISPLAY_OFFSET_MODE_SET                             = 0b11010011,   //0xD3
    SET_DISPLAY_DIVIDE_RATIO_FREQUENCY_MODE             = 0b11010101,   //0xD5
    DISCHARGE_PRECHARGE_PERIOD_MODE_SET                 = 0b11011001,   //0xD9
    COMMON_PADS_HARDWARE_CONFIGURATION_MODE_SET         = 0b11011010,   //0xDA
    VCOM_DESELECT_LEVEL_MODE_SET                        = 0b11011011,   //0xDB
    READ_MODIFY_WRITE_START                             = 0b11100000,   //0xE0
    READ_MODIFY_WRITE_END                               = 0b11101110,   //0xEE
    NOP                                                 = 0b11100011    //0xE3
} SH1101A_COMMAND;

/******** Internal Variables ****************/
static CURSOR cursor;

static const uint8_t characters[][6] =
{
    { 0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
      0b00000000,
    }, // " " 0x20

    { 0b00000000,
      0b00000000,
      0b11110010,
      0b00000000,
      0b00000000,
      0b00000000,
    }, // !   0x21

    { 0b00000000,
      0b11100000,
      0b00000000,
      0b11100000,
      0b00000000,
      0b00000000,
    }, // "   0x22

    { 0b00101000,
      0b11111110,
      0b00101000,
      0b11111110,
      0b00101000,
      0b00000000,
    }, // #   0x23

    { 0b00100100,
      0b01010100,
      0b11111110,
      0b01010100,
      0b01001000,
      0b00000000,
    }, // $   0x24

    { 0b11000100,
      0b11001000,
      0b00010000,
      0b00100110,
      0b01000110,
      0b00000000,
    }, // %   0x25

    { 0b01101100,
      0b10010010,
      0b10101010,
      0b01000100,
      0b10100000,
      0b00000000,
    }, // &   0x26

    { 0b00000000,
      0b10100000,
      0b11000000,
      0b00000000,
      0b00000000,
      0b00000000,
    }, // '   0x27

    { 0b00000000,
      0b00111000,
      0b01000100,
      0b10000010,
      0b00000000,
      0b00000000,
    }, // (   0x28

    { 0b00000000,
      0b10000010,
      0b01000100,
      0b00111000,
      0b00000000,
      0b00000000,
    }, // )   0x29

    { 0b00101000,
      0b00010000,
      0b01111100,
      0b00010000,
      0b00101000,
      0b00000000,
    }, // *   0x2A

    { 0b00010000,
      0b00010000,
      0b01111100,
      0b00010000,
      0b00010000,
      0b00000000,
    }, // +   0x2B

    { 0b00000000,
      0b00001010,
      0b00001100,
      0b00000000,
      0b00000000,
      0b00000000,
    }, // ,   0x2C

    { 0b00010000,
      0b00010000,
      0b00010000,
      0b00010000,
      0b00010000,
      0b00000000,
    }, // -   0x2D

    { 0b00000000,
      0b00000110,
      0b00000110,
      0b00000000,
      0b00000000,
      0b00000000,
    }, // .   0x2E

    { 0b00000100,
      0b00001000,
      0b00010000,
      0b00100000,
      0b01000000,
      0b00000000,
    }, // /   0x2F

    { 0b01111100,
      0b10001010,
      0b10010010,
      0b10100010,
      0b01111100,
      0b00000000,
    }, // 0   0x30

    { 0b00000000,
      0b01000010,
      0b11111110,
      0b00000010,
      0b00000000,
      0b00000000,
    }, // 1   0x31

    { 0b01000010,
      0b10000110,
      0b10001010,
      0b10010010,
      0b01100010,
      0b00000000,
    }, // 2   0x32

    { 0b10000100,
      0b10000010,
      0b10100010,
      0b11010010,
      0b10001100,
      0b00000000,
    }, // 3   0x33

    { 0b00011000,
      0b00101000,
      0b01001000,
      0b11111110,
      0b00001000,
      0b00000000,
    }, // 4   0x34

    { 0b11100100,
      0b10100010,
      0b10100010,
      0b10100010,
      0b10011100,
      0b00000000,
    }, // 5   0x35

    { 0b00111100,
      0b01010010,
      0b10010010,
      0b10010010,
      0b00001100,
      0b00000000,
    }, // 6   0x36

    { 0b10000000,
      0b10001110,
      0b10010000,
      0b10100000,
      0b11000000,
      0b00000000,
    }, // 7   0x37

    { 0b01101100,
      0b10010010,
      0b10010010,
      0b10010010,
      0b01101100,
      0b00000000,
    }, // 8   0x38

    { 0b01100000,
      0b10010010,
      0b10010010,
      0b10010100,
      0b01111000,
      0b00000000,
    }, // 9   0x39

    { 0b00000000,
      0b01101100,
      0b01101100,
      0b00000000,
      0b00000000,
      0b00000000,
    }, // :   0x3A

    { 0b00000000,
      0b01101010,
      0b01101100,
      0b00000000,
      0b00000000,
      0b00000000,
    }, // ;   0x3B

    { 0b00010000,
      0b00101000,
      0b01000100,
      0b10000010,
      0b00000000,
      0b00000000,
    }, // <   0x3C

    { 0b00101000,
      0b00101000,
      0b00101000,
      0b00101000,
      0b00101000,
      0b00000000,
    }, // =   0x3D

    { 0b00000000,
      0b10000010,
      0b01000100,
      0b00101000,
      0b00010000,
      0b00000000,
    }, // >   0x3E

    { 0b01000000,
      0b10000000,
      0b10001010,
      0b10010000,
      0b01100000,
      0b00000000,
    }, // ?   0x3F

    { 0b01001100,
      0b10010010,
      0b10011110,
      0b10000010,
      0b01111100,
      0b00000000,
    }, // @   0x40

    { 0b01111110,
      0b10001000,
      0b10001000,
      0b10001000,
      0b01111110,
      0b00000000,
    }, // A   0x41

    { 0b11111110,
      0b10010010,
      0b10010010,
      0b01101100,
      0b00000000,
      0b00000000,
    }, // B   0x42

    { 0b01111100,
      0b10000010,
      0b10000010,
      0b10000010,
      0b01000100,
      0b00000000,
    }, // C   0x43

    { 0b11111110,
      0b10000010,
      0b10000010,
      0b01000100,
      0b00111000,
      0b00000000,
    }, // D   0x44

    { 0b11111110,
      0b10010010,
      0b10010010,
      0b10010010,
      0b10000010,
      0b00000000,
    }, // E   0x45

    { 0b11111110,
      0b10010000,
      0b10010000,
      0b10010000,
      0b10000000,
      0b00000000,
    }, // F   0x46

    { 0b01111100,
      0b10000010,
      0b10010010,
      0b10010010,
      0b01011110,
      0b00000000,
    }, // G   0x47

    { 0b11111110,
      0b00010000,
      0b00010000,
      0b00010000,
      0b11111110,
      0b00000000,
    }, // H   0x48

    { 0b00000000,
      0b10000010,
      0b11111110,
      0b10000010,
      0b00000000,
      0b00000000,
    }, // I   0x49

    { 0b00000100,
      0b00000010,
      0b10000010,
      0b11111100,
      0b10000000,
      0b00000000,
    }, // J   0x4A

    { 0b11111110,
      0b00010000,
      0b00101000,
      0b01000100,
      0b10000010,
      0b00000000,
    }, // K   0x4B

    { 0b11111110,
      0b00000010,
      0b00000010,
      0b00000010,
      0b00000010,
      0b00000000,
    }, // L   0x4C

    { 0b11111110,
      0b01000000,
      0b00110000,
      0b01000000,
      0b11111110,
      0b00000000,
    }, // M   0x4D

    { 0b11111110,
      0b00100000,
      0b00010000,
      0b00001000,
      0b11111110,
      0b00000000,
    }, // N   0x4E

    { 0b01111100,
      0b10000010,
      0b10000010,
      0b10000010,
      0b01111100,
      0b00000000,
    }, // O   0x4F

    { 0b11111110,
      0b10010000,
      0b10010000,
      0b01100000,
      0b00000000,
      0b00000000,
    }, // P   0x50

    { 0b01111100,
      0b10000010,
      0b10001010,
      0b10000100,
      0b01111010,
      0b00000000,
    }, // Q   0x51

    { 0b11111110,
      0b10010000,
      0b10011000,
      0b10010100,
      0b01100010,
      0b00000000,
    }, // R   0x52

    { 0b01100010,
      0b10010010,
      0b10010010,
      0b10001100,
      0b00000000,
      0b00000000,
    }, // S   0x53

    { 0b10000000,
      0b10000000,
      0b11111110,
      0b10000000,
      0b10000000,
      0b00000000,
    }, // T   0x54

    { 0b11111100,
      0b00000010,
      0b00000010,
      0b00000010,
      0b11111100,
      0b00000000,
    }, // U   0x55

    { 0b11111000,
      0b00000100,
      0b00000010,
      0b00000100,
      0b11111000,
      0b00000000,
    }, // V   0x56

    { 0b11111100,
      0b00000010,
      0b00011100,
      0b00000010,
      0b11111100,
      0b00000000,
    }, // W   0x57

    { 0b11000110,
      0b00101000,
      0b00010000,
      0b00101000,
      0b11000110,
      0b00000000,
    }, // X   0x58

    { 0b11100000,
      0b00010000,
      0b00001110,
      0b00010000,
      0b11100000,
      0b00000000,
    }, // Y   0x59

    { 0b10000110,
      0b10001010,
      0b10010010,
      0b10100010,
      0b11000010,
      0b00000000,
    }, // Z   0x5A

    { 0b00000000,
      0b11111110,
      0b10000010,
      0b10000010,
      0b00000000,
      0b00000000,
    }, // [   0x5B

    { 0b01000000,
      0b00100000,
      0b00010000,
      0b00001000,
      0b00000100,
      0b00000000,
    }, // "\" 0x5C

    { 0b00000000,
      0b10000010,
      0b10000010,
      0b11111110,
      0b00000000,
      0b00000000,
    }, // ]   0x5D

    { 0b00100000,
      0b01000000,
      0b10000000,
      0b01000000,
      0b00100000,
      0b00000000,
    }, // ^   0x5E

    { 0b00000010,
      0b00000010,
      0b00000010,
      0b00000010,
      0b00000010,
      0b00000000,
    }, // _   0x5F

    { 0b00000000,
      0b10000000,
      0b01000000,
      0b00100000,
      0b00000000,
      0b00000000,
    }, // `   0x60

    { 0b00000100,
      0b00101010,
      0b00101010,
      0b00101010,
      0b00011110,
      0b00000000,
    }, // a   0x61

    { 0b11111110,
      0b00010010,
      0b00100010,
      0b00100010,
      0b00011100,
      0b00000000,
    }, // b   0x62

    { 0b00011100,
      0b00100010,
      0b00100010,
      0b00100010,
      0b00010100,
      0b00000000,
    }, // c   0x63

    { 0b00011100,
      0b00100010,
      0b00100010,
      0b00010010,
      0b11111110,
      0b00000000,
    }, // d   0x64

    { 0b00011100,
      0b00101010,
      0b00101010,
      0b00101010,
      0b00011000,
      0b00000000,
    }, // e   0x65

    { 0b00010000,
      0b01111110,
      0b10010000,
      0b10000000,
      0b01000000,
      0b00000000,
    }, // f   0x66

    { 0b00110000,
      0b01001010,
      0b01001010,
      0b01001010,
      0b01111100,
      0b00000000,
    }, // g   0x67

    { 0b11111110,
      0b00010000,
      0b00100000,
      0b00100000,
      0b00011110,
      0b00000000,
    }, // h   0x68

    { 0b00000000,
      0b00100010,
      0b10111110,
      0b00000010,
      0b00000000,
      0b00000000,
    }, // i   0x69

    { 0b00000100,
      0b00000010,
      0b00100010,
      0b10111100,
      0b00000000,
      0b00000000,
    }, // j   0x6A

    { 0b11111110,
      0b00001000,
      0b00010100,
      0b00100010,
      0b00000000,
      0b00000000,
    }, // k   0x6B

    { 0b00000000,
      0b10000010,
      0b11111110,
      0b00000010,
      0b00000000,
      0b00000000,
    }, // l   0x6C

    { 0b00011110,
      0b00100000,
      0b00011000,
      0b00100000,
      0b00011110,
      0b00000000,
    }, // m   0x6D

    { 0b00111110,
      0b00010000,
      0b00100000,
      0b00100000,
      0b00011110,
      0b00000000,
    }, // n   0x6E

    { 0b00011100,
      0b00100010,
      0b00100010,
      0b00100010,
      0b00011100,
      0b00000000,
    }, // o   0x6F

    { 0b00111110,
      0b00101000,
      0b00101000,
      0b00101000,
      0b00010000,
      0b00000000,
    }, // p   0x70

    { 0b00010000,
      0b00101000,
      0b00101000,
      0b00011000,
      0b00111110,
      0b00000000,
    }, // q   0x71

    { 0b00111110,
      0b00010000,
      0b00100000,
      0b00100000,
      0b00010000,
      0b00000000,
    }, // r   0x72

    { 0b00010010,
      0b00101010,
      0b00101010,
      0b00101010,
      0b00000100,
      0b00000000,
    }, // s   0x73

    { 0b00100000,
      0b11111100,
      0b00100010,
      0b00000010,
      0b00000100,
      0b00000000,
    }, // t   0x74

    { 0b00111100,
      0b00000010,
      0b00000010,
      0b00000100,
      0b00111110,
      0b00000000,
    }, // u   0x75

    { 0b00111000,
      0b00000100,
      0b00000010,
      0b00000100,
      0b00111000,
      0b00000000,
    }, // v   0x76

    { 0b00111100,
      0b00000010,
      0b00001100,
      0b00000010,
      0b00111100,
      0b00000000,
    }, // w   0x77

    { 0b00100010,
      0b00010100,
      0b00001000,
      0b00010100,
      0b00100010,
      0b00000000,
    }, // x   0x78

    { 0b00110000,
      0b00001010,
      0b00001010,
      0b00001010,
      0b00111100,
      0b00000000,
    }, // y   0x79

    { 0b00100010,
      0b00100110,
      0b00101010,
      0b00110010,
      0b00000000,
      0b00000000,
    }, // z   0x7A

    { 0b00000000,
      0b00010000,
      0b01101100,
      0b10000010,
      0b00000000,
      0b00000000,
    }, // {   0x7B

    { 0b00000000,
      0b00000000,
      0b11111110,
      0b00000000,
      0b00000000,
      0b00000000,
    }, // |   0x7C

    { 0b00000000,
      0b10000010,
      0b01101100,
      0b00010000,
      0b00000000,
      0b00000000,
    }, // }   0x7D

    { 0b01000000,
      0b10000000,
      0b01000000,
      0b00100000,
      0b01000000,
      0b00000000,
    } // ~   0x7E
};

/******** Internal Function Prototypes ****************/
static void OLED_CursorShiftLeft(void);
static void DelayMs(uint16_t time);
static void SH1101A_DataWrite(uint8_t data);
static void SH1101A_CommandWrite(SH1101A_COMMAND command, uint8_t data);
static void SH1101A_AddressSet(uint8_t page, uint8_t address);
static void OLED_CursorMove(uint8_t row, uint8_t column);

/*********************************************************************
 *                           Functions
 ********************************************************************/


/*********************************************************************
* Function:  void  DelayMs(uint16_t time)
*
* PreCondition: none
*
* Input: time - delay in ms
*
* Output: none
*
* Side Effects: execution is blocked here (interrupts still allowed) during
* the duration of the requested wait time.
*
* Overview: delays execution on time specified in ms
*
********************************************************************/
#define DELAY_1MS 32000

static void  DelayMs(uint16_t time)
{
    unsigned delay;

    while(time--)
    {
        for(delay=0; delay<DELAY_1MS; delay++)
        {
            Nop();
        }
    }
}

/*********************************************************************
* Function:  static void SH1101A_DataWrite(uint8_t data)
*
* PreCondition: SH1101A initialized
*
* Input: data - the data to write to the controller
*
* Output: none
*
* Side Effects: data is written to the controller
*
* Overview: Writes the requested data to the SH1101A controller
*
********************************************************************/
static void SH1101A_DataWrite(uint8_t data)
{
    LATE = data;

    SH1101A_CHIP_SELECT = SH1101A_CHIP_SELECT_ASSERT;
    
    SH1101A_WRITE = SH1101A_WRITE_ENABLED;
    Nop();
    Nop();
    SH1101A_WRITE = SH1101A_WRITE_DISABLED;

    SH1101A_CHIP_SELECT = SH1101A_CHIP_SELECT_DEASSERT;
}

/*********************************************************************
* Function:  static void SH1101A_CommandWrite(SH1101A_COMMAND command, uint8_t data)
*
* PreCondition: SH1101A initialized
*
* Input: command - the command to execute on the SH1101A
*        data - the data payload (if applicable)
*
* Output: none
*
* Side Effects: command/command data is written to the controller
*
* Overview: Executes the requested command.
*
********************************************************************/
static void SH1101A_CommandWrite(SH1101A_COMMAND command, uint8_t data)
{
    switch(command)
    {
        case SET_COLUMN_ADDRESS_LOWER:
        case SET_COLUMN_ADDRESS_UPPER:
        case SET_DISPLAY_START_LINE:
        case SET_PAGE_ADDRESS:
            command |= data;
            break;
        default:
            break;
    }

    LATE = command;

    SH1101A_CHIP_SELECT = SH1101A_CHIP_SELECT_ASSERT;
    SH1101A_DATA_COMMAND_SELECT = SH1101A_COMMAND_SELECTED;
    
    SH1101A_WRITE = SH1101A_WRITE_ENABLED;
    Nop();
    Nop();
    SH1101A_WRITE = SH1101A_WRITE_DISABLED;

    switch(command)
    {
        case CONTRAST_CONTROL_MODE_SET:
        case MULTIPLEX_RATION_MODE_SET:
        case DISPLAY_OFFSET_MODE_SET:
        case SET_DISPLAY_DIVIDE_RATIO_FREQUENCY_MODE:
        case DISCHARGE_PRECHARGE_PERIOD_MODE_SET:
        case VCOM_DESELECT_LEVEL_MODE_SET:
        case COMMON_PADS_HARDWARE_CONFIGURATION_MODE_SET:
            LATE = data;
            SH1101A_WRITE = SH1101A_WRITE_ENABLED;
            Nop();
            Nop();
            SH1101A_WRITE = SH1101A_WRITE_DISABLED;
            break;
        default:
            break;
    }
    
    SH1101A_CHIP_SELECT = SH1101A_CHIP_SELECT_DEASSERT;

    SH1101A_DATA_COMMAND_SELECT = SH1101A_DATA_SELECTED;
}

/*********************************************************************
* Function:  static void SH1101A_AddressSet(uint8_t page, uint8_t address)
*
* PreCondition: SH1101A initialized
*
* Input: page - page that the cursor should move to
*        address - the address within that page that we should move to
*
* Output: none
*
* Side Effects: SH1101A cursor moved
*
* Overview: Moves the cursor of the SH1101A to the requested location
*
********************************************************************/
static void SH1101A_AddressSet(uint8_t page, uint8_t address)
{
    SH1101A_CommandWrite(SET_PAGE_ADDRESS, page);
    SH1101A_CommandWrite(SET_COLUMN_ADDRESS_LOWER, (address & 0x0F) );
    SH1101A_CommandWrite(SET_COLUMN_ADDRESS_UPPER, (address >> 4) );
}

/*********************************************************************
* Function:  static void OLED_CursorMove(uint8_t row, uint8_t column)
*
* PreCondition: SH1101A initialized
*
* Input: row - the logical row to write to [0 - (OLED_NUMBER_OF_ROWS-1)]
*        column - the logical column to write to [0 - (OLED_NUMBER_OF_COLUMNS - 1)]
*
* Output: none
*
* Side Effects: SH1101A cursor moved
*
* Overview: Moves the cursor of the SH1101A to the requested location.  Also
* updates the internal record of where the cursor is.
*
********************************************************************/
static void OLED_CursorMove(uint8_t row, uint8_t column)
{
    if(row >= OLED_NUMBER_OF_ROWS)
    {
        return;
    }

    if(column >= OLED_NUMBER_OF_COLUMNS)
    {
        return;
    }

    /* The row is "ROW_TOP - row" because on this board the OLED is oreiented
     * upside down compared to the button text. */
    SH1101A_AddressSet(ROW_TOP - row, CLIP_LEFT + (6*column) );
    
    cursor.row = row;
    cursor.column = column;
}

/*********************************************************************
* Function: void OLED_ClearStreen()
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
void OLED_ClearScreen(void)
{
    uint8_t    page, segment;

    for(page = 0; page < 8; page++)
    {       
        //Set to the start of the page
        SH1101A_AddressSet(page, 0x00);

        for(segment = 0; segment < 132; segment++)
        {
            // Write to all 132 bytes
            SH1101A_DataWrite(0x00);
        }
    }

    OLED_CursorMove(0,0);
}

/*********************************************************************
* Function: bool OLED_Initialize(void);
*
* Overview: Configures the OLED
*
* PreCondition: none
*
* Input: None
*
* Output: true if successful, false if OLED not configured or doesn?t
*         exist for this board.
*
********************************************************************/
bool OLED_Initialize(void)
{
    SH1101A_WRITE = SH1101A_WRITE_DISABLED;
    SH1101A_WRITE_TRIS = 0;
    SH1101A_READ = SH1101A_READ_DISABLED;
    SH1101A_READ_TRIS = 0;
    SH1101A_CHIP_SELECT = SH1101A_CHIP_SELECT_DEASSERT;
    SH1101A_CHIP_SELECT_TRIS = 0;
    SH1101A_RESET = SH1101A_RESET_DEASSERT;
    SH1101A_RESET_TRIS = 0;
    SH1101A_DATA_COMMAND_SELECT = SH1101A_DATA_SELECTED;
    SH1101A_DATA_COMMAND_SELECT_TRIS = 0;

    AD1PCFGLbits.PCFG15 = 1;
    TRISE = 0x00;

    SH1101A_RESET = SH1101A_RESET_ASSERT;
    DelayMs(1);

    SH1101A_RESET = SH1101A_RESET_DEASSERT;

    // hard delay inserted here for devices that needs delays after reset.
    // Value will vary from device to device, please refer to the specific
    // device data sheet for details.
    DelayMs(1);

    // Setup Display

    SH1101A_CommandWrite(DISPLAY_OFF, 0x00);
    SH1101A_CommandWrite(VCOM_DESELECT_LEVEL_MODE_SET, 0x23);
    SH1101A_CommandWrite(DISCHARGE_PRECHARGE_PERIOD_MODE_SET, 0x22);

    // [A0]:column address 0 is map to SEG0
    SH1101A_CommandWrite(SET_SEGMENT_REMAP_LEFT, 0x00);

    // [A1]:column address 131 is map to SEG0
    // COM Output Scan Direction
    // C0 is COM0 to COMn, C8 is COMn to COM0
    SH1101A_CommandWrite(SET_COMMON_OUTPUT_SCAN_DIRECTION, 8);

    // COM Pins Hardware Configuration
    SH1101A_CommandWrite(COMMON_PADS_HARDWARE_CONFIGURATION_MODE_SET, 0x12);

    // Multiplex Ratio
    // set to 64 mux
    SH1101A_CommandWrite(MULTIPLEX_RATION_MODE_SET, 0x3F);

    // Display Clock Divide
    // set to 100Hz
    SH1101A_CommandWrite(SET_DISPLAY_DIVIDE_RATIO_FREQUENCY_MODE, 0xA0);

    // Contrast Control Register
    // display 0 ~ 127; 2C
    SH1101A_CommandWrite(CONTRAST_CONTROL_MODE_SET, 0x60);

    // Display Offset
    // no offset
    SH1101A_CommandWrite(DISPLAY_OFFSET_MODE_SET, 0x00);

    //Normal display
    SH1101A_CommandWrite(SET_NORMAL_DISPLAY, 0x00);

    SH1101A_CommandWrite(DC_DC_CONTROL_MODE_SET, 0x00);
    SH1101A_CommandWrite(DC_DC_MODE_SET_ON, 0x00);

    // Display ON/OFF
    SH1101A_CommandWrite(DISPLAY_ON, 0x00);
    DelayMs(150);

    SH1101A_CommandWrite(SET_ENTIRE_DISPLAY_OFF, 0x00);

    // Display Start Line
    SH1101A_CommandWrite(SET_DISPLAY_START_LINE, 0x00);

    // Lower Column Address
    SH1101A_CommandWrite(SET_COLUMN_ADDRESS_LOWER, 0x00);

    // Higher Column Address
    SH1101A_CommandWrite(SET_COLUMN_ADDRESS_UPPER, 0x00);
    DelayMs(1);

    OLED_ClearScreen();

    return true;
}

/*********************************************************************
* Function: static void OLED_CursorShiftLeft(void)
*
* Overview: Shifts the cursor left one.  Takes into account wraps if
* required.
*
* PreCondition: none
*
* Input: none
*
* Output: None
*
********************************************************************/
static void OLED_CursorShiftLeft(void)
{
    if(cursor.column == 0)
    {
        if(cursor.row == 0)
        {
            cursor.row = OLED_NUMBER_OF_ROWS - 1;
        }
        else
        {
            cursor.row--;
        }
        OLED_CursorMove(cursor.row, OLED_NUMBER_OF_COLUMNS - 1);
    }
    else
    {
        OLED_CursorMove(cursor.row, cursor.column - 1);
    }
}

/*********************************************************************
* Function: void OLED_PutChar(char inputCharacter)
*
* Overview: Prints a character
*
* PreCondition: none
*
* Input: char inputCharacter - the character to print
*
* Output: None
*
********************************************************************/
void OLED_PutChar(char inputCharacter)
{
    /* if the character is one of the printable characters, let's print it. */
    if( (inputCharacter >= ' ') &&
        (inputCharacter <= '~') )
    {
        /* First we need to see if we are at the end of a row. */
        if(cursor.column >= OLED_NUMBER_OF_COLUMNS)
        {
            /* If we are at the end of a row, then move to the next row. */
            cursor.row++;

            /* If we were on the last row, move back to the first row. */
            if(cursor.row >= OLED_NUMBER_OF_ROWS)
            {
                cursor.row = 0;
            }
            
            //Move to the updated row, in the first column
            OLED_CursorMove(cursor.row,0);
        }
        
        /* The character is in the displayable character range.  Load the
         * character to be printed. */
        inputCharacter -= ' ';

        SH1101A_DataWrite(characters[inputCharacter][0]);
        SH1101A_DataWrite(characters[inputCharacter][1]);
        SH1101A_DataWrite(characters[inputCharacter][2]);
        SH1101A_DataWrite(characters[inputCharacter][3]);
        SH1101A_DataWrite(characters[inputCharacter][4]);
        SH1101A_DataWrite(characters[inputCharacter][5]);

        cursor.column++;
    }
    else
    {
        /* If the input character wasn't a standard character, then let's handle
         * it here. */
        switch (inputCharacter)
        {
            /* On a carriage return, we want to return to the start of the row
             * that we are on. */
            case '\r':
                OLED_CursorMove(cursor.row,0);
                break ;

            /* On a new line, we need to move to the next row, but stay at the
             * column number that we are already at. */
            case '\n':
                cursor.row++;

                if(cursor.row >= OLED_NUMBER_OF_ROWS)
                {
                    cursor.row = 0;
                }
                
                OLED_CursorMove(cursor.row, cursor.column);
                break ;

            /* If backspace is hit, then move back one character, blank it out,
             * (which then requires us to move the cursor back to the space that
             * we just blanked). */
            case '\b':
                OLED_CursorShiftLeft();
                OLED_PutChar(' ');
                OLED_CursorShiftLeft();
                break ;
        }
    }
}

/*********************************************************************
* Function: void OLED_String(char* string, uint16_t length)
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
void OLED_PutString(char* string, uint16_t length)
{
    while(length)
    {
        OLED_PutChar(*string);
        string++;
        length--;
    }
}







