/*
 * Copyright 2014-2016 Esrille Inc.
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

// Internal pull-ups are always enabled in NISSE Rev.1 to Rev.5
#define BUTTON_Enable()
#define BUTTON_Disable()

// Returns true if any one of the keys is pressed
bool BUTTON_IsPressed();

#endif // BUTTONS_H
