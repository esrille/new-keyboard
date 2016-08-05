;-------------------------------------------------------------------------------
;* Copyright 2016 Esrille Inc.
;*
;* This file is a modified version of VectorRemap.asm provided by
;* Microchip Technology, Inc. for using Esrille New Keyboard.
;* See the file NOTICE and the Software License Agreement below for the
;* License.
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
;* FileName:        VectorRemap.asm
;* Dependencies:    None
;* Processor:       PIC18
;* Compiler:        MPLAB C18 3.46+ or XC8 v1.21+
;* Company:         Microchip Technology, Inc.
;*
;* Software License Agreement
;*
;* The software supplied herewith by Microchip Technology Incorporated
;* (the "Company") for its PIC(R) Microcontroller is intended and
;* supplied to you, the Company's customer, for use solely and
;* exclusively on Microchip PIC Microcontroller products. The
;* software is owned by the Company and/or its supplier, and is
;* protected under applicable copyright laws. All rights are reserved.
;* Any use in violation of the foregoing restrictions may subject the
;* user to criminal sanctions under applicable laws, as well as to
;* civil liability for the breach of the terms and conditions of this
;* license.
;*
;* THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
;* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
;* TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
;* PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
;* IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
;* CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
;-------------------------------------------------------------------------------

    ;//High priority interrupt vector remapping
#ifdef __XC8__
    PSECT HiVector,class=CODE,delta=1,abs
#endif
        org 0x08
    goto 0x2008     ;Resides at 0x0008 (hardware high priority interrupt vector), and causes PC to jump to 0x1008 upon a high priority interrupt event


    ;//Low priority interrupt vector remapping, as well as bootloader mode absolute
    ;//entry point (located at 0x001C).
#ifdef __XC8__
    PSECT LoVector,class=CODE,delta=1,abs
#endif
        org 0x18
    goto    0x2018  ;Resides at 0x0018 (hardware low priority interrupt vector), and causes PC to jump to 0x1018 upon a low priority interrupt event
    goto    0x30    ;Resides at 0x001C  //Serves as absolute entry point from application program into the bootloader mode



    end