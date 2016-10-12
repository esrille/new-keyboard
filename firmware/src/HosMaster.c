/*
 * Copyright 2016 Esrille Inc.
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

/*
 * ⚠ ATTENTION
 *
 * This file must not be modified to maintain Bluetooth® Qualification of the
 * Esrille New Keyboard - NISSE completed by Esrille Inc.
 *
 * See "Bluetooth Qualification and Declaration Processes" for more details:
 *   https://www.bluetooth.org/en-us/test-qualification/qualification-overview
 */

#include <xc.h>
#include <HosMaster.h>
#include <pps.h>
#include <usart.h>
#include <spi.h>
#include <string.h>

#ifdef ESRILLE_NEW_KEYBOARD     // for HID bootloader?
#include "HardwareProfile.h"
#else
#include "system.h"
#include "app_led_usb_status.h"
#include "app_device_keyboard.h"
#include "app_device_mouse.h"
#endif

#include <Keyboard.h>
#ifdef ENABLE_MOUSE
#include <Mouse.h>
#endif

// BSP indication states
#define ADVERTISING_DIRECTED_LED_ON_INTERVAL   200      // Directed advertising
#define ADVERTISING_DIRECTED_LED_OFF_INTERVAL  200      // Period 0.4 sec, duty cycle 50%
#define ADVERTISING_WHITELIST_LED_ON_INTERVAL  300      // Fast/slow whitelist advertising
#define ADVERTISING_WHITELIST_LED_OFF_INTERVAL 700      // Period 1 sec, duty cycle 30%
#define ADVERTISING_LED_ON_INTERVAL            200      // Fast advertising
#define ADVERTISING_LED_OFF_INTERVAL           800      // Period 1 sec, duty cycle 20%
#define ADVERTISING_SLOW_LED_ON_INTERVAL       100      // Slow advertizing
#define ADVERTISING_SLOW_LED_OFF_INTERVAL      900      // Period 1 sec, duty cycle 10%
#define BONDING_INTERVAL                       100      // Bonding

#define CS_LAT      LATDbits.LATD5
#define CS_TRIS     TRISDbits.TRISD5

#define RETRY_MAX   5
#define RETRY_WAIT  128 // [usec]

#define BATTERY_LEVELS_SIZE                     100     // from 2.00 (200) to 2.99 (299)
#define BATTERY_LEVEL_MEAS_INTERVAL             (WDT_FREQ * HOS_BATTERY_LEVEL_MEAS_INTERVAL / 1000)

static uint8_t status[HOS_STATE_COMMON_LAST + 1];

typedef struct Info {
    uint8_t revisionMajor;
    uint8_t revisionMinor;
    uint8_t versionMajor;
    uint8_t versionMinor;
} Info;

typedef struct Tsap {
    uint8_t x;
    uint8_t y;
    uint8_t touchLo;
    uint8_t touchHi;
} Tsap;

static Info     info;
static Tsap     tsap;

#ifndef ESRILLE_NEW_KEYBOARD    // i.e. not for bootloader
static uint16_t battery_voltage;
static uint8_t  battery_level;

static const uint8_t battery_levels[BATTERY_LEVELS_SIZE] = {
//  .00  .01  .02  .03  .04  .05  .06  .07  .08  .09
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   1,   1,   1,   1,
      1,   1,   1,   1,   2,   2,   2,   2,   3,   3,
      4,   5,   5,   6,   7,   8,   9,  10,  11,  12,
     14,  15,  17,  18,  21,  24,  28,  33,  40,  47,
     54,  64,  72,  76,  78,  80,  82,  83,  85,  86,
     87,  88,  89,  90,  91,  92,  93,  93,  94,  95,
     95,  96,  96,  97,  97,  98,  98,  99,  99,  99,
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
};
#endif

void HosInitialize(void)
{
    // CTS  CS      CS      RD5/RP22
    // TXD  MOSI    SDO     RD4/RP21
    // RXD  MISO    SDI     RC7/RX1/RP18
    // RTS  SCK     SCK     RC6/TX1/RP17

    CS_LAT = 1;
    CS_TRIS = 0;

    TRISDbits.TRISD4 = 0;   // SDO
    TRISCbits.TRISC7 = 1;   // SDI
    TRISCbits.TRISC6 = 0;   // SCK

    PPSUnLock();
    iPPSInput(IN_FN_PPS_SDI2, IN_PIN_PPS_RP18);     // RP18
    iPPSInput(IN_FN_PPS_SCK2IN, IN_PIN_PPS_RP17);   // RP17
    iPPSOutput(OUT_PIN_PPS_RP17, OUT_FN_PPS_SCK2);  // RP17
    iPPSOutput(OUT_PIN_PPS_RP21, OUT_FN_PPS_SDO2);  // RP21
    PPSLock();

    memset(status, 0, sizeof status);
}

uint8_t HosXfer(uint8_t byte)
{
    WriteSPI2(byte);
    return SSP2BUF;
}

static int8_t CheckProfile(uint8_t profile)
{
    return ((~profile >> 4) & 0x0f) == (profile & 0x0f);
}

int8_t HosReport(uint8_t type, uint8_t cmd, uint8_t len, const uint8_t* data)
{
    uint8_t buffer[HOS_STATE_LAST + 1];
    uint8_t state;
    int8_t good = 0;

    CloseSPI2();
    OpenSPI2(SPI_FOSC_64, MODE_00, SMPMID); // Use MODE_00 for SPI_MODE_0 of nRF51

    for (int8_t retry = 0; retry < RETRY_MAX; ++retry) {
        state = 0;

        __delay_us(1);
        CS_LAT = 0;
        __delay_us(8);  // Wait nRF51 SPIS for 7.1 [us].

        buffer[state++] = HosXfer(type);
        buffer[state++] = HosXfer(cmd);
        buffer[state++] = HosXfer(len);
        if (len == 0) {
            buffer[state++] = HosXfer(HOS_CMD_NONE);  // Send a dummy command.
        } else {
            buffer[state++] = HosXfer(data[0]);
            for (uint8_t i = 1; i < len; ++i) {
                if (state <= HOS_STATE_LAST)
                    buffer[state++] = HosXfer(data[i]);
                else
                    HosXfer(data[i]);
            }
        }
        while (state <= HOS_STATE_LAST)
            buffer[state++] = HosXfer(HOS_CMD_NONE);  // Send a dummy command.

        __delay_us(2);
        CS_LAT = 1;

        if (buffer[0] == HOS_DEF_CHARACTER) {
            __delay_us(RETRY_WAIT);
            continue;
        }

        if (CheckProfile(buffer[HOS_STATE_PROFILE])) {
            memmove(status, buffer, HOS_STATE_COMMON_LAST + 1);
            switch (status[HOS_STATE_TYPE]) {
            case HOS_TYPE_INFO:
                memmove(&info, buffer + HOS_STATE_REV_MAJOR, HOS_STATE_VER_MINOR - HOS_STATE_REV_MAJOR + 1);
                break;
            case HOS_TYPE_TSAP:
                memmove(&tsap, buffer + HOS_STATE_X, HOS_STATE_TOUCH_HI - HOS_STATE_X + 1);
                break;
            default:
                break;
            }
            good = 1;
        }
        break;
    }
    CloseSPI2();
    return good;
}

int8_t HosGetStatus(uint8_t type)
{
    return HosReport(type, HOS_CMD_GET_STATUS, 0, NULL);
}

int8_t HosSetEvent(uint8_t type, uint8_t key)
{
    return HosReport(type, HOS_CMD_SET_EVENT, 1, &key);
}

int8_t HosSleep(uint8_t type)
{
    int8_t good = HosSetEvent(type, HOS_EVENT_SLEEP);
    if (good) {
        status[HOS_STATE_INDICATE] = HOS_BLE_STATE_IDLE;
    }
    return good;
}

#ifndef ESRILLE_NEW_KEYBOARD    // i.e. not for bootloader
uint16_t HosGetBatteryVoltage(void)
{
    return battery_voltage;
}

uint8_t HosGetBatteryLevel(void)
{
    uint8_t  level;
    uint16_t voltage = HosGetBatteryVoltage();

    if (300 <= voltage)
        level = 100u;
    else if (voltage < 200)
        level = 0u;
    else
        level = battery_levels[voltage - 200];
    return level;
}

static uint8_t HosUpdateBatteryLevel(uint16_t tick)
{
    int8_t good = 1;

    if (!(tick % BATTERY_LEVEL_MEAS_INTERVAL)) {
        uint16_t v = HOS_BATTERY_VOLTAGE_OFFSET + status[HOS_STATE_BATT];
        uint16_t diff = (battery_voltage < v) ? (v - battery_voltage) : (battery_voltage - v);
        if (50 < diff) {
            battery_voltage = v;
        } else {
            // Apply low pass filter:
            // 0.75 * prev + (1 - 0.75) * current
            battery_voltage += (v >> 2) - (battery_voltage >> 2);
        }

        uint8_t level = HosGetBatteryLevel();
        if (battery_level != level) {
            battery_level = level;
            good = HosSetBatteryLevel(HOS_TYPE_DEFAULT, battery_level);
        }
    }
    return good;
}

#endif

int8_t HosSetBatteryLevel(uint8_t type, uint8_t level)
{
    return HosReport(type, HOS_CMD_BATT_REPORT, 1, &level);
}

uint8_t HosGetLED(void)
{
    return status[HOS_STATE_LED];
}

uint8_t HosGetProfile(void)
{
    return status[HOS_STATE_PROFILE] & 0x0f;
}

uint8_t HosGetIndication(void)
{
    return status[HOS_STATE_INDICATE] & ~(HOS_BLE_STATE_LESC | HOS_BLE_STATE_SUSPENDED);
}

uint8_t HosGetSuspended(void)
{
    return (status[HOS_STATE_INDICATE] & HOS_BLE_STATE_SUSPENDED) ? 1 : 0;
}

uint8_t HosGetLESC(void)
{
    return (status[HOS_STATE_INDICATE] & HOS_BLE_STATE_LESC) ? 1 : 0;
}

static int8_t inRange(uint16_t onInterval, uint16_t offInterval, uint16_t tick)
{
    uint16_t range = onInterval + offInterval;

    tick *= (1000 / WDT_FREQ);  // tick to msec
    tick %= range;
    return tick < onInterval;
}

void HosUpdateLED(LED led, uint16_t tick)
{
    uint8_t indicate = HOS_BLE_STATE_IDLE;

    if (led != LED_NONE) {
        indicate = HosGetIndication();
    }
    switch (indicate) {
    case HOS_BLE_STATE_SCANNING:
    case HOS_BLE_STATE_ADVERTISING:
        if (inRange(ADVERTISING_LED_ON_INTERVAL, ADVERTISING_LED_OFF_INTERVAL, tick))
            LED_On(led);
        else
            LED_Off(led);
        break;
    case HOS_BLE_STATE_ADVERTISING_WHITELIST:
        if (inRange(ADVERTISING_WHITELIST_LED_ON_INTERVAL, ADVERTISING_WHITELIST_LED_OFF_INTERVAL, tick))
            LED_On(led);
        else
            LED_Off(led);
        break;
    case HOS_BLE_STATE_ADVERTISING_SLOW:
        if (inRange(ADVERTISING_SLOW_LED_ON_INTERVAL, ADVERTISING_SLOW_LED_OFF_INTERVAL, tick))
            LED_On(led);
        else
            LED_Off(led);
        break;
    case HOS_BLE_STATE_ADVERTISING_DIRECTED:
        if (inRange(ADVERTISING_DIRECTED_LED_ON_INTERVAL, ADVERTISING_DIRECTED_LED_OFF_INTERVAL, tick))
            LED_On(led);
        else
            LED_Off(led);
        break;
    case HOS_BLE_STATE_BONDING:
        if (inRange(BONDING_INTERVAL, BONDING_INTERVAL, tick))
            LED_On(led);
        else
            LED_Off(led);
        break;
    case HOS_BLE_STATE_CONNECTED:
        break;
    default:
        LED_Off(LED_D1);
        LED_Off(LED_D2);
        LED_Off(LED_D3);
        break;
    }
}

uint16_t HosGetTouch(void)
{
    return (tsap.touchHi << 8) | tsap.touchLo;
}

uint8_t HosGetKeyboardMouseX(void)
{
    return tsap.x;
}

uint8_t HosGetKeyboardMouseY(void)
{
    return tsap.y;
}

uint16_t HosGetVersion(void)
{
    return (info.versionMajor << 8) | info.versionMinor;
}

uint16_t HosGetRevision(void)
{
    return (info.revisionMajor << 8) | info.revisionMinor;
}

void HosCheckDFU(bool dfu)
{
    // Enable watchdog timer
    WDTCONbits.REGSLP = 1;
    WDTCONbits.SWDTEN = 1;

    bool responded = false;
    for (uint16_t i = 0; i < HOS_STARTUP_DELAY; ++i) {
        if (HosGetStatus(HOS_TYPE_INFO)) {
            responded = true;
            if (!dfu)
                break;
            if (HosSetEvent(HOS_TYPE_INFO, HOS_EVENT_DFU))
                break;
        }
        Sleep();
        Nop();
    }

    LED_Off(LED_D1);
    LED_Off(LED_D2);
    LED_Off(LED_D3);

    if (dfu || !responded) {
        for (uint16_t tick = 0;; ++tick) {
            Sleep();
            Nop();
            if (HosGetStatus(HOS_TYPE_INFO))
                break;
            uint16_t range = tick * (1000 / WDT_FREQ) % 1000;
            if (range < 500)
                LED_On(LED_D3);
            else
                LED_Off(LED_D3);
        }
    }

    // Disable watchdog timer
    WDTCONbits.SWDTEN = 0;
}


#ifndef ESRILLE_NEW_KEYBOARD

// Lower the clock frequency to extend battery life.
// Note lowering frequency saves battery better than sleeping with WDT.
static void WaitForResume(void)
{
    APP_LEDUpdate(0);

    WDTCONbits.REGSLP = 1;
    WDTCONbits.SWDTEN = 0;
    APP_Suspend();

    while (!BUTTON_IsPressed()) {
        _delay(16000 / 32);  // Note _delay(1) = 32 [usec] while suspended at 125kHz.
    }

    APP_WakeFromSuspend();
    // Enable watchdog timer again
    WDTCONbits.REGSLP = 1;
    WDTCONbits.SWDTEN = 1;
}

void HosMainLoop(void)
{
    static int8_t starting = 1;
    static uint8_t mouse_report[4];

    if (isUSBMode() && isBusPowered())
        return;

    // Save ~0.5mA
    PMDIS0 = 0xfb;
    PMDIS1 = 0xfe;
    PMDIS2 = 0x5f;
    PMDIS3 = 0xfe;

    // Enable watchdog timer
    WDTCONbits.REGSLP = 1;
    WDTCONbits.SWDTEN = 1;

    for (uint16_t i = 0; i < HOS_STARTUP_DELAY; ++i) {
        if (HosGetStatus(HOS_TYPE_INFO)) {
            break;
        }
        Sleep();
        Nop();
    }
    LED_Off(LED_D1);
    LED_Off(LED_D2);
    LED_Off(LED_D3);

    for (uint16_t tick = 0;; ++tick)
    {
        uint8_t* keyboard_report = APP_KeyboardScan();

        if (isUSBMode()) {
            if (isBusPowered()) {
                HosGetStatus(HOS_TYPE_INFO);  // Get info after reset.
                Reset();
                Nop();
                Nop();
                // NOT REACHED HERE
            }
        }

        if (HosGetProfile() != CurrentProfile()) {
            for (uint8_t i = 0; !HosGetStatus(HOS_TYPE_DEFAULT) && i < HOS_SYNC_DELAY; ++i) {
                Sleep();
                Nop();
            }
            if (HosGetProfile() != CurrentProfile()) {
                HosSetEvent(HOS_TYPE_DEFAULT, HOS_EVENT_KEY_0 + CurrentProfile());
                APP_LEDUpdate(1u << (CurrentProfile() -1));
            }
            tick = 0;   // Reset
        }
        else
        {
            switch (HosGetIndication()) {
            case HOS_BLE_STATE_IDLE:
                if (keyboard_report || starting) {
                    starting = 1;
                    tick = 0;   // Reset
                    HosGetStatus(HOS_TYPE_DEFAULT);
                }
                HosUpdateLED(CurrentProfile(), tick);
                break;

            case HOS_BLE_STATE_ADVERTISING:
            case HOS_BLE_STATE_ADVERTISING_WHITELIST:
            case HOS_BLE_STATE_ADVERTISING_SLOW:
            case HOS_BLE_STATE_ADVERTISING_DIRECTED:
                starting = 0;
                HosGetStatus(HOS_TYPE_DEFAULT);
                // A new bonding process can be interrupted if there are pre-bonded peers that are active.
                // In such a case, the BLE module timers are also reset, and we must manually stop
                // advertising if a new bonding cannot be made within a reasonable time.
                if (HOS_ADV_TIMEOUT < tick) {
                    HosSleep(HOS_TYPE_DEFAULT);
                }
                HosUpdateLED(CurrentProfile(), tick);
                break;

            case HOS_BLE_STATE_BONDING:
                if (keyboard_report) {
                    // Send HOS_CMD_KEYBOARD_REPORT anyway to support passkey entry.
                    HosReport(HOS_TYPE_DEFAULT, HOS_CMD_KEYBOARD_REPORT, 8, keyboard_report);
                } else {
                    HosGetStatus(HOS_TYPE_DEFAULT);
                }
                HosUpdateLED(CurrentProfile(), tick);
                break;

            case HOS_BLE_STATE_CONNECTED:
                if (keyboard_report) {
                    HosReport(HOS_TYPE_DEFAULT, HOS_CMD_KEYBOARD_REPORT, 8, keyboard_report);
                } else {
                    HosGetStatus(HOS_TYPE_DEFAULT);
                }
#ifdef ENABLE_MOUSE
                // Do not report unchanged state.
                processMouseData();
                if (mouse_report[0] != getKeyboardMouseButtons() ||
                    mouse_report[1] != 0 || mouse_report[1] != getKeyboardMouseX() ||
                    mouse_report[2] != 0 || mouse_report[2] != getKeyboardMouseY() ||
                    mouse_report[3] != 0 || mouse_report[3] != getKeyboardMouseWheel())
                {
                    mouse_report[0] = getKeyboardMouseButtons();
                    mouse_report[1] = getKeyboardMouseX();
                    mouse_report[2] = getKeyboardMouseY();
                    mouse_report[3] = getKeyboardMouseWheel();
                    HosReport(HOS_TYPE_DEFAULT, HOS_CMD_MOUSE_REPORT, sizeof mouse_report, mouse_report);
                }
#endif
                APP_LEDUpdate(controlLED(HosGetLED()));
                HosUpdateBatteryLevel(tick);
                break;

            default:
                APP_LEDUpdate(LED_NUM_LOCK | LED_CAPS_LOCK | LED_SCROLL_LOCK);
                HosGetStatus(HOS_TYPE_INFO);  // Get info after reset.
                Reset();
                Nop();
                Nop();
                // NOT REACHED HERE
                break;
            }

            if (HosGetSuspended() || HosGetIndication() == HOS_BLE_STATE_IDLE)
                WaitForResume();
        }

        Sleep();
        Nop();
    }
}

#endif // ESRILLE_NEW_KEYBOARD
