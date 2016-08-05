/*
 * Copyright 2016 Esrille Inc.
 *
 * This file is a modified version of main.c provided by
 * Microchip Technology, Inc. for using Esrille New Keyboard.
 * See the file NOTICE and the Software License Agreement below for the
 * License.
 */
/*********************************************************************
 *
 *      Microchip USB C18/XC8 HID Bootloader specific usb_device.c
 *       (modified from usbdrv.c included in MCHPFSUSB v1.2/v1.3)
 *
 *********************************************************************
 * FileName:        usb_device.c
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC16 or PIC18 USB microcontrollers
 * Compiler:        C18 3.46+ or XC8 v1.21+
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the "Company") for its PICmicro(R) Microcontroller is intended and
 * supplied to you, the Company's customer, for use solely and
 * exclusively on Microchip PICmicro Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * File version         Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 1.0                  11/19/04    Original.
 * 1.2                  08/14/07    Bug fixes.
 * 1.4                  05/31/07    A few non-critical changes made to
 *                                  decrease code size for use in
 *                                  bootloader application.
 * 1.5                  05/07/09    Small update to work with the
 *                                  new usbctrltrf.c file.
 * 1.6                  06/10/13    Minor cleanup and optimization.
 * 2.9j+                10/08/2013  Modified project to be buildable with XC8 v1.21
 *                                  or later compiler (as well as C18 v3.46+). Also
 *                                  re-organized and renamed several files to make this
 *                                  project look more like other projects in the MLA.
 *                                  Turned on ping pong bufferring on EP0 OUT, for
 *                                  better control transfer handling.
 ********************************************************************/

/** I N C L U D E S **********************************************************/
#include "usb.h"
#include "HardwareProfile.h"             // Required for USBCheckBusStatus()


//Private prototypes (see usb_device.h for public API functions exposed by the USB stack)
void USBCheckStdRequest(void);
void USBSuspend(void);
void USBProtocolResetHandler(void);
void USBWakeFromSuspend(void);
void USBStdGetDscHandler(void);
void USBStdSetCfgHandler(void);
void USBStdGetStatusHandler(void);
void USBStdFeatureReqHandler(void);
void USBCtrlTrfSetupHandler(void);
void USBCtrlTrfInHandler(void);
void USBCtrlTrfTxService(void);
void USBCtrlEPServiceComplete(void);
void LoadBDTandSetUOWN(uint8_t BDTIndexToLoad);
#if !defined(ENABLE_CONTROL_TRANSFERS_WITH_OUT_DATA_STAGE)
#define USBCtrlTrfOutHandler(a)
#endif



/** V A R I A B L E S ********************************************************/
#ifndef __XC8__
#pragma udata
#endif
uint8_t bTRNIFCount;
uint8_t ctrl_trf_state;             // Control Transfer State
uint8_t ctrl_trf_session_owner;     // Current transfer session owner
POINTER pSrc;                       // Data source pointer
POINTER pDst;                       // Data destination pointer
WORD_VAL wCount;                    // Data counter
uint8_t short_pkt_status;           // Flag used by Control Transfer Read
CTRL_TRF_SETUP SetupPkt;
bool EP0OutOddNeedsArmingNext;
BDT TempBDT;
uint8_t usb_device_state;          // Device States: DETACHED, ATTACHED, ...
USB_DEVICE_STATUS usb_stat;        // Global USB flags (remote wakeup armed status, etc.)
uint8_t usb_active_cfg;            // Value of current configuration
uint8_t usb_alt_intf[MAX_NUM_INT]; // Array to keep track of the current alternate
                                   // setting for each interface ID

#ifdef __XC8__
    #pragma warning disable 1090        //Disable unused variable warning.  USTATSave may not be used, depending upon user settings, but that is okay/harmless/expected.
#endif
uint8_t USTATSave;
uint16_t Counter;
bool DeviceIsSoftDetached;

/** USB FIXED LOCATION VARIABLES ***********************************/
#if defined(__18F14K50) || defined(__18F13K50) || defined(__18LF14K50) || defined(__18LF13K50)
    #define BDT_ADDR 0x200              //BDT in bank 2 on these devices
#elif defined(__18F47J53) || defined(__18F46J53) || defined(__18F27J53) || defined(__18F26J53) || defined(__18LF47J53) || defined(__18LF46J53) || defined(__18LF27J53) || defined(__18LF26J53)
    #define BDT_ADDR 0xD00              //BDT in Bank 13 on these devices
#elif defined(__18F97J94) || defined(__18F87J94) || defined(__18F67J94) || defined(__18F96J99) || defined(__18F86J99) || defined(__18F66J99) || defined(__18F96J94) || defined(__18F86J94) || defined(__18F66J94) || defined(__18F95J94) || defined(__18F85J94) || defined(__18F65J94)
    #define BDT_ADDR 0x100      // The USB BDT is located in Bank 1 on the PIC18F97J94 Family of devices
#elif defined(__16F1459) || defined(__16LF1459) || defined(__16F1455) || defined(__16LF1455) || defined(__16F1454) || defined(__16LF1454)
    #define BDT_ADDR    0x20
    #define USB_RAM_BUFF_ADDR   0x40    //EP0 OUT and IN buffers
    #define USB_HID_BUFF_OUT_ADDR  @0xA0
    #define USB_HID_BUFF_IN_ADDR   @0x120
#else
    //Most other PIC18 USB families use bank 4 for the BDT (ex: PIC18F46J50 family, PIC18F87J50 family, PIC18F4550/3 family, etc.)
    #define BDT_ADDR 0x400
#endif

#ifndef __XC8__
    #pragma udata USB_BDT = BDT_ADDR
#else
    #ifndef USB_RAM_BUFF_ADDR
        #define USB_RAM_BUFF_ADDR   (BDT_ADDR + 12 + (MAX_EP_NUMBER * 8))
    #endif
#endif

/******************************************************************************
 * Section A: Buffer Descriptor Table - must be allocated by the linker to the
 *            correct address that the hardware USB module DMA engine will be looking at.
 * - 0xX00 - 0xXFF( 256 bytes max)
 * - MAX_EP_NUMBER is defined in usb_config.h
 * - BDT data type is defined in usb_device.h
 *****************************************************************************/
#ifdef __XC8__
    #define BDT_ADDR_TAG    @BDT_ADDR
    #define BDT_ADDR_TAG_EP0O_EVEN   @BDT_ADDR
    #define BDT_ADDR_TAG_EP0O_ODD    BDT_ADDR_TAG_EP0O_EVEN+4
    #define BDT_ADDR_TAG_EP0I   BDT_ADDR_TAG+8
    #define BDT_ADDR_TAG_EP1O   BDT_ADDR_TAG+12
    #define BDT_ADDR_TAG_EP1I   BDT_ADDR_TAG+16
    #define USB_EP0_BUFF_ADDR   @USB_RAM_BUFF_ADDR
    #define USB_EP0_BUFF_ADDR2  USB_EP0_BUFF_ADDR+EP0_BUFF_SIZE
    #define USB_CTRL_TRF_DATA_ADDR USB_EP0_BUFF_ADDR2+EP0_BUFF_SIZE
    #if defined(USB_USE_HID)
        #ifndef USB_HID_BUFF_OUT_ADDR
            #define USB_HID_BUFF_OUT_ADDR  USB_CTRL_TRF_DATA_ADDR+EP0_BUFF_SIZE
            #define USB_HID_BUFF_IN_ADDR   USB_HID_BUFF_OUT_ADDR+HID_INT_OUT_EP_SIZE
        #endif
    #endif
#else
    //Else must be using the C18 compiler.  The C18 compiler puts things in
    //consecutive order with a udata section, so no explicit addresses for
    //each BDT/buffer is needed, provided the variables go into the correct
    //udata section (with the right start address of the BDT).
    #define BDT_ADDR_TAG
    #define BDT_ADDR_TAG_EP0O_EVEN
    #define BDT_ADDR_TAG_EP0O_ODD
    #define BDT_ADDR_TAG_EP0I
    #define BDT_ADDR_TAG_EP1O
    #define BDT_ADDR_TAG_EP1I
    #define USB_EP0_BUFF_ADDR
    #define USB_EP0_BUFF_ADDR2
    #define USB_CTRL_TRF_DATA_ADDR
    #define USB_HID_BUFF_OUT_ADDR
    #define USB_HID_BUFF_IN_ADDR
#endif

#if(0 <= MAX_EP_NUMBER)
volatile BDT ep0BoEven BDT_ADDR_TAG_EP0O_EVEN;         //Endpoint #0 BD Out EVEN
volatile BDT ep0BoOdd BDT_ADDR_TAG_EP0O_ODD;          //Endpoint #0 BD Out ODD
volatile BDT ep0Bi BDT_ADDR_TAG_EP0I;         //Endpoint #0 BD In
#endif

#if(1 <= MAX_EP_NUMBER)
volatile BDT ep1Bo BDT_ADDR_TAG_EP1O;         //Endpoint #1 BD Out
volatile BDT ep1Bi BDT_ADDR_TAG_EP1I;         //Endpoint #1 BD In
#endif

#if(2 <= MAX_EP_NUMBER)
volatile BDT ep2Bo;         //Endpoint #2 BD Out
volatile BDT ep2Bi;         //Endpoint #2 BD In
#endif

#if(3 <= MAX_EP_NUMBER)
volatile BDT ep3Bo;         //Endpoint #3 BD Out
volatile BDT ep3Bi;         //Endpoint #3 BD In
#endif

#if(4 <= MAX_EP_NUMBER)
volatile BDT ep4Bo;         //Endpoint #4 BD Out
volatile BDT ep4Bi;         //Endpoint #4 BD In
#endif

#if(5 <= MAX_EP_NUMBER)
volatile far BDT ep5Bo;         //Endpoint #5 BD Out
volatile far BDT ep5Bi;         //Endpoint #5 BD In
#endif

#if(6 <= MAX_EP_NUMBER)
volatile far BDT ep6Bo;         //Endpoint #6 BD Out
volatile far BDT ep6Bi;         //Endpoint #6 BD In
#endif

#if(7 <= MAX_EP_NUMBER)
volatile far BDT ep7Bo;         //Endpoint #7 BD Out
volatile far BDT ep7Bi;         //Endpoint #7 BD In
#endif

#if(8 <= MAX_EP_NUMBER)
volatile far BDT ep8Bo;         //Endpoint #8 BD Out
volatile far BDT ep8Bi;         //Endpoint #8 BD In
#endif

#if(9 <= MAX_EP_NUMBER)
volatile far BDT ep9Bo;         //Endpoint #9 BD Out
volatile far BDT ep9Bi;         //Endpoint #9 BD In
#endif

#if(10 <= MAX_EP_NUMBER)
volatile far BDT ep10Bo;        //Endpoint #10 BD Out
volatile far BDT ep10Bi;        //Endpoint #10 BD In
#endif

#if(11 <= MAX_EP_NUMBER)
volatile far BDT ep11Bo;        //Endpoint #11 BD Out
volatile far BDT ep11Bi;        //Endpoint #11 BD In
#endif

#if(12 <= MAX_EP_NUMBER)
volatile far BDT ep12Bo;        //Endpoint #12 BD Out
volatile far BDT ep12Bi;        //Endpoint #12 BD In
#endif

#if(13 <= MAX_EP_NUMBER)
volatile far BDT ep13Bo;        //Endpoint #13 BD Out
volatile far BDT ep13Bi;        //Endpoint #13 BD In
#endif

#if(14 <= MAX_EP_NUMBER)
volatile far BDT ep14Bo;        //Endpoint #14 BD Out
volatile far BDT ep14Bi;        //Endpoint #14 BD In
#endif

#if(15 <= MAX_EP_NUMBER)
volatile far BDT ep15Bo;        //Endpoint #15 BD Out
volatile far BDT ep15Bi;        //Endpoint #15 BD In
#endif

/******************************************************************************
 * Section B: EP0 endpoint data buffer space - must be located in USB module
 *            accessible RAM region
 *****************************************************************************/
volatile uint8_t EP0OutEvenBuf[EP0_BUFF_SIZE] USB_EP0_BUFF_ADDR;
volatile uint8_t EP0OutOddBuf[EP0_BUFF_SIZE] USB_EP0_BUFF_ADDR2;
volatile CTRL_TRF_DATA CtrlTrfData USB_CTRL_TRF_DATA_ADDR;

/******************************************************************************
 * Section C: HID application interrupt endpoint buffers - must be located in
 *            USB module accessible RAM region
 *****************************************************************************/
#if defined(USB_USE_HID)
volatile unsigned char hid_report_out[HID_INT_OUT_EP_SIZE] USB_HID_BUFF_OUT_ADDR;
volatile unsigned char hid_report_in[HID_INT_IN_EP_SIZE] USB_HID_BUFF_IN_ADDR;
#endif

#ifndef __XC8__
#pragma udata
#endif



//If the user is not using an I/O pin for USB +5V VBUS presence detection (ex: because
//they are using a bus powered only application), default to assuming that VBUS
//is effectively always present (which is true for bus powered only applications).
#if !defined(USE_USB_BUS_SENSE_IO)
    #define usb_bus_sense       1
#endif

//If the user is not using an I/O pin for self powered detection (ex: because
//the application is bus powered only or is dual self/bus powered but uses less
//than 100mA max from VBUS), default to assuming that the device is currently bus
//powered.
#if !defined(USE_SELF_POWER_SENSE_IO)
    #define self_power          0
#endif



/** D E C L A R A T I O N S **************************************************/
#ifndef __XC8__
#pragma code
#endif


/******************************************************************************
 * Function:        void USBDeviceInit(void)
 *
 * PreCondition:    User application code is responsible for making sure the
 *                  currently selected oscillator settings are compatible with
 *                  USB operation, prior to calling this function.  The user
 *                  application code must also initialize the VBUS sensing I/O
 *                  pin before calling this code, if it uses VBUS sensing
 *                  (only needed in self powered applications).
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine initializes the USB stack state machine
 *                  and turns on the USB module.  It should be called only
 *                  once during bootup of the application code.
 *
 * Note:            None
 *****************************************************************************/
void USBDeviceInit(void)
{
    //Check if the USB module is already enabled.  If so, disable it and wait
    //~100ms+ (>1 second recommended if CDC application firmware on XP), to
    //ensure that the host has a chance to see and process the USB device detach
    //event.
    if(UCONbits.USBEN == 1)
    {
        //USB module was already on.  This is likely because the user applicaiton
        //firmware jumped into this bootloader firmware using the absolute
        //software entry method, without first turning off the USB module
        USBDisableWithLongDelay();
    }
    DeviceIsSoftDetached = FALSE;

    //Make initial call to USBCheckBusStatus() which will either do nothing (if
    //VBUS is not present), or, will totally re-init everything and perform
    //a soft attach event (if VBUS is present, or the user doesn't use
    //VBUS sense I/O in their application).  Either way, everything gets
    //totally re-initialized either now, or in the future, once VBUS 0->1 transition
    //is detected.
    USBCheckBusStatus();
}


//Turns on the USB module and attaches to the host.  Normally, this function should
//only be called after you are already sure that VBUS is present/the USB cable
//is attached to a powered host.
void USBSoftAttach(void)
{
    //Check if we are transitioning from deliberate soft detached to attached
    //state.  If so, need to delay a relatively long time (ex: 100ms+) to ensure
    //enough detached dwell time is met, to ensure the host correctly detects
    //the detach/reattach event (very short detachment less than possible via
    //a human unplug/reattachment event can sometimes be misprocessed by the host).
    if(DeviceIsSoftDetached == TRUE)
    {
        USBDisableWithLongDelay();
    }
    //(Re-)initialize registers and turn on the USB module
    UCON = 0;
    UCFG = UCFG_VAL;
    UIE = 0;
    UCONbits.USBEN = 1;
    //(Re-)initialize various USB stack variables and other things
    USBProtocolResetHandler();
    usb_device_state = ATTACHED_STATE;
    DeviceIsSoftDetached = FALSE;
}

//Disables the USB module and performs a soft detach from the USB host, regardless
//of the detected VBUS status.  However, if you call this function, and you are
//not using VBUS I/O sensing in your application you must stop calling USBCheckBusStatus(),
//since it will promptly try to re-enable the USB module.
void USBSoftDetach(void)
{
    //Disable USB module (this doesn't need to be done in the loop, but we
    //add it to the loop to deliberately make the loop execute slower)
    UCONbits.SUSPND = 0;
    UCON = 0x00;            //Disable module
    usb_device_state = DETACHED_STATE;
    DeviceIsSoftDetached = TRUE;
}

/******************************************************************************
 * Function:        void USBCheckBusStatus(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine enables/disables the USB module by monitoring
 *                  the USB power signal.
 *
 * Note:            None
 *****************************************************************************/
void USBCheckBusStatus(void)
{
    //Check if the user's firmware deliberately soft detached from the host.
    //If so, don't automatically try to reconnect, until the user subsequently
    //calls the USBSoftAttach() API.
    if(DeviceIsSoftDetached == TRUE)
    {
        return;
    }

    /**************************************************************************
     * Bus Attachment & Detachment Detection
     * usb_bus_sense is an i/o pin defined in HardwareProfile.h
     *************************************************************************/
    #define USB_BUS_ATTACHED    1
    #define USB_BUS_DETACHED    0

    #ifdef USE_USB_BUS_SENSE_IO
        if(usb_bus_sense == USB_BUS_ATTACHED)       // Is USB bus attached?
        {
            if(UCONbits.USBEN == 0)                 // Is the module off?
            {
                USBSoftAttach();                    // Is off, enable it
            }
        }
        else
        {
            if(UCONbits.USBEN == 1)                 // Is the module on?
            {
                USBSoftDetach();
                DeviceIsSoftDetached = FALSE;
            }
        }//end if(usb_bus_sense...)
    #else
        if(UCONbits.USBEN == 0)                 // Is the module off?
            USBSoftAttach();                    // Is off, enable it
    #endif
}//end USBCheckBusStatus







/******************************************************************************
 * Function:        void USBDeviceTasks(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine is the heart of this firmware. It manages
 *                  all USB interrupts/events that need processing.
 *
 * Note:            Device state transitions through the following stages:
 *                  DETACHED -> ATTACHED -> POWERED -> DEFAULT ->
 *                  ADDRESS_PENDING -> ADDRESSED -> CONFIGURED.  Once in the
 *                  CONFIGURED state the application is ready to run.  Only
 *                  in the CONFIGURED state should the host try to send/receive
 *                  packets on the application endpoints (ex: endpoints > 0).
 *****************************************************************************/
void USBDeviceTasks(void)
{
    static volatile BDT* pBDTEntry;
    static uint8_t i;

    //Check for possible changes in VBUS (USB connection) status.
    USBCheckBusStatus();

    /*
     * Pointless to continue servicing if USB cable is not even attached.
     */
    if(usb_device_state == DETACHED_STATE) return;

    /*
     * Task A: Service USB Activity Interrupt
     */

    if(UIRbits.ACTVIF)    USBWakeFromSuspend();

    /*
     * Pointless to continue servicing if the device is in suspend mode.
     */
    if(UCONbits.SUSPND == 1) return;

    /*
     * Task B: Service USB Bus Reset Interrupt.
     * When bus reset is received during suspend, ACTVIF will be set first,
     * once the UCONbits.SUSPND is clear, then the URSTIF bit will be asserted.
     * This is why URSTIF is checked after ACTVIF.
     */
    if(UIRbits.URSTIF)    USBProtocolResetHandler();

    /*
     * Task C: Service other USB interrupts
     */
    if(UIRbits.IDLEIF)    USBSuspend();

    //Don't need to actually do anything for SOF packets (except in isochronous devices)
    //if(UIRbits.SOFIF && UIEbits.SOFIE)      USB_SOF_Handler();

    //Don't need to actually do anything for STALL packets being sent.  For stalls
    //on EP0, the host is required to send a new SETUP packet, which the hardware
    //accepts (even if the BSTALL bit is set), as STALL is not a valid response to a
    //SETUP packet.
    //if(UIRbits.STALLIF)  USBStallHandler();


    /*
     * Pointless to continue servicing if the host has not sent a bus reset.
     * Once bus reset is received, the device transitions into the DEFAULT
     * state and is ready for communication.
     */
    if(usb_device_state < DEFAULT_STATE) return;

    /*
     * Task D: Servicing USB Transaction Complete Interrupt
     */
    for(bTRNIFCount = 0; bTRNIFCount < 4; bTRNIFCount++)
    {
        if(UIRbits.TRNIF)
        {
            //Check what endpoint and direction the last packet was sent or received on.
            USTATSave = USTAT;   //AND out any bits other than EP dir + num + ping pong dest
            if((USTAT & 0x7C) == EP00_OUT)
            {
                //Check if it was even or odd EP0 OUT buffer
                if(USTATbits.PPBI == 0)
                {
                    //The OUT or SETUP packet arrived on EP0 OUT EVEN endpoint
                    pBDTEntry = &ep0BoEven;
                }
                else
                {
                    //The OUT or SETUP packet arrived on EP0 OUT ODD endpoint
                    pBDTEntry = &ep0BoOdd;
                }

                //Allow USTAT FIFO to advance, done with the value
                UIRbits.TRNIF = 0;

                //Check if the current EP0 OUT buffer has a SETUP packet (or a regular OUT instead)
                if(pBDTEntry->Stat.PID == SETUP_TOKEN)
                {
                    //Copy the 8 bytes of packet data that arrived for the SETUP transaction
                    //from the USB endpoint buffer, into the global SetupPkt buffer.
                    for(i = 0; i < sizeof(CTRL_TRF_SETUP); i++)
                    {
                        SetupPkt._byte[i] = *pBDTEntry->ADR++;
                    }

                    //Handle the control transfer (parse the 8-byte SETUP command and
                    //figure out what to do with the host's request)
                    USBCtrlTrfSetupHandler();
                }
                else
                {
                    //Handle the conventional OUT DATA packet the just arrived from the host
                    USBCtrlTrfOutHandler(USTATSave);
                }
            }
            else if(USTAT == EP00_IN)                       // EP0 IN
            {
                //A packet just got sent to the host over the EP0 IN endpoint
                UIRbits.TRNIF = 0;
                USBCtrlTrfInHandler();  //Figure out what to do now (ex: such as preparing the next IN data packet in the sequence)
            }
            else
            {
                //The last packet sent or received was on EP1 or higher.  In this
                //case the USB stack doesn't need to handle anything, since the
                //user's application code is responsible for checking status/arming
                //the applicaiton endpoints
                UIRbits.TRNIF = 0;         //Just clear TRNIF to advance the USTAT FIFO.
            }
        }//if(UIRbits.TRNIF)
        else
        {
            break;
        }
    }// end for(bTRNIFCount = 0; bTRNIFCount < 4; bTRNIFCount++)

}//end USBDeviceTasks()

/******************************************************************************
 * Function:        void USBSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        When the USB host sends USB suspend signalling, this function
 *                  gets called.  In order to be USB compliant, the device
 *                  firmware should configure the application so that it takes
 *                  no more than 2.5mA from the +5VBus supply from the
 *                  USB port during USB suspend events.  Bus powered devices can
 *                  meet this by suspending the USB module, and either clock switching
 *                  to a low frequency or sleeping.  This is not necessary if the
 *                  application is self powered, or if it doesn't need to meet
 *                  all USB compliance requirements.
 *
 * Note:            None
 *****************************************************************************/
void USBSuspend(void)
{
    static unsigned char UIESave;
    #if defined(_PIC14E)
        static unsigned char INTCONSave;
    #endif

    /*
     * NOTE: Do not clear UIRbits.ACTVIF here!
     * Reason:
     * ACTVIF is only generated once an IDLEIF has been generated.
     * This is a 1:1 ratio interrupt generation.
     * For every IDLEIF, there will be only one ACTVIF regardless of
     * the number of subsequent bus transitions.
     *
     * If the ACTIF is cleared here, a problem could occur when:
     * [       IDLE       ][bus activity ->
     * <--- 3 ms ----->     ^
     *                ^     ACTVIF=1
     *                IDLEIF=1
     *  #           #           #           #   (#=Program polling flags)
     *                          ^
     *                          This polling loop will see both
     *                          IDLEIF=1 and ACTVIF=1.
     *                          However, the program services IDLEIF first
     *                          because ACTIVIE=0.
     *                          If this routine clears the only ACTIVIF,
     *                          then it can never get out of the suspend
     *                          mode.
     */

    //After the suspend condition is detected, we should do what is necessary
    //to reduce USB VBUS power consumption from the host down to <2.5mA.
    //For self powered devices, no action is really strictly necessary.
    //For bus powered devices, either the microcontroller must clock switch or
    //enter sleep mode to conserve enough power to meet the 2.5mA limit.

    UIESave = UIE;      //Save UIE values, only want to wake on certain events
    UIE = 0b00000100;   //Enabling the ACTVIF interrupt source only
                        //Since ACTVIF triggers on any bus activity, it will also trigger on USB reset events.
                        //But that is okay, we want that.

    UIRbits.IDLEIF = 0;     //Clear interrupt flag, we are servicing the idle event
    UCONbits.SUSPND = 1;    // Put USB module in power conserve
                            // mode, SIE clock inactive

    //Disable all microcontroller wake up sources, except for the one(s) which will
    //be used to wake up the microcontroller.  At the very least, the USB activity
    //detect interrupt should be enabled as a wake up source.

    USBIF_FLAG = 0;     //Won't get clear if an enabled and pending wake up source was already triggered
                        //However, since only the ACTVIF interrupt source is currently enabled,
                        //only bus activity events will prevent entry into sleep.

    USBIE_BIT = 1;      //Set USB wakeup source

    #if defined(_PIC14E)
        INTCONbits.PEIE = 1;     //Enable interrupts to wake device on PIC16 (but GIE = 0, so no vectoring)
    #endif

    //Now call the user's callback function, so that they can configure I/O pins
    //for low power states, and then either sleep the microcontroller or clock
    //switch to a lower power state.
    USBCBSuspend();

    #if defined(_PIC14E)
        INTCONbits.PEIE = 0;     //Clear interrupts
    #endif


    USBIE_BIT = 0;
    UIE |= UIESave;     //Restore UIE to state it was in prior to entering USB suspend (with the ACTVIF enabled as well, used later in stack)
                        //USB suspend events do not by themselves cause any loss of
                        //state information inside either the USB device firmware, or
                        //in the USB host software.
}//end USBSuspend

/******************************************************************************
 * Function:        void USBWakeFromSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:
 *
 * Note:            None
 *****************************************************************************/
void USBWakeFromSuspend(void)
{
    //Call the user's callback function letting them know of wake up from suspend
    //state.  The user is responsible for clock switching back to fully USB compatible
    //clock settings, prior to returning from the USBCBWakeFromSuspend() function.
    USBCBWakeFromSuspend();

    //Now bring the USB module out of suspend state and clear the interrupt flag.
    UCONbits.SUSPND = 0;    //Bring USB module hardware out of suspend mode/begin clocking it normally
    UIEbits.ACTVIE = 0;     //Disable ACTVIF as a USB wake up source (until another USB suspend condition is detected)
    while(UIRbits.ACTVIF){UIRbits.ACTVIF = 0;}  //May not be immediately clearable if USB module not yet clocked, therefore, using loop to allow osc startup time if necessary

}//end USBWakeFromSuspend








/******************************************************************************
 * Function:        void USBProtocolResetHandler(void)
 *
 * PreCondition:    A USB bus reset is received from the host (or we have just
 *                  turned on the USB module).
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    Totally re-initializes USB stack variables and several
 *                  USB module hardware settings as well.
 *
 * Overview:        Once a USB bus reset is received from the host, this
 *                  routine should be called. It resets the device address to
 *                  zero, disables all non-EP0 endpoints, initializes EP0 to
 *                  be ready for default communication, clears all USB
 *                  interrupt flags, unmasks applicable USB interrupts, and
 *                  reinitializes internal state-machine variables.
 *
 * Note:            None
 *****************************************************************************/
void USBProtocolResetHandler(void)
{
    usb_device_state = DEFAULT_STATE;
    UEIE = 0;                       // Not using USB error interrupts (no special handling required anyway)
    UIR = 0;                        // Clears all USB interrupts
    UIE = 0b01111011;               // Enable all interrupts except ACTVIE
    UADDR = 0x00;                   // Reset to default address
    mDisableEP1to7();               // Reset all non-EP0 UEPn registers
    UEP0 = EP_CTRL|HSHK_EN;         // Init EP0 as a Ctrl EP, see usb_device.h
    UCONbits.PPBRST = 1;            // Reset ping pong buffer pointers
    while(UIRbits.TRNIF == 1)       // Flush any pending transactions
    {
        UIRbits.TRNIF = 0;
        ClearWatchdog();    //5 Tcy minimum (2 for call, 2 for return, 1 for clearing) to allow TRNIF to (potentially) reassert
    }
    UCONbits.PPBRST = 0;
    UCONbits.PKTDIS = 0;            // Make sure packet processing is enabled

    //Prepare EP0 OUT Even to receive the first SETUP packet
    TempBDT.Stat._byte = _DAT0|_BSTALL;
    LoadBDTandSetUOWN(EP0_OUT_EVEN_BDT_INDEX);    //Configures address/size fields and sets UOWN
    EP0OutOddNeedsArmingNext = TRUE;
    usb_stat._byte = 0x00;          // Clear USB flags (like remote wakeup armed status)
    usb_active_cfg = 0;             // Clear active configuration
    USBCBInitEP(0);                 // Call application callback function to give it notification
                                    // it is getting un-configured (ex: equiv of set configuration to 0).
}//end USBProtocolResetHandler



/******************************************************************************
 * Function:        void USBCtrlTrfSetupHandler(void)
 *
 * PreCondition:    SetupPkt buffer is loaded with valid USB Setup Data
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine is a task dispatcher and has 3 stages.
 *                  1. It initializes the control transfer state machine.
 *                  2. It calls on each of the module that may know how to
 *                     service the Setup Request from the host.
 *                     Module Example: USB9, HID, CDC, MSD, ...
 *                     As new classes are added, the USBCBCheckOtherReq() function
 *                     in main.c should be modified to call all of the new class handlers.
 *                  3. Once each of the modules has had a chance to check if
 *                     it is responsible for servicing the request, stage 3
 *                     then checks direction of the transfer to determine how
 *                     to prepare EP0 for the control transfer.
 *                     Refer to USBCtrlEPServiceComplete() for more details.
 *
 * Note:            Microchip USB Firmware has three different states for
 *                  the control transfer state machine:
 *                  1. WAIT_SETUP
 *                  2. CTRL_TRF_TX
 *                  3. CTRL_TRF_RX
 *
 *                  A Control Transfer is composed of many USB transactions.
 *                  When transferring data over multiple transactions,
 *                  it is important to keep track of data source, data
 *                  destination, and data count. These three parameters are
 *                  stored in pSrc,pDst, and wCount. A flag is used to
 *                  note if the data source is from ROM or RAM.
 *
 *****************************************************************************/
void USBCtrlTrfSetupHandler(void)
{
    //If we get to here, that means we just received a new SETUP packet from the
    //host.  In this case, we should re-initalize any control transfer status
    //tracking variables, since only one control transfer can be pending at a
    //time (and if the last one didn't fully complete for whatever reason,
    //abandon it).
    ep0Bi.Stat._byte = _UCPU;
    short_pkt_status = SHORT_PKT_NOT_SENT;

    //Make sure none of the EP0 OUT endpoints are still armed (one could still
    //be UOWN == 1, if EP0 OUT was previously double armed).
    //Clear UOWN on all EP0 OUT BDTs until we are done parsing/processing this SETUP.
    if(ep0BoEven.Stat.UOWN == 1)
    {
        ep0BoEven.Stat._byte = _UCPU;
        EP0OutOddNeedsArmingNext = FALSE;
    }
    if(ep0BoOdd.Stat.UOWN == 1)
    {
        ep0BoOdd.Stat._byte = _UCPU;
        EP0OutOddNeedsArmingNext = TRUE;
    }
    ctrl_trf_state = WAIT_SETUP;
    ctrl_trf_session_owner = MUID_NULL;     // Set owner to NULL so stack knows to STALL if no control transfer handlers know how to handle the request
    wCount.Val = 0;
    UCONbits.PKTDIS = 0;    //Clear PKTDIS bit now, since all EP0 IN/OUT endpoints are currently configured to NAK

    //Now that we are done re-initializing state tracking variables, it is
    //time to parse the recently received SETUP packet to determine how to respond
    //to the host's request (which could be USB 2.0 "Chapter 9" or a USB device
    //class specific request).
    USBCheckStdRequest();                   // Takes care of handling USB "Chapter 9" standard requests
    USBCBCheckOtherReq();                   // User callback function, which should call any class specific control transfer handler functions

    //Now do final endpoint servicing to complete our response for the request
    USBCtrlEPServiceComplete();

}//end USBCtrlTrfSetupHandler


/******************************************************************************
 * Function:        void USBCtrlTrfOutHandler(uint8_t)
 *
 * PreCondition:    None
 *
 * Input:           The USTAT register value for the most recently received
 *                  OUT data packet that we are processing.
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine handles an OUT transaction according to
 *                  which control transfer state is currently active.
 *
 * Note:            Note that if the the control transfer was from
 *                  host to device, the session owner should be notified
 *                  at the end of the transfer to service/consume the
 *                  received data.  To do this, the user must implement
 *                  a function called USBCBControlTransferOutDataReady()
 *                  in their application code, which consumes the OUT (from host)
 *                  data that was just received.  The USB stack will call this
 *                  user callback function once all the expected bytes are received
 *                  from the host.
 *
 *                  This is an optional function, and may be disabled to save code space by
 *                  commenting out the "#define ENABLE_CONTROL_TRANSFERS_WITH_OUT_DATA_STAGE"
 *                  option in usb_config.h, for applications that never use
 *                  host to device control transfers with OUT data stage.
 *
 *****************************************************************************/
#if defined (ENABLE_CONTROL_TRANSFERS_WITH_OUT_DATA_STAGE)
void USBCtrlTrfOutHandler(uint8_t USTATValue)
{
    uint8_t bytes_received;

    //Check if a host to device control transfer is in progress (ex: SETUP, OUT, OUT, ..., IN[status])
    if(ctrl_trf_state == CTRL_TRF_RX)
    {
        //We just received an OUT data packet from the host.  Need to copy it
        //to the control transfer session owner's receive buffer.
        //First: Figure out how many bytes arrived in the packet, and where they went

        //Check if the OUT packet arrived on the odd EP0 OUT endpoint
        if(USTATValue & 0x02)
        {
            //The OUT packet arrived on EP0 OUT Odd
            bytes_received = ep0BoOdd.Cnt;
            pSrc.bRam = ep0BoOdd.ADR;
        }
        else
        {
            //The OUT packet arrived on EP0 OUT Even
            bytes_received = ep0BoEven.Cnt;
            pSrc.bRam = ep0BoEven.ADR;
        }

        //Keep track of how many total bytes have been received in this OUT
        //control transfer (host to device), so we know when the host
        //is finished sending us all the data.
        wCount.Val = wCount.Val + bytes_received;

        //Copy the bytes received from the OUT endpoint buffer, into the user
        //specified buffer (the pDst pointer was setup at the start of the control transfer).
        while(bytes_received)
        {
            *pDst.bRam++ = *pSrc.bRam++;
            bytes_received--;
        }//end while(byte_to_read.word_val)


        //Check if there are anymore OUT packets in the request or not
        if(wCount.Val < SetupPkt.wLength)
        {
            //If we get to here, then the host is still going to send us more data.
            //Re-arm the OUT endpoint to receive the next OUT data packet
            if(EP0OutOddNeedsArmingNext == TRUE)
            {
                if(ep0BoEven.Stat.DTS == 1)
                {
                    TempBDT.Stat._byte = _DAT0 | _DTSEN;
                }
                else
                {
                    TempBDT.Stat._byte = _DAT1 | _DTSEN;
                }
                LoadBDTandSetUOWN(EP0_OUT_ODD_BDT_INDEX);
                EP0OutOddNeedsArmingNext = FALSE;
            }
            else
            {
                if(ep0BoOdd.Stat.DTS == 1)
                {
                    TempBDT.Stat._byte = _DAT0 | _DTSEN;
                }
                else
                {
                    TempBDT.Stat._byte = _DAT1 | _DTSEN;
                }
                LoadBDTandSetUOWN(EP0_OUT_EVEN_BDT_INDEX);
                EP0OutOddNeedsArmingNext = TRUE;
            }
        }//if(wCount.Val < SetupPkt.wLength)
        else
        {
            //No more OUT data packets expected, re-arm EP0 OUT to receive the
            //next SETUP packet, and then allow the IN status stage to complete
            TempBDT.Stat._byte = _BSTALL;
            if(EP0OutOddNeedsArmingNext == TRUE)
            {
                LoadBDTandSetUOWN(EP0_OUT_ODD_BDT_INDEX);
                EP0OutOddNeedsArmingNext = FALSE;
            }
            else
            {
                LoadBDTandSetUOWN(EP0_OUT_EVEN_BDT_INDEX);
                EP0OutOddNeedsArmingNext = TRUE;
            }

            //Allow the IN status stage to complete now.
            ep0Bi.Cnt = 0;
            ep0Bi.Stat._byte = _DAT1|_DTSEN;
            ep0Bi.Stat._byte |= _USIE;


            //Call the application's callback function now to consume/use
            //the data that has arrived as a control transfer over EP0 OUT...
            USBCBControlTransferOutDataReady();   //<--Implement a real application call back function of this name, to consume the received OUT data...
        }
    }//if(ctrl_trf_state == CTRL_TRF_RX)
    else //In this case the last OUT transaction must have been a status stage of a CTRL_TRF_TX
    {
        //Don't need to do anything here.  EP0 OUT was already double armed (when
        //we first processed the intial SETUP packet), so it is already ready
        //for a new SETUP packet.
    }
}//end USBCtrlTrfOutHandler()
#endif


/******************************************************************************
 * Function:        void USBCtrlTrfInHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine handles an IN transaction according to
 *                  which control transfer state is currently active.
 *
 * Note:            A Set Address Request must not change the actual address
 *                  of the device until the completion of the control
 *                  transfer (ex: after status stage packet is ACKed). The
 *                  end (status stage) of the control transfer for Set Address
 *                  Request is an IN transaction. Therefore it is necessary
 *                  to service this unique situation when the condition is
 *                  right.
 *****************************************************************************/
void USBCtrlTrfInHandler(void)
{
    //Check if we are in the address pending state.  If so, we just completed
    //the status stage of the SET ADDRESS command, and we now must switch
    //ourself to the new USB device address.
    if(usb_device_state == ADR_PENDING_STATE)
    {
        UADDR = SetupPkt.bDevADR;
        if(UADDR > 0)
            usb_device_state = ADDRESS_STATE;
        else
            usb_device_state = DEFAULT_STATE;
    }//end if

    //Now check what control transfer state we are in, so we know if the most
    //recent IN packet was a data stage packet, or a status stage packet.
    if(ctrl_trf_state == CTRL_TRF_TX)
    {
        //If we get to here, we must have just sent an IN data packet, as part
        //of a data stage (ex: SETUP, IN, IN [one of these data IN's just occurred], OUT[status, probably hasn't occurred yet])
        USBCtrlTrfTxService();

        //Check if we have finisihed sending a short packet or not.
        if(short_pkt_status == SHORT_PKT_SENT)
        {
            //If a short packet has been sent, don't want to send any more,
            //stall next time if host is tries to read more data than it originally requested.
            ep0Bi.Stat._byte = _BSTALL;
            ep0Bi.Stat._byte |= _USIE;
        }
        else
        {
            //We still have one or more IN data packets to send.  Arm the IN endpoint now.
            if(ep0Bi.Stat.DTS == 0)
                ep0Bi.Stat._byte = _DAT1|_DTSEN;
            else
                ep0Bi.Stat._byte = _DAT0|_DTSEN;

            ep0Bi.Stat._byte |= _USIE;
        }//end if(...)else
    }
    else // CTRL_TRF_RX
    {
        //The status stage of a host to device control transfer just completed (SETUP, OUT, OUT,...IN[status, just completed])
        //Don't need to do anything here, already armed EP0 OUT to be ready for the
        //next SETUP packet after the last OUT packet arrived.
    }

}//end USBCtrlTrfInHandler


/******************************************************************************
 * Function:        void USBCtrlTrfTxService(void)
 *
 * PreCondition:    pSrc, wCount, and usb_stat.ctrl_trf_mem are setup properly.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine should be called from only two places.
 *                  One from USBCtrlEPServiceComplete() and one from
 *                  USBCtrlTrfInHandler(). It takes care of managing a
 *                  device to host (IN to host) control transfer over multiple
 *                  USB transactions.
 *
 * Note:            None
 *****************************************************************************/
void USBCtrlTrfTxService(void)
{
    static uint8_t bytes_to_send;

    //First, have to figure out how many bytes of data to send in the transaction
    //Check if the number of byte remaining to be sent it less than one full width
    //packet worth of data payload.
    bytes_to_send = EP0_BUFF_SIZE;
    if(wCount.Val < EP0_BUFF_SIZE)
    {
        //The data remaining is less than a full width packet.  We will send a
        //short packet on the next transaction.
        bytes_to_send = wCount.Val;
        if(short_pkt_status == SHORT_PKT_NOT_SENT)
        {
            short_pkt_status = SHORT_PKT_PENDING;
        }
        else if(short_pkt_status == SHORT_PKT_PENDING)
        {
            short_pkt_status = SHORT_PKT_SENT;
        }
    }

    //Setup BDT so USB module knows how big of a USB packet to send IN to the host on the next EP0 IN transaction
    ep0Bi.Cnt = bytes_to_send;

    //Keep track of how many bytes still need to be sent in this control transfer
    wCount.Val -= bytes_to_send;

    //Now copy the data from the original source location (which was previously
    //setup by the Chapter 9 or class specific control transfer handler code), to
    //the USB endpoint buffer.
    pDst.bRam = (uint8_t*)&CtrlTrfData;        // Set destination pointer
    if(usb_stat.ctrl_trf_mem == _ROM)       // Determine type of memory source
    {
        while(bytes_to_send)
        {
            *pDst.bRam = *pSrc.bRom;
            pDst.bRam++;
            pSrc.bRom++;
            bytes_to_send--;
        }//end while(byte_to_send.Val)
    }
    else // RAM
    {
        while(bytes_to_send)
        {
            *pDst.bRam = *pSrc.bRam;
            pDst.bRam++;
            pSrc.bRam++;
            bytes_to_send--;
        }//end while(byte_to_send.Val)
    }//end if(usb_stat.ctrl_trf_mem == _ROM)

}//end USBCtrlTrfTxService



/******************************************************************************
 * Function:        void USBCtrlEPServiceComplete(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine wrap up the ramaining tasks in servicing
 *                  a Setup Request. Its main task is to set the endpoint
 *                  controls appropriately for a given situation. See code
 *                  below.
 *                  There are three main scenarios:
 *                  a) There was no handler for the Request, in this case
 *                     a STALL should be sent out.
 *                  b) The host has requested a read control transfer,
 *                     endpoints are required to be setup in a specific way.
 *                  c) The host has requested a write control transfer, or
 *                     a control data stage is not required, endpoints are
 *                     required to be setup in a specific way.
 *
 * Note:            None
 *****************************************************************************/
void USBCtrlEPServiceComplete(void)
{
    //Check if any USB chapter 9 or device class specific control transfer handlers
    //knew how to handle the request or not.
    if(ctrl_trf_session_owner == MUID_NULL)
    {
        //If we get to here, that means no request handlers knew how to handle
        //the host's control transfer request.  In this case, we are supposed
        //to send STALL response to the host, to let it know of the situation.
        ep0Bi.Stat._byte = _BSTALL;
        ep0Bi.Stat._byte |= _USIE;
        TempBDT.Stat._byte = _BSTALL;
        if(EP0OutOddNeedsArmingNext == TRUE)
        {
            LoadBDTandSetUOWN(EP0_OUT_ODD_BDT_INDEX);
            EP0OutOddNeedsArmingNext = FALSE;
        }
        else
        {
            LoadBDTandSetUOWN(EP0_OUT_EVEN_BDT_INDEX);
            EP0OutOddNeedsArmingNext = TRUE;
        }
    }
    else    // A module has claimed ownership of the control transfer session.
    {
        if(SetupPkt.DataDir == DEV_TO_HOST)
        {
            /*
             * If we get to here, this means a control read is occurring:
             * <SETUP[0]><IN[1]><IN[0]>...<OUT[1]> | <SETUP[0]>
             *
             * To handle this case, we need to arm both EP0 IN and OUT (twice)
             * so as to handle the case where the host performs early termination
             * (ex: when it sends the OUT status stage before is receives all
             * of the requested data).
             *
             * NOTE:
             * If something went wrong during the control transfer,
             * the last status stage may not be sent by the host.
             * When this happens, two different things could happen
             * depending on the host.
             * a) The host could send out a RESET.
             * b) The host could send out a new SETUP transaction
             *    without sending a RESET first.
             * To properly handle case (b), the OUT EP must be setup
             * to receive either a zero length OUT transaction, or a
             * new SETUP transaction.
             */

            //Set state machine to keep track of what we are doing accross multiple USB packets.
            ctrl_trf_state = CTRL_TRF_TX;

            //Boundary check how many bytes the transfer handler is expecting
            //to send IN to the host, versus the number that the host is actually
            //requesting.  We should always send the lesser of the two.
            if(SetupPkt.wLength < wCount.Val)
                wCount.Val = SetupPkt.wLength;

            //Allow the IN transaction hanlder function to execute, so as to copy
            //the requested IN data into the proper USB IN endpoint buffer.
            USBCtrlTrfTxService();

            //Double arm EP0 OUT (even and odd both active at the same time): Once
            //for the 0-byte status stage OUT packet, and once for the next 8-byte SETUP packet
            TempBDT.Stat._byte = _DAT1 | _DTSEN;  //DTS = 1 for the status stage, DTS ignored/irrelevant for SETUP packets
            LoadBDTandSetUOWN(EP0_OUT_ODD_BDT_INDEX);
            LoadBDTandSetUOWN(EP0_OUT_EVEN_BDT_INDEX);

            //Arm EP0 IN to send the first packet worth of data that the host has
            //requested for this particular control transfer.  The USBCtrlTrfTxService()
            //function will have already copied the data into the CtrlTrfData[] buffer.
            ep0Bi.ADR = (uint8_t*)&CtrlTrfData;
            ep0Bi.Stat._byte = _DAT1|_DTSEN;
            ep0Bi.Stat._byte |= _USIE;
        }//if(SetupPkt.DataDir == DEV_TO_HOST)
        else    //else we must be (SetupPkt.DataDir == HOST_TO_DEV)
        {
            /*
             * Control Write (with data stage):
             * <SETUP[0]><OUT[1]><OUT[0]>...<IN[1]> | <SETUP[0]>
             *
             * Certain host to device requests may not have any data stage, such
             * as the "set address" request:
             * <SETUP[0]> <IN[1]> | <SETUP[0]>
             */

            //Keep track of what we are doing, accross multiple USB packets.
            ctrl_trf_state = CTRL_TRF_RX;

            //Prepare OUT EP to receive either the first DATA1 OUT data packet in the host
            //to device control transfer, or the SETUP packet (if no data stage).
            //We only arm one of the EP0 OUT buffers for this.
            TempBDT.Stat._byte = _BSTALL;   //Assume initially we will get a SETUP
            //Check the length of the transfer, if is not 0, then the next packet will be a normal OUT instead
            if(SetupPkt.wLength == 0)
            {
                TempBDT.Stat._byte = _DAT1|_DTSEN;    //Prepare for normal OUT packet instead
            }
            //Check which EP0 out needs arming, and arm it.
            if(EP0OutOddNeedsArmingNext == TRUE)
            {
                LoadBDTandSetUOWN(EP0_OUT_ODD_BDT_INDEX);
                EP0OutOddNeedsArmingNext = FALSE;
            }
            else
            {
                LoadBDTandSetUOWN(EP0_OUT_EVEN_BDT_INDEX);
                EP0OutOddNeedsArmingNext = TRUE;
            }

            //Check if there is any actual OUT data in this transfer.  If there
            //is no data stage, then we can proceed to arm the status stage now.
            if(SetupPkt.wLength == 0)
            {
                //Arm the status stage 0-byte IN packet
                ep0Bi.Cnt = 0;
                ep0Bi.Stat._byte = _DAT1|_DTSEN;
                ep0Bi.Stat._byte |= _USIE;
            }
            //else
            //{
            //    //If there is OUT data pending to be received, we wait until we have
            //    //received the host's data before arming the status stage
            //}
        }//end if(SetupPkt.DataDir == DEV_TO_HOST)
    }//end if(ctrl_trf_session_owner == MUID_NULL)

}//end USBCtrlEPServiceComplete



/******************************************************************************
 * Function:        void USBCheckStdRequest(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine checks the setup data packet to see if it
 *                  knows how to handle it
 *
 * Note:            None
 *****************************************************************************/
void USBCheckStdRequest(void)
{
    if(SetupPkt.RequestType != STANDARD) return;

    switch(SetupPkt.bRequest)
    {
        case SET_ADR:
            ctrl_trf_session_owner = MUID_USB9;
            usb_device_state = ADR_PENDING_STATE;       // Update state only
            /* See USBCtrlTrfInHandler() in usbctrltrf.c for the next step */
            break;
        case GET_DSC:
            USBStdGetDscHandler();
            break;
        case SET_CFG:
            USBStdSetCfgHandler();
            break;
        case GET_CFG:
            ctrl_trf_session_owner = MUID_USB9;
            pSrc.bRam = (uint8_t*)&usb_active_cfg;         // Set Source
            usb_stat.ctrl_trf_mem = _RAM;               // Set memory type
            //LSB(wCount) = 1;                            // Set data count
            wCount.v[0] = 1;
            break;
        case GET_STATUS:
            USBStdGetStatusHandler();
            break;
        case CLR_FEATURE:
        case SET_FEATURE:
            USBStdFeatureReqHandler();
            break;
        case GET_INTF:
            ctrl_trf_session_owner = MUID_USB9;
            pSrc.bRam = (uint8_t*)&usb_alt_intf+SetupPkt.bIntfID;  // Set source
            usb_stat.ctrl_trf_mem = _RAM;               // Set memory type
            wCount.v[0] = 1;                            // Set data count
            break;
        case SET_INTF:
            ctrl_trf_session_owner = MUID_USB9;
            usb_alt_intf[SetupPkt.bIntfID] = SetupPkt.bAltID;
            break;
        case SET_DSC:
        case SYNCH_FRAME:
        default:
            break;
    }//end switch

}//end USBCheckStdRequest

/******************************************************************************
 * Function:        void USBStdGetDscHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine handles the standard GET_DESCRIPTOR request.
 *                  It utilizes tables dynamically looks up descriptor size.
 *                  This routine should never have to be modified if the tables
 *                  in usb_descriptors.c are declared correctly.
 *
 * Note:            None
 *****************************************************************************/
void USBStdGetDscHandler(void)
{
    if(SetupPkt.bmRequestType == 0x80)
    {
        switch(SetupPkt.bDscType)
        {
            case DSC_DEV:
                ctrl_trf_session_owner = MUID_USB9;
                pSrc.bRom = (ROM uint8_t*)&device_dsc;
                wCount.v[0] = sizeof(device_dsc);          // Set data count
                break;
            case DSC_CFG:
                //Error check to make sure the host is requesting a legal/implemented configuration descriptor
                if(SetupPkt.bDscIndex < USB_MAX_NUM_CONFIG_DSC)
                {
                    ctrl_trf_session_owner = MUID_USB9;
                    pSrc.bRom = (ROM BYTE*)&CFG01;
                    wCount.Val = sizeof(CFG01);              // Set data count
                }
                break;
            case DSC_STR:
                ctrl_trf_session_owner = MUID_USB9;
                pSrc.bRom = *(USB_SD_Ptr+SetupPkt.bDscIndex);
                wCount.Val = *pSrc.bRom;                  // Set data count
                break;
        }//end switch

        usb_stat.ctrl_trf_mem = _ROM;                       // Set memory type
    }//end if
}//end USBStdGetDscHandler

/******************************************************************************
 * Function:        void USBStdSetCfgHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine first disables all endpoints by clearing
 *                  UEP registers. It then configures (initializes) endpoints
 *                  specified in the modifiable section.
 *
 * Note:            None
 *****************************************************************************/
void USBStdSetCfgHandler(void)
{
    static unsigned char i;

    ctrl_trf_session_owner = MUID_USB9;
    //Initially disable all endpoints (except EP0, which we are still using)
    mDisableEP1to7();                          // See usb_device.h
    //Reset alternate interface selection settings to default value
    for(i = 0; i < MAX_NUM_INT; i++)
    {
        usb_alt_intf[i] = 0;
    }

    //Save the current configuration number that we have been set to
    usb_active_cfg = SetupPkt.bCfgValue;

    //Call the user's set configuration handler function.  Note: We pass
    //the user's set configuration handler the configuration number, since
    //the USB host could either configure us (set to !0), or de-configure
    //us (set to == 0).  The application firmware should disable endpoints
    //when getting de-configured.
    USBCBInitEP(usb_active_cfg);

    if(SetupPkt.bCfgValue == 0)
    {
        usb_device_state = ADDRESS_STATE;
    }
    else
    {
        usb_device_state = CONFIGURED_STATE;
    }//end if(SetupPkt.bcfgValue == 0)
}//end USBStdSetCfgHandler


/******************************************************************************
 * Function:        void USBStdGetStatusHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine handles the standard GET_STATUS request
 *
 * Note:            None
 *****************************************************************************/
void USBStdGetStatusHandler(void)
{
    CtrlTrfData._byte0 = 0;                         // Initialize content
    CtrlTrfData._byte1 = 0;

    switch(SetupPkt.Recipient)
    {
        case RCPT_DEV:
            ctrl_trf_session_owner = MUID_USB9;
            /*
             * _byte0: bit0: Self-Powered Status [0] Bus-Powered [1] Self-Powered
             *         bit1: RemoteWakeup        [0] Disabled    [1] Enabled
             */

            if(self_power == 1)                     // self_power defined in HardwareProfile.h
                CtrlTrfData._byte0 |= 0b00000001;   // Set bit0
            if(usb_stat.RemoteWakeup == 1)          // usb_stat defined in usbmmap.c
                CtrlTrfData._byte0|=0b00000010;     // Set bit1
            break;
        case RCPT_INTF:
            ctrl_trf_session_owner = MUID_USB9;     // No data to update
            break;
        case RCPT_EP:
            ctrl_trf_session_owner = MUID_USB9;
            /*
             * _byte0: bit0: Halt Status [0] Not Halted [1] Halted
             */
            pDst.bRam = (uint8_t*)&ep0BoEven+(SetupPkt.EPNum*8)+(SetupPkt.EPDir*4)+4;   //+4 is to skip past the EP0 OUT ODD BDT entry
            if(*pDst.bRam & _BSTALL)    // Use _BSTALL as a bit mask
                CtrlTrfData._byte0=0x01;// Set bit0
            break;
    }//end switch

    if(ctrl_trf_session_owner == MUID_USB9)
    {
        pSrc.bRam = (uint8_t*)&CtrlTrfData;            // Set Source
        usb_stat.ctrl_trf_mem = _RAM;               // Set memory type
        wCount.v[0] = 2;                            // Set data count
    }//end if(...)
}//end USBStdGetStatusHandler


/******************************************************************************
 * Function:        void USBStdFeatureReqHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine handles the standard SET & CLEAR FEATURES
 *                  requests
 *
 * Note:            None
 *****************************************************************************/
void USBStdFeatureReqHandler(void)
{
    //Check if the host GET STATUS request was for the device's remote wakeup armed status
    if((SetupPkt.bFeature == DEVICE_REMOTE_WAKEUP)&&(SetupPkt.Recipient == RCPT_DEV))
    {
        ctrl_trf_session_owner = MUID_USB9;
        if(SetupPkt.bRequest == SET_FEATURE)
            usb_stat.RemoteWakeup = 1;
        else
            usb_stat.RemoteWakeup = 0;
    }//end if

    //Check if the host GET STATUS request was for the endpoint halt status of an endpoint
    if((SetupPkt.bFeature == ENDPOINT_HALT)&&(SetupPkt.Recipient == RCPT_EP)&&(SetupPkt.EPNum != 0))
    {
        ctrl_trf_session_owner = MUID_USB9;
        /* Must do address calculation here */
        pDst.bRam = (uint8_t*)&ep0BoEven+(SetupPkt.EPNum*8)+(SetupPkt.EPDir*4)+4;

        if(SetupPkt.bRequest == SET_FEATURE)
        {
            *pDst.bRam = _BSTALL;
            *pDst.bRam |= _USIE;
        }
        else
        {
            if(SetupPkt.EPDir == 1) // IN
                *pDst.bRam = _UCPU|_DAT1;
            else
            {
                *pDst.bRam = _DAT0|_DTSEN;
                *pDst.bRam |= _USIE;
            }
        }//end if
    }//end if
}//end USBStdFeatureReqHandler



//This is a helper function for copying prepared BDT entry data from the global
//TempBDT buffer, into the real BDT EP0 OUT (even or odd) entry of the user's
//specified index.  Once copied, this function also sets the UOWN bit in the
//real BDT entry.  This helper function helps to reduce code space, by minimizing
//use of pointers in the rest of the code.  This function is only intended to
//be used to arm EP0 OUT (even or odd).
void LoadBDTandSetUOWN(uint8_t BDTIndexToLoad)
{
    static volatile BDT* pBDTEntry;

    //Setup the byte count and destination address values.  Since this function
    //is only intended to be used for EP0 OUT even/odd, the size should always
    //be the endpoint size (the host determines that actual size of packet sent),
    //and the address should always be the respective even/odd USB packet buffer.
    TempBDT.Cnt = EP0_BUFF_SIZE;
    TempBDT.ADR = (uint8_t*)&EP0OutOddBuf[0];
    if(BDTIndexToLoad == EP0_OUT_EVEN_BDT_INDEX)
    {
        TempBDT.ADR = (uint8_t*)&EP0OutEvenBuf[0];
        pBDTEntry = (volatile BDT*)BDT_ADDR;
    }
    else
    {
        pBDTEntry = (volatile BDT*)(BDT_ADDR + 4);
    }

    //Copy the global TempBDT contents into the user specified actual BDT location
    *pBDTEntry = TempBDT;

    //Now set the UOWN bit to arm the endpoint
    pBDTEntry->Stat.UOWN = 1;
}



//Disable the USB module and then wait a long time (long enough for the host
//to correctly detect the USB detach event, process the plug and play event,
//and get itself back into a state ready for a new USB attach event.
void USBDisableWithLongDelay(void)
{
    UCONbits.SUSPND = 0;    //Make sure not in suspend mode
    UCON = 0x00;            //Disable USB module
    for (uint8_t i = 0; i < 12; ++i)
        DelayRoutine(0xFFFF);   //Wait long time for host to recognize detach event
    usb_device_state = DETACHED_STATE;
}


//Helper function to execute some blocking delay.
void DelayRoutine(unsigned int DelayAmount)
{
    while(DelayAmount)
    {
        ClearWatchdog();
        DelayAmount--;
    }
}



//Helper function to reduce code size when built with C18.  The ClrWdt() is an
//inline assembly macro, and on the C18 compiler, if you execute an inline asm
//instruction in a C function, it prevents the compiler from implementing
//optimizations to that particular function (since the compiler doesn't know what
//the user did in the inline asm).  Therefore, inline asm is more efficient if
//implemented outside of large C functions, when using C18.
void ClearWatchdog(void)
{
    ClrWdt();
}


/** EOF usb_device.c *************************************************************/
