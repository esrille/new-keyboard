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

#ifndef HOS_H
#define HOS_H

#include <stdint.h>

//
// HID over SPI
//

#define HOS_DEF_CHARACTER                   0xAAu   // SPI default character. Character clocked out in case of an ignored transaction.

#define HOS_CMD_NONE                        0x00
#define HOS_CMD_GET_STATUS                  0xF1
#define HOS_CMD_SET_EVENT                   0xF2
#define HOS_CMD_BATT_REPORT                 0xF3
#define HOS_CMD_MOUSE_REPORT                0xF4
#define HOS_CMD_KEYBOARD_REPORT             0xF5

#define HOS_BATTERY_LEVEL_MEAS_INTERVAL     2000u   // Battery level measurement interval [msec]
#define HOS_BATTERY_VOLTAGE_OFFSET          180     // Battery voltage offset [1/100V]

#define HOS_STATE_PROFILE                   0
#define HOS_STATE_LED                       1
#define HOS_STATE_BATT                      2
#define HOS_STATE_INDICATE                  3
#define HOS_STATE_TYPE                      4
#define HOS_STATE_COMMON_LAST               4

#define HOS_STATE_REV_MAJOR                 5   // For HOS_TYPE_INFO
#define HOS_STATE_REV_MINOR                 6
#define HOS_STATE_VER_MAJOR                 7
#define HOS_STATE_VER_MINOR                 8

#define HOS_STATE_X                         5   // For HOS_TYPE_TSAP
#define HOS_STATE_Y                         6
#define HOS_STATE_TOUCH_LO                  7
#define HOS_STATE_TOUCH_HI                  8

#define HOS_STATE_LAST                      8

#define HOS_TYPE_NONE                       0
#define HOS_TYPE_INFO                       1
#define HOS_TYPE_TSAP                       2

#define HOS_BLE_STATE_IDLE                  0
#define HOS_BLE_STATE_SCANNING              1
#define HOS_BLE_STATE_ADVERTISING           2
#define HOS_BLE_STATE_ADVERTISING_WHITELIST 3
#define HOS_BLE_STATE_ADVERTISING_SLOW      4
#define HOS_BLE_STATE_ADVERTISING_DIRECTED  5
#define HOS_BLE_STATE_BONDING               6
#define HOS_BLE_STATE_CONNECTED             7
#define HOS_BLE_STATE_SUSPENDED             0x40
#define HOS_BLE_STATE_LESC                  0x80

// cf. BSP events
#define HOS_EVENT_NOTHING                   0
#define HOS_EVENT_DEFAULT                   1
#define HOS_EVENT_CLEAR_BONDING_DATA        2
#define HOS_EVENT_CLEAR_ALERT               3
#define HOS_EVENT_DISCONNECT                4
#define HOS_EVENT_ADVERTISING_START         5
#define HOS_EVENT_ADVERTISING_STOP          6
#define HOS_EVENT_WHITELIST_OFF             7
#define HOS_EVENT_BOND                      8
#define HOS_EVENT_RESET                     9
#define HOS_EVENT_SLEEP                     10
#define HOS_EVENT_WAKEUP                    11
#define HOS_EVENT_DFU                       12
#define HOS_EVENT_KEY_0                     13
#define HOS_EVENT_KEY_1                     14
#define HOS_EVENT_KEY_2                     15
#define HOS_EVENT_KEY_3                     16
#define HOS_EVENT_KEY_4                     17
#define HOS_EVENT_KEY_5                     18
#define HOS_EVENT_KEY_6                     19
#define HOS_EVENT_KEY_7                     20
#define HOS_EVENT_KEY_LAST                  HOS_EVENT_KEY_7

#endif  // HOS_H
