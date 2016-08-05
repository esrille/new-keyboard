/*
 * Copyright 2015, 2016 Esrille Inc.
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

#include <stdio.h>
#include <system.h>
#include <plib/usart.h>

// http://microchip.wikidot.com/faq:29
void putch(char c)
{
#ifdef DEBUG
#ifdef WITH_HOS
    while (Busy2USART())
        CLRWDT();
    Write2USART(c);
    while (Busy2USART())
        CLRWDT();
#else
    while (Busy1USART())
        ;
    Write1USART(c);
    while (Busy1USART())
        ;
#endif
#endif
}
