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

#ifndef HOS_MASTER_H
#define HOS_MASTER_H

#include <stdint.h>
#include <leds.h>

#include <Hos.h>

#define HOS_STARTUP_DELAY   (WDT_FREQ * 4u)
#define HOS_SYNC_DELAY      (WDT_FREQ / 2u)     // Usually it takes about 240 msec to 300 msec to restart.
#define HOS_ADV_TIMEOUT     (WDT_FREQ * 210u)   // > APP_ADV_FAST_TIMEOUT + APP_ADV_SLOW_TIMEOUT

void HosInitialize(void);

int8_t HosReport(uint8_t type, uint8_t cmd, uint8_t len, const uint8_t* data);
int8_t HosGetStatus(uint8_t type);
int8_t HosSetEvent(uint8_t type, uint8_t key);
int8_t HosSetBatteryLevel(uint8_t type, uint8_t level);
int8_t HosSleep(uint8_t type);
int8_t HosReset(uint8_t gpregret);

uint8_t HosGetLED(void);
uint8_t HosGetProfile(void);
uint16_t HosGetBatteryVoltage(void);
uint8_t HosGetBatteryLevel(void);
uint8_t HosGetIndication(void);
uint8_t HosGetSuspended(void);
uint8_t HosGetLESC(void);

void HosUpdateLED(LED led, uint16_t tick);

// Information
uint16_t HosGetVersion(void);
uint16_t HosGetRevision(void);

// TSPA
uint16_t HosGetTouch(void);
uint8_t HosGetKeyboardMouseX(void);
uint8_t HosGetKeyboardMouseY(void);

void HosCheckDFU(bool dfu);
void HosMainLoop(void);

#endif // HOS_MASTER_H
