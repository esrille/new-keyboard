/*
 * Copyright 2014-2016 Esrille Inc.
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
#include <stdio.h>
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

// PIC18F47J53 Configuration Bit Settings

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG1L
#pragma config WDTEN = OFF      // Watchdog Timer (Disabled - Controlled by SWDTEN bit)
#pragma config PLLDIV = 1       // PLL Prescaler Selection (No prescale (4 MHz oscillator input drives PLL directly))
#pragma config CFGPLLEN = ON    // PLL Enable Configuration Bit (PLL Enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset (Enabled)
#pragma config XINST = OFF      // Extended Instruction Set (Disabled)

// CONFIG1H
#pragma config CPUDIV = OSC1    // CPU System Clock Postscaler (No CPU system clock divide)
#pragma config CP0 = OFF        // Code Protect (Program memory is not code-protected)

// CONFIG2L
#pragma config OSC = HSPLL      // Oscillator (HS+PLL, USB-HS+PLL)
#pragma config SOSCSEL = LOW    // T1OSC/SOSC Power Selection Bits (Low Power T1OSC/SOSC circuit selected)
#pragma config CLKOEC = OFF     // EC Clock Out Enable Bit  (CLKO output disabled on the RA6 pin)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor (Disabled)
#pragma config IESO = OFF       // Internal External Oscillator Switch Over Mode (Disabled)

// CONFIG2H
#pragma config WDTPS = 4        // Watchdog Postscaler (1:4)

// CONFIG3L
#pragma config DSWDTOSC = INTOSCREF// DSWDT Clock Select (DSWDT uses INTRC)
#pragma config RTCOSC = INTOSCREF// RTCC Clock Select (RTCC uses INTRC)
#pragma config DSBOREN = OFF    // Deep Sleep BOR (Disabled)
#pragma config DSWDTEN = OFF    // Deep Sleep Watchdog Timer (Disabled)
#pragma config DSWDTPS = 8192   // Deep Sleep Watchdog Postscaler (1:8,192 (8.5 seconds))

// CONFIG3H
#pragma config IOL1WAY = OFF    // IOLOCK One-Way Set Enable bit (The IOLOCK bit (PPSCON<0>) can be set and cleared as needed)
#pragma config ADCSEL = BIT10   // ADC 10 or 12 Bit Select (10 - Bit ADC Enabled)
#pragma config MSSP7B_EN = MSK7 // MSSP address masking (7 Bit address masking mode)

// CONFIG4L
#pragma config WPFP = PAGE_1    // Write/Erase Protect Page Start/End Location (Write Protect Program Flash Page 1)
#pragma config WPCFG = OFF      // Write/Erase Protect Configuration Region  (Configuration Words page not erase/write-protected)

// CONFIG4H
#pragma config WPDIS = OFF      // Write Protect Disable bit (WPFP<6:0>/WPEND region ignored)
#pragma config WPEND = PAGE_0   // Write/Erase Protect Region Select bit (valid when WPDIS = 0) (Pages 0 through WPFP<6:0> erase/write protected)
#pragma config LS48MHZ = SYS48X8// Low Speed USB mode with 48 MHz system clock bit (System clock at 48 MHz USB CLKEN divide-by is set to 8)

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data Types
// *****************************************************************************
// *****************************************************************************

const unsigned int VersionWord @ APP_VERSION_ADDRESS = APP_VERSION_VALUE;
const unsigned int MachineWord @ APP_MACHINE_ADDRESS = APP_MACHINE_VALUE;


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
            //In this devices family of USB microcontrollers, the PLL will not power up and be enabled
            //by default, even if a PLL enabled oscillator configuration is selected (such as HS+PLL).
            //This allows the device to power up at a lower initial operating frequency, which can be
            //advantageous when powered from a source which is not guaranteed to be adequate for 48MHz
            //operation.  On these devices, user firmware needs to manually set the OSCTUNE<PLLEN> bit to
            //power up the PLL.
            {
                OSCTUNEbits.PLLEN = 1;  //Enable the PLL and wait 2+ms until the PLL locks before enabling USB module
                __delay_ms(2);
            }
            //Device switches over automatically to PLL output after PLL is locked and ready.

            INTCONbits.PEIE = 0;    // Disable peripheral interrupt
            INTCONbits.GIE = 0;     // Disable global interrupt

            OSCCONbits.IDLEN = 0;   // Disable idle mode

            // Configure as  digital ports
            ANCON0 = 0xff;
            ANCON1 = 0x1f;

            // Initialize all of the LED pins and key matrix ports
            // PORT A
            //   RA0: R0 (output by default)
            //   RA1: R1 (output by default)
            //   RA2: R2 (output by default)
            //   RA3: R3 (output by default)
            //   -
            //   RA5: R4 (output by default)
            //   CLKO: (output)
            //   CLKI: (input)
            LATA = 0x00;
            TRISA = 0x80;

            // PORT B
            //   RB0: C2 (output by default)
            //   RB1: C3 (output by default)
            //   RB2: C4 (output by default)
            //   RB3: C5 (output by default)
            //   RB4: RX2 (input: configured later)
            //   RB5: C11 (output by default)
            //   RB6: C10 (output by default)
            //   RB7: C9 (output by default)
            LATB = 0x00;
            TRISB = 0x10;

            // PORT C
            //   RC0: L1 (output: 0)
            //   RC1: L2 (output: 0)
            //   RC2: L3 (output: 0)
            //   -
            //   RC4: D- (-)
            //   RC5: D+ (-)
            //   RC6: SCK (input: configured later)
            //   RC7: MISO (input: configured later)
            LATC = 0x00;
            TRISC = 0xF0;

            // PORT D
            //   RD0: USB Sense (input)
            //   RD1: C6 (output by default)
            //   RD2: C7 (output by default)
            //   RD3: C8 (output by default)
            //   RD4: MOSI (input: configured later)
            //   RD5: CS (output: 1)
            //   RD6: C0 (output by default)
            //   RD7: C1 (output by default)
            LATD = 0x20;
            TRISD = 0x11;

            // PORT E
            //   RE0: R5 (output by default)
            //   RE1: R6 (output by default)
            //   RE2: R7 (output by default)
            //   -
            //   -
            //   -
            //   REPU: disable (0)
            //   RDPU: disable (0)
            LATE = 0x00;
            TRISE = 0x00;

            BUTTON_Disable();

            InitNvram();
            initKeyboard();

#ifndef WITH_HOS
            // Initialize USART (9600bps: 1249, 38400bps: 312)
            //   Note ignore CPDIV here; see "4. Module: EUSART (Receive Baud Rate)" in
            //   "PIC18F47J53 Family Silicon Errata and Data Sheet Clarification" for more detail.
            baud1USART(BAUD_IDLE_RX_PIN_STATE_HIGH & BAUD_IDLE_TX_PIN_STATE_HIGH & BAUD_16_BIT_RATE & BAUD_WAKEUP_OFF & BAUD_AUTO_OFF);
            Open1USART(USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, 312);
#else   // WITH_HOS
            HosInitialize();
#if defined(DEBUG) && !defined(ENABLE_MOUSE)
            PPSUnLock();
            // Set RP7 as TX2 (only for debugging without TSAP)
            iPPSOutput(OUT_PIN_PPS_RP7, OUT_FN_PPS_TX2CK2);
            PPSLock();

            // Initialize USART (9600bps: 1249, 38400bps: 312)
            //   Note ignore CPDIV here; see "4. Module: EUSART (Receive Baud Rate)" in
            //   "PIC18F47J53 Family Silicon Errata and Data Sheet Clarification" for more detail.
            baud2USART(BAUD_IDLE_RX_PIN_STATE_HIGH & BAUD_IDLE_TX_PIN_STATE_HIGH & BAUD_16_BIT_RATE & BAUD_WAKEUP_OFF & BAUD_AUTO_OFF);
            Open2USART(USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, 312);
#endif
#endif  // WITH_HOS

#ifdef ENABLE_MOUSE
            initMouse();
#endif
            break;

        case SYSTEM_STATE_USB_SUSPEND:
            OSCCON = 0x13;      // Sleep on sleep, 125kHz selected as microcontroller clock source
            break;

        case SYSTEM_STATE_USB_RESUME:
            OSCCON = 0x60;      // Primary clock source selected.

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
    if (DataRdy2USART()) {
        if (RCSTA2bits.OERR || RCSTA2bits.FERR) {
            RCREG2 = 0;
            Read2USART();   // Clear FERR
            RCSTA2 = 0;     // Clear OERR
            RCSTA2 = 0x90;  // Restart USARTs
        } else {
            uint8_t data = Read2USART();    // Clear FERR
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


uint8_t isBusPowered(void)
{
    return PORTDbits.RD0;
}


int8_t isUSBMode(void)
{
    return CurrentProfile() == 0;
}


/*******************************************************************************
 End of File
*/
