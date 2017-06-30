/*
 * Copyright 2013-2017 Esrille Inc.
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
#include <system.h>

#define MAX_KANA_KEY_NAME    6

static uint8_t const kanaKeys[KANA_MAX + 1][MAX_KANA_KEY_NAME] =
{
    {KEY_R, KEY_O, KEY_M, KEY_A, KEY_ENTER},
    {KEY_N, KEY_I, KEY_C, KEY_O, KEY_ENTER},
    {KEY_M, KEY_T, KEY_Y, KEY_P, KEY_E, KEY_ENTER},
    {KEY_T, KEY_R, KEY_O, KEY_N, KEY_ENTER},
    {KEY_S, KEY_T, KEY_I, KEY_C, KEY_K, KEY_ENTER},
    {KEY_X, KEY_6, KEY_0, KEY_0, KEY_4, KEY_ENTER},
};

#define MAX_LED_KEY_NAME    4

static uint8_t const ledKeyNames[LED_MAX + 1][MAX_LED_KEY_NAME] =
{
    {KEY_L, KEY_ENTER},
    {KEY_C, KEY_ENTER},
    {KEY_R, KEY_ENTER},
    {KEY_L, KEY_MINUS, KEY_N, KEY_ENTER},
    {KEY_C, KEY_MINUS, KEY_C, KEY_ENTER},
    {KEY_R, KEY_MINUS, KEY_S, KEY_ENTER},
    {KEY_O, KEY_F, KEY_F, KEY_ENTER},
};

#define MAX_IME_KEY_NAME     5

static uint8_t const imeKeyNames[IME_MAX + 1][MAX_IME_KEY_NAME] =
{
    {KEY_M, KEY_S, KEY_ENTER},
    {KEY_A, KEY_T, KEY_O, KEY_K, KEY_ENTER},
    {KEY_G, KEY_O, KEY_O, KEY_G, KEY_ENTER},
    {KEY_A, KEY_P, KEY_P, KEY_L, KEY_ENTER},
};

static uint8_t const consonantSet[][2] =
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

static uint8_t const vowelSet[] =
{
    0,
    KEY_A,
    KEY_I,
    KEY_U,
    KEY_E,
    KEY_O,
    KEY_Y
};

#ifndef DISABLE_MTYPE
static uint8_t const mtypeSet[][3] =
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
#endif

// ROMA_NN - ROMA_BANG
static uint8_t const commonSet[][2] =
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

    // Extra
    {KEY_LEFTSHIFT, KEY_1},
};

//
// ROMA_LCB - ROMA_NAMI
//
static uint8_t const msSet[][3] =
{
    {KEY_LEFT_BRACKET},
    {KEY_RIGHT_BRACKET},
    {KEY_LEFT_BRACKET},
    {KEY_RIGHT_BRACKET},
    {KEY_LEFT_BRACKET},
    {KEY_RIGHT_BRACKET},
    {KEY_SLASH},
    {KEY_SLASH},
    {KEY_SLASH, KEY_SLASH, KEY_SLASH},
    {KEY_COMMA},
    {KEY_PERIOD},
    {KEY_LEFTSHIFT, KEY_GRAVE_ACCENT},
};

static uint8_t const googleSet[][3] =
{
    {KEY_LEFT_BRACKET},
    {KEY_RIGHT_BRACKET},
    {KEY_Z, KEY_LEFT_BRACKET},
    {KEY_Z, KEY_RIGHT_BRACKET},
    {KEY_LEFT_BRACKET},
    {KEY_RIGHT_BRACKET},
    {KEY_Z, KEY_SLASH},
    {KEY_SLASH},
    {KEY_Z, KEY_PERIOD},
    {KEY_COMMA},
    {KEY_PERIOD},
    {KEY_LEFTSHIFT, KEY_GRAVE_ACCENT},
};

static uint8_t const atokSet[][3] =
{
    {KEY_LEFT_BRACKET},
    {KEY_RIGHT_BRACKET},
    {KEY_LEFT_BRACKET},
    {KEY_RIGHT_BRACKET},
    {KEY_LEFT_BRACKET},
    {KEY_RIGHT_BRACKET},
    {KEY_SLASH},
    {KEY_SLASH},
    {KEY_SLASH, KEY_SLASH, KEY_SLASH},
    {KEY_COMMA},
    {KEY_PERIOD},
    {KEY_LEFTSHIFT, KEY_GRAVE_ACCENT},
};

static uint8_t const appleSet[][3] =
{
    {KEY_LEFT_BRACKET},
    {KEY_RIGHT_BRACKET},
    {KEY_LEFTSHIFT, KEY_LEFT_BRACKET},
    {KEY_LEFTSHIFT, KEY_RIGHT_BRACKET},
    {KEY_LEFTALT, KEY_LEFTSHIFT, KEY_9},
    {KEY_LEFTALT, KEY_LEFTSHIFT, KEY_0},
    {KEY_SLASH},
    {KEY_SLASH},
    {KEY_SLASH, KEY_SLASH, KEY_SLASH},
    {KEY_COMMA},
    {KEY_PERIOD},
    {KEY_LEFTSHIFT, KEY_GRAVE_ACCENT},
};

#ifndef DISABLE_STICKNEY
//
// Stickney Next
//
static uint8_t const matrixStickney[7][12] =
{
    {KANA_LCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {KANA_RCB, KANA_HO, 0, 0, 0, 0, 0, 0, 0, 0, 0, KANA_KUTEN},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KANA_TOUTEN},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KANA_DAKUTEN},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, KANA_WO, 0, 0, 0, 0, 0, 0, 0, 0, KANA_CHOUON},
};

static uint8_t const matrixStickneyShift[7][12] =
{
    {KANA_LCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {KANA_RCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KANA_KUTEN},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KANA_NAKAGURO},
    {0, 0, 0, KANA_SO, 0, 0, 0, 0, 0, 0, 0, KANA_HANDAKU},
    {0, 0, KANA_SE, KANA_HE, KANA_KE, 0, 0, 0, KANA_ME, KANA_NU, KANA_RO, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, KANA_MU, 0, 0, 0},
};
#endif

//
// TRON
//
static uint8_t const matrixTron[7][12] =
{
    {ROMA_LCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_RCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_RA, ROMA_RU, ROMA_KO, ROMA_HA, ROMA_XYO, 0, 0, ROMA_KI, ROMA_NO, ROMA_KU, ROMA_A, ROMA_RE},
    {ROMA_TA, ROMA_TO, ROMA_KA, ROMA_TE, ROMA_MO, 0, 0, ROMA_WO, ROMA_I, ROMA_U, ROMA_SI, ROMA_NN},
    {ROMA_MA, ROMA_RI, ROMA_NI, ROMA_SA, ROMA_NA, 0, 0, ROMA_SU, ROMA_TU, ROMA_TOUTEN, ROMA_KUTEN, ROMA_XTU},
};

static uint8_t const matrixTronLeft[7][12] =
{
    {ROMA_LCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_SANTEN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_RCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_HI, ROMA_SO, ROMA_NAKAGURO, ROMA_XYA, ROMA_HO, 0, 0, ROMA_GI, ROMA_GE, ROMA_GU, ROMA_QUESTION, ROMA_WYI},
    {ROMA_NU, ROMA_NE, ROMA_XYU, ROMA_YO, ROMA_HU, 0, 0, ROMA_DAKUTEN, ROMA_DI, ROMA_VU, ROMA_ZI, ROMA_WYE},
    {ROMA_XE, ROMA_XO, ROMA_SE, ROMA_YU, ROMA_HE, 0, 0, ROMA_ZU, ROMA_DU, ROMA_COMMA, ROMA_PERIOD, ROMA_XWA},
};

static uint8_t const matrixTronRight[7][12] =
{
    {ROMA_LWCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_RWCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_BI, ROMA_ZO, ROMA_GO, ROMA_BA, ROMA_BO, 0, 0, ROMA_E, ROMA_KE, ROMA_ME, ROMA_MU, ROMA_RO},
    {ROMA_DA, ROMA_DO, ROMA_GA, ROMA_DE, ROMA_BU, 0, 0, ROMA_O, ROMA_TI, ROMA_CHOUON, ROMA_MI, ROMA_YA},
    {ROMA_XKA, ROMA_XKE, ROMA_ZE, ROMA_ZA, ROMA_BE, 0, 0, ROMA_WA, ROMA_XI, ROMA_XA, ROMA_HANDAKU, ROMA_XU},
};

//
// Nicola
//
static uint8_t const matrixNicola[7][12] =
{
    {ROMA_LCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ROMA_DAKUTEN},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_RCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ROMA_TOUTEN},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_KUTEN, ROMA_KA, ROMA_TA, ROMA_KO, ROMA_SA, 0, 0, ROMA_RA, ROMA_TI, ROMA_KU, ROMA_TU, ROMA_TOUTEN},
    {ROMA_U, ROMA_SI, ROMA_TE, ROMA_KE, ROMA_SE, 0, 0, ROMA_HA, ROMA_TO, ROMA_KI, ROMA_I, ROMA_NN},
    {ROMA_KUTEN, ROMA_HI, ROMA_SU, ROMA_HU, ROMA_HE, 0, 0, ROMA_ME, ROMA_SO, ROMA_NE, ROMA_HO, ROMA_NAKAGURO},
};

static uint8_t const matrixNicolaLeft[7][12] =
{
    {ROMA_LCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ROMA_DAKUTEN},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_RCB, ROMA_QUESTION, 0, 0, 0, 0, 0, 0, 0, 0, 0, ROMA_TOUTEN},
    {0, ROMA_SLASH, ROMA_NAMI, ROMA_LCB, ROMA_RCB, 0, 0, ROMA_LSB, ROMA_RSB, 0, 0, 0},
    {ROMA_XA, ROMA_E, ROMA_RI, ROMA_XYA, ROMA_RE, 0, 0, ROMA_PA, ROMA_DI, ROMA_GU, ROMA_DU, ROMA_PI},
    {ROMA_WO, ROMA_A, ROMA_NA, ROMA_XYU, ROMA_MO, 0, 0, ROMA_BA, ROMA_DO, ROMA_GI, ROMA_PO, ROMA_NN},
    {ROMA_XU, ROMA_CHOUON, ROMA_RO, ROMA_YA, ROMA_XI, 0, 0, ROMA_PU, ROMA_ZO, ROMA_PE, ROMA_BO, ROMA_NAKAGURO},
};

static uint8_t const matrixNicolaRight[7][12] =
{
    {ROMA_LWCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ROMA_HANDAKU},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_RWCB, ROMA_QUESTION, 0, 0, 0, 0, 0, 0, 0, 0, 0, ROMA_TOUTEN},
    {0, ROMA_SLASH, ROMA_NAMI, ROMA_LCB, ROMA_RCB, 0, 0, ROMA_LSB, ROMA_RSB, 0, 0, 0},
    {ROMA_KUTEN, ROMA_GA, ROMA_DA, ROMA_GO, ROMA_ZA, 0, 0, ROMA_YO, ROMA_NI, ROMA_RU, ROMA_MA, ROMA_XE},
    {ROMA_VU, ROMA_ZI, ROMA_DE, ROMA_GE, ROMA_ZE, 0, 0, ROMA_MI, ROMA_O, ROMA_NO, ROMA_XYO, ROMA_XTU},
    {ROMA_KUTEN, ROMA_BI, ROMA_ZU, ROMA_BU, ROMA_BE, 0, 0, ROMA_NU, ROMA_YU, ROMA_MU, ROMA_WA, ROMA_XO},
};

#ifndef DISABLE_MTYPE
//
// M type
//
static uint8_t const matrixMtype[7][12] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_Q, ROMA_L, ROMA_J, ROMA_F, ROMA_C, 0, 0, ROMA_M, ROMA_Y, ROMA_R, ROMA_W, ROMA_P},
    {ROMA_E, ROMA_U, ROMA_I, ROMA_A, ROMA_O, 0, 0, ROMA_K, ROMA_S, ROMA_T, ROMA_N, ROMA_H},
    {ROMA_EI, ROMA_X, ROMA_V, ROMA_AI, ROMA_OU, 0, 0, ROMA_G, ROMA_Z, ROMA_D, ROMA_TOUTEN, ROMA_B},
};

static uint8_t const matrixMtypeShift[7][12] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_EKI, ROMA_UKU, ROMA_IKU, ROMA_AKU, ROMA_OKU, 0, 0, ROMA_MY, ROMA_XTU, ROMA_RY, ROMA_NN, ROMA_PY},
    {ROMA_ENN, ROMA_UNN, ROMA_INN, ROMA_ANN, ROMA_ONN, 0, 0, ROMA_KY, ROMA_SY, ROMA_TY, ROMA_NY, ROMA_HY},
    {ROMA_ETU, ROMA_UTU, ROMA_ITU, ROMA_ATU, ROMA_OTU, 0, 0, ROMA_GY, ROMA_ZY, ROMA_DY, ROMA_KUTEN, ROMA_BY},
};
#endif

//
// JIS X 6004
//
static uint8_t const matrixX6004[7][12] =
{
    {ROMA_LCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_RCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ROMA_TI},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ROMA_NA},
    {ROMA_SO, ROMA_KE, ROMA_SE, ROMA_TE, ROMA_XYO, 0, 0, ROMA_TU, ROMA_NN, ROMA_NO, ROMA_WO, ROMA_RI},
    {ROMA_HA, ROMA_KA, ROMA_SI, ROMA_TO, ROMA_TA, 0, 0, ROMA_KU, ROMA_U, ROMA_I, ROMA_DAKUTEN, ROMA_KI},
    {ROMA_SU, ROMA_KO, ROMA_NI, ROMA_SA, ROMA_A, 0, 0, ROMA_XTU, ROMA_RU, ROMA_TOUTEN, ROMA_KUTEN, ROMA_RE},
};

static uint8_t const matrixX6004Shift[7][12] =
{
    {ROMA_LWCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {ROMA_RWCB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ROMA_LCB},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ROMA_RCB},
    {ROMA_XA, ROMA_HANDAKU, ROMA_HO, ROMA_HU, ROMA_ME, 0, 0, ROMA_HI, ROMA_E, ROMA_MI, ROMA_YA, ROMA_NU},
    {ROMA_XI, ROMA_HE, ROMA_RA, ROMA_XYU, ROMA_YO, 0, 0, ROMA_MA, ROMA_O, ROMA_MO, ROMA_WA, ROMA_YU},
    {ROMA_XU, ROMA_XE, ROMA_XO, ROMA_NE, ROMA_XYA, 0, 0, ROMA_MU, ROMA_RO, ROMA_NAKAGURO, ROMA_CHOUON, ROMA_QUESTION},
};

static uint8_t const dakuonFrom[] = { KEY_K, KEY_S, KEY_T, KEY_H };
static uint8_t const dakuonTo[] = { KEY_G, KEY_Z, KEY_D, KEY_B };

static uint8_t mode;
static uint8_t led;
static uint8_t ime;
static uint8_t kana_led;
static uint8_t eisuu_mode;

static uint8_t sent[3];
static uint8_t last[3];
static uint8_t lastMod;

void loadKanaSettings(void)
{
    mode = ReadNvram(EEPROM_KANA);
    if (KANA_MAX < mode)
        mode = 0;

    led = ReadNvram(EEPROM_LED);
    if (LED_MAX < led)
        led = LED_DEFAULT;

    ime = ReadNvram(EEPROM_IME);
    if (IME_MAX < ime)
        ime = 0;
}

void emitLEDName(void)
{
    emitStringN(ledKeyNames[led], MAX_LED_KEY_NAME);
}

void switchLED(void)
{
    ++led;
    if (LED_MAX < led)
        led = 0;
    WriteNvram(EEPROM_LED, led);
    emitLEDName();
}

void emitKanaName(void)
{
    emitStringN(kanaKeys[mode], MAX_KANA_KEY_NAME);
}

void switchKana(void)
{
    ++mode;
#ifdef DISABLE_MTYPE
    if (mode == KANA_MTYPE)
        ++mode;
#endif
#ifdef DISABLE_STICKNEY
    if (mode == KANA_STICKNEY)
        ++mode;
#endif
    if (KANA_MAX < mode)
        mode = 0;
    WriteNvram(EEPROM_KANA, mode);
    emitKanaName();
}

void emitIMEName(void)
{
    emitStringN(imeKeyNames[ime], MAX_IME_KEY_NAME);
}

void switchIME(void)
{
    ++ime;
    if (IME_MAX < ime)
        ime = 0;
    WriteNvram(EEPROM_IME, ime);
    emitIMEName();
}

static void processRomaji(uint8_t roma, uint8_t a[])
{
    uint8_t const* c;
    uint8_t i;

    if (roma < ROMA_ANN) {
        c = consonantSet[roma / 7];
        for (i = 0; i < 2 && c[i]; ++i)
            a[i] = c[i];
        a[i++] = vowelSet[roma % 7];
        while (i < 3)
            a[i++] = 0;
        return;
    }
#ifndef DISABLE_MTYPE
    if (ROMA_ANN <= roma && roma <= ROMA_Q) {
        memcpy(a, mtypeSet[roma - ROMA_ANN], 3);
        return;
    }
#endif
    if (ROMA_NN <= roma && roma <= ROMA_BANG) {
        memcpy(a, commonSet[roma - ROMA_NN], 2);
        a[2] = 0;
        return;
    }
    if (ROMA_LCB <= roma && roma <= ROMA_NAMI) {
        i = roma - ROMA_LCB;
        switch (ime) {
        case IME_GOOGLE:
            c = googleSet[i];
            break;
        case IME_APPLE:
            c = appleSet[i];
            break;
        case IME_ATOK:
            c = atokSet[i];
            break;
        case IME_MS:
        default:
            c = msSet[i];
            break;
        }
        for (i = 0; i < 3; ++i) {
            uint8_t key = c[i];
            if (isJP()) {
                switch (key) {
                case KEY_LEFT_BRACKET:
                    key = KEY_RIGHT_BRACKET;
                    break;
                case KEY_RIGHT_BRACKET:
                    key = KEY_NON_US_HASH;
                    break;
                case KEY_GRAVE_ACCENT:
                    key = KEY_EQUAL;
                    break;
                case KEY_9:
                    key = KEY_8;
                    break;
                case KEY_0:
                    key = KEY_9;
                    break;
                default:
                    break;
                }
            }
            a[i] = key;
        }
        return;
    }
    memset(a, 0, 3);
}

static int8_t processKana(const uint8_t* current, const uint8_t* processed, uint8_t* report,
                          const uint8_t base[][12], const uint8_t left[][12], const uint8_t right[][12])
{
    uint8_t mod = current[0];
    uint8_t modifiers;
    uint8_t key;
    uint8_t count = 2;
    uint8_t roma;
    uint8_t a[3];
    const uint8_t* dakuon;
    int8_t xmit = XMIT_NORMAL;

    modifiers = current[0] & ~MOD_SHIFT;
    report[0] = modifiers;
    for (int8_t i = 2; i < 8 && count < 8; ++i) {
        uint8_t code = current[i];
        uint8_t row = code / 12;
        uint8_t column = code % 12;

        key = getKeyNumLock(code);
        if (key) {
            report[count++] = key;
            memset(last, 0, 3);
            lastMod = current[0];
            modifiers = current[0];
            continue;
        }

        uint8_t no_repeat = 0;
        if ((mod & MOD_SHIFT) == MOD_SHIFT) {
            if (lastMod & MOD_LEFTSHIFT)
                mod &= ~MOD_LEFTSHIFT;
            else if (processed[0] & MOD_RIGHTSHIFT)
                mod &= ~MOD_RIGHTSHIFT;
        }

        if (7 <= row)
            roma = 0;
        else if (mod & MOD_LEFTSHIFT)
            roma = left[row][column];
        else if (mod & MOD_RIGHTSHIFT)
            roma = right[row][column];
        else
            roma = base[row][column];
        if (roma && (roma < KANA_DAKUTEN || KANA_CHOUON < roma)) {
            no_repeat = 1;
            for (int8_t j = 2; j < 8; ++j) {
                if (code == processed[j]) {
                    code = VOID_KEY;
                    row = VOID_KEY / 12;
                    column = VOID_KEY % 12;
                    roma = 0;
                    break;
                }
            }
        }
        if (roma)
            processRomaji(roma, a);
        if (!roma || !a[0]) {
            key = getKeyBase(code);
            if (key) {
                key = toggleKanaMode(key, current[0], !memchr(processed + 2, key, 6));
                report[count++] = key;
                memset(last, 0, 3);
                lastMod = current[0];
                modifiers = current[0];
            }
            continue;
        }
        if (no_repeat) {
            for (int8_t i = 0; i < 3 && sent[i]; ++i) {
                for (int8_t j = 0; j < 3 && a[j]; ++j) {
                    if (sent[i] == a[j]) {
                        memset(sent, 0, 3);
                        return XMIT_BRK;
                    }
                }
            }
        }
        xmit = XMIT_IN_ORDER;
        for (int8_t i = 0; i < 3 && a[i] && count < 8; ++i) {
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

int8_t isKanaMode(const uint8_t* current)
{
    return kana_led && !(current[0] & (MOD_ALT | MOD_CONTROL | MOD_GUI)) && !(current[1] & (MOD_FN | MOD_PAD)) && mode != KANA_ROMAJI && (!eisuu_mode || !is109());
}

uint8_t toggleKanaMode(uint8_t key, uint8_t mod, int8_t make)
{
    switch (key) {
    case KEY_LANG1:
        kana_led = 1;
        break;
    case KEY_LANG2:
        kana_led = 0;
        prefix = 0;
        break;
    case KEY_CAPS_LOCK:
        if (make && isJP())
            eisuu_mode ^= 1;
        break;
    case KEY_0:
        if ((mod & MOD_SHIFT) && isJP())
            key = KEY_INTERNATIONAL1;
        break;
    default:
        break;
    }
    return key;
}

int8_t processKeysKana(const uint8_t* current, const uint8_t* processed, uint8_t* report)
{
    switch (mode) {
    case KANA_TRON:
        return processKana(current, processed, report, matrixTron, matrixTronLeft, matrixTronRight);
    case KANA_NICOLA:
        return processKana(current, processed, report, matrixNicola, matrixNicolaLeft, matrixNicolaRight);
#ifndef DISABLE_MTYPE
    case KANA_MTYPE:
        return processKana(current, processed, report, matrixMtype, matrixMtypeShift, matrixMtypeShift);
#endif
#ifndef DISABLE_STICKNEY
    case KANA_STICKNEY:
        return processKana(current, processed, report, matrixStickney, matrixStickneyShift, matrixStickneyShift);
#endif
    case KANA_X6004:
        return processKana(current, processed, report, matrixX6004, matrixX6004Shift, matrixX6004Shift);
    default:
        return processKeysBase(current, processed, report);
    }
}

uint8_t controlKanaLED(uint8_t report)
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
        if (prefix_shift == PREFIXSHIFT_LED) {
            if (prefix & MOD_LEFTSHIFT)
                report |= LED_NUM_LOCK;
            if (prefix & MOD_RIGHTSHIFT)
                report |= LED_SCROLL_LOCK;
        }

    }
    return report;
}
