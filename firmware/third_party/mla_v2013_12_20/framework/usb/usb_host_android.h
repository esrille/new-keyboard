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

#include <stdint.h>
#include <stdbool.h>

/* Error code indicating that the buffer passed to the read function was too small.  
   Since the USB host can't control how much data it will receive in a single packet,
   the user must provide a buffer that is at least the size of the endpoint of the
   attached device.  If a buffer is passed in that is too small, the read will not 
   start and this error is returned to the user. */
#define USB_ERROR_BUFFER_TOO_SMALL      USB_ERROR_CLASS_DEFINED + 0

/* This defintion is used in the usbClientDrvTable[] in the flags field in order to
   bypass the Android accessory initialization phase.  This should be used only when
   a device is known to already be in accessory mode (in protocol v1 if the VID/PID
   are already matching the accessory mode VID/PID).  In some cases an Android device
   doesn't exit accessory mode and thus those other protocol commands will not work.
   This flag must be used to save those devices */
#define ANDROID_INIT_FLAG_BYPASS_PROTOCOL           0x00000001


#ifndef ANDROID_BASE_OFFSET
    /* Defines the event offset for the Android specific events.  If not defined, then a default of 0 is used. */
    #define ANDROID_BASE_OFFSET 0
#endif

#ifndef NUM_ANDROID_DEVICES_SUPPORTED
    /* Defines the number of concurrent Android devices this implementation is allowed to 
       talk to.  This definition is only used for implementations where the accessory is
       the host and the Android device is the slave.  This is also most often defined to
       be 1.  If this is not defined by the user, a default of 1 is used. 

       This option is only used when compiling the source version of the library. This
       value is set to 1 for pre-compiled versions of the library. */
    #define NUM_ANDROID_DEVICES_SUPPORTED 1
#endif

#define ANDROID_EVENT_BASE EVENT_USER_BASE + ANDROID_BASE_OFFSET



/* This event is thrown when an Android device is attached and successfully entered into
   accessory mode already.  The data portion of this event is the handle that is 
   required to communicate to the device and should be saved so that it can be passed to
   all of the transfer functions.  Always use this definition in the code and never put
   a static value as the value of this event may change based on various build options. */
#define EVENT_ANDROID_ATTACH      ANDROID_EVENT_BASE + 0

/* This event is thrown when an Android device is removed.  The data portion of the
   event is the handle of the device that has been removed.  Always use this definition
   in the code and never put a static value as the value of this event may change based
   on various build options. */
#define EVENT_ANDROID_DETACH      ANDROID_EVENT_BASE + 1

/* This event is thrown after a HID report is successfully registered.  That report is
   now available for use by the application */
#define EVENT_ANDROID_HID_REGISTRATION_COMPLETE ANDROID_EVENT_BASE + 2

/* The requested report has been sent to the requested device.  */
#define EVENT_ANDROID_HID_SEND_EVENT_COMPLETE   ANDROID_EVENT_BASE + 3

/* Defines the available audio modes */
typedef enum
{
    /* No audio support enabled */
    ANDROID_AUDIO_MODE__NONE = 0,

    /* 44K 16B PCM audio mode enabled */
    ANDROID_AUDIO_MODE__44K_16B_PCM = 1
} ANDROID_AUDIO_MODE;

/* This structure contains the informatin that is required to successfully create a link 
   between the Android device and the accessory.  This information must match the 
   information entered in the accessory filter in the Android application in order for 
   the Android application to access the device.  An instance of this structure should be 
   passed into the AndroidAppStart() at initialization. */
typedef struct
{
    char* manufacturer;         //String: manufacturer name
    uint8_t manufacturer_size;     //length of manufacturer string

    char* model;                //String: model name
    uint8_t model_size;            //length of model name string

    char* description;          //String: description of the accessory
    uint8_t description_size;      //length of the description string

    char* version;              //String: version number
    uint8_t version_size;          //length of the version number string

    char* URI;                  //String: URI for the accessory (most commonly a URL)
    uint8_t URI_size;              //length of the URI string

    char* serial;               //String: serial number of the device
    uint8_t serial_size;           //length of the serial number string

    ANDROID_AUDIO_MODE  audio_mode;
} ANDROID_ACCESSORY_INFORMATION;



/****************************************************************************
  Function:
    void AndroidAppStart(ANDROID_ACCESSORY_INFORMATION *info)

  Summary:
    Sets the accessory information and initializes the client driver information
    after the initial power cycles.

  Description:
    Sets the accessory information and initializes the client driver information
    after the initial power cycles.  Since this resets all device information
    this function should be used only after a compete system reset.  This should 
    not be called while the USB is active or while connected to a device.

  Precondition:
    USB module should not be in operation

  Parameters:
    ANDROID_ACCESSORY_INFORMATION *info  - the information about the Android accessory

  Return Values:
    None

  Remarks:
    None
  ***************************************************************************/
void AndroidAppStart(ANDROID_ACCESSORY_INFORMATION* accessoryInfo);

/****************************************************************************
  Function:
    void AndroidTasks(void)

  Summary:
    Tasks function that keeps the Android client driver moving

  Description:
    Tasks function that keeps the Android client driver moving.  Keeps the driver
    processing requests and handling events.  This function should be called
    periodically (the same frequency as USBHostTasks() would be helpful).

  Precondition:
    AndroidAppStart() function has been called before the first calling of this function

  Parameters:
    None

  Return Values:
    None

  Remarks:
    This function should be called periodically to keep the Android driver moving.
  ***************************************************************************/
void AndroidTasks(void);

/****************************************************************************
  Function:
    uint8_t AndroidAppWrite(void* handle, uint8_t* data, uint32_t size)

  Summary:
    Sends data to the Android device specified by the passed in handle.

  Description:
    Sends data to the Android device specified by the passed in handle.

  Precondition:
    Transfer is not already in progress.  USB module is initialized and Android
    device has attached.

  Parameters:
    void* handle - the handle passed to the device in the EVENT_ANDROID_ATTACH event
    uint8_t* data - the data to send to the Android device
    uint32_t size - the size of the data that needs to be sent

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
uint8_t AndroidAppWrite(void* handle, uint8_t* data, uint32_t size);

/****************************************************************************
  Function:
    bool AndroidAppIsWriteComplete(void* handle, uint8_t* errorCode, uint32_t* size)

  Summary:
    Check to see if the last write to the Android device was completed

  Description:
    Check to see if the last write to the Android device was completed.  If 
    complete, returns the amount of data that was sent and the corresponding 
    error code for the transmission.

  Precondition:
    Transfer has previously been sent to Android device.

  Parameters:
    void* handle - the handle passed to the device in the EVENT_ANDROID_ATTACH event
    uint8_t* errorCode - a pointer to the location where the resulting error code should be written
    uint32_t* size - a pointer to the location where the resulting size information should be written

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
bool AndroidAppIsWriteComplete(void* handle, uint8_t* errorCode, uint32_t* size);

/****************************************************************************
  Function:
    uint8_t AndroidAppRead(void* handle, uint8_t* data, uint32_t size)

  Summary:
    Attempts to read information from the specified Android device

  Description:
    Attempts to read information from the specified Android device.  This
    function does not block.  Data availability is checked via the 
    AndroidAppIsReadComplete() function.

  Precondition:
    A read request is not already in progress and an Android device is attached.

  Parameters:
    void* handle - the handle passed to the device in the EVENT_ANDROID_ATTACH event
    uint8_t* data - a pointer to the location of where the data should be stored.  This location
                should be accessible by the USB module
    uint32_t size - the amount of data to read.

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
    USB_ERROR_BUFFER_TOO_SMALL      - The buffer passed to the read function was
                                        smaller than the endpoint size being used
                                        (buffer must be larger than or equal to 
                                        the endpoint size).

  Remarks:
    None
  ***************************************************************************/
uint8_t AndroidAppRead(void* handle, uint8_t* data, uint32_t size);

/****************************************************************************
  Function:
    bool AndroidAppIsReadComplete(void* handle, uint8_t* errorCode, uint32_t* size)

  Summary:
    Check to see if the last read to the Android device was completed

  Description:
    Check to see if the last read to the Android device was completed.  If 
    complete, returns the amount of data that was sent and the corresponding 
    error code for the transmission.

  Precondition:
    Transfer has previously been requested from an Android device.

  Parameters:
    void* handle - the handle passed to the device in the EVENT_ANDROID_ATTACH event
    uint8_t* errorCode - a pointer to the location where the resulting error code should be written
    uint32_t* size - a pointer to the location where the resulting size information should be written

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
bool AndroidAppIsReadComplete(void* handle, uint8_t* errorCode, uint32_t* size);


/****************************************************************************
  Function:
    bool AndroidAppInitialize( uint8_t address, uint32_t flags, uint8_t clientDriverID )

  Summary:
    Per instance client driver for Android device.  Called by USB host stack from
    the client driver table.

  Description:
    Per instance client driver for Android device.  Called by USB host stack from
    the client driver table.

  Precondition:
    None

  Parameters:
    uint8_t address - the address of the device that is being initialized
    uint32_t flags - the initialization flags for the device
    uint8_t clientDriverID - the clientDriverID for the device

  Return Values:
    true - initialized successfully
    false - does not support this device

  Remarks:
    This is a internal API only.  This should not be called by anything other
    than the USB host stack via the client driver table
  ***************************************************************************/
bool AndroidAppInitialize( uint8_t address, uint32_t flags, uint8_t clientDriverID );

/****************************************************************************
  Function:
    bool AndroidAppEventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size )

  Summary:
    Handles events from the host stack

  Description:
    Handles events from the host stack

  Precondition:
    None

  Parameters:
    uint8_t address - the address of the device that caused the event
    USB_EVENT event - the event that occured
    void* data - the data for the event
    uint32_t size - the size of the data in bytes

  Return Values:
    true - the event was handled
    false - the event was not handled

  Remarks:
    This is a internal API only.  This should not be called by anything other
    than the USB host stack via the client driver table
  ***************************************************************************/
bool AndroidAppEventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size );

/****************************************************************************
  Function:
    bool AndroidAppDataEventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size )

  Summary:
    Handles data events from the host stack

  Description:
    Handles data events from the host stack

  Precondition:
    None

  Parameters:
    uint8_t address - the address of the device that caused the event
    USB_EVENT event - the event that occured
    void* data - the data for the event
    uint32_t size - the size of the data in bytes

  Return Values:
    true - the event was handled
    false - the event was not handled

  Remarks:
    This is a internal API only.  This should not be called by anything other
    than the USB host stack via the client driver table
  ***************************************************************************/
bool AndroidAppDataEventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size );


/****************************************************************************
  Function:
    uint8_t AndroidAppHIDSendEvent(uint8_t address, uint8_t id, uint8_t* report, uint8_t length);

  Summary:
    Sends a HID report to the associated Android device

  Description:
    Sends a HID report to the associated Android device

  Precondition:
    HID device should have already been registers with the AndroidAppHIDRegister() function

  Parameters:
    uint8_t address - address of the USB peripheral to send the report to
    uint8_t id - Report ID of the report being sent
    uint8_t* report - data for the report
    uint8_t length - length of the report

  Return Values:
    None

  Remarks:
    None
  ***************************************************************************/
uint8_t AndroidAppHIDSendEvent(uint8_t address, uint8_t id, uint8_t* report, uint8_t length);


/****************************************************************************
  Function:
    bool AndroidAppHIDRegister(uint8_t address, uint8_t id, uint8_t* descriptor, uint8_t length);

  Summary:
    Registers a HID report with the Android device

  Description:
    Registers a HID report with the Android device

  Precondition:
    HID device already attached

  Parameters:
    uint8_t address - address of the USB peripheral to send the report to
    uint8_t id - Report ID of the report that is being registered
    uint8_t* descriptor - HID report descriptor
    uint8_t length - length of the report

  Return Values:
    None

  Remarks:
    None
  ***************************************************************************/
bool AndroidAppHIDRegister(uint8_t address, uint8_t id, uint8_t* descriptor, uint8_t length);
