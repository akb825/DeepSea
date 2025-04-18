/*
 * Copyright 2017-2023 Aaron Barany
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

#include <SDL.h>

#define DS_GAME_INPUT_RUMBLE_COUNT 4

typedef struct dsSDLRumbleState
{
	float baselineStrength;
	float timedStrength;
	float timedDuration;
} dsSDLRumbleState;

typedef struct dsSDLGameInput
{
	dsGameInput gameInput;
	SDL_Joystick* joystick;
	SDL_GameController* controller;
#if !SDL_VERSION_ATLEAST(2, 0, 9)
	SDL_Haptic* haptic;
#endif
	dsVector2i* dpadValues;
	dsSDLRumbleState rumbleState[DS_GAME_INPUT_RUMBLE_COUNT];
} dsSDLGameInput;

float dsSDLGameInput_getAxisValue(Sint16 value);
void dsSDLGameInput_convertHatDirection(dsVector2i* outDirection, Sint8 hat);
dsGameControllerMap dsSDLGameInput_controllerMapForAxis(SDL_GameControllerAxis axis);
dsGameControllerMap dsSDLGameInput_controllerMapForButton(SDL_GameControllerButton button);

bool dsSDLGameInput_setup(dsApplication* application);
void dsSDLGameInput_freeAll(dsGameInput** gameInputs, uint32_t gameInputCount);

dsGameInput* dsSDLGameInput_add(dsApplication* application, uint32_t index);
bool dsSDLGameInput_remove(dsApplication* application, SDL_JoystickID id);
dsGameInput* dsSDLGameInput_find(dsApplication* application, SDL_JoystickID id);

void dsSDLGameInput_update(dsGameInput* gameInput, float time);
void dsSDLGameInput_dispatchControllerDPadEvents(dsGameInput* gameInput, dsApplication* application,
	dsWindow* window, uint32_t dpad, Sint8 value, double time);

dsGameInputBattery dsSDLGameInput_getBattery(const dsApplication* application,
	const dsGameInput* gameInput);
float dsSDLGameInput_getAxis(const dsApplication* application, const dsGameInput* gameInput,
	uint32_t axis);
float dsSDLGameInput_getControllerAxis(const dsApplication* application,
	const dsGameInput* gameInput, dsGameControllerMap mapping);
bool dsSDLGameInput_isButtonPressed(const dsApplication* application,
	const dsGameInput* gameInput, uint32_t button);
bool dsSDLGameInput_isControllerButtonPressed(const dsApplication* application,
	const dsGameInput* gameInput, dsGameControllerMap mapping);
bool dsSDLGameInput_getDPadDirection(dsVector2i* outDirection, const dsApplication* application,
	const dsGameInput* gameInput, uint32_t dpad);
bool dsSDLGameInput_setBaselineRumble(dsApplication* application, dsGameInput* gameInput,
	dsGameInputRumble rumble, float strength);
float dsSDLGameInput_getBaselineRumble(dsApplication* application, const dsGameInput* gameInput,
	dsGameInputRumble rumble);
bool dsSDLGameInput_setTimedRumble(dsApplication* application, dsGameInput* gameInput,
	dsGameInputRumble rumble, float strength, float duration);
float dsSDLGameInput_getTimedRumble(float* outDuration, dsApplication* application,
	const dsGameInput* gameInput, dsGameInputRumble rumble);
bool dsSDLGameInput_setLEDColor(dsApplication* application, dsGameInput* gameInput, dsColor color);
bool dsSDLGameInput_hasMotionSensor(const dsApplication* application, const dsGameInput* gameInput,
	dsMotionSensorType type);
bool dsSDLGameInput_getMotionSensorData(dsVector3f* outData, const dsApplication* application,
	const dsGameInput* gameInput, dsMotionSensorType type);
