/*
 * Copyright 2014 Esrille Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>

#include <system.h>
#include <plib/usart.h>

#include "app_device_keyboard.h"

#define EEPROM_BT   7

void SendUSART(unsigned char data);
void SendCommand(const char *cmd);
void delay_ms(unsigned int msec);

static volatile bool outputReport = false;

int main(void)
{
    SYSTEM_Initialize(SYSTEM_STATE_USB_START);
    LED_Initialize();

    // Initialize the Bluetooth module by applying pulse of at least 160 μs
    LATDbits.LATD0 = 1;
    delay_ms(500);
    LATDbits.LATD0 = 0; // RN42 RESET_N
    __delay_us(200);
    LATDbits.LATD0 = 1;
    delay_ms(500);

    // Initialize USART
    baudUSART(BAUD_IDLE_RX_PIN_STATE_HIGH & BAUD_IDLE_TX_PIN_STATE_HIGH & BAUD_16_BIT_RATE & BAUD_WAKEUP_OFF & BAUD_AUTO_OFF);
    OpenUSART(USART_TX_INT_OFF & USART_RX_INT_ON & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, 51);
    INTCONbits.PEIE = 1;    // Enable peripheral interrupt
    INTCONbits.GIE = 1;     // Enable global interrupt

    uint8_t bt = eeprom_read(EEPROM_BT);
    if (bt == 0) {
        SendCommand("$$$");                     // Put RN42 into command mode
        SendCommand("SF,1\r");                  // Restore factory defaults
        SendCommand("SN,Esrille - NISSE\r");    // Set the device name to "Esrille - NISSE"
        SendCommand("SH,0207\r");               // Set the device as a keyboard
        SendCommand("S~,6\r");                  // Enable HID profile
        SendCommand("SM,6\r");                  // Automatically make connections without using GPIO6
        SendCommand("R,1\r");                   // Reboot to use HID profile
        bt = 1;
        eeprom_write(EEPROM_BT, bt);
    }

    APP_KeyboardInit();
    for (;;) {
        APP_KeyboardWait();
        uint8_t* report = APP_KeyboardScan();
        if (report) {
            // Send a raw HID report
            SendUSART(0xFD);
            SendUSART(9);
            SendUSART(1);
            for (char i = 0; i < 8; ++i)
                SendUSART(report[i]);
        }
        // Request the current status
        outputReport = true;
        SendUSART(0xFF);
    }
}

void SYSTEM_Disconnect(void)
{
    SendUSART(0x00);
}

void interrupt SYS_InterruptHigh(void)
{
    if (DataRdyUSART()) {
        if (RCSTAbits.OERR || RCSTAbits.FERR) {
            ReadUSART();    // Clear FERR
            RCSTA = 0;      // Clear OERR
            RCSTA = 0x90;   // Restart USARTs
        } else {
            uint8_t data = ReadUSART();    // Clear FERR
            if (outputReport) {
                outputReport = false;
                APP_SetOutputReport(data);
            }
        }
    }
}

void SendUSART(unsigned char data)
{
    while (BusyUSART())
        ;
    WriteUSART(data);
}

void SendCommand(const char* cmd)
{
    while (*cmd != 0)
        SendUSART(*cmd++);
    delay_ms(2000);
}

void delay_ms(unsigned int msec)
{
    unsigned int i;

    for (i = 0; i < msec; ++i)
        __delay_us(1000);
}
