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

static unsigned char const matrixQwerty[7][12] =
{
    KEY_LEFT_BRACKET, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F12, KEY_EQUAL,
    KEY_GRAVE_ACCENT, KEY_F1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_F11, KEY_BACKSLASH,
    KEY_RIGHT_BRACKET, KEY_1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_0, KEY_MINUS,
    KEY_CAPS_LOCK, KEY_2, KEY_3, KEY_4, KEY_5, 0, 0, KEY_6, KEY_7, KEY_8, KEY_9, KEY_QUOTE,
    KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, 0, 0, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,
    KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_ESCAPE, KEY_APPLICATION, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON,
    KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_TAB, KEY_ENTER, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH,
};

static unsigned char const matrixDvorak[7][12] =
{
    KEY_LEFT_BRACKET, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F12, KEY_EQUAL,
    KEY_GRAVE_ACCENT, KEY_F1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_F11, KEY_BACKSLASH,
    KEY_RIGHT_BRACKET, KEY_1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_0, KEY_SLASH,
    KEY_CAPS_LOCK, KEY_2, KEY_3, KEY_4, KEY_5, 0, 0, KEY_6, KEY_7, KEY_8, KEY_9, KEY_MINUS,
    KEY_QUOTE, KEY_COMMA, KEY_PERIOD, KEY_P, KEY_Y, 0, 0, KEY_F, KEY_G, KEY_C, KEY_R, KEY_L,
    KEY_A, KEY_O, KEY_E, KEY_U, KEY_I, KEY_ESCAPE, KEY_APPLICATION, KEY_D, KEY_H, KEY_T, KEY_N, KEY_S,
    KEY_SEMICOLON, KEY_Q, KEY_J, KEY_K, KEY_X, KEY_TAB, KEY_ENTER, KEY_B, KEY_M, KEY_W, KEY_V, KEY_Z,
};

static unsigned char mode;

void initKeyboardUS(void)
{
    mode = eeprom_read(EEPROM_MODE_US);
}

void setModeUS(unsigned char modeUS)
{
    mode = modeUS;
    eeprom_write(EEPROM_MODE_US, mode);
}

char processKeysUS(const unsigned char* current, const unsigned char* processed, unsigned char* report)
{
    unsigned char modifiers = current[0];
    unsigned char key;
    unsigned char count = 2;
    const unsigned char* a;

    if (current[1]) {
        for (char i = 2; i < 8; ++i) {
            unsigned char code = current[i];
            a = getKeyFn(code);
            for (char j = 0; a[j] && count < 8; ++j) {
                key = a[j];
                switch (key) {
                case 0:
                    break;
                case KEY_TYPE:
                    if (!memchr(processed + 2, code, 6))
                        count = switchType(report, count);
                    break;
                case KEY_QWERTY:
                    setModeUS(MODE_QWERTY);
                    break;
                case KEY_DVORAK:
                    setModeUS(MODE_DVORAK);
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
                    report[count++] = key;
                    break;
                }
            }
        }
    } else {
        for (char i = 2; i < 8; ++i) {
            unsigned char code = current[i];
            key = getKeyNumLock(code);
            if (!key)
                key = getKeyUS(code);
            if (key && count < 8)
                report[count++] = key;
        }
    }
    report[0] = modifiers;
    return XMIT_NORMAL;
}

unsigned char getKeyUS(unsigned char code)
{
    if (12 * 7 <= code)
        return modifierKeys[code % 12];
    switch (mode) {
    case MODE_DVORAK:
        return matrixDvorak[code / 12][code % 12];
    default:
        return matrixQwerty[code / 12][code % 12];
    }
}