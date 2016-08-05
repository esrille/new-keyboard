/*
 * Copyright 2015, 2016 Esrille Inc.
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

#include "Mouse.h"
#include "Keyboard.h"

#include <system.h>
#include <stdio.h>

typedef struct {
    uint8_t count;
    uint8_t x;
    uint8_t y;
    uint16_t touch;
} SerialData;

typedef struct {
    uint16_t current;
    uint16_t thresh;
    uint16_t low;
} TouchSensor;

#define PLAY_MAX    4

#define CODE_F1     (1*1+1)
#define CODE_F9     8
#define CODE_F10    9
#define CODE_F11    10
#define CODE_F12    (1*12+10)
#define CODE_U      (4*12+8)
#define CODE_I      (4*12+9)
#define CODE_O      (4*12+10)
#define CODE_D      (5*12+2)
#define CODE_J      (5*12+8)
#define CODE_K      (5*12+9)
#define CODE_L      (5*12+10)
#define CODE_Z      (6*12+0)
#define CODE_X      (6*12+1)
#define CODE_C      (6*12+2)
#define CODE_V      (6*12+3)
#define CODE_B      (6*12+4)
#define CODE_COMMA  (6*12+9)

#define PLAY_XY      24         // x or y value smaller than PLAY_XY should be ignored.

const static uint8_t playTable[PLAY_MAX] = {
    64, 56, 48, 40
};

static const uint8_t about[] = {
    KEY_P, KEY_A, KEY_D, KEY_SPACEBAR, 0
};

static uint8_t play;
static uint8_t tick;

static uint8_t buttons;
static int8_t  x;
static int8_t  y;
static int8_t  wheel;

static SerialData rawData;
static TouchSensor touchSensor;

#ifdef __XC8

void initMouse(void)
{
    play = ReadNvram(EEPROM_MOUSE);
    if (PLAY_MAX <= play)
        play = 0;
    touchSensor.current = touchSensor.thresh = 0;
}

void emitMouse(void)
{
    emitString(about);
    emitKey(KEY_1 + play);

#ifdef WITH_HOS
    emitKey(KEY_SPACEBAR);
    emitNumber(touchSensor.current);
    emitKey(KEY_SLASH);
    emitNumber(touchSensor.thresh);
#endif
    emitKey(KEY_ENTER);
}

static void setPlay(uint8_t val)
{
    if (PLAY_MAX <= val)
        play = 0;
    else
        play = val;
    WriteNvram(EEPROM_MOUSE, play);
}

void processMouseKeys(uint8_t* current, const uint8_t* processed)
{
    uint8_t b = 0;
    int8_t w = 0;

    for (uint8_t i = 2; i < 8; ++i) {
        uint8_t code = current[i];
        switch (code) {
        case CODE_F9:
            setPlay(3);
            break;
        case CODE_F10:
            setPlay(2);
            break;
        case CODE_F11:
            setPlay(1);
            break;
        case CODE_F12:
            setPlay(0);
            break;
        case CODE_J:
        case CODE_V:
            b |= 0x01;    // Left button
            break;
        case CODE_L:
        case CODE_X:
            b |= 0x02;    // Right button
            break;
        case CODE_COMMA:
            b |= 0x04;    // Wheel button
            break;
        case CODE_U:
        case CODE_Z:
            b |= 0x8;     // Back button
            break;
        case CODE_O:
        case CODE_B:
            b |= 0x10;    // Forward button
            break;
        case CODE_I:
        case CODE_D:
            w = 1;
            break;
        case CODE_K:
        case CODE_C:
            w = -1;
            break;
        default:
            break;
        }
    }

    // Update buttons and wheel atomically
    buttons = b;
    wheel = w;
}

#endif  // __XC8

static int8_t trimXY(uint8_t raw)
{
    int8_t sign;
    uint16_t value;
    uint8_t playXY = playTable[play];

    if (128 <= raw) {
        sign = 1;
        value = raw - 128;
    } else {
        sign = -1;
        value = 128 - raw;
    }
    if (value < (playXY >> 1) || value < PLAY_XY)
        return 0;
    if (value < playXY) {
        value = value * value / playXY * value / playXY * 10 / playXY;
        value = (tick <= value) ? 1 : 0;
    } else {
        value = (value * value / (playXY * playXY)) * value / playXY;
        if (128 <= value)
            return (0 < sign) ? 127 : -127;
    }
    return sign * value;
}

// Return 0.75 * prev + (1 - 0.75) * raw
static uint16_t lowPassFilter(uint16_t prev, uint16_t raw)
{
    prev -= (prev >> 2);
    raw >>= 2;
    return prev + raw;
}

static void processSerialData(void)
{
    x = trimXY(rawData.x);
    y = trimXY(rawData.y);

    touchSensor.current = lowPassFilter(touchSensor.current, rawData.touch);
    if (touchSensor.current < touchSensor.low)
        touchSensor.low = touchSensor.current;
    if (touchSensor.low * 7 / 6 < touchSensor.current) {
        touchSensor.thresh = (touchSensor.low + touchSensor.current) / 2;
        touchSensor.low = touchSensor.current;
    }

    if (10 < ++tick)
        tick = 0;
}

// Protocol:
// 1  tB tA t9 t8 t7 y7 x7
// 0  t6 t5 t4 t3 t2 t1 t0
// 0  x6 x5 x4 x3 x2 x1 x0
// 0  y6 y5 y4 y3 y2 y1 y0
int8_t processSerialUnit(uint8_t data)
{
    int8_t ready = 0;

    if (data & 0x80)
        rawData.count = 1;
    switch (rawData.count) {
    case 1:
        rawData.touch = ((uint16_t) (data & 0x7c)) << 5;
        rawData.y = (data & 0x02) << 6;
        rawData.x = (data & 0x01) << 7;
        ++rawData.count;
        break;
    case 2:
        rawData.touch |= data;
        ++rawData.count;
        break;
    case 3:
        rawData.x |= data;
        ++rawData.count;
        break;
    case 4:
        rawData.y |= data;
        rawData.count = 0;
        processSerialData();
        ready = 1;
        break;
    default:
        break;
    }
    return ready;
}

int8_t isMouseTouched(void)
{
    return touchSensor.current < touchSensor.thresh;
}

int8_t getKeyboardMouseX(void)
{
    return x;
}

int8_t getKeyboardMouseY(void)
{
    return y;
}

uint8_t getKeyboardMouseButtons(void)
{
    return isMouseTouched() ? buttons: 0;
}

int8_t getKeyboardMouseWheel(void)
{
    return isMouseTouched() ? wheel : 0;
}

int8_t isProcessingSrialData(void)
{
    return 1 <= rawData.count;
}

#ifdef WITH_HOS
void processMouseData(void)
{
    rawData.x = HosGetKeyboardMouseX();
    rawData.y = HosGetKeyboardMouseY();
    rawData.touch = HosGetTouch();
    processSerialData();
}
#endif

#ifdef DEBUG
void dumpMouse(void)
{
    printf("%u %u %u\n", rawData.x, rawData.y, rawData.touch);
}
#endif
