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

#include <string.h>
#include <xc.h>

#define LANG_US     0
#define LANG_JP     1
#define LANG_MAX    1

__EEPROM_DATA(LANG_US, MODE_QWERTY, MODE_ROMAJI, 0, 0, 0, 0, 0);

unsigned char lang;

static unsigned char const langKeys[2][3] =
{
    {KEY_U, KEY_S, KEY_ENTER},
    {KEY_J, KEY_P, KEY_ENTER},
};

unsigned char const modifierKeys[12] =
{
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_FN, KEY_LEFTSHIFT, KEY_BACKSPACE, KEY_LEFTALT,
    KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
};

unsigned char const modifierKeyArrays[12][2] =
{
    {KEY_LEFTCONTROL}, {KEY_LEFT_GUI}, {KEY_FN}, {KEY_LEFTSHIFT}, {0}, {KEY_LEFTALT},
    {KEY_RIGHTALT}, {0}, {KEY_RIGHTSHIFT}, {KEY_FN}, {KEY_RIGHT_GUI}, {KEY_RIGHTCONTROL}
};

static unsigned char const matrixFn[7][12][4] =
{
    {{0}, {KEY_DVORAK}, {KEY_ROMAJI}, {KEY_NICOLA}, {KEY_MTYPE}, {KEY_TRON}, {KEY_TYPE}, {0}, {0}, {KEY_MUTE}, {KEY_VOLUME_UP}, {KEY_PAUSE}},
    {{KEY_INSERT}, {KEY_QWERTY}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {KEY_VOLUME_DOWN}, {KEYPAD_NUM_LOCK}},
    {{KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_Z}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {KEY_PRINTSCREEN}},
    {{KEY_DELETE}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_LEFTARROW}, {KEY_LEFTSHIFT, KEY_UPARROW}, {KEY_LEFTCONTROL, KEY_LEFTSHIFT, KEY_RIGHTARROW}, {KEY_SCROLL_LOCK}},
    {{KEY_LEFTCONTROL, KEY_Q}, {KEY_LEFTCONTROL, KEY_W}, {KEY_PAGEUP}, {KEY_LEFTCONTROL, KEY_R}, {KEY_LEFTCONTROL, KEY_T}, {0}, {0}, {KEY_LEFTCONTROL, KEY_HOME}, {KEY_LEFTCONTROL, KEY_LEFTARROW}, {KEY_UPARROW}, {KEY_LEFTCONTROL, KEY_RIGHTARROW}, {KEY_LEFTCONTROL, KEY_END}},
    {{KEY_LEFTCONTROL, KEY_A}, {KEY_LEFTCONTROL, KEY_S}, {KEY_PAGEDOWN}, {KEY_LEFTCONTROL, KEY_F}, {KEY_LEFTCONTROL, KEY_G}, {KEY_ESCAPE}, {KEY_APPLICATION}, {KEY_HOME}, {KEY_LEFTARROW}, {KEY_DOWNARROW}, {KEY_RIGHTARROW}, {KEY_END}},
    {{KEY_LEFTCONTROL, KEY_Z}, {KEY_LEFTCONTROL, KEY_X}, {KEY_LEFTCONTROL, KEY_C}, {KEY_LEFTCONTROL, KEY_V}, {KEY_F14}, {KEY_TAB}, {KEY_ENTER}, {KEY_F13}, {KEY_LEFTSHIFT, KEY_LEFTARROW}, {KEY_LEFTSHIFT, KEY_DOWNARROW}, {KEY_LEFTSHIFT, KEY_RIGHTARROW}, {KEY_LEFTSHIFT, KEY_END}},
};

static unsigned char const matrixNumLock[7][12] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEYPAD_DIVIDE, 0,
    0, 0, 0, 0, 0, 0, 0, KEY_CALC, KEYPAD_7, KEYPAD_8, KEYPAD_9, 0,
    0, 0, 0, 0, 0, 0, 0, 0, KEYPAD_4, KEYPAD_5, KEYPAD_6, KEYPAD_MULTIPLY,
    0, 0, 0, 0, 0, 0, 0, 0, KEYPAD_1, KEYPAD_2, KEYPAD_3, KEYPAD_SUBTRACT,
    0, 0, 0, 0, 0, 0, KEYPAD_ENTER, 0, KEYPAD_0, 0, KEYPAD_DOT, KEYPAD_ADD,
};

static unsigned char tick;
static unsigned char holding;
static unsigned char hold[8] = {0, 0, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY};
static unsigned char processed[8] = {0, 0, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY, VOID_KEY};

static unsigned char modifiers;
static unsigned char current[8];
static signed char count = 2;

static unsigned char led;

void initKeyboard(void)
{
    lang = eeprom_read(EEPROM_LANG);
    initKeyboardUS();
    initKeyboardJP();
}

unsigned char switchType(const unsigned char* current, unsigned char count)
{
    ++lang;
    if (LANG_MAX < lang)
        lang = 0;
    eeprom_write(EEPROM_LANG, lang);
    if (count <= 5) {
        memmove(current + count, langKeys[lang], 3);
        count += 3;
    }
    return count;
}

void onPressed(signed char row, unsigned char column)
{
    unsigned char key;

    if (row == 7) {
        key = modifierKeys[column];
        if (KEY_LEFTCONTROL <= key && key <= KEY_RIGHT_GUI) {
            modifiers |= 1u << (key - KEY_LEFTCONTROL);
            return;
        }
        if (key == KEY_FN) {
            current[1] = 1;
            return;
        }
    }
    if (count < 8)
        current[count++] = 12 * row + column;
}

static char processKeys(const unsigned char* current, const unsigned char* processed, unsigned char* report)
{
    char xmit;

    if (!memcmp(current, processed, 8))
        return XMIT_NONE;
    memset(report, 0, 8);
    switch (lang) {
    case LANG_JP:
        xmit = processKeysJP(current, processed, report);
        break;
    case LANG_US:
    default:
        xmit = processKeysUS(current, processed, report);
        break;
    }
    if (xmit == XMIT_NORMAL)
        memmove(processed, current, 8);
    return xmit;
}

char makeReport(unsigned char* report)
{
    char xmit = XMIT_NONE;

    current[0] = modifiers;
    if (led & LED_SCROLL_LOCK)
        current[1] |= 1;
    while (count < 8)
        current[count++] = VOID_KEY;

    if (!memcmp(current, hold, 8)) {
        if (10 <= ++tick) {
            xmit = processKeys(current, processed, report);
            tick = 0;
            holding = 0;
        }
    } else if (memcmp(current + 2, hold + 2, 6)) {
        if (current[1] || (current[0] & MOD_SHIFT))
            holding |= (!processed[1] || !(processed[0] & MOD_SHIFT));
        xmit = processKeys(holding ? current : hold, processed, report);
        holding = 0;
        tick = 0;
        memmove(hold, current, 8);
    } else {
        tick = 0;
        memmove(hold, current, 8);
        if (current[1] || (current[0] & MOD_SHIFT)) {
            holding = (!processed[1] || !(processed[0] & MOD_SHIFT));
            xmit = processKeys(current, processed, report);
        } else if (processed[1] && !current[1] ||
            (processed[0] & MOD_LEFTSHIFT) && !(current[0] & MOD_LEFTSHIFT) ||
            (processed[0] & MOD_RIGHTSHIFT) && !(current[0] & MOD_RIGHTSHIFT)) {
            holding = 1;
        }
    }
    count = 2;
    modifiers = 0;
    current[1] = 0;
    return xmit;
}

unsigned char controlLed(unsigned char report)
{
    led = report;
    switch (lang) {
    case LANG_JP:
        return controlLedJP(report);
    default:
        return report;
    }
}

const unsigned char* getKeyFn(unsigned char code)
{
    if (12 * 7 <= code)
        return modifierKeyArrays[code % 12];
    return matrixFn[code / 12][code % 12];
}

unsigned char getKeyNumLock(unsigned char code)
{
    if (12 * 7 <= code)
        return modifierKeys[code % 12];
    if (led & LED_NUM_LOCK)
        return matrixNumLock[code / 12][code % 12];
    return 0;
}
