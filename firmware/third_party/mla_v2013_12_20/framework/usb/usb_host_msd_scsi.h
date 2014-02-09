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

#ifndef __USBHOSTMSDSCSI_H__
#define __USBHOSTMSDSCSI_H__
//DOM-IGNORE-END

#include "usb/usb.h"
#include "system_config.h"
#include "fileio/fileio.h"

// *****************************************************************************
// *****************************************************************************
// Section: Constants
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************

/****************************************************************************
  Function:
    uint8_t USBHostMSDSCSIMediaDetect( uint8_t * address)

  Description:
    This function determines if a mass storage device is attached and ready
    to use.

  Precondition:
    None

  Parameters:
    address - Endpoint address of the device

  Return Values:
    true    -   MSD present and ready
    false   -   MSD not present or not ready

  Remarks:
    Since this will often be called in a loop while waiting for a device,
    we need to make sure that USB tasks are executed.
  ***************************************************************************/

uint8_t    USBHostMSDSCSIMediaDetect( uint8_t * address);


/****************************************************************************
  Function:
    MEDIA_INFORMATION * USBHostMSDSCSIMediaInitialize( uint8_t * address )

  Description:
    This function initializes the media.

  Precondition:
    None

  Parameters:
    address - Endpoint address of the device

  Returns:
    The function returns a pointer to the MEDIA_INFORMATION structure.  The
    errorCode member may contain the following values:
        * MEDIA_NO_ERROR - The media initialized successfully, and the 
                sector size should be valid (confirm using the validityFlags 
                bit). 
        * MEDIA_DEVICE_NOT_PRESENT - The requested device is not attached.
        * MEDIA_CANNOT_INITIALIZE - Cannot initialize the media.

  Remarks:
    This function performs the following SCSI commands:
                        * READ CAPACITY 10
                        * REQUEST SENSE

    The READ CAPACITY 10 command block is as follows:

    <code>
        Byte/Bit    7       6       5       4       3       2       1       0
           0                    Operation Code (0x25)
           1        [                      Reserved                         ]
           2        [ (MSB)
           3                        Logical Block Address
           4
           5                                                          (LSB) ]
           6        [                      Reserved
           7                                                                ]
           8        [                      Reserved                 ] [ PMI ]
           9        [                    Control                            ]
    </code>

    The REQUEST SENSE command block is as follows:

    <code>
        Byte/Bit    7       6       5       4       3       2       1       0
           0                    Operation Code (0x02)
           1        [                      Reserved                 ] [ DESC]
           2        [                      Reserved
           3                                                                ]
           4        [                  Allocation Length                    ]
           5        [                    Control                            ]
    </code>
  ***************************************************************************/

FILEIO_MEDIA_INFORMATION * USBHostMSDSCSIMediaInitialize( uint8_t * address );


/****************************************************************************
  Function:
    bool USBHostMSDSCSIMediaReset( uint8_t * address )

  Summary:
    This function resets the media.

  Description:
    This function resets the media.  It is called if an operation returns an
    error.  Or the application can call it.

  Precondition:
    None

  Parameters:
    address - Endpoint address of the device

  Return Values:
    USB_SUCCESS                 - Reset successful
    USB_MSD_DEVICE_NOT_FOUND    - No device with specified address
    USB_ILLEGAL_REQUEST         - Device is in an illegal USB state
                                  for reset

  Remarks:
    None
  ***************************************************************************/

uint8_t    USBHostMSDSCSIMediaReset( uint8_t * address );


/****************************************************************************
  Function:
    uint8_t USBHostMSDSCSISectorRead( uint8_t * address, uint32_t sectorAddress, uint8_t *dataBuffer)

  Summary:
    This function reads one sector.

  Description:
    This function uses the SCSI command READ10 to read one sector.  The size
    of the sector was determined in the USBHostMSDSCSIMediaInitialize()
    function.  The data is stored in the application buffer.

  Precondition:
    None

  Parameters:
    uint8_t * address - Endpoint address of the device
    uint32_t   sectorAddress   - address of sector to read
    uint8_t    *dataBuffer     - buffer to store data

  Return Values:
    true    - read performed successfully
    false   - read was not successful

  Remarks:
    The READ10 command block is as follows:

    <code>
        Byte/Bit    7       6       5       4       3       2       1       0
           0                    Operation Code (0x28)
           1        [    RDPROTECT      ]  DPO     FUA      -     FUA_NV    -
           2        [ (MSB)
           3                        Logical Block Address
           4
           5                                                          (LSB) ]
           6        [         -         ][          Group Number            ]
           7        [ (MSB)         Transfer Length
           8                                                          (LSB) ]
           9        [                    Control                            ]
    </code>
  ***************************************************************************/

uint8_t    USBHostMSDSCSISectorRead( uint8_t * address, uint32_t sectorAddress, uint8_t *dataBuffer );


/****************************************************************************
  Function:
    uint8_t USBHostMSDSCSISectorWrite( uint8_t * address, uint32_t sectorAddress, uint8_t *dataBuffer, uint8_t allowWriteToZero )

  Summary:
    This function writes one sector.

  Description:
    This function uses the SCSI command WRITE10 to write one sector.  The size
    of the sector was determined in the USBHostMSDSCSIMediaInitialize()
    function.  The data is read from the application buffer.

  Precondition:
    None

  Parameters:
    uint8_t * address - Endpoint address of the device
    uint32_t   sectorAddress   - address of sector to write
    uint8_t    *dataBuffer     - buffer with application data
    uint8_t    allowWriteToZero- If a write to sector 0 is allowed.

  Return Values:
    true    - write performed successfully
    false   - write was not successful

  Remarks:
    To follow convention, this function blocks until the write is complete.

    The WRITE10 command block is as follows:

    <code>
        Byte/Bit    7       6       5       4       3       2       1       0
           0                    Operation Code (0x2A)
           1        [    WRPROTECT      ]  DPO     FUA      -     FUA_NV    -
           2        [ (MSB)
           3                        Logical Block Address
           4
           5                                                          (LSB) ]
           6        [         -         ][          Group Number            ]
           7        [ (MSB)         Transfer Length
           8                                                          (LSB) ]
           9        [                    Control                            ]
    </code>
  ***************************************************************************/

uint8_t    USBHostMSDSCSISectorWrite( uint8_t * address, uint32_t sectorAddress, uint8_t *dataBuffer, uint8_t allowWriteToZero);


/****************************************************************************
  Function:
    uint8_t USBHostMSDSCSIWriteProtectState( uint8_t * address )

  Description:
    This function returns the write protect status of the device.

  Precondition:
    None

  Parameters:
    uint8_t * address - Endpoint address of the device

  Return Values:
    0 - not write protected


  Remarks:
    None
  ***************************************************************************/

uint8_t    USBHostMSDSCSIWriteProtectState( uint8_t * address );


// *****************************************************************************
// *****************************************************************************
// Section: SCSI Interface Callback Functions
// *****************************************************************************
// *****************************************************************************

/****************************************************************************
  Function:
    bool USBHostMSDSCSIInitialize( uint8_t address, uint32_t flags, uint8_t clientDriverID )

  Description:
    This function is called when a USB Mass Storage device is being
    enumerated.

  Precondition:
    None

  Parameters:
    uint8_t address    -   Address of the new device
    uint32_t flags     -   Initialization flags
    uint8_t clientDriverID - ID for this layer.  Not used by the media interface layer.

  Return Values:
    true    -   We can support the device.
    false   -   We cannot support the device.

  Remarks:
    None
  ***************************************************************************/

bool USBHostMSDSCSIInitialize( uint8_t address, uint32_t flags, uint8_t clientDriverID );


/****************************************************************************
  Function:
    bool USBHostMSDSCSIEventHandler( uint8_t address, USB_EVENT event,
                        void *data, uint32_t size )

  Description:
    This function is called when various events occur in the USB Host Mass
    Storage client driver.

  Precondition:
    The device has been initialized.

  Parameters:
    uint8_t address    -   Address of the device
    USB_EVENT event -   Event that has occurred
    void *data      -   Pointer to data pertinent to the event
    uint32_t size      -   Size of the data

  Return Values:
    true    -   Event was handled
    false   -   Event was not handled

  Remarks:
    None
  ***************************************************************************/

bool USBHostMSDSCSIEventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size );


#endif
