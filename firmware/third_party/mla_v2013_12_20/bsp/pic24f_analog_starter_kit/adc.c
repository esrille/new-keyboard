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
#include <xc.h>
#include <adc.h>
#include <stdint.h>
#include <stdbool.h>

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
( ADC_CHANNEL channel )
{
    uint8_t percent ;

    switch (channel)
    {
        case ADC_CHANNEL_19:
            break ;
			
        default:
            return 0xFF ;
    }

    //A very crude percentage calculation
    percent = ( ADC_Read10bit ( channel ) / 10 ) ;

    if (percent > 100)
    {
        percent = 100 ;
    }
    return percent ;
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
uint16_t ADC_Read10bit ( ADC_CHANNEL channel )
{
    uint16_t result ;

    switch (channel)
    {
        case ADC_CHANNEL_19:
            break ;

        default:
            return 0xFFFF ;
    }

    ADTBL0bits.ADCH = channel ;

    IFS0bits.AD1IF = 0;

    ADL0CONLbits.SAMP = 0; // Start conversion.
    while(IFS0bits.AD1IF == 0);  //Wait for it to complete

    ADL0CONLbits.SAMP = 1; // Close the sample switch.

    result = ADRES0 >> 2;

    return result ;
}
/*********************************************************************
 * Function: bool ADC_ChannelEnable(ADC_CHANNEL channel, ADC_CONFIGURATION configuration);
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
bool ADC_ChannelEnable ( ADC_CHANNEL channel )
{
    switch (channel)
    {
        case ADC_CHANNEL_19:
            TRISGbits.TRISG8 = 1;
            ANSGbits.ANSG8 = 1;
            return true ;

        default:
            return false ;
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
bool ADC_SetConfiguration ( ADC_CONFIGURATION configuration )
{
    if (configuration == ADC_CONFIGURATION_DEFAULT)
    {
        // GLOBAL SETTINGS
        ADCON1=0;
        ADCON2=0;
        ADCON3=0;

        // Configure the A/D voltage references.
        ADCON2bits.PVCFG = 0; // Vref+ = AVdd
        ADCON2bits.NVCFG = 0; // Vref- = AVss

        // Configure the A/D clock.
        ADCON3bits.ADRC = 0; // Conversion clock derived from system clock.
        ADCON3bits.ADCS = 0b11111; //Max TAD

        // Configure buffer storage settings and interrupt generation.
        ADCON1bits.FORM = 0; // Output format is unsigned integer.
        ADCON2bits.BUFORG = 1; // Result buffer is an indexed buffer.

        // Configure power-saving.
        ADCON1bits.PWRLVL = 0; // Low power, reduced frequency operation.

        // Sample List SETTINGS
        ADL0CONL = 0;
        ADL0CONH = 0;
        ADL0CONLbits.SLSIZE = 0; // Sample list length: 1 channel.

        // Sampling settings.
        ADL0CONHbits.ASEN = 1; // Enable auto-scan.
        ADL0CONHbits.SLINT = 1; // Interrupt after auto-scan completion.
        ADL0CONHbits.SAMC = 0b11111; // Max TAD.
        ADL0CONLbits.SLTSRC = 0; // Single trigger generated when SAMP is cleared.
        ADL0PTR = 0; // Start from the first list entry.

        // Threshod compare settings.
        ADL0CONHbits.CM = 0; // Disable threshold compare.

        // ENABLE A/D
        ADCON1bits.ADON = 1; // Enable A/D.
        while(ADSTATHbits.ADREADY == 0); // Wait for ready flag set.

        ADCON1bits.ADCAL = 1; // Start calibration.
        while(ADSTATHbits.ADREADY == 0);

        ADL0CONLbits.SAMP = 1; // Close sample switch.
        ADL0CONLbits.SLEN = 1; // Enable sample list.
        return true ;
    }

    return false ;
}