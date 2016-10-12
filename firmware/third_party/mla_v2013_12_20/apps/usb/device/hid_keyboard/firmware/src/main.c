/*
 * Copyright 2014-2016 Esrille Inc.
 *
 * This file is a modified version of main.c provided by
 * Microchip Technology, Inc. for using Esrille New Keyboard.
 * See the file NOTICE for copying permission.
 */

/*******************************************************************************
  Main Demo File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    Main demo file for keyboard project.  Entry point for compiler.

  Description:
    Main demo file for keyboard project.  Entry point for compiler.  Also
    calls the system initailization and handles system issues like calling the
    demo code appropriately and handing device level events from the USB stack.
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
/* Standard C includes */
#include <stdint.h>
#include <stdio.h>

/* Microchip library includes */
#include <system.h>
#include <system_config.h>

#include <usb/usb.h>
#include <usb/usb_device_hid.h>

#include "app_led_usb_status.h"
#include "app_device_keyboard.h"
#include "app_device_mouse.h"

#include <Keyboard.h>

#ifdef ENABLE_MOUSE
#include <Mouse.h>
#endif


// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Constants
// *****************************************************************************
// *****************************************************************************
static void USBCBSendResume(void);

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

int main(void)
{
    SYSTEM_Initialize(SYSTEM_STATE_USB_START);
    LED_Initialize();
    APP_KeyboardConfigure();

#ifdef WITH_HOS
    HosCheckDFU(BOOT_FLAGS_VALUE & BOOT_WITH_APP);
    if (!isUSBMode() || !isBusPowered()) {
        HosMainLoop();
    }
    for (uint16_t i = 0; i < HOS_STARTUP_DELAY; ++i) {
        if (HosSleep(HOS_TYPE_DEFAULT)) {
            break;
        }
        __delay_ms(4);
    }
#endif

    USBDeviceInit();
    USBDeviceAttach();

    for (;;)
    {
#ifdef WITH_HOS
        if (!isBusPowered() || !isUSBMode()) {
            Reset();
            Nop();
            Nop();
            // NOT REACHED HERE
        }
#endif

        SYSTEM_Tasks();

#if defined(USB_POLLING)
        /* Check bus status and service USB interrupts.  Interrupt or polling
         * method.  If using polling, must call this function periodically.
         * This function will take care of processing and responding to SETUP
         * transactions (such as during the enumeration process when you first
         * plug in).  USB hosts require that USB devices should accept and
         * process SETUP packets in a timely fashion.  Therefore, when using
         * polling, this function should be called regularly (such as once every
         * 1.8ms or faster** [see inline code comments in usb_device.c for
         * explanation when "or faster" applies])  In most cases, the
         * USBDeviceTasks() function does not take very long to execute
         * (ex: <100 instruction cycles) before it returns. */
        USBDeviceTasks();
#endif

        APP_LEDUpdateUSBStatus();

        /* If the USB device isn't configured yet, we can't really do anything
         * else since we don't have a host to talk to.  So jump back to the
         * top of the while loop. */
        if (USBGetDeviceState() < CONFIGURED_STATE)
        {
            /* Jump back to the top of the while loop. */
            continue;
        }

        /* If we are currently suspended, then we need to see if we need to
         * issue a remote wakeup.  In either case, we shouldn't process any
         * keyboard commands since we aren't currently communicating to the host
         * thus just continue back to the start of the while loop. */
        if (USBIsDeviceSuspended())
        {
            //Check if we should assert a remote wakeup request to the USB host,
            //when the user presses the pushbutton.
            if (BUTTON_IsPressed())
            {
                USBCBSendResume();  //Does nothing unless we are in USB suspend with remote wakeup armed.
            }

            /* Jump back to the top of the while loop. */
            continue;
        }

        if (USBIsBusSuspended())
        {
            /* Jump back to the top of the while loop. */
            continue;
        }

        /* Run the keyboard tasks. */
        APP_KeyboardTasks();
    }//end while
}//end main

/********************************************************************
 * Function:        void USBCBSendResume(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        The USB specifications allow some types of USB
 * 					peripheral devices to wake up a host PC (such
 *					as if it is in a low power suspend to RAM state).
 *					This can be a very useful feature in some
 *					USB applications, such as an Infrared remote
 *					control	receiver.  If a user presses the "power"
 *					button on a remote control, it is nice that the
 *					IR receiver can detect this signalling, and then
 *					send a USB "command" to the PC to wake up.
 *
 *					The USBCBSendResume() "callback" function is used
 *					to send this special USB signalling which wakes
 *					up the PC.  This function may be called by
 *					application firmware to wake up the PC.  This
 *					function will only be able to wake up the host if
 *                  all of the below are true:
 *
 *					1.  The USB driver used on the host PC supports
 *						the remote wakeup capability.
 *					2.  The USB configuration descriptor indicates
 *						the device is remote wakeup capable in the
 *						bmAttributes field.
 *					3.  The USB host PC is currently sleeping,
 *						and has previously sent your device a SET
 *						FEATURE setup packet which "armed" the
 *						remote wakeup capability.
 *
 *                  If the host has not armed the device to perform remote wakeup,
 *                  then this function will return without actually performing a
 *                  remote wakeup sequence.  This is the required behavior,
 *                  as a USB device that has not been armed to perform remote
 *                  wakeup must not drive remote wakeup signalling onto the bus;
 *                  doing so will cause USB compliance testing failure.
 *
 *					This callback should send a RESUME signal that
 *                  has the period of 1-15ms.
 *
 * Note:            This function does nothing and returns quickly, if the USB
 *                  bus and host are not in a suspended condition, or are
 *                  otherwise not in a remote wakeup ready state.  Therefore, it
 *                  is safe to optionally call this function regularly, ex:
 *                  anytime application stimulus occurs, as the function will
 *                  have no effect, until the bus really is in a state ready
 *                  to accept remote wakeup.
 *
 *                  When this function executes, it may perform clock switching,
 *                  depending upon the application specific code in
 *                  USBCBWakeFromSuspend().  This is needed, since the USB
 *                  bus will no longer be suspended by the time this function
 *                  returns.  Therefore, the USB module will need to be ready
 *                  to receive traffic from the host.
 *
 *                  The modifiable section in this routine may be changed
 *                  to meet the application needs. Current implementation
 *                  temporary blocks other functions from executing for a
 *                  period of ~3-15 ms depending on the core frequency.
 *
 *                  According to USB 2.0 specification section 7.1.7.7,
 *                  "The remote wakeup device must hold the resume signaling
 *                  for at least 1 ms but for no more than 15 ms."
 *                  The idea here is to use a delay counter loop, using a
 *                  common value that would work over a wide range of core
 *                  frequencies.
 *                  That value selected is 1800. See table below:
 *                  ==========================================================
 *                  Core Freq(MHz)      MIP         RESUME Signal Period (ms)
 *                  ==========================================================
 *                      48              12          1.05
 *                       4              1           12.6
 *                  ==========================================================
 *                  * These timing could be incorrect when using code
 *                    optimization or extended instruction mode,
 *                    or when having other interrupts enabled.
 *                    Make sure to verify using the MPLAB SIM's Stopwatch
 *                    and verify the actual signal on an oscilloscope.
 *******************************************************************/
static void USBCBSendResume(void)
{
    //First verify that the host has armed us to perform remote wakeup.
    //It does this by sending a SET_FEATURE request to enable remote wakeup,
    //usually just before the host goes to standby mode (note: it will only
    //send this SET_FEATURE request if the configuration descriptor declares
    //the device as remote wakeup capable, AND, if the feature is enabled
    //on the host (ex: on Windows based hosts, in the device manager
    //properties page for the USB device, power management tab, the
    //"Allow this device to bring the computer out of standby." checkbox
    //should be checked).
    if(USBGetRemoteWakeupStatus())
    {
        //Verify that the USB bus is in fact suspended, before we send
        //remote wakeup signalling.
        if(USBIsBusSuspended())
        {
            USBMaskInterrupts();

            //Clock switch to settings consistent with normal USB operation.
            APP_WakeFromSuspend();
            USBSuspendControl = 0;      //So we don't execute this code again,
                                        //until a new suspend condition is detected.

            //Section 7.1.7.7 of the USB 2.0 specifications indicates a USB
            //device must continuously see 5ms+ of idle on the bus, before it sends
            //remote wakeup signalling.  One way to be certain that this parameter
            //gets met, is to add a 2ms+ blocking delay here (2ms plus at
            //least 3ms from bus idle to USBIsBusSuspended() == TRUE, yeilds
            //5ms+ total delay since start of idle).
            __delay_ms(3);

            //Now drive the resume K-state signalling onto the USB bus.
            USBResumeControl = 1;       // Start RESUME signaling
            __delay_ms(2);              // Set RESUME line for 1-13 ms
            USBResumeControl = 0;       // Finished driving resume signalling

            USBUnmaskInterrupts();
        }
    }
}

bool USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, uint16_t size)
{
    switch((int)event)
    {
        case EVENT_TRANSFER:
            break;

        case EVENT_SOF:
            break;

        case EVENT_SUSPEND:
            APP_Suspend();
            break;

        case EVENT_RESUME:
            APP_WakeFromSuspend();
            break;

        case EVENT_CONFIGURED:
            /* When the device is configured, we can (re)initialize the keyboard
             * demo code. */
            APP_KeyboardInit();
#ifdef ENABLE_MOUSE
            APP_DeviceMouseInitialize();
#endif
            break;

        case EVENT_SET_DESCRIPTOR:
            break;

        case EVENT_EP0_REQUEST:
            /* We have received a non-standard USB request.  The HID driver
             * needs to check to see if the request was for it. */
            USBCheckHIDRequest();
            break;

        case EVENT_BUS_ERROR:
            break;

        case EVENT_TRANSFER_TERMINATED:
            break;

        default:
            break;
    }
    return true;
}

/*******************************************************************************
 End of File
*/
