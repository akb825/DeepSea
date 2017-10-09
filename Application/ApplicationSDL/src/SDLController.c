/*
 * Copyright 2017 Aaron Barany
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

#include "SDLController.h"
#include <DeepSea/Application/Application.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <math.h>

static dsController* createController(dsApplication* application, uint32_t index)
{
	dsSDLController* controller = (dsSDLController*)dsAllocator_alloc(application->allocator,
		sizeof(dsSDLController));
	if (!controller)
		return NULL;

	controller->joystick = SDL_JoystickOpen(index);
	DS_ASSERT(controller->joystick);
	controller->haptic = SDL_HapticOpenFromJoystick(controller->joystick);

	dsController* baseController = (dsController*)controller;
	baseController->application = application;
	baseController->allocator = application->allocator;
	baseController->name = SDL_JoystickName(controller->joystick);
	switch (SDL_JoystickGetType(controller->joystick))
	{
		case SDL_JOYSTICK_TYPE_GAMECONTROLLER:
			baseController->type = dsControllerType_Controller;
			break;
		case SDL_JOYSTICK_TYPE_WHEEL:
			baseController->type = dsControllerType_Wheel;
			break;
		case SDL_JOYSTICK_TYPE_ARCADE_STICK:
			baseController->type = dsControllerType_ArcadeStick;
			break;
		case SDL_JOYSTICK_TYPE_FLIGHT_STICK:
			baseController->type = dsControllerType_FlightStick;
			break;
		case SDL_JOYSTICK_TYPE_DANCE_PAD:
			baseController->type = dsControllerType_DancePad;
			break;
		case SDL_JOYSTICK_TYPE_GUITAR:
			baseController->type = dsControllerType_Guitar;
			break;
		case SDL_JOYSTICK_TYPE_DRUM_KIT:
			baseController->type = dsControllerType_DrumKit;
			break;
		case SDL_JOYSTICK_TYPE_ARCADE_PAD:
			baseController->type = dsControllerType_ArcadePad;
			break;
		case SDL_JOYSTICK_TYPE_THROTTLE:
			baseController->type = dsControllerType_Throttle;
			break;
		default:
			baseController->type = dsControllerType_Unknown;
			break;
	}
	baseController->axisCount = SDL_JoystickNumAxes(controller->joystick);
	baseController->buttonCount = SDL_JoystickNumButtons(controller->joystick);
	baseController->ballCount = SDL_JoystickNumBalls(controller->joystick);
	baseController->hatCount = SDL_JoystickNumHats(controller->joystick);
	baseController->rumbleSupported = controller->haptic &&
		SDL_HapticRumbleSupported(controller->haptic);

	return baseController;
}

static void freeController(dsController* controller)
{
	if (!controller)
		return;

	dsSDLController* sdlController = (dsSDLController*)controller;
	if (sdlController->haptic)
		SDL_HapticClose(sdlController->haptic);
	SDL_JoystickClose(sdlController->joystick);
	DS_VERIFY(dsAllocator_free(controller->allocator, sdlController));
}


float dsSDLController_getAxisValue(Sint16 value)
{
	return value < 0 ? value/-32768.0f : value/32767.0f;
}

void dsSDLController_convertHatDirection(int8_t* outX, int8_t* outY, Sint8 hat)
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

bool dsSDLController_setup(dsApplication* application)
{
	DS_ASSERT(!application->controllers);
	DS_ASSERT(application->controllerCount == 0);
	uint32_t controllerCount = SDL_NumJoysticks();
	if (controllerCount == 0)
		return true;

	dsController** controllers = NULL;
		controllers = (dsController**)dsAllocator_alloc(application->allocator,
			sizeof(dsController*)*controllerCount);
	if (!controllers)
		return false;

	for (uint32_t i = 0; i < controllerCount; ++i)
	{
		// Search for an existing controller.
		dsController* controller = createController(application, i);
		if (!controller)
		{
			dsSDLController_freeAll(controllers, i);
			DS_VERIFY(dsAllocator_free(application->allocator, controllers));
			return false;
		}

		controllers[i] = controller;
	}

	application->controllers = controllers;
	application->controllerCount = controllerCount;
	application->controllerCapacity = controllerCount;
	return true;
}

void dsSDLController_freeAll(dsController** controllers, uint32_t controllerCount)
{
	if (!controllers)
		return;

	for (uint32_t i = 0; i < controllerCount; ++i)
		freeController(controllers[i]);
}

dsController* dsSDLController_add(dsApplication* application, uint32_t index)
{
	dsController* controller = createController(application, index);
	if (!controller)
		return NULL;

	if (!dsApplication_addController(application, controller))
	{
		freeController(controller);
		return NULL;
	}

	return controller;
}

bool dsSDLController_remove(dsApplication* application, SDL_JoystickID id)
{
	dsController* controller = NULL;
	for (uint32_t i = 0; i < application->controllerCount; ++i)
	{
		if (SDL_JoystickInstanceID(((dsSDLController*)application->controllers[i])->joystick) == id)
		{
			controller = application->controllers[i];
			break;
		}
	}

	if (!dsApplication_removeController(application, controller))
		return false;

	freeController(controller);
	return true;
}

float dsSDLController_getAxis(const dsApplication* application, const dsController* controller,
	uint32_t axis)
{
	DS_UNUSED(application);
	return dsSDLController_getAxisValue(
		SDL_JoystickGetAxis(((const dsSDLController*)controller)->joystick, axis));
}

bool dsSDLController_isButtonPressed(const dsApplication* application,
	const dsController* controller, uint32_t button)
{
	DS_UNUSED(application);
	return SDL_JoystickGetButton(((const dsSDLController*)controller)->joystick, button) != 0;
}

bool dsSDLController_getHatDirection(dsVector2i* outDirection, const dsApplication* application,
	const dsController* controller, uint32_t hat)
{
	DS_UNUSED(application);
	int8_t x, y;
	dsSDLController_convertHatDirection(&x, &y,
		SDL_JoystickGetHat(((const dsSDLController*)controller)->joystick, hat));
	outDirection->x = x;
	outDirection->y = y;
	return true;
}

bool dsSDLController_startRumble(dsApplication* application, dsController* controller,
	float strength, float duration)
{
	DS_UNUSED(application);
	if (SDL_HapticRumblePlay(((dsSDLController*)controller)->haptic, strength,
		(unsigned int)roundf(duration*1000.0f)) != 0)
	{
		errno = EPERM;
		return false;
	}

	return true;
}

bool dsSDLController_stopRumble(dsApplication* application, dsController* controller)
{
	DS_UNUSED(application);
	if (SDL_HapticRumbleStop(((dsSDLController*)controller)->haptic) != 0)
	{
		errno = EPERM;
		return false;
	}

	return true;
}
