/*
 * Copyright 2014-2016 Esrille Inc.
 *
 * This file is a modified version of app_device_keyboard.c provided by
 * Microchip Technology, Inc. for using Esrille New Keyboard.
 * See the file NOTICE for copying permission.
 */

/*******************************************************************************
  USB device keyboard demo source file

  Company:
    Microchip Technology Inc.

  File Name:
    keyboard.c

  Summary:
    This demo will make the USB peripheral look like a USB keyboard.

  Description:
    This demo will make the USB peripheral look like a USB keyboard.  The code
    defines the behavior of the keyboard and how it operates.
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
#include <stdint.h>
#include <stdio.h>
#include <usb/usb.h>
#include <usb/usb_device_hid.h>
#include <plib/timers.h>

#include "app_device_keyboard.h"
#include "app_led_usb_status.h"

#include <Keyboard.h>

#define SCAN_DELAY  (_XTAL_FREQ / 256 / 4 / 167 + 1) // About 6 [msec]

// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Constants
// *****************************************************************************
// *****************************************************************************

//Class specific descriptor - HID Keyboard
const struct{uint8_t report[HID_RPT01_SIZE];}hid_rpt01={
{   0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, DESC_CONFIG_WORD(0xFF),  //   LOGICAL_MAXIMUM (255)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0xFF,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0}                          // End Collection
};


// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data Types
// *****************************************************************************
// *****************************************************************************

/* This typedef defines the only INPUT report found in the HID report
 * descriptor and gives an easy way to create the OUTPUT report. */
typedef struct __attribute__((packed))
{
    /* The union below represents the first byte of the INPUT report.  It is
     * formed by the following HID report items:
     *
     *  0x19, 0xe0, //   USAGE_MINIMUM (Keyboard LeftControl)
     *  0x29, 0xe7, //   USAGE_MAXIMUM (Keyboard Right GUI)
     *  0x15, 0x00, //   LOGICAL_MINIMUM (0)
     *  0x25, 0x01, //   LOGICAL_MAXIMUM (1)
     *  0x75, 0x01, //   REPORT_SIZE (1)
     *  0x95, 0x08, //   REPORT_COUNT (8)
     *  0x81, 0x02, //   INPUT (Data,Var,Abs)
     *
     * The report size is 1 specifying 1 bit per entry.
     * The report count is 8 specifying there are 8 entries.
     * These entries represent the Usage items between Left Control (the usage
     * minimum) and Right GUI (the usage maximum).
     */
    union __attribute__((packed))
    {
        uint8_t value;
        struct __attribute__((packed))
        {
            unsigned leftControl    :1;
            unsigned leftShift      :1;
            unsigned leftAlt        :1;
            unsigned leftGUI        :1;
            unsigned rightControl   :1;
            unsigned rightShift     :1;
            unsigned rightAlt       :1;
            unsigned rightGUI       :1;
        } bits;
    } modifiers;

    /* There is one byte of constant data/padding that is specified in the
     * input report:
     *
     *  0x95, 0x01,                    //   REPORT_COUNT (1)
     *  0x75, 0x08,                    //   REPORT_SIZE (8)
     *  0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
     */
    unsigned :8;

    /* The last INPUT item in the INPUT report is an array type.  This array
     * contains an entry for each of the keys that are currently pressed until
     * the array limit, in this case 6 concurent key presses.
     *
     *  0x95, 0x06,                    //   REPORT_COUNT (6)
     *  0x75, 0x08,                    //   REPORT_SIZE (8)
     *  0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
     *  0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
     *  0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
     *  0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
     *  0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
     *
     * Report count is 6 indicating that the array has 6 total entries.
     * Report size is 8 indicating each entry in the array is one byte.
     * The usage minimum indicates the lowest key value (Reserved/no event)
     * The usage maximum indicates the highest key value (Application button)
     * The logical minimum indicates the remapped value for the usage minimum:
     *   No Event has a logical value of 0.
     * The logical maximum indicates the remapped value for the usage maximum:
     *   Application button has a logical value of 101.
     *
     * In this case the logical min/max match the usage min/max so the logical
     * remapping doesn't actually change the values.
     *
     * To send a report with the 'a' key pressed (usage value of 0x04, logical
     * value in this example of 0x04 as well), then the array input would be the
     * following:
     *
     * LSB [0x04][0x00][0x00][0x00][0x00][0x00] MSB
     *
     * If the 'b' button was then pressed with the 'a' button still held down,
     * the report would then look like this:
     *
     * LSB [0x04][0x05][0x00][0x00][0x00][0x00] MSB
     *
     * If the 'a' button was then released with the 'b' button still held down,
     * the resulting array would be the following:
     *
     * LSB [0x05][0x00][0x00][0x00][0x00][0x00] MSB
     *
     * The 'a' key was removed from the array and all other items in the array
     * were shifted down. */
    uint8_t keys[6];
} KEYBOARD_INPUT_REPORT;


/* This typedef defines the only OUTPUT report found in the HID report
 * descriptor and gives an easy way to parse the OUTPUT report. */
typedef union __attribute__((packed))
{
    /* The OUTPUT report is comprised of only one byte of data. */
    uint8_t value;
    struct
    {
        /* There are two report items that form the one byte of OUTPUT report
         * data.  The first report item defines 5 LED indicators:
         *
         *  0x95, 0x05,                    //   REPORT_COUNT (5)
         *  0x75, 0x01,                    //   REPORT_SIZE (1)
         *  0x05, 0x08,                    //   USAGE_PAGE (LEDs)
         *  0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
         *  0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
         *  0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
         *
         * The report count indicates there are 5 entries.
         * The report size is 1 indicating each entry is just one bit.
         * These items are located on the LED usage page
         * These items are all of the usages between Num Lock (the usage
         * minimum) and Kana (the usage maximum).
         */
        unsigned numLock        :1;
        unsigned capsLock       :1;
        unsigned scrollLock     :1;
        unsigned compose        :1;
        unsigned kana           :1;

        /* The second OUTPUT report item defines 3 bits of constant data
         * (padding) used to make a complete byte:
         *
         *  0x95, 0x01,                    //   REPORT_COUNT (1)
         *  0x75, 0x03,                    //   REPORT_SIZE (3)
         *  0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
         *
         * Report count of 1 indicates that there is one entry
         * Report size of 3 indicates the entry is 3 bits long. */
        unsigned                :3;
    } leds;
} KEYBOARD_OUTPUT_REPORT;


/* This creates a storage type for all of the information required to track the
 * current state of the keyboard. */
typedef struct
{
    USB_HANDLE lastINTransmission;
    USB_HANDLE lastOUTTransmission;
} KEYBOARD;

// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Variables
// *****************************************************************************
// *****************************************************************************
static KEYBOARD keyboard;

#if !defined(KEYBOARD_INPUT_REPORT_DATA_BUFFER_ADDRESS_TAG)
    #define KEYBOARD_INPUT_REPORT_DATA_BUFFER_ADDRESS_TAG
#endif
static KEYBOARD_INPUT_REPORT inputReport KEYBOARD_INPUT_REPORT_DATA_BUFFER_ADDRESS_TAG;

#if !defined(KEYBOARD_OUTPUT_REPORT_DATA_BUFFER_ADDRESS_TAG)
    #define KEYBOARD_OUTPUT_REPORT_DATA_BUFFER_ADDRESS_TAG
#endif
static volatile KEYBOARD_OUTPUT_REPORT outputReport KEYBOARD_OUTPUT_REPORT_DATA_BUFFER_ADDRESS_TAG;

static volatile unsigned char* rowPorts[8] = {
    &TRISA,
    &TRISA,
    &TRISA,
    &TRISA,
    &TRISA,
    &TRISA,
    &TRISE,
    &TRISE
};

static volatile unsigned char* rowPorts4[8] = {
    &TRISE,
    &TRISE,
    &TRISE,
    &TRISA,
    &TRISA,
    &TRISA,
    &TRISA,
    &TRISA,
};

#if APP_MACHINE_VALUE != 0x4550
// Rev 6
static volatile unsigned char* rowPorts6[8] = {
    &TRISA,
    &TRISA,
    &TRISA,
    &TRISA,
    &TRISA,
    &TRISE,
    &TRISE,
    &TRISE,
};
#endif

static unsigned char rowBits[8] = {
    1u << 0,
    1u << 1,
    1u << 2,
    1u << 3,
    1u << 4,
    1u << 5,
    1u << 0,
    1u << 1
};

// Rev 3
static unsigned char rowBits3[8] = {
    1u << 1,
    1u << 2,
    1u << 3,
    1u << 4,
    1u << 5,
    1u << 0,
    1u << 1,
    1u << 2
};

// Rev 4
static unsigned char rowBits4[8] = {
    1u << 2,
    1u << 1,
    1u << 0,
    1u << 5,
    1u << 4,
    1u << 3,
    1u << 2,
    1u << 1
};

#if APP_MACHINE_VALUE != 0x4550
// Rev 6
static unsigned char rowBits6[8] = {
    1u << 0,
    1u << 1,
    1u << 2,
    1u << 3,
    1u << 5,
    1u << 0,
    1u << 1,
    1u << 2
};
#endif

static volatile unsigned char* columnPorts[12] = {
    &PORTD,
    &PORTD,
    &PORTD,
    &PORTD,
    &PORTD,
    &PORTD,
    &PORTB,
    &PORTB,
    &PORTB,
    &PORTB,
    &PORTB,
    &PORTB,
};

// Rev 4
static volatile unsigned char* columnPorts4[12] = {
    &PORTB,
    &PORTB,
    &PORTB,
    &PORTB,
    &PORTB,
    &PORTB,
    &PORTD,
    &PORTD,
    &PORTD,
    &PORTD,
    &PORTD,
    &PORTD,
};

#if APP_MACHINE_VALUE != 0x4550
// Rev 6
static volatile unsigned char* columnPorts6[12] = {
    &PORTD,
    &PORTD,
    &PORTB,
    &PORTB,
    &PORTB,
    &PORTB,
    &PORTD,
    &PORTD,
    &PORTD,
    &PORTB,
    &PORTB,
    &PORTB,
};
#endif

static unsigned char columnBits[12] = {
    1u << 4,
    1u << 5,
    1u << 6,
    1u << 7,
    1u << 2,
    1u << 3,
    1u << 5,
    1u << 4,
    1u << 1,
    1u << 0,
    1u << 2,
    1u << 3,
};

// Rev 3
static unsigned char columnBits3[12] = {
    1u << 7,
    1u << 6,
    1u << 5,
    1u << 4,
    1u << 3,
    1u << 2,
    1u << 5,
    1u << 4,
    1u << 3,
    1u << 2,
    1u << 0,
    1u << 1,
};

// Rev 4
static unsigned char columnBits4[12] = {
    1u << 0,
    1u << 1,
    1u << 2,
    1u << 3,
    1u << 4,
    1u << 5,
    1u << 7,
    1u << 6,
    1u << 5,
    1u << 4,
    1u << 1,
    1u << 0,
};

#if APP_MACHINE_VALUE != 0x4550
// Rev 6
static unsigned char columnBits6[12] = {
    1u << 6,
    1u << 7,
    1u << 0,
    1u << 1,
    1u << 2,
    1u << 3,
    1u << 1,
    1u << 2,
    1u << 3,
    1u << 7,
    1u << 6,
    1u << 5,
};
#endif

static int tick;
static int8_t xmit = XMIT_NORMAL;


// *****************************************************************************
// *****************************************************************************
// Section: Private Prototypes
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Macros or Functions
// *****************************************************************************
// *****************************************************************************

void APP_KeyboardConfigure(void)
{
#if APP_MACHINE_VALUE != 0x4550
    if (6 <= BOARD_REV_VALUE) {
        for (char i = 0; i < 8; ++i) {
            rowPorts[i] = rowPorts6[i];
            rowBits[i] = rowBits6[i];
        }
        for (char i = 0; i < 12; ++i) {
            columnPorts[i] = columnPorts6[i];
            columnBits[i] = columnBits6[i];
        }
    }
    else
#endif
    if (4 <= BOARD_REV_VALUE)
    {
        for (char i = 0; i < 8; ++i) {
            rowPorts[i] = rowPorts4[i];
            rowBits[i] = rowBits4[i];
        }
        for (char i = 0; i < 12; ++i) {
            columnPorts[i] = columnPorts4[i];
            columnBits[i] = columnBits4[i];
        }
        if (5 <= BOARD_REV_VALUE) {
            columnBits[10] = 1u << 0;
            columnBits[11] = 1u << 1;
        }
    }
    else if (BOARD_REV_VALUE == 3)
    {
        rowPorts[5] = &TRISE;
        for (char i = 0; i < 8; ++i)
            rowBits[i] = rowBits3[i];
        for (char i = 0; i < 12; ++i)
            columnBits[i] = columnBits3[i];
    }
}

void APP_KeyboardInit(void)
{
    //initialize the variable holding the handle for the last
    // transmission
    keyboard.lastINTransmission = 0;

    //initialize the variable holding the keyboard LED state data.
    //Note OS X assumes every LED is turned off by default.
    outputReport.value = 0;

    //enable the HID endpoint
    USBEnableEndpoint(HID_EP, USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);

    //Arm OUT endpoint so we can receive caps lock, num lock, etc. info from host
    keyboard.lastOUTTransmission = HIDRxPacket(HID_EP, (uint8_t*) &outputReport, sizeof(outputReport));

    OpenTimer0(TIMER_INT_OFF & T0_16BIT & T0_SOURCE_INT & T0_PS_1_256);
    tick = (int) ReadTimer0();
}

uint8_t* APP_KeyboardScan(void)
{
    int8_t row;
    uint8_t column;

    if (xmit == XMIT_IN_ORDER) {
        uint8_t key = peekMacro();
        uint8_t mod = 0;
#if APP_MACHINE_VALUE != 0x4550
        if (key == KEYPAD_PERCENT) {
            key = KEY_5;
            mod = MOD_LEFTSHIFT;
        }
#endif
        if (inputReport.keys[0] && inputReport.keys[0] == key)
            inputReport.keys[0] = 0;    // BRK
        else {
            getMacro();
            inputReport.keys[0] = key;
            inputReport.modifiers.value = mod;
            if (!inputReport.keys[0])
                xmit = XMIT_NONE;
        }
    } else {
        if (BUTTON_IsPressed()) {
            BUTTON_Enable();
            for (row = 7; 0 <= row; --row) {
                *rowPorts[row] &= ~rowBits[row];
                for (column = 0; column < 12; ++column) {
                    if (!(*columnPorts[column] & columnBits[column]))
                        onPressed(row, column);
                }
                *rowPorts[row] |= rowBits[row];
            }
            BUTTON_Disable();
        }

        xmit = makeReport((uint8_t*) &inputReport);
        switch (xmit) {
        case XMIT_BRK:
            memset(&inputReport + 2, 0, 6);
            break;
        case XMIT_NORMAL:
            break;
        case XMIT_IN_ORDER:
            for (uint8_t i = 0; i < 6; ++i)
                emitKey(inputReport.keys[i]);
            inputReport.keys[0] = beginMacro(6);
            memset(inputReport.keys + 1, 0, 5);
            break;
        case XMIT_MACRO:
            xmit = XMIT_IN_ORDER;
            inputReport.modifiers.value = 0;
            inputReport.keys[0] = beginMacro(MAX_MACRO_SIZE);
            memset(inputReport.keys + 1, 0, 5);
            break;
        default:
            break;
        }
    }
    if (!xmit)
        return NULL;
    return (uint8_t*) &inputReport;
}

void APP_KeyboardTasks(void)
{
    static int8_t cnt;

    while (((int) ReadTimer0()) - tick < (int) SCAN_DELAY)
        ;
    tick = (int) ReadTimer0();
    if (++cnt & 1)
        return;

    /* Check if the IN endpoint is busy, and if it isn't check if we want to send
     * keystroke data to the host. */
    if (!HIDTxHandleBusy(keyboard.lastINTransmission)) {
        uint8_t* report = APP_KeyboardScan();
        if (report) {
            keyboard.lastINTransmission = HIDTxPacket(HID_EP, report, sizeof(inputReport));
        }
    }

    /* Check if any data was sent from the PC to the keyboard device.  Report
     * descriptor allows host to send 1 byte of data.  Bits 0-4 are LED states,
     * bits 5-7 are unused pad bits.  The host can potentially send this OUT
     * report data through the HID OUT endpoint (EP1 OUT), or, alternatively,
     * the host may try to send LED state information by sending a SET_REPORT
     * control transfer on EP0.  See the USBHIDCBSetReportHandler() function. */
    if (HIDRxHandleBusy(keyboard.lastOUTTransmission) == false)
        keyboard.lastOUTTransmission = HIDRxPacket(HID_EP,(uint8_t*)&outputReport,sizeof(outputReport));
}

void APP_KeyboardProcessOutputReport(void)
{
    APP_LEDUpdate(controlLED(outputReport.value));
}

void APP_Suspend()
{
    SYSTEM_Initialize(SYSTEM_STATE_USB_SUSPEND);
}

void APP_WakeFromSuspend()
{
    SYSTEM_Initialize(SYSTEM_STATE_USB_RESUME);
}

static void USBHIDCBSetReportComplete(void)
{
    /* 1 byte of LED state data should now be in the CtrlTrfData buffer.  Copy
     * it to the OUTPUT report buffer for processing */
    outputReport.value = CtrlTrfData[0];
}

void USBHIDCBSetReportHandler(void)
{
    /* Prepare to receive the keyboard LED state data through a SET_REPORT
     * control transfer on endpoint 0.  The host should only send 1 byte,
     * since this is all that the report descriptor allows it to send. */
    USBEP0Receive((uint8_t*)&CtrlTrfData, USB_EP0_BUFF_SIZE, USBHIDCBSetReportComplete);
}

/*******************************************************************************
 End of File
*/
