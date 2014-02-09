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
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <system.h>
#include <string.h>
#include <timer_1ms.h>

/* Compiler checks and configuration *******************************/
#ifndef TIMER_MAX_1MS_CLIENTS
    #define TIMER_MAX_1MS_CLIENTS 1
#endif

/* Type Definitions ************************************************/
typedef struct
{
    TICK_HANDLER handle;
    uint32_t rate;
    uint32_t count;
} TICK_REQUEST;

/* Variables *******************************************************/
TICK_REQUEST requests[TIMER_MAX_1MS_CLIENTS];

/*********************************************************************
* Function: void TIMER_CancelTick(TICK_HANDLER handle)
*
* Overview: Cancels a tick request.
*
* PreCondition: None
*
* Input:  handle - the function that was handling the tick request
*
* Output: None
*
********************************************************************/
void TIMER_CancelTick(TICK_HANDLER handle)
{
    uint8_t i;

    for(i = 0; i < TIMER_MAX_1MS_CLIENTS; i++)
    {
        if(requests[i].handle == handle)
        {
            requests[i].handle = NULL;
        }
    }
}

/*********************************************************************
* Function: bool TIMER_RequestTick(TICK_HANDLER handle, uint32_t rate)
*
* Overview: Requests to receive a periodic event.
*
* PreCondition: None
*
* Input:  handle - the function that will be called when the time event occurs
*         rate - the number of ticks per event.
*
* Output: bool - true if successful, false if unsuccessful
*
********************************************************************/
bool TIMER_RequestTick(TICK_HANDLER handle, uint32_t rate)
{
    uint8_t i;

    for(i = 0; i < TIMER_MAX_1MS_CLIENTS; i++)
    {
        if(requests[i].handle == NULL)
        {
            requests[i].handle = handle;
            requests[i].rate = rate;
            requests[i].count = 0;

            return true;
        }
    }

    return false;
}

/*********************************************************************
* Function: bool TIMER_SetConfiguration(TIMER_CONFIGURATIONS configuration)
*
* Overview: Initializes the timer.
*
* PreCondition: None
*
* Input:  None
*
* Output: bool - true if successful, false if unsuccessful
*
********************************************************************/
bool TIMER_SetConfiguration(TIMER_CONFIGURATIONS configuration)
{
    switch(configuration)
    {
        case TIMER_CONFIGURATION_1MS:
            memset(requests, 0, sizeof(requests));
            
            T4CON = 0x0; //Stop and Init Timer
            T4CONbits.TCKPS= 0b110;     //prescaler=1:64

            TMR4 = 0; //Clear timer register
            PR4 = 1000; //Load period register

            IPC4SET = 0x00000004; // Set priority level=1 and
            IPC4SET = 0x00000001; // Set subpriority level=1

            IFS0bits.T4IF = 0;
            IEC0bits.T4IE = 1;

            T4CONSET = 0x8000;  //Start Timer

            return true;
            
        case TIMER_CONFIGURATION_OFF:
            IEC0bits.T3IE = 0;
            return true;
    }

    return false;
}

/****************************************************************************
  Function:
    void __attribute__((__interrupt__, auto_psv)) _T3Interrupt(void)

  Description:
    Timer ISR, used to update application state. If no transfers are pending
    new input request is scheduled.
  Precondition:
    None

  Parameters:
    None

  Return Values:
    None

  Remarks:
    None
  ***************************************************************************/
#pragma interrupt _T4Interrupt ipl4 vector 16
void _T4Interrupt( void )
{
    uint8_t i;

    for(i = 0; i < TIMER_MAX_1MS_CLIENTS; i++)
    {
        if(requests[i].handle != NULL)
        {
            requests[i].count++;
            
            if(requests[i].count == requests[i].rate)
            {
                requests[i].handle();
                requests[i].count = 0;
            }
        }
    }
    
    IFS0bits.T4IF = 0;
}
