/*
 * Copyright 2015-2017 Esrille Inc.
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
    uint8_t x;
    uint8_t y;
} Pos;

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

#define CODE_F1         (1*1+1)
#define CODE_F9         (0*1+8)
#define CODE_F10        (0*1+9)
#define CODE_F11        (0*1+10)
#define CODE_F12        (1*12+10)
#define CODE_U          (4*12+8)
#define CODE_I          (4*12+9)
#define CODE_O          (4*12+10)
#define CODE_D          (5*12+2)
#define CODE_H          (5*12+7)
#define CODE_J          (5*12+8)
#define CODE_K          (5*12+9)
#define CODE_L          (5*12+10)
#define CODE_SEMICOLON  (5*12+11)
#define CODE_Z          (6*12+0)
#define CODE_X          (6*12+1)
#define CODE_C          (6*12+2)
#define CODE_V          (6*12+3)
#define CODE_B          (6*12+4)
#define CODE_COMMA      (6*12+9)

#define PLAY_XY     36  // x or y value smaller than PLAY_XY should be ignored.
#define THRESH_XY   48  // x or y value threshold
#define AIM_BUTTON  0x80

const static uint8_t normalTable[PAD_SENSE_MAX + 1] = {
    3, 4, 5, 6
};
const static uint8_t aimTable[PAD_SENSE_MAX + 1] = {
    2, 2, 3, 3
};

static const uint8_t about[] = {
    KEY_P, KEY_A, KEY_D, KEY_SPACEBAR, 0
};

static uint8_t resolution;
static uint8_t tick;

static uint8_t buttons;
static int8_t  x;
static int8_t  y;
static int8_t  wheel;

static SerialData rawData;
static TouchSensor touchSensor;

static Pos center;
static Pos prev;

void initMouse(void)
{
    touchSensor.current = touchSensor.thresh = 0;
    center.x = center.y = 0;
    loadMouseSettings();
}

void loadMouseSettings(void)
{
    resolution = ReadNvram(EEPROM_MOUSE);
    if (PAD_SENSE_MAX < resolution)
        resolution = 0;
}

void emitMouse(void)
{
    emitString(about);
    emitKey(KEY_1 + resolution);

#if APP_MACHINE_VALUE != 0x4550
    emitKey(KEY_SPACEBAR);
    emitNumber(touchSensor.current);
    emitKey(KEY_SLASH);
    emitNumber(touchSensor.thresh);
    emitKey(KEY_SPACEBAR);
    emitNumber(prev.x);
    emitKey(KEY_COMMA);
    emitNumber(prev.y);
#endif

    emitKey(KEY_ENTER);
}

static void setResolution(uint8_t val)
{
    if (PAD_SENSE_MAX < val)
        resolution = 0;
    else
        resolution = val;
    WriteNvram(EEPROM_MOUSE, resolution);
}

void processMouseKeys(uint8_t* current, const uint8_t* processed)
{
    uint8_t b = 0;
    int8_t w = 0;

    for (uint8_t i = 2; i < 8; ++i) {
        uint8_t code = current[i];
        switch (code) {
        case CODE_F9:
            setResolution(PAD_SENSE_4);
            break;
        case CODE_F10:
            setResolution(PAD_SENSE_3);
            break;
        case CODE_F11:
            setResolution(PAD_SENSE_2);
            break;
        case CODE_F12:
            setResolution(PAD_SENSE_1);
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
        case CODE_SEMICOLON:
            b |= AIM_BUTTON;
            break;
        default:
            break;
        }
    }

    // Update buttons and wheel atomically
    buttons = b;
    wheel = w;
}

static uint8_t distance(uint8_t raw, uint8_t center)
{
    return (raw < center) ? (center - raw) : (raw - center);
}

static int8_t trimXY(uint8_t raw, uint8_t center)
{
    int8_t sign;
    uint16_t value;
    uint8_t r;

    if (center <= raw) {
        sign = 1;
        value = raw - center;
    } else {
        sign = -1;
        value = center - raw;
    }
    if (value < THRESH_XY / 2)
        return 0;
    if (buttons & AIM_BUTTON)
        r = aimTable[resolution];
    else
        r = normalTable[resolution];
    value *= r;
    value / = 3;
    if (value < THRESH_XY) {
        value = value * value / THRESH_XY * value / THRESH_XY * 10 / THRESH_XY;
        value = (tick <= value) ? 1 : 0;
    } else {
        value = (value * value / (THRESH_XY * THRESH_XY)) * value / THRESH_XY;
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
    touchSensor.current = lowPassFilter(touchSensor.current, rawData.touch);
    if (touchSensor.current < touchSensor.low)
        touchSensor.low = touchSensor.current;
    if ((touchSensor.low * 7) / 6 < touchSensor.current) {
        touchSensor.thresh = (touchSensor.current * 6) / 7;
        touchSensor.low = touchSensor.thresh;
    }
    if (touchSensor.thresh < touchSensor.current) { // not touched?
        if ((distance(rawData.x, 128u) < PLAY_XY && distance(rawData.y, 128u) < PLAY_XY) || (center.x == 0 && center.y == 0)) {
            if (rawData.x == prev.x && rawData.y == prev.y) {
                center.x = rawData.x;
                center.y = rawData.y;
            }
        }
    }
    x = trimXY(rawData.x, center.x);
    y = trimXY(rawData.y, center.y);
    prev.x = rawData.x;
    prev.y = rawData.y;
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
    return (touchSensor.current < touchSensor.thresh) || x || y;
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
    return isMouseTouched() ? (buttons & ~AIM_BUTTON) : 0;
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
