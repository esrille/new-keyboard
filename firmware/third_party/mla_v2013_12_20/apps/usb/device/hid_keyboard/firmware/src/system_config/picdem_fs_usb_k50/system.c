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
 *******************************************************************/

#include <system.h>
#include <system_config.h>
#include <usb/usb.h>

/** CONFIGURATION Bits **********************************************/
#pragma config PLLSEL   = PLL3X     // 3X PLL multiplier selected
#pragma config CFGPLLEN = OFF       // PLL turned on during execution
#pragma config CPUDIV   = NOCLKDIV  // 1:1 mode (for 48MHz CPU)
#pragma config LS48MHZ  = SYS48X8   // Clock div / 8 in Low Speed USB mode
#pragma config FOSC     = INTOSCIO  // HFINTOSC selected at powerup, no clock out
#pragma config PCLKEN   = OFF       // Primary oscillator driver
#pragma config FCMEN    = OFF       // Fail safe clock monitor
#pragma config IESO     = OFF       // Internal/external switchover (two speed startup)
#pragma config nPWRTEN  = OFF       // Power up timer
#pragma config BOREN    = SBORDIS   // BOR enabled
#pragma config nLPBOR   = ON        // Low Power BOR
#pragma config WDTEN    = SWON      // Watchdog Timer controlled by SWDTEN
#pragma config WDTPS    = 32768     // WDT postscalar
#pragma config PBADEN   = OFF       // Port B Digital/Analog Powerup Behavior
#pragma config SDOMX    = RC7       // SDO function location
#pragma config LVP      = OFF       // Low voltage programming
#pragma config MCLRE    = ON        // MCLR function enabled (RE3 disabled)
#pragma config STVREN   = ON        // Stack overflow reset
//#pragma config ICPRT  = OFF       // Dedicated ICPORT program/debug pins enable
#pragma config XINST    = OFF       // Extended instruction set





/*********************************************************************
* Function: void SYSTEM_Initialize( SYSTEM_STATE state )
*
* Overview: Initializes the system.
*
* PreCondition: None
*
* Input:  SYSTEM_STATE - the state to initialize the system into
*
* Output: None
*
********************************************************************/
void SYSTEM_Initialize( SYSTEM_STATE state )
{
    switch(state)
    {
        case SYSTEM_STATE_USB_START:
		
            //Configure oscillator settings for clock settings compatible with USB
            //operation.  Note: Proper settings depends on USB speed (full or low).
            #if(USB_SPEED_OPTION == USB_FULL_SPEED)
                OSCTUNE = 0x80; //3X PLL ratio mode selected
                OSCCON = 0x70;  //Switch to 16MHz HFINTOSC
                OSCCON2 = 0x10; //Enable PLL, SOSC, PRI OSC drivers turned off
                while(OSCCON2bits.PLLRDY != 1);   //Wait for PLL lock
                ACTCON = 0x90;  //Enable active clock tuning for USB operation
            #endif
			
            LED_Enable(LED_USB_DEVICE_STATE);
            LED_Enable(LED_USB_DEVICE_HID_KEYBOARD_CAPS_LOCK);
            BUTTON_Enable(BUTTON_USB_DEVICE_HID_KEYBOARD_KEY);
            break;
			
        case SYSTEM_STATE_USB_SUSPEND: 
            break;
            
        case SYSTEM_STATE_USB_RESUME:
            break;
    }
}

			
			
#if defined(__XC8)
void interrupt SYS_InterruptHigh(void)
{
    #if defined(USB_INTERRUPT)
        USBDeviceTasks();
    #endif
}
#else
    void YourHighPriorityISRCode();
    void YourLowPriorityISRCode();

    //On PIC18 devices, addresses 0x00, 0x08, and 0x18 are used for
    //the reset, high priority interrupt, and low priority interrupt
    //vectors.  However, the current Microchip USB bootloader
    //examples are intended to occupy addresses 0x00-0x7FF or
    //0x00-0xFFF depending on which bootloader is used.  Therefore,
    //the bootloader code remaps these vectors to new locations
    //as indicated below.  This remapping is only necessary if you
    //wish to program the hex file generated from this project with
    //the USB bootloader.  If no bootloader is used, edit the
    //usb_config.h file and comment out the following defines:
    //#define PROGRAMMABLE_WITH_USB_HID_BOOTLOADER
    //#define PROGRAMMABLE_WITH_USB_LEGACY_CUSTOM_CLASS_BOOTLOADER

    #if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
            #define REMAPPED_RESET_VECTOR_ADDRESS			0x1000
            #define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x1008
            #define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x1018
    #elif defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
            #define REMAPPED_RESET_VECTOR_ADDRESS			0x800
            #define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x808
            #define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x818
    #else
            #define REMAPPED_RESET_VECTOR_ADDRESS			0x00
            #define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x08
            #define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x18
    #endif

    #if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
    extern void _startup (void);        // See c018i.c in your C18 compiler dir
    #pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS
    void _reset (void)
    {
        _asm goto _startup _endasm
    }
    #endif

    #pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
    void Remapped_High_ISR (void)
    {
         _asm goto YourHighPriorityISRCode _endasm
    }
    #pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
    void Remapped_Low_ISR (void)
    {
         _asm goto YourLowPriorityISRCode _endasm
    }

    #if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
    //Note: If this project is built while one of the bootloaders has
    //been defined, but then the output hex file is not programmed with
    //the bootloader, addresses 0x08 and 0x18 would end up programmed with 0xFFFF.
    //As a result, if an actual interrupt was enabled and occured, the PC would jump
    //to 0x08 (or 0x18) and would begin executing "0xFFFF" (unprogrammed space).  This
    //executes as nop instructions, but the PC would eventually reach the REMAPPED_RESET_VECTOR_ADDRESS
    //(0x1000 or 0x800, depending upon bootloader), and would execute the "goto _startup".  This
    //would effective reset the application.

    //To fix this situation, we should always deliberately place a
    //"goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS" at address 0x08, and a
    //"goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS" at address 0x18.  When the output
    //hex file of this project is programmed with the bootloader, these sections do not
    //get bootloaded (as they overlap the bootloader space).  If the output hex file is not
    //programmed using the bootloader, then the below goto instructions do get programmed,
    //and the hex file still works like normal.  The below section is only required to fix this
    //scenario.
    #pragma code HIGH_INTERRUPT_VECTOR = 0x08
    void High_ISR (void)
    {
         _asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
    }
    #pragma code LOW_INTERRUPT_VECTOR = 0x18
    void Low_ISR (void)
    {
         _asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
    }
    #endif	//end of "#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_LEGACY_CUSTOM_CLASS_BOOTLOADER)"

    #pragma code


    //These are your actual interrupt handling routines.
    #pragma interrupt YourHighPriorityISRCode
    void YourHighPriorityISRCode()
    {
            //Check which interrupt flag caused the interrupt.
            //Service the interrupt
            //Clear the interrupt flag
            //Etc.
    #if defined(USB_INTERRUPT)
            USBDeviceTasks();
    #endif

    }	//This return will be a "retfie fast", since this is in a #pragma interrupt section
    #pragma interruptlow YourLowPriorityISRCode
    void YourLowPriorityISRCode()
    {
            //Check which interrupt flag caused the interrupt.
            //Service the interrupt
            //Clear the interrupt flag
            //Etc.

    }	//This return will be a "retfie", since this is in a #pragma interruptlow section
#endif
