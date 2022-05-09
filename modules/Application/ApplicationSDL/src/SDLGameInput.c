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

#include "SDLGameInput.h"
#include <DeepSea/Application/Application.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <math.h>

static dsGameInput* createGameInput(dsApplication* application, uint32_t index)
{
	dsSDLGameInput* gameInput = DS_ALLOCATE_OBJECT(application->allocator, dsSDLGameInput);
	if (!gameInput)
		return NULL;

	gameInput->joystick = SDL_JoystickOpen(index);
	DS_ASSERT(gameInput->joystick);
	gameInput->haptic = SDL_HapticOpenFromJoystick(gameInput->joystick);

	dsGameInput* baseGameInput = (dsGameInput*)gameInput;
	baseGameInput->application = application;
	baseGameInput->allocator = application->allocator;
	baseGameInput->name = SDL_JoystickName(gameInput->joystick);
#if SDL_VERSION_ATLEAST(2, 0, 6)
	switch (SDL_JoystickGetType(gameInput->joystick))
	{
		case SDL_JOYSTICK_TYPE_GAMECONTROLLER:
#if SDL_VERSION_ATLEAST(2, 0, 12)
			switch (SDL_GameControllerTypeForIndex(index))
			{
				case SDL_CONTROLLER_TYPE_XBOX360:
					baseGameInput->type = dsGameInputType_XBox360Controller;
					break;
				case SDL_CONTROLLER_TYPE_XBOXONE:
					baseGameInput->type = dsGameInputType_XBoxOneController;
					break;
				case SDL_CONTROLLER_TYPE_PS3:
					baseGameInput->type = dsGameInputType_PS3Controller;
					break;
				case SDL_CONTROLLER_TYPE_PS4:
					baseGameInput->type = dsGameInputType_PS4Controller;
					break;
				case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO:
					baseGameInput->type = dsGameInputType_NintendoSwitchController;
					break;
				case SDL_CONTROLLER_TYPE_VIRTUAL:
					baseGameInput->type = dsGameInputType_VirtualController;
					break;
				case SDL_CONTROLLER_TYPE_PS5:
					baseGameInput->type = dsGameInputType_PS5Controller;
					break;
				case SDL_CONTROLLER_TYPE_AMAZON_LUNA:
					baseGameInput->type = dsGameInputType_AmazonLunaController;
					break;
				case SDL_CONTROLLER_TYPE_GOOGLE_STADIA:
					baseGameInput->type = dsGameInputType_GoogleStadiaController;
					break;
				default:
					baseGameInput->type = dsGameInputType_UnknownController;
					break;
			}
#else
			baseGameInput->type = dsGameInputType_UnknownController;
#endif
			break;
		case SDL_JOYSTICK_TYPE_WHEEL:
			baseGameInput->type = dsGameInputType_Wheel;
			break;
		case SDL_JOYSTICK_TYPE_ARCADE_STICK:
			baseGameInput->type = dsGameInputType_ArcadeStick;
			break;
		case SDL_JOYSTICK_TYPE_FLIGHT_STICK:
			baseGameInput->type = dsGameInputType_FlightStick;
			break;
		case SDL_JOYSTICK_TYPE_DANCE_PAD:
			baseGameInput->type = dsGameInputType_DancePad;
			break;
		case SDL_JOYSTICK_TYPE_GUITAR:
			baseGameInput->type = dsGameInputType_Guitar;
			break;
		case SDL_JOYSTICK_TYPE_DRUM_KIT:
			baseGameInput->type = dsGameInputType_DrumKit;
			break;
		case SDL_JOYSTICK_TYPE_ARCADE_PAD:
			baseGameInput->type = dsGameInputType_ArcadePad;
			break;
		case SDL_JOYSTICK_TYPE_THROTTLE:
			baseGameInput->type = dsGameInputType_Throttle;
			break;
		default:
			baseGameInput->type = dsGameInputType_Unknown;
			break;
	}
#else
	baseGameInput->type = dsGameInputType_Unknown;
#endif
	baseGameInput->axisCount = SDL_JoystickNumAxes(gameInput->joystick);
	baseGameInput->buttonCount = SDL_JoystickNumButtons(gameInput->joystick);
	baseGameInput->ballCount = SDL_JoystickNumBalls(gameInput->joystick);
	baseGameInput->dpadCount = SDL_JoystickNumHats(gameInput->joystick);
	baseGameInput->rumbleSupported = gameInput->haptic &&
		SDL_HapticRumbleSupported(gameInput->haptic);

	return baseGameInput;
}

static void freeGameInput(dsGameInput* gameInput)
{
	if (!gameInput)
		return;

	dsSDLGameInput* sdlGameInput = (dsSDLGameInput*)gameInput;
	if (sdlGameInput->haptic)
		SDL_HapticClose(sdlGameInput->haptic);
	SDL_JoystickClose(sdlGameInput->joystick);
	DS_VERIFY(dsAllocator_free(gameInput->allocator, sdlGameInput));
}

float dsSDLGameInput_getAxisValue(Sint16 value)
{
	return value < 0 ? value/-32768.0f : value/32767.0f;
}

void dsSDLGameInput_convertHatDirection(int8_t* outX, int8_t* outY, Sint8 hat)
{
	switch (hat)
	{
		case SDL_HAT_LEFT:
			*outX = -1;
			*outY = 0;
			break;
		case SDL_HAT_LEFTUP:
			*outX = -1;
			*outY = 1;
			break;
		case SDL_HAT_UP:
			*outX = 0;
			*outY = 1;
			break;
		case SDL_HAT_RIGHTUP:
			*outX = 1;
			*outY = 1;
			break;
		case SDL_HAT_RIGHT:
			*outX = 1;
			*outY = 0;
			break;
		case SDL_HAT_RIGHTDOWN:
			*outX = 1;
			*outY = -1;
			break;
		case SDL_HAT_DOWN:
			*outX = 0;
			*outY = -1;
			break;
		case SDL_HAT_LEFTDOWN:
			*outX = -1;
			*outY = -1;
			break;
		default:
			DS_ASSERT(false);
			break;
	}
}

bool dsSDLGameInput_setup(dsApplication* application)
{
	DS_ASSERT(!application->gameInputs);
	DS_ASSERT(application->gameInputCount == 0);
	uint32_t gameInputCount = SDL_NumJoysticks();
	if (gameInputCount == 0)
		return true;

	dsGameInput** gameInputs = NULL;
		gameInputs = DS_ALLOCATE_OBJECT_ARRAY(application->allocator, dsGameInput*,
			gameInputCount);
	if (!gameInputs)
		return false;

	for (uint32_t i = 0; i < gameInputCount; ++i)
	{
		// Search for an existing gameInput.
		dsGameInput* gameInput = createGameInput(application, i);
		if (!gameInput)
		{
			dsSDLGameInput_freeAll(gameInputs, i);
			DS_VERIFY(dsAllocator_free(application->allocator, gameInputs));
			return false;
		}

		gameInputs[i] = gameInput;
	}

	application->gameInputs = gameInputs;
	application->gameInputCount = gameInputCount;
	application->gameInputCapacity = gameInputCount;
	return true;
}

void dsSDLGameInput_freeAll(dsGameInput** gameInputs, uint32_t gameInputCount)
{
	if (!gameInputs)
		return;

	for (uint32_t i = 0; i < gameInputCount; ++i)
		freeGameInput(gameInputs[i]);
}

dsGameInput* dsSDLGameInput_add(dsApplication* application, uint32_t index)
{
	dsGameInput* gameInput = createGameInput(application, index);
	if (!gameInput)
		return NULL;

	if (!dsApplication_addGameInput(application, gameInput))
	{
		freeGameInput(gameInput);
		return NULL;
	}

	return gameInput;
}

bool dsSDLGameInput_remove(dsApplication* application, SDL_JoystickID id)
{
	dsGameInput* gameInput = NULL;
	for (uint32_t i = 0; i < application->gameInputCount; ++i)
	{
		if (SDL_JoystickInstanceID(((dsSDLGameInput*)application->gameInputs[i])->joystick) == id)
		{
			gameInput = application->gameInputs[i];
			break;
		}
	}

	if (!dsApplication_removeGameInput(application, gameInput))
		return false;

	freeGameInput(gameInput);
	return true;
}

dsGameInputBattery dsSDLGameInput_getBattery(const dsApplication* application,
	const dsGameInput* gameInput)
{
	DS_UNUSED(application);
	switch (SDL_JoystickCurrentPowerLevel(((const dsSDLGameInput*)gameInput)->joystick))
	{
		case SDL_JOYSTICK_POWER_EMPTY:
			return dsGameInputBattery_Empty;
		case SDL_JOYSTICK_POWER_LOW:
			return dsGameInputBattery_Low;
		case SDL_JOYSTICK_POWER_MEDIUM:
			return dsGameInputBattery_Medium;
		case SDL_JOYSTICK_POWER_FULL:
			return dsGameInputBattery_Full;
		case SDL_JOYSTICK_POWER_WIRED:
			return dsGameInputBattery_Wired;
		default:
			return dsGameInputBattery_Unknown;
	}
}

float dsSDLGameInput_getAxis(const dsApplication* application, const dsGameInput* gameInput,
	uint32_t axis)
{
	DS_UNUSED(application);
	return dsSDLGameInput_getAxisValue(
		SDL_JoystickGetAxis(((const dsSDLGameInput*)gameInput)->joystick, axis));
}

bool dsSDLGameInput_isButtonPressed(const dsApplication* application,
	const dsGameInput* gameInput, uint32_t button)
{
	DS_UNUSED(application);
	return SDL_JoystickGetButton(((const dsSDLGameInput*)gameInput)->joystick, button) != 0;
}

bool dsSDLGameInput_getDPadDirection(dsVector2i* outDirection, const dsApplication* application,
	const dsGameInput* gameInput, uint32_t dpad)
{
	DS_UNUSED(application);
	int8_t x = 0, y = 0;
	dsSDLGameInput_convertHatDirection(&x, &y,
		SDL_JoystickGetHat(((const dsSDLGameInput*)gameInput)->joystick, dpad));
	outDirection->x = x;
	outDirection->y = y;
	return true;
}

bool dsSDLGameInput_startRumble(dsApplication* application, dsGameInput* gameInput,
	float strength, float duration)
{
	DS_UNUSED(application);
	if (SDL_HapticRumblePlay(((dsSDLGameInput*)gameInput)->haptic, strength,
		(unsigned int)roundf(duration*1000.0f)) != 0)
	{
		errno = EPERM;
		return false;
	}

	return true;
}

bool dsSDLGameInput_stopRumble(dsApplication* application, dsGameInput* gameInput)
{
	DS_UNUSED(application);
	if (SDL_HapticRumbleStop(((dsSDLGameInput*)gameInput)->haptic) != 0)
	{
		errno = EPERM;
		return false;
	}

	return true;
}
