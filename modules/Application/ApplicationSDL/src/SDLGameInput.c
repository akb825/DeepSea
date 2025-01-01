/*
 * Copyright 2017-2024 Aaron Barany
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

#include "SDLApplicationInternal.h"

#include <DeepSea/Application/Application.h>
#include <DeepSea/Application/GameInput.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>

#include <DeepSea/Math/Core.h>

static void setInputMapping(dsGameInputMap* outMapping, SDL_GameControllerButtonBind binding)
{
	switch (binding.bindType)
	{
		case SDL_CONTROLLER_BINDTYPE_BUTTON:
			outMapping->method = dsGameInputMethod_Button;
			outMapping->index = (uint16_t)binding.value.button;
			break;
		case SDL_CONTROLLER_BINDTYPE_AXIS:
			outMapping->method = dsGameInputMethod_Axis;
			outMapping->index = (uint16_t)binding.value.axis;
			break;
		case SDL_CONTROLLER_BINDTYPE_HAT:
			outMapping->method = dsGameInputMethod_DPad;
			outMapping->index = (uint16_t)binding.value.hat.hat;
			if (binding.value.hat.hat_mask & SDL_HAT_UP)
			{
				outMapping->dpadAxis = 1;
				outMapping->dpadAxisValue = 1;
			}
			else if (binding.value.hat.hat_mask & SDL_HAT_RIGHT)
			{
				outMapping->dpadAxis = 0;
				outMapping->dpadAxisValue = 1;
			}
			else if (binding.value.hat.hat_mask & SDL_HAT_DOWN)
			{
				outMapping->dpadAxis = 1;
				outMapping->dpadAxisValue = -1;
			}
			else if (binding.value.hat.hat_mask & SDL_HAT_LEFT)
			{
				outMapping->dpadAxis = 0;
				outMapping->dpadAxisValue = -1;
			}
			break;
		default:
			outMapping->method = dsGameInputMethod_Invalid;
			break;
	}
}

static dsGameInput* createGameInput(dsApplication* application, uint32_t index)
{
	SDL_Joystick* joystick = NULL;
	SDL_GameController* controller = NULL;
	if (SDL_IsGameController(index))
	{
		controller = SDL_GameControllerOpen(index);
		if (!controller)
		{
			errno = ENOMEM;
			return NULL;
		}
		joystick = SDL_GameControllerGetJoystick(controller);
		DS_ASSERT(joystick);
	}
	else
	{
		joystick = SDL_JoystickOpen(index);
		if (!joystick)
		{
			errno = ENOMEM;
			return NULL;
		}
	}

	uint32_t dpadCount = SDL_JoystickNumHats(joystick);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSDLGameInput));
	if (controller && dpadCount > 0)
		fullSize += DS_ALIGNED_SIZE(sizeof(dsVector2i)*dpadCount);
	void* buffer = dsAllocator_alloc(application->allocator, fullSize);
	if (!buffer)
	{
		if (controller)
			SDL_GameControllerClose(controller);
		else
			SDL_JoystickClose(joystick);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSDLGameInput* gameInput = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSDLGameInput);
	DS_ASSERT(gameInput);

	dsGameInput* baseGameInput = (dsGameInput*)gameInput;

	// Initialize the mappings due to different levels of support on SDL.
	for (int i = 0; i < dsGameControllerMap_Count; ++i)
	{
		dsGameInputMap* inputMap = baseGameInput->controllerMapping + i;
		inputMap->method = dsGameInputMethod_Invalid;
		inputMap->index = 0;
		inputMap->dpadAxis = 0;
		inputMap->dpadAxisValue = 0;
	}

	gameInput->controller = controller;
	gameInput->joystick = joystick;
	gameInput->dpadValues = NULL;
	for (unsigned int i = 0; i < DS_GAME_INPUT_RUMBLE_COUNT; ++i)
	{
		dsSDLRumbleState* rumbleState = gameInput->rumbleState + i;
		rumbleState->baselineStrength = 0.0f;
		rumbleState->timedStrength = 0.0f;
		rumbleState->timedDuration = 0.0f;
	}

	if (controller)
	{
		if (dpadCount > 0)
		{
			gameInput->dpadValues = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsVector2i, dpadCount);
			DS_ASSERT(gameInput->dpadValues);
			for (uint32_t i = 0; i < dpadCount; ++i)
			{
				dsSDLGameInput_convertHatDirection(gameInput->dpadValues + i,
					SDL_JoystickGetHat(joystick, i));
			}
		}

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
		baseGameInput->hasControllerMappings = false;

#if !SDL_VERSION_ATLEAST(2, 0, 9)
	gameInput->haptic = SDL_HapticOpenFromJoystick(gameInput->joystick);
#endif

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
	baseGameInput->dpadCount = dpadCount;
#if SDL_VERSION_ATLEAST(2, 0, 14)
	if (gameInput->controller)
		baseGameInput->touchpadCount = SDL_GameControllerGetNumTouchpads(gameInput->controller);
	else
#endif
		baseGameInput->touchpadCount = 0;
#if SDL_VERSION_ATLEAST(2, 0, 18)
	baseGameInput->rumbleSupported = SDL_JoystickHasRumble(gameInput->joystick);
#elif SDL_VERSION_ATLEAST(2, 0, 9)
	// Guess as there's no way to query without setting a rumble.
	baseGameInput->rumbleSupported = baseGameInput->type >= dsGameInputType_UnknownController;
#else
	baseGameInput->rumbleSupported = gameInput->haptic &&
		SDL_HapticRumbleSupported(gameInput->haptic);
#endif

#if SDL_VERSION_ATLEAST(2, 0, 18)
	baseGameInput->triggerRumbleSupported = SDL_JoystickHasRumbleTriggers(gameInput->joystick);
#elif SDL_VERSION_ATLEAST(2, 0, 14)
	// Guess as there's no way to query without setting a rumble.
	baseGameInput->triggerRumbleSupported =
		baseGameInput->type == dsGameInputType_XBoxOneController;
#else
	baseGameInput->triggerRumbleSupported = false;
#endif

#if SDL_VERSION_ATLEAST(2, 0, 14)
	baseGameInput->hasLED = SDL_JoystickHasLED(gameInput->joystick);
#else
	baseGameInput->hasLED = false;
#endif

#if SDL_VERSION_ATLEAST(2, 0, 14)
	if (gameInput->controller && dsSDLApplication_useMotionSensors(application))
	{
		if (SDL_GameControllerHasSensor(gameInput->controller, SDL_SENSOR_ACCEL))
			SDL_GameControllerSetSensorEnabled(gameInput->controller, SDL_SENSOR_ACCEL, true);
		if (SDL_GameControllerHasSensor(gameInput->controller, SDL_SENSOR_GYRO))
			SDL_GameControllerSetSensorEnabled(gameInput->controller, SDL_SENSOR_GYRO, true);
	}
#endif

	return baseGameInput;
}

static void freeGameInput(dsGameInput* gameInput)
{
	if (!gameInput)
		return;

	dsSDLGameInput* sdlGameInput = (dsSDLGameInput*)gameInput;
#if !SDL_VERSION_ATLEAST(2, 0, 9)
	if (sdlGameInput->haptic)
		SDL_HapticClose(sdlGameInput->haptic);
#endif
	if (sdlGameInput->controller)
		SDL_GameControllerClose(sdlGameInput->controller);
	else
		SDL_JoystickClose(sdlGameInput->joystick);
	DS_VERIFY(dsAllocator_free(gameInput->allocator, gameInput));
}

static bool isAxisPressed(Sint16 value)
{
	return value > 16383 || value < -16383;
}

static void updateRumble(dsSDLGameInput* sdlGameInput)
{
	const dsSDLRumbleState* lowFrequencyState =
		sdlGameInput->rumbleState + dsGameInputRumble_LowFrequency;
	const dsSDLRumbleState* highFrequencyState =
		sdlGameInput->rumbleState + dsGameInputRumble_HighFrequency;
	float lowFrequencyStrength = dsMax(lowFrequencyState->baselineStrength,
		lowFrequencyState->timedStrength);
	float highFrequencyStrength = dsMax(highFrequencyState->baselineStrength,
		highFrequencyState->timedStrength);
#if SDL_VERSION_ATLEAST(2, 0, 9)
	SDL_JoystickRumble(sdlGameInput->joystick, (uint16_t)roundf(lowFrequencyStrength*0xFFFF),
		(uint16_t)roundf(highFrequencyStrength*0xFFFF), 1000);
#else
	float strength = (lowFrequencyStrength + highFrequencyStrength) * 0.5f;
	if (strength == 0)
		SDL_HapticRumbleStop(((dsSDLGameInput*)gameInput)->haptic);
	else
		SDL_HapticRumblePlay(((dsSDLGameInput*)gameInput)->haptic, strength, 1000);
#endif
}

static void updateTriggerRumble(dsSDLGameInput* sdlGameInput)
{
#if SDL_VERSION_ATLEAST(2, 0, 14)
	const dsSDLRumbleState* leftState =
		sdlGameInput->rumbleState + dsGameInputRumble_LeftTrigger;
	const dsSDLRumbleState* rightState =
		sdlGameInput->rumbleState + dsGameInputRumble_RightTrigger;
	float leftStrength = dsMax(leftState->baselineStrength, leftState->timedStrength);
	float rightStrength = dsMax(rightState->baselineStrength, rightState->timedStrength);
	SDL_JoystickRumbleTriggers(sdlGameInput->joystick, (uint16_t)roundf(leftStrength*0xFFFF),
		(uint16_t)roundf(rightStrength*0xFFFF), 1000);
#else
	DS_UNUSED(sdlGameInput);
#endif
}

float dsSDLGameInput_getAxisValue(Sint16 value)
{
	return value/32767.0f;
}

void dsSDLGameInput_convertHatDirection(dsVector2i* outDirection, Sint8 hat)
{
	if (hat & SDL_HAT_LEFT)
		outDirection->x = -1;
	else if (hat & SDL_HAT_RIGHT)
		outDirection->x = 1;
	else
		outDirection->x = 0;

	if (hat & SDL_HAT_DOWN)
		outDirection->y = -1;
	else if (hat & SDL_HAT_UP)
		outDirection->y = 1;
	else
		outDirection->y = 0;
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

	dsGameInput** gameInputs = DS_ALLOCATE_OBJECT_ARRAY(application->allocator, dsGameInput*,
		gameInputCount);
	if (!gameInputs)
		return false;

	for (uint32_t i = 0; i < gameInputCount; ++i)
	{
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
	dsGameInput* gameInput = dsSDLGameInput_find(application, id);
	if (!gameInput)
	{
		errno = ENOTFOUND;
		return false;
	}

	if (!dsApplication_removeGameInput(application, gameInput))
		return false;

	freeGameInput(gameInput);
	return true;
}

dsGameInput* dsSDLGameInput_find(dsApplication* application, SDL_JoystickID id)
{
	for (uint32_t i = 0; i < application->gameInputCount; ++i)
	{
		if (SDL_JoystickInstanceID(((dsSDLGameInput*)application->gameInputs[i])->joystick) == id)
			return application->gameInputs[i];
	}

	return NULL;
}

void dsSDLGameInput_update(dsGameInput* gameInput, float time)
{
	dsSDLGameInput* sdlGameInput = (dsSDLGameInput*)gameInput;
	for (unsigned int i = 0; i < DS_GAME_INPUT_RUMBLE_COUNT; ++i)
	{
		dsSDLRumbleState* rumbleState = sdlGameInput->rumbleState + i;
		rumbleState->timedDuration -= time;
		if (rumbleState->timedDuration <= 0.0f)
		{
			rumbleState->timedStrength = 0.0f;
			rumbleState->timedDuration = 0.0f;
		}
	}

	updateRumble(sdlGameInput);
	updateTriggerRumble(sdlGameInput);
}

void dsSDLGameInput_dispatchControllerDPadEvents(dsGameInput* gameInput, dsApplication* application,
	dsWindow* window, uint32_t dpad, Sint8 value, double time)
{
	dsSDLGameInput* sdlGameInput = (dsSDLGameInput*)gameInput;
	dsVector2i direction;
	dsSDLGameInput_convertHatDirection(&direction, value);
	dsVector2i* curDirection = sdlGameInput->dpadValues + dpad;

	dsEvent event;
	event.time = time;
	event.gameInputButton.gameInput = gameInput;
	event.gameInputButton.button = 0;
	dsGameInputMap inputMap = {dsGameInputMethod_DPad, (uint16_t)dpad};

	// Send up events first.
	event.type = dsAppEventType_GameInputButtonUp;
	for (uint8_t i = 0; i < 2; ++i)
	{
		int8_t curValue = (int8_t)curDirection->values[i];
		if (curValue == 0 || curValue == direction.values[i])
			continue;

		inputMap.dpadAxis = i;
		inputMap.dpadAxisValue = curValue;
		event.gameInputButton.mapping =
			dsGameInput_findControllerMapping(gameInput, &inputMap);
		DS_ASSERT(event.gameInputButton.mapping != dsGameControllerMap_Invalid);
		dsApplication_dispatchEvent(application, window, &event);

		if (direction.values[i] != 0)
		{
			event.type = dsAppEventType_GameInputButtonDown;
			inputMap.dpadAxis = i;
			inputMap.dpadAxisValue = (int8_t)direction.values[i];
			event.gameInputButton.mapping =
				dsGameInput_findControllerMapping(gameInput, &inputMap);
			DS_ASSERT(event.gameInputButton.mapping != dsGameControllerMap_Invalid);
			dsApplication_dispatchEvent(application, window, &event);
		}
	}

	// Then send down events.
	event.type = dsAppEventType_GameInputButtonDown;
	for (uint8_t i = 0; i < 2; ++i)
	{
		int8_t newValue = (int8_t)direction.values[i];
		if (newValue == 0 || newValue == curDirection->values[i])
			continue;

		inputMap.dpadAxis = i;
		inputMap.dpadAxisValue = newValue;
		event.gameInputButton.mapping =
			dsGameInput_findControllerMapping(gameInput, &inputMap);
		DS_ASSERT(event.gameInputButton.mapping != dsGameControllerMap_Invalid);
		dsApplication_dispatchEvent(application, window, &event);
	}

	*curDirection = direction;
}

dsGameInputBattery dsSDLGameInput_getBattery(const dsApplication* application,
	const dsGameInput* gameInput)
{
	DS_UNUSED(application);
#if SDL_VERSION_ATLEAST(2, 0, 4)
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
#else
	DS_UNUSED(gameInput);
	return dsGameInputBattery_Unknown;
#endif
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
	const dsSDLGameInput* sdlGameInput = (const dsSDLGameInput*)gameInput;
	DS_ASSERT(mapping > dsGameControllerMap_Invalid && mapping < dsGameControllerMap_Count);
	const dsGameInputMap* inputMap = gameInput->controllerMapping + mapping;
	switch (inputMap->method)
	{
		case dsGameInputMethod_Axis:
			return SDL_JoystickGetAxis(sdlGameInput->joystick, inputMap->index);
		case dsGameInputMethod_Button:
			return (float)SDL_JoystickGetButton(sdlGameInput->joystick, inputMap->index);
		case dsGameInputMethod_DPad:
		{
			dsVector2i direction;
			dsSDLGameInput_convertHatDirection(&direction,
				SDL_JoystickGetHat(sdlGameInput->joystick, inputMap->index));
			return (float)(direction.values[inputMap->dpadAxis] == inputMap->dpadAxisValue);
		}
		default:
			DS_ASSERT(false);
			return 0.0;
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
	const dsSDLGameInput* sdlGameInput = (const dsSDLGameInput*)gameInput;
	DS_ASSERT(mapping > dsGameControllerMap_Invalid && mapping < dsGameControllerMap_Count);
	const dsGameInputMap* inputMap = gameInput->controllerMapping + mapping;
	switch (inputMap->method)
	{
		case dsGameInputMethod_Axis:
			return isAxisPressed(SDL_JoystickGetAxis(sdlGameInput->joystick, inputMap->index));
		case dsGameInputMethod_Button:
			return SDL_JoystickGetButton(sdlGameInput->joystick, inputMap->index);
		case dsGameInputMethod_DPad:
		{
			dsVector2i direction;
			dsSDLGameInput_convertHatDirection(&direction,
				SDL_JoystickGetHat(sdlGameInput->joystick, inputMap->index));
			return direction.values[inputMap->dpadAxis] == inputMap->dpadAxisValue;
		}
		default:
			DS_ASSERT(false);
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

bool dsSDLGameInput_setBaselineRumble(dsApplication* application, dsGameInput* gameInput,
	dsGameInputRumble rumble, float strength)
{
	DS_UNUSED(application);
	dsSDLGameInput* sdlGameInput = (dsSDLGameInput*)gameInput;
	dsSDLRumbleState* rumbleState = sdlGameInput->rumbleState + rumble;
	if (strength == rumbleState->baselineStrength)
		return true;

	rumbleState->baselineStrength = strength;

	switch (rumble)
	{
		case dsGameInputRumble_LowFrequency:
		case dsGameInputRumble_HighFrequency:
			updateRumble(sdlGameInput);
			break;
		case dsGameInputRumble_LeftTrigger:
		case dsGameInputRumble_RightTrigger:
			updateTriggerRumble(sdlGameInput);
			break;
	}

	return true;
}

float dsSDLGameInput_getBaselineRumble(dsApplication* application, const dsGameInput* gameInput,
	dsGameInputRumble rumble)
{
	DS_UNUSED(application);
	const dsSDLGameInput* sdlGameInput = (const dsSDLGameInput*)gameInput;
	return sdlGameInput->rumbleState[rumble].baselineStrength;
}

bool dsSDLGameInput_setTimedRumble(dsApplication* application, dsGameInput* gameInput,
	dsGameInputRumble rumble, float strength, float duration)
{
	DS_UNUSED(application);
	dsSDLGameInput* sdlGameInput = (dsSDLGameInput*)gameInput;
	dsSDLRumbleState* rumbleState = sdlGameInput->rumbleState + rumble;
	if (duration == 0.0f)
		strength = 0.0f;

	rumbleState->timedDuration = duration;
	if (strength == rumbleState->baselineStrength)
		return true;

	rumbleState->timedStrength = strength;

	switch (rumble)
	{
		case dsGameInputRumble_LowFrequency:
		case dsGameInputRumble_HighFrequency:
			updateRumble(sdlGameInput);
			break;
		case dsGameInputRumble_LeftTrigger:
		case dsGameInputRumble_RightTrigger:
			updateTriggerRumble(sdlGameInput);
			break;
	}

	return true;
}

float dsSDLGameInput_getTimedRumble(float* outDuration, dsApplication* application,
	const dsGameInput* gameInput, dsGameInputRumble rumble)
{
	DS_UNUSED(application);
	const dsSDLGameInput* sdlGameInput = (const dsSDLGameInput*)gameInput;
	const dsSDLRumbleState* rumbleState = sdlGameInput->rumbleState + rumble;
	if (outDuration)
		*outDuration = rumbleState->timedDuration;
	return rumbleState->timedStrength;
}

bool dsSDLGameInput_setLEDColor(dsApplication* application, dsGameInput* gameInput, dsColor color)
{
	DS_UNUSED(application);

#if SDL_VERSION_ATLEAST(2, 0, 14)
	if (SDL_JoystickSetLED(((dsSDLGameInput*)gameInput)->joystick, color.r, color.g, color.b) != 0)
	{
		errno = EPERM;
		return false;
	}

	return true;
#else
	DS_UNUSED(gameInput);
	DS_UNUSED(color);

	errno = EPERM;
	return false;
#endif
}

bool dsSDLGameInput_hasMotionSensor(const dsApplication* application, const dsGameInput* gameInput,
	dsMotionSensorType type)
{
#if SDL_VERSION_ATLEAST(2, 0, 14)
	if (!dsSDLApplication_useMotionSensors(application))
		return false;

	SDL_GameController* controller = ((dsSDLGameInput*)gameInput)->controller;
	if (!controller)
		return false;

	SDL_SensorType sdlType;
	switch (type)
	{
		case dsMotionSensorType_Accelerometer:
			sdlType = SDL_SENSOR_ACCEL;
			break;
		case dsMotionSensorType_Gyroscope:
			sdlType = SDL_SENSOR_GYRO;
			break;
		default:
			DS_ASSERT(false);
			return false;
	}
	return SDL_GameControllerHasSensor(controller, sdlType);
#else
	DS_UNUSED(application);
	DS_UNUSED(gameInput);
	DS_UNUSED(type);
	return false;
#endif
}

bool dsSDLGameInput_getMotionSensorData(dsVector3f* outData, const dsApplication* application,
	const dsGameInput* gameInput, dsMotionSensorType type)
{
#if SDL_VERSION_ATLEAST(2, 0, 14)
	if (!dsSDLApplication_useMotionSensors(application))
	{
		errno = EPERM;
		return false;
	}

	SDL_GameController* controller = ((dsSDLGameInput*)gameInput)->controller;
	if (!controller)
	{
		errno = EPERM;
		return false;
	}

	SDL_SensorType sdlType;
	switch (type)
	{
		case dsMotionSensorType_Accelerometer:
			sdlType = SDL_SENSOR_ACCEL;
			break;
		case dsMotionSensorType_Gyroscope:
			sdlType = SDL_SENSOR_GYRO;
			break;
		default:
			DS_ASSERT(false);
			return false;
	}

	if (SDL_GameControllerGetSensorData(controller, sdlType, (float*)outData, 3) != 0)
	{
		errno = EPERM;
		return false;
	}

	return true;
#else
	DS_UNUSED(outData);
	DS_UNUSED(application);
	DS_UNUSED(gameInput);
	DS_UNUSED(type);
	errno = EPERM;
	return false;
#endif
}
