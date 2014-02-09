/*****************************************************************************

    Microchip USB Host Printer Client Driver, Graphics Library Interfaces Layer

  Summary:
    This file contains the routines needed to utilize the Microchip Graphics
    Library functions to create graphic images on a USB printer.

  Description:
    This file contains the routines needed to utilize the Microchip Graphics
    Library functions to create graphic images on a USB printer.

    The label USE_GRAPHICS_LIBRARY_PRINTER_INTERFACE must be defining in the
    USB configuration header file usb_config.h to utilize these functions.

  Remarks:
    None

******************************************************************************/
//DOM-IGNORE-BEGIN
/******************************************************************************

 FileName:        usb_host_printer_primitives.c
 Dependencies:    None
 Processor:       PIC24F/PIC32MX
 Compiler:        C30/C32
 Company:         Microchip Technology, Inc.

Software License Agreement

The software supplied herewith by Microchip Technology Incorporated
(the �Company�) for its PICmicro� Microcontroller is intended and
supplied to you, the Company�s customer, for use solely and
exclusively on Microchip PICmicro Microcontroller products. The
software is owned by the Company and/or its supplier, and is
protected under applicable copyright laws. All rights are reserved.
Any use in violation of the foregoing restrictions may subject the
user to criminal sanctions under applicable laws, as well as to
civil liability for the breach of the terms and conditions of this
license.

THIS SOFTWARE IS PROVIDED IN AN �AS IS� CONDITION. NO WARRANTIES,
WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

Change History:
  Rev         Description
  ----------  ----------------------------------------------------------
  2.6 - 2.6a  No change

*******************************************************************************/
//DOM-IGNORE-END


#include <stdlib.h>
#include <string.h>
#include <usb/usb.h>
#include <usb/usb_host_printer_primitives.h>

#ifndef USB_MALLOC
    #define USB_MALLOC(size) malloc(size)
#endif

#ifndef USB_FREE
    #define USB_FREE(ptr) free(ptr)
#endif

#define USB_FREE_AND_CLEAR(ptr) {USB_FREE(ptr); ptr = NULL;}

#ifdef USE_GRAPHICS_LIBRARY_PRINTER_INTERFACE


// *****************************************************************************
// *****************************************************************************
// Section: Subroutines
// *****************************************************************************
// *****************************************************************************




#endif

