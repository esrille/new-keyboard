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

#ifndef NVRAM_H
#define NVRAM_H

#include <stdint.h>

#define NVRAM_INITIAL_DATA_SIZE 8

#define NVRAM_DATA(a, b, c, d, e, f, g, h)  \
    const uint8_t nvram_initial_data[NVRAM_INITIAL_DATA_SIZE] = { a, b, c, d, e, f, g, h }

void InitNvram(void);
uint8_t ReadNvram(uint8_t offset);
void WriteNvram(uint8_t offset, uint8_t value);

void SelectProfile(uint8_t profile);
uint8_t CurrentProfile(void);

extern const uint8_t nvram_initial_data[NVRAM_INITIAL_DATA_SIZE];

#endif // NVRAM_H
