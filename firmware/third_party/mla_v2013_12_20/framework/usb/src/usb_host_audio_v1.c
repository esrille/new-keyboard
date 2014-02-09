/******************************************************************************

  USB Host Audio v1.0 Client Driver

This is the Audio v1.0 Class Client driver file for a USB Embedded Host device.
This file should be used in a project with usb_host.c to provided the USB
hardware interface.

To interface with usb_host.c, the routine USBHostAudioV1Initialize() should be
specified as the Initialize() function, and USBHostAudioV1EventHandler() should
be specified as the EventHandler() function in the usbClientDrvTable[] array
declared in usb_config.h.

This driver utilizes transfer events from the host driver; therefore,
USB_ENABLE_TRANSFER_EVENT must be defined.

Since audio support is performed with both isochronous and interrupt transfers,
USB_SUPPORT_INTERRUPT_TRANSFERS and USB_SUPPORT_ISOCHRONOUS_TRANSFERS
must be defined.

* FileName:        usb_host_audio_v1.c
* Dependencies:    None
* Processor:       PIC24/dsPIC30/dsPIC33/PIC32MX
* Compiler:        C30 v3.12/C32 v1.05
* Company:         Microchip Technology, Inc.

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

Author          Date    Comments
--------------------------------------------------------------------------------
KO          08-Apr-2009 First release

*******************************************************************************/


#include <stdlib.h>
#include <string.h>
#include <usb/usb.h>
#include <usb/usb_host_audio_v1.h>


// *****************************************************************************
// *****************************************************************************
// Section: Configuration
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Confirm Configuration

Ensure that all of the proper configurations have been set up for this client
driver.
*/

#ifndef USB_SUPPORT_INTERRUPT_TRANSFERS
    #error The Audio v1.0 Client Driver requires interrupt transfers.
#endif

#ifndef USB_SUPPORT_ISOCHRONOUS_TRANSFERS
    #error The Audio v1.0 Client Driver requires isochronous transfers.
#endif

#ifndef USB_ENABLE_TRANSFER_EVENT
    #error The Audio v1.0 Client Driver requires transfer events.
#endif


// *****************************************************************************
/* Max Number of Supported Devices

This value represents the maximum number of attached devices this class driver
can support.  If the user does not define a value, it will be set to 1.
Currently this must be set to 1, due to limitations in the USB Host layer.
*/
#ifndef USB_MAX_AUDIO_DEVICES
    #define USB_MAX_AUDIO_DEVICES        1
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Constants
// *****************************************************************************
// *****************************************************************************

#define USB_CLASS_AUDIO                             0x01

#define USB_SUBCLASS_AUDIO_UNDEFINED                0x00
#define USB_SUBCLASS_AUDIO_CONTROL                  0x01
#define USB_SUBCLASS_AUDIO_STREAMING                0x02
#define USB_SUBCLASS_AUDIO_MIDISTREAMING            0x03

#define USB_AUDIO_DESCRIPTOR_UNDEFINED              0x20
#define USB_AUDIO_DESCRIPTOR_DEVICE                 0x21
#define USB_AUDIO_DESCRIPTOR_CONFIGURATION          0x22
#define USB_AUDIO_DESCRIPTOR_STRING                 0x23
#define USB_AUDIO_DESCRIPTOR_INTERFACE              0x24
#define USB_AUDIO_DESCRIPTOR_ENDPOINT               0x25

#define USB_AUDIO_AC_IF_DESCRIPTOR_UNDEFINED        0x00
#define USB_AUDIO_AC_IF_DESCRIPTOR_HEADER           0x01
#define USB_AUDIO_AC_IF_DESCRIPTOR_INPUT_TERMINAL   0x02
#define USB_AUDIO_AC_IF_DESCRIPTOR_OUTPUT_TERMINAL  0x03
#define USB_AUDIO_AC_IF_DESCRIPTOR_MIXER_UNIT       0x04
#define USB_AUDIO_AC_IF_DESCRIPTOR_SELECTOR_UNIT    0x05
#define USB_AUDIO_AC_IF_DESCRIPTOR_FEATURE_UNIT     0x06
#define USB_AUDIO_AC_IF_DESCRIPTOR_PROCESSING_UNIT  0x07
#define USB_AUDIO_AC_IF_DESCRIPTOR_EXTENSION_UNIT   0x08

#define USB_AUDIO_AS_IF_DESCRIPTOR_UNDEFINED        0x00
#define USB_AUDIO_AS_IF_DESCRIPTOR_GENERAL          0x01
#define USB_AUDIO_AS_IF_DESCRIPTOR_FORMAT_TYPE      0x02
#define USB_AUDIO_AS_IF_DESCRIPTOR_FORMAT_SPECIFIC  0x03

#define USB_AUDIO_FORMAT_TYPE_UNDEFINED             0x00
#define USB_AUDIO_FORMAT_TYPE_I                     0x01
#define USB_AUDIO_FORMAT_TYPE_II                    0x02
#define USB_AUDIO_FORMAT_TYPE_III                   0x03

// *****************************************************************************
// Section: Device Request Constants
// *****************************************************************************

// Class-specific Request Codes
#define SET_CUR                                     0x01
#define GET_CUR                                     0x81
#define SET_MIN                                     0x02
#define GET_MIN                                     0x82
#define SET_MAX                                     0x03
#define GET_MAX                                     0x83
#define SET_RES                                     0x04
#define GET_RES                                     0x84
#define SET_MEM                                     0x05
#define GET_MEM                                     0x85
#define GET_STAT                                    0xFF

// Endpoint Control Selectors
#if defined( CONTROL_SELECTOR_IN_LOW_BYTE )
    // Some devices require the CS value in the low byte, which contradicts the USB Audio v1.0 spec
    #define SAMPLING_FREQ_CONTROL                       0x01
    #define PITCH_CONTROL                               0x02
#else
    #define SAMPLING_FREQ_CONTROL                       0x0100
    #define PITCH_CONTROL                               0x0200
#endif

// *****************************************************************************
// Section: Other Constants
// *****************************************************************************



//******************************************************************************
//******************************************************************************
// Section: Data Structures
//******************************************************************************
//******************************************************************************

// *****************************************************************************
/* USB Audio Device Information

This structure is used to hold all the information about an attached Mass Storage device.
*/
typedef struct _USB_AUDIO_DEVICE_INFO
{
    USB_AUDIO_V1_DEVICE_ID              ID;
    uint8_t                                *pFormatTypeDescriptor;
    union
    {
        struct
        {
            uint8_t                        bfFoundZeroBandwidthSetting : 1;
            uint8_t                        bfFoundFullBandwidthSetting : 1;
            uint8_t                        bfSettingInterface          : 1;
            uint8_t                        bfSettingFrequency          : 1;
        };
        uint8_t                            val;
    }                                   flags;
    uint8_t                                interface;              // Interface number we are using.
    uint8_t                                settingZeroBandwidth;   // The zero bandwidth alternate setting.
    uint8_t                                settingFullBandwidth;   // The full bandwidth alternate setting.
    uint8_t                                endpointAudioStream;    // Streaming audio endpoint.
} USB_AUDIO_DEVICE_INFO;



//******************************************************************************
//******************************************************************************
// Section: Local Prototypes
//******************************************************************************
//******************************************************************************



//******************************************************************************
//******************************************************************************
// Section: Macros
//******************************************************************************
//******************************************************************************



//******************************************************************************
//******************************************************************************
// Section: Audio V1.0 Host Global Variables
//******************************************************************************
//******************************************************************************

static USB_AUDIO_DEVICE_INFO         deviceInfoAudioV1[USB_MAX_AUDIO_DEVICES] __attribute__ ((aligned));


// *****************************************************************************
// *****************************************************************************
// Section: Application Callable Functions
// *****************************************************************************
// *****************************************************************************


/****************************************************************************
  Function:
    uint8_t USBHostAudioV1ReceiveAudioData( uint8_t deviceAddress,
        ISOCHRONOUS_DATA *pIsochronousData )

  Summary:
    This function starts the reception of streaming, isochronous audio data.

  Description:
    This function starts the reception of streaming, isochronous audio data.

  Precondition:
    USBHostAudioV1SetInterfaceFullBandwidth() must be called to set the 
    device to its full bandwidth interface.

  Parameters:
    uint8_t deviceAddress      - Device address
    ISOCHRONOUS_DATA *pIsochronousData - Pointer to an ISOCHRONOUS_DATA
                            structure, containing information for the
                            application and the host driver for the
                            isochronous transfer.

  Return Values:
    USB_SUCCESS                 - Request started successfully
    USB_AUDIO_DEVICE_NOT_FOUND  - No device with specified address
    USB_AUDIO_DEVICE_BUSY       - Device is already receiving audio data or
                                    setting an interface.
    Others                      - See USBHostIssueDeviceRequest() errors.

  Remarks:
    Some devices require other operations between setting the full bandwidth
    interface and starting the streaming audio data.  Therefore, these two 
    functions are broken out separately.
  ***************************************************************************/

uint8_t USBHostAudioV1ReceiveAudioData( uint8_t deviceAddress,
        ISOCHRONOUS_DATA *pIsochronousData )
{
    uint32_t   byteCount;
    uint8_t    errorCode;
    uint8_t    i;

    // Find the correct device.
    for (i=0; (i<USB_MAX_AUDIO_DEVICES) && (deviceInfoAudioV1[i].ID.deviceAddress != deviceAddress); i++);
    if (i == USB_MAX_AUDIO_DEVICES)
    {
        return USB_AUDIO_DEVICE_NOT_FOUND;
    }

    // Make sure the device is not already receiving data or setting the interface.
    if (!USBHostTransferIsComplete( deviceInfoAudioV1[i].ID.deviceAddress, 
            deviceInfoAudioV1[i].endpointAudioStream, &errorCode, &byteCount ) || 
        deviceInfoAudioV1[i].flags.bfSettingInterface)
    {
        return USB_AUDIO_DEVICE_BUSY;
    }

    // Start receiving data
    errorCode = USBHostReadIsochronous( deviceInfoAudioV1[i].ID.deviceAddress, 
            deviceInfoAudioV1[i].endpointAudioStream, pIsochronousData );
    if (errorCode)
    {
    }
    
    return errorCode;    
}


/****************************************************************************
  Function:
    uint8_t USBHostAudioV1SetInterfaceFullBandwidth( uint8_t deviceAddress )

  Summary:
    This function sets the full bandwidth interface.

  Description:
    This function sets the full bandwidth interface.  This function should
    be called before calling USBHostAudioV1ReceiveAudioData() to receive the
    audio stream.  Upon completion, the event EVENT_AUDIO_INTERFACE_SET will
    be generated.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress      - Device address

  Return Values:
    USB_SUCCESS                 - Request started successfully
    USB_AUDIO_DEVICE_NOT_FOUND  - No device with specified address
    USB_AUDIO_DEVICE_BUSY       - Device is already receiving audio data or
                                    setting an interface.
    Others                      - See USBHostIssueDeviceRequest() errors.

  Remarks:
    None
  ***************************************************************************/

uint8_t USBHostAudioV1SetInterfaceFullBandwidth( uint8_t deviceAddress )
{
    uint32_t   byteCount;
    uint8_t    errorCode;
    uint8_t    i;

    // Find the correct device.
    for (i=0; (i<USB_MAX_AUDIO_DEVICES) && (deviceInfoAudioV1[i].ID.deviceAddress != deviceAddress); i++);
    if (i == USB_MAX_AUDIO_DEVICES)
    {
        return USB_AUDIO_DEVICE_NOT_FOUND;
    }

    // Make sure the device is not already receiving data.
    if (!USBHostTransferIsComplete( deviceInfoAudioV1[i].ID.deviceAddress, 
            deviceInfoAudioV1[i].endpointAudioStream, &errorCode, &byteCount ) ||
        deviceInfoAudioV1[i].flags.bfSettingInterface)
    {
        return USB_AUDIO_DEVICE_BUSY;
    }

    // Set the interface to the full bandwidth setting.
    errorCode = USBHostIssueDeviceRequest( deviceInfoAudioV1[i].ID.deviceAddress, 0x01, USB_REQUEST_SET_INTERFACE,
            deviceInfoAudioV1[i].settingFullBandwidth, deviceInfoAudioV1[i].interface, 0, NULL, USB_DEVICE_REQUEST_SET,
            deviceInfoAudioV1[i].ID.clientDriverID );
    if (errorCode)
    {
    }
    else    
    {
        deviceInfoAudioV1[i].flags.bfSettingInterface = 1;
    }
  
    return errorCode;    
}


/****************************************************************************
  Function:
    uint8_t USBHostAudioV1SetInterfaceZeroBandwidth( uint8_t deviceAddress )

  Summary:
    This function sets the zero bandwidth interface. 

  Description:
    This function sets the full bandwidth interface.  This function can
    be called after calling USBHostAudioV1TerminateTransfer() to terminate the
    audio stream.  Upon completion, the event EVENT_AUDIO_INTERFACE_SET will
    be generated.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address

  Return Values:
    USB_SUCCESS                 - Request started successfully
    USB_AUDIO_DEVICE_NOT_FOUND  - No device with the specified address.
    Others                      - See USBHostIssueDeviceRequest()
    
  Remarks:
    None
  ***************************************************************************/

uint8_t USBHostAudioV1SetInterfaceZeroBandwidth( uint8_t deviceAddress )
{
    uint32_t   byteCount;
    uint8_t    errorCode;
    uint8_t    i;

    // Find the correct device.
    for (i=0; (i<USB_MAX_AUDIO_DEVICES) && (deviceInfoAudioV1[i].ID.deviceAddress != deviceAddress); i++);
    if (i == USB_MAX_AUDIO_DEVICES)
    {
        return USB_AUDIO_DEVICE_NOT_FOUND;
    }

    // Make sure the device is not already receiving data.
    if (!USBHostTransferIsComplete( deviceInfoAudioV1[i].ID.deviceAddress, 
            deviceInfoAudioV1[i].endpointAudioStream, &errorCode, &byteCount ) ||
        deviceInfoAudioV1[i].flags.bfSettingInterface)
    {
        return USB_AUDIO_DEVICE_BUSY;
    }

    // Set the interface to the zero bandwidth setting.
    errorCode = USBHostIssueDeviceRequest( deviceInfoAudioV1[i].ID.deviceAddress, 0x01, USB_REQUEST_SET_INTERFACE,
            deviceInfoAudioV1[i].settingZeroBandwidth, deviceInfoAudioV1[i].interface, 0, NULL, USB_DEVICE_REQUEST_SET,
            deviceInfoAudioV1[i].ID.clientDriverID );
            
    if (errorCode)
    {
    }
    else    
    {
        deviceInfoAudioV1[i].flags.bfSettingInterface = 1;
    }
  
    return errorCode;                
}

/****************************************************************************
  Function:
    uint8_t USBHostAudioV1SetSamplingFrequency( uint8_t deviceAddress, uint8_t *frequency )

  Summary:
    This function sets the sampling frequency for the device.

  Description:
    This function sets the sampling frequency for the device.  If the exact 
    frequency is not supported by the device, the device will round it to the
    closest supported value.

    IMPORTANT: If the request is initiated successfully, the frequency value 
    must remain valid until the EVENT_AUDIO_FREQUENCY_SET event is received.  
    Therefore, this value cannot be a local (stack) variable.  The application
    can either use a global variable for this value, or it can use the 
    function USBHostAudioV1SupportedFrequencies() to obtain a pointer to the
    number and list of supported frequencies, and pass a pointer to the desired
    frequency in this list.
    
  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address
    uint8_t *frequency     - Pointer to three bytes that specify the desired
                            sampling frequency.  NOTE: If the request is 
                            initiated successfully, this location must 
                            remain valid until the EVENT_AUDIO_FREQUENCY_SET 
                            event is received.  

  Return Values:
    USB_SUCCESS                 - Request started successfully
    Others                      - See USBHostIssueDeviceRequest() errors.

  Example:
    <code>
    uint8_t  numFrequencies;
    uint8_t  *ptr;

    ptr = USBHostAudioV1SupportedFrequencies( deviceAddress );
    if (ptr)
    {
        numFrequencies = *ptr;
        ptr++;
        if (numFrequencies == 0)
        {
            // Continuous sampling, minimum and maximum are specified.
            uint32_t   minFrequency;
            uint32_t   maxFrequency;
            
            minFrequency = *ptr + (*(ptr+1) << 8) + (*(ptr+2) << 16);
            ptr += 3;
            maxFrequency = *ptr + (*(ptr+1) << 8) + (*(ptr+2) << 16);
            if ((minFrequency <= desiredFrequency) && (desiredFrequency <= maxFrequency))
            {
                rc = USBHostAudioV1SetSamplingFrequency( deviceAddress, &desiredFrequency );
            }
            else
            {
                // Desired frequency out of range
            }
        }
        else
        {
            // Discrete sampling frequencies are specified.    
            uint32_t frequency;
            
            while (numFrequencies)
            {
                frequency = *ptr + (*(ptr+1) << 8) + (*(ptr+2) << 16);
                if (frequency == desiredFrequency)
                {
                    rc = USBHostAudioV1SetSamplingFrequency( deviceAddress, ptr );
                    continue;
                }
                numFrequencies--;
                ptr += 3;
            }
            if (numFrequencies == 0)
            {
                // Desired frequency not found.
            }
        }
    }
    </code>
  
  Remarks:
    If a global variable is used to old the frequency, it can be declared as
    a uint32_t.  Since PIC Microcontrollers are little endian machines, a
    pointer to the uint32_t can be used as the frequency parameter:
    <code>
    uint32_t   desiredFrequency = 44100;   // Hertz

    rc = USBHostAudioV1SetSamplingFrequency( deviceAddress, (uint8_t *)(&desiredFrequency) );
    </code>
    
  ***************************************************************************/

uint8_t USBHostAudioV1SetSamplingFrequency( uint8_t deviceAddress, uint8_t *frequency )
{
    uint8_t    errorCode;
    uint8_t    i;
    
    // Find the correct device.
    for (i=0; (i<USB_MAX_AUDIO_DEVICES) && (deviceInfoAudioV1[i].ID.deviceAddress != deviceAddress); i++);
    if (i == USB_MAX_AUDIO_DEVICES)
    {
        return USB_AUDIO_DEVICE_NOT_FOUND;
    }

    // Issue the device request.
    errorCode = USBHostIssueDeviceRequest( deviceInfoAudioV1[i].ID.deviceAddress, 0x22, SET_CUR,
            SAMPLING_FREQ_CONTROL, deviceInfoAudioV1[i].endpointAudioStream, 3, frequency, USB_DEVICE_REQUEST_SET,
            deviceInfoAudioV1[i].ID.clientDriverID );
    if (errorCode)
    {
    }
    else    
    {
        // Set a flag so we will send back the correct event when the request is done.
        deviceInfoAudioV1[i].flags.bfSettingFrequency   = 1;
    }
    
    return errorCode;
}    


/****************************************************************************
  Function:
    uint8_t * USBHostAudioV1SupportedFrequencies( uint8_t deviceAddress )
    
  Summary:
    This function returns a pointer to the list of supported frequencies.

  Description:
    This function returns a pointer to the list of supported frequencies.  It
    is intended to be used with the function USBHostAudioV1SetSamplingFrequency()
    to set the device's sampling frequency.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address

  Returns:
    This function returns a uint8_t pointer to the list of supported frequencies.
    The first byte of this list is the number of supported frequencies.  Each
    supported frequency is then listed, with three bytes for each frequency.

  Example:
    <code>
    uint8_t  numFrequencies;
    uint8_t  *ptr;

    ptr = USBHostAudioV1SupportedFrequencies( deviceAddress );
    if (ptr)
    {
        numFrequencies = *ptr;
        ptr++;
        if (numFrequencies == 0)
        {
            // Continuous sampling, minimum and maximum are specified.
            uint32_t   minFrequency;
            uint32_t   maxFrequency;
            
            minFrequency = *ptr + (*(ptr+1) << 8) + (*(ptr+2) << 16);
            ptr += 3;
            maxFrequency = *ptr + (*(ptr+1) << 8) + (*(ptr+2) << 16);
            if ((minFrequency <= desiredFrequency) && (desiredFrequency <= maxFrequency))
            {
                rc = USBHostAudioV1SetSamplingFrequency( deviceAddress, &desiredFrequency );
            }
            else
            {
                // Desired frequency out of range
            }
        }
        else
        {
            // Discrete sampling frequencies are specified.    
            uint32_t frequency;
            
            while (numFrequencies)
            {
                frequency = *ptr + (*(ptr+1) << 8) + (*(ptr+2) << 16);
                if (frequency == desiredFrequency)
                {
                    rc = USBHostAudioV1SetSamplingFrequency( deviceAddress, ptr );
                    continue;
                }
                numFrequencies--;
                ptr += 3;
            }
            if (numFrequencies == 0)
            {
                // Desired frequency not found.
            }
        }
    }
    </code>
  
  Remarks:
    None
  ***************************************************************************/

uint8_t * USBHostAudioV1SupportedFrequencies( uint8_t deviceAddress )
{
    uint8_t    i;
    
    // Find the correct device.
    for (i=0; (i<USB_MAX_AUDIO_DEVICES) && (deviceInfoAudioV1[i].ID.deviceAddress != deviceAddress); i++);
    if (i == USB_MAX_AUDIO_DEVICES)
    {
        return NULL;
    }

    #if defined( __PIC32MX__)
        return (uint8_t *)((UINT32)deviceInfoAudioV1[i].pFormatTypeDescriptor + (UINT32)7);
    #else
        return (uint8_t *)((uint16_t)deviceInfoAudioV1[i].pFormatTypeDescriptor + (uint16_t)7);
    #endif
}


/****************************************************************************
  Function:
    void USBHostAudioV1TerminateTransfer( uint8_t deviceAddress )

  Summary:
    This function terminates an audio stream.  

  Description:
    This function terminates an audio stream.  It does not change the device's
    selected interface.  The application may wish to call 
    USBHostAudioV1SetInterfaceZeroBandwidth() after this function to set
    the device to the zero bandwidth interface.
    
    Between terminating one audio stream and starting another, the application
    should call USBHostIsochronousBuffersReset() to reset the data buffers.  
    This is done from the application layer rather than from this function, so
    the application can process all received audio data. 

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address

  Returns:
    None

  Remarks:
    None
  ***************************************************************************/

void USBHostAudioV1TerminateTransfer( uint8_t deviceAddress )
{
    uint8_t    i;

    // Find the correct device.
    for (i=0; (i<USB_MAX_AUDIO_DEVICES) && (deviceInfoAudioV1[i].ID.deviceAddress != deviceAddress); i++);
    if (i == USB_MAX_AUDIO_DEVICES)
    {
        return;
    }

    // Terminate any endpoint tranfers that are occurring.
    USBHostTerminateTransfer( deviceInfoAudioV1[i].ID.deviceAddress, deviceInfoAudioV1[i].endpointAudioStream );

    return;
}

// *****************************************************************************
// *****************************************************************************
// Section: Host Stack Interface Functions
// *****************************************************************************
// *****************************************************************************

/****************************************************************************
  Function:
    bool USBHostAudioV1DataEventHandler( uint8_t address, USB_EVENT event,
                            void *data, uint32_t size )

  Summary:
    This function is the data event handler for this client driver.

  Description:
    This function is the data event handler for this client driver.  It is called
    by the host layer when isochronous data events occur.

  Precondition:
    The device has been initialized.

  Parameters:
    uint8_t address    - Address of the device
    USB_EVENT event - Event that has occurred
    void *data      - Pointer to data pertinent to the event
    uint16_t size       - Size of the data

  Return Values:
    true   - Event was handled
    false  - Event was not handled

  Remarks:
    The client driver does not need to process the data.  Just pass the 
    translated event up to the application layer.
  ***************************************************************************/

bool USBHostAudioV1DataEventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size )
{
    if (event == EVENT_DATA_ISOC_READ)
    {
        return USB_HOST_APP_DATA_EVENT_HANDLER( address, EVENT_AUDIO_STREAM_RECEIVED, data, size );
    }
    else
    {
        return false;
    }        
}

    
/****************************************************************************
  Function:
    bool USBHostAudioV1Initialize( uint8_t address, uint32_t flags, uint8_t clientDriverID )

  Summary:
    This function is the initialization routine for this client driver.

  Description:
    This function is the initialization routine for this client driver.  It
    is called by the host layer when the USB device is being enumerated.

  Precondition:
    None

  Parameters:
    uint8_t address        - Address of the new device
    uint32_t flags         - Initialization flags
    uint8_t clientDriverID - ID to send when issuing a Device Request via
                            USBHostIssueDeviceRequest() or USBHostSetDeviceConfiguration().

  Return Values:
    true   - We can support the device.
    false  - We cannot support the device.

  Remarks:
    None
  ***************************************************************************/

bool USBHostAudioV1Initialize( uint8_t address, uint32_t flags, uint8_t clientDriverID )
{
    uint8_t   *descriptor;
    uint8_t    device;
    uint16_t    i;
    uint16_t    saveIndex;


    // See if the device has already been initialized, and therefore is 
    // already in the audio device table.  If we change the configuration, the 
    // host driver will call the Initialize function again.
    for (device = 0; (device < USB_MAX_AUDIO_DEVICES) && (deviceInfoAudioV1[device].ID.deviceAddress != address); device++);
    if (device < USB_MAX_AUDIO_DEVICES)
    {
        return true;
    }

    // Find the free slot in the table.  If we cannot find one, kick off the device.
    for (device = 0; (device < USB_MAX_AUDIO_DEVICES) && (deviceInfoAudioV1[device].ID.deviceAddress != 0); device++);
    if (device == USB_MAX_AUDIO_DEVICES)
    {
        // Kick off the device
        return false;
    }

    // Fill in the VID and PID before we lose the device descriptor pointer.
    // It's not valid unless deviceAddress is non-zero.
    descriptor = USBHostGetDeviceDescriptor( address );
    deviceInfoAudioV1[device].ID.vid = ((USB_DEVICE_DESCRIPTOR *)descriptor)->idVendor;
    deviceInfoAudioV1[device].ID.pid = ((USB_DEVICE_DESCRIPTOR *)descriptor)->idProduct;
    
    // Get ready to parse the configuration descriptor.
    descriptor = USBHostGetCurrentConfigurationDescriptor( address );

    i = 0;

    // Find the next interface descriptor.
    while (i < ((USB_CONFIGURATION_DESCRIPTOR *)descriptor)->wTotalLength)
    {
        // See if we are pointing to an interface descriptor.
        if (descriptor[i+1] == USB_DESCRIPTOR_INTERFACE)
        {
            // See if the interface is a streaming audio interface.  Protocol is not used
            if ((descriptor[i+5] == USB_CLASS_AUDIO) && (descriptor[i+6] == USB_SUBCLASS_AUDIO_STREAMING))
            {
                // Record the interface number.
                deviceInfoAudioV1[device].interface = descriptor[i+2];

                if (descriptor[i+4] == 0)
                {
                    // If the number of endpoints is 0, then this is the zero bandwidth configuration.
                    deviceInfoAudioV1[device].settingZeroBandwidth = descriptor[i+3];
                    deviceInfoAudioV1[device].flags.bfFoundZeroBandwidthSetting = 1;
                }
                else
                {
                    // If the number of endpoints is not 0, then this is the full bandwidth configuration.
                    deviceInfoAudioV1[device].settingFullBandwidth = descriptor[i+3];
                    deviceInfoAudioV1[device].flags.bfFoundFullBandwidthSetting = 1;

                    // Initialize variables prior to scanning.
                    i += descriptor[i];
                    saveIndex = i;

                    // Look for an audio stream endpoint.
                    while ((deviceInfoAudioV1[device].endpointAudioStream == 0) &&
                           (i < ((USB_CONFIGURATION_DESCRIPTOR *)descriptor)->wTotalLength))
                    {
                        if (descriptor[i+1] == USB_DESCRIPTOR_ENDPOINT)
                        {
                            if ((descriptor[i+3] & 0x03) == 0x01) // Isochronous
                            {
                                deviceInfoAudioV1[device].endpointAudioStream    = descriptor[i+2];
                                deviceInfoAudioV1[device].ID.audioDataPacketSize = (descriptor[i+5] << 8) + descriptor[i+4];
                            }
                        }
                        i += descriptor[i];
                    }

                    // Look for the CS_INTERFACE, FORMAT_TYPE descriptor
                    i = saveIndex;
                    while ((deviceInfoAudioV1[device].pFormatTypeDescriptor == NULL) &&
                           (i < ((USB_CONFIGURATION_DESCRIPTOR *)descriptor)->wTotalLength))
                    {
                        if ((descriptor[i+1] == USB_AUDIO_DESCRIPTOR_INTERFACE) &&
                            (descriptor[i+2] == USB_AUDIO_AS_IF_DESCRIPTOR_FORMAT_TYPE))
                        {
                            if (descriptor[i+3] == USB_AUDIO_FORMAT_TYPE_I) // iPods should be only this format.
                            {
                                deviceInfoAudioV1[device].pFormatTypeDescriptor = &(descriptor[i]);
                            }
                        }
                        i += descriptor[i];
                    }

                    // Other audio devices may need to check more parameters.
                    // Since we support only iPods, we do not have to check
                    // data format, etc.

                }

                // Make sure we've found everything we need.
                if ((deviceInfoAudioV1[device].flags.bfFoundZeroBandwidthSetting == 1) &&
                    (deviceInfoAudioV1[device].flags.bfFoundFullBandwidthSetting == 1) &&
                    (deviceInfoAudioV1[device].pFormatTypeDescriptor != NULL) &&
                    (deviceInfoAudioV1[device].endpointAudioStream != 0))
                {
                    // Initialize the device information.
                    deviceInfoAudioV1[device].ID.deviceAddress  = address;
                    deviceInfoAudioV1[device].ID.clientDriverID = clientDriverID;

                    // Tell the application layer that we have a device.
                    USB_HOST_APP_EVENT_HANDLER( deviceInfoAudioV1[device].ID.deviceAddress, EVENT_AUDIO_ATTACH, &(deviceInfoAudioV1[device].ID), sizeof(USB_AUDIO_V1_DEVICE_ID) );

                    return true;
                }
            }
        }

        // Jump to the next descriptor in this configuration.
        i += descriptor[i];
    }

    // This configuration is not valid for an audio device.
    return false;
}


/****************************************************************************
  Function:
    bool USBHostAudioV1EventHandler( uint8_t address, USB_EVENT event,
                            void *data, uint32_t size )

  Summary:
    This function is the event handler for this client driver.

  Description:
    This function is the event handler for this client driver.  It is called
    by the host layer when various events occur.

  Precondition:
    The device has been initialized.

  Parameters:
    uint8_t address    - Address of the device
    USB_EVENT event - Event that has occurred
    void *data      - Pointer to data pertinent to the event
    uint16_t size       - Size of the data

  Return Values:
    true   - Event was handled
    false  - Event was not handled

  Remarks:
    None
  ***************************************************************************/

bool USBHostAudioV1EventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size )
{
    uint8_t    i;

    switch (event)
    {
        case EVENT_NONE:             // No event occured (NULL event)
            return true;
            break;

        case EVENT_DETACH:           // USB cable has been detached (data: uint8_t, address of device)
            // Find the device in the table.  If found, clear the important fields.
            for (i=0; (i<USB_MAX_AUDIO_DEVICES) && (deviceInfoAudioV1[i].ID.deviceAddress != address); i++);
            if (i < USB_MAX_AUDIO_DEVICES)
            {
                // Notify that application that the device has been detached.
                USB_HOST_APP_EVENT_HANDLER( address, EVENT_AUDIO_DETACH,
                    &deviceInfoAudioV1[i].ID.deviceAddress, sizeof(uint8_t) );

                deviceInfoAudioV1[i].ID.deviceAddress       = 0;
                deviceInfoAudioV1[i].flags.val              = 0;
                deviceInfoAudioV1[i].endpointAudioStream    = 0;
                deviceInfoAudioV1[i].pFormatTypeDescriptor  = NULL;
            }
            return true;
            break;

        case EVENT_TRANSFER:         // A USB transfer has completed
            for (i=0; (i<USB_MAX_AUDIO_DEVICES) && (deviceInfoAudioV1[i].ID.deviceAddress != address); i++) {}
            if (i == USB_MAX_AUDIO_DEVICES)
            {
                #ifdef DEBUG_MODE
                    UART2PrintString( "AUD: Unknown device\r\n" );
                #endif
                return false;
            }

            // The transfer event comes with data of the type HOST_TRANSFER_DATA.
            
            if (((HOST_TRANSFER_DATA *)data)->bEndpointAddress == 0)
            {
                if (deviceInfoAudioV1[i].flags.bfSettingInterface)
                {
                    // Either the full or the zero bandwidth interface has been set.
                    deviceInfoAudioV1[i].flags.bfSettingInterface = 0;
                    USB_HOST_APP_EVENT_HANDLER( i, EVENT_AUDIO_INTERFACE_SET, NULL, ((HOST_TRANSFER_DATA *)data)->bErrorCode );
                }    
                if (deviceInfoAudioV1[i].flags.bfSettingFrequency)
                {
                    // The sampling frequency has been set.
                    deviceInfoAudioV1[i].flags.bfSettingFrequency = 0;
                    USB_HOST_APP_EVENT_HANDLER( i, EVENT_AUDIO_FREQUENCY_SET, data, size );
                }    
            }
            #ifdef USB_ENABLE_ISOC_TRANSFER_EVENT
            else if (((HOST_TRANSFER_DATA *)data)->bEndpointAddress == deviceInfoAudioV1[i].endpointAudioStream)
            {
                // If we received streaming audio data, pass the event up to the application.
                // It's only one more byte of information more than they need (bmAttributes).
                USB_HOST_APP_EVENT_HANDLER( i, EVENT_AUDIO_RECEIVE_STREAM, data, size );
            }    
            #endif
            break;

        case EVENT_BUS_ERROR:
            for (i=0; (i<USB_MAX_AUDIO_DEVICES) && (deviceInfoAudioV1[i].ID.deviceAddress != address); i++) {}
            if (i == USB_MAX_AUDIO_DEVICES)
            {
                return false;
            }

            // The bus error event comes with data of the type HOST_TRANSFER_DATA.
            
            if (((HOST_TRANSFER_DATA *)data)->bEndpointAddress == 0)
            {
                ((HOST_TRANSFER_DATA *)data)->bErrorCode = USB_AUDIO_COMMAND_FAILED;
                if (deviceInfoAudioV1[i].flags.bfSettingInterface)
                {
                    // The interface could not be set.
                    deviceInfoAudioV1[i].flags.bfSettingInterface = 0;
                    USB_HOST_APP_EVENT_HANDLER( i, EVENT_AUDIO_INTERFACE_SET, NULL, ((HOST_TRANSFER_DATA *)data)->bErrorCode );
                }    
                if (deviceInfoAudioV1[i].flags.bfSettingFrequency)
                {
                    // The sampling frequency could not be set.
                    deviceInfoAudioV1[i].flags.bfSettingFrequency = 0;
                    USB_HOST_APP_EVENT_HANDLER( i, EVENT_AUDIO_FREQUENCY_SET, data, size );
                }    
            }
            else if (((HOST_TRANSFER_DATA *)data)->bEndpointAddress == deviceInfoAudioV1[i].endpointAudioStream)
            {
                // Bus error on data stream.
                USB_HOST_APP_EVENT_HANDLER( i, EVENT_AUDIO_STREAM_RECEIVED, NULL, 0 );
            }    
            break;
        
        case EVENT_SOF:              // Start of frame - NOT NEEDED
        case EVENT_RESUME:           // Device-mode resume received
        case EVENT_SUSPEND:          // Device-mode suspend/idle event received
        case EVENT_RESET:            // Device-mode bus reset received
        case EVENT_STALL:            // A stall has occured
            return true;
            break;

        default:
            return false;
            break;
    }
    return false;
}


// *****************************************************************************
// *****************************************************************************
// Section: Internal Functions
// *****************************************************************************
// *****************************************************************************


