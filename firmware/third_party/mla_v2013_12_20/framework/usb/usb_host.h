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

#ifndef __USBHOST_H__
#define __USBHOST_H__
//DOM-IGNORE-END

#include <limits.h>

// *****************************************************************************
// *****************************************************************************
// Section: Host Firmware Version
// *****************************************************************************
// *****************************************************************************

#define USB_HOST_FW_MAJOR_VER   1       // Firmware version, major release number.
#define USB_HOST_FW_MINOR_VER   0       // Firmware version, minor release number.
#define USB_HOST_FW_DOT_VER     0       // Firmware version, dot release number.


// *****************************************************************************
// *****************************************************************************
// Section: Set Default Configuration Constants
// *****************************************************************************
// *****************************************************************************

#ifndef USB_NUM_BULK_NAKS
    #define USB_NUM_BULK_NAKS       10000   // Define how many NAK's are allowed
                                            // during a bulk transfer before erroring.
#endif

#ifndef USB_NUM_COMMAND_TRIES
    #define USB_NUM_COMMAND_TRIES       3   // During enumeration, define how many
                                            // times each command will be tried before
                                            // giving up and resetting the device.
#endif

#ifndef USB_NUM_CONTROL_NAKS
    #define USB_NUM_CONTROL_NAKS        20  // Define how many NAK's are allowed
                                            // during a control transfer before erroring.
#endif

#ifndef USB_NUM_ENUMERATION_TRIES
    #define USB_NUM_ENUMERATION_TRIES   3   // Define how many times the host will try
                                            // to enumerate the device before giving
                                            // up and setting the state to DETACHED.
#endif

#ifndef USB_NUM_INTERRUPT_NAKS
    #define USB_NUM_INTERRUPT_NAKS      3   // Define how many NAK's are allowed
                                            // during an interrupt OUT transfer before
                                            // erroring.  Interrupt IN transfers that
                                            // are NAK'd are terminated without error.
#endif


#ifndef USB_INITIAL_VBUS_CURRENT
    #error The application must define USB_INITIAL_VBUS_CURRENT as 100 mA for Host or 8-100 mA for OTG.
#endif

#if defined (USB_SUPPORT_HOST)
    #if defined (USB_SUPPORT_OTG)
        #if (USB_INITIAL_VBUS_CURRENT < 8/2) || (USB_INITIAL_VBUS_CURRENT > 100/2)
            #warning USB_INITIAL_VBUS_CURRENT is in violation of the USB specification.
        #endif
    #else
        #if (USB_INITIAL_VBUS_CURRENT != 100/2)
            #warning USB_INITIAL_VBUS_CURRENT is in violation of the USB specification.
        #endif
    #endif
#endif


// *****************************************************************************
// *****************************************************************************
// Section: USB Constants
// *****************************************************************************
// *****************************************************************************

// Section: Values for USBHostIssueDeviceRequest(), dataDirection

#define USB_DEVICE_REQUEST_SET                  0       // USBHostIssueDeviceRequest() will set information.
#define USB_DEVICE_REQUEST_GET                  1       // USBHostIssueDeviceRequest() will get information.

// Section: Dummy Device ID's

#define USB_ROOT_HUB                            255     // Invalid Device ID used to indicate the root hub.


// *****************************************************************************
// *****************************************************************************
// Section: USB Data Structures
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Transfer Attributes

This structure describes the transfer attributes of an endpoint.
*/
typedef union
{
   uint8_t     val;                            //
   struct
   {
       uint8_t bfTransferType          : 2;    // See USB_TRANSFER_TYPE_* for values.
       uint8_t bfSynchronizationType   : 2;    // For isochronous endpoints only.
       uint8_t bfUsageType             : 2;    // For isochronous endpoints only.
   };
} TRANSFER_ATTRIBUTES;


// *****************************************************************************
/* Host Transfer Information

This structure is used when the event handler is used to notify the upper layer
of transfer completion.
*/

typedef struct _HOST_TRANSFER_DATA
{
   uint32_t                dataCount;          // Count of bytes transferred.
   uint8_t                *pUserData;          // Pointer to transfer data.
   uint8_t                 bEndpointAddress;   // Transfer endpoint.
   uint8_t                 bErrorCode;         // Transfer error code.
   TRANSFER_ATTRIBUTES  bmAttributes;       // INTERNAL USE ONLY - Endpoint transfer attributes.
   uint8_t                 clientDriver;       // INTERNAL USE ONLY - Client driver index for sending the event.
} HOST_TRANSFER_DATA;


// *****************************************************************************
/* Isochronous Data Buffer

Isochronous data transfers are continuous, until they are explicitly terminated.
The maximum transfer size is given in the endpoint descriptor, but a single
transfer may contain less data than the maximum.  Also, the USB peripheral can
store data to RAM in a linear fashion only.  Therefore, we cannot use a simple
circular buffer for the data.  Instead, the application or client driver must
allocate multiple independent data buffers.  These buffers must be the
maximum transfer size.  This structure contains a pointer to an allocated
buffer, plus the valid data length of the buffer.
*/

typedef struct _ISOCHRONOUS_DATA_BUFFER
{
    uint8_t                *pBuffer;               // Data buffer pointer.
    uint16_t                dataLength;             // Amount of valid data in the buffer.
    uint8_t                bfDataLengthValid : 1;  // dataLength value is valid.
} ISOCHRONOUS_DATA_BUFFER;


// *****************************************************************************
/* Isochronous Data

Isochronous data transfers are continuous, until they are explicitly terminated.
This requires a tighter integration between the host layer and the application
layer to manage the streaming data.

If an application uses isochronous transfers, it must allocate one variable
of type ISOCHRONOUS_DATA for each concurrent transfer.  When the device 
attaches, the client driver must inform the application layer of the maximum
transfer size.  At this point, the application must allocate space for the 
data buffers, and set the data buffer points in this structure to point to them.
*/

#if !defined( USB_MAX_ISOCHRONOUS_DATA_BUFFERS )
    #define USB_MAX_ISOCHRONOUS_DATA_BUFFERS    2
#endif
#if USB_MAX_ISOCHRONOUS_DATA_BUFFERS < 2
    #error At least two buffers must be defined for isochronous data.
#endif

typedef struct _ISOCHRONOUS_DATA
{
    uint8_t    totalBuffers;       // Total number of buffers available.
    uint8_t    currentBufferUSB;   // The current buffer the USB peripheral is accessing.
    uint8_t    currentBufferUser;  // The current buffer the user is reading/writing.
    uint8_t    *pDataUser;         // User pointer for accessing data.
    
    ISOCHRONOUS_DATA_BUFFER buffers[USB_MAX_ISOCHRONOUS_DATA_BUFFERS];  // Data buffer information.
} ISOCHRONOUS_DATA;
    

// *****************************************************************************
/* Targeted Peripheral List

This structure is used to define the devices that this host can support.  If the
host is a USB Embedded Host or Dual Role Device that does not support OTG, the
TPL may contain both specific devices and generic classes.  If the host supports
OTG, then the TPL may contain ONLY specific devices.
*/
typedef struct _USB_TPL
{
    union
    {
        uint32_t       val;                        //
        struct
        {
            uint16_t    idVendor;                   // Vendor ID
            uint16_t    idProduct;                  // Product ID
        };
        struct
        {
            uint8_t    bClass;                     // Class ID
            uint8_t    bSubClass;                  // SubClass ID
            uint8_t    bProtocol;                  // Protocol ID
        };
    } device;                                   //
    uint8_t            bConfiguration;             // Initial device configuration
    uint8_t            ClientDriver;               // Index of client driver in the Client Driver table
    union
    {
        uint8_t         val;                       //
        struct
        {
            uint8_t     bfAllowHNP             :1;     // Is HNP allowed?
            uint8_t     bfIsClassDriver        :1;     // Client driver is a class-level driver
            uint8_t     bfSetConfiguration     :1;     // bConfiguration is valid
            uint8_t     bfIgnoreProtocol       :1;
            uint8_t     bfIgnoreSubClass       :1;
            uint8_t     bfIgnoreClass          :1;
            uint8_t     bfIgnorePID            :1;
            uint8_t     bfEP0OnlyCustomDriver  :1;
        };
    } flags;                                    //
} USB_TPL;

// Section: TPL Initializers
#define INIT_VID_PID(v,p)   {((v)|((p)<<16))}           // Set VID/PID support in the TPL.
#define INIT_CL_SC_P(c,s,p) {((c)|((s)<<8)|((p)<<16))}  // Set class support in the TPL (non-OTG only).

// Section: TPL Flags
#define TPL_ALLOW_HNP               0x01                    // Bitmask for Host Negotiation Protocol.
#define TPL_CLASS_DRV               0x02                    // Bitmask for class driver support.
#define TPL_SET_CONFIG              0x04                    // Bitmask for setting the configuration.
#define TPL_IGNORE_PROTOCOL         0x08                    // Bitmask for ignoring the protocol of a CL/SC/P driver
#define TPL_IGNORE_SUBCLASS         0x10                    // Bitmask for ignoring the subclass of a CL/SC/P driver
#define TPL_IGNORE_CLASS            0x20                    // Bitmask for ignoring the class of a CL/SC/P driver
#define TPL_IGNORE_PID              0x40                    // Bitmask for ignoring the PID of a VID/PID driver
#define TPL_EP0_ONLY_CUSTOM_DRIVER  0x80                    // Bitmask to let a custom driver gain EP0 only and allow other interfaces to use standard drivers


// *****************************************************************************
// *****************************************************************************
// Section: USB Host - Client Driver Interface
// *****************************************************************************
// *****************************************************************************

/****************************************************************************
  Function:
    bool (*USB_CLIENT_EVENT_HANDLER) ( uint8_t address, USB_EVENT event,
                        void *data, uint32_t size )

  Summary:
    This is a typedef to use when defining a client driver event handler.

  Description:
    This data type defines a pointer to a call-back function that must be
    implemented by a client driver if it needs to be aware of events on the
    USB.  When an event occurs, the Host layer will call the client driver
    via this pointer to handle the event.  Events are identified by the
    "event" parameter and may have associated data.  If the client driver was
    able to handle the event, it should return true.  If not (or if
    additional processing is required), it should return false.

  Precondition:
    The client must have been initialized.

  Parameters:
    uint8_t address    - Address of device where event occurred
    USB_EVENT event - Identifies the event that occured
    void *data      - Pointer to event-specific data
    uint32_t size      - Size of the event-specific data

  Return Values:
    true    - The event was handled
    false   - The event was not handled

  Remarks:
    The application may also implement an event handling routine if it
    requires knowledge of events.  To do so, it must implement a routine that
    matches this function signature and define the USB_HOST_APP_EVENT_HANDLER
    macro as the name of that function.
  ***************************************************************************/

typedef bool (*USB_CLIENT_EVENT_HANDLER) ( uint8_t address, USB_EVENT event, void *data, uint32_t size );


/****************************************************************************
  Function:
    bool (*USB_CLIENT_INIT)   ( uint8_t address, uint32_t flags, uint8_t clientDriverID )

  Summary:
    This is a typedef to use when defining a client driver initialization
    handler.

  Description:
    This routine is a call out from the host layer to a USB client driver.
    It is called when the system has been configured as a USB host and a new
    device has been attached to the bus.  Its purpose is to initialize and
    activate the client driver.

  Precondition:
    The device has been configured.

  Parameters:
    uint8_t address        - Device's address on the bus
    uint32_t flags         - Initialization flags
    uint8_t clientDriverID - ID to send when issuing a Device Request via
                            USBHostIssueDeviceRequest() or USBHostSetDeviceConfiguration().
                            
  Return Values:
    true    - Successful
    false   - Not successful

  Remarks:
    There may be multiple client drivers.  If so, the USB host layer will
    call the initialize routine for each of the clients that are in the
    selected configuration.
  ***************************************************************************/

typedef bool (*USB_CLIENT_INIT)   ( uint8_t address, uint32_t flags, uint8_t clientDriverID );


/****************************************************************************
  Function:
    bool USB_HOST_APP_EVENT_HANDLER ( uint8_t address, USB_EVENT event,
            void *data, uint32_t size )

  Summary:
    This is a typedef to use when defining the application level events
    handler.

  Description:
    This function is implemented by the application.  The function name can
    be anything - the macro USB_HOST_APP_EVENT_HANDLER must be set in
    usb_config.h to the name of the application function.

    In the application layer, this function is responsible for handling all
    application-level events that are generated by the stack.  See the
    enumeration USB_EVENT for a complete list of all events that can occur.
    Note that some of these events are intended for client drivers
    (e.g. EVENT_TRANSFER), while some are intended for for the application
    layer (e.g. EVENT_UNSUPPORTED_DEVICE).

    If the application can handle the event successfully, the function
    should return true.  For example, if the function receives the event
    EVENT_VBUS_REQUEST_POWER and the system can allocate that much power to
    an attached device, the function should return true.  If, however, the
    system cannot allocate that much power to an attached device, the
    function should return false.

  Precondition:
    None

  Parameters:
    uint8_t address        - Address of the USB device generating the event
    USB_EVENT event     - Event that occurred
    void *data          - Optional pointer to data for the event
    uint32_t size          - Size of the data pointed to by *data

  Return Values:
    true    - Event was processed successfully
    false   - Event was not processed successfully

  Remarks:
    If this function is not provided by the application, then all application
    events are assumed to function without error.
  ***************************************************************************/
#if defined( USB_HOST_APP_EVENT_HANDLER )
    bool USB_HOST_APP_EVENT_HANDLER ( uint8_t address, USB_EVENT event, void *data, uint32_t size );
#else
    // If the application does not provide an event handler, then we will
    // assume that all events function without error.
    #define USB_HOST_APP_EVENT_HANDLER(a,e,d,s) ((e==EVENT_OVERRIDE_CLIENT_DRIVER_SELECTION)?false:true)
#endif


/****************************************************************************
  Function:
    bool USB_HOST_APP_DATA_EVENT_HANDLER ( uint8_t address, USB_EVENT event,
            void *data, uint32_t size )

  Summary:
    This is a typedef to use when defining the application level data events
    handler.

  Description:
    This function is implemented by the application.  The function name can
    be anything - the macro USB_HOST_APP_EVENT_HANDLER must be set in
    usb_config.h to the name of the application function.

    In the application layer, this function is responsible for handling all
    application-level data events that are generated by the stack.  See the
    enumeration USB_EVENT for a complete list of all events that can occur.
    Note that only data events, such as EVENT_DATA_ISOC_READ, will be
    passed to this event handler.

    If the application can handle the event successfully, the function
    should return true.  

  Precondition:
    None

  Parameters:
    uint8_t address        - Address of the USB device generating the event
    USB_EVENT event     - Event that occurred
    void *data          - Optional pointer to data for the event
    uint32_t size          - Size of the data pointed to by *data

  Return Values:
    true    - Event was processed successfully
    false   - Event was not processed successfully

  Remarks:
    If this function is not provided by the application, then all application
    events are assumed to function without error.
  ***************************************************************************/
#if defined( USB_HOST_APP_DATA_EVENT_HANDLER )
    bool USB_HOST_APP_DATA_EVENT_HANDLER ( uint8_t address, USB_EVENT event, void *data, uint32_t size );
#else
    // If the application does not provide an event handler, then we will
    // assume that all events function without error.
    #define USB_HOST_APP_DATA_EVENT_HANDLER(a,e,d,s) true
#endif


// *****************************************************************************
/* Client Driver Table Structure

This structure is used to define an entry in the client-driver table.
Each entry provides the information that the Host layer needs to
manage a particular USB client driver, including pointers to the
interface routines that the Client Driver must implement.
 */

typedef struct _CLIENT_DRIVER_TABLE
{
    USB_CLIENT_INIT          Initialize;     // Initialization routine
    USB_CLIENT_EVENT_HANDLER EventHandler;   // Event routine
    
    #ifdef USB_HOST_APP_DATA_EVENT_HANDLER
    USB_CLIENT_EVENT_HANDLER DataEventHandler;  // Data Event routine
    #endif
    
    uint32_t                    flags;          // Initialization flags

} CLIENT_DRIVER_TABLE;


// *****************************************************************************
// *****************************************************************************
// Section: USB Host - Device Information Hooks
// *****************************************************************************
// *****************************************************************************

extern uint8_t                *pCurrentConfigurationDescriptor;    // Pointer to the current Configuration Descriptor of the attached device.
extern uint8_t                *pDeviceDescriptor;                  // Pointer to the Device Descriptor of the attached device.
extern USB_TPL              usbTPL[];                           // Application's Targeted Peripheral List.
extern CLIENT_DRIVER_TABLE  usbClientDrvTable[];                // Application's client driver table.


// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes and Macro Functions
// *****************************************************************************
// *****************************************************************************

/****************************************************************************
  Function:
    uint8_t USBHostClearEndpointErrors( uint8_t deviceAddress, uint8_t endpoint )

  Summary:
    This function clears an endpoint's internal error condition.

  Description:
    This function is called to clear the internal error condition of a device's
    endpoint.  It should be called after the application has dealt with the
    error condition on the device.  This routine clears internal status only;
    it does not interact with the device.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Address of device
    uint8_t endpoint       - Endpoint to clear error condition

  Return Values:
    USB_SUCCESS             - Errors cleared
    USB_UNKNOWN_DEVICE      - Device not found
    USB_ENDPOINT_NOT_FOUND  - Specified endpoint not found

  Remarks:
    None
  ***************************************************************************/

uint8_t    USBHostClearEndpointErrors( uint8_t deviceAddress, uint8_t endpoint );


/****************************************************************************
  Function:
    bool    USBHostDeviceSpecificClientDriver( uint8_t deviceAddress )

  Summary:
    This function indicates if the specified device has explicit client
    driver support specified in the TPL.

  Description:
    This function indicates if the specified device has explicit client
    driver support specified in the TPL.  It is used in client drivers'
    USB_CLIENT_INIT routines to indicate that the client driver should be
    used even though the class, subclass, and protocol values may not match
    those normally required by the class.  For example, some printing devices
    do not fulfill all of the requirements of the printer class, so their
    class, subclass, and protocol fields indicate a custom driver rather than
    the printer class.  But the printer class driver can still be used, with
    minor limitations.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Address of device

  Return Values:
    true    - This device is listed in the TPL by VID andPID, and has explicit
                client driver support.
    false   - This device is not listed in the TPL by VID and PID.

  Remarks:
    This function is used so client drivers can allow certain
    devices to enumerate.  For example, some printer devices indicate a custom
    class rather than the printer class, even though the device has only minor
    limitations from the full printer class.   The printer client driver will
    fail to initialize the device if it does not indicate printer class support
    in its interface descriptor.  The printer client driver could allow any
    device with an interface that matches the printer class endpoint
    configuration, but both printer and mass storage devices utilize one bulk
    IN and one bulk OUT endpoint.  So a mass storage device would be
    erroneously initialized as a printer device.  This function allows a
    client driver to know that the client driver support was specified
    explicitly in the TPL, so for this particular device only, the class,
    subclass, and protocol fields can be safely ignored.
  ***************************************************************************/

bool    USBHostDeviceSpecificClientDriver( uint8_t deviceAddress );


/****************************************************************************
  Function:
    uint8_t USBHostDeviceStatus( uint8_t deviceAddress )

  Summary:
    This function returns the current status of a device.

  Description:
    This function returns the current status of a device.  If the device is
    in a holding state due to an error, the error is returned.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address

  Return Values:
    USB_DEVICE_ATTACHED                 - Device is attached and running
    USB_DEVICE_DETACHED                 - No device is attached
    USB_DEVICE_ENUMERATING              - Device is enumerating
    USB_HOLDING_OUT_OF_MEMORY           - Not enough heap space available
    USB_HOLDING_UNSUPPORTED_DEVICE      - Invalid configuration or
                                            unsupported class
    USB_HOLDING_UNSUPPORTED_HUB         - Hubs are not supported
    USB_HOLDING_INVALID_CONFIGURATION   - Invalid configuration requested
    USB_HOLDING_PROCESSING_CAPACITY     - Processing requirement excessive
    USB_HOLDING_POWER_REQUIREMENT       - Power requirement excessive
    USB_HOLDING_CLIENT_INIT_ERROR       - Client driver failed to initialize
    USB_DEVICE_SUSPENDED                - Device is suspended
    Other                               - Device is holding in an error
                                            state. The return value
                                            indicates the error.

  Remarks:
    None
  ***************************************************************************/

uint8_t    USBHostDeviceStatus( uint8_t deviceAddress );


/****************************************************************************
  Function:
    uint8_t * USBHostGetCurrentConfigurationDescriptor( uint8_t deviceAddress )

  Description:
    This function returns a pointer to the current configuration descriptor
    of the requested device.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Address of device

  Returns:
    uint8_t *  - Pointer to the Configuration Descriptor.

  Remarks:
    This will need to be expanded to a full function when multiple device
    support is added.
  ***************************************************************************/

#define USBHostGetCurrentConfigurationDescriptor( deviceAddress) ( pCurrentConfigurationDescriptor )


/****************************************************************************
  Function:
    uint8_t * USBHostGetDeviceDescriptor( uint8_t deviceAddress )

  Description:
    This function returns a pointer to the device descriptor of the
    requested device.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Address of device

  Returns:
    uint8_t *  - Pointer to the Device Descriptor.

  Remarks:
    This will need to be expanded to a full function when multiple device
    support is added.
  ***************************************************************************/

#define USBHostGetDeviceDescriptor( deviceAddress )     ( pDeviceDescriptor )


/****************************************************************************
  Function:
    uint8_t USBHostGetStringDescriptor ( uint8_t deviceAddress,  uint8_t stringNumber,
                        uint8_t LangID, uint8_t *stringDescriptor, uint8_t stringLength, 
                        uint8_t clientDriverID )

  Summary:
    This routine initiates a request to obtains the requested string
    descriptor.

  Description:
    This routine initiates a request to obtains the requested string
    descriptor.  If the request cannot be started, the routine returns an
    error.  Otherwise, the request is started, and the requested string
    descriptor is stored in the designated location.

    Example Usage:
    <code>
    USBHostGetStringDescriptor(
        deviceAddress,
        stringDescriptorNum,
        LangID,
        stringDescriptorBuffer,
        sizeof(stringDescriptorBuffer),
        0xFF 
        );

    while(1)
    {
        if(USBHostTransferIsComplete( deviceAddress , 0, &errorCode, &byteCount))
        {
            if(errorCode)
            {
                //There was an error reading the string, bail out of loop
            }
            else
            {
                //String is located in specified buffer, do something with it.

                //The length of the string is both in the byteCount variable
                //  as well as the first byte of the string itself
            }
            break;
        }
        USBTasks();
    }
    </code>

  Precondition:
    None

  Parameters:
    deviceAddress       - Address of the device
    stringNumber        - Index of the desired string descriptor
    LangID              - The Language ID of the string to read (should be 0
                            if trying to read the language ID list
    *stringDescriptor   - Pointer to where to store the string.
    stringLength        - Maximum length of the returned string.
    clientDriverID      - Client driver to return the completion event to.
    
  Return Values:
    USB_SUCCESS             - The request was started successfully.
    USB_UNKNOWN_DEVICE      - Device not found
    USB_INVALID_STATE       - We must be in a normal running state.
    USB_ENDPOINT_BUSY       - The endpoint is currently processing a request.

  Remarks:
    The returned string descriptor will be in the exact format as obtained
    from the device.  The length of the entire descriptor will be in the
    first byte, and the descriptor type will be in the second.  The string
    itself is represented in UNICODE.  Refer to the USB 2.0 Specification
    for more information about the format of string descriptors.
  ***************************************************************************/

#define USBHostGetStringDescriptor( deviceAddress, stringNumber, LangID, stringDescriptor, stringLength, clientDriverID )                   \
        USBHostIssueDeviceRequest( deviceAddress, USB_SETUP_DEVICE_TO_HOST | USB_SETUP_TYPE_STANDARD | USB_SETUP_RECIPIENT_DEVICE,   \
                USB_REQUEST_GET_DESCRIPTOR, (USB_DESCRIPTOR_STRING << 8) | stringNumber,                                            \
                LangID, stringLength, stringDescriptor, USB_DEVICE_REQUEST_GET, clientDriverID )


/****************************************************************************
  Function:
    bool USBHostInit(  unsigned long flags  )

  Summary:
    This function initializes the variables of the USB host stack.

  Description:
    This function initializes the variables of the USB host stack.  It does
    not initialize the hardware.  The peripheral itself is initialized in one
    of the state machine states.  Therefore, USBHostTasks() should be called
    soon after this function.

  Precondition:
    None

  Parameters:
    flags - reserved

  Return Values:
    true  - Initialization successful
    false - Could not allocate memory.

  Remarks:
    If the endpoint list is empty, an entry is created in the endpoint list
    for EP0.  If the list is not empty, free all allocated memory other than
    the EP0 node.  This allows the routine to be called multiple times by the
    application.
  ***************************************************************************/

bool USBHostInit(  unsigned long flags  );


/****************************************************************************
  Function:
    bool USBHostIsochronousBuffersCreate( ISOCHRONOUS_DATA * isocData, 
            uint8_t numberOfBuffers, uint16_t bufferSize )
    
  Description:
    This function initializes the isochronous data buffer information and
    allocates memory for each buffer.  This function will not allocate memory
    if the buffer pointer is not NULL.

  Precondition:
    None

  Parameters:
    None

  Return Values:
    true    - All buffers are allocated successfully.
    false   - Not enough heap space to allocate all buffers - adjust the 
                project to provide more heap space.

  Remarks:
    This function is available only if USB_SUPPORT_ISOCHRONOUS_TRANSFERS
    is defined in usb_config.h.
***************************************************************************/

#ifdef USB_SUPPORT_ISOCHRONOUS_TRANSFERS
bool USBHostIsochronousBuffersCreate( ISOCHRONOUS_DATA * isocData, uint8_t numberOfBuffers, uint16_t bufferSize );
#endif


/****************************************************************************
  Function:
    void USBHostIsochronousBuffersDestroy( ISOCHRONOUS_DATA * isocData, uint8_t numberOfBuffers )
    
  Description:
    This function releases all of the memory allocated for the isochronous
    data buffers.  It also resets all other information about the buffers.

  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    This function is available only if USB_SUPPORT_ISOCHRONOUS_TRANSFERS
    is defined in usb_config.h.
***************************************************************************/

#ifdef USB_SUPPORT_ISOCHRONOUS_TRANSFERS
void USBHostIsochronousBuffersDestroy( ISOCHRONOUS_DATA * isocData, uint8_t numberOfBuffers );
#endif


/****************************************************************************
  Function:
    void USBHostIsochronousBuffersReset( ISOCHRONOUS_DATA * isocData, uint8_t numberOfBuffers )
    
  Description:
    This function resets all the isochronous data buffers.  It does not do 
    anything with the space allocated for the buffers.

  Precondition:
    None

  Parameters:
    None

  Returns:
    None

  Remarks:
    This function is available only if USB_SUPPORT_ISOCHRONOUS_TRANSFERS
    is defined in usb_config.h.
***************************************************************************/

#ifdef USB_SUPPORT_ISOCHRONOUS_TRANSFERS
void USBHostIsochronousBuffersReset( ISOCHRONOUS_DATA * isocData, uint8_t numberOfBuffers );
#endif


/****************************************************************************
  Function:
    uint8_t USBHostIssueDeviceRequest( uint8_t deviceAddress, uint8_t bmRequestType,
                    uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength,
                    uint8_t *data, uint8_t dataDirection, uint8_t clientDriverID )

  Summary:
    This function sends a standard device request to the attached device.

  Description:
    This function sends a standard device request to the attached device.
    The user must pass in the parameters of the device request.  If there is
    input or output data associated with the request, a pointer to the data
    must be provided.  The direction of the associated data (input or output)
    must also be indicated.

    This function does no special processing in regards to the request except
    for three requests.  If SET INTERFACE is sent, then DTS is reset for all
    endpoints.  If CLEAR FEATURE (ENDPOINT HALT) is sent, then DTS is reset
    for that endpoint.  If SET CONFIGURATION is sent, the request is aborted
    with a failure.  The function USBHostSetDeviceConfiguration() must be
    called to change the device configuration, since endpoint definitions may
    change.

  Precondition:
    The host state machine should be in the running state, and no reads or
    writes to EP0 should be in progress.

  Parameters:
    uint8_t deviceAddress  - Device address
    uint8_t bmRequestType  - The request type as defined by the USB
                            specification.
    uint8_t bRequest       - The request as defined by the USB specification.
    uint16_t wValue         - The value for the request as defined by the USB
                            specification.
    uint16_t wIndex         - The index for the request as defined by the USB
                            specification.
    uint16_t wLength        - The data length for the request as defined by the
                            USB specification.
    uint8_t *data          - Pointer to the data for the request.
    uint8_t dataDirection  - USB_DEVICE_REQUEST_SET or USB_DEVICE_REQUEST_GET
    uint8_t clientDriverID - Client driver to send the event to.

  Return Values:
    USB_SUCCESS                 - Request processing started
    USB_UNKNOWN_DEVICE          - Device not found
    USB_INVALID_STATE           - The host must be in a normal running state
                                    to do this request
    USB_ENDPOINT_BUSY           - A read or write is already in progress
    USB_ILLEGAL_REQUEST         - SET CONFIGURATION cannot be performed with
                                    this function.

  Remarks:
    DTS reset is done before the command is issued.
  ***************************************************************************/

uint8_t    USBHostIssueDeviceRequest( uint8_t deviceAddress, uint8_t bmRequestType, uint8_t bRequest,
             uint16_t wValue, uint16_t wIndex, uint16_t wLength, uint8_t *data, uint8_t dataDirection, 
             uint8_t clientDriverID );


/****************************************************************************
  Function:
    uint8_t USBHostRead( uint8_t deviceAddress, uint8_t endpoint, uint8_t *pData,
                        uint32_t size )
  Summary:
    This function initiates a read from the attached device.

  Description:
    This function initiates a read from the attached device.

    If the endpoint is isochronous, special conditions apply.  The pData and
    size parameters have slightly different meanings, since multiple buffers
    are required.  Once started, an isochronous transfer will continue with
    no upper layer intervention until USBHostTerminateTransfer() is called.
    The ISOCHRONOUS_DATA_BUFFERS structure should not be manipulated until
    the transfer is terminated.

    To clarify parameter usage and to simplify casting, use the macro
    USBHostReadIsochronous() when reading from an isochronous endpoint.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address
    uint8_t endpoint       - Endpoint number
    uint8_t *pData         - Pointer to where to store the data. If the endpoint
                            is isochronous, this points to an
                            ISOCHRONOUS_DATA_BUFFERS structure, with multiple
                            data buffer pointers.
    uint32_t size          - Number of data bytes to read. If the endpoint is
                            isochronous, this is the number of data buffer
                            pointers pointed to by pData.

  Return Values:
    USB_SUCCESS                     - Read started successfully.
    USB_UNKNOWN_DEVICE              - Device with the specified address not found.
    USB_INVALID_STATE               - We are not in a normal running state.
    USB_ENDPOINT_ILLEGAL_TYPE       - Must use USBHostControlRead to read
                                        from a control endpoint.
    USB_ENDPOINT_ILLEGAL_DIRECTION  - Must read from an IN endpoint.
    USB_ENDPOINT_STALLED            - Endpoint is stalled.  Must be cleared
                                        by the application.
    USB_ENDPOINT_ERROR              - Endpoint has too many errors.  Must be
                                        cleared by the application.
    USB_ENDPOINT_BUSY               - A Read is already in progress.
    USB_ENDPOINT_NOT_FOUND          - Invalid endpoint.

  Remarks:
    None
  ***************************************************************************/

uint8_t    USBHostRead( uint8_t deviceAddress, uint8_t endpoint, uint8_t *data, uint32_t size );


/****************************************************************************
  Function:
    uint8_t USBHostReadIsochronous( uint8_t deviceAddress, uint8_t endpoint,
            ISOCHRONOUS_DATA *pIsochronousData )

  Summary:
    This function initiates a read from an isochronous endpoint on the
    attached device.

  Description:
    This function initiates a read from an isochronous endpoint on the
    attached device.  If the endpoint is not isochronous, use USBHostRead().

    Once started, an isochronous transfer will continue with no upper layer
    intervention until USBHostTerminateTransfer() is called.  

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address
    uint8_t endpoint       - Endpoint number
    ISOCHRONOUS_DATA *pIsochronousData - Pointer to an ISOCHRONOUS_DATA
                            structure, containing information for the
                            application and the host driver for the
                            isochronous transfer.

  Return Values:
    USB_SUCCESS                     - Read started successfully.
    USB_UNKNOWN_DEVICE              - Device with the specified address not found.
    USB_INVALID_STATE               - We are not in a normal running state.
    USB_ENDPOINT_ILLEGAL_TYPE       - Must use USBHostControlRead to read
                                        from a control endpoint.
    USB_ENDPOINT_ILLEGAL_DIRECTION  - Must read from an IN endpoint.
    USB_ENDPOINT_STALLED            - Endpoint is stalled.  Must be cleared
                                        by the application.
    USB_ENDPOINT_ERROR              - Endpoint has too many errors.  Must be
                                        cleared by the application.
    USB_ENDPOINT_BUSY               - A Read is already in progress.
    USB_ENDPOINT_NOT_FOUND          - Invalid endpoint.

  Remarks:
    None
  ***************************************************************************/

#define USBHostReadIsochronous( a, e, p ) USBHostRead( a, e, (uint8_t *)p, (uint32_t)0 );


/****************************************************************************
  Function:
    uint8_t USBHostResetDevice( uint8_t deviceAddress )

  Summary:
    This function resets an attached device.

  Description:
    This function places the device back in the RESET state, to issue RESET
    signaling.  It can be called only if the state machine is not in the
    DETACHED state.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address

  Return Values:
    USB_SUCCESS         - Success
    USB_UNKNOWN_DEVICE  - Device not found
    USB_ILLEGAL_REQUEST - Device cannot RESUME unless it is suspended

  Remarks:
    In order to do a full clean-up, the state is set back to STATE_DETACHED
    rather than a reset state.  The ATTACH interrupt will automatically be
    triggered when the module is re-enabled, and the proper reset will be
    performed.
  ***************************************************************************/

uint8_t    USBHostResetDevice( uint8_t deviceAddress );


/****************************************************************************
  Function:
    uint8_t USBHostResumeDevice( uint8_t deviceAddress )

  Summary:
    This function issues a RESUME to the attached device.

  Description:
    This function issues a RESUME to the attached device.  It can called only
    if the state machine is in the suspend state.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address

  Return Values:
    USB_SUCCESS         - Success
    USB_UNKNOWN_DEVICE  - Device not found
    USB_ILLEGAL_REQUEST - Device cannot RESUME unless it is suspended

  Remarks:
    None
  ***************************************************************************/

uint8_t    USBHostResumeDevice( uint8_t deviceAddress );


/****************************************************************************
  Function:
    uint8_t USBHostSetDeviceConfiguration( uint8_t deviceAddress, uint8_t configuration )

  Summary:
    This function changes the device's configuration.

  Description:
    This function is used by the application to change the device's
    Configuration.  This function must be used instead of
    USBHostIssueDeviceRequest(), because the endpoint definitions may change.

    To see when the reconfiguration is complete, use the USBHostDeviceStatus()
    function.  If configuration is still in progress, this function will
    return USB_DEVICE_ENUMERATING.

  Precondition:
    The host state machine should be in the running state, and no reads or
    writes should be in progress.

  Parameters:
    uint8_t deviceAddress  - Device address
    uint8_t configuration  - Index of the new configuration
    
  Return Values:
    USB_SUCCESS         - Process of changing the configuration was started
                            successfully.
    USB_UNKNOWN_DEVICE  - Device not found
    USB_INVALID_STATE   - This function cannot be called during enumeration
                            or while performing a device request.
    USB_BUSY            - No IN or OUT transfers may be in progress.

  Example:
    <code>
    rc = USBHostSetDeviceConfiguration( attachedDevice, configuration );
    if (rc)
    {
        // Error - cannot set configuration.
    }
    else
    {
        while (USBHostDeviceStatus( attachedDevice ) == USB_DEVICE_ENUMERATING)
        {
            USBHostTasks();
        }
    }
    if (USBHostDeviceStatus( attachedDevice ) != USB_DEVICE_ATTACHED)
    {
        // Error - cannot set configuration.
    }
    </code>

  Remarks:
    If an invalid configuration is specified, this function cannot return
    an error.  Instead, the event USB_UNSUPPORTED_DEVICE will the sent to the
    application layer and the device will be placed in a holding state with a
    USB_HOLDING_UNSUPPORTED_DEVICE error returned by USBHostDeviceStatus().
  ***************************************************************************/

uint8_t USBHostSetDeviceConfiguration( uint8_t deviceAddress, uint8_t configuration );


/****************************************************************************
  Function:
    uint8_t USBHostSetNAKTimeout( uint8_t deviceAddress, uint8_t endpoint, uint16_t flags,
                uint16_t timeoutCount )

  Summary:
    This function specifies NAK timeout capability.

  Description:
    This function is used to set whether or not an endpoint on a device
    should time out a transaction based on the number of NAKs received, and
    if so, how many NAKs are allowed before the timeout.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address
    uint8_t endpoint       - Endpoint number to configure
    uint16_t flags          - Bit 0:
                            * 0 = disable NAK timeout
                            * 1 = enable NAK timeout
    uint16_t timeoutCount   - Number of NAKs allowed before a timeout

  Return Values:
    USB_SUCCESS             - NAK timeout was configured successfully.
    USB_UNKNOWN_DEVICE      - Device not found.
    USB_ENDPOINT_NOT_FOUND  - The specified endpoint was not found.

  Remarks:
    None
  ***************************************************************************/

uint8_t USBHostSetNAKTimeout( uint8_t deviceAddress, uint8_t endpoint, uint16_t flags, uint16_t timeoutCount );


/****************************************************************************
  Function:
    void USBHostShutdown( void )

  Description:
    This function turns off the USB module and frees all unnecessary memory.
    This routine can be called by the application layer to shut down all
    USB activity, which effectively detaches all devices.  The event
    EVENT_DETACH will be sent to the client drivers for the attached device,
    and the event EVENT_VBUS_RELEASE_POWER will be sent to the application
    layer.

  Precondition:
    None

  Parameters:
    None - None

  Returns:
    None

  Remarks:
    None
  ***************************************************************************/

void    USBHostShutdown( void );


/****************************************************************************
  Function:
    uint8_t USBHostSuspendDevice( uint8_t deviceAddress )

  Summary:
    This function suspends a device.

  Description:
    This function put a device into an IDLE state.  It can only be called
    while the state machine is in normal running mode.  After 3ms, the
    attached device should go into SUSPEND mode.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device to suspend

  Return Values:
    USB_SUCCESS         - Success
    USB_UNKNOWN_DEVICE  - Device not found
    USB_ILLEGAL_REQUEST - Cannot suspend unless device is in normal run mode

  Remarks:
    None
  ****************************************************************************/

uint8_t    USBHostSuspendDevice( uint8_t deviceAddress );


/****************************************************************************
  Function:
    void USBHostTasks( void )

  Summary:
    This function executes the host tasks for USB host operation.

  Description:
    This function executes the host tasks for USB host operation.  It must be
    executed on a regular basis to keep everything functioning.

    The primary purpose of this function is to handle device attach/detach
    and enumeration.  It does not handle USB packet transmission or
    reception; that must be done in the USB interrupt handler to ensure
    timely operation.

    This routine should be called on a regular basis, but there is no
    specific time requirement.  Devices will still be able to attach,
    enumerate, and detach, but the operations will occur more slowly as the
    calling interval increases.

  Precondition:
    USBHostInit() has been called.

  Parameters:
    None

  Returns:
    None

  Remarks:
    None
  ***************************************************************************/

void    USBHostTasks( void );


/****************************************************************************
  Function:
    void USBHostTerminateTransfer( uint8_t deviceAddress, uint8_t endpoint )


  Summary:
    This function terminates the current transfer for the given endpoint.

  Description:
    This function terminates the current transfer for the given endpoint.  It
    can be used to terminate reads or writes that the device is not
    responding to.  It is also the only way to terminate an isochronous
    transfer.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address
    uint8_t endpoint       - Endpoint number

  Returns:
    None

  Remarks:
    None
  ***************************************************************************/

void    USBHostTerminateTransfer( uint8_t deviceAddress, uint8_t endpoint );


/****************************************************************************
  Function:
    bool USBHostTransferIsComplete( uint8_t deviceAddress, uint8_t endpoint,
                        uint8_t *errorCode, uint32_t *byteCount )

  Summary:
    This function initiates whether or not the last endpoint transaction is
    complete.

  Description:
    This function initiates whether or not the last endpoint transaction is
    complete.  If it is complete, an error code and the number of bytes
    transferred are returned.

    For isochronous transfers, byteCount is not valid.  Instead, use the
    returned byte counts for each EVENT_TRANSFER event that was generated
    during the transfer.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address
    uint8_t endpoint       - Endpoint number
    uint8_t *errorCode     - Error code indicating the status of the transfer.
                            Only valid if the transfer is complete.
    uint32_t *byteCount    - The number of bytes sent or received.  Invalid
                            for isochronous transfers.

  Return Values:
    true    - Transfer is complete.
    false   - Transfer is not complete.

  Remarks:
    Possible values for errorCode are:
        * USB_SUCCESS                     - Transfer successful
        * USB_UNKNOWN_DEVICE              - Device not attached
        * USB_ENDPOINT_STALLED            - Endpoint STALL'd
        * USB_ENDPOINT_ERROR_ILLEGAL_PID  - Illegal PID returned
        * USB_ENDPOINT_ERROR_BIT_STUFF
        * USB_ENDPOINT_ERROR_DMA
        * USB_ENDPOINT_ERROR_TIMEOUT
        * USB_ENDPOINT_ERROR_DATA_FIELD
        * USB_ENDPOINT_ERROR_CRC16
        * USB_ENDPOINT_ERROR_END_OF_FRAME
        * USB_ENDPOINT_ERROR_PID_CHECK
        * USB_ENDPOINT_ERROR              - Other error
  ***************************************************************************/

bool    USBHostTransferIsComplete( uint8_t deviceAddress, uint8_t endpoint, uint8_t *errorCode, uint32_t *byteCount );


/****************************************************************************
  Function:
    uint8_t  USBHostVbusEvent( USB_EVENT vbusEvent, uint8_t hubAddress,
                                        uint8_t portNumber)

  Summary:
    This function handles Vbus events that are detected by the application.

  Description:
    This function handles Vbus events that are detected by the application.
    Since Vbus management is application dependent, the application is
    responsible for monitoring Vbus and detecting overcurrent conditions
    and removal of the overcurrent condition.  If the application detects
    an overcurrent condition, it should call this function with the event
    EVENT_VBUS_OVERCURRENT with the address of the hub and port number that
    has the condition.  When a port returns to normal operation, the
    application should call this function with the event
    EVENT_VBUS_POWER_AVAILABLE so the stack knows that it can allow devices
    to attach to that port.

  Precondition:
    None

  Parameters:
    USB_EVENT vbusEvent     - Vbus event that occured.  Valid events:
                                    * EVENT_VBUS_OVERCURRENT
                                    * EVENT_VBUS_POWER_AVAILABLE
    uint8_t hubAddress         - Address of the hub device (USB_ROOT_HUB for the
                                root hub)
    uint8_t portNumber         - Number of the physical port on the hub (0 - based)

  Return Values:
    USB_SUCCESS             - Event handled
    USB_ILLEGAL_REQUEST     - Invalid event, hub, or port

  Remarks:
    None
  ***************************************************************************/

uint8_t    USBHostVbusEvent(USB_EVENT vbusEvent, uint8_t hubAddress, uint8_t portNumber);


/****************************************************************************
  Function:
    uint8_t USBHostWrite( uint8_t deviceAddress, uint8_t endpoint, uint8_t *data,
                        uint32_t size )

  Summary:
    This function initiates a write to the attached device.

  Description:
    This function initiates a write to the attached device.  The data buffer
    pointed to by *data must remain valid during the entire time that the
    write is taking place; the data is not buffered by the stack.

    If the endpoint is isochronous, special conditions apply.  The pData and
    size parameters have slightly different meanings, since multiple buffers
    are required.  Once started, an isochronous transfer will continue with
    no upper layer intervention until USBHostTerminateTransfer() is called.
    The ISOCHRONOUS_DATA_BUFFERS structure should not be manipulated until
    the transfer is terminated.

    To clarify parameter usage and to simplify casting, use the macro
    USBHostWriteIsochronous() when writing to an isochronous endpoint.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address
    uint8_t endpoint       - Endpoint number
    uint8_t *data          - Pointer to where the data is stored. If the endpoint
                            is isochronous, this points to an
                            ISOCHRONOUS_DATA_BUFFERS structure, with multiple
                            data buffer pointers.
    uint32_t size          - Number of data bytes to send. If the endpoint is
                            isochronous, this is the number of data buffer
                            pointers pointed to by pData.

  Return Values:
    USB_SUCCESS                     - Write started successfully.
    USB_UNKNOWN_DEVICE              - Device with the specified address not found.
    USB_INVALID_STATE               - We are not in a normal running state.
    USB_ENDPOINT_ILLEGAL_TYPE       - Must use USBHostControlWrite to write
                                        to a control endpoint.
    USB_ENDPOINT_ILLEGAL_DIRECTION  - Must write to an OUT endpoint.
    USB_ENDPOINT_STALLED            - Endpoint is stalled.  Must be cleared
                                        by the application.
    USB_ENDPOINT_ERROR              - Endpoint has too many errors.  Must be
                                        cleared by the application.
    USB_ENDPOINT_BUSY               - A Write is already in progress.
    USB_ENDPOINT_NOT_FOUND          - Invalid endpoint.

  Remarks:
    None
  ***************************************************************************/

uint8_t    USBHostWrite( uint8_t deviceAddress, uint8_t endpoint, uint8_t *data, uint32_t size );


/****************************************************************************
  Function:
    uint8_t USBHostWriteIsochronous( uint8_t deviceAddress, uint8_t endpoint,
            ISOCHRONOUS_DATA *pIsochronousData )

  Summary:
    This function initiates a write to an isochronous endpoint on the
    attached device.

  Description:
    This function initiates a write to an isochronous endpoint on the
    attached device.  If the endpoint is not isochronous, use USBHostWrite().

    Once started, an isochronous transfer will continue with
    no upper layer intervention until USBHostTerminateTransfer() is called.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address
    uint8_t endpoint       - Endpoint number
    ISOCHRONOUS_DATA *pIsochronousData - Pointer to an ISOCHRONOUS_DATA
                            structure, containing information for the
                            application and the host driver for the
                            isochronous transfer.

  Return Values:
    USB_SUCCESS                     - Write started successfully.
    USB_UNKNOWN_DEVICE              - Device with the specified address not found.
    USB_INVALID_STATE               - We are not in a normal running state.
    USB_ENDPOINT_ILLEGAL_TYPE       - Must use USBHostControlWrite to write
                                        to a control endpoint.
    USB_ENDPOINT_ILLEGAL_DIRECTION  - Must write to an OUT endpoint.
    USB_ENDPOINT_STALLED            - Endpoint is stalled.  Must be cleared
                                        by the application.
    USB_ENDPOINT_ERROR              - Endpoint has too many errors.  Must be
                                        cleared by the application.
    USB_ENDPOINT_BUSY               - A Write is already in progress.
    USB_ENDPOINT_NOT_FOUND          - Invalid endpoint.

  Remarks:
    None
  ***************************************************************************/

#define USBHostWriteIsochronous( a, e, p ) USBHostWrite( a, e, (uint8_t *)p, (uint32_t)0 );

/****************************************************************************
  Function:
    void USB_HostInterruptHandler(void);

  Summary:
   This function handles the interrupts when the USB module is running in host
   mode.

  Description:
   This function handles the interrupts when the USB module is running in host
   mode.  It will clear all USB based interrupts as applicable.  It should only
   be called when the module is in host mode.

  Precondition:
    Should only be called when in host mode.

  Parameters:
    None

  Return Values:
    None
  ***************************************************************************/
void USB_HostInterruptHandler(void);

#endif

// *****************************************************************************
// EOF


