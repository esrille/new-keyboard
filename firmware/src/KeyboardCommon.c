/*
 * Copyright 2013-2014 Esrille Inc.
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

#include "Keyboard.h"

#include <stdint.h>
#include <string.h>
#include <xc.h>

__EEPROM_DATA(BASE_QWERTY, KANA_ROMAJI, OS_PC, 5, 0, 0, 0, 0);

unsigned char os;
unsigned char kana_led;
unsigned char mod;

#define MAX_OS_KEY_NAME     5

static unsigned char const osKeys[OS_MAX + 1][MAX_OS_KEY_NAME] =
{
    {KEY_P, KEY_C, KEY_ENTER},
    {KEY_M, KEY_A, KEY_C, KEY_ENTER},
    {KEY_1, KEY_0, KEY_9, KEY_A, KEY_ENTER},
    {KEY_1, KEY_0, KEY_9, KEY_B, KEY_ENTER},
    {KEY_A, KEY_MINUS, KEY_S, KEY_P, KEY_ENTER},
    {KEY_S, KEY_MINUS, KEY_S, KEY_P, KEY_ENTER},
};

#define MAX_MOD             7
#define MAX_MOD_KEY_NAME    6
#define MAX_MOD_KEYS        8

static unsigned char const modMap[MAX_MOD + 1][MAX_MOD_KEYS] =
{
    {KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_LEFT_GUI, KEY_LEFTALT, KEY_RIGHTALT, KEY_APPLICATION, KEY_RIGHTSHIFT, KEY_RIGHTCONTROL },
    {KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_LEFTALT, KEY_LANG2, KEY_LANG1, KEY_APPLICATION, KEY_RIGHTSHIFT, KEY_RIGHTCONTROL },
    {KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_LEFTALT, KEY_LANG2, KEY_LANG1, KEY_GRAVE_ACCENT, KEY_RIGHTSHIFT, KEY_RIGHTCONTROL },
    {KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_LEFT_GUI, KEY_LANG2, KEY_LANG1, KEY_LEFTALT, KEY_RIGHTSHIFT, KEY_RIGHTCONTROL },
    {KEY_LEFTSHIFT, KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_LEFTALT, KEY_RIGHTALT, KEY_APPLICATION, KEY_RIGHTCONTROL, KEY_RIGHTSHIFT },
    {KEY_LEFTSHIFT, KEY_LEFTCONTROL, KEY_LEFTALT, KEY_LANG2, KEY_LANG1, KEY_APPLICATION, KEY_RIGHTCONTROL, KEY_RIGHTSHIFT },
    {KEY_LEFTSHIFT, KEY_LEFTCONTROL, KEY_LEFTALT, KEY_LANG2, KEY_LANG1, KEY_GRAVE_ACCENT, KEY_RIGHTCONTROL, KEY_RIGHTSHIFT },
    {KEY_LEFTSHIFT, KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_LANG2, KEY_LANG1, KEY_LEFTALT, KEY_RIGHTCONTROL, KEY_RIGHTSHIFT },
};

static unsigned char const modKeys[MAX_MOD + 1][MAX_MOD_KEY_NAME] =
{
    {KEY_C, KEY_ENTER},
    {KEY_C, KEY_J, KEY_ENTER},
    {KEY_C, KEY_J, KEY_1, KEY_0, KEY_9, KEY_ENTER},
    {KEY_C, KEY_J, KEY_M, KEY_A, KEY_C, KEY_ENTER},
    {KEY_S, KEY_ENTER},
    {KEY_S, KEY_J, KEY_ENTER},
    {KEY_S, KEY_J, KEY_1, KEY_0, KEY_9, KEY_ENTER},
    {KEY_S, KEY_J, KEY_M, KEY_A, KEY_C, KEY_ENTER},
};

static unsigned char const matrixFn[8][12][4] =
{
    {{KEY_INSERT}, {KEY_KANA}, {KEY_OS}, {KEY_DELAY}, {KEY_MOD}, {0}, {0}, {0}, {0}, {KEY_MUTE}, {KEY_VOLUME_DOWN}, {KEY_PAUSE}},
    {{KEY_LEFTCONTROL, KEY_DELETE}, {KEY_BASE}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {KEY_VOLUME_UP}, {KEY_SCROLL_LOCK}},
    {{KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_Z}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {KEY_PRINTSCREEN}},
    {{KEY_DELETE}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_LEFTARROW}, {KEY_LEFTSHIFT, KEY_UPARROW}, {KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_RIGHTARROW}, {KEYPAD_NUM_LOCK}},
    {{KEY_LEFTCONTROL, KEY_Q}, {KEY_LEFTCONTROL, KEY_W}, {KEY_PAGEUP}, {KEY_LEFTCONTROL, KEY_R}, {KEY_LEFTCONTROL, KEY_T}, {0}, {0}, {KEY_LEFTCONTROL, KEY_HOME}, {KEY_LEFTCONTROL, KEY_LEFTARROW}, {KEY_UPARROW}, {KEY_LEFTCONTROL, KEY_RIGHTARROW}, {KEY_LEFTCONTROL, KEY_END}},
    {{KEY_LEFTCONTROL, KEY_A}, {KEY_LEFTCONTROL, KEY_S}, {KEY_PAGEDOWN}, {KEY_LEFTCONTROL, KEY_F}, {KEY_LEFTCONTROL, KEY_G}, {KEY_ESCAPE}, {KEY_APPLICATION}, {KEY_HOME}, {KEY_LEFTARROW}, {KEY_DOWNARROW}, {KEY_RIGHTARROW}, {KEY_END}},
    {{KEY_LEFTCONTROL, KEY_Z}, {KEY_LEFTCONTROL, KEY_X}, {KEY_LEFTCONTROL, KEY_C}, {KEY_LEFTCONTROL, KEY_V}, {KEY_LANG2}, {KEY_TAB}, {KEY_ENTER}, {KEY_LANG1}, {KEY_LEFTSHIFT, KEY_LEFTARROW}, {KEY_LEFTSHIFT, KEY_DOWNARROW}, {KEY_LEFTSHIFT, KEY_RIGHTARROW}, {KEY_LEFTSHIFT, KEY_END}},
    {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}
};

static unsigned char const matrixNumLock[8][12] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEYPAD_MULTIPLY, 0,
    0, 0, 0, 0, 0, 0, 0, KEY_CALC, 0, KEYPAD_EQUAL, KEYPAD_DIVIDE, 0,
    0, 0, 0, 0, 0, 0, 0, 0, KEYPAD_7, KEYPAD_8, KEYPAD_9, KEYPAD_SUBTRACT,
    0, 0, 0, 0, 0, 0, 0, 0, KEYPAD_4, KEYPAD_5, KEYPAD_6, KEYPAD_ADD,
    0, 0, 0, 0, 0, 0, 0, 0, KEYPAD_1, KEYPAD_2, KEYPAD_3, KEY_ENTER,
    0, 0, 0, 0, 0, 0, 0, 0, KEYPAD_0, 0, KEYPAD_DOT, 0,
};

#define MAX_DELAY           10
#define MAX_DELAY_KEY_NAME  4

static unsigned char const delayKeyNames[11][MAX_DELAY_KEY_NAME] =
{
    {KEY_D, KEY_0, KEY_ENTER},
    {KEY_D, KEY_5, KEY_ENTER},
    {KEY_D, KEY_1, KEY_0, KEY_ENTER},
    {KEY_D, KEY_1, KEY_5, KEY_ENTER},
    {KEY_D, KEY_2, KEY_0, KEY_ENTER},
    {KEY_D, KEY_2, KEY_5, KEY_ENTER},
    {KEY_D, KEY_3, KEY_0, KEY_ENTER},
    {KEY_D, KEY_3, KEY_5, KEY_ENTER},
    {KEY_D, KEY_4, KEY_0, KEY_ENTER},
    {KEY_D, KEY_4, KEY_5, KEY_ENTER},
    {KEY_D, KEY_5, KEY_0, KEY_ENTER},
};

typedef struct Keys {
    uint8_t keys[6];
} Keys;

static unsigned char currentDelay;
static Keys keys[MAX_DELAY + 1];
static char currentKey;

static unsigned char tick;
static unsigned char holding;
static unsigned char hold[8] = {0, 0, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY};
static unsigned char processed[8] = {0, 0, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY};

static unsigned char modifiers;
static unsigned char current[8];
static signed char count = 2;
static unsigned char rowCount[8];
static unsigned char columnCount[12];

static unsigned char led;

void initKeyboard(void)
{
    os = eeprom_read(EEPROM_OS);
    if (OS_MAX < os)
        os = 0;
    mod = eeprom_read(EEPROM_MOD);
    if (MAX_MOD < mod)
        mod = 0;
    currentDelay = eeprom_read(EEPROM_DELAY);
    if (MAX_DELAY < currentDelay)
        currentDelay = 0;
    initKeyboardBase();
    initKeyboardKana();
}

unsigned char switchOS(unsigned char* report, unsigned char count)
{
    ++os;
    if (OS_MAX < os)
        os = 0;
    eeprom_write(EEPROM_OS, os);
    const unsigned char* message = osKeys[os];
    for (char i = 0; i < MAX_OS_KEY_NAME && count < 8; ++i, ++count) {
        if (!message[i])
            break;
        report[count] = message[i];
    }
    return count;
}

unsigned char switchMod(unsigned char* report, unsigned char count)
{
    ++mod;
    if (MAX_MOD < mod)
        mod = 0;
    eeprom_write(EEPROM_MOD, mod);
    const unsigned char* message = modKeys[mod];
    for (char i = 0; i < MAX_MOD_KEY_NAME && count < 8; ++i, ++count) {
        if (!message[i])
            break;
        report[count] = message[i];
    }
    return count;
}

unsigned char switchDelay(unsigned char* report, unsigned char count)
{
    ++currentDelay;
    if (MAX_DELAY < currentDelay)
        currentDelay = 0;
    eeprom_write(EEPROM_DELAY, currentDelay);
    const unsigned char* message = delayKeyNames[currentDelay];
    for (char i = 0; i < MAX_DELAY_KEY_NAME && count < 8; ++i, ++count) {
        if (!message[i])
            break;
        report[count] = message[i];
    }
    return count;
}

void onPressed(signed char row, unsigned char column)
{
    unsigned char key;
    unsigned char code = 12 * row + column;
    ++columnCount[column];
    ++rowCount[row];
    key = getKeyBase(code);
    if (KEY_LEFTCONTROL <= key && key <= KEY_RIGHT_GUI) {
        modifiers |= 1u << (key - KEY_LEFTCONTROL);
        return;
    }
    if (KEY_FN <= key && key <= KEY_RIGHT_ALTSHIFT) {
        current[1] |= 1u << (key - KEY_FN);
        return;
    }
    if (count < 8)
        current[count++] = code;
}

static char detectGhost(void)
{
    char i;
    char detected;
    unsigned char rx = 0;
    unsigned char cx = 0;

    for (i = 0; i < sizeof rowCount; ++i) {
        if (2 <= rowCount[i])
            ++rx;
    }
    for (i = 0; i < sizeof columnCount; ++i) {
        if (2 <= columnCount[i])
            ++cx;
    }
    detected = (2 <= rx && 2 <= cx);
    memset(rowCount, 0, sizeof rowCount);
    memset(columnCount, 0, sizeof columnCount);
    return detected;
}

static char processKeys(const unsigned char* current, const unsigned char* processed, unsigned char* report)
{
    char xmit;

    if (!memcmp(current, processed, 8))
        return XMIT_NONE;
    memset(report, 0, 8);
    if (current[1] & MOD_FN) {
        unsigned char modifiers = current[0];
        unsigned char count = 2;
        xmit = XMIT_NORMAL;
        for (char i = 2; i < 8; ++i) {
            unsigned char code = current[i];
            const unsigned char* a = getKeyFn(code);
            for (char j = 0; a[j] && count < 8; ++j) {
                unsigned char key = a[j];
                switch (key) {
                case 0:
                    break;
                case KEY_BASE:
                    if (!memchr(processed + 2, code, 6)) {
                        count = switchBase(report, count);
                        xmit = XMIT_IN_ORDER;
                    }
                    break;
                case KEY_KANA:
                    if (!memchr(processed + 2, code, 6)) {
                        count = switchKana(report, count);
                        xmit = XMIT_IN_ORDER;
                    }
                    break;
                case KEY_OS:
                    if (!memchr(processed + 2, code, 6)) {
                        count = switchOS(report, count);
                        xmit = XMIT_IN_ORDER;
                    }
                    break;
                case KEY_DELAY:
                    if (!memchr(processed + 2, code, 6)) {
                        count = switchDelay(report, count);
                        xmit = XMIT_IN_ORDER;
                    }
                    break;
                case KEY_MOD:
                    if (!memchr(processed + 2, code, 6)) {
                        count = switchMod(report, count);
                        xmit = XMIT_IN_ORDER;
                    }
                    break;
                case KEY_LEFTCONTROL:
                    modifiers |= MOD_LEFTCONTROL;
                    break;
                case KEY_RIGHTCONTROL:
                    modifiers |= MOD_CONTROL;
                    break;
                case KEY_LEFTSHIFT:
                    modifiers |= MOD_LEFTSHIFT;
                    break;
                case KEY_RIGHTSHIFT:
                    modifiers |= MOD_RIGHTSHIFT;
                    break;
                default:
                    if (key == KEY_LANG1)
                        kana_led = 1;
                    else if (key == KEY_LANG2)
                        kana_led = 0;
                    report[count++] = key;
                    break;
                }
            }
        }
        report[0] = modifiers;
    } else if (isKanaMode(current))
        xmit = processKeysKana(current, processed, report);
    else
        xmit = processKeysBase(current, processed, report);
    if (xmit == XMIT_NORMAL || xmit == XMIT_IN_ORDER)
        memmove(processed, current, 8);
    return xmit;
}

static void processOSMode(unsigned char* report)
{
    char i;
    switch (os) {
    case OS_PC:
        for (i = 2; i < 8; ++i) {
            switch (report[i]) {
            case KEY_LANG1:
                report[i] = KEY_F13;
                break;
            case KEY_LANG2:
                report[i] = KEY_F14;
                break;
            default:
                break;
            }
        }
        break;
    case OS_109A:
        for (i = 2; i < 8; ++i) {
            switch (report[i]) {
            case KEY_LANG1:
                report[i] = KEY_INTERNATIONAL4;
                report[0] |= MOD_LEFTSHIFT | MOD_LEFTCONTROL;
                break;
            case KEY_LANG2:
                report[i] = KEY_INTERNATIONAL5;
                report[0] |= MOD_LEFTSHIFT | MOD_LEFTCONTROL;
                break;
            default:
                break;
            }
        }
        break;
    case OS_109B:
        for (i = 2; i < 8; ++i) {
            switch (report[i]) {
            case KEY_LANG1:
            case KEY_LANG2:
                report[i] = KEY_GRAVE_ACCENT;
                break;
            default:
                break;
            }
        }
        break;
    case OS_ALT_SP:
        for (i = 2; i < 8; ++i) {
            switch (report[i]) {
            case KEY_LANG1:
            case KEY_LANG2:
                report[i] = KEY_SPACEBAR;
                report[0] |= MOD_LEFTALT;
                break;
            default:
                break;
            }
        }
        break;
    case OS_SHIFT_SP:
        for (i = 2; i < 8; ++i) {
            switch (report[i]) {
            case KEY_LANG1:
            case KEY_LANG2:
                report[i] = KEY_SPACEBAR;
                report[0] |= MOD_LEFTSHIFT;
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }
}

unsigned char processModKey(unsigned char key)
{
    const unsigned char* map = modMap[0];
    for (char i = 0; i < MAX_MOD_KEYS; ++i) {
        if (key == map[i])
            return modMap[mod][i];
    }
    return key;
}

char makeReport(unsigned char* report)
{
    char xmit = XMIT_NONE;

    if (!detectGhost()) {
        while (count < 8)
            current[count++] = VOID_KEY;

        memmove(keys[currentKey].keys, current + 2, 6);
        if (currentDelay < ++currentKey)
            currentKey = 0;

        current[0] = modifiers;
        memmove(current + 2, keys[currentKey].keys, 6);
        if (led & LED_SCROLL_LOCK)
            current[1] |= MOD_FN;

        if (memcmp(current, processed, 8)) {
            if (memcmp(current + 2, processed + 2, 6) || current[2] == VOID_KEY || current[1] || (current[0] & MOD_SHIFT))
                xmit = processKeys(current, processed, report);
            else if (processed[1] && !current[1] ||
                     (processed[0] & MOD_LEFTSHIFT) && !(current[0] & MOD_LEFTSHIFT) ||
                     (processed[0] & MOD_RIGHTSHIFT) && !(current[0] & MOD_RIGHTSHIFT))
            {
                /* empty */
            } else
                xmit = processKeys(current, processed, report);
        }
    }
    processOSMode(report);
    count = 2;
    modifiers = 0;
    current[1] = 0;
    return xmit;
}

unsigned char controlLED(unsigned char report)
{
    led = report;
    return controlKanaLED(report);
}

const unsigned char* getKeyFn(unsigned char code)
{
    return matrixFn[code / 12][code % 12];
}

unsigned char getKeyNumLock(unsigned char code)
{
    if (led & LED_NUM_LOCK)
        return matrixNumLock[code / 12][code % 12];
    return 0;
}
