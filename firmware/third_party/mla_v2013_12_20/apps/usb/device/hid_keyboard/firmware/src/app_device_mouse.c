/*
 * Copyright 2015, 2016 Esrille Inc.
 *
 * This file is a modified version of app_device_mouse.c provided by
 * Microchip Technology, Inc. for using Esrille New Keyboard.
 * See the file NOTICE for copying permission.
 */

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

#ifdef ENABLE_MOUSE

/** INCLUDES *******************************************************/
#include <system.h>

#include <stdint.h>

#include <usb/usb.h>
#include <usb/usb_device.h>
#include <usb/usb_device_hid.h>

#include <app_led_usb_status.h>
#include <app_device_mouse.h>
#include <usb_config.h>

#include <Mouse.h>

/*******************************************************************************
 * HID Report Descriptor - this describes the data format of the reports that
 * are sent between the host and the device.
 *
 * In this example there are only one INPUT report.  This report descriptor can
 * be generated using the HID descriptor tool available at www.usb.org.
 ******************************************************************************/
const struct{uint8_t report[HID_RPT02_SIZE];}hid_rpt02=
{
    {
        0x05, 0x01, /* Usage Page (Generic Desktop)             */
        0x09, 0x02, /* Usage (Mouse)                            */
        0xA1, 0x01, /* Collection (Application)                 */
        0x09, 0x01, /*  Usage (Pointer)                         */
        0xA1, 0x00, /*  Collection (Physical)                   */
        0x05, 0x09, /*      Usage Page (Buttons)                */
        0x19, 0x01, /*      Usage Minimum (01)                  */
        0x29, 0x05, /*      Usage Maximum (05)                  */
        0x15, 0x00, /*      Logical Minimum (0)                 */
        0x25, 0x01, /*      Logical Maximum (1)                 */
        0x95, 0x05, /*      Report Count (5)                    */
        0x75, 0x01, /*      Report Size (1)                     */
        0x81, 0x02, /*      Input (Data, Variable, Absolute)    */
        0x95, 0x01, /*      Report Count (1)                    */
        0x75, 0x03, /*      Report Size (3)                     */
        0x81, 0x01, /*      Input (Constant)    ;3 bit padding  */
        0x05, 0x01, /*      Usage Page (Generic Desktop)        */
        0x09, 0x30, /*      Usage (X)                           */
        0x09, 0x31, /*      Usage (Y)                           */
        0x09, 0x38, /*      Usage (Wheel)                       */
        0x15, 0x81, /*      Logical Minimum (-127)              */
        0x25, 0x7F, /*      Logical Maximum (127)               */
        0x75, 0x08, /*      Report Size (8)                     */
        0x95, 0x03, /*      Report Count (3)                    */
        0x81, 0x06, /*      Input (Data, Variable, Relative)    */
        0xC0, 0xC0  /* End Collection,End Collection            */
    }
};

/*******************************************************************************
 * Report Data Types - These typedefs will mirror the reports defined in the
 * HID report descriptor so that the application has an easy way to see what
 * the report will look like as well as access/modify each of the members of the
 * report.
 ******************************************************************************/

/* INPUT report - this structure will represent the only INPUT report in the HID
 * descriptor.
 */
typedef struct __attribute__((packed))
{
    /* The first INPUT item is the following:
     *   0x05, 0x09,    //Usage Page (Buttons)
     *   0x19, 0x01,    //Usage Minimum (01)
     *   0x29, 0x03,    //Usage Maximum (03)
     *   0x15, 0x00,    //Logical Minimum (0)
     *   0x25, 0x01,    //Logical Maximum (1)
     *   0x95, 0x03,    //Report Count (3)
     *   0x75, 0x01,    //Report Size (1)
     *   0x81, 0x02,    //Input (Data, Variable, Absolute)
     *
     * The usage page is buttons
     * The report size is 1 (1-bit)
     * The report count is 3, thus 3 1-bit items
     * The Usage Min is 1 and the Usage maximum is 5, thus buttons 1-5, also
     *   call the left, right, middle, back and forward buttons.
     *
     * The second INPUT item comes from the fact that the report must be byte
     * aligned, so we need to pad the previous 3-bit report with 5-bits of
     * constant(filler) data.
     *   0x95, 0x01,    //Report Count (1)
     *   0x75, 0x05,    //Report Size (5)
     *   0x81, 0x01,    //Input (Constant)
     */
    union __attribute__((packed))
    {
        struct __attribute__((packed))
        {
            unsigned button1   :1;
            unsigned button2   :1;
            unsigned button3   :1;
            unsigned button4   :1;
            unsigned button5   :1;
            unsigned :3;
        };
        struct __attribute__((packed))
        {
            unsigned left   :1;
            unsigned right  :1;
            unsigned middle :1;
            unsigned back   :1;
            unsigned forward:1;
            unsigned :3;
        };
        uint8_t value;
    } buttons;

    /* The final INPUT item is the following:
     *   0x05, 0x01,    //Usage Page (Generic Desktop)
     *   0x09, 0x30,    //Usage (X)
     *   0x09, 0x31,    //Usage (Y)
     *   0x15, 0x81,    //Logical Minimum (-127)
     *   0x25, 0x7F,    //Logical Maximum (127)
     *   0x75, 0x08,    //Report Size (8)
     *   0x95, 0x02,    //Report Count (2)
     *   0x81, 0x06,    //Input (Data, Variable, Relative)
     *
     * The report size is 8 (8-bit)
     * The report count is 2, thus 2 bytes of data.
     * The first usage is (X) and the second is (Y) so the first byte will
     *   represent the X mouse value, and the second the Y value.
     * The logical min/max determines the bounds for X and Y, -127 to 127.
     * The INPUT type is relative so each report item is relative to the last
     *   report item.  So reporting "-1" for X means that since the last report
     *   was sent, the mouse has move left
     */
    uint8_t x;
    uint8_t y;
    int8_t wheel;
} MOUSE_REPORT;

/** VARIABLES ******************************************************/
/* Some processors have a limited range of RAM addresses where the USB module
 * is able to access.  The following section is for those devices.  This section
 * assigns the buffers that need to be used by the USB module into those
 * specific areas.
 */
#if defined(FIXED_ADDRESS_MEMORY)
    #if defined(COMPILER_MPLAB_C18)
        #pragma udata MOUSE_REPORT_DATA_BUFFER=MOUSE_REPORT_DATA_BUFFER_ADDRESS
            static MOUSE_REPORT mouseReport;
        #pragma udata
    #elif defined(__XC8)
        static MOUSE_REPORT mouseReport @ MOUSE_REPORT_DATA_BUFFER_ADDRESS;
    #endif
#else
    static MOUSE_REPORT mouseReport;
#endif

typedef struct
{
    USB_HANDLE lastINTransmission;
} MOUSE;

static MOUSE mouse;

/*********************************************************************
* Function: void APP_DeviceMouseInitialize(void);
*
* Overview: Initializes the demo code
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceMouseInitialize(void)
{
#if APP_MACHINE_VALUE != 0x4550
    PPSUnLock();
    // Support TSAP via UART2
    // Set RP7 as RX2
    iPPSInput(IN_FN_PPS_RX2DT2, IN_PIN_PPS_RP7);
    PPSLock();

    // Initialize USART (9600bps: 1249, 38400bps: 312)
    //   Note ignore CPDIV here; see "4. Module: EUSART (Receive Baud Rate)" in
    //   "PIC18F47J53 Family Silicon Errata and Data Sheet Clarification" for more detail.
    baud2USART(BAUD_IDLE_RX_PIN_STATE_HIGH & BAUD_IDLE_TX_PIN_STATE_HIGH & BAUD_16_BIT_RATE & BAUD_WAKEUP_OFF & BAUD_AUTO_OFF);
    Open2USART(USART_TX_INT_OFF & USART_RX_INT_ON & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, 312);
    IPR3bits.RC2IP = 1;     // High priority
#endif

    INTCONbits.PEIE = 1;    // Enable peripheral interrupt
    INTCONbits.GIE = 1;     // Enable global interrupt

    /* initialize the handles to invalid so we know they aren't being used. */
    mouse.lastINTransmission = NULL;

    //enable the HID endpoint
    USBEnableEndpoint(HID_MOUSE_EP,USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
}//end UserInit

/*********************************************************************
* Function: void APP_DeviceMouseTasks(void);
*
* Overview: Keeps the demo running.
*
* PreCondition: The demo should have been initialized and started via
*   the APP_DeviceMouseInitialize() and APP_DeviceMouseStart() demos
*   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceMouseTasks(void)
{
    /* Do not report unchanged state.
     */
    if (mouseReport.buttons.value == getKeyboardMouseButtons() &&
        mouseReport.x == 0 && mouseReport.x == getKeyboardMouseX() &&
        mouseReport.y == 0 && mouseReport.y == getKeyboardMouseY() &&
        mouseReport.wheel == 0 && mouseReport.wheel == getKeyboardMouseWheel())
    {
        return;
    }

    /* We can only send a report if the last report has been sent.
     */
    if(HIDTxHandleBusy(mouse.lastINTransmission) == false)
    {
        mouseReport.buttons.value = getKeyboardMouseButtons();
        mouseReport.x = getKeyboardMouseX();
        mouseReport.y = getKeyboardMouseY();
        mouseReport.wheel = getKeyboardMouseWheel();
        mouse.lastINTransmission = HIDTxPacket(HID_MOUSE_EP, (uint8_t*) &mouseReport, sizeof mouseReport);
    }
}//end ProcessIO

#endif
