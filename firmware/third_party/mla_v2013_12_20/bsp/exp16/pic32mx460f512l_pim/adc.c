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

#include <adc.h>
#include <stdint.h>
#include <stdbool.h>
#include <p32xxxx.h>

#include <adc.h>

#define PIN_ANALOG      0
#define PIN_DIGITAL     1

#define PIN_INPUT       1
#define PIN_OUTPUT      0

/*********************************************************************
* Function: ADC_ReadPercentage(ADC_CHANNEL channel);
*
* Overview: Reads the requested ADC channel and returns the percentage 
*            of that conversions result (0-100%).
*
* PreCondition: channel is configured via the ADCConfigure() function
*
* Input: ADC_CHANNEL channel - enumeration of the ADC channels
*        available in this demo.  They should be meaningful names and 
*        not the names of the ADC pins on the device (as the demo code 
*        may be ported to other boards).
*         i.e. ADC_ReadPercentage(ADC_CHANNEL_POTENTIOMETER);
*
* Output: uint8_t indicating the percentage of the result 0-100% or
*         0xFF for an error
*
********************************************************************/
uint8_t ADC_ReadPercentage
    (ADC_CHANNEL channel) {
    uint8_t percent;

    switch(channel)
    {
        case ADC_CHANNEL_2:
            break;
        default:
            return 0xFF;
    }
    
    //A very crude percentage calculation
    percent = (ADC_Read10bit(channel) / 10);

    if(percent > 100)
    {
        percent = 100;
    }
    return percent;
}

/*********************************************************************
* Function: ADC_Read10bit(ADC_CHANNEL channel);
*
* Overview: Reads the requested ADC channel and returns the 10-bit
*           representation of this data.
*
* PreCondition: channel is configured via the ADCConfigure() function
*
* Input: ADC_CHANNEL channel - enumeration of the ADC channels
*        available in this demo.  They should be meaningful names and
*        not the names of the ADC pins on the device (as the demo code
*        may be ported to other boards).
*         i.e. - ADCReadPercentage(ADC_CHANNEL_POTENTIOMETER);
*
* Output: uint16_t the right adjusted 10-bit representation of the ADC
*         channel conversion or 0xFFFF for an error.
*
********************************************************************/
uint16_t ADC_Read10bit(ADC_CHANNEL channel)
{
    uint16_t i;
    
    switch(channel)
    {
        case ADC_CHANNEL_2:
            break;
        default:
            return 0xFFFF;
    }
	
    AD1CON1 = 0x0000; // SAMP bit = 0 ends sampling ...
    // and starts converting
    AD1CHS = channel; // Connect RB2/AN2 as CH0 input ..
    // in this example RB2/AN2 is the input
    AD1CSSL = 0;
    AD1CON3 = 0x0002; // Manual Sample, Tad = internal 6 TPB
    AD1CON2 = 0;
    AD1CON1SET = 0x8000; // turn ADC ON

    AD1CON1SET = 0x0002; // start sampling ...
    for(i=0;i<1000;i++); //Sample delay, conversion start automatically
    AD1CON1CLR = 0x0002; // start Converting
    while (!(AD1CON1 & 0x0001));// conversion done?

    return ADC1BUF0;
}

/*********************************************************************
* Function: bool ADC_Enable(ADC_CHANNEL channel, ADC_CONFIGURATION configuration);
*
* Overview: Configures the ADC module to specified setting
*
* PreCondition: none
*
* Input: ADC_CHANNEL channel - the channel to enable
*        ADC_CONFIGURATION configuration - the mode in which to run the ADC
*
* Output: bool - true if successfully configured.  false otherwise.
*
********************************************************************/
bool ADC_Enable(ADC_CHANNEL channel)
{
    switch(channel)
    {
        case ADC_CHANNEL_2:
            AD1PCFGbits.PCFG2 = PIN_ANALOG;
            return true;

        default:
            return false;
    }
}

/*********************************************************************
* Function: bool ADC_SetConfiguration(ADC_CONFIGURATION configuration)
*
* Overview: Configures the ADC module to specified setting
*
* PreCondition: none
*
* Input: ADC_CONFIGURATION configuration - the mode in which to run the ADC
*
* Output: bool - true if successfully configured.  false otherwise.
*
********************************************************************/
bool ADC_SetConfiguration(ADC_CONFIGURATION configuration)
{
    if(configuration == ADC_CONFIGURATION_DEFAULT)
    {
        AD1CON2bits.VCFG = 0x0;
        AD1CON3bits.ADCS = 0xFF;
        AD1CON1bits.SSRC = 0x0;
        AD1CON3bits.SAMC = 0x10;    
        AD1CON1bits.FORM = 0x0;
        AD1CON2bits.SMPI = 0x0;
        AD1CON1bits.ADON = 1;
        
        return true;
    }

    return false;
}
