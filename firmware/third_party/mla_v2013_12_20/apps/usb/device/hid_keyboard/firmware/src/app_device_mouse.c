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

/** INCLUDES *******************************************************/
#include <system.h>

#include <stdint.h>

#include <usb/usb.h>
#include <usb/usb_device.h>
#include <usb/usb_device_hid.h>

#include <app_led_usb_status.h>
#include <app_device_mouse.h>
#include <usb_config.h>

/*******************************************************************************
 * HID Report Descriptor - this describes the data format of the reports that
 * are sent between the host and the device.
 *
 * In this example there are only one INPUT report.  This report descriptor can
 * be generated using the HID descriptor tool available at www.usb.org.
 ******************************************************************************/
const struct{uint8_t report[HID_RPT01_SIZE];}hid_rpt01=
{
    {
        0x05, 0x01, /* Usage Page (Generic Desktop)             */
        0x09, 0x02, /* Usage (Mouse)                            */
        0xA1, 0x01, /* Collection (Application)                 */
        0x09, 0x01, /*  Usage (Pointer)                         */
        0xA1, 0x00, /*  Collection (Physical)                   */
        0x05, 0x09, /*      Usage Page (Buttons)                */
        0x19, 0x01, /*      Usage Minimum (01)                  */
        0x29, 0x03, /*      Usage Maximum (03)                  */
        0x15, 0x00, /*      Logical Minimum (0)                 */
        0x25, 0x01, /*      Logical Maximum (1)                 */
        0x95, 0x03, /*      Report Count (3)                    */
        0x75, 0x01, /*      Report Size (1)                     */
        0x81, 0x02, /*      Input (Data, Variable, Absolute)    */
        0x95, 0x01, /*      Report Count (1)                    */
        0x75, 0x05, /*      Report Size (5)                     */
        0x81, 0x01, /*      Input (Constant)    ;5 bit padding  */
        0x05, 0x01, /*      Usage Page (Generic Desktop)        */
        0x09, 0x30, /*      Usage (X)                           */
        0x09, 0x31, /*      Usage (Y)                           */
        0x15, 0x81, /*      Logical Minimum (-127)              */
        0x25, 0x7F, /*      Logical Maximum (127)               */
        0x75, 0x08, /*      Report Size (8)                     */
        0x95, 0x02, /*      Report Count (2)                    */
        0x81, 0x06, /*      Input (Data, Variable, Relative)    */
        0xC0, 0xC0  /* End Collection,End Collection            */
    }
};

/*******************************************************************************
 * Report Data Types - These typedefs will mirror the reports defined in the
 * HID report descriptor so that the applicaiton has an easy way to see what
 * the report will look like as well as access/modify each of the members of the
 * report.
 ******************************************************************************/

/* INPUT report - this structure will represet the only INPUT report in the HID
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
     * The Usage Min is 1 and the Usage maximum is 3, thus buttons 1-3, also
     *   call the primary, secondary, and tertiary buttons.
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
            unsigned :5;
        };
        struct __attribute__((packed))
        {
            unsigned primary   :1;
            unsigned secondary :1;
            unsigned tertiary  :1;
            unsigned :5;
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

#define MOUSE_MOVEMENT_DISTANCE 400

static const uint8_t xVector[]={ -1,  0, 1, 0};
static const uint8_t yVector[]={  0, -1, 0, 1};

typedef struct
{
    bool sentStop;
    bool lastButtonState;
    uint8_t vectorPosition;
    uint16_t movementCount;
    bool movementMode;
    
    struct
    {
        USB_HANDLE handle;
        uint8_t idleRate;
        uint8_t idleRateSofCount;
    } inputReport[1];

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
    /* Enable emulation mode.  This means that the mouse data
     * will be send to the PC causing the mouse to move.  If this is
     * set to false then the demo board will send 0,0,0 resulting
     * in no mouse movement.
     */
    mouse.movementMode = true;
    
    /* initialize the handles to invalid so we know they aren't being used. */
    mouse.inputReport[0].handle = NULL;

    mouse.sentStop = false;

    mouse.movementCount = 0;

    //enable the HID endpoint
    USBEnableEndpoint(HID_EP,USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
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
    bool currentButtonState;

    /* Get the current button state */
    currentButtonState = BUTTON_IsPressed(BUTTON_USB_DEVICE_HID_MOUSE);

    /* If the button state has changed since the last time we checked it, then
     * we enable/disable movement mode.
     */
    if(mouse.lastButtonState != currentButtonState)
    {
        mouse.lastButtonState = currentButtonState;

        if(currentButtonState == false)
        {
            mouse.movementMode = !mouse.movementMode;

            mouse.inputReport[0].idleRateSofCount = 0;
            mouse.sentStop = false;
        }
    }
}//end ProcessIO

void APP_DeviceMouseIdleRateCallback(uint8_t reportId, uint8_t idleRate)
{
    mouse.inputReport[reportId].idleRate = idleRate;
}

/*******************************************************************************
 * Function: void APP_DeviceMouseSOFHandler(void)
 *
 * Overview: Handles SOF events.  This is used to calculate the mouse movement
 *           based on the SOF counter instead of a device timer or CPU clocks.
 *           It can also be used to handle idle rate issues, if applicable for
 *           the demo.
 *
 * Input: none
 * Output: none
 *
 ******************************************************************************/
void APP_DeviceMouseSOFHandler(void)
{
    /* We will be getting SOF packets before we get the SET_CONFIGURATION
     * packet that will configure this device, thus, we need to make sure that
     * we are actually initialized and open before we do anything else,
     * otherwise we should exit the function without doing anything.
     */
    if(USBGetDeviceState() != CONFIGURED_STATE)
    {
        return;
    }

    /* On each SOF, which should happen nearly every millisecond, check to see
     * if our idle duration has expired.  If it has, then we need to send a
     * repeat report.
     *
     * Note here that the idle rate duration specified by the host is as
     * follows:
     *
     * if the duration == 0, then it is indefinate duration
     * if the duration != 0, then it specifies the number of 4ms blocks between
     *   duplicate reports.  Thus an idle rate duration of 1 repeats reports
     *   every 4ms even if none of the data has changed.
     *
     * Here are the cases where we are suppose to transmit data:
     * 1) The report data has changed since the last time we sent a report (any
     *    movement or button press/release in the case of a mouse).
     * 2) The idle rate duration counter has expired and there hasn't been any
     *    change in data, then we are are going resend the same data.  Note here
     *    that if the idle rate duration = 0, then it is infinite so you would
     *    thus never send a duplicate report.
     */

    /* We can only send a report if the last report has been sent.
     */
    if(HIDTxHandleBusy(mouse.inputReport[0].handle) == false)
    {
        /* We are going to move a certain number of pixels in each direction.
         * The last transmission was successful so let's count it now.
         */
        mouse.movementCount++;

        /* Once we've reached the specified number of pixels moved, we'll move
         * to the next direction in the position array.
         */
        if(mouse.movementCount == MOUSE_MOVEMENT_DISTANCE)
        {
            mouse.movementCount = 0;

            mouse.vectorPosition++;
            if(mouse.vectorPosition >= sizeof(xVector))
            {
                mouse.vectorPosition = 0;
            }
        }

        /* Here we will provide a way for the application to disable the mouse
         * mouse momement by pressing a button.  If emulateMode == true, then we
         * are emulating the mouse, if == false, then make it so there is no
         * movement.
         */
        if(mouse.movementMode == true)
        {
            mouseReport.buttons.button1 = 0;
            mouseReport.buttons.button2 = 0;
            mouseReport.buttons.button3 = 0;

            mouseReport.x = xVector[mouse.vectorPosition];
            mouseReport.y = yVector[mouse.vectorPosition];
        }
        else
        {
            mouseReport.buttons.value = 0;
            mouseReport.x = 0;
            mouseReport.y = 0;

            if(mouse.inputReport[0].idleRate != 0)
            {
                mouse.inputReport[0].idleRateSofCount++;
            }
            
            /* if we haven't sent this "stop" packet yet, then we need to send
             * it regardless of the current idle count because it represents a
             * change in value.  If we have already sent the "stop" then we
             * need to check the idle rate counter before we send it again.
             */
            if(mouse.sentStop == true)
            {
                /* If we have haven't exceeded the idle rate yet, then we don't
                 * need to send a repeat report.  This needs to be "<=" because
                 * both the count and the rate can be set to 0 if the host
                 * selects an indefiniate duration.
                 */
                if(mouse.inputReport[0].idleRateSofCount <= mouse.inputReport[0].idleRate)
                {
                    return;
                }
            }
        }
        
        mouse.inputReport[0].handle = HIDTxPacket(
                                                    HID_EP,
                                                    (uint8_t*)&mouseReport,
                                                    sizeof(mouseReport)
                                                 );
    }
}