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

#include <system.h>
#include <nvram.h>
#include <stdio.h>
#include <string.h>
#include <plib/flash.h>

#define NVRAM_ADDRESS   0x1F800
#define NVRAM_SIZE      1024
#define NVRAM_BLOCK     64
#define NVRAM_MAX       (NVRAM_SIZE / NVRAM_BLOCK)

#define PROFILE_SIZE    10
#define PROFILE_MAX     4

typedef struct Profile {
    uint8_t data[PROFILE_SIZE];
} Profile;

typedef struct Profiles {
    Profile profiles[PROFILE_MAX];
    uint8_t reserved[NVRAM_BLOCK - (PROFILE_SIZE * PROFILE_MAX + 2)];
    uint8_t current_profile;
    uint8_t sig;    // 0x01: flashed, 0xff: erased
} Profiles;

static const uint8_t nvramArray[NVRAM_SIZE] @ NVRAM_ADDRESS;    // Note __at() seems not working here with xc8 v1.34
static int8_t current = -1;
static Profiles shadow;

static void PutNvram(void)
{
    uint8_t swdten = WDTCONbits.SWDTEN;
    if (swdten) {
        // Disable watchdog timer while modifying the flash block.
        CLRWDT();
        WDTCONbits.SWDTEN = 0;
    }

    shadow.sig = 0x01;
    if (NVRAM_MAX <= ++current) {
        EraseFlash(NVRAM_ADDRESS, NVRAM_ADDRESS + NVRAM_SIZE);
        current = 0;
    }
    WriteBlockFlash(NVRAM_ADDRESS + NVRAM_BLOCK * current, 1, (void*) &shadow);

    WDTCONbits.SWDTEN = swdten;
}

void InitNvram(void)
{
    current = -1;
    for (int8_t i = 0; i < NVRAM_MAX; ++i) {
        ReadFlash(NVRAM_ADDRESS + NVRAM_BLOCK * i, NVRAM_BLOCK, (void*) &shadow);
        if (shadow.sig == 0x01 && shadow.current_profile < PROFILE_MAX) {
            current = i;
            continue;
        }
        break;
    }
    if (current == -1) {
        current = NVRAM_MAX - 1;
        shadow.current_profile = 0;
        for (int8_t i = 0; i < PROFILE_MAX; ++i) {
            memcpy(shadow.profiles[i].data, nvram_initial_data, NVRAM_INITIAL_DATA_SIZE);
            memset(shadow.profiles[i].data + NVRAM_INITIAL_DATA_SIZE, 0, PROFILE_SIZE - NVRAM_INITIAL_DATA_SIZE);
        }
    } else
        ReadFlash(NVRAM_ADDRESS + NVRAM_BLOCK * current, NVRAM_BLOCK, (void*) &shadow);
}

uint8_t ReadNvram(uint8_t offset)
{
    return shadow.profiles[shadow.current_profile].data[offset];
}

void WriteNvram(uint8_t offset, uint8_t value)
{
    shadow.profiles[shadow.current_profile].data[offset] = value;
    PutNvram();
}

void SelectProfile(uint8_t profile)
{
    shadow.current_profile = profile;
    PutNvram();
}

uint8_t CurrentProfile(void)
{
    return shadow.current_profile;
}
