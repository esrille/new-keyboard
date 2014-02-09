//DOM-IGNORE-BEGIN
/*******************************************************************************
Software License Agreement

The software supplied herewith by Microchip Technology Incorporated
(the "Company") for its PICmicro(R) Microcontroller is intended and
supplied to you, the Company's customer, for use solely and
exclusively on Microchip PICmicro Microcontroller products. The
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

*******************************************************************************/
//DOM-IGNORE-END

//DOM-IGNORE-BEGIN
#ifndef _USBHOSTAUDIOV1_H_
#define _USBHOSTAUDIOV1_H_
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Constants
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// Section: Initialization Handler Flags
// *****************************************************************************

#define USB_AUDIO_IPOD_CHECK                1

// *****************************************************************************
// Section: Class Error Codes
// *****************************************************************************

#define AUDIO_COMMAND_PASSED                0x00    // Transfer was successful. Returned in dCSWStatus.
#define AUDIO_COMMAND_FAILED                0x01    // Transfer failed. Returned in dCSWStatus.

#define USB_AUDIO_ERROR                     USB_ERROR_CLASS_DEFINED                 // Error code offset.

#define USB_AUDIO_COMMAND_PASSED            USB_SUCCESS                             // Command was successful.
#define USB_AUDIO_COMMAND_FAILED            (USB_AUDIO_ERROR | 0x01)                // Command failed at the device.
#define USB_AUDIO_OUT_OF_MEMORY             (USB_AUDIO_ERROR | 0x02)                // No dynamic memory is available.
#define USB_AUDIO_DEVICE_NOT_FOUND          (USB_AUDIO_ERROR | 0x03)                // Device with the specified address is not available.
#define USB_AUDIO_DEVICE_BUSY               (USB_AUDIO_ERROR | 0x04)                // A transfer is currently in progress.
#define USB_AUDIO_ILLEGAL_REQUEST           (USB_AUDIO_ERROR | 0x05)                // Cannot perform requested operation.

// *****************************************************************************
// Section: Additional return values for USBHostMSDDeviceStatus (see USBHostDeviceStatus also)
// *****************************************************************************

#define USB_MSD_DEVICE_DETACHED             0x50    // Device is detached.
#define USB_MSD_INITIALIZING                0x51    // Device is initializing.
#define USB_MSD_NORMAL_RUNNING              0x52    // Device is running and available for data transfers.
#define USB_MSD_RESETTING_DEVICE            0x53    // Device is being reset.
#define USB_MSD_ERROR_STATE                 0x55    // Device is holding due to a MSD error.

// *****************************************************************************
// Section: Interface and Protocol Constants
// *****************************************************************************

#define DEVICE_CLASS_MASS_STORAGE           0x08    // Class code for Mass Storage.

#define DEVICE_SUBCLASS_RBC                 0x01    // SubClass code for Reduced Block Commands (not supported).
#define DEVICE_SUBCLASS_CD_DVD              0x02    // SubClass code for a CD/DVD drive (not supported).
#define DEVICE_SUBCLASS_TAPE_DRIVE          0x03    // SubClass code for a tape drive (not supported).
#define DEVICE_SUBCLASS_FLOPPY_INTERFACE    0x04    // SubClass code for a floppy disk interface (not supported).
#define DEVICE_SUBCLASS_REMOVABLE           0x05    // SubClass code for removable media (not supported).
#define DEVICE_SUBCLASS_SCSI                0x06    // SubClass code for a SCSI interface device (supported).

#define DEVICE_INTERFACE_PROTOCOL_BULK_ONLY 0x50    // Protocol code for Bulk-only mass storage.

// *****************************************************************************
// Section: Audio Event Definition
// *****************************************************************************

// If the application has not defined an offset for audio events, set it to 0.
#ifndef EVENT_AUDIO_OFFSET
    #define EVENT_AUDIO_OFFSET    0
#endif

    // No event occured (NULL event).
#define EVENT_AUDIO_NONE            EVENT_AUDIO_BASE + EVENT_AUDIO_OFFSET + 0   
    // An audio device has attached.  The returned data pointer points to a
    // USB_AUDIO_V1_DEVICE_ID structure.
#define EVENT_AUDIO_ATTACH          EVENT_AUDIO_BASE + EVENT_AUDIO_OFFSET + 1   
    // An audio device has detached.  The returned data pointer points to a
    // uint8_t with the previous address of the detached device.
#define EVENT_AUDIO_DETACH          EVENT_AUDIO_BASE + EVENT_AUDIO_OFFSET + 2   
    // An audio stream data packet has been received.  The returned data 
    // pointer points to a HOST_TRANSFER_DATA structure, with information about
    // the most recent transfer.  One event will be returned for each transfer,
    // so the application will know how much data was actually received in
    // each transfer.  If there was a bus error, both the returned data pointer
    // and the size will be zero.
#define EVENT_AUDIO_STREAM_RECEIVED EVENT_AUDIO_BASE + EVENT_AUDIO_OFFSET + 3
    // This event is returned after the sampling frequency is set via
    // USBHostAudioV1SetSamplingFrequency().  The returned data pointer points
    // to a HOST_TRANSFER_DATA structure, with the error code for this request.
#define EVENT_AUDIO_FREQUENCY_SET   EVENT_AUDIO_BASE + EVENT_AUDIO_OFFSET + 4 
    // This event is returned after the full or zero bandwidth interface has
    // been set.  The returned data pointer is NULL, but the size is the
    // error code from the transfer.
#define EVENT_AUDIO_INTERFACE_SET   EVENT_AUDIO_BASE + EVENT_AUDIO_OFFSET + 5 

// *****************************************************************************
// *****************************************************************************
// Section: Data Structures
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Audio Device ID Information

This structure contains identification information about an attached device.
*/
typedef struct _USB_AUDIO_V1_DEVICE_ID
{
    uint16_t                            vid;                    // Vendor ID of the device
    uint16_t                            pid;                    // Product ID of the device
    uint16_t                            audioDataPacketSize;    // uint8_ts transferred in one isochronous transfer.
    uint8_t                            deviceAddress;          // Address of the device on the USB
    uint8_t                            clientDriverID;         // Client driver ID for device requests.
} USB_AUDIO_V1_DEVICE_ID;


// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes and Macro Functions
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
        ISOCHRONOUS_DATA *pIsochronousData );


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

uint8_t USBHostAudioV1SetInterfaceFullBandwidth( uint8_t deviceAddress );


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

uint8_t USBHostAudioV1SetInterfaceZeroBandwidth( uint8_t deviceAddress );


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
    uint8_t *frequency     - Pointer to three uint8_ts that specify the desired
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

uint8_t USBHostAudioV1SetSamplingFrequency( uint8_t deviceAddress, uint8_t *frequency );


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
    The first uint8_t of this list is the number of supported frequencies.  Each
    supported frequency is then listed, with three uint8_ts for each frequency.

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

uint8_t * USBHostAudioV1SupportedFrequencies( uint8_t deviceAddress );


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

void    USBHostAudioV1TerminateTransfer( uint8_t deviceAddress );



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
    The client driver does not need to process the data.  Just pass the event 
    up to the application layer.
  ***************************************************************************/

bool USBHostAudioV1DataEventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size );


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

bool USBHostAudioV1Initialize( uint8_t address, uint32_t flags, uint8_t clientDriverID );


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

bool USBHostAudioV1EventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size );


#endif
