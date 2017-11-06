/*
 * Copyright 2015-2017 Esrille Inc.
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

#define PAD_SENSE_1     0
#define PAD_SENSE_2     1
#define PAD_SENSE_3     2
#define PAD_SENSE_4     3
#define PAD_SENSE_MAX   PAD_SENSE_4

void initMouse(void);
void loadMouseSettings(void);
void emitMouse(void);
int8_t processSerialUnit(uint8_t data);
void processMouseKeys(uint8_t* current, const uint8_t* processed);
int8_t isMouseTouched(void);
uint8_t getKeyboardMouseButtons(void);
int8_t getKeyboardMouseX(void);
int8_t getKeyboardMouseY(void);
int8_t getKeyboardMouseWheel(void);
int8_t isProcessingSrialData(void);

#ifdef WITH_HOS
void processMouseData(void);
#endif

#endif  // #ifndef MOUSE_H
