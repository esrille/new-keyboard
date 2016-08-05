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

#define NVRAM_DATA(a, b, c, d, e, f, g, h)  __EEPROM_DATA(a, b, c, d, e, f, g, h)

#define InitNvram()
#define ReadNvram(offset)           eeprom_read(offset)
#define WriteNvram(offset, value)   eeprom_write(offset, value)

#endif //NVRAM_H
