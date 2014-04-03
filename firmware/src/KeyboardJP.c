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

#define MAX_KANA_KEY_NAME    6

static unsigned char const kanaKeys[KANA_MAX + 1][MAX_KANA_KEY_NAME] =
{
    {KEY_R, KEY_O, KEY_M, KEY_A, KEY_ENTER},
    {KEY_N, KEY_I, KEY_C, KEY_O, KEY_ENTER},
    {KEY_M, KEY_T, KEY_Y, KEY_P, KEY_E, KEY_ENTER},
    {KEY_T, KEY_R, KEY_O, KEY_N, KEY_ENTER},
    {KEY_S, KEY_T, KEY_I, KEY_C, KEY_K, KEY_ENTER},
};

#define MAX_LED_KEY_NAME    4

static unsigned char const ledKeyNames[LED_MAX + 1][MAX_LED_KEY_NAME] =
{
    {KEY_L, KEY_ENTER},
    {KEY_C, KEY_ENTER},
    {KEY_R, KEY_ENTER},
    {KEY_L, KEY_MINUS, KEY_N, KEY_ENTER},
    {KEY_C, KEY_MINUS, KEY_C, KEY_ENTER},
    {KEY_R, KEY_MINUS, KEY_S, KEY_ENTER},
    {KEY_O, KEY_F, KEY_F, KEY_ENTER},
};

static unsigned char const consonantSet[][2] =
{
    {0},
    {KEY_K},
    {KEY_S},
    {KEY_T},
    {KEY_N},
    {KEY_H},
    {KEY_M},
    {KEY_Y},
    {KEY_R},
    {KEY_W},
    {KEY_P},
    {KEY_G},
    {KEY_Z},
    {KEY_D},
    {KEY_B},
    {KEY_X},
    {KEY_X, KEY_K},
    {KEY_X, KEY_T},
    {KEY_X, KEY_Y},
    {KEY_X, KEY_W},
    {KEY_W, KEY_Y},
    {KEY_V},
    {KEY_L},
};

static unsigned char const vowelSet[] =
{
    0,
    KEY_A,
    KEY_I,
    KEY_U,
    KEY_E,
    KEY_O,
    KEY_Y
};

static unsigned char const mtypeSet[][3] =
{
    {KEY_A, KEY_N, KEY_N},
    {KEY_A, KEY_K, KEY_U},
    {KEY_A, KEY_T, KEY_U},
    {KEY_A, KEY_I},
    {KEY_I, KEY_N, KEY_N},
    {KEY_I, KEY_K, KEY_U},
    {KEY_I, KEY_T, KEY_U},
    {KEY_U, KEY_N, KEY_N},
    {KEY_U, KEY_K, KEY_U},
    {KEY_U, KEY_T, KEY_U},
    {KEY_E, KEY_N, KEY_N},
    {KEY_E, KEY_K, KEY_I},
    {KEY_E, KEY_T, KEY_U},
    {KEY_E, KEY_I},
    {KEY_O, KEY_N, KEY_N},
    {KEY_O, KEY_K, KEY_U},
    {KEY_O, KEY_T, KEY_U},
    {KEY_O, KEY_U},
    {KEY_C},
    {KEY_F},
    {KEY_J},
    {KEY_Q},
};

// ROMA_NN - KANA_CHOUON
static unsigned char const commonSet[][2] =
{
    {KEY_N, KEY_N},
    {KEY_MINUS},
    {KEY_DAKUTEN},
    {KEY_HANDAKU},
    {KEY_LEFTSHIFT, KEY_SLASH},
    {KEY_COMMA},
    {KEY_PERIOD},
    {KEY_LEFTSHIFT, KEY_COMMA},
    {KEY_LEFTSHIFT, KEY_PERIOD},

    // Stickney Next
    {KEY_LEFT_BRACKET},
    {KEY_RIGHT_BRACKET},
    {KEY_LEFTSHIFT, KEY_RIGHT_BRACKET},
    {KEY_LEFTSHIFT, KEY_NON_US_HASH},
    {KEY_QUOTE},
    {KEY_EQUAL},
    {KEY_MINUS},
    {KEY_1},
    {KEY_SLASH},
    {KEY_NON_US_HASH},
    {KEY_LEFTSHIFT, KEY_0},
    {KEY_INTERNATIONAL1},
    {KEY_LEFTSHIFT, KEY_COMMA},
    {KEY_LEFTSHIFT, KEY_PERIOD},
    {KEY_LEFTSHIFT, KEY_SLASH},
    {KEY_INTERNATIONAL3},
};

// ROMA_NAKAGURO - ROMA_NAMI
static unsigned char const nonCommonSet[][2] =
{
    {KEY_Z, KEY_SLASH},
    {KEY_SLASH},
    {KEY_Z, KEY_PERIOD},
    {KEY_RIGHT_BRACKET},
    {KEY_NON_US_HASH},
    {KEY_RIGHT_BRACKET},
    {KEY_NON_US_HASH},
    {KEY_COMMA},
    {KEY_PERIOD},
    {KEY_LEFTSHIFT, KEY_EQUAL},
};

//
// Stickney Next
//
static unsigned char const matrixStickney[7][12] =
{
    {KANA_LCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {KANA_RCB, KANA_HO, 0, 0, 0, 0, 0, 0, 0, 0, 0, KANA_KUTEN},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KANA_TOUTEN},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KANA_DAKUTEN},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, KANA_WO, 0, 0, 0, 0, 0, 0, 0, 0, KANA_CHOUON},
};

static unsigned char const matrixStickneyShift[7][12] =
{
    {KANA_LCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {KANA_RCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KANA_KUTEN},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KANA_NAKAGURO},
    {0, 0, 0, KANA_SO, 0, 0, 0, 0, 0, 0, 0, KANA_HANDAKU},
    {0, 0, KANA_SE, KANA_HE, KANA_KE, 0, 0, 0, KANA_ME, KANA_NU, KANA_RO, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, KANA_MU, 0, 0, 0},
};

//
// TRON
//
static unsigned char const matrixTron[7][12] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_RA, ROMA_RU, ROMA_KO, ROMA_HA, ROMA_XYO, 0, 0, ROMA_KI, ROMA_NO, ROMA_KU, ROMA_A, ROMA_RE},
    {ROMA_TA, ROMA_TO, ROMA_KA, ROMA_TE, ROMA_MO, 0, 0, ROMA_WO, ROMA_I, ROMA_U, ROMA_SI, ROMA_NN},
    {ROMA_MA, ROMA_RI, ROMA_NI, ROMA_SA, ROMA_NA, 0, 0, ROMA_SU, ROMA_TU, ROMA_TOUTEN, ROMA_KUTEN, ROMA_XTU},
};

static unsigned char const matrixTronLeft[7][12] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_SANTEN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_HI, ROMA_SO, ROMA_NAKAGURO, ROMA_XYA, ROMA_HO, 0, 0, ROMA_GI, ROMA_GE, ROMA_GU, ROMA_QUESTION, ROMA_WYI},
    {ROMA_NU, ROMA_NE, ROMA_XYU, ROMA_YO, ROMA_HU, 0, 0, ROMA_DAKUTEN, ROMA_DI, ROMA_VU, ROMA_ZI, ROMA_WYE},
    {ROMA_XE, ROMA_XO, ROMA_SE, ROMA_YU, ROMA_HE, 0, 0, ROMA_ZU, ROMA_DU, ROMA_COMMA, ROMA_PERIOD, ROMA_XWA},
};

static unsigned char const matrixTronRight[7][12] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_BI, ROMA_ZO, ROMA_GO, ROMA_BA, ROMA_BO, 0, 0, ROMA_E, ROMA_KE, ROMA_ME, ROMA_MU, ROMA_RO},
    {ROMA_DA, ROMA_DO, ROMA_GA, ROMA_DE, ROMA_BU, 0, 0, ROMA_O, ROMA_TI, ROMA_CHOUON, ROMA_MI, ROMA_YA},
    {ROMA_XKA, ROMA_XKE, ROMA_ZE, ROMA_ZA, ROMA_BE, 0, 0, ROMA_WA, ROMA_XI, ROMA_XA, ROMA_HANDAKU, ROMA_XU},
};

//
// Nicola
//
static unsigned char const matrixNicola[7][12] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ROMA_DAKUTEN},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ROMA_TOUTEN},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_KUTEN, ROMA_KA, ROMA_TA, ROMA_KO, ROMA_SA, 0, 0, ROMA_RA, ROMA_TI, ROMA_KU, ROMA_TU, ROMA_TOUTEN},
    {ROMA_U, ROMA_SI, ROMA_TE, ROMA_KE, ROMA_SE, 0, 0, ROMA_HA, ROMA_TO, ROMA_KI, ROMA_I, ROMA_NN},
    {ROMA_KUTEN, ROMA_HI, ROMA_SU, ROMA_HU, ROMA_HE, 0, 0, ROMA_ME, ROMA_SO, ROMA_NE, ROMA_HO, ROMA_NAKAGURO},
};

static unsigned char const matrixNicolaLeft[7][12] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ROMA_DAKUTEN},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, ROMA_QUESTION, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, ROMA_SLASH, ROMA_NAMI, ROMA_LCB, ROMA_RCB, 0, 0, ROMA_LSB, ROMA_RSB, 0, 0, 0},
    {ROMA_XA, ROMA_E, ROMA_RI, ROMA_XYA, ROMA_RE, 0, 0, ROMA_PA, ROMA_DI, ROMA_GU, ROMA_DU, ROMA_PI},
    {ROMA_WO, ROMA_A, ROMA_NA, ROMA_XYU, ROMA_MO, 0, 0, ROMA_BA, ROMA_DO, ROMA_GI, ROMA_PO, ROMA_NN},
    {ROMA_XU, ROMA_CHOUON, ROMA_RO, ROMA_YA, ROMA_XI, 0, 0, ROMA_PU, ROMA_ZO, ROMA_PE, ROMA_BO, ROMA_NAKAGURO},
};

static unsigned char const matrixNicolaRight[7][12] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ROMA_HANDAKU},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, ROMA_QUESTION, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, ROMA_SLASH, ROMA_NAMI, ROMA_LCB, ROMA_RCB, 0, 0, ROMA_LSB, ROMA_RSB, 0, 0, 0},
    {ROMA_KUTEN, ROMA_GA, ROMA_DA, ROMA_GO, ROMA_ZA, 0, 0, ROMA_YO, ROMA_NI, ROMA_RU, ROMA_MA, ROMA_XE},
    {ROMA_VU, ROMA_ZI, ROMA_DE, ROMA_GE, ROMA_ZE, 0, 0, ROMA_MI, ROMA_O, ROMA_NO, ROMA_XYO, ROMA_XTU},
    {ROMA_KUTEN, ROMA_BI, ROMA_ZU, ROMA_BU, ROMA_BE, 0, 0, ROMA_NU, ROMA_YU, ROMA_MU, ROMA_WA, ROMA_XO},
};

//
// M type
//
static unsigned char const matrixMtype[7][12] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_Q, ROMA_L, ROMA_J, ROMA_F, ROMA_C, 0, 0, ROMA_M, ROMA_Y, ROMA_R, ROMA_W, ROMA_P},
    {ROMA_E, ROMA_U, ROMA_I, ROMA_A, ROMA_O, 0, 0, ROMA_K, ROMA_S, ROMA_T, ROMA_N, ROMA_H},
    {ROMA_EI, ROMA_X, ROMA_V, ROMA_AI, ROMA_OU, 0, 0, ROMA_G, ROMA_Z, ROMA_D, ROMA_TOUTEN, ROMA_B},
};

static unsigned char const matrixMtypeShift[7][12] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_EKI, ROMA_UKU, ROMA_IKU, ROMA_AKU, ROMA_OKU, 0, 0, ROMA_MY, ROMA_XTU, ROMA_RY, ROMA_NN, ROMA_PY},
    {ROMA_ENN, ROMA_UNN, ROMA_INN, ROMA_ANN, ROMA_ONN, 0, 0, ROMA_KY, ROMA_SY, ROMA_TY, ROMA_NY, ROMA_HY},
    {ROMA_ETU, ROMA_UTU, ROMA_ITU, ROMA_ATU, ROMA_OTU, 0, 0, ROMA_GY, ROMA_ZY, ROMA_DY, ROMA_KUTEN, ROMA_BY},
};

static unsigned char const dakuonFrom[] = { KEY_K, KEY_S, KEY_T, KEY_H };
static unsigned char const dakuonTo[] = { KEY_G, KEY_Z, KEY_D, KEY_B };

static unsigned char mode;
static unsigned char led;

static unsigned char sent[3];
static unsigned char last[3];
static unsigned char lastMod;

void initKeyboardKana(void)
{
    mode = eeprom_read(EEPROM_KANA);
    if (KANA_MAX < mode)
        mode = 0;

    led = eeprom_read(EEPROM_LED);
    if (LED_MAX < led)
        led = LED_DEFAULT;
}

void emitLEDName(void)
{
    const unsigned char* message = ledKeyNames[led];
    for (char i = 0; i < MAX_LED_KEY_NAME; ++i) {
        if (!message[i])
            break;
        emitKey(message[i]);
    }
}

void switchLED(void)
{
    ++led;
    if (LED_MAX < led)
        led = 0;
    eeprom_write(EEPROM_LED, led);
    emitLEDName();
}

void emitKanaName(void)
{
    const unsigned char* message = kanaKeys[mode];
    for (char i = 0; i < MAX_KANA_KEY_NAME; ++i) {
        if (!message[i])
            break;
        emitKey(message[i]);
    }
}

void switchKana(void)
{
    ++mode;
    if (KANA_MAX < mode)
        mode = 0;
    eeprom_write(EEPROM_KANA, mode);
    emitKanaName();
}

static void processRomaji(unsigned char roma, unsigned char a[])
{
    unsigned char const* c;
    char i;

    if (roma < ROMA_ANN) {
        c = consonantSet[roma / 7];
        for (i = 0; i < 2 && c[i]; ++i)
            a[i] = c[i];
        a[i++] = vowelSet[roma % 7];
        while (i < 3)
            a[i++] = 0;
        return;
    }
    if (ROMA_ANN <= roma && roma <= ROMA_Q) {
        memcpy(a, mtypeSet[roma - ROMA_ANN], 3);
        return;
    }
    if (ROMA_NN <= roma && roma <= KANA_CHOUON) {
        memcpy(a, commonSet[roma - ROMA_NN], 2);
        a[2] = 0;
        return;
    }
    if (ROMA_NAKAGURO <= roma && roma <= ROMA_NAMI) {
        memcpy(a, nonCommonSet[roma - ROMA_NAKAGURO], 2);
        a[2] = 0;
        return;
    }
    memset(a, 0, 3);
}

static char processKana(const unsigned char* current, const unsigned char* processed, unsigned char* report,
                        const unsigned char base[][12], const unsigned char left[][12], const unsigned char right[][12])
{
    unsigned char mod = current[0];
    unsigned char modifiers;
    unsigned char key;
    unsigned char count = 2;
    unsigned char roma;
    unsigned char a[3];
    const unsigned char* dakuon;
    char xmit = XMIT_NORMAL;

    modifiers = current[0] & ~MOD_SHIFT;
    report[0] = modifiers;
    for (char i = 2; i < 8 && count < 8; ++i) {
        unsigned char code = current[i];

        key = getKeyNumLock(code);
        if (key) {
            report[count++] = key;
            memset(last, 0, 3);
            lastMod = current[0];
            modifiers = current[0];
            continue;
        }

        unsigned char no_repeat = 12 * 4 <= code && code < 12 * 7  && code % 12 != 5 && code % 12 != 6;
        for (char j = 2; j < 8; ++j) {
            if (no_repeat && code == processed[j]) {
                code = VOID_KEY;
                break;
            }
        }
        if ((mod & MOD_SHIFT) == MOD_SHIFT) {
            if (lastMod & MOD_LEFTSHIFT)
                mod &= ~MOD_LEFTSHIFT;
            else if (processed[0] & MOD_RIGHTSHIFT)
                mod &= ~MOD_RIGHTSHIFT;
        }

        if (7 <= code / 12)
            roma = 0;
        else if (mod & MOD_LEFTSHIFT)
            roma = left[code / 12][code % 12];
        else if (mod & MOD_RIGHTSHIFT)
            roma = right[code / 12][code % 12];
        else
            roma = base[code / 12][code % 12];
        if (mode == KANA_NICOLA && !isJP() && isDigit(code))
            roma = base[code / 12][code % 12];
        if (roma)
            processRomaji(roma, a);
        if (!roma || !a[0]) {
            key = getKeyBase(code);
            if (key) {
                if (key == KEY_LANG1)
                    kana_led = 1;
                else if (key == KEY_LANG2)
                    kana_led = 0;
                else if (key == KEY_0 && (mod & MOD_SHIFT) && isJP())
                    key = KEY_INTERNATIONAL1;
                report[count++] = key;
                memset(last, 0, 3);
                lastMod = current[0];
                modifiers = current[0];
            }
            continue;
        }
        if (no_repeat) {
            for (char i = 0; i < 3 && sent[i]; ++i) {
                for (char j = 0; j < 3 && a[j]; ++j) {
                    if (sent[i] == a[j]) {
                        memset(sent, 0, 3);
                        return XMIT_BRK;
                    }
                }
            }
        }
        xmit = XMIT_IN_ORDER;
        for (char i = 0; i < 3 && a[i] && count < 8; ++i) {
            key = a[i];
            switch (key) {
            case KEY_DAKUTEN:
                if (last[0]) {
                    dakuon = memchr(dakuonFrom, last[0], 4);
                    if (dakuon && count <= 5) {
                        report[count++] = KEY_BACKSPACE;
                        report[count++] = dakuonTo[dakuon - dakuonFrom];
                        report[count++] = last[1];
                    }
                }
                break;
            case KEY_HANDAKU:
                if (last[0] == KEY_H) {
                    if (count <= 5) {
                        report[count++] = KEY_BACKSPACE;
                        report[count++] = KEY_P;
                        report[count++] = last[1];
                    }
                }
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
        memcpy(last, a, 3);
        lastMod = current[0];
    }
    if (2 < count) {
        memcpy(sent, last, 3);
        report[0] = modifiers;
    } else {
        memset(sent, 0, 3);
        report[0] = current[0];
    }
    return xmit;
}

char isKanaMode(const unsigned char* current)
{
    return kana_led && !(current[0] & (MOD_ALT | MOD_CONTROL | MOD_GUI)) && mode != KANA_ROMAJI;
}

char processKeysKana(const unsigned char* current, const unsigned char* processed, unsigned char* report)
{
    switch (mode) {
    case KANA_TRON:
        return processKana(current, processed, report, matrixTron, matrixTronLeft, matrixTronRight);
    case KANA_NICOLA:
        return processKana(current, processed, report, matrixNicola, matrixNicolaLeft, matrixNicolaRight);
    case KANA_MTYPE:
        return processKana(current, processed, report, matrixMtype, matrixMtypeShift, matrixMtypeShift);
    case KANA_STICKNEY:
        return processKana(current, processed, report, matrixStickney, matrixStickneyShift, matrixStickneyShift);
    default:
        return processKeysBase(current, processed, report);
    }
}

unsigned char controlKanaLED(unsigned char report)
{
    if (mode != KANA_ROMAJI) {
        switch (led) {
        case LED_LEFT:
        case LED_CENTER:
        case LED_RIGHT:
            report &= ~(1u << led);
            report |= (kana_led << led);
            break;
        case LED_LEFT_NUM:
        case LED_CENTER_CAPS:
        case LED_RIGHT_SCROLL:
            report |= (kana_led << (led - LED_LEFT_NUM));
            break;
        default:
            break;
        }
    }
    return report;
}
