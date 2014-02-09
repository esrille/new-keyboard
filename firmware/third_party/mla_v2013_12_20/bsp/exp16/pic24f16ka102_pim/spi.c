/*******************************************************************************
  SPI Master Driver File

  Company:
    Microchip Technology Inc.

  File Name:
    spi.c

  Summary:
    SPI Master Driver File

  Description:
    This is the file that contains the driver for the SPI module which implements
    the SPI as a Master in the polled mode
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

#include <xc.h>
#include "spi.h"
/*******************************************************************************
 * Function: void SPI_Initialize(void)                                                 
 * 
 * Summary:
 *   Function to initialize the SPI as master in polled mode
 *
 * Description:
 *   This function will initialize the SPI module as master in polled mode
 *
 * Input: None
 *                                                                       
 * Output: None                                                          
 *                                                                       
 ******************************************************************************/
void SPI_Initialize ( )
{
    SPISTAT = 0 ;
    SPICON = ( SPIM_PPRE | SPIM_SPRE ) ;
    SPICONbits.MSTEN = 1 ;
    SPICON2 = 0 ;
    SPICONbits.MODE16 = SPIM_MODE8 ;
    SPICONbits.CKE = SPIM_CKE ;
    SPICONbits.CKP = SPIM_CKP ;
    SPICONbits.SMP = SPIM_SMP ;
    SPIINTENbits.SPIIE = 0 ;
    SPIINTFLGbits.SPIIF = 0 ;
    SPISTATbits.SPIEN = 1 ;
}
/*******************************************************************************
 * Function: unsigned int SPI_PutData(unsigned int)
 *
 * Summary:
 *   Function to send data
 *
 * Description:
 *   In the non-blocking option, this function sends the byte over SPI bus and
 *   checks for Write Collision. In the blocking option, this function waits
 *   for a free transmission buffer.
 *
 * Preconditions:
 *   'SPI_Initialize' should have been called.
 *
 * Input:
 *   None
 *
 * Output:
 *   This function returns ‘0’  on proper initialization of transmission and
 *   ‘SPIM_STS_WRITE_COLLISION’ on occurrence of the Write Collision error.
 *
 ******************************************************************************/
unsigned int SPI_PutData ( unsigned int Data )
{
#ifndef SPIM_BLOCKING_FUNCTION

    if (SPISTATbits.SPITBF)
        return SPIM_STS_WRITE_COLLISION ;
    SPI1BUF = Data ;
    return 0 ;

#else

    // Wait for a data byte reception
    while (SPISTATbits.SPITBF) ;
    SPI1BUF = Data ;
    SPI1STATbits.SPIROV = 0 ;
    IFS0bits.SPI1IF = 0 ;
    Data = SPI1BUF ;

    return 0 ;

#endif
}
/*******************************************************************************
 * Function: unsigned int SPI_Transmit_IsComplete(void)
 *
 * Summary:
 *   Function to check if transmission of data is complete
 *
 * Description:
 *   In the non-blocking option, this function checks whether the transmission
 *   of the byte is completed. In the blocking option, this function waits till
 *   the transmission of the byte is completed.
 *
 * Preconditions:
 *   ‘SPI_PutData’ should have been called.
 *
 * Input:
 *    None
 *
 * Output:
 *    This function returns nothing in the blocking option and returns ‘0’ in the
 *    non-blocking option. It returns SPIM_STS_TRANSMIT_NOT_OVER if the
 *    transmission is not yet over.
 *
 ******************************************************************************/
unsigned SPI_Transmit_IsComplete ( )
{
#ifndef  SPIM_BLOCKING_FUNCTION

    if (SPISTATbits.SPIRBF == 0)
        return SPIM_STS_TRANSMIT_NOT_OVER ;
    return 0 ;

#else

    // Wait for a data byte reception

    while (SPISTATbits.SPIRBF == 0) ;
    return 0 ;

#endif
}


