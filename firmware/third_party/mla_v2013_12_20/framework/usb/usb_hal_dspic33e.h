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

#ifndef USB_HAL_DSPIC33E_H
#define USB_HAL_DSPIC33E_H

/*****************************************************************************/
/****** include files ********************************************************/
/*****************************************************************************/

#include "system.h"
#include "system_config.h"

#include <stdint.h>
#include <string.h>

/*****************************************************************************/
/****** Constant definitions *************************************************/
/*****************************************************************************/


#if(USB_PING_PONG_MODE != USB_PING_PONG__FULL_PING_PONG)
    #error "Unsupported ping pong mode for this device"
#endif
#define BDT_NUM_ENTRIES      ((USB_MAX_EP_NUMBER + 1) * 4)


//----- USBEnableEndpoint() input defintions ----------------------------------
#define USB_HANDSHAKE_ENABLED           0x01
#define USB_HANDSHAKE_DISABLED          0x00

#define USB_OUT_ENABLED                 0x08
#define USB_OUT_DISABLED                0x00

#define USB_IN_ENABLED                  0x04
#define USB_IN_DISABLED                 0x00

#define USB_ALLOW_SETUP                 0x00
#define USB_DISALLOW_SETUP              0x10

#define USB_STALL_ENDPOINT              0x02

//----- usb_config.h input defintions -----------------------------------------
#define USB_PULLUP_ENABLE               0x00
#define USB_PULLUP_DISABLE 				0x04

#define USB_INTERNAL_TRANSCEIVER        0x00
#define USB_EXTERNAL_TRANSCEIVER        0x01

#define USB_FULL_SPEED                  0x04
//USB_LOW_SPEED not currently supported in dsPIC33E USB products

#define USB_OTG_ENABLE                  0x04

//----- Interrupt Flag definitions --------------------------------------------
#define USBTransactionCompleteIE        U1IEbits.TRNIE
#define USBTransactionCompleteIF        U1IRbits.TRNIF
#define USBTransactionCompleteIFReg     U1IR
#define USBTransactionCompleteIFBitNum  3

#define USBResetIE                      U1IEbits.URSTIE
#define USBResetIF                      U1IRbits.URSTIF
#define USBResetIFReg                   U1IR
#define USBResetIFBitNum                0

#define USBIdleIE                       U1IEbits.IDLEIE
#define USBIdleIF                       U1IRbits.IDLEIF
#define USBIdleIFReg                    U1IR
#define USBIdleIFBitNum                 4

#define USBActivityIE                   U1OTGIEbits.ACTVIE
#define USBActivityIF                   U1OTGIRbits.ACTVIF
#define USBActivityIFReg                U1OTGIR
#define USBActivityIFBitNum             4

#define USBSOFIE                        U1IEbits.SOFIE
#define USBSOFIF                        U1IRbits.SOFIF
#define USBSOFIFReg                     U1IR
#define USBSOFIFBitNum                  2

#define USBStallIE                      U1IEbits.STALLIE
#define USBStallIF                      U1IRbits.STALLIF
#define USBStallIFReg                   U1IR
#define USBStallIFBitNum                7

#define USBErrorIE                      U1IEbits.UERRIE
#define USBErrorIF                      U1IRbits.UERRIF
#define USBErrorIFReg                   U1IR
#define USBErrorIFBitNum                1

#define USBT1MSECIE                     U1OTGIEbits.T1MSECIE
#define USBT1MSECIF                     U1OTGIRbits.T1MSECIF
#define USBT1MSECIFReg                  U1OTGIR
#define USBT1MSECIFBitNum               6

#define USBIDIE                         U1OTGIEbits.IDIE
#define USBIDIF                         U1OTGIRbits.IDIF
#define USBIDIFReg                      U1OTGIR
#define USBIDIFBitNum                   7

//----- Event call back defintions --------------------------------------------
#if defined(USB_DISABLE_SOF_HANDLER)
    #define USB_SOF_INTERRUPT           0x00
#else
    #define USB_SOF_INTERRUPT           0x04
#endif
#if defined(USB_DISABLE_ERROR_HANDLER)
    #define USB_ERROR_INTERRUPT         0x02
#else
    #define USB_ERROR_INTERRUPT         0x02
#endif

//----- USB module control bits -----------------------------------------------
#define USBPingPongBufferReset          U1CONbits.PPBRST
#define USBSE0Event 					U1CONbits.SE0//0// U1IRbits.URSTIF//  U1CONbits.SE0
#define USBSuspendControl               U1PWRCbits.USUSPEND
#define USBPacketDisable                U1CONbits.PKTDIS
#define USBResumeControl                U1CONbits.RESUME

//----- BDnSTAT bit definitions -----------------------------------------------
//The _BSTALL definition is changed from 0x04 to 0x00 to
// fix a difference in the PIC18 and PIC24 definitions of this
// bit.  This should be changed back once the definitions are
// synced.
#define _BSTALL                         0x04        //Buffer Stall enable
#define _DTSEN                          0x08        //Data Toggle Synch enable
#define _DAT0                           0x00        //DATA0 packet expected next
#define _DAT1                           0x40        //DATA1 packet expected next
#define _DTSMASK                        0x40        //DTS Mask
#define _USIE                           0x80        //SIE owns buffer
#define _UCPU                           0x00        //CPU owns buffer
#define _STAT_MASK                      0xFC

//----- USTAT bit definitions -------------------------------------------------
#define USTAT_EP0_PP_MASK               ~0x04
#define USTAT_EP_MASK                   0xFC
#define USTAT_EP0_OUT                   0x00
#define USTAT_EP0_OUT_EVEN              0x00
#define USTAT_EP0_OUT_ODD               0x04
#define USTAT_EP0_IN                    0x08
#define USTAT_EP0_IN_EVEN               0x08
#define USTAT_EP0_IN_ODD                0x0C
#define ENDPOINT_MASK                   0b11110000

//----- U1OTGCON bit definitions ----------------------------------------------
#define USB_OTG_DPLUS_ENABLE            0x80

//----- U1EP bit definitions --------------------------------------------------
#define UEP_STALL                       0x0002
// Cfg Control pipe for this ep
#define EP_CTRL                         0x0C     // Cfg Control pipe for this ep   
#define EP_OUT                          0x18     // Cfg OUT only pipe for this ep
#define EP_IN                           0x14     // Cfg IN only pipe for this ep
#define EP_OUT_IN                       0x1C     // Cfg both OUT & IN pipes for this ep
#define HSHK_EN                         0x01     // Enable handshake packet
                                                 // Handshake should be disable for isoch


#define BDT_BASE_ADDR_TAG   __attribute__ ((aligned (512)))
#define CTRL_TRF_SETUP_ADDR_TAG
#define CTRL_TRF_DATA_ADDR_TAG

//----- Depricated defintions - will be removed at some point of time----------
//--------- Depricated in v2.2
#define _LS                             0x00        // Use Low-Speed USB Mode
#define _FS                             0x00        // Use Full-Speed USB Mode
#define _TRINT                          0x00        // Use internal transceiver
#define _TREXT                          0x00        // Use external transceiver
#define _PUEN                           0x00        // Use internal pull-up resistor
#define _OEMON                          0x00        // Use SIE output indicator

/*****************************************************************************/
/****** Type definitions *****************************************************/
/*****************************************************************************/

// Buffer Descriptor Status Register layout.
typedef union __attribute__ ((packed)) _BD_STAT 
{
    struct __attribute__ ((packed)){
        unsigned            :2;     //Byte count
        unsigned    BSTALL  :1;     //Buffer Stall Enable
        unsigned    DTSEN   :1;     //Data Toggle Synch Enable
        unsigned            :2;     //Reserved - write as 00
        unsigned    DTS     :1;     //Data Toggle Synch Value
        unsigned    UOWN    :1;     //USB Ownership
    };
    struct __attribute__ ((packed)){
        unsigned            :2;
        unsigned    PID0    :1;
        unsigned    PID1    :1;
        unsigned    PID2    :1;
        unsigned    PID3    :1;
    };
    struct __attribute__ ((packed)){
        unsigned            :2;
        unsigned    PID     :4;         //Packet Identifier
    };
    uint16_t           Val;
} BD_STAT;                      //Buffer Descriptor Status Register

// BDT Entry Layout
typedef union __attribute__ ((packed))__BDT
{
    union
    {
	    struct __attribute__ ((packed))
        {
	        BD_STAT         STAT;
	        uint16_t        CNT:10;
	        uint16_t        ADR;                      
	        uint16_t        ADRH;                   
        };
	    struct __attribute__ ((packed))
        {
	        uint32_t       res  :16;
	        uint32_t       count:10;
        };
    };
    uint32_t           Val;
	uint16_t            v[4];
	uint32_t            w[2];
} BDT_ENTRY;
// USTAT Register Layout
typedef union __USTAT
{
    struct
    {
        unsigned char filler1           :2;
        unsigned char ping_pong         :1;
        unsigned char direction         :1;
        unsigned char endpoint_number   :4;
    };
    uint8_t Val;
} USTAT_FIELDS;

//Macros for fetching parameters from a USTAT_FIELDS variable.
#define USBHALGetLastEndpoint(stat)     stat.endpoint_number
#define USBHALGetLastDirection(stat)    stat.direction
#define USBHALGetLastPingPong(stat)     stat.ping_pong


typedef union
{
    uint16_t UEP[16];
} _UEP;

typedef union _POINTER
{
    struct
    {
        uint8_t bLow;
        uint8_t bHigh;
        //byte bUpper;
    };
    uint16_t _word;                         // bLow & bHigh
    
    //pFunc _pFunc;                       // Usage: ptr.pFunc(); Init: ptr.pFunc = &<Function>;

    uint8_t* bRam;                         // Ram byte pointer: 2 bytes pointer pointing
                                        // to 1 byte of data
    uint16_t* wRam;                         // Ram word poitner: 2 bytes poitner pointing
                                        // to 2 bytes of data

    const uint8_t* bRom;                     // Size depends on compiler setting
    const uint16_t* wRom;
    //rom near byte* nbRom;               // Near = 2 bytes pointer
    //rom near word* nwRom;
    //rom far byte* fbRom;                // Far = 3 bytes pointer
    //rom far word* fwRom;
} POINTER;

/*****************************************************************************/
/****** Function prototypes and macro functions ******************************/
/*****************************************************************************/

#define ConvertToPhysicalAddress(a) ((uint16_t)(a))
#define ConvertToVirtualAddress(a)  ((void *)(a))
#define USBClearUSBInterrupt() IFS5bits.USB1IF = 0;
#if defined(USB_INTERRUPT)
    #define USBMaskInterrupts() {IEC5bits.USB1IE = 0;}
    #define USBUnmaskInterrupts() {IEC5bits.USB1IE = 1;}
#else
    #define USBMaskInterrupts() 
    #define USBUnmaskInterrupts() 
#endif

//STALLIE, IDLEIE, TRNIE, and URSTIE are all enabled by default and are required
#if defined(USB_INTERRUPT)
    #define USBEnableInterrupts() {IEC5bits.USB1IE = 1;IPC21bits.USB1IP = 4;}
#else
    #define USBEnableInterrupts()
#endif

#define USBDisableInterrupts() {IEC5bits.USB1IE=0;}
#define USBInterruptFlag                IFS5bits.USB1IF



#if (USB_PULLUP_OPTION == USB_PULLUP_ENABLE) || !defined(USB_PULLUP_OPTION)
    #define PullUpConfiguration() U1OTGCONbits.OTGEN = 0;
#else
    #define PullUpConfiguration() U1OTGCONbits.OTGEN = 1; U1OTGCON &= 0xFF0F;
#endif

    #define SetConfigurationOptions()   {\
                                            U1CNFG1 = 0;\
                                            U1CNFG2 = USB_TRANSCEIVER_OPTION;\
                                            PullUpConfiguration();\
                                            U1EIE = 0x9F;\
                                            U1IE = 0x99 | USB_SOF_INTERRUPT | USB_ERROR_INTERRUPT;\
                                        } 


/********************************************************************
Function:
    bool USBSleepOnSuspend(void)
    
Summary:
    Places the core into sleep and sets up the USB module
    to wake up the device on USB activity.
    
PreCondition:
    IPL (in the SR register) must be non-zero.
    
Parameters:
    None
    
Return Values:
    true  - if entered sleep successfully
    false - if there was an error entering sleep
    
Remarks:
    Please note that before calling this function that it is the
    responsibility of the application to place all of the other
    peripherals or board features into a lower power state if
    required.

*******************************************************************/
bool USBSleepOnSuspend(void);

/****************************************************************
    Function:
        void USBPowerModule(void)
        
    Description:
        This macro is used to power up the USB module if required<br>
        PIC18: defines as nothing<br>
        PIC24: defines as U1PWRCbits.USBPWR = 1;<br>
        
    Parameters:
        None
        
    Return Values:
        None
        
    Remarks:
        None
        
  ****************************************************************/
#define USBPowerModule() U1PWRCbits.USBPWR = 1;

/****************************************************************
    Function:
        void USBModuleDisable(void)
        
    Description:
        This macro is used to disable the USB module
        
    Parameters:
        None
        
    Return Values:
        None
        
    Remarks:
        None
        
  ****************************************************************/
#define USBModuleDisable() {\
    U1CON = 0;\
    U1IE = 0;\
    U1OTGIE = 0;\
    U1PWRCbits.USBPWR = 1;\
    USBDeviceState = DETACHED_STATE;\
}    

/****************************************************************
    Function:
        USBSetBDTAddress(addr)
        
    Description:
        This macro is used to power up the USB module if required
        
    Parameters:
        None
        
    Return Values:
        None
        
    Remarks:
        None
        
  ****************************************************************/
#define USBSetBDTAddress(addr)         U1BDTP1 = (((unsigned int)addr)/256);

/****************************************************************
    Function:
        void USBClearInterruptRegister(int register)
        
    Description:
        Clears all of the interrupts in the requested register
        
    Parameters:
        register - the regsister that needs to be cleared.
        
    Return Values:
        None
        
    Remarks:
        Note that on these devices to clear an interrupt you must
        write a '1' to the interrupt location.
        
  ****************************************************************/
#define USBClearInterruptRegister(reg) reg = 0xFFFF;

/********************************************************************
    Function:
        void USBClearInterruptFlag(register, uint8_t if_flag_offset)
        
    Summary:
        Clears the specified USB interrupt flag.
        
    PreCondition:
        None
        
    Parameters:
        register - the register mnemonic for the register holding the interrupt 
                   flag to be cleared
        uint8_t if_flag_offset - the bit position offset (for the interrupt flag to
                   clear) from the "right of the register"
        
    Return Values:
        None
        
    Remarks:
        Individual USB interrupt flag bits are cleared by writing '1' to the 
        bit, in a word write operation.
 
 *******************************************************************/
#define USBClearInterruptFlag(reg_name, if_flag_offset)	(reg_name = (1 << if_flag_offset))	

/********************************************************************
    Function:
        void DisableNonZeroEndpoints(UINT8 last_ep_num)
        
    Summary:
        Clears the control registers for the specified non-zero endpoints
        
    PreCondition:
        None
        
    Parameters:
        UINT8 last_ep_num - the last endpoint number to clear.  This
        number should include all endpoints used in any configuration.
        
    Return Values:
        None
        
    Remarks:
        None
 
 *******************************************************************/
#define DisableNonZeroEndpoints(last_ep_num) memset((void*)&U1EP1,0x00,(last_ep_num * 2));                                          


/*****************************************************************************/
/****** Compiler checks ******************************************************/
/*****************************************************************************/
#ifndef USB_PING_PONG_MODE
    #error "No ping pong mode defined."
#endif  //Device or OTG

/*****************************************************************************/
/****** Extern variable definitions ******************************************/
/*****************************************************************************/

#if defined(USB_SUPPORT_DEVICE) | defined(USB_SUPPORT_OTG)
    #if !defined(USBDEVICE_C)
        //extern USB_VOLATILE USB_DEVICE_STATE USBDeviceState;
        extern USB_VOLATILE uint8_t USBActiveConfiguration;
        extern USB_VOLATILE IN_PIPE inPipes[1];
        extern USB_VOLATILE OUT_PIPE outPipes[1];
    #endif
	extern volatile BDT_ENTRY* pBDTEntryOut[USB_MAX_EP_NUMBER+1];
	extern volatile BDT_ENTRY* pBDTEntryIn[USB_MAX_EP_NUMBER+1];	
#endif

#endif  //USB_HAL_DSPIC33E_H
