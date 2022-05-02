/*
 * Copyright 2017-2022 Aaron Barany
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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Application/Types.h>

#include <math.h>
#include <SDL.h>

typedef struct dsSDLController
{
	dsController controller;
	SDL_Joystick* joystick;
	SDL_Haptic* haptic;
} dsSDLController;

float dsSDLController_getAxisValue(Sint16 value);
void dsSDLController_convertHatDirection(int8_t* outX, int8_t* outY, Sint8 hat);

bool dsSDLController_setup(dsApplication* application);
void dsSDLController_freeAll(dsController** controllers, uint32_t controllerCount);

dsController* dsSDLController_add(dsApplication* application, uint32_t index);
bool dsSDLController_remove(dsApplication* application, SDL_JoystickID id);

dsControllerBattery dsSDLController_getBattery(const dsApplication* application,
	const dsController* controller);
float dsSDLController_getAxis(const dsApplication* application, const dsController* controller,
	uint32_t axis);
bool dsSDLController_isButtonPressed(const dsApplication* application,
	const dsController* controller, uint32_t button);
bool dsSDLController_getHatDirection(dsVector2i* outDirection, const dsApplication* application,
	const dsController* controller, uint32_t hat);
bool dsSDLController_startRumble(dsApplication* application, dsController* controller,
	float strength, float duration);
bool dsSDLController_stopRumble(dsApplication* application, dsController* controller);
