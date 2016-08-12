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

#include <stdbool.h>

#ifndef BUTTONS_H
#define BUTTONS_H

// Column bits
#define COLUMN_RB_BITS  0xEF
#define COLUMN_RD_BITS  0xCE

// Row bits
#define ROW_RA_BITS     0x2F
#define ROW_RE_BITS     0x07

#define BUTTON_Enable()             \
    do {                            \
        INTCON2bits.RBPU = 0;       \
        TRISEbits.RDPU = 1;         \
        TRISB |= COLUMN_RB_BITS;    \
        TRISD |= COLUMN_RD_BITS;    \
        TRISA |= ROW_RA_BITS;       \
        TRISE |= ROW_RE_BITS;       \
    } while (0)

#define BUTTON_Disable()            \
    do {                            \
        TRISEbits.RDPU = 0;         \
        INTCON2bits.RBPU = 1;       \
        TRISA &= ~ROW_RA_BITS;      \
        TRISE &= ~ROW_RE_BITS;      \
        TRISB &= ~COLUMN_RB_BITS;   \
        TRISD &= ~COLUMN_RD_BITS;   \
    } while (0)

// Returns true if any one of the keys is pressed
bool BUTTON_IsPressed();

#endif // BUTTONS_H
