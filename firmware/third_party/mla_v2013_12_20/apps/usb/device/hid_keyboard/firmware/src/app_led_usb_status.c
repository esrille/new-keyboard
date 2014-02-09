/*******************************************************************************
  USB Status Indicator LED

  Company:
    Microchip Technology Inc.

  File Name:
    led_usb_status.c

  Summary:
    Indicates the USB device status to the user via an LED.

  Description:
    Indicates the USB device status to the user via an LED.
    * The LED is turned off for suspend mode.
    * It blinks quickly with 50% on time when configured
    * It blinks slowly at a low on time (~5% on, 95% off) for all other states.
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
#include <stdint.h>
#include <system.h>
#include <usb/usb_device.h>


// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Constants
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data Types
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Macros or Functions
// *****************************************************************************
// *****************************************************************************

void APP_LEDUpdateUSBStatus(void)
{
    static uint16_t ledCount = 0;

    if(USBIsDeviceSuspended() == true)
    {
        LED_Off(LED_USB_DEVICE_STATE);
        return;
    }

    switch(USBGetDeviceState())
    {         
        case CONFIGURED_STATE:
            /* We are configured.  Blink fast.
             * On for 75ms, off for 75ms, then reset/repeat. */
            if(ledCount == 1)
            {
                LED_On(LED_USB_DEVICE_STATE);
            }
            else if(ledCount == 75)
            {
                LED_Off(LED_USB_DEVICE_STATE);
            }
            else if(ledCount > 150)
            {
                ledCount = 0;
            }
            break;

        default:
            /* We aren't configured yet, but we aren't suspended so let's blink with
             * a slow pulse. On for 50ms, then off for 950ms, then reset/repeat. */
            if(ledCount == 1)
            {
                LED_On(LED_USB_DEVICE_STATE);
            }
            else if(ledCount == 50)
            {
                LED_Off(LED_USB_DEVICE_STATE);
            }
            else if(ledCount > 950)
            {
                ledCount = 0;
            }
            break;
    }

    /* Increment the millisecond counter. */
    ledCount++;
}

/*******************************************************************************
 End of File
*/
