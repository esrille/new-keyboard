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
// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler Select (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler Ratio Select (1:128)
#pragma config WINDIS = OFF             // Windowed WDT Disable (Standard Watchdog Timer)
#pragma config FWDTEN = WDT_DIS         // Watchdog Timer Enable (WDT disabled in hardware; SWDTEN bit disabled)
#pragma config ICS = PGx3               // Emulator Pin Placement Select bits (Emulator functions are shared with PGEC3/PGED3)
#pragma config LPCFG = OFF              // Low power regulator control (Disabled - regardless of RETEN)
#pragma config GWRP = OFF               // General Segment Write Protect (Disabled)
#pragma config GCP = OFF                // General Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (Disabled)

// CONFIG2
#pragma config POSCMD = HS              // Primary Oscillator Select (HS Oscillator Enabled)
#pragma config WDTCLK = LPRC            // WDT Clock Source Select bits (WDT uses LPRC)
#ifdef OSCIOFCN_ON
#pragma config OSCIOFCN=ON              // OSCO/CLKO/RC15 functions as port I/O (RC15)
#else
#pragma config OSCIOFNC=ON              // OSCO/CLKO/RC15 functions as port I/O (RC15)
#endif
#pragma config FCKSM = CSDCMD           // Clock Switching and Fail-Safe Clock Monitor Configuration bits (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = PRIPLL           // Initial Oscillator Select (Primary Oscillator with PLL module (XTPLL,HSPLL, ECPLL))
#pragma config ALTADREF = AVREF_RA      // External 12-Bit A/D Reference Location Select bit (AVREF+/AVREF- are mapped to RA9/RA10)
#pragma config ALTCVREF = CVREF_RA      // External Comparator Reference Location Select bit (CVREF+/CVREF- are mapped to RA9/RA10)
#pragma config WDTCMX = WDTCLK          // WDT Clock Source Select bits (WDT clock source is determined by the WDTCLK Configuration bits)
#pragma config IESO = OFF               // Internal External Switchover (Disabled)

// CONFIG3
#pragma config WPFP = WPFP127           // Write Protection Flash Page Segment Boundary (Page 127 (0x1FC00))
#pragma config SOSCSEL = ON             // SOSC Selection bits (SOSC circuit selected)
#pragma config WDTWIN = PS25_0          // Window Mode Watchdog Timer Window Width Select (Watch Dog Timer Window Width is 25 percent)
#pragma config BOREN = OFF              // Brown-out Reset Enable (Brown-out Reset Disabled)
#pragma config WPDIS = WPDIS            // Segment Write Protection Disable (Disabled)
#pragma config WPCFG = WPCFGDIS         // Write Protect Configuration Page Select (Disabled)
#pragma config WPEND = WPENDMEM         // Segment Write Protection End Page Select (Write Protect from WPFP to the last page of memory)

// CONFIG4
#pragma config DSWDTPS = DSWDTPS1F      // Deep Sleep Watchdog Timer Postscale Select bits (1:68719476736 (25.7 Days))
#pragma config DSWDTOSC = LPRC          // DSWDT Reference Clock Select (DSWDT uses LPRC as reference clock)
#pragma config DSBOREN = OFF            // Deep Sleep BOR Enable bit (DSBOR Disabled)
#pragma config DSWDTEN = OFF            // Deep Sleep Watchdog Timer Enable (DSWDT Disabled)
#pragma config DSSWEN = OFF             // DSEN Bit Enable (Deep Sleep operation is always disabled)
#pragma config RTCBAT = OFF             // RTC Battery Operation Enable (RTC operation is discontinued in VBAT)
#pragma config PLLDIV = DIV2            // PLL Input Prescaler Select bits (Oscillator divided by 2 (8 MHz input))
#pragma config I2C2SEL = PRI            // Alternate I2C2 Location Select bit (I2C2 is multiplexed to SDA2/RA3 and SCL2/RA2 )
#pragma config IOL1WAY = ON             // PPS IOLOCK Set Only Once Enable bit (Once set, the IOLOCK bit cannot be cleared)

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

#if defined(USB_INTERRUPT)
void __attribute__((interrupt,auto_psv)) _USB1Interrupt()
{
    USBDeviceTasks();
}
#endif