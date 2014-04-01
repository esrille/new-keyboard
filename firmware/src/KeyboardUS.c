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

static unsigned char const baseKeys[BASE_MAX + 1][5] =
{
    {KEY_U, KEY_S, KEY_ENTER},
    {KEY_U, KEY_S, KEY_MINUS, KEY_D, KEY_ENTER},
    {KEY_U, KEY_S, KEY_MINUS, KEY_C, KEY_ENTER},
    {KEY_J, KEY_P, KEY_ENTER},
    {KEY_J, KEY_P, KEY_MINUS, KEY_N, KEY_ENTER},
};

static unsigned char const matrixQwerty[8][12] =
{
    KEY_LEFT_BRACKET, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_EQUAL,
    KEY_GRAVE_ACCENT, KEY_F1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_F12, KEY_BACKSLASH,
    KEY_RIGHT_BRACKET, KEY_1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_0, KEY_MINUS,
    KEY_CAPS_LOCK, KEY_2, KEY_3, KEY_4, KEY_5, 0, 0, KEY_6, KEY_7, KEY_8, KEY_9, KEY_QUOTE,
    KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, 0, 0, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,
    KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_ESCAPE, KEY_APPLICATION, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON,
    KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_TAB, KEY_ENTER, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH,
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_FN, KEY_LEFTSHIFT, KEY_BACKSPACE, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
};

static unsigned char const matrixDvorak[8][12] =
{
    KEY_LEFT_BRACKET, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_EQUAL,
    KEY_GRAVE_ACCENT, KEY_F1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_F12, KEY_BACKSLASH,
    KEY_RIGHT_BRACKET, KEY_1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_0, KEY_SLASH,
    KEY_CAPS_LOCK, KEY_2, KEY_3, KEY_4, KEY_5, 0, 0, KEY_6, KEY_7, KEY_8, KEY_9, KEY_MINUS,
    KEY_QUOTE, KEY_COMMA, KEY_PERIOD, KEY_P, KEY_Y, 0, 0, KEY_F, KEY_G, KEY_C, KEY_R, KEY_L,
    KEY_A, KEY_O, KEY_E, KEY_U, KEY_I, KEY_ESCAPE, KEY_APPLICATION, KEY_D, KEY_H, KEY_T, KEY_N, KEY_S,
    KEY_SEMICOLON, KEY_Q, KEY_J, KEY_K, KEY_X, KEY_TAB, KEY_ENTER, KEY_B, KEY_M, KEY_W, KEY_V, KEY_Z,
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_FN, KEY_LEFTSHIFT, KEY_BACKSPACE, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
};

static unsigned char const matrixColemak[8][12] =
{
    KEY_LEFT_BRACKET, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_EQUAL,
    KEY_GRAVE_ACCENT, KEY_F1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_F12, KEY_BACKSLASH,
    KEY_RIGHT_BRACKET, KEY_1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_0, KEY_MINUS,
    KEY_BACKSPACE, KEY_2, KEY_3, KEY_4, KEY_5, 0, 0, KEY_6, KEY_7, KEY_8, KEY_9, KEY_QUOTE,
    KEY_Q, KEY_W, KEY_F, KEY_P, KEY_G, 0, 0, KEY_J, KEY_L, KEY_U, KEY_Y, KEY_SEMICOLON,
    KEY_A, KEY_R, KEY_S, KEY_T, KEY_D, KEY_ESCAPE, KEY_APPLICATION, KEY_H, KEY_N, KEY_E, KEY_I, KEY_O,
    KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_TAB, KEY_ENTER, KEY_K, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH,
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_FN, KEY_LEFTSHIFT, KEY_SPACEBAR, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
};

//
// Japanese layouts
//
// [{   KEY_RIGHT_BRACKET
// ]}   KEY_NON_US_HASH
// \|   KEY_INTERNATIONAL3
// @`   KEY_LEFT_BRACKET
// -=   KEY_MINUS
// :*   KEY_QUOTE
// ^~   KEY_EQUAL
//  _   KEY_INTERNATIONAL1
// no-convert   KEY_INTERNATIONAL5
// convert      KEY_INTERNATIONAL4
// hiragana     KEY_INTERNATIONAL2
// zenkaku      KEY_GRAVE_ACCENT
//

static unsigned char const matrixJIS[8][12] =
{
    KEY_RIGHT_BRACKET, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_EQUAL,
    KEY_INTERNATIONAL3, KEY_F1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_F12, KEY_LEFT_BRACKET,
    KEY_NON_US_HASH, KEY_1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_0, KEY_MINUS,
    KEY_CAPS_LOCK, KEY_2, KEY_3, KEY_4, KEY_5, 0, 0, KEY_6, KEY_7, KEY_8, KEY_9, KEY_QUOTE,
    KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, 0, 0, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,
    KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_ESCAPE, KEY_APPLICATION, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON,
    KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_TAB, KEY_ENTER, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH,
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_FN, KEY_LEFTSHIFT, KEY_BACKSPACE, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
};

static unsigned char const matrixNicolaF[8][12] =
{
    KEY_RIGHT_BRACKET, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_MINUS,
    KEY_INTERNATIONAL3, KEY_F1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_F12, KEY_LEFT_BRACKET,
    KEY_NON_US_HASH, KEY_1, 0, 0, 0, 0, 0, 0, 0, 0, KEY_0, KEY_QUOTE,
    KEY_EQUAL, KEY_2, KEY_3, KEY_4, KEY_5, 0, 0, KEY_6, KEY_7, KEY_8, KEY_9, KEY_BACKSPACE,
    KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, 0, 0, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,
    KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_ESCAPE, KEY_APPLICATION, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON,
    KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_TAB, KEY_ENTER, KEY_N, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_SLASH,
    KEY_LEFTCONTROL, KEY_LEFT_GUI, KEY_FN, KEY_LEFTSHIFT, KEY_INTERNATIONAL5, KEY_LEFTALT, KEY_RIGHTALT, KEY_SPACEBAR, KEY_RIGHTSHIFT, KEY_FN, KEY_RIGHT_GUI, KEY_RIGHTCONTROL
};

static unsigned char mode;

void initKeyboardBase(void)
{
    mode = eeprom_read(EEPROM_BASE);
    if (BASE_MAX < mode)
        mode = 0;
}

void emitBaseName(void)
{
    const unsigned char* message = baseKeys[mode];
    for (char i = 0; i < 5; ++i) {
        if (!message[i])
            break;
        emitKey(message[i]);
    }
}

void switchBase(void)
{
    ++mode;
    if (BASE_MAX < mode)
        mode = 0;
    eeprom_write(EEPROM_BASE, mode);
    emitBaseName();
}

char isDigit(unsigned char code)
{
    return code == 25 || code == 34 || (37 <= code && code <= 46);
}

char isJP(void)
{
    return mode == BASE_JIS || mode == BASE_NICOLA_F;
}

char processKeysBase(const unsigned char* current, const unsigned char* processed, unsigned char* report)
{
    unsigned char count = 2;
    unsigned char modifiers = current[0];
    for (char i = 2; i < 8; ++i) {
        unsigned char code = current[i];
        unsigned char key = getKeyNumLock(code);
        if (!key)
            key = getKeyBase(code);
        if (key && count < 8) {
            if (key == KEY_LANG1)
                kana_led = 1;
            else if (key == KEY_LANG2)
                kana_led = 0;
            else if (key == KEY_0 && (modifiers & MOD_SHIFT) && isJP())
                key = KEY_INTERNATIONAL1;
            report[count++] = key;
        }
    }
    report[0] = modifiers;
    return XMIT_NORMAL;
}

unsigned char getKeyBase(unsigned char code)
{
    unsigned char key = getKeyNumLock(code);
    if (key)
        return key;
    switch (mode) {
    case BASE_QWERTY:
        key = matrixQwerty[code / 12][code % 12];
        break;
    case BASE_DVORAK:
        key = matrixDvorak[code / 12][code % 12];
        break;
    case BASE_COLEMAK:
        key = matrixColemak[code / 12][code % 12];
        break;
    case BASE_JIS:
        key = matrixJIS[code / 12][code % 12];
        break;
    case BASE_NICOLA_F:
        key = matrixNicolaF[code / 12][code % 12];
        break;
    default:
        key = matrixQwerty[code / 12][code % 12];
        break;
    }
    return processModKey(key);
}
