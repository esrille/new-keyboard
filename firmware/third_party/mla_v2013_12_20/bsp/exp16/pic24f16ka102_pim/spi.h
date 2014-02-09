/*******************************************************************************
  SPI Master Driver Header File

  Company:
    Microchip Technology Inc.

  File Name:
    spi.h

  Summary:
    SPI Master Driver Header File

  Description:
    This is the file that contains the driver for the SPI module
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
#ifndef _spi_h_
#define _spi_h_

#define SPIM_PIC24
#define SPIM_BLOCKING_FUNCTION
#define SPIM_MODE16 1
#define SPIM_MODE8 0
#define SPIM_SMP 1
#define SPIM_CKE 0
#define SPIM_CKP 1
#define SPIM_PPRE (unsigned)0
#define SPIM_SPRE (unsigned)0


/*******************************************************************************
*                                                                       
* This section defines names of control registers of SPI Module.        
* Names depends of processor type and module number.                    
*                                                                       
*******************************************************************************/

    #define SPIBUF  SPI1BUF
    #define SPISTAT SPI1STAT
    #define SPIBUFbits  SPI1BUFbits
    #define SPISTATbits SPI1STATbits
    #define SPIINTEN IEC0 
    #define SPIINTFLG IFS0
    #define SPIINTENbits IEC0bits
    #define SPIINTFLGbits IFS0bits
    #define SPIIF SPI1IF
    #define SPIIE SPI1IE
    #define SPICON SPI1CON1
    #define SPICONbits SPI1CON1bits
    #define SPICON2 SPI1CON2
    #define SPICON2bits SPI1CON2bits


/******************************************************************************
* Error and Status Flags                                                
* SPIM_STS_WRITE_COLLISION - indicates that Write collision has occurred
* while trying to transmit the byte.                                    
*                                                                           
* SPIM_STS_TRANSMIT_NOT_OVER - indicates that the transmission is
* not yet over. This is to be checked only when non-blocking
* option is opted.                                                      
*                                                                           
* SPIM_STS_DATA_NOT_READY - indicates that reception SPI buffer is empty
* and there's no data avalable yet.                                      
*                                                                           
*******************************************************************************/
#define SPIM_STS_WRITE_COLLISION    1
#define SPIM_STS_TRANSMIT_NOT_OVER  2  
#define SPIM_STS_DATA_NOT_READY     3  

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
extern void SPI_Initialize();
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
extern unsigned int  SPI_PutData(unsigned int Data);
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
extern unsigned int SPI_Transmit_IsComplete();

#endif
