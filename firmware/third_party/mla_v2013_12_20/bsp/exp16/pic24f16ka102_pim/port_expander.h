/*******************************************************************************
  Port Expander Driver Header File

  Company:
    Microchip Technology Inc.

  File Name:
    port_expander.h

  Summary:
    Port Expander Driver Header File

  Description:
    This is the header file that contains the driver for the port expander.
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


#define PORTEX_SS_PORT      PORTAbits.PORTA5
#define PORTEX_SS_TRIS      TRISAbits.TRISA5
#define PORTEX_SCK_TRIS     TRISBbits.TRISB11
#define PORTEX_SDO_TRIS     TRISBbits.TRISB13
#define PORTEX_SDI_TRIS     TRISBbits.TRISB10

#define PEXA        0x14    //Latch for port A of Port Expander
#define PEXB        0x15    //Latch for port B of Port Expander

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

void PortExpander_ByteWrite(unsigned char port, unsigned char data);

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
void PortExpander_Initialize ( void );




