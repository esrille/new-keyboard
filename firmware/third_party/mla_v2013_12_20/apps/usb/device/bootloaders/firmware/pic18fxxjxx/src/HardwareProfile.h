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
#if defined(__18F46J50)
    #if !defined(PIC18F_STARTER_KIT)    //Defined in the build configuration when the PIC18F_Starter_Kit_XC8 build config is selected
        #define PIC18F46J50_PIM
    #endif
#elif defined(__18F47J53)
    #define PIC18F47J53_PIM
#elif defined(__18F87J50)
    #define PIC18F87J50_FS_USB_PIM
#elif defined(__18F87J94)
    #define PIC18F87J94_FS_USB_PIM
#elif defined(__18F97J94)
    #define PIC18F97J94_FS_USB_PIM
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




#if defined(PIC18F46J50_PIM) || defined(PIC18F47J53_PIM)
    //VBUS sensing pin definition, applicable if using the USE_USB_BUS_SENSE_IO option in usb_config.
    #if defined(USE_USB_BUS_SENSE_IO)
        // Bus sense pin is RB5 on PIC18F46J50/PIC18F47J53 FS USB Plug-In Module.
        // Must put jumper JP1 in R-U position
        #define tris_usb_bus_sense  TRISCbits.TRISC2    // Input
        #define usb_bus_sense       PORTCbits.RC2
    #endif
    #if defined(USE_SELF_POWER_SENSE_IO)
        #define tris_self_power     TRISXbits.TRISXX    //Replace with real value if your hardware supports this capbility
        #define self_power          PORTXbits.RXX       //Replace with real value if your hardware supports this capbility
    #endif

    //LED definition, applicable if using ENABLE_USB_LED_BLINK_STATUS option in usb_config.h
    #define mLED1       LATEbits.LATE1
    #define mLED1Tris   TRISEbits.TRISE1
    /** SWITCH *********************************************************/
    #define mInitSwitch2()      {ANCON1bits.PCFG8 = 1;}   
    #define sw2                 PORTBbits.RB2
    #define mDeInitSwitch2()    {ANCON1bits.PCFG8 = 0;}


#elif defined(PIC18F_STARTER_KIT)
    //VBUS sensing pin definition, applicable if using the USE_USB_BUS_SENSE_IO option in usb_config.
    #if defined(USE_USB_BUS_SENSE_IO)
        #define tris_usb_bus_sense  TRISXbits.TRISXX    //Replace with real value if your hardware supports this capbility
        #define usb_bus_sense       PORTXbits.RXX       //Replace with real value if your hardware supports this capbility
    #endif
    #if defined(USE_SELF_POWER_SENSE_IO)
        #define tris_self_power     TRISXbits.TRISXX    //Replace with real value if your hardware supports this capbility
        #define self_power          PORTXbits.RXX       //Replace with real value if your hardware supports this capbility
    #endif

    //LED definition, applicable if using ENABLE_USB_LED_BLINK_STATUS option in usb_config.h
    #if defined(ENABLE_USB_LED_BLINK_STATUS)
        #undef ENABLE_USB_LED_BLINK_STATUS      //No general purpose LED available on this particular demo board
    #endif
    /** SWITCH *********************************************************/
    #define mInitSwitch2()      {ANCON1bits.PCFG12 = 1;}
    #define sw2                 PORTBbits.RB0
    #define mDeInitSwitch2()    {ANCON1bits.PCFG12 = 0;}


#elif defined(PIC18F87J50_FS_USB_PIM)
    //VBUS sensing pin definition, applicable if using the USE_USB_BUS_SENSE_IO option in usb_config.
    #if defined(USE_USB_BUS_SENSE_IO)
        // Bus sense pin is RB5 on PIC18F87J50 FS USB Plug-In Module.
        // Must put jumper JP1 in R-U position
        #define tris_usb_bus_sense  TRISBbits.TRISB5    // Input
        #define usb_bus_sense       PORTBbits.RB5
    #endif
    //Self powered sensing pin definition, applicable if using the USE_SELF_POWER_SENSE_IO option in usb_config.
    #if defined(USE_SELF_POWER_SENSE_IO)
        #define tris_self_power     TRISXbits.TRISXX    //Replace with real value if your hardware supports this capbility
        #define self_power          PORTXbits.RXX       //Replace with real value if your hardware supports this capbility
    #endif

    //LED definition, applicable if using ENABLE_USB_LED_BLINK_STATUS option in usb_config.h
    #define mLED1       LATEbits.LATE1
    #define mLED1Tris   TRISEbits.TRISE1
    /** SWITCH *********************************************************/
    #define mInitSwitch2()
    #define sw2                 PORTBbits.RB4
    #define mDeInitSwitch2()


#elif defined(PIC18F87J94_FS_USB_PIM)
    //VBUS sensing pin definition, applicable if using the USE_USB_BUS_SENSE_IO option in usb_config.
    #if defined(USE_USB_BUS_SENSE_IO)
        // Bus sense pin is RB5 on PIC18F87J94 FS USB Plug-In Module.
        // Must put jumper JP1 in R-U position
        #define tris_usb_bus_sense  TRISBbits.TRISB5    // Input
        #define usb_bus_sense       PORTBbits.RB5
    #endif
    #if defined(USE_SELF_POWER_SENSE_IO)
        #define tris_self_power     TRISXbits.TRISXX    //Replace with real value if your hardware supports this capbility
        #define self_power          PORTXbits.RXX       //Replace with real value if your hardware supports this capbility
    #endif
    
    //LED definition, applicable if using ENABLE_USB_LED_BLINK_STATUS option in usb_config.h
    #define mLED1       LATEbits.LATE1
    #define mLED1Tris   TRISEbits.TRISE1
    /** S W I T C H *****************************************************/
    #define mInitSwitch2()      
    #define sw2                 PORTBbits.RB4
    #define mDeInitSwitch2()    


#elif defined(PIC18F97J94_FS_USB_PIM)
    //VBUS sensing pin definition, applicable if using the USE_USB_BUS_SENSE_IO option in usb_config.
    #if defined(USE_USB_BUS_SENSE_IO)
        #define tris_usb_bus_sense  TRISXbits.TRISXX    //Replace with real value if your hardware supports this capbility
        #define usb_bus_sense       PORTXbits.RXX       //Replace with real value if your hardware supports this capbility
    #endif
    #if defined(USE_SELF_POWER_SENSE_IO)
        #define tris_self_power     TRISXbits.TRISXX    //Replace with real value if your hardware supports this capbility
        #define self_power          PORTXbits.RXX       //Replace with real value if your hardware supports this capbility
    #endif

    //LED definition, applicable if using ENABLE_USB_LED_BLINK_STATUS option in usb_config.h
    #define mLED1       LATBbits.LATB0
    #define mLED1Tris   TRISBbits.TRISB0
    /** SWITCH *********************************************************/
    #define mInitSwitch2()
    #define sw2                 PORTGbits.RG7
    #define mDeInitSwitch2()


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
