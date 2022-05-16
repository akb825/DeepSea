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

#include <DeepSea/Application/GameInput.h>
#include <DeepSea/Core/Error.h>

bool dsGameInput_hasControllerMapping(const dsGameInput* gameInput, dsGameControllerMap mapping)
{
	return gameInput && mapping > dsGameControllerMap_Invalid &&
		mapping < dsGameControllerMap_Count &&
		gameInput->controllerMapping[mapping].method != dsGameInputMethod_Invalid;
}

dsGameControllerMap dsGameInput_findControllerMapping(const dsGameInput* gameInput,
	dsGameInputMethod method, uint32_t index)
{
	if (method == dsGameInputMethod_Invalid)
		return dsGameControllerMap_Invalid;

	for (int i = 0; i < dsGameControllerMap_Count; ++i)
	{
		const dsGameInputMap* mapping = gameInput->controllerMapping + i;
		if (mapping->method == method && mapping->index == index)
			return (dsGameControllerMap)i;
	}

	return dsGameControllerMap_Invalid;
}

dsGameInputBattery dsGameInput_getBattery(const dsGameInput* gameInput)
{
	if (!gameInput || !gameInput->application || !gameInput->application->getGameInputBatteryFunc)
		return dsGameInputBattery_Unknown;

	const dsApplication* application = gameInput->application;
	return application->getGameInputBatteryFunc(application, gameInput);
}

float dsGameInput_getAxis(const dsGameInput* gameInput, uint32_t axis)
{
	if (!gameInput || !gameInput->application || !gameInput->application->getGameInputAxisFunc ||
		axis >= gameInput->axisCount)
	{
		return 0.0f;
	}

	const dsApplication* application = gameInput->application;
	return application->getGameInputAxisFunc(application, gameInput, axis);
}

float dsGameInput_getControllerAxis(const dsGameInput* gameInput, dsGameControllerMap mapping)
{
	if (!dsGameInput_hasControllerMapping(gameInput, mapping) || !gameInput->application ||
		!gameInput->application->getGameInputControllerAxisFunc)
	{
		return 0.0f;
	}

	const dsApplication* application = gameInput->application;
	return application->getGameInputControllerAxisFunc(application, gameInput, mapping);
}

bool dsGameInput_isButtonPressed(const dsGameInput* gameInput, uint32_t button)
{
	if (!gameInput || !gameInput->application ||
		!gameInput->application->isGameInputButtonPressedFunc || button >= gameInput->buttonCount)
	{
		return false;
	}

	const dsApplication* application = gameInput->application;
	return application->isGameInputButtonPressedFunc(application, gameInput, button);
}

bool dsGameInput_isControllerButtonPressed(const dsGameInput* gameInput,
	dsGameControllerMap mapping)
{
	if (!dsGameInput_hasControllerMapping(gameInput, mapping) || !gameInput->application ||
		!gameInput->application->isGameInputControllerButtonPressedFunc)
	{
		return false;
	}

	const dsApplication* application = gameInput->application;
	return application->isGameInputControllerButtonPressedFunc(application, gameInput, mapping);
}

bool dsGameInput_getDPadDirection(dsVector2i* outDirection, const dsGameInput* gameInput,
	uint32_t dpad)
{
	if (!outDirection || !gameInput || !gameInput->application ||
		!gameInput->application->getGameInputDPadDirectionFunc || dpad >= gameInput->dpadCount)
	{
		errno = EINVAL;
		return false;
	}

	const dsApplication* application = gameInput->application;
	return application->getGameInputDPadDirectionFunc(outDirection, application, gameInput, dpad);
}

bool dsGameInput_startRumble(dsGameInput* gameInput, float strength, float duration)
{
	if (!gameInput || !gameInput->rumbleSupported || !gameInput->application ||
		!gameInput->application->startGameInputRumbleFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = gameInput->application;
	return application->startGameInputRumbleFunc(application, gameInput, strength, duration);
}

bool dsGameInput_hasMotionSensor(const dsGameInput* gameInput, dsMotionSensorType type)
{
	if (!gameInput || !gameInput->application ||
		!gameInput->application->gameInputHasMotionSensorFunc)
	{
		errno = EINVAL;
		return false;
	}

	const dsApplication* application = gameInput->application;
	return application->gameInputHasMotionSensorFunc(application, gameInput, type);
}

bool dsGameInput_getMotionSensorData(dsVector3f* outData, const dsGameInput* gameInput,
	dsMotionSensorType type)
{
	if (!outData || !gameInput || !gameInput->application ||
		!gameInput->application->getGameInputMotionSensorDataFunc)
	{
		errno = EINVAL;
		return false;
	}

	const dsApplication* application = gameInput->application;
	return application->getGameInputMotionSensorDataFunc(outData, application, gameInput, type);
}
