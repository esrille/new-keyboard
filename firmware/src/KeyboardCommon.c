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

#include <system.h>

__EEPROM_DATA(BASE_QWERTY, KANA_ROMAJI, OS_PC, 5 /* delay */, 0 /* mod */, LED_DEFAULT, IME_MS, 0);

unsigned char os;
unsigned char mod;
unsigned char kana_led;
unsigned char eisuu_mode = 0;

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

static unsigned char const matrixFn[8][12][3] =
{
    {{KEY_INSERT}, {KEY_OS}, {KEY_BASE}, {KEY_KANA}, {KEY_DELAY}, {KEY_MOD}, {KEY_IME}, {KEY_LED}, {0}, {KEY_MUTE}, {KEY_VOLUME_DOWN}, {KEY_PAUSE}},
    {{KEY_LEFTCONTROL, KEY_DELETE}, {KEY_ABOUT}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {KEY_VOLUME_UP}, {KEY_SCROLL_LOCK}},
    {{KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_Z}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {KEY_PRINTSCREEN}},
    {{KEY_DELETE}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_LEFTARROW}, {KEY_LEFTSHIFT, KEY_UPARROW}, {KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_RIGHTARROW}, {KEYPAD_NUM_LOCK}},
    {{KEY_LEFTCONTROL, KEY_Q}, {KEY_LEFTCONTROL, KEY_W}, {KEY_PAGEUP}, {KEY_LEFTCONTROL, KEY_R}, {KEY_LEFTCONTROL, KEY_T}, {0}, {0}, {KEY_LEFTCONTROL, KEY_HOME}, {KEY_LEFTCONTROL, KEY_LEFTARROW}, {KEY_UPARROW}, {KEY_LEFTCONTROL, KEY_RIGHTARROW}, {KEY_LEFTCONTROL, KEY_END}},
    {{KEY_LEFTCONTROL, KEY_A}, {KEY_LEFTCONTROL, KEY_S}, {KEY_PAGEDOWN}, {KEY_LEFTCONTROL, KEY_F}, {KEY_LEFTCONTROL, KEY_G}, {KEY_ESCAPE}, {KEY_CAPS_LOCK}, {KEY_HOME}, {KEY_LEFTARROW}, {KEY_DOWNARROW}, {KEY_RIGHTARROW}, {KEY_END}},
    {{KEY_LEFTCONTROL, KEY_Z}, {KEY_LEFTCONTROL, KEY_X}, {KEY_LEFTCONTROL, KEY_C}, {KEY_LEFTCONTROL, KEY_V}, {KEY_LANG2}, {KEY_TAB}, {KEY_ENTER}, {KEY_LANG1}, {KEY_LEFTSHIFT, KEY_LEFTARROW}, {KEY_LEFTSHIFT, KEY_DOWNARROW}, {KEY_LEFTSHIFT, KEY_RIGHTARROW}, {KEY_LEFTSHIFT, KEY_END}},
    {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}}
};

static unsigned char const matrixFn109[4][3] =
{
    {KEY_INTERNATIONAL5},   // no-convert
    {KEY_INTERNATIONAL4},   // convert
    {KEY_INTERNATIONAL2},   // hiragana
    {KEY_GRAVE_ACCENT}      // zenkaku
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

static unsigned char const delayKeyNames[MAX_DELAY + 1][MAX_DELAY_KEY_NAME] =
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

static unsigned char const codeRev2[8][12] =
{
    13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 22,
    36, 0, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, 11, 47,
    84, 24, VOID_KEY, VOID_KEY, VOID_KEY, 65, 66, VOID_KEY, VOID_KEY, VOID_KEY, 35, 95,
    85, 12, VOID_KEY, VOID_KEY, VOID_KEY, 77, 78, VOID_KEY, VOID_KEY, VOID_KEY, 23, 94,
    86, 25, 37, 38, 39, 40, 43, 44, 45, 46, 34, 93,
    87, 48, 49, 50, 51, 52, 55, 56, 57, 58, 59, 92,
    88, 60, 61, 62, 63, 64, 67, 68, 69, 70, 71, 91,
    89, 72, 73, 74, 75, 76, 79, 80, 81, 82, 83, 90,
};

typedef struct Keys {
    uint8_t keys[6];
} Keys;

static unsigned char ordered_keys[128];
static unsigned char ordered_pos = 0;
static uint8_t ordered_max;

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
    memset(keys, VOID_KEY, sizeof keys);

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

void emitOSName(void)
{
    const unsigned char* message = osKeys[os];
    for (char i = 0; i < MAX_OS_KEY_NAME; ++i) {
        if (!message[i])
            break;
        emitKey(message[i]);
    }
}

void switchOS(void)
{
    ++os;
    if (OS_MAX < os)
        os = 0;
    eeprom_write(EEPROM_OS, os);
    emitOSName();
}

char is109(void)
{
    return os == OS_109A || os == OS_109B;
}

void emitModName(void)
{
    const unsigned char* message = modKeys[mod];
    for (char i = 0; i < MAX_MOD_KEY_NAME; ++i) {
        if (!message[i])
            break;
        emitKey(message[i]);
    }
}

void switchMod(void)
{
    ++mod;
    if (MAX_MOD < mod)
        mod = 0;
    eeprom_write(EEPROM_MOD, mod);
    emitModName();
}

void emitDelayName(void)
{
    const unsigned char* message = delayKeyNames[currentDelay];
    for (char i = 0; i < MAX_DELAY_KEY_NAME; ++i) {
        if (!message[i])
            break;
        emitKey(message[i]);
    }
}

void switchDelay(void)
{
    ++currentDelay;
    if (MAX_DELAY < currentDelay)
        currentDelay = 0;
    eeprom_write(EEPROM_DELAY, currentDelay);
    emitDelayName();
}

void onPressed(signed char row, unsigned char column)
{
    unsigned char key;
    unsigned char code;

    if (2 <= BOARD_REV_VALUE)
        code = codeRev2[row][column];
    else
        code = 12 * row + column;
    ++columnCount[column];
    ++rowCount[row];
    key = getKeyBase(code);
    if (KEY_LEFTCONTROL <= key && key <= KEY_RIGHT_GUI) {
        modifiers |= 1u << (key - KEY_LEFTCONTROL);
        return;
    }
    if (KEY_FN == key) {
        current[1] |= MOD_FN;
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

unsigned char beginMacro(unsigned char max)
{
    ordered_pos = 1;
    ordered_max = max;
    return ordered_keys[0];
}

unsigned char peekMacro(void)
{
    if (ordered_max <= ordered_pos)
        return 0;
    return ordered_keys[ordered_pos];
}

unsigned char getMacro(void)
{
    if (ordered_max <= ordered_pos) {
        ordered_pos = 0;
        return 0;
    }
    unsigned char key = ordered_keys[ordered_pos++];
    if (key == 0)
        ordered_pos = 0;
    return key;
}

void emitKey(unsigned char c)
{
    if (ordered_pos < sizeof ordered_keys)
        ordered_keys[ordered_pos++] = c;
    if (ordered_pos < sizeof ordered_keys)
        ordered_keys[ordered_pos] = 0;
}

static void emitString(const unsigned char s[])
{
    unsigned char i = 0;
    unsigned char c;
    for (c = s[i]; c; c = s[++i])
        emitKey(c);
}

static unsigned char getNumKeycode(unsigned int n)
{
    if (n == 0)
        return KEY_0;
    if (1 <= n && n <= 9)
        return KEY_1 - 1 + n;
    return KEY_SPACEBAR;
}

static const unsigned char about1[] = {
    KEY_E, KEY_S, KEY_R, KEY_I, KEY_L, KEY_L, KEY_E, KEY_SPACEBAR, KEY_N, KEY_E, KEY_W, KEY_SPACEBAR, KEY_K, KEY_E, KEY_Y, KEY_B, KEY_O, KEY_A, KEY_R, KEY_D, KEY_ENTER,
    KEY_R, KEY_E, KEY_V, KEY_PERIOD, KEY_SPACEBAR, 0
};
static const unsigned char about2[] = {
    KEY_V, KEY_E, KEY_R, KEY_PERIOD, KEY_SPACEBAR, 0
};
static const unsigned char about3[] = {
    KEY_C, KEY_O, KEY_P, KEY_Y, KEY_R, KEY_I, KEY_G, KEY_H, KEY_T, KEY_SPACEBAR, KEY_2, KEY_0, KEY_1, KEY_3, KEY_COMMA, KEY_SPACEBAR, KEY_2, KEY_0, KEY_1, KEY_4, KEY_SPACEBAR,
    KEY_E, KEY_S, KEY_R, KEY_I, KEY_L, KEY_L, KEY_E, KEY_SPACEBAR, KEY_I, KEY_N, KEY_C, KEY_PERIOD, KEY_ENTER,
    KEY_F, KEY_2, KEY_SPACEBAR, 0
};
static const unsigned char about4[] = {
    KEY_F, KEY_3, KEY_SPACEBAR, 0
};
static const unsigned char about5[] = {
    KEY_F, KEY_4, KEY_SPACEBAR, 0
};
static const unsigned char about6[] = {
    KEY_F, KEY_5, KEY_SPACEBAR, 0
};
static const unsigned char about7[] = {
    KEY_F, KEY_6, KEY_SPACEBAR, 0
};
static const unsigned char about8[] = {
    KEY_F, KEY_7, KEY_SPACEBAR, 0
};
static const unsigned char about9[] = {
    KEY_F, KEY_8, KEY_SPACEBAR, 0
};

static void about(void)
{
    // REV.
    emitString(about1);
    emitKey(getNumKeycode(BOARD_REV_VALUE));
    emitKey(KEY_ENTER);

    // VER.
    emitString(about2);
    emitKey(getNumKeycode((APP_VERSION_VALUE >> 8) & 0xf));
    emitKey(KEY_PERIOD);
    emitKey(getNumKeycode((APP_VERSION_VALUE >> 4) & 0xf));
    emitKey(getNumKeycode(APP_VERSION_VALUE & 0xf));
    emitKey(KEY_ENTER);

    // F2 OS
    emitString(about3);
    emitOSName();

    // F3 Layout
    emitString(about4);
    emitBaseName();

    // F4 Kana Layout
    emitString(about5);
    emitKanaName();

    // F5 Delay
    emitString(about6);
    emitDelayName();

    // F6 Modifiers
    emitString(about7);
    emitModName();

    // F7 IME
    emitString(about8);
    emitIMEName();

    // F8 LED
    emitString(about9);
    emitLEDName();
}

static const unsigned char* getKeyFn(unsigned char code)
{
    if (is109()) {
        if (12 * 6 + 8 <= code && code <= 12 * 6 + 11)
            return matrixFn109[code - (12 * 6 + 8)];
    }
    return matrixFn[code / 12][code % 12];
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
        for (char i = 2; i < 8 && xmit != XMIT_MACRO; ++i) {
            unsigned char code = current[i];
            const unsigned char* a = getKeyFn(code);
            for (char j = 0; j < 3 && count < 8; ++j) {
                unsigned char key = a[j];
                switch (key) {
                case 0:
                    break;
                case KEY_BASE:
                    if (!memchr(processed + 2, code, 6)) {
                        switchBase();
                        xmit = XMIT_MACRO;
                    }
                    break;
                case KEY_KANA:
                    if (!memchr(processed + 2, code, 6)) {
                        switchKana();
                        xmit = XMIT_MACRO;
                    }
                    break;
                case KEY_OS:
                    if (!memchr(processed + 2, code, 6)) {
                        switchOS();
                        xmit = XMIT_MACRO;
                    }
                    break;
                case KEY_DELAY:
                    if (!memchr(processed + 2, code, 6)) {
                        switchDelay();
                        xmit = XMIT_MACRO;
                    }
                    break;
                case KEY_MOD:
                    if (!memchr(processed + 2, code, 6)) {
                        switchMod();
                        xmit = XMIT_MACRO;
                    }
                    break;
                case KEY_LED:
                    if (!memchr(processed + 2, code, 6)) {
                        switchLED();
                        xmit = XMIT_MACRO;
                    }
                    break;
                case KEY_IME:
                    if (!memchr(processed + 2, code, 6)) {
                        switchIME();
                        xmit = XMIT_MACRO;
                    }
                    break;
                case KEY_ABOUT:
                    if (!memchr(processed + 2, code, 6)) {
                        about();
                        xmit = XMIT_MACRO;
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
                    else if (key == KEY_CAPS_LOCK) {
                        if (!memchr(processed + 2, key, 6) && isJP())
                            eisuu_mode ^= 1;
                    }
                    report[count++] = key;
                    break;
                }
            }
        }
        report[0] = modifiers;
    } else if (isKanaMode(current) && (!eisuu_mode || !is109()))
        xmit = processKeysKana(current, processed, report);
    else
        xmit = processKeysBase(current, processed, report);
    if (xmit == XMIT_NORMAL || xmit == XMIT_IN_ORDER || xmit == XMIT_MACRO)
        memmove(processed, current, 8);
    return xmit;
}

static void processOSMode(unsigned char* report)
{
    for (char i = 2; i < 8; ++i) {
        switch (os) {
        case OS_PC:
            switch (report[i]) {
            case KEY_LANG1:
                report[i] = KEY_F13;
                break;
            case KEY_LANG2:
                report[i] = KEY_F14;
                break;
            case KEY_INTERNATIONAL4:
            case KEY_INTERNATIONAL5:
                report[i] = KEY_SPACEBAR;
                break;
            default:
                break;
            }
            break;
        case OS_MAC:
            switch (report[i]) {
            case KEY_INTERNATIONAL4:
            case KEY_INTERNATIONAL5:
                report[i] = KEY_SPACEBAR;
                break;
            default:
                break;
            }
            break;
        case OS_109A:
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
            break;
        case OS_109B:
            switch (report[i]) {
            case KEY_LANG1:
            case KEY_LANG2:
                report[i] = KEY_GRAVE_ACCENT;
                break;
            default:
                break;
            }
            break;
        case OS_ALT_SP:
            switch (report[i]) {
            case KEY_LANG1:
            case KEY_LANG2:
                report[i] = KEY_SPACEBAR;
                report[0] |= MOD_LEFTALT;
                break;
            default:
                break;
            }
            break;
        case OS_SHIFT_SP:
            switch (report[i]) {
            case KEY_LANG1:
            case KEY_LANG2:
                report[i] = KEY_SPACEBAR;
                report[0] |= MOD_LEFTSHIFT;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
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

unsigned char getKeyNumLock(unsigned char code)
{
    if (led & LED_NUM_LOCK)
        return matrixNumLock[code / 12][code % 12];
    return 0;
}
