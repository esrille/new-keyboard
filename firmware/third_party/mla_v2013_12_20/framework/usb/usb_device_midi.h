//DOM-IGNORE-BEGIN
/*******************************************************************************
Software License Agreement

The software supplied herewith by Microchip Technology Incorporated
(the "Company") for its PICmicro(R) Microcontroller is intended and
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
//DOM-IGNORE-END

typedef union
{
    uint32_t Val;
    uint8_t v[4];
    union
    {
        struct
        {
            uint8_t CIN :4;
            uint8_t CN  :4;
            uint8_t MIDI_0;
            uint8_t MIDI_1;
            uint8_t MIDI_2;
        }; 
        struct
        {
            uint8_t CodeIndexNumber :4;
            uint8_t CableNumber     :4;
            uint8_t DATA_0;
            uint8_t DATA_1;
            uint8_t DATA_2;
        };
    };
} USB_AUDIO_MIDI_EVENT_PACKET, *P_USB_AUDIO_MIDI_EVENT_PACKET;

/* Code Index Number (CIN) values */
/*   Table 4-1 of midi10.pdf      */
#define MIDI_CIN_MISC_FUNCTION_RESERVED         0x0
#define MIDI_CIN_CABLE_EVENTS_RESERVED          0x1
#define MIDI_CIN_2_uint8_t_MESSAGE                 0x2
#define MIDI_CIN_MTC                            0x2
#define MIDI_CIN_SONG_SELECT                    0x2
#define MIDI_CIN_3_uint8_t_MESSAGE                 0x3
#define MIDI_CIN_SSP                            0x3
#define MIDI_CIN_SYSEX_START                    0x4
#define MIDI_CIN_SYSEX_CONTINUE                 0x4
#define MIDI_CIN_1_uint8_t_MESSAGE                 0x5
#define MIDI_CIN_SYSEX_ENDS_1                   0x5
#define MIDI_CIN_SYSEX_ENDS_2                   0x6
#define MIDI_CIN_SYSEX_ENDS_3                   0x7
#define MIDI_CIN_NOTE_OFF                       0x8
#define MIDI_CIN_NOTE_ON                        0x9
#define MIDI_CIN_POLY_KEY_PRESS                 0xA
#define MIDI_CIN_CONTROL_CHANGE                 0xB
#define MIDI_CIN_PROGRAM_CHANGE                 0xC
#define MIDI_CIN_CHANNEL_PREASURE               0xD
#define MIDI_CIN_PITCH_BEND_CHANGE              0xE
#define MIDI_CIN_SINGLE_uint8_t                    0xF
