/******************************************************************************

  USB Host Printer Client Driver

  Summary:
    This is the Printer client driver file for a USB Embedded Host device.

  Description:
    This is the Printer client driver file for a USB Embedded Host device.
    It allows an embedded application to utilize a USB printer to provide
    printed output.

    USB printers utilize the USB Printer Class to communicate with a USB
    Host.  This class defines the USB transfer type, the endpoint structure,
    a device requests that can be performed.  The actual commands sent to
    the printer, however, are dictated by the printer language used by the
    particular printer.

    Many different printer languages are utilized by the wide variety of
    printers on the market.  Typically, low end printers receive printer-specific
    binary data, utilizing the processing power of the USB Host to perform
    all of the complex calculations required to translate text and graphics to
    a simple binary representation.  This works well when a PC is the USB Host,
    but it is not conducive to an embedded application with limited resources.

    Many printers on the market use a command based printer language, relying
    on the printer itself to interpret commands to produce the desired output.
    Some languages are standardized across printers from a particular
    manufacturer, and some are used across multiple manufacturer.  This method
    lends itself better to embedded applications by allowing the printer to
    take on some of the computational overhead.  Microchip provides support for
    some printer languages, including PostScript and PCL 5.  Additional printer
    language can be implemented.  Refer to the USB Embedded Host Printer Class
    application notes for more details on implementing printer language support.


  Remarks:
    This driver should be used in a project with usb_host.c to provided the USB
    Embedded Host and hardware interfaces, plus one or more language support
    files.

    To interface with USB Embedded Host layer, the routine USBHostPrinterInitialize()
    should be specified as the Initialize() function, and
    USBHostPrinterEventHandler() should be specified as the EventHandler()
    function in the usbClientDrvTable[] array declared in usb_config.c.

    This driver requires transfer events from usb_host.c, so
    USB_ENABLE_TRANSFER_EVENT must be defined.

    Since the printer class is performed with bulk transfers,
    USB_SUPPORT_BULK_TRANSFERS must be defined.

*******************************************************************************/
//DOM-IGNORE-BEGIN
/******************************************************************************

FileName:        usb_host_printer.c
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
  Rev           Description
  ----------    -----------
  2.6 - 2.6a    No chance except stack revision number
  2.7           Minor updates to USBHostPrinterGetStatus() header
                to better describe the function requirements and
                operation.

                Changed how transfer queues are handled to do a peek
                now before removing the item from the queue.
********************************************************************/

//DOM-IGNORE-END

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "usb_config.h"
#include <usb/usb_struct_queue.h>
#include <usb/usb.h>
#include <usb/usb_host_printer.h>

#ifdef USB_PRINTER_LANGUAGE_PCL_5
    #include <usb/usb_host_printer_pcl_5.h>
#endif

#ifdef USB_PRINTER_LANGUAGE_POSTSCRIPT
    #include <usb/usb_host_printer_postscript.h>
#endif

#ifndef USB_MALLOC
    #define USB_MALLOC(size) malloc(size)
#endif

#ifndef USB_FREE
    #define USB_FREE(ptr) free(ptr)
#endif

#define USB_FREE_AND_CLEAR(ptr) {USB_FREE(ptr); ptr = NULL;}


// *****************************************************************************
// *****************************************************************************
// Section: Configuration
// *****************************************************************************
// *****************************************************************************

#if !defined(USB_ENABLE_TRANSFER_EVENT)
    #error The USB Host Printer Client Driver requires transfer events.
#endif


// *****************************************************************************
// *****************************************************************************
// Section: Constants
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// Section: Interface and Protocol Constants
// *****************************************************************************

#define DEVICE_CLASS_PRINTER                0x07    // Class code for Printers

#define DEVICE_SUBCLASS_PRINTERS            0x01    // SubClass code for Printers

#define DEVICE_INTERFACE_UNIDIRECTIONAL     0x01    // Protocol code for unidirectional interface
#define DEVICE_INTERFACE_BIDIRECTIONAL      0x02    // Protocol code for bidirectional interface
#define DEVICE_INTERFACE_IEEE1284_4         0x03    // Protocol code for IEEE 1284.4 interface


// *****************************************************************************
// *****************************************************************************
// Section: Data Structures
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Printer Transfer Queue Information

This structure contains the information needed for one entry in the transfer
queue.
*/
typedef struct _USB_PRINTER_QUEUE_ITEM
{
    uint32_t   size;
    uint8_t    *data;
    uint8_t    flags;
} USB_PRINTER_QUEUE_ITEM;


// *****************************************************************************
/* Printer Transfer Queue

This is the structure for the printer transfer queue.
*/
typedef struct _USB_PRINTER_QUEUE
{
    int                     head;
    int                     tail;
    int                     count;
    USB_PRINTER_QUEUE_ITEM  buffer[USB_PRINTER_TRANSFER_QUEUE_SIZE];
} USB_PRINTER_QUEUE;


// *****************************************************************************
/* Printer Device Information

This structure contains information about an attached device, including
status flags and device identification.
*/
typedef struct _USB_PRINTER_DEVICE
{
    USB_PRINTER_DEVICE_ID           ID;             // Identification information about the device
    uint8_t                            clientDriverID;
    uint32_t                           rxLength;       // Number of bytes received in the last IN transfer
    #ifdef USB_PRINTER_ALLOW_DYNAMIC_LANGUAGE_DETERMINATION
        char*                       deviceIDString;
        uint16_t                        deviceIDStringLength;
        uint16_t                        deviceIDStringIndex;
    #endif
    USB_PRINTER_LANGUAGE_HANDLER    languageHandler;
    uint8_t                            endpointIN;     // Bulk IN endpoint
    uint8_t                            endpointOUT;    // Bulk OUT endpoint

    USB_PRINTER_QUEUE               transferQueueIN;
    USB_PRINTER_QUEUE               transferQueueOUT;

    union
    {
        uint8_t value;                     // uint8_t representation of device status flags
        struct
        {
            uint8_t inUse                          : 1;    // This array member is in use
            uint8_t initialized                    : 1;    // Driver has been initialized
            uint8_t txBusy                         : 1;    // Driver busy transmitting data
            uint8_t rxBusy                         : 1;    // Driver busy receiving data
            #ifdef USB_PRINTER_ALLOW_DYNAMIC_LANGUAGE_DETERMINATION
                uint8_t deviceIDStringLengthValid  : 1;    // Device ID string length is valid
            #endif
        };
    } flags;                            // Printer client driver status flags

} USB_PRINTER_DEVICE;


// *****************************************************************************
// *****************************************************************************
// Section: Global Variables
// *****************************************************************************
// *****************************************************************************

uint8_t                                    currentPrinterRecord;
extern USB_PRINTER_INTERFACE            usbPrinterClientLanguages[];
USB_PRINTER_DEVICE                      usbPrinters[USB_MAX_PRINTER_DEVICES];
extern USB_PRINTER_SPECIFIC_INTERFACE   usbPrinterSpecificLanguage[];


// *****************************************************************************
// *****************************************************************************
// Section: Local Prototypes
// *****************************************************************************
// *****************************************************************************

bool _USBHostPrinter_FindDevice( uint8_t address );
#ifdef USB_PRINTER_ALLOW_DYNAMIC_LANGUAGE_DETERMINATION
    bool _USBHostPrinter_GetDeviceIDString( void );
#endif
uint8_t _USBHostPrinter_ReadFromQueue( uint8_t deviceAddress );
uint8_t _USBHostPrinter_WriteFromQueue( uint8_t deviceAddress );

// *****************************************************************************
// *****************************************************************************
// Section: Host Stack Interface Functions
// *****************************************************************************
// *****************************************************************************

/****************************************************************************
  Function:
    bool USBHostPrinterInitialize ( uint8_t address, uint32_t flags, uint8_t clientDriverID )

  Summary:
    This function is called by the USB Embedded Host layer when a printer
    attaches.

  Description:
    This routine is a call out from the USB Embedded Host layer to the USB
    printer client driver.  It is called when a "printer" device has been
    connected to the host.  Its purpose is to initialize and activate the USB
    Printer client driver.

  Preconditions:
    The device has been configured.

  Parameters:
    uint8_t address    - Device's address on the bus
    uint32_t flags     - Initialization flags
    uint8_t clientDriverID - Client driver identification for device requests

  Return Values:
    true    - Initialization was successful
    false   - Initialization failed

  Remarks:
    Multiple client drivers may be used in a single application.  The USB
    Embedded Host layer will call the initialize routine required for the
    attached device.
  ***************************************************************************/

bool USBHostPrinterInitialize ( uint8_t address, uint32_t flags, uint8_t clientDriverID )
{
    uint8_t        endpointIN;
    uint8_t        endpointOUT;
    #ifdef USB_PRINTER_ALLOW_DYNAMIC_LANGUAGE_DETERMINATION
        uint8_t    errorCode;
    #endif
    uint16_t        i;
    uint16_t        j;
    uint8_t        *pDesc;
    uint8_t        *pDescriptor;

    #ifdef DEBUG_MODE
        UART2PrintString( "PRN: Printer Client Init called\r\n" );
    #endif

    for (currentPrinterRecord=0; currentPrinterRecord<USB_MAX_PRINTER_DEVICES; currentPrinterRecord++)
    {
        if (!usbPrinters[currentPrinterRecord].flags.inUse) break;
    }
    if (currentPrinterRecord == USB_MAX_PRINTER_DEVICES)
    {
        #ifdef DEBUG_MODE
            UART2PrintString( "PRN: No more space\r\n" );
        #endif
        return false;   // We have no more room for a new device.
    }

    pDesc  = USBHostGetDeviceDescriptor(address);

    #ifdef USB_PRINTER_ALLOW_DYNAMIC_LANGUAGE_DETERMINATION
        usbPrinters[currentPrinterRecord].deviceIDString = (char *)USB_MALLOC( ((USB_DEVICE_DESCRIPTOR*)pDesc)->bMaxPacketSize0 );
        if (usbPrinters[currentPrinterRecord].deviceIDString == NULL)
        {
            return false;   // Out of memory
        }
    #endif

    // Initialize state
    usbPrinters[currentPrinterRecord].rxLength                  = 0;
    usbPrinters[currentPrinterRecord].flags.value               = 0x01; // Set the inUse flag.
    #ifdef USB_PRINTER_ALLOW_DYNAMIC_LANGUAGE_DETERMINATION
        usbPrinters[currentPrinterRecord].deviceIDStringIndex   = 0;
    #endif

    // Save device the address, VID, & PID, and client driver ID.
    usbPrinters[currentPrinterRecord].ID.deviceAddress          = address;
    pDesc += 8;
    usbPrinters[currentPrinterRecord].ID.vid                    =  (uint16_t)*pDesc;        pDesc++;
    usbPrinters[currentPrinterRecord].ID.vid                    |= ((uint16_t)*pDesc) << 8; pDesc++;
    usbPrinters[currentPrinterRecord].ID.pid                    =  (uint16_t)*pDesc;        pDesc++;
    usbPrinters[currentPrinterRecord].ID.pid                    |= ((uint16_t)*pDesc) << 8; pDesc++;
    usbPrinters[currentPrinterRecord].clientDriverID            = clientDriverID;

    //  Extract the bulk IN and OUT endpoint that the printer uses.

    pDescriptor = (uint8_t *)USBHostGetCurrentConfigurationDescriptor( address );

    i = 0;

    // Find the next interface descriptor.
    while (i < ((USB_CONFIGURATION_DESCRIPTOR *)pDescriptor)->wTotalLength)
    {
        // See if we are pointing to an interface descriptor.
        if (pDescriptor[i+1] == USB_DESCRIPTOR_INTERFACE)
        {
            if (USBHostDeviceSpecificClientDriver( address ) ||
                ((pDescriptor[i+5] == DEVICE_CLASS_PRINTER) &&
                 (pDescriptor[i+6] == DEVICE_SUBCLASS_PRINTERS) &&
                 ((pDescriptor[i+7] == DEVICE_INTERFACE_UNIDIRECTIONAL) ||
                  (pDescriptor[i+7] == DEVICE_INTERFACE_BIDIRECTIONAL))))
            {
                // Either this client driver was specified in the TPL for this
                // exact device, or the interface has the correct class,
                // subclass, and protocol values for the printer class.

                // Look for bulk IN and OUT endpoints.
                endpointIN  = 0;
                endpointOUT = 0;

                // Scan for endpoint descriptors.
                i += pDescriptor[i];
                while (pDescriptor[i+1] == USB_DESCRIPTOR_ENDPOINT)
                {
                    if (pDescriptor[i+3] == 0x02) // Bulk
                    {
                        if (((pDescriptor[i+2] & 0x80) == 0x80) && (endpointIN == 0))
                        {
                            endpointIN = pDescriptor[i+2];
                        }
                        if (((pDescriptor[i+2] & 0x80) == 0x00) && (endpointOUT == 0))
                        {
                            endpointOUT = pDescriptor[i+2];
                        }
                    }
                    i += pDescriptor[i];
                }

                if ((endpointIN != 0) && (endpointOUT != 0))
                {
                    // Initialize the device endpoint information.
                    usbPrinters[currentPrinterRecord].endpointIN       = endpointIN;
                    usbPrinters[currentPrinterRecord].endpointOUT      = endpointOUT;
                    USBHostSetNAKTimeout( address, endpointIN,  1, USB_NUM_BULK_NAKS );
                    USBHostSetNAKTimeout( address, endpointOUT, 1, USB_NUM_BULK_NAKS );

                    // See if the printer language has been specified explicitly.
                    j = 0;
                    usbPrinters[currentPrinterRecord].languageHandler = NULL;
                    while ((usbPrinterSpecificLanguage[j].vid != 0x0000) &&
                           (usbPrinters[currentPrinterRecord].languageHandler == NULL))
                    {
                        if ((usbPrinterSpecificLanguage[j].vid == usbPrinters[currentPrinterRecord].ID.vid) &&
                            (usbPrinterSpecificLanguage[j].pid == usbPrinters[currentPrinterRecord].ID.pid))
                        {
                            usbPrinters[currentPrinterRecord].languageHandler = usbPrinterClientLanguages[usbPrinterSpecificLanguage[j].languageIndex].languageCommandHandler;
                            usbPrinters[currentPrinterRecord].ID.support      = usbPrinterSpecificLanguage[j].support;
                        }
                        j ++;
                    }

                    if (usbPrinters[currentPrinterRecord].languageHandler != NULL)
                    {
                        // We have a printer language that we can use with this printer.
                        // Complete the client driver attachment.
                        usbPrinters[currentPrinterRecord].flags.initialized = 1;
                        USBHostPrinterCommand( usbPrinters[currentPrinterRecord].ID.deviceAddress, USB_PRINTER_ATTACHED,
                                USB_DATA_POINTER_RAM(&(usbPrinters[currentPrinterRecord].ID.support)), sizeof(USB_PRINTER_FUNCTION_SUPPORT), 0 );

                        // Tell the application layer that we have a device.
                        USB_HOST_APP_EVENT_HANDLER( usbPrinters[currentPrinterRecord].ID.deviceAddress, EVENT_PRINTER_ATTACH, &(usbPrinters[currentPrinterRecord].ID), sizeof(USB_PRINTER_DEVICE_ID) );
                    }
                    else
                    {
                        #ifdef USB_PRINTER_ALLOW_DYNAMIC_LANGUAGE_DETERMINATION
                            // No printer language has been specified for this printer.
                            // Get the printer device ID string, so we can try to determine
                            // what printer languages it supports.
                            errorCode = USBHostIssueDeviceRequest( address, USB_SETUP_DEVICE_TO_HOST | USB_SETUP_TYPE_CLASS | USB_SETUP_RECIPIENT_INTERFACE,
                                            PRINTER_DEVICE_REQUEST_GET_DEVICE_ID, 0, 0, ((USB_DEVICE_DESCRIPTOR*)pDeviceDescriptor)->bMaxPacketSize0,
                                            (uint8_t *)usbPrinters[currentPrinterRecord].deviceIDString, USB_DEVICE_REQUEST_GET,
                                            usbPrinters[currentPrinterRecord].clientDriverID );
                            if (errorCode)
                            {
                                // If we cannot read the ID string, then we cannot determine the required printer language.
                                usbPrinters[currentPrinterRecord].flags.value = 0;
                                return false;
                            }
                        #else
                            // No printer language has been specified for this printer.
                            // We cannot enumerate it.
                            #ifdef DEBUG_MODE
                                UART2PrintString( "PRN: No printer language specified.\r\n" );
                            #endif
                            usbPrinters[currentPrinterRecord].flags.value = 0;
                            return false;
                        #endif
                    }

                    return true;
                }
            }
        }

        // Jump to the next descriptor in this configuration.
        i += pDescriptor[i];
    }

    // This client driver could not initialize the device
    usbPrinters[currentPrinterRecord].flags.value = 0;
    return false;

} // USBHostPrinterInitialize


/****************************************************************************
  Function:
    bool USBHostPrinterEventHandler ( uint8_t address, USB_EVENT event,
                            void *data, uint32_t size )

  Summary:
    This routine is called by the Host layer to notify the general client of
    events that occur.

  Description:
    This routine is called by the Host layer to notify the general client of
    events that occur.  If the event is recognized, it is handled and the
    routine returns true.  Otherwise, it is ignored and the routine returns
    false.

    This routine can notify the application with the following events:
        * EVENT_PRINTER_ATTACH
        * EVENT_PRINTER_DETACH
        * EVENT_PRINTER_TX_DONE
        * EVENT_PRINTER_RX_DONE
        * EVENT_PRINTER_REQUEST_DONE
        * EVENT_PRINTER_UNSUPPORTED

  Preconditions:
    None

  Parameters:
    uint8_t address    - Address of device with the event
    USB_EVENT event - The bus event that occured
    void *data      - Pointer to event-specific data
    uint32_t size      - Size of the event-specific data

  Return Values:
    true    - The event was handled
    false   - The event was not handled

  Remarks:
    None
  ***************************************************************************/

bool USBHostPrinterEventHandler ( uint8_t address, USB_EVENT event, void *data, uint32_t size )
{
    USB_PRINTER_QUEUE_ITEM  *transfer;

    // Make sure it was for our device
    if (!_USBHostPrinter_FindDevice( address ))
    {
        return false;   // The device was not found.
    }

    // Handle specific events.
    switch (event)
    {
        case EVENT_DETACH:
            // Purge the IN and OUT transfer queues.
            while (StructQueueIsNotEmpty( &(usbPrinters[currentPrinterRecord].transferQueueIN), USB_PRINTER_TRANSFER_QUEUE_SIZE ))
            {
                transfer = StructQueueRemove( &(usbPrinters[currentPrinterRecord].transferQueueIN), USB_PRINTER_TRANSFER_QUEUE_SIZE );
            }
            while (StructQueueIsNotEmpty( &(usbPrinters[currentPrinterRecord].transferQueueOUT), USB_PRINTER_TRANSFER_QUEUE_SIZE ))
            {
                transfer = StructQueueRemove( &(usbPrinters[currentPrinterRecord].transferQueueOUT), USB_PRINTER_TRANSFER_QUEUE_SIZE );
                if (transfer->flags & USB_PRINTER_TRANSFER_COPY_DATA)
                {
                    USB_FREE( transfer->data );
                }
            }

            // Tell the printer language support that the device has been detached.
            USBHostPrinterCommand( usbPrinters[currentPrinterRecord].ID.deviceAddress, USB_PRINTER_DETACHED, USB_NULL, 0, 0 );
            // Notify that application that the device has been detached.
            USB_HOST_APP_EVENT_HANDLER(usbPrinters[currentPrinterRecord].ID.deviceAddress, EVENT_PRINTER_DETACH,
                    &usbPrinters[currentPrinterRecord].ID.deviceAddress, sizeof(uint8_t) );
            #ifdef USB_PRINTER_ALLOW_DYNAMIC_LANGUAGE_DETERMINATION
                USB_FREE( usbPrinters[currentPrinterRecord].deviceIDString );
            #endif
            usbPrinters[currentPrinterRecord].flags.value = 0;
            return true;

        case EVENT_TRANSFER:

            if ( (data != NULL) && (size == sizeof(HOST_TRANSFER_DATA)) )
            {
                uint32_t                   dataCount = ((HOST_TRANSFER_DATA *)data)->dataCount;

                if ( ((HOST_TRANSFER_DATA *)data)->bEndpointAddress == (USB_IN_EP | usbPrinters[currentPrinterRecord].endpointIN) )
                {
                    usbPrinters[currentPrinterRecord].flags.rxBusy   = 0;
                    usbPrinters[currentPrinterRecord].rxLength       = dataCount;

                    transfer = StructQueueRemove( &(usbPrinters[currentPrinterRecord].transferQueueIN), USB_PRINTER_TRANSFER_QUEUE_SIZE );
                    if (transfer->flags & USB_PRINTER_TRANSFER_COPY_DATA)
                    {
                        USB_FREE( transfer->data );
                    }

                    if (StructQueueIsNotEmpty( &(usbPrinters[currentPrinterRecord].transferQueueIN), USB_PRINTER_TRANSFER_QUEUE_SIZE ))
                    {
                        _USBHostPrinter_ReadFromQueue( usbPrinters[currentPrinterRecord].ID.deviceAddress );
                    }

                    if (transfer->flags & USB_PRINTER_TRANSFER_NOTIFY)
                    {
                        USB_HOST_APP_EVENT_HANDLER( usbPrinters[currentPrinterRecord].ID.deviceAddress, EVENT_PRINTER_RX_DONE, &dataCount, sizeof(uint32_t) );
                    }
                }
                else if ( ((HOST_TRANSFER_DATA *)data)->bEndpointAddress == (USB_OUT_EP | usbPrinters[currentPrinterRecord].endpointOUT) )
                {
                    usbPrinters[currentPrinterRecord].flags.txBusy   = 0;

                    transfer = StructQueueRemove( &(usbPrinters[currentPrinterRecord].transferQueueOUT), USB_PRINTER_TRANSFER_QUEUE_SIZE );
                    if (transfer->flags & USB_PRINTER_TRANSFER_COPY_DATA)
                    {
                        USB_FREE( transfer->data );
                    }

                    if (StructQueueIsNotEmpty( &(usbPrinters[currentPrinterRecord].transferQueueOUT), USB_PRINTER_TRANSFER_QUEUE_SIZE ))
                    {
                        _USBHostPrinter_WriteFromQueue( usbPrinters[currentPrinterRecord].ID.deviceAddress );
                    }

                    if (transfer->flags & USB_PRINTER_TRANSFER_NOTIFY)
                    {
                        USB_HOST_APP_EVENT_HANDLER( usbPrinters[currentPrinterRecord].ID.deviceAddress, EVENT_PRINTER_TX_DONE, &dataCount, sizeof(uint32_t) );
                    }
                }
                else if ((((HOST_TRANSFER_DATA *)data)->bEndpointAddress & 0x7F) == 0)
                {
                    #ifdef USB_PRINTER_ALLOW_DYNAMIC_LANGUAGE_DETERMINATION
                    if (!usbPrinters[currentPrinterRecord].flags.initialized)
                    {
                        if (!_USBHostPrinter_GetDeviceIDString())
                        {
                             USB_HOST_APP_EVENT_HANDLER( usbPrinters[currentPrinterRecord].ID.deviceAddress, EVENT_PRINTER_UNSUPPORTED, NULL, 0 );
                        }
                    }
                    else
                    #endif
                    {
                        USB_HOST_APP_EVENT_HANDLER( usbPrinters[currentPrinterRecord].ID.deviceAddress, EVENT_PRINTER_REQUEST_DONE, &dataCount, sizeof(uint32_t) );
                    }
                }
                else
                {
                    // Event is on an unknown endpoint.
                    return false;
                }
                return true;
            }
            else
            {
                // The data does not match what was expected for this event.
                return false;
            }
            break;

        case EVENT_BUS_ERROR:
            // A bus error occurred. Clean up the best we can.

            if ( (data != NULL) && (size == sizeof(HOST_TRANSFER_DATA)) )
            {
                if ( ((HOST_TRANSFER_DATA *)data)->bEndpointAddress == (USB_IN_EP | usbPrinters[currentPrinterRecord].endpointIN) )
                {
                    usbPrinters[currentPrinterRecord].flags.rxBusy   = 0;
                    usbPrinters[currentPrinterRecord].rxLength       = 0;

                    transfer = StructQueueRemove( &(usbPrinters[currentPrinterRecord].transferQueueIN), USB_PRINTER_TRANSFER_QUEUE_SIZE );
                    if (transfer->flags & USB_PRINTER_TRANSFER_COPY_DATA)
                    {
                        USB_FREE( transfer->data );
                    }

                    if (StructQueueIsNotEmpty( &(usbPrinters[currentPrinterRecord].transferQueueIN), USB_PRINTER_TRANSFER_QUEUE_SIZE ))
                    {
                        _USBHostPrinter_ReadFromQueue( usbPrinters[currentPrinterRecord].ID.deviceAddress );
                    }

//                    if (transfer->flags & USB_PRINTER_TRANSFER_NOTIFY)
                    {
                        USB_HOST_APP_EVENT_HANDLER( usbPrinters[currentPrinterRecord].ID.deviceAddress, EVENT_PRINTER_RX_ERROR, NULL, ((HOST_TRANSFER_DATA *)data)->bErrorCode );
                    }
                }
                else if ( ((HOST_TRANSFER_DATA *)data)->bEndpointAddress == (USB_OUT_EP | usbPrinters[currentPrinterRecord].endpointOUT) )
                {
                    usbPrinters[currentPrinterRecord].flags.txBusy   = 0;

                    transfer = StructQueueRemove( &(usbPrinters[currentPrinterRecord].transferQueueOUT), USB_PRINTER_TRANSFER_QUEUE_SIZE );
                    if (transfer->flags & USB_PRINTER_TRANSFER_COPY_DATA)
                    {
                        USB_FREE( transfer->data );
                    }

                    if (StructQueueIsNotEmpty( &(usbPrinters[currentPrinterRecord].transferQueueOUT), USB_PRINTER_TRANSFER_QUEUE_SIZE ))
                    {
                        _USBHostPrinter_WriteFromQueue( usbPrinters[currentPrinterRecord].ID.deviceAddress );
                    }

//                    if (transfer->flags & USB_PRINTER_TRANSFER_NOTIFY)
                    {
                        USB_HOST_APP_EVENT_HANDLER( usbPrinters[currentPrinterRecord].ID.deviceAddress, EVENT_PRINTER_TX_ERROR, NULL, ((HOST_TRANSFER_DATA *)data)->bErrorCode );
                    }
                }
                else if ((((HOST_TRANSFER_DATA *)data)->bEndpointAddress & 0x7F) == 0)
                {
                    #ifdef USB_PRINTER_ALLOW_DYNAMIC_LANGUAGE_DETERMINATION
                    if (!usbPrinters[currentPrinterRecord].flags.initialized &&
                        !usbPrinters[currentPrinterRecord].flags.deviceIDStringLengthValid)
                    {
                        // The printer does not support the GET_DEVICE_ID printer class request.
                        // We cannot determine a printer language for this printer.
                        usbPrinters[currentPrinterRecord].flags.value = 0;
                        USB_HOST_APP_EVENT_HANDLER( usbPrinters[currentPrinterRecord].ID.deviceAddress, EVENT_PRINTER_UNSUPPORTED, NULL, 0 );

                    }
                    else
                    #endif
                    {
                        // The printer gave an error during an application control transfer.
                        USB_HOST_APP_EVENT_HANDLER( usbPrinters[currentPrinterRecord].ID.deviceAddress, EVENT_PRINTER_REQUEST_ERROR, NULL, ((HOST_TRANSFER_DATA *)data)->bErrorCode );
                    }
                }
                else
                {
                    // Event is on an unknown endpoint.
                    return false;
                }
                return true;
            }
            else
            {
                // The data does not match what was expected for this event.
                return false;
            }
            break;

        case EVENT_SUSPEND:
        case EVENT_RESUME:
        default:
            break;
    }

    return false;
} // USBHostPrinterEventHandler


// *****************************************************************************
// *****************************************************************************
// Section: Application Callable Functions
// *****************************************************************************
// *****************************************************************************

/****************************************************************************
  Function:
    uint8_t USBHostPrinterCommand( uint8_t deviceAddress, USB_PRINTER_COMMAND command,
                    USB_DATA_POINTER data, uint32_t size, uint8_t flags )

  Summary:
    This is the primary user interface function for the printer client
    driver.  It is used to issue all printer commands.

  Description:
    This is the primary user interface function for the printer client
    driver.  It is used to issue all printer commands.  Each generic printer
    command is translated to the appropriate command for the supported
    language and is enqueued for transfer to the printer.  Before calling
    this routine, it is recommended to call the function
    USBHostPrinterCommandReady() to determine if there is space available
    in the printer's output queue.

  Preconditions:
    None

  Parameters:
    uint8_t address                - Device's address on the bus
    USB_PRINTER_COMMAND command - Command to execute.  See the enumeration
                                    USB_PRINTER_COMMAND for the list of
                                    valid commands and their requirements.
    USB_DATA_POINTER data       - Pointer to the required data.  Note that
                                    the caller must set transferFlags
                                    appropriately to indicate if the pointer is
                                    a RAM pointer or a ROM pointer.
    uint32_t size                  - Size of the data.  For some commands, this
                                    parameter is used to hold the data itself.
    uint8_t transferFlags          - Flags that indicate details about the
                                    transfer operation.  Refer to these flags
                                    * USB_PRINTER_TRANSFER_COPY_DATA
                                    * USB_PRINTER_TRANSFER_STATIC_DATA
                                    * USB_PRINTER_TRANSFER_NOTIFY
                                    * USB_PRINTER_TRANSFER_FROM_ROM
                                    * USB_PRINTER_TRANSFER_FROM_RAM

  Returns:
    See the USB_PRINTER_ERRORS enumeration.  Also, refer to the printer
    language command handler function, such as
    USBHostPrinterLanguagePostScript().

  Example:
    <code>
    if (USBHostPrinterCommandReady( address ))
    {
        USBHostPrinterCommand( address, USB_PRINTER_JOB_START, NULL, 0, 0 );
    }
    </code>

  Remarks:
    Use the definitions USB_DATA_POINTER_RAM() and USB_DATA_POINTER_ROM()
    to cast data pointers.  For example:
    <code>
        USBHostPrinterCommand( address, USB_PRINTER_TEXT, USB_DATA_POINTER_RAM(buffer), strlen(buffer), 0 );
    </code>

    When developing new commands, keep in mind that the function
    USBHostPrinterCommandReady() will often be used before calling this
    function to see if there is space available in the output transfer queue.
    USBHostPrinterCommandReady() will routine true if a single space is
    available in the output queue.  Therefore, each command can generate only
    one output transfer.
  ***************************************************************************/

uint8_t USBHostPrinterCommand( uint8_t deviceAddress, USB_PRINTER_COMMAND command,
                    USB_DATA_POINTER data, uint32_t size, uint8_t flags )
{
    if (!_USBHostPrinter_FindDevice( deviceAddress ))
    {
        // The device was not found.
        return USB_PRINTER_UNKNOWN_DEVICE;
    }

    return usbPrinters[currentPrinterRecord].languageHandler( deviceAddress, command, data, size, flags );
}


/****************************************************************************
  Function:
    bool USBHostPrinterCommandReady( uint8_t deviceAddress )

  Description:
    This interface is used to check if the client driver has space available
    to enqueue another transfer.

  Preconditions:
    None

  Parameters:
    deviceAddress     - USB Address of the device

  Return Values:
    true    - The printer client driver has room for at least one more
                transfer request, or the device is not attached.  The latter
                allows this routine to be called without generating an
                infinite loop if the device detaches.
    false   - The transfer queue is full.

  Example:
    <code>
    if (USBHostPrinterCommandReady( address ))
    {
        USBHostPrinterCommand( address, USB_PRINTER_JOB_START, NULL, 0, 0 );
    }
    </code>

  Remarks:
    This routine will return true if a single transfer can be enqueued.  Since
    this routine is the check to see if USBHostPrinterCommand() can be called,
    every command can generate at most one transfer.
  ***************************************************************************/

bool USBHostPrinterCommandReady( uint8_t deviceAddress )
{
    if (!_USBHostPrinter_FindDevice( deviceAddress ))
    {
        // The device was not found.
        return true;
    }

    if (StructQueueSpaceAvailable( 1, &(usbPrinters[currentPrinterRecord].transferQueueOUT), USB_PRINTER_TRANSFER_QUEUE_SIZE ))
    {
        return true;
    }
    return false;
}


/****************************************************************************
  Function:
    bool USBHostPrinterDeviceDetached( uint8_t deviceAddress )

  Description:
    This interface is used to check if the device has been detached from the
    bus.

  Preconditions:
    None

  Parameters:
    uint8_t deviceAddress  - USB Address of the device.

  Return Values:
    true    - The device has been detached, or an invalid deviceAddress is given.
    false   - The device is attached

  Example:
    <code>
    if (USBHostPrinterDeviceDetached( deviceAddress ))
    {
        // Handle detach
    }
    </code>

  Remarks:
    The event EVENT_PRINTER_DETACH can also be used to detect a detach.
  ***************************************************************************/

bool USBHostPrinterDeviceDetached( uint8_t deviceAddress )
{
    if (_USBHostPrinter_FindDevice( deviceAddress ))
    {
        return false;
    }
    return true;
}


/****************************************************************************
  Function:
    uint32_t USBHostPrinterGetRxLength( uint8_t deviceAddress )

  Description:
    This function retrieves the number of bytes copied to user's buffer by
    the most recent call to the USBHostPrinterRead() function.

  Preconditions:
    The device must be connected and enumerated.

  Parameters:
    uint8_t deviceAddress  - USB Address of the device

  Returns:
    Returns the number of bytes most recently received from the Generic
    device with address deviceAddress.

  Remarks:
    None
  ***************************************************************************/

uint32_t USBHostPrinterGetRxLength( uint8_t deviceAddress )
{
    if (!_USBHostPrinter_FindDevice( deviceAddress ))
    {
        return 0;
    }
    return usbPrinters[currentPrinterRecord].rxLength;
}


/****************************************************************************
  Function:
    uint8_t USBHostPrinterGetStatus( uint8_t deviceAddress, uint8_t *status )

  Summary:
    This function issues the Printer class-specific Device Request to
    obtain the printer status.

  Description:
    This function issues the Printer class-specific Device Request to
    obtain the printer status.  The returned status should have the following
    format, per the USB specification.  Any deviation will be due to the
    specific printer implementation.
    * Bit 5 - Paper Empty; 1 = paper empty, 0 = paper not empty
    * Bit 4 - Select; 1 = selected, 0 = not selected
    * Bit 3 - Not Error; 1 = no error, 0 = error
    * All other bits are reserved.

    The *status parameter is not updated until the EVENT_PRINTER_REQUEST_DONE
    event is thrown.  Until that point the value of *status is unknown.

    The *status parameter will only be updated if this function returns
    USB_SUCCESS.  If this function returns with any other error code then the
    EVENT_PRINTER_REQUEST_DONE event will not be thrown and the status field
    will not be updated.

  Preconditions:
    The device must be connected and enumerated.

  Parameters:
    deviceAddress   - USB Address of the device
    *status         - pointer to the returned status byte

  Returns:
    See the return values for the USBHostIssueDeviceRequest() function.

  Remarks:
    None
  ***************************************************************************/

uint8_t USBHostPrinterGetStatus( uint8_t deviceAddress, uint8_t *status )
{
    if (!_USBHostPrinter_FindDevice( deviceAddress ))
    {
        return USB_UNKNOWN_DEVICE;
    }

   return USBHostIssueDeviceRequest( deviceAddress,
        USB_SETUP_DEVICE_TO_HOST | USB_SETUP_TYPE_CLASS | USB_SETUP_RECIPIENT_INTERFACE,
        PRINTER_DEVICE_REQUEST_GET_PORT_STATUS,
        0, 0x0000, 1,
        status, USB_DEVICE_REQUEST_GET, usbPrinters[currentPrinterRecord].clientDriverID );
}


/****************************************************************************
  Function:
    uint8_t USBHostPrinterRead( uint8_t deviceAddress, uint8_t *buffer, uint32_t length,
                uint8_t transferFlags )

  Description:
    Use this routine to receive from the device and store it into memory.

  Preconditions:
    The device must be connected and enumerated.

  Parameters:
    deviceAddress  - USB Address of the device.
    buffer         - Pointer to the data buffer
    length         - Number of bytes to be transferred
    transferFlags  - Flags for how to perform the operation

  Return Values:
    USB_SUCCESS         - The Read was started successfully
    (USB error code)    - The Read was not started.  See USBHostRead() for
                            a list of errors.

  Example:
    <code>
    if (!USBHostPrinterRxIsBusy( deviceAddress ))
    {
        USBHostPrinterRead( deviceAddress, &buffer, sizeof(buffer), 0 );
    }
    </code>

  Remarks:
    None
  ***************************************************************************/

uint8_t USBHostPrinterRead( uint8_t deviceAddress, void *buffer, uint32_t length,
            uint8_t transferFlags )
{
    USB_PRINTER_QUEUE_ITEM  *transfer;

    // Validate the call
    if (!_USBHostPrinter_FindDevice( deviceAddress ) ||
        !usbPrinters[currentPrinterRecord].flags.initialized)
    {
        return USB_INVALID_STATE;
    }

    // Check transfer path
    if (StructQueueIsFull( &(usbPrinters[currentPrinterRecord].transferQueueIN), USB_PRINTER_TRANSFER_QUEUE_SIZE ))
    {
        return USB_PRINTER_BUSY;
    }

    // Add the newest transfer to the queue

    transfer = StructQueueAdd( &(usbPrinters[currentPrinterRecord].transferQueueIN), USB_PRINTER_TRANSFER_QUEUE_SIZE);
    transfer->data  = buffer;
    transfer->size  = length;
    transfer->flags = transferFlags;

    if (usbPrinters[currentPrinterRecord].flags.rxBusy)
    {
        // The request has been queued.  We'll execute it when the current transfer is complete.
        return USB_SUCCESS;
    }
    else
    {
        return _USBHostPrinter_ReadFromQueue( deviceAddress );
    }
} // USBHostPrinterRead


/****************************************************************************
  Function:
    uint8_t USBHostPrinterReset( uint8_t deviceAddress )

  Description:
    This function issues the Printer class-specific Device Request to
    perform a soft reset.

  Preconditions:
    The device must be connected and enumerated.

  Parameters:
    uint8_t deviceAddress  - USB Address of the device

  Returns:
    See the return values for the USBHostIssueDeviceRequest() function.

  Remarks:
    Not all printers support this command.
  ***************************************************************************/

uint8_t USBHostPrinterReset( uint8_t deviceAddress )
{
    if (!_USBHostPrinter_FindDevice( deviceAddress ))
    {
        return USB_UNKNOWN_DEVICE;
    }

    return USBHostIssueDeviceRequest( deviceAddress,
        USB_SETUP_HOST_TO_DEVICE | USB_SETUP_TYPE_CLASS | USB_SETUP_RECIPIENT_INTERFACE,
        PRINTER_DEVICE_REQUEST_SOFT_RESET,
        0, 0x0000, 0,
        NULL, USB_DEVICE_REQUEST_SET, usbPrinters[currentPrinterRecord].clientDriverID );
}


/****************************************************************************
  Function:
    bool USBHostPrinterRxIsBusy( uint8_t deviceAddress )

  Summary:
    This interface is used to check if the client driver is currently busy
    receiving data from the device.

  Description:
    This interface is used to check if the client driver is currently busy
    receiving data from the device.  This function is intended for use with
    transfer events.  With polling, the function USBHostPrinterRxIsComplete()
    should be used.

  Preconditions:
    None

  Parameters:
    deviceAddress     - USB Address of the device

  Return Values:
    true    - The device is receiving data or an invalid deviceAddress is
                given.
    false   - The device is not receiving data

  Example:
    <code>
    if (!USBHostPrinterRxIsBusy( deviceAddress ))
    {
        USBHostPrinterRead( deviceAddress, &Buffer, sizeof( Buffer ) );
    }
    </code>

  Remarks:
    None
  ***************************************************************************/

bool USBHostPrinterRxIsBusy( uint8_t deviceAddress )
{
    if (!_USBHostPrinter_FindDevice( deviceAddress ))
    {
        // The device was not found.
        return true;
    }

    if (usbPrinters[currentPrinterRecord].flags.rxBusy)
    {
        return true;
    }
    return false;
}


/****************************************************************************
  Function:
    uint8_t USBHostPrinterWrite( uint8_t deviceAddress, void *buffer, uint32_t length,
                uint8_t transferFlags )

  Description:
    Use this routine to transmit data from memory to the device.  This
    routine will not usually be called by the application directly.  The
    application will use the USBHostPrinterCommand() function, which will
    call the appropriate printer language support function, which will
    utilize this routine.

  Preconditions:
    The device must be connected and enumerated.

  Parameters:
    uint8_t deviceAddress  - USB Address of the device.
    void *buffer        - Pointer to the data buffer
    uint32_t length        - Number of bytes to be transferred
    uint8_t transferFlags  - Flags for how to perform the operation

  Return Values:
    USB_SUCCESS                 - The Write was started successfully.
    USB_PRINTER_UNKNOWN_DEVICE  - Device not found or has not been initialized.
    USB_PRINTER_BUSY            - The printer's output queue is full.
    (USB error code)            - The Write was not started.  See USBHostWrite() for
                                    a list of errors.

  Remarks:
    None
  ***************************************************************************/


uint8_t USBHostPrinterWrite( uint8_t deviceAddress, void *buffer, uint32_t length,
            uint8_t transferFlags)
{
    USB_PRINTER_QUEUE_ITEM  *transfer;

    // Validate the call
    if (!_USBHostPrinter_FindDevice( deviceAddress ) ||
        !usbPrinters[currentPrinterRecord].flags.initialized)
    {
        return USB_PRINTER_UNKNOWN_DEVICE;
    }

    // Check transfer path
    if (StructQueueIsFull( &(usbPrinters[currentPrinterRecord].transferQueueOUT), USB_PRINTER_TRANSFER_QUEUE_SIZE ))
    {
        // The queue is full.  If the caller was using heap space for the data,
        // deallocate the memory.
        if (transferFlags & USB_PRINTER_TRANSFER_COPY_DATA)
        {
            USB_FREE( buffer );
        }
        return USB_PRINTER_BUSY;
    }

    // Add the newest transfer to the queue

    transfer = StructQueueAdd( &(usbPrinters[currentPrinterRecord].transferQueueOUT), USB_PRINTER_TRANSFER_QUEUE_SIZE );
    transfer->data  = buffer;
    transfer->size  = length;
    transfer->flags = transferFlags;

    if (usbPrinters[currentPrinterRecord].flags.txBusy)
    {
        // The request has been queued.  We'll execute it when the current transfer is complete.
        return USB_SUCCESS;
    }
    else
    {
        // We can send the request now.  We still have to put it in the
        // queue, because that's where the event handler gets its information.
        return _USBHostPrinter_WriteFromQueue( deviceAddress );
    }
}


/****************************************************************************
  Function:
    bool USBHostPrinterWriteComplete( uint8_t deviceAddress )

  Description:
    This interface is used to check if the client driver is currently
    transmitting data to the printer, or if it is between transfers but still
    has transfers queued.

  Preconditions:
    None

  Parameters:
    deviceAddress     - USB Address of the device

  Return Values:
    true    - The device is done transmitting data or an invalid deviceAddress
                is given.
    false   - The device is transmitting data or has a transfer in the queue.

  Remarks:
    None
  ***************************************************************************/

bool USBHostPrinterWriteComplete( uint8_t deviceAddress )
{
    if (!_USBHostPrinter_FindDevice( deviceAddress ))
    {
        // The device was not found.
        return true;
    }

    if (usbPrinters[currentPrinterRecord].flags.txBusy ||
        !(StructQueueIsEmpty( &(usbPrinters[currentPrinterRecord].transferQueueOUT), USB_PRINTER_TRANSFER_QUEUE_SIZE )))
    {
        return false;
    }
    return true;
}


// *****************************************************************************
// *****************************************************************************
// Section: Internal Functions
// *****************************************************************************
// *****************************************************************************

/****************************************************************************
  Function:
    bool _USBHostPrinter_FindDevice( uint8_t address )

  Description:
    This function looks in the array of printers to see if there is currently
    a printer attached at the indicated device.  If so, it returns true, and
    the global variable currentPrinterRecord is changed to the index of the
    printer.  If not, it returns false.  Note that the function implies
    nothing about the status of the printer; it may not be ready to use.

  Preconditions:
    None

  Parameters:
    deviceAddress   - USB Address of the device.

  Return Values:
    true    - The indicated device is attached
    false   - The indicated device is not attached

  Remarks:
    None
  ***************************************************************************/

bool _USBHostPrinter_FindDevice( uint8_t address )
{
    for (currentPrinterRecord=0; currentPrinterRecord<USB_MAX_PRINTER_DEVICES; currentPrinterRecord++)
    {
        if (usbPrinters[currentPrinterRecord].flags.inUse &&
            (usbPrinters[currentPrinterRecord].ID.deviceAddress == address))
        {
            return true;
        }
    }
    return false;   // The device was not found.
}


/****************************************************************************
  Function:
    bool _USBHostPrinter_GetDeviceIDString( void )

  Description:
    This routine performs most of the process required to get the device ID
    string.  The initial request to get the length is performed in the
    initialization handler.  This routine handles the processing of the
    length, the request for the entire string, and processing of the string.
    The format of this string is specified by IEEE 1284.

  Preconditions:
    None

  Parameters:
    None - None

  Return Values:
    true    - Processing is proceeding normally
    false   - Cannot read the device ID string, or cannot find a printer
                language to use to communicate with the printer.

  Remarks:
    This function is available only if USB_PRINTER_ALLOW_DYNAMIC_LANGUAGE_DETERMINATION
    has been defined.
  ***************************************************************************/

#ifdef USB_PRINTER_ALLOW_DYNAMIC_LANGUAGE_DETERMINATION

bool _USBHostPrinter_GetDeviceIDString( void )
{
    uint8_t errorCode;

    if (!usbPrinters[currentPrinterRecord].flags.deviceIDStringLengthValid)
    {
        // This transfer was to get the length of the string.
        // The length is a 2-byte value, MSB first.
        usbPrinters[currentPrinterRecord].deviceIDStringLength = ( (uint8_t)usbPrinters[currentPrinterRecord].deviceIDString[0] << 8) +  (uint8_t)usbPrinters[currentPrinterRecord].deviceIDString[1];
        usbPrinters[currentPrinterRecord].flags.deviceIDStringLengthValid = 1;

        USB_FREE( usbPrinters[currentPrinterRecord].deviceIDString );
        usbPrinters[currentPrinterRecord].deviceIDString = (char *)USB_MALLOC( usbPrinters[currentPrinterRecord].deviceIDStringLength + 3 );
        if (usbPrinters[currentPrinterRecord].deviceIDString == NULL)
        {
            #ifdef DEBUG_MODE
                UART2PrintString( "PRN: Out of memory for the device ID string.\r\n" );
            #endif
            usbPrinters[currentPrinterRecord].flags.value = 0;
            return false;
        }
        else
        {
            // Get the full string
            errorCode = USBHostIssueDeviceRequest(  usbPrinters[currentPrinterRecord].ID.deviceAddress,
                            USB_SETUP_DEVICE_TO_HOST | USB_SETUP_TYPE_CLASS | USB_SETUP_RECIPIENT_INTERFACE,
                            PRINTER_DEVICE_REQUEST_GET_DEVICE_ID, 0, 0, usbPrinters[currentPrinterRecord].deviceIDStringLength,
                            (uint8_t *)usbPrinters[currentPrinterRecord].deviceIDString, USB_DEVICE_REQUEST_GET,
                            usbPrinters[currentPrinterRecord].clientDriverID );
            if (errorCode)
            {
                // If we cannot read the ID string, then we cannot determine the required printer language.
                usbPrinters[currentPrinterRecord].flags.value = 0;
                return false;
            }
            #ifdef DEBUG_MODE
                UART2PrintString( "PRN: Getting device ID string ...\r\n" );
            #endif
        }
    }
    else
    {
        char                            *commandSet;
        uint16_t                            i;
        uint16_t                            semicolonLocation;

        // Null terminate the device ID string so we can do string manipulation on it.
        usbPrinters[currentPrinterRecord].deviceIDString[usbPrinters[currentPrinterRecord].deviceIDStringLength + 2] = 0;

        // Determine if one of the languages we are currently using can support this
        // printer.  Languages should be listed in ascending order of preference
        // in the usbPrinterClientLanguages array, so we can stop looking as soon
        // as we find a match.
        commandSet = strstr( &(usbPrinters[currentPrinterRecord].deviceIDString[2]), "COMMAND SET:" );
        if (!commandSet)
        {
            commandSet = strstr( &(usbPrinters[currentPrinterRecord].deviceIDString[2]), "CMD:" );
        }
        if (!commandSet)
        {
            usbPrinters[currentPrinterRecord].flags.value = 0;
            return false;
        }

        // Replace the semicolon at the end of the COMMAND SET: specification with a
        // null, so we don't get a false positive based on some other portion of the
        // device ID string.  If we don't find a semicolon, set the location to 0.
        for (semicolonLocation = 0;
             (commandSet[semicolonLocation] != 0) && (commandSet[semicolonLocation] != ';');
             semicolonLocation ++ ) {};
        if (commandSet[semicolonLocation] == 0)
        {
            semicolonLocation = 0;
        }
        else
        {
            commandSet[semicolonLocation] = 0;
        }

        // Convert the command set to all upper case.
        for (i=0; i<semicolonLocation; i++)
        {
            commandSet[i] = toupper( commandSet[i] );
        }

        // Look for a supported printer language in the array of available languages.
        i = 0;
        usbPrinters[currentPrinterRecord].languageHandler = NULL;
        while ((usbPrinterClientLanguages[i].isLanguageSupported != NULL) &&
               (usbPrinters[currentPrinterRecord].languageHandler == NULL))
        {
            if (usbPrinterClientLanguages[i].isLanguageSupported( commandSet, &(usbPrinters[currentPrinterRecord].ID.support) ))
            {
                usbPrinters[currentPrinterRecord].languageHandler = usbPrinterClientLanguages[i].languageCommandHandler;
            }
            i ++;
        }

        // Restore the device ID string to its original state.
        if (semicolonLocation != 0)
        {
            commandSet[semicolonLocation] = ';';
        }

        // See if we were able to find a printer language.
        if (usbPrinters[currentPrinterRecord].languageHandler == NULL)
        {
            usbPrinters[currentPrinterRecord].flags.value = 0;
            return false;
        }

        // We have a printer language that we can use with this printer.
        usbPrinters[currentPrinterRecord].flags.initialized = 1;
        USBHostPrinterCommand( usbPrinters[currentPrinterRecord].ID.deviceAddress, USB_PRINTER_ATTACHED,
                USB_DATA_POINTER_RAM(&(usbPrinters[currentPrinterRecord].ID.support)), sizeof(USB_PRINTER_FUNCTION_SUPPORT), 0 );

        // Tell the application layer that we have a device.
        USB_HOST_APP_EVENT_HANDLER( usbPrinters[currentPrinterRecord].ID.deviceAddress, EVENT_PRINTER_ATTACH,
                &(usbPrinters[currentPrinterRecord].ID), sizeof(USB_PRINTER_DEVICE_ID) );
    }

    return true;
}

#endif


/****************************************************************************
  Function:
    void _USBHostPrinter_ReadFromQueue( uint8_t deviceAddress )

  Description:
    This routine initiates the IN transfer described by the first entry
    in the queue.

  Preconditions:
    * The receive path must be clear before calling this routine.
    * The queue must contain at least one valid entry.
    * currentPrinterRecord must be valid.

  Parameters:
    deviceAddress  - USB Address of the device.

  Return Values:
    USB_SUCCESS         - The Read was started successfully
    (USB error code)    - The Read was not started.  See USBHostRead() for
                            a list of errors.

  Remarks:
    None
  ***************************************************************************/

uint8_t _USBHostPrinter_ReadFromQueue( uint8_t deviceAddress )
{
    uint8_t                    returnValue;
    USB_PRINTER_QUEUE_ITEM  *transfer;

    transfer = StructQueuePeekTail( &(usbPrinters[currentPrinterRecord].transferQueueIN), USB_PRINTER_TRANSFER_QUEUE_SIZE );

    usbPrinters[currentPrinterRecord].flags.rxBusy = 1;
    usbPrinters[currentPrinterRecord].rxLength     = 0;
    returnValue = USBHostRead( deviceAddress, USB_IN_EP|usbPrinters[currentPrinterRecord].endpointIN, (uint8_t *)(transfer->data), transfer->size );
    if (returnValue != USB_SUCCESS)
    {
        usbPrinters[currentPrinterRecord].flags.rxBusy = 0;    // Clear flag to allow re-try
    }

    return returnValue;
}


/****************************************************************************
  Function:
    void _USBHostPrinter_WriteFromQueue( uint8_t deviceAddress )

  Description:
    This routine initiates the OUT transfer described by the first entry
    in the queue.

  Preconditions:
    * The transmit path must be clear before calling this routine.
    * The queue must contain at least one valid entry.
    * currentPrinterRecord must be valid.

  Parameters:
    deviceAddress  - USB Address of the device.

  Return Values:
    USB_SUCCESS         - The Read was started successfully
    (USB error code)    - The Read was not started.  See USBHostWrite() for
                            a list of errors.

  Remarks:
    None
  ***************************************************************************/

uint8_t _USBHostPrinter_WriteFromQueue( uint8_t deviceAddress )
{
    uint8_t                    returnValue;
    USB_PRINTER_QUEUE_ITEM  *transfer;

    transfer = StructQueuePeekTail( &(usbPrinters[currentPrinterRecord].transferQueueOUT), USB_PRINTER_TRANSFER_QUEUE_SIZE );

    usbPrinters[currentPrinterRecord].flags.txBusy = 1;
    returnValue = USBHostWrite( deviceAddress, USB_OUT_EP|usbPrinters[currentPrinterRecord].endpointOUT, (uint8_t *)(transfer->data), transfer->size );
    if (returnValue != USB_SUCCESS)
    {
        usbPrinters[currentPrinterRecord].flags.txBusy = 0;    // Clear flag to allow re-try
    }

    return returnValue;
}


/*************************************************************************
 * EOF usb_client_generic.c
 */


