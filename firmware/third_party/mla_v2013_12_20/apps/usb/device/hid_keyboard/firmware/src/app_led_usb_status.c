/*
 * Copyright 2014 Esrille Inc.
 *
 * This file is a modified version of app_led_usb_status.c provided by
 * Microchip Technology, Inc. for using Esrille New Keyboard.
 * See the Software License Agreement below for the License.
 */

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

#include "app_device_keyboard.h"

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
    if(USBIsDeviceSuspended())
    {
        LED_Off(LED_USB_DEVICE_HID_KEYBOARD_NUM_LOCK);
        LED_Off(LED_USB_DEVICE_HID_KEYBOARD_CAPS_LOCK);
        LED_Off(LED_USB_DEVICE_HID_KEYBOARD_SCROLL_LOCK);
        return;
    }

    switch(USBGetDeviceState())
    {         
        case CONFIGURED_STATE:
            APP_KeyboardProcessOutputReport();
            break;

        default:
            LED_On(LED_USB_DEVICE_HID_KEYBOARD_NUM_LOCK);
            LED_On(LED_USB_DEVICE_HID_KEYBOARD_CAPS_LOCK);
            LED_On(LED_USB_DEVICE_HID_KEYBOARD_SCROLL_LOCK);
            break;
    }
}

/*******************************************************************************
 End of File
*/
