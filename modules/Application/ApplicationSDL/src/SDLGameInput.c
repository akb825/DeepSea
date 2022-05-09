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

static void setInputMapping(dsGameInputMap* outMapping, SDL_GameControllerButtonBind binding)
{
	switch (binding.bindType)
	{
		case SDL_CONTROLLER_BINDTYPE_BUTTON:
			outMapping->method = dsGameInputMethod_Button;
			outMapping->index = binding.value.button;
			break;
		case SDL_CONTROLLER_BINDTYPE_AXIS:
			outMapping->method = dsGameInputMethod_Axis;
			outMapping->index = binding.value.axis;
			break;
		case SDL_CONTROLLER_BINDTYPE_HAT:
			outMapping->method = dsGameInputMethod_DPad;
			outMapping->index = binding.value.hat.hat;
			break;
		default:
			outMapping->method = dsGameInputMethod_Invalid;
			outMapping->index = 0;
			break;
	}
}

static dsGameInput* createGameInput(dsApplication* application, uint32_t index)
{
	dsSDLGameInput* gameInput = DS_ALLOCATE_OBJECT(application->allocator, dsSDLGameInput);
	if (!gameInput)
		return NULL;

	dsGameInput* baseGameInput = (dsGameInput*)gameInput;

	// Initialize the mappings due to different levels of support on SDL.
	for (int i = 0; i < dsGameControllerMap_Count; ++i)
	{
		dsGameInputMap* inputMap = baseGameInput->controllerMapping + i;
		inputMap->method = dsGameInputMethod_Invalid;
		inputMap->index = 0;
	}

	if (SDL_IsGameController(index))
	{
		gameInput->controller = SDL_GameControllerOpen(index);
		if (!gameInput->controller)
		{
			errno = ENOMEM;
			DS_VERIFY(dsAllocator_free(application->allocator, gameInput));
			return NULL;
		}
		gameInput->joystick = SDL_GameControllerGetJoystick(gameInput->controller);
		baseGameInput->hasControllerMappings = true;

		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_LeftXAxis,
			SDL_GameControllerGetBindForAxis(gameInput->controller, SDL_CONTROLLER_AXIS_LEFTX));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_LeftYAxis,
			SDL_GameControllerGetBindForAxis(gameInput->controller, SDL_CONTROLLER_AXIS_LEFTY));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_RightXAxis,
			SDL_GameControllerGetBindForAxis(gameInput->controller, SDL_CONTROLLER_AXIS_RIGHTX));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_RightYAxis,
			SDL_GameControllerGetBindForAxis(gameInput->controller, SDL_CONTROLLER_AXIS_RIGHTY));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_DPadUp,
			SDL_GameControllerGetBindForButton(gameInput->controller,
				SDL_CONTROLLER_BUTTON_DPAD_UP));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_DPadDown,
			SDL_GameControllerGetBindForButton(gameInput->controller,
				SDL_CONTROLLER_BUTTON_DPAD_DOWN));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_DPadLeft,
			SDL_GameControllerGetBindForButton(gameInput->controller,
				SDL_CONTROLLER_BUTTON_DPAD_LEFT));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_DPadRight,
			SDL_GameControllerGetBindForButton(gameInput->controller,
				SDL_CONTROLLER_BUTTON_DPAD_RIGHT));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_FaceButton0,
			SDL_GameControllerGetBindForButton(gameInput->controller, SDL_CONTROLLER_BUTTON_A));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_FaceButton1,
			SDL_GameControllerGetBindForButton(gameInput->controller, SDL_CONTROLLER_BUTTON_B));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_FaceButton2,
			SDL_GameControllerGetBindForButton(gameInput->controller, SDL_CONTROLLER_BUTTON_X));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_FaceButton3,
			SDL_GameControllerGetBindForButton(gameInput->controller, SDL_CONTROLLER_BUTTON_Y));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_Start,
			SDL_GameControllerGetBindForButton(gameInput->controller, SDL_CONTROLLER_BUTTON_START));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_Select,
			SDL_GameControllerGetBindForButton(gameInput->controller, SDL_CONTROLLER_BUTTON_BACK));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_Home,
			SDL_GameControllerGetBindForButton(gameInput->controller, SDL_CONTROLLER_BUTTON_GUIDE));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_LeftStick,
			SDL_GameControllerGetBindForButton(gameInput->controller,
				SDL_CONTROLLER_BUTTON_LEFTSTICK));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_RightStick,
			SDL_GameControllerGetBindForButton(gameInput->controller,
				SDL_CONTROLLER_BUTTON_RIGHTSTICK));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_LeftShoulder,
			SDL_GameControllerGetBindForButton(gameInput->controller,
				SDL_CONTROLLER_BUTTON_LEFTSHOULDER));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_RightShoulder,
			SDL_GameControllerGetBindForButton(gameInput->controller,
				SDL_CONTROLLER_BUTTON_RIGHTSHOULDER));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_LeftTrigger,
			SDL_GameControllerGetBindForAxis(gameInput->controller,
				SDL_CONTROLLER_AXIS_TRIGGERLEFT));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_RightTrigger,
			SDL_GameControllerGetBindForAxis(gameInput->controller,
				SDL_CONTROLLER_AXIS_TRIGGERRIGHT));
#if SDL_VERSION_ATLEAST(2, 0, 14)
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_Paddle0,
			SDL_GameControllerGetBindForButton(gameInput->controller,
				SDL_CONTROLLER_BUTTON_PADDLE1));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_Paddle1,
			SDL_GameControllerGetBindForButton(gameInput->controller,
				SDL_CONTROLLER_BUTTON_PADDLE2));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_Paddle2,
			SDL_GameControllerGetBindForButton(gameInput->controller,
				SDL_CONTROLLER_BUTTON_PADDLE3));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_Paddle3,
			SDL_GameControllerGetBindForButton(gameInput->controller,
				SDL_CONTROLLER_BUTTON_PADDLE4));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_Touchpad,
			SDL_GameControllerGetBindForButton(gameInput->controller,
				SDL_CONTROLLER_BUTTON_TOUCHPAD));
		setInputMapping(baseGameInput->controllerMapping + dsGameControllerMap_MiscButton0,
			SDL_GameControllerGetBindForButton(gameInput->controller,
				SDL_CONTROLLER_BUTTON_MISC1));
#endif
	}
	else
	{
		gameInput->controller = NULL;
		gameInput->joystick = SDL_JoystickOpen(index);
		if (!gameInput->joystick)
		{
			DS_VERIFY(dsAllocator_free(application->allocator, gameInput));
			errno = ENOMEM;
			return NULL;
		}
		baseGameInput->hasControllerMappings = false;
	}

	gameInput->haptic = SDL_HapticOpenFromJoystick(gameInput->joystick);
	if (!gameInput->haptic)
	{
		if (gameInput->controller)
			SDL_GameControllerClose(gameInput->controller);
		else
			SDL_JoystickClose(gameInput->joystick);
		DS_VERIFY(dsAllocator_free(application->allocator, gameInput));
		errno = ENOMEM;
		return NULL;
	}

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
#if SDL_VERSION_ATLEAST(2, 0, 14)
				case SDL_CONTROLLER_TYPE_VIRTUAL:
					baseGameInput->type = dsGameInputType_VirtualController;
					break;
				case SDL_CONTROLLER_TYPE_PS5:
					baseGameInput->type = dsGameInputType_PS5Controller;
					break;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 16)
				case SDL_CONTROLLER_TYPE_AMAZON_LUNA:
					baseGameInput->type = dsGameInputType_AmazonLunaController;
					break;
				case SDL_CONTROLLER_TYPE_GOOGLE_STADIA:
					baseGameInput->type = dsGameInputType_GoogleStadiaController;
					break;
#endif
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
#if SDL_VERSION_ATLEAST(2, 0, 14)
	if (gameInput->controller)
		baseGameInput->touchpadCount = SDL_GameControllerGetNumTouchpads(gameInput->controller);
	else
#endif
		baseGameInput->touchpadCount = 0;
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
	if (sdlGameInput->controller)
		SDL_GameControllerClose(sdlGameInput->controller);
	else
		SDL_JoystickClose(sdlGameInput->joystick);
	DS_VERIFY(dsAllocator_free(gameInput->allocator, sdlGameInput));
}

static bool isAxisPressed(Sint16 value)
{
	return value > 16383 || value < -16383;
}

float dsSDLGameInput_getAxisValue(Sint16 value)
{
	return value/32767.0f;
}

void dsSDLGameInput_convertHatDirection(dsVector2i* outDirection, Sint8 hat)
{
	switch (hat)
	{
		case SDL_HAT_LEFT:
			outDirection->x = -1;
			outDirection->y = 0;
			break;
		case SDL_HAT_LEFTUP:
			outDirection->x = -1;
			outDirection->y = 1;
			break;
		case SDL_HAT_UP:
			outDirection->x = 0;
			outDirection->y = 1;
			break;
		case SDL_HAT_RIGHTUP:
			outDirection->x = 1;
			outDirection->y = 1;
			break;
		case SDL_HAT_RIGHT:
			outDirection->x = 1;
			outDirection->y = 0;
			break;
		case SDL_HAT_RIGHTDOWN:
			outDirection->x = 1;
			outDirection->y = -1;
			break;
		case SDL_HAT_DOWN:
			outDirection->x = 0;
			outDirection->y = -1;
			break;
		case SDL_HAT_LEFTDOWN:
			outDirection->x = -1;
			outDirection->y = -1;
			break;
		default:
			DS_ASSERT(false);
			break;
	}
}

dsGameControllerMap dsSDLGameInput_controllerMapForAxis(SDL_GameControllerAxis axis)
{
	switch (axis)
	{
		case SDL_CONTROLLER_AXIS_LEFTX:
			return dsGameControllerMap_LeftXAxis;
		case SDL_CONTROLLER_AXIS_LEFTY:
			return dsGameControllerMap_LeftYAxis;
		case SDL_CONTROLLER_AXIS_RIGHTX:
			return dsGameControllerMap_RightXAxis;
		case SDL_CONTROLLER_AXIS_RIGHTY:
			return dsGameControllerMap_RightYAxis;
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
			return dsGameControllerMap_LeftTrigger;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
			return dsGameControllerMap_RightTrigger;
		default:
			return dsGameControllerMap_Invalid;
	}
}

dsGameControllerMap dsSDLGameInput_controllerMapForButton(SDL_GameControllerButton button)
{
	switch (button)
	{
		case SDL_CONTROLLER_BUTTON_A:
			return dsGameControllerMap_FaceButton0;
		case SDL_CONTROLLER_BUTTON_B:
			return dsGameControllerMap_FaceButton1;
		case SDL_CONTROLLER_BUTTON_X:
			return dsGameControllerMap_FaceButton2;
		case SDL_CONTROLLER_BUTTON_Y:
			return dsGameControllerMap_FaceButton3;
		case SDL_CONTROLLER_BUTTON_BACK:
			return dsGameControllerMap_Select;
		case SDL_CONTROLLER_BUTTON_GUIDE:
			return dsGameControllerMap_Home;
		case SDL_CONTROLLER_BUTTON_START:
			return dsGameControllerMap_Start;
		case SDL_CONTROLLER_BUTTON_LEFTSTICK:
			return dsGameControllerMap_LeftStick;
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
			return dsGameControllerMap_RightStick;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
			return dsGameControllerMap_LeftShoulder;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			return dsGameControllerMap_RightShoulder;
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			return dsGameControllerMap_DPadUp;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			return dsGameControllerMap_DPadDown;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			return dsGameControllerMap_DPadLeft;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			return dsGameControllerMap_DPadRight;
#if SDL_VERSION_ATLEAST(2, 0, 14)
		case SDL_CONTROLLER_BUTTON_MISC1:
			return dsGameControllerMap_MiscButton0;
		case SDL_CONTROLLER_BUTTON_PADDLE1:
			return dsGameControllerMap_Paddle0;
		case SDL_CONTROLLER_BUTTON_PADDLE2:
			return dsGameControllerMap_Paddle1;
		case SDL_CONTROLLER_BUTTON_PADDLE3:
			return dsGameControllerMap_Paddle2;
		case SDL_CONTROLLER_BUTTON_PADDLE4:
			return dsGameControllerMap_Paddle3;
		case SDL_CONTROLLER_BUTTON_TOUCHPAD:
			return dsGameControllerMap_Touchpad;
#endif
		default:
			return dsGameControllerMap_Invalid;
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

dsGameInput* dsSDLGameInput_find(dsApplication* application, SDL_JoystickID joystickId)
{
	for (uint32_t i = 0; i < application->gameInputCount; ++i)
	{
		if (SDL_JoystickInstanceID(((dsSDLGameInput*)application->gameInputs[i])->joystick) ==
			joystickId)
		{
			return application->gameInputs[i];
		}
	}

	return NULL;
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

float dsSDLGameInput_getControllerAxis(const dsApplication* application,
	const dsGameInput* gameInput, dsGameControllerMap mapping)
{
	DS_UNUSED(application);
	SDL_GameController* gameController = ((const dsSDLGameInput*)gameInput)->controller;
	if (!gameController)
		return 0.0f;

	// TODO: May want to check for button values if they are actually axes and get the joystick
	// state instead. This is probably unlikely to be common enough to be worth implementing.
	switch (mapping)
	{
		case dsGameControllerMap_LeftXAxis:
			return dsSDLGameInput_getAxisValue(
				SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_LEFTX));
		case dsGameControllerMap_LeftYAxis:
			return dsSDLGameInput_getAxisValue(
				SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_LEFTY));
		case dsGameControllerMap_RightXAxis:
			return dsSDLGameInput_getAxisValue(
				SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_RIGHTX));
		case dsGameControllerMap_RightYAxis:
			return dsSDLGameInput_getAxisValue(
				SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_RIGHTY));
		case dsGameControllerMap_DPadUp:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_UP);
		case dsGameControllerMap_DPadDown:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
		case dsGameControllerMap_DPadLeft:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
		case dsGameControllerMap_DPadRight:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
		case dsGameControllerMap_FaceButton0:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_A);
		case dsGameControllerMap_FaceButton1:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_B);
		case dsGameControllerMap_FaceButton2:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_X);
		case dsGameControllerMap_FaceButton3:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_Y);
		case dsGameControllerMap_Start:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_START);
		case dsGameControllerMap_Select:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_BACK);
		case dsGameControllerMap_Home:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_GUIDE);
		case dsGameControllerMap_LeftStick:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_LEFTSTICK);
		case dsGameControllerMap_RightStick:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
		case dsGameControllerMap_LeftShoulder:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
		case dsGameControllerMap_RightShoulder:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
		case dsGameControllerMap_LeftTrigger:
			return dsSDLGameInput_getAxisValue(
				SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_TRIGGERLEFT));
		case dsGameControllerMap_RightTrigger:
			return dsSDLGameInput_getAxisValue(
				SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT));
#if SDL_VERSION_ATLEAST(2, 0, 14)
		case dsGameControllerMap_Paddle0:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_PADDLE1);
		case dsGameControllerMap_Paddle1:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_PADDLE2);
		case dsGameControllerMap_Paddle2:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_PADDLE3);
		case dsGameControllerMap_Paddle3:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_PADDLE4);
		case dsGameControllerMap_Touchpad:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_TOUCHPAD);
		case dsGameControllerMap_MiscButton0:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_MISC1);
#endif
		default:
			return 0.0f;
	}
}

bool dsSDLGameInput_isButtonPressed(const dsApplication* application,
	const dsGameInput* gameInput, uint32_t button)
{
	DS_UNUSED(application);
	return SDL_JoystickGetButton(((const dsSDLGameInput*)gameInput)->joystick, button) != 0;
}

bool dsSDLGameInput_isControllerButtonPressed(const dsApplication* application,
	const dsGameInput* gameInput, dsGameControllerMap mapping)
{
	DS_UNUSED(application);
	SDL_GameController* gameController = ((const dsSDLGameInput*)gameInput)->controller;
	if (!gameController)
		return false;

	switch (mapping)
	{
		case dsGameControllerMap_LeftXAxis:
			return isAxisPressed(
				SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_LEFTX));
		case dsGameControllerMap_LeftYAxis:
			return isAxisPressed(
				SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_LEFTY));
		case dsGameControllerMap_RightXAxis:
			return isAxisPressed(
				SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_RIGHTX));
		case dsGameControllerMap_RightYAxis:
			return isAxisPressed(
				SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_RIGHTY));
		case dsGameControllerMap_DPadUp:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_UP);
		case dsGameControllerMap_DPadDown:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
		case dsGameControllerMap_DPadLeft:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
		case dsGameControllerMap_DPadRight:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
		case dsGameControllerMap_FaceButton0:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_A);
		case dsGameControllerMap_FaceButton1:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_B);
		case dsGameControllerMap_FaceButton2:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_X);
		case dsGameControllerMap_FaceButton3:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_Y);
		case dsGameControllerMap_Start:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_START);
		case dsGameControllerMap_Select:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_BACK);
		case dsGameControllerMap_Home:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_GUIDE);
		case dsGameControllerMap_LeftStick:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_LEFTSTICK);
		case dsGameControllerMap_RightStick:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
		case dsGameControllerMap_LeftShoulder:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
		case dsGameControllerMap_RightShoulder:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
		case dsGameControllerMap_LeftTrigger:
			return isAxisPressed(
				SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_TRIGGERLEFT));
		case dsGameControllerMap_RightTrigger:
			return isAxisPressed(
				SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT));
#if SDL_VERSION_ATLEAST(2, 0, 14)
		case dsGameControllerMap_Paddle0:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_PADDLE1);
		case dsGameControllerMap_Paddle1:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_PADDLE2);
		case dsGameControllerMap_Paddle2:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_PADDLE3);
		case dsGameControllerMap_Paddle3:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_PADDLE4);
		case dsGameControllerMap_Touchpad:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_TOUCHPAD);
		case dsGameControllerMap_MiscButton0:
			return SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_MISC1);
#endif
		default:
			return false;
	}
}

bool dsSDLGameInput_getDPadDirection(dsVector2i* outDirection, const dsApplication* application,
	const dsGameInput* gameInput, uint32_t dpad)
{
	DS_UNUSED(application);
	dsSDLGameInput_convertHatDirection(outDirection,
		SDL_JoystickGetHat(((const dsSDLGameInput*)gameInput)->joystick, dpad));
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
