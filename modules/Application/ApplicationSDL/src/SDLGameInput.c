/*
 * Copyright 2017-2026 Aaron Barany
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

#include <DeepSea/ApplicationSDL/Types.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Timer.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Round.h>

static dsGameInputAxisRange convertAxisRange(int minValue, int maxValue)
{
	if (minValue < 0 && maxValue == 0)
		return dsGameInputAxisRange_Negative;
	if (minValue == 0 && maxValue > 0)
		return dsGameInputAxisRange_Positive;
	return dsGameInputAxisRange_Full;
}

static SDL_SensorType toSDLSensorType(dsMotionSensorType type)
{
	switch (type)
	{
		case dsMotionSensorType_Accelerometer:
			return SDL_SENSOR_ACCEL;
		case dsMotionSensorType_Gyroscope:
			return SDL_SENSOR_GYRO;
		case dsMotionSensorType_AccelerometerLeft:
			return SDL_SENSOR_ACCEL_L;
		case dsMotionSensorType_GyroscopeLeft:
			return SDL_SENSOR_GYRO_L;
		case dsMotionSensorType_AccelerometerRight:
			return SDL_SENSOR_ACCEL_R;
		case dsMotionSensorType_GyroscopeRight:
			return SDL_SENSOR_GYRO_R;
	}

	DS_ASSERT(false);
	return SDL_SENSOR_UNKNOWN;
}

static void setInputMapping(dsGameInputMap* outMapping, const SDL_GamepadBinding* binding)
{
	switch (binding->input_type)
	{
		case SDL_GAMEPAD_BINDTYPE_BUTTON:
			outMapping->method = dsGameInputMethod_Button;
			outMapping->index = (uint16_t)binding->input.button;
			break;
		case SDL_GAMEPAD_BINDTYPE_AXIS:
			outMapping->method = dsGameInputMethod_Axis;
			outMapping->index = (uint16_t)binding->input.axis.axis;
			if (binding->input.axis.axis_min > binding->input.axis.axis_max)
			{
				outMapping->invertAxis = true;
				outMapping->origAxisRange = (uint8_t)convertAxisRange(
					binding->input.axis.axis_max, binding->input.axis.axis_min);
			}
			else
			{
				outMapping->invertAxis = false;
				outMapping->origAxisRange = (uint8_t)convertAxisRange(
					binding->input.axis.axis_min, binding->input.axis.axis_max);
			}

			if (binding->output_type == SDL_GAMEPAD_BINDTYPE_AXIS)
			{
				outMapping->mappedAxisRange = (uint8_t)convertAxisRange(
					binding->output.axis.axis_max, binding->output.axis.axis_min);
			}
			else
			{
				// If not an axis for the output, assume positive range, such as for a button.
				outMapping->mappedAxisRange = (uint8_t)dsGameInputAxisRange_Positive;
			}
			break;
		case SDL_GAMEPAD_BINDTYPE_HAT:
			outMapping->method = dsGameInputMethod_DPad;
			outMapping->index = (uint16_t)binding->input.hat.hat;
			if (binding->input.hat.hat_mask == SDL_HAT_UP)
				outMapping->dpadDirection = dsGameInputDirection_Up;
			else if (binding->input.hat.hat_mask == SDL_HAT_RIGHT)
				outMapping->dpadDirection = dsGameInputDirection_Right;
			else if (binding->input.hat.hat_mask == SDL_HAT_DOWN)
				outMapping->dpadDirection = dsGameInputDirection_Down;
			else if (binding->input.hat.hat_mask == SDL_HAT_LEFT)
				outMapping->dpadDirection = dsGameInputDirection_Left;
			else if (binding->input.hat.hat_mask == (SDL_HAT_LEFT | SDL_HAT_RIGHT))
			{
				if (binding->output_type == SDL_GAMEPAD_BINDTYPE_AXIS &&
					binding->output.axis.axis_min > binding->output.axis.axis_max)
				{
					outMapping->dpadDirection = dsGameInputDirection_InvXAxis;
				}
				else
					outMapping->dpadDirection = dsGameInputDirection_XAxis;
			}
			else if (binding->input.hat.hat_mask == (SDL_HAT_DOWN | SDL_HAT_UP))
			{
				if (binding->output_type == SDL_GAMEPAD_BINDTYPE_AXIS &&
					binding->output.axis.axis_min > binding->output.axis.axis_max)
				{
					outMapping->dpadDirection = dsGameInputDirection_InvYAxis;
				}
				else
					outMapping->dpadDirection = dsGameInputDirection_YAxis;
			}
			else
				outMapping->method = dsGameInputMethod_Invalid;
			break;
		default:
			outMapping->method = dsGameInputMethod_Invalid;
			break;
	}
}

static dsGameInput* createGameInput(dsApplication* application, SDL_JoystickID id)
{
	SDL_Joystick* joystick = NULL;
	SDL_Gamepad* controller = NULL;
	if (SDL_IsGamepad(id))
	{
		controller = SDL_OpenGamepad(id);
		if (!controller)
		{
			errno = ENOMEM;
			return NULL;
		}
		joystick = SDL_GetGamepadJoystick(controller);
		DS_ASSERT(joystick);
	}
	else
	{
		joystick = SDL_OpenJoystick(id);
		if (!joystick)
		{
			errno = ENOMEM;
			return NULL;
		}
	}

	uint32_t axisCount = SDL_GetNumJoystickAxes(joystick);
	uint32_t buttonCount = SDL_GetNumJoystickButtons(joystick);
	uint32_t dpadCount = SDL_GetNumJoystickHats(joystick);

	size_t fullSize = sizeof(dsSDLGameInput);
	dsMemorySize sizes[] =
	{
		{sizeof(dsVector2f), dpadCount},
		{sizeof(dsGameControllerMap), axisCount},
		{sizeof(dsGameControllerMap), buttonCount},
		{sizeof(dsGameControllerMap)*4, dpadCount},
	};

	if (controller &&
		!dsAccumulateAlignedSizes(&fullSize, sizes, DS_ARRAY_SIZE(sizes), DS_ALLOC_ALIGNMENT))
	{
		return NULL;
	}

	void* buffer = dsAllocator_alloc(application->allocator, fullSize);
	if (!buffer)
	{
		if (controller)
			SDL_CloseGamepad(controller);
		else
			SDL_CloseJoystick(joystick);
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
		inputMap->origAxisRange = 0;
		inputMap->mappedAxisRange = 0;
		inputMap->invertAxis = false;
		inputMap->dpadDirection = 0;
	}

	dsGameControllerMap* axisControllerMaps = NULL;
	dsGameControllerMap* buttonControllerMaps = NULL;
	dsGameControllerMap* dpadControllerMaps = NULL;

	gameInput->controller = controller;
	gameInput->joystick = joystick;
	gameInput->dpadValues = NULL;
	for (unsigned int i = 0; i < DS_GAME_INPUT_RUMBLE_COUNT; ++i)
	{
		dsSDLRumbleState* rumbleState = gameInput->rumbleState + i;
		rumbleState->baselineStrength = 0.0f;
		rumbleState->timedStrength = 0.0f;
		rumbleState->timedDuration = 0;
	}

	if (controller)
	{
		if (dpadCount > 0)
		{
			gameInput->dpadValues = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsVector2i, dpadCount);
			DS_ASSERT(gameInput->dpadValues);
			for (uint32_t i = 0; i < dpadCount; ++i)
			{
				dsSDLGameInput_convertHatDirection(
					gameInput->dpadValues + i, SDL_GetJoystickHat(joystick, i));
			}
		}

		baseGameInput->hasControllerMappings = true;

		if (axisCount > 0)
		{
			axisControllerMaps = DS_ALLOCATE_OBJECT_ARRAY(
				&bufferAlloc, dsGameControllerMap, axisCount);
			DS_ASSERT(axisControllerMaps);
			for (uint32_t i = 0; i < axisCount; ++i)
				axisControllerMaps[i] = dsGameControllerMap_Invalid;
			baseGameInput->axisControllerMaps = axisControllerMaps;
		}

		if (buttonCount > 0)
		{
			buttonControllerMaps = DS_ALLOCATE_OBJECT_ARRAY(
				&bufferAlloc, dsGameControllerMap, buttonCount);
			DS_ASSERT(buttonControllerMaps);
			for (uint32_t i = 0; i < buttonCount; ++i)
				buttonControllerMaps[i] = dsGameControllerMap_Invalid;
			baseGameInput->buttonControllerMaps = buttonControllerMaps;
		}

		if (dpadCount > 0)
		{
			size_t fullCount = (size_t)dpadCount*4;
			dpadControllerMaps = DS_ALLOCATE_OBJECT_ARRAY(
				&bufferAlloc, dsGameControllerMap, fullCount);
			DS_ASSERT(dpadControllerMaps);
			for (size_t i = 0; i < fullCount*4; ++i)
				dpadControllerMaps[i] = dsGameControllerMap_Invalid;
			baseGameInput->dpadControllerMaps = dpadControllerMaps;
		}

		int bindingCount;
		SDL_GamepadBinding** bindings = SDL_GetGamepadBindings(
			gameInput->controller, &bindingCount);
		if (!bindings)
		{
			DS_LOG_ERROR_F(
				DS_APPLICATION_SDL_LOG_TAG, "Couldn't get gamepad bindings: %s", SDL_GetError());
			errno = EPERM;
			return NULL;
		}

		for (int i = 0; i < bindingCount; ++i)
		{
			const SDL_GamepadBinding* binding = bindings[i];
			DS_ASSERT(binding);
			dsGameControllerMap controllerMap;
			switch (binding->output_type)
			{
				case SDL_GAMEPAD_BINDTYPE_BUTTON:
					controllerMap = dsSDLGameInput_controllerMapForButton(binding->output.button);
					break;
				case SDL_GAMEPAD_BINDTYPE_AXIS:
					controllerMap = dsSDLGameInput_controllerMapForAxis(binding->output.axis.axis);
					break;
				default:
					continue;
			}

			if (controllerMap != dsGameControllerMap_Invalid)
			{
				dsGameInputMap* inputMap = baseGameInput->controllerMapping + controllerMap;
				setInputMapping(inputMap + controllerMap, binding);
				switch (inputMap->method)
				{
					case dsGameInputMethod_Axis:
						if (inputMap->index < axisCount)
							axisControllerMaps[inputMap->index] = controllerMap;
						else
							inputMap->method = dsGameInputMethod_Invalid;
						break;
					case dsGameInputMethod_Button:
						if (inputMap->index < buttonCount)
							buttonControllerMaps[inputMap->index] = controllerMap;
						else
							inputMap->method = dsGameInputMethod_Invalid;
						break;
					case dsGameInputMethod_DPad:
						if (inputMap->index < dpadCount)
						{
							switch (inputMap->dpadDirection)
							{
								case dsGameInputDirection_Left:
								case dsGameInputDirection_Right:
								case dsGameInputDirection_Down:
								case dsGameInputDirection_Up:
								{
									uint32_t dpadMapIndex =
										inputMap->index*4 + inputMap->dpadDirection;
									dpadControllerMaps[dpadMapIndex] = controllerMap;
									break;
								}
								case dsGameInputDirection_XAxis:
								case dsGameInputDirection_InvXAxis:
								{
									uint32_t dpadMapIndex =
										inputMap->index*4 + dsGameInputDirection_Left;
									dpadControllerMaps[dpadMapIndex] = controllerMap;
									dpadMapIndex = inputMap->index*4 + dsGameInputDirection_Right;
									dpadControllerMaps[dpadMapIndex] = controllerMap;
									break;
								}
								case dsGameInputDirection_YAxis:
								case dsGameInputDirection_InvYAxis:
								{
									uint32_t dpadMapIndex =
										inputMap->index*4 + dsGameInputDirection_Down;
									dpadControllerMaps[dpadMapIndex] = controllerMap;
									dpadMapIndex = inputMap->index*4 + dsGameInputDirection_Up;
									dpadControllerMaps[dpadMapIndex] = controllerMap;
									break;
								}
							}
						}
						else
							inputMap->method = dsGameInputMethod_Invalid;
						break;
					case dsGameInputMethod_Invalid:
						break;
				}
			}
		}

		SDL_free(bindings);
	}
	else
		baseGameInput->hasControllerMappings = false;

	baseGameInput->application = application;
	baseGameInput->allocator = application->allocator;
	baseGameInput->name = SDL_GetJoystickName(gameInput->joystick);
	switch (SDL_GetJoystickType(gameInput->joystick))
	{
		case SDL_JOYSTICK_TYPE_GAMEPAD:
			switch (SDL_GetGamepadTypeForID(id))
			{
				case SDL_GAMEPAD_TYPE_XBOX360:
					baseGameInput->type = dsGameInputType_XBox360Controller;
					break;
				case SDL_GAMEPAD_TYPE_XBOXONE:
					baseGameInput->type = dsGameInputType_XBoxOneController;
					break;
				case SDL_GAMEPAD_TYPE_PS3:
					baseGameInput->type = dsGameInputType_PS3Controller;
					break;
				case SDL_GAMEPAD_TYPE_PS4:
					baseGameInput->type = dsGameInputType_PS4Controller;
					break;
				case SDL_GAMEPAD_TYPE_PS5:
					baseGameInput->type = dsGameInputType_PS5Controller;
					break;
				case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
					baseGameInput->type = dsGameInputType_NintendoSwitchProController;
					break;
				case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
					baseGameInput->type = dsGameInputType_NintendoSwitchJoyconLeft;
					break;
				case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
					baseGameInput->type = dsGameInputType_NintendoSwitchJoyconRight;
					break;
				case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
					baseGameInput->type = dsGameInputType_NintendoSwitchJoyconPair;
					break;
				case SDL_GAMEPAD_TYPE_GAMECUBE:
					baseGameInput->type = dsGameInputType_GameCubeController;
					break;
				default:
					baseGameInput->type = dsGameInputType_UnknownController;
					break;
			}
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

	baseGameInput->vendorID = SDL_GetJoystickVendor(gameInput->joystick);
	baseGameInput->productID = SDL_GetJoystickProduct(gameInput->joystick);
	baseGameInput->player = SDL_GetJoystickPlayerIndex(gameInput->joystick);
	baseGameInput->axisCount = axisCount;
	baseGameInput->buttonCount = buttonCount;
	baseGameInput->ballCount = SDL_GetNumJoystickBalls(gameInput->joystick);
	baseGameInput->dpadCount = dpadCount;
	if (gameInput->controller)
		baseGameInput->touchpadCount = SDL_GetNumGamepadTouchpads(gameInput->controller);
	else
		baseGameInput->touchpadCount = 0;

	SDL_PropertiesID properties = SDL_GetJoystickProperties(gameInput->joystick);
	baseGameInput->rumbleSupported = SDL_GetBooleanProperty(
		properties, SDL_PROP_JOYSTICK_CAP_RUMBLE_BOOLEAN, false);
	baseGameInput->triggerRumbleSupported = SDL_GetBooleanProperty(
		properties, SDL_PROP_JOYSTICK_CAP_TRIGGER_RUMBLE_BOOLEAN, false);
	baseGameInput->hasLED = SDL_GetBooleanProperty(
			properties, SDL_PROP_JOYSTICK_CAP_RGB_LED_BOOLEAN, false) ||
		SDL_GetBooleanProperty(
			properties, SDL_PROP_JOYSTICK_CAP_MONO_LED_BOOLEAN, false);

	if (gameInput->controller && dsSDLApplication_useMotionSensors(application))
	{
		if (SDL_GamepadHasSensor(gameInput->controller, SDL_SENSOR_ACCEL))
			SDL_SetGamepadSensorEnabled(gameInput->controller, SDL_SENSOR_ACCEL, true);
		if (SDL_GamepadHasSensor(gameInput->controller, SDL_SENSOR_GYRO))
			SDL_SetGamepadSensorEnabled(gameInput->controller, SDL_SENSOR_GYRO, true);
	}

	return baseGameInput;
}

static void freeGameInput(dsGameInput* gameInput)
{
	if (!gameInput)
		return;

	dsSDLGameInput* sdlGameInput = (dsSDLGameInput*)gameInput;
	if (sdlGameInput->controller)
		SDL_CloseGamepad(sdlGameInput->controller);
	else
		SDL_CloseJoystick(sdlGameInput->joystick);
	DS_VERIFY(dsAllocator_free(gameInput->allocator, gameInput));
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
	SDL_RumbleJoystick(sdlGameInput->joystick, (uint16_t)dsRoundf(lowFrequencyStrength*0xFFFF),
		(uint16_t)dsRoundf(highFrequencyStrength*0xFFFF), 1000);
}

static void updateTriggerRumble(dsSDLGameInput* sdlGameInput)
{
	const dsSDLRumbleState* leftState =
		sdlGameInput->rumbleState + dsGameInputRumble_LeftTrigger;
	const dsSDLRumbleState* rightState =
		sdlGameInput->rumbleState + dsGameInputRumble_RightTrigger;
	float leftStrength = dsMax(leftState->baselineStrength, leftState->timedStrength);
	float rightStrength = dsMax(rightState->baselineStrength, rightState->timedStrength);
	SDL_RumbleJoystickTriggers(sdlGameInput->joystick, (uint16_t)dsRoundf(leftStrength*0xFFFF),
		(uint16_t)dsRoundf(rightStrength*0xFFFF), 1000);
}

float dsSDLGameInput_getAxisValue(Sint16 value)
{
	value = dsMax(value, -SDL_JOYSTICK_AXIS_MAX);
	return (float)value/(float)SDL_JOYSTICK_AXIS_MAX;
}

float dsSDLGameInput_getMappedAxisValue(const dsGameInputMap* inputMap, Sint16 value)
{
	DS_ASSERT(inputMap);
	DS_ASSERT(inputMap->method == dsGameInputMethod_Axis);

	float axisValue = dsSDLGameInput_getAxisValue(value);
	if (inputMap->origAxisRange == dsGameInputAxisRange_Negative)
	{
		axisValue = dsMin(axisValue, 0.0f);
		axisValue = (axisValue*2.0f) + 1.0f;
	}
	else if (inputMap->origAxisRange == dsGameInputAxisRange_Positive)
	{
		axisValue = dsMax(axisValue, 0.0f);
		axisValue = (axisValue*2.0f) - 1.0f;
	}
	if (inputMap->invertAxis)
		axisValue = -axisValue;
	if (inputMap->mappedAxisRange == dsGameInputAxisRange_Negative)
		return axisValue*0.5f - 0.5f;
	if (inputMap->mappedAxisRange == dsGameInputAxisRange_Positive)
		return axisValue*0.5f - 0.5f;
	return axisValue;
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

dsGameControllerMap dsSDLGameInput_controllerMapForAxis(SDL_GamepadAxis axis)
{
	switch (axis)
	{
		case SDL_GAMEPAD_AXIS_LEFTX:
			return dsGameControllerMap_LeftXAxis;
		case SDL_GAMEPAD_AXIS_LEFTY:
			return dsGameControllerMap_LeftYAxis;
		case SDL_GAMEPAD_AXIS_RIGHTX:
			return dsGameControllerMap_RightXAxis;
		case SDL_GAMEPAD_AXIS_RIGHTY:
			return dsGameControllerMap_RightYAxis;
		case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
			return dsGameControllerMap_LeftTrigger;
		case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
			return dsGameControllerMap_RightTrigger;
		default:
			return dsGameControllerMap_Invalid;
	}
}

dsGameControllerMap dsSDLGameInput_controllerMapForButton(SDL_GamepadButton button)
{
	switch (button)
	{
		case SDL_GAMEPAD_BUTTON_SOUTH:
			return dsGameControllerMap_FaceButton0;
		case SDL_GAMEPAD_BUTTON_EAST:
			return dsGameControllerMap_FaceButton1;
		case SDL_GAMEPAD_BUTTON_WEST:
			return dsGameControllerMap_FaceButton2;
		case SDL_GAMEPAD_BUTTON_NORTH:
			return dsGameControllerMap_FaceButton3;
		case SDL_GAMEPAD_BUTTON_BACK:
			return dsGameControllerMap_Back;
		case SDL_GAMEPAD_BUTTON_START:
			return dsGameControllerMap_Start;
		case SDL_GAMEPAD_BUTTON_LEFT_STICK:
			return dsGameControllerMap_LeftStick;
		case SDL_GAMEPAD_BUTTON_RIGHT_STICK:
			return dsGameControllerMap_RightStick;
		case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
			return dsGameControllerMap_LeftShoulder;
		case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
			return dsGameControllerMap_RightShoulder;
		case SDL_GAMEPAD_BUTTON_DPAD_UP:
			return dsGameControllerMap_DPadUp;
		case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
			return dsGameControllerMap_DPadDown;
		case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
			return dsGameControllerMap_DPadLeft;
		case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
			return dsGameControllerMap_DPadRight;
		case SDL_GAMEPAD_BUTTON_MISC1:
			return dsGameControllerMap_MiscButton0;
		case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1:
			return dsGameControllerMap_RightPaddle1;
		case SDL_GAMEPAD_BUTTON_LEFT_PADDLE1:
			return dsGameControllerMap_LeftPaddle1;
		case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2:
			return dsGameControllerMap_RightPaddle1;
		case SDL_GAMEPAD_BUTTON_LEFT_PADDLE2:
			return dsGameControllerMap_LeftPaddle1;
		case SDL_GAMEPAD_BUTTON_TOUCHPAD:
			return dsGameControllerMap_Touchpad;
		case SDL_GAMEPAD_BUTTON_MISC2:
			return dsGameControllerMap_MiscButton1;
		case SDL_GAMEPAD_BUTTON_MISC3:
			return dsGameControllerMap_MiscButton2;
		case SDL_GAMEPAD_BUTTON_MISC4:
			return dsGameControllerMap_MiscButton3;
		case SDL_GAMEPAD_BUTTON_MISC5:
			return dsGameControllerMap_MiscButton4;
		case SDL_GAMEPAD_BUTTON_MISC6:
			return dsGameControllerMap_MiscButton5;
		default:
			return dsGameControllerMap_Invalid;
	}
}

bool dsSDLGameInput_setup(dsApplication* application)
{
	DS_ASSERT(!application->gameInputs);
	DS_ASSERT(application->gameInputCount == 0);
	int gameInputCount;
	SDL_JoystickID* joysticks = SDL_GetJoysticks(&gameInputCount);
	if (!joysticks)
	{
		DS_LOG_ERROR_F(
			DS_APPLICATION_SDL_LOG_TAG, "Couldn't get joysticks: %s", SDL_GetError());
		errno = EPERM;
		return false;
	}

	if (gameInputCount == 0)
	{
		SDL_free(joysticks);
		return true;
	}

	dsGameInput** gameInputs = DS_ALLOCATE_OBJECT_ARRAY(
		application->allocator, dsGameInput*, gameInputCount);
	if (!gameInputs)
	{
		SDL_free(joysticks);
		return false;
	}

	for (int i = 0; i < gameInputCount; ++i)
	{
		dsGameInput* gameInput = createGameInput(application, joysticks[i]);
		if (!gameInput)
		{
			dsSDLGameInput_freeAll(gameInputs, i);
			DS_VERIFY(dsAllocator_free(application->allocator, gameInputs));
			SDL_free(joysticks);
			return false;
		}

		gameInputs[i] = gameInput;
	}
	SDL_free(joysticks);

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

dsGameInput* dsSDLGameInput_add(dsApplication* application, SDL_JoystickID id)
{
	dsGameInput* gameInput = createGameInput(application, id);
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
		if (SDL_GetJoystickID(((dsSDLGameInput*)application->gameInputs[i])->joystick) == id)
			return application->gameInputs[i];
	}

	return NULL;
}

void dsSDLGameInput_update(dsGameInput* gameInput, uint64_t elapsedTime)
{
	dsSDLGameInput* sdlGameInput = (dsSDLGameInput*)gameInput;
	for (unsigned int i = 0; i < DS_GAME_INPUT_RUMBLE_COUNT; ++i)
	{
		dsSDLRumbleState* rumbleState = sdlGameInput->rumbleState + i;
		rumbleState->timedDuration -= elapsedTime;
		if (rumbleState->timedDuration <= 0)
		{
			rumbleState->timedStrength = 0.0f;
			rumbleState->timedDuration = 0;
		}
	}

	updateRumble(sdlGameInput);
	updateTriggerRumble(sdlGameInput);
}

bool dsSDLGameInput_dispatchControllerDPadEvents(dsGameInput* gameInput, dsApplication* application,
	dsWindow* window, uint32_t dpad, Sint8 value, uint64_t time)
{
	DS_ASSERT(dpad < gameInput->dpadCount);
	if (!gameInput->dpadControllerMaps)
		return false;

	const dsGameControllerMap* directionMappings = gameInput->dpadControllerMaps + dpad*4;

	// Check if all events are mapped. If not, send the dpad event as well.
	bool allMapped = true;
	bool anyMapped = false;
	for (unsigned int i = 0; i < 4; ++i)
	{
		if (directionMappings[i] == dsGameControllerMap_Invalid)
			allMapped = false;
		else
			anyMapped = true;
	}
	if (!anyMapped)
		return false;

	dsSDLGameInput* sdlGameInput = (dsSDLGameInput*)gameInput;
	dsVector2i direction;
	dsSDLGameInput_convertHatDirection(&direction, value);
	dsVector2i* curDirection = sdlGameInput->dpadValues + dpad;

	for (uint8_t i = 0; i < 2; ++i)
	{
		int8_t curValue = (int8_t)curDirection->values[i];
		int8_t newValue = (int8_t)direction.values[i];
		if (curValue == newValue)
			continue;

		// Order of left/right down/up, or x -1/+1 and y -1/+1.
		const dsGameControllerMap* axisMappings = directionMappings + i*2;

		// Treat as an axis if both directions have the same mapping.
		if (axisMappings[0] == axisMappings[1])
		{
			dsGameControllerMap mapping = axisMappings[0];
			const dsGameInputMap* inputMap = gameInput->controllerMapping + mapping;
			DS_ASSERT(inputMap->method == dsGameInputMethod_DPad);

			dsEvent event;
			event.time = time;
			event.type = dsAppEventType_GameInputAxis;
			event.gameInputAxis.window = window;
			event.gameInputAxis.gameInput = gameInput;
			event.gameInputAxis.mapping = mapping;
			event.gameInputAxis.axis = 0;
			if (inputMap->dpadDirection == dsGameInputDirection_InvXAxis ||
				inputMap->dpadDirection == dsGameInputDirection_InvYAxis)
			{
				event.gameInputAxis.value = (float)-curValue;
			}
			else
				event.gameInputAxis.value = curValue;
			dsApplication_dispatchEvent(application, &event);
			continue;
		}

		// First send up event.
		if (curValue != 0)
		{
			dsGameControllerMap mapping = axisMappings[curValue > 0];
			if (mapping != dsGameControllerMap_Invalid)
			{
				dsEvent event;
				event.time = time;
				event.type = dsAppEventType_GameInputButtonUp;
				event.gameInputButton.window = window;
				event.gameInputButton.gameInput = gameInput;
				event.gameInputButton.mapping = mapping;
				event.gameInputButton.button = 0;
				dsApplication_dispatchEvent(application, &event);
			}
		}

		// Then send down event.
		if (newValue != 0)
		{
			dsGameControllerMap mapping = axisMappings[newValue > 0];
			if (mapping != dsGameControllerMap_Invalid)
			{
				dsEvent event;
				event.time = time;
				event.type = dsAppEventType_GameInputButtonDown;
				event.gameInputButton.window = window;
				event.gameInputButton.gameInput = gameInput;
				event.gameInputButton.mapping = mapping;
				event.gameInputButton.button = 0;
				dsApplication_dispatchEvent(application, &event);
			}
		}
	}

	*curDirection = direction;
	return allMapped;
}

dsSystemPowerState dsSDLGameInput_getPowerState(
	int* outBatteryPercent, const dsApplication* application, const dsGameInput* gameInput)
{
	DS_UNUSED(application);
	SDL_Joystick* joystick = ((const dsSDLGameInput*)gameInput)->joystick;
	switch (SDL_GetJoystickPowerInfo(joystick, outBatteryPercent))
	{
		case SDL_POWERSTATE_ERROR:
		case SDL_POWERSTATE_UNKNOWN:
			return dsSystemPowerState_Unknown;
		case SDL_POWERSTATE_ON_BATTERY:
			return dsSystemPowerState_OnBattery;
		case SDL_POWERSTATE_NO_BATTERY:
			return dsSystemPowerState_External;
		case SDL_POWERSTATE_CHARGING:
			return dsSystemPowerState_Charging;
		case SDL_POWERSTATE_CHARGED:
			return dsSystemPowerState_Charged;
	}

	DS_ASSERT(false);
	return dsSystemPowerState_Unknown;
}

float dsSDLGameInput_getAxis(
	const dsApplication* application, const dsGameInput* gameInput, uint32_t axis)
{
	DS_UNUSED(application);
	return dsSDLGameInput_getAxisValue(
		SDL_GetJoystickAxis(((const dsSDLGameInput*)gameInput)->joystick, axis));
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
		{
			float axisValue = dsSDLGameInput_getAxisValue(
				SDL_GetJoystickAxis(sdlGameInput->joystick, inputMap->index));
			if (inputMap->origAxisRange == dsGameInputAxisRange_Negative)
			{
				axisValue = dsMin(axisValue, 0.0f);
				axisValue = (axisValue*2.0f) + 1.0f;
			}
			else if (inputMap->origAxisRange == dsGameInputAxisRange_Positive)
			{
				axisValue = dsMax(axisValue, 0.0f);
				axisValue = (axisValue*2.0f) - 1.0f;
			}
			if (inputMap->invertAxis)
				axisValue = -axisValue;
			if (inputMap->mappedAxisRange == dsGameInputAxisRange_Negative)
				return axisValue*0.5f - 0.5f;
			if (inputMap->mappedAxisRange == dsGameInputAxisRange_Positive)
				return axisValue*0.5f - 0.5f;
			return axisValue;
		}
		case dsGameInputMethod_Button:
			return (float)SDL_GetJoystickButton(sdlGameInput->joystick, inputMap->index);
		case dsGameInputMethod_DPad:
		{
			dsVector2i direction;
			dsSDLGameInput_convertHatDirection(&direction,
				SDL_GetJoystickHat(sdlGameInput->joystick, inputMap->index));
			switch (inputMap->dpadDirection)
			{
				case dsGameInputDirection_Left:
					return (float)(direction.x < 0);
				case dsGameInputDirection_Right:
					return (float)(direction.x > 0);
				case dsGameInputDirection_Down:
					return (float)(direction.y < 0);
				case dsGameInputDirection_Up:
					return (float)(direction.y > 0);
				case dsGameInputDirection_XAxis:
					return (float)direction.x;
				case dsGameInputDirection_InvXAxis:
					return (float)-direction.x;
				case dsGameInputDirection_YAxis:
					return (float)direction.y;
				case dsGameInputDirection_InvYAxis:
					return (float)-direction.y;
			}
			DS_ASSERT(false);
			return 0.0;
		}
		default:
			DS_ASSERT(false);
			return 0.0;
	}
}

bool dsSDLGameInput_isButtonPressed(
	const dsApplication* application, const dsGameInput* gameInput, uint32_t button)
{
	DS_UNUSED(application);
	return SDL_GetJoystickButton(((const dsSDLGameInput*)gameInput)->joystick, button);
}

bool dsSDLGameInput_isControllerButtonPressed(
	const dsApplication* application, const dsGameInput* gameInput, dsGameControllerMap mapping)
{
	DS_UNUSED(application);
	const dsSDLGameInput* sdlGameInput = (const dsSDLGameInput*)gameInput;
	DS_ASSERT(mapping > dsGameControllerMap_Invalid && mapping < dsGameControllerMap_Count);
	const dsGameInputMap* inputMap = gameInput->controllerMapping + mapping;
	switch (inputMap->method)
	{
		case dsGameInputMethod_Axis:
		{
			float axisValue = dsSDLGameInput_getMappedAxisValue(
				inputMap, SDL_GetJoystickAxis(sdlGameInput->joystick, inputMap->index));
			return axisValue < -0.5f || axisValue > 0.5f;
		}
		case dsGameInputMethod_Button:
			return SDL_GetJoystickButton(sdlGameInput->joystick, inputMap->index);
		case dsGameInputMethod_DPad:
		{
			dsVector2i direction;
			dsSDLGameInput_convertHatDirection(&direction,
				SDL_GetJoystickHat(sdlGameInput->joystick, inputMap->index));
			switch (inputMap->dpadDirection)
			{
				case dsGameInputDirection_Left:
					return direction.x < 0;
				case dsGameInputDirection_Right:
					return direction.x > 0;
				case dsGameInputDirection_Down:
					return direction.y < 0;
				case dsGameInputDirection_Up:
					return direction.y > 0;
				case dsGameInputDirection_XAxis:
				case dsGameInputDirection_InvXAxis:
					return direction.x != 0;
				case dsGameInputDirection_YAxis:
				case dsGameInputDirection_InvYAxis:
					return direction.y != 0;
			}
			DS_ASSERT(false);
			return false;
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
		SDL_GetJoystickHat(((const dsSDLGameInput*)gameInput)->joystick, dpad));
	return true;
}

bool dsSDLGameInput_setBaselineRumble(
	dsApplication* application, dsGameInput* gameInput, dsGameInputRumble rumble, float strength)
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

float dsSDLGameInput_getBaselineRumble(
	dsApplication* application, const dsGameInput* gameInput, dsGameInputRumble rumble)
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

	rumbleState->timedDuration = dsTimer_secondsToTicks(application->timer, duration);
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
	{
		*outDuration = (float)dsTimer_ticksToSeconds(
			application->timer, rumbleState->timedDuration);
	}
	return rumbleState->timedStrength;
}

bool dsSDLGameInput_setLEDColor(dsApplication* application, dsGameInput* gameInput, dsColor color)
{
	DS_UNUSED(application);
	if (!SDL_SetJoystickLED(((dsSDLGameInput*)gameInput)->joystick, color.r, color.g, color.b))
	{
		errno = EPERM;
		return false;
	}

	return true;
}

bool dsSDLGameInput_setPlayer(dsApplication* application, dsGameInput* gameInput, uint32_t player)
{
	DS_UNUSED(application);
	if (!SDL_SetJoystickPlayerIndex(((dsSDLGameInput*)gameInput)->joystick, player))
	{
		errno = EPERM;
		return false;
	}

	return true;
}

bool dsSDLGameInput_hasMotionSensor(
	const dsApplication* application, const dsGameInput* gameInput, dsMotionSensorType type)
{
	if (!dsSDLApplication_useMotionSensors(application))
		return false;

	SDL_Gamepad* controller = ((dsSDLGameInput*)gameInput)->controller;
	if (!controller)
		return false;

	return SDL_GamepadHasSensor(controller, toSDLSensorType(type));
}

bool dsSDLGameInput_getMotionSensorData(dsVector3f* outData, const dsApplication* application,
	const dsGameInput* gameInput, dsMotionSensorType type)
{
	if (!dsSDLApplication_useMotionSensors(application))
	{
		errno = EPERM;
		return false;
	}

	SDL_Gamepad* controller = ((dsSDLGameInput*)gameInput)->controller;
	if (!controller ||
		!SDL_GetGamepadSensorData(controller, toSDLSensorType(type), (float*)outData, 3))
	{
		errno = EPERM;
		return false;
	}

	return true;
}
