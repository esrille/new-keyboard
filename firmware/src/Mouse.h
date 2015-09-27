/*
 * Copyright 2015 Esrille Inc.
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

#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

void initMouse(void);
void emitMouse(void);
int8_t processSerialUnit(uint8_t data);
void processMouseKeys(uint8_t* current, const uint8_t* processed);
int8_t isMouseTouched(void);
uint8_t getKeyboardMouseButtons(void);
int8_t getKeyboardMouseX(void);
int8_t getKeyboardMouseY(void);
int8_t getKeyboardMouseWheel(void);

#endif  // #ifndef MOUSE_H

