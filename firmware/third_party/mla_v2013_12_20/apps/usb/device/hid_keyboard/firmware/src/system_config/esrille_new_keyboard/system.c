/*
 * Copyright 2014, 2015 Esrille Inc.
 *
 * This file is a modified version of system.c provided by
 * Microchip Technology, Inc. for using Esrille New Keyboard.
 * See the file NOTICE for copying permission.
 */

/*******************************************************************************
  System Initialization and Controls (PICDEM FS USB Demo Board)

  Company:
    Microchip Technology Inc.

  File Name:
    system.c

  Summary:
    This file initializes and controls system level features.

  Description:
    This file initializes and controls system level features.
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
#include <plib/usart.h>
#include <usb/usb_device.h>

#include <app_device_mouse.h>

#include <Keyboard.h>
#include <Mouse.h>

// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Constants
// *****************************************************************************
// *****************************************************************************
#pragma config PLLDIV   = 1         // 4 MHz resonator
#pragma config CPUDIV   = OSC3_PLL4 // USB Low Speed
#pragma config USBDIV   = 2         // Clock source from 96MHz PLL/2
#pragma config FOSC     = HSPLL_HS
#pragma config FCMEN    = OFF
#pragma config IESO     = OFF
#pragma config PWRT     = OFF
#pragma config BOR      = ON
#pragma config BORV     = 3
#pragma config VREGEN   = ON      //USB Voltage Regulator
#pragma config WDT      = OFF
#pragma config WDTPS    = 32768
#pragma config MCLRE    = ON
#pragma config LPT1OSC  = OFF
#pragma config PBADEN   = OFF
//#pragma config CCP2MX   = ON
#pragma config STVREN   = ON
#pragma config LVP      = OFF
//#pragma config ICPRT    = OFF       // Dedicated In-Circuit Debug/Programming
#pragma config XINST    = OFF       // Extended Instruction Set
#pragma config CP0      = OFF
#pragma config CP1      = OFF
//#pragma config CP2      = OFF
//#pragma config CP3      = OFF
#pragma config CPB      = OFF
//#pragma config CPD      = OFF
#pragma config WRT0     = OFF
#pragma config WRT1     = OFF
//#pragma config WRT2     = OFF
//#pragma config WRT3     = OFF
#pragma config WRTB     = OFF       // Boot Block Write Protection
#pragma config WRTC     = OFF
//#pragma config WRTD     = OFF
#pragma config EBTR0    = OFF
#pragma config EBTR1    = OFF
//#pragma config EBTR2    = OFF
//#pragma config EBTR3    = OFF
#pragma config EBTRB    = OFF


// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data Types
// *****************************************************************************
// *****************************************************************************

const unsigned int VersionWord @ APP_VERSION_ADDRESS = APP_VERSION_VALUE;

static unsigned char trisD = 0xfc;  // 0~1, output; 2~7, input
static unsigned char trisE = 0x03;  // 0~1, input initially

// *****************************************************************************
// *****************************************************************************
// Section: Macros or Functions
// *****************************************************************************
// *****************************************************************************
void SYSTEM_Initialize( SYSTEM_STATE state )
{
    switch(state)
    {
        case SYSTEM_STATE_USB_START:
            CCP1CON = 0;
            CCP2CON = 0;
            ADCON1 = 0x0F; // Digital I/O

            if (3 <= BOARD_REV_VALUE) {
                ADCON0 = 0x00;
                ADCON1 = 0x0E;  // Enable AN0
                ADCON2 = 0x9E;  // 6 Tad, Fosc/64 10 011 110
                if (BOARD_REV_VALUE == 3)
                    trisD = 0xfe;   // 0, output; 1~7, input
                else
                    trisD = 0xf3;   // 2,3, output; 0, 1, 4~7, input
                trisE = 0x07;   // 0~2, input initially
            }

            //Initialize all of the LED pins and key matrix ports
            // PORT A (0~5, input initially)
            LATA = 0x00;
            TRISA = 0x3F;

            // PORT B (0~7, input)
            LATB = 0x00;
            TRISB = 0xFF;
            INTCON2bits.RBPU = 0;   // Enable pull up

            // PORT C (0-2, 6-7 output)
            LATC = 0x00;
            TRISC = 0x38;

            // PORT D
            LATD = 0x00;
            TRISD = trisD;
            PORTEbits.RDPU = 1;     // Enable pull up

            // PORT E
            LATE = 0x00;
            TRISE = trisE;

            initKeyboard();

#ifdef ENABLE_MOUSE
            // Initialize USART (9600bps: 624, 38400bps: 155)
            baudUSART(BAUD_IDLE_RX_PIN_STATE_HIGH & BAUD_IDLE_TX_PIN_STATE_HIGH & BAUD_16_BIT_RATE & BAUD_WAKEUP_OFF & BAUD_AUTO_OFF);
            OpenUSART(USART_TX_INT_OFF & USART_RX_INT_ON & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, 155);
            INTCONbits.PEIE = 1;    // Enable peripheral interrupt
            INTCONbits.GIE = 1;     // Enable global interrupt
            initMouse();
#endif
            break;

        case SYSTEM_STATE_USB_SUSPEND:
            OSCCON = 0x13;	//Sleep on sleep, 125kHz selected as microcontroller clock source
            break;

        case SYSTEM_STATE_USB_RESUME:
            OSCCON = 0x60;      //Primary clock source selected.

            //Adding a software start up delay will ensure
            //that the primary oscillator and PLL are running before executing any other
            //code.  If the PLL isn't being used, (ex: primary osc = 48MHz externally applied EC)
            //then this code adds a small unnecessary delay, but it is harmless to execute anyway.
            __delay_ms(2);
            break;
    }
}

#if defined(__XC8)
void interrupt SYS_InterruptHigh(void)
{
#if defined(USB_INTERRUPT)
    USBDeviceTasks();
#endif

#ifdef ENABLE_MOUSE
    if (DataRdyUSART()) {
        if (RCSTAbits.OERR || RCSTAbits.FERR) {
            ReadUSART();    // Clear FERR
            RCSTA = 0;      // Clear OERR
            RCSTA = 0x90;   // Restart USARTs
        } else {
            uint8_t data = ReadUSART();    // Clear FERR

            /* We will be getting data before we get the SET_CONFIGURATION
             * packet that will configure this device, thus, we need to make sure that
             * we are actually initialized and open before we do anything else,
             * otherwise we should exit the function without doing anything.
             */
            if (USBGetDeviceState() == CONFIGURED_STATE && processSerialUnit(data)) {
                APP_DeviceMouseTasks();
            }
        }
    }
#endif
}
#endif

/*******************************************************************************
 End of File
*/
