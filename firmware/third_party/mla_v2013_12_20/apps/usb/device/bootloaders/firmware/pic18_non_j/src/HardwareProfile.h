/*
 * Copyright 2014 Esrille Inc.
 *
 * This file is a modified version of HardwareProfile.h provided by
 * Microchip Technology, Inc. for using Esrille New Keyboard.
 * See the Software License Agreement below for the License.
 */

/*********************************************************************
 * FileName:        HardwareProfile.h
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18
 * Compiler:        C18 3.45+ or XC8 v1.21+
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the "Company") for its PIC(R) Microcontroller is intended and
 * supplied to you, the Company's customer, for use solely and
 * exclusively on Microchip PIC Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ********************************************************************/

#ifndef __HARDWARE_PROFILE_H_
#define __HARDWARE_PROFILE_H_


/** I N C L U D E S *************************************************/
#include "usb_config.h"


//Make sure board/platform specific definitions (like config bit settings and
//I/O pin definitions are correct for your hardware platform).
#ifndef ESRILLE_NEW_KEYBOARD
#if defined(__18F4550)
    #define PIC18F4550_PICDEM_FS_USB
#elif defined(__18F45K50)
    #define PIC18F4550_PICDEM_FS_USB_K50
#elif defined(__18F14K50)
    #define LOW_PIN_COUNT_USB_DEVELOPMENT_KIT
#else
    #define YOUR_CUSTOM_BOARD
    #warning "You need to add platform specific settings for your hardware.  Double click this message for more details."
    //In order to use a hardware platform other than a Microchip USB demo board, you need to make
    //sure the following are correctly configured for your hardware platform:
    //1. Configuration bit settings (especially the oscillator settings, which must be compatible with USB operation).
    //2. I/O pin definitions for VBUS sensing, self power sensing, I/O pushbutton for entry into bootloader, and for LED blink settings.
    //3. Optional behavioral settings: ENABLE_IO_PIN_CHECK_BOOTLOADER_ENTRY, ENABLE_USB_LED_BLINK_STATUS, USE_SELF_POWER_SENSE_IO, USE_USB_BUS_SENSE_IO.  See usb_config.h file.
    //4. Oscillator and other settings are correctly being initialized in the InitializeSystem() function, specific to your hardware (ex: turn on PLL [if needed] for proper USB clock, etc.)
#endif
#endif


#if defined(PIC18F4550_PICDEM_FS_USB)   //Based on PIC18F4550, for PIC18F45K50 based board, see farther below
    //VBUS sensing pin definition, applicable if using the USE_USB_BUS_SENSE_IO option in usb_config.
    #if defined(USE_USB_BUS_SENSE_IO)
        #define tris_usb_bus_sense  TRISAbits.TRISA1    // Input
        #define usb_bus_sense       PORTAbits.RA1
    #endif
    #if defined(USE_SELF_POWER_SENSE_IO)
        #define tris_self_power     TRISAbits.TRISA2
        #define self_power          PORTAbits.RA2
    #endif

    //LED definition, applicable if using ENABLE_USB_LED_BLINK_STATUS option in usb_config.h
    #define mLED1       LATDbits.LATD0
    #define mLED1Tris   TRISDbits.TRISD0
    /** SWITCH *********************************************************/
    #define mInitSwitch2()      {ADCON1 = 0x0F; TRISBbits.TRISB4 = 1;}   
    #define sw2                 PORTBbits.RB4
    #define mDeInitSwitch2()    {ADCON1 = 0x07;} 



#elif defined(PIC18F4550_PICDEM_FS_USB_K50)  //Section for PIC18F45K50 when on the PICDEM FS USB Demo Board
    //VBUS sensing pin definition, applicable if using the USE_USB_BUS_SENSE_IO option in usb_config.
    #if defined(USE_USB_BUS_SENSE_IO)
        #define tris_usb_bus_sense  TRISAbits.TRISA1    // Input
        #define usb_bus_sense       PORTAbits.RA1
    #endif
    #if defined(USE_SELF_POWER_SENSE_IO)
        #define tris_self_power     TRISAbits.TRISA2
        #define self_power          PORTAbits.RA2       
    #endif

    //LED definition, applicable if using ENABLE_USB_LED_BLINK_STATUS option in usb_config.h
    #define mLED1       LATDbits.LATD0
    #define mLED1Tris   TRISDbits.TRISD0
    /** SWITCH *********************************************************/
    #define mInitSwitch2()      {ANSELBbits.ANSB4 = 0; TRISBbits.TRISB4 = 1;}   
    #define sw2                 PORTBbits.RB4
    #define mDeInitSwitch2()    {ANSELBbits.ANSB4 = 1;} 



#elif defined(LOW_PIN_COUNT_USB_DEVELOPMENT_KIT)  //Based on PIC18F14K50
    //VBUS sensing pin definition, applicable if using the USE_USB_BUS_SENSE_IO option in usb_config.
    #if defined(USE_USB_BUS_SENSE_IO)
        #define tris_usb_bus_sense  TRISAbits.TRISA1    // Input
        #define usb_bus_sense       PORTAbits.RA1
    #endif
    #if defined(USE_SELF_POWER_SENSE_IO)
        #define tris_self_power     TRISXbits.TRISXX    //Replace with real value if your hardware supports this feature
        #define self_power          PORTXbits.RXX       //Replace with real value if your hardware supports this feature
    #endif

    //LED definition, applicable if using ENABLE_USB_LED_BLINK_STATUS option in usb_config.h
    #define mLED1       LATCbits.LATC0
    #define mLED1Tris   TRISCbits.TRISC0
    /** SWITCH *********************************************************/
    #define mInitSwitch2()      {}                      //No TRISA3 bit.  RA3 is input only on this device
    #define sw2                 PORTAbits.RA3           //Requires MCLR disabled to use RA3 as general purpose input
    #define mDeInitSwitch2()    {} 
	
	

#elif defined(ESRILLE_NEW_KEYBOARD)
    //VBUS sensing pin definition, applicable if using the USE_USB_BUS_SENSE_IO option in usb_config.
    #if defined(USE_USB_BUS_SENSE_IO)
        #define tris_usb_bus_sense  TRISAbits.TRISA1    // Input
        #define usb_bus_sense       PORTAbits.RA1
    #endif
    #if defined(USE_SELF_POWER_SENSE_IO)
        #define tris_self_power     TRISAbits.TRISA2
        #define self_power          PORTAbits.RA2
    #endif

    //LED definition, applicable if using ENABLE_USB_LED_BLINK_STATUS option in usb_config.h
    #define mLED1       LATDbits.LATD1
    #define mLED1Tris   TRISDbits.TRISD1
    /** SWITCH *********************************************************/
#if ESRILLE_NEW_KEYBOARD == 1
    #define mInitSwitch2()      {ADCON1 = 0x0F; INTCON2bits.RBPU = 0; PORTEbits.RDPU = 1; PORTA &= 0xC0; PORTE &= 0xFC; TRISA |= 0x3F; TRISE |= 0x03; TRISAbits.TRISA5 = 0; TRISC = 0x38; LATC = 0xC7; TRISD = 0xFC; LATD = 3; }
#else
    #define mInitSwitch2()      {ADCON1 = 0x0F; INTCON2bits.RBPU = 0; PORTEbits.RDPU = 1; PORTA &= 0xC0; PORTE &= 0xFC; TRISA |= 0x3F; TRISE |= 0x03; TRISAbits.TRISA2 = 0; TRISC = 0x38; LATC = 0xC3; TRISD = 0xFC; LATD = 0; }
#endif
    #define sw2                 (PORTDbits.RD3)
    #define mDeInitSwitch2()    {ADCON1 = 0x07;}



#elif defined(YOUR_CUSTOM_BOARD)
    #warning "Add your hardware specific I/O pin mapping here."
    //Use one of the existing sections as a template for adding the proper
    //macro defintions appropriate for your hardware platform.

#else
    #error Not a supported board (yet), add I/O pin mapping in __FILE__, line __LINE__
#endif




/** T R I S *********************************************************/
#define INPUT_PIN           1
#define OUTPUT_PIN          0


#endif //__HARDWARE_PROFILE_H_
