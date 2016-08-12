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
#include <buttons.h>

bool BUTTON_IsPressed()
{
    bool pressed;

    INTCON2bits.RBPU = 0;
    TRISEbits.RDPU = 1;
    TRISB |= COLUMN_RB_BITS;
    TRISD |= COLUMN_RD_BITS;
    Nop(); Nop(); Nop();
    pressed = (~PORTD & COLUMN_RD_BITS) || (~PORTB & COLUMN_RB_BITS);
    TRISEbits.RDPU = 0;
    INTCON2bits.RBPU = 1;
    TRISB &= ~COLUMN_RB_BITS;
    TRISD &= ~COLUMN_RD_BITS;
    return pressed;
}
