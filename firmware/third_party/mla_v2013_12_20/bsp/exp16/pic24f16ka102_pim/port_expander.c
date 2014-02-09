/*******************************************************************************
  Port Expander Driver File

  Company:
    Microchip Technology Inc.

  File Name:
    port_expander.c

  Summary:
    Port Expander Driver File

  Description:
    This is the file that contains the driver for the port expander.
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
#include <xc.h>

#define PEXA        0x14    //Latch for port A of Port Expander
#define PEXB        0x15    //Latch for port B of Port Expander
#define PEXIOCON    0x05    //IOCON Register of Port Expander
#define PEXDIRA     0x00    //Port Expander Direction Register A
#define PEXDIRB     0x01    //Port Expander Direction Register A
/*****************************************************************************
 * Function: SPI_Write
 *
 * Preconditions: None.
 *
 * Overview: This function handles SPI write to peripheral.
 *
 * Input: None.
 *
 * Output: None.
 *
 ******************************************************************************/
void SPI_Write ( unsigned char tempData )
{
    SPI1BUF = tempData ;                //shift the data into the SPI buffer
    while (!IFS0bits.SPI1IF)            //wait for transmission to complete
        tempData = SPI1BUF ;                //do a dummy read to clear buffer full flag
    IFS0bits.SPI1IF = 0 ;           //clear the SPI interrupt
}
/*****************************************************************************
 * Function: PortExpander_ByteWrite
 *
 * Preconditions: None.
 *
 * Overview: This function initiates one complete SPI transfer to Port Expander.
 *
 * Input: None.
 *
 * Output: None.
 *
 ******************************************************************************/
void PortExpander_ByteWrite ( unsigned char port , unsigned char data )
{
    PORTAbits.RA6 = 0 ;     //Set CS for Port Expander
    Nop ( ) ;
    SPI_Write ( 0x40 ) ;            //Write Command
    SPI_Write ( port ) ;            //Output port of Port Expander
    SPI_Write ( data ) ;            //Send the data
    PORTAbits.RA6 = 1 ;     //Return CS to signifiy end of transfer
    Nop ( ) ;
}
/*****************************************************************************
 * Function: PortExpander_Initialize
 *
 * Preconditions: None.
 *
 * Overview: This function initializes the Port Expander.
 *
 * Input: None.
 *
 * Output: None.
 *
 ******************************************************************************/

void PortExpander_Initialize ( void )
{
  
   // extern unsigned char PortEx_B_State ;
    //extern unsigned char PortEx_LED_State ;

    IFS0bits.SPI1IF = 0 ;
    TRISAbits.TRISA6 = 0 ;
    PORTAbits.RA6 = 1 ;     //set CS high for port expander
    Nop ( ) ;
    Nop ( ) ;

    PortExpander_ByteWrite ( PEXIOCON , 0x20 ) ;    //setup port expander
    PortExpander_ByteWrite ( PEXDIRA , 0x00 ) ; //set port A of expander to all outputs
    PortExpander_ByteWrite ( PEXDIRB , 0x00 ) ; //set port B of expander to all outputs
}

