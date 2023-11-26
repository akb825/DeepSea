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

#include <DeepSea/Application/GameInput.h>
#include <DeepSea/Core/Error.h>

bool dsGameInput_hasControllerMapping(const dsGameInput* gameInput, dsGameControllerMap mapping)
{
	return gameInput && mapping > dsGameControllerMap_Invalid &&
		mapping < dsGameControllerMap_Count &&
		gameInput->controllerMapping[mapping].method != dsGameInputMethod_Invalid;
}

bool dsGameInput_isInputControllerMapped(const dsGameInput* gameInput, dsGameInputMethod method,
	uint32_t index)
{
	if (!gameInput || method == dsGameInputMethod_Invalid)
		return false;

	for (int i = 0; i < dsGameControllerMap_Count; ++i)
	{
		const dsGameInputMap* mapping = gameInput->controllerMapping + i;
		{
			if (mapping->method == method && mapping->index == index)
				return true;
		}
	}

	return false;
}

dsGameControllerMap dsGameInput_findControllerMapping(const dsGameInput* gameInput,
	const dsGameInputMap* inputMap)
{
	if (!gameInput || !inputMap || inputMap->method == dsGameInputMethod_Invalid)
		return dsGameControllerMap_Invalid;

	for (int i = 0; i < dsGameControllerMap_Count; ++i)
	{
		const dsGameInputMap* mapping = gameInput->controllerMapping + i;
		{
			if (mapping->method == inputMap->method && mapping->index == inputMap->index &&
				(mapping->method != dsGameInputMethod_DPad ||
					(mapping->method == dsGameInputMethod_DPad &&
						mapping->dpadAxis == inputMap->dpadAxis &&
						mapping->dpadAxisValue == inputMap->dpadAxisValue)))
			{
				return (dsGameControllerMap)i;
			}
		}
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
		!gameInput->application->getGameInputDPadDirectionFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (dpad >= gameInput->dpadCount)
	{
		errno = EINDEX;
		return false;
	}

	const dsApplication* application = gameInput->application;
	return application->getGameInputDPadDirectionFunc(outDirection, application, gameInput, dpad);
}

bool dsGameInput_setRumble(dsGameInput* gameInput, float lowFrequencyStrength,
	float highFrequencyStrength, float duration)
{
	if (!gameInput || !gameInput->application || !gameInput->application->setGameInputRumbleFunc ||
		lowFrequencyStrength < 0 || lowFrequencyStrength > 1 ||
		highFrequencyStrength < 0 || highFrequencyStrength > 1 || duration < 0)
	{
		errno = EINVAL;
		return false;
	}

	if (!gameInput->rumbleSupported)
	{
		errno = EPERM;
		return false;
	}

	dsApplication* application = gameInput->application;
	return application->setGameInputRumbleFunc(application, gameInput, lowFrequencyStrength,
		highFrequencyStrength, duration);
}

bool dsGameInput_setTriggerRumble(dsGameInput* gameInput, float leftStrength,
	float rightStrength, float duration)
{
	if (!gameInput || !gameInput->application ||
		!gameInput->application->setGameInputTriggerRumbleFunc || leftStrength < 0 ||
		leftStrength > 1 || rightStrength < 0 || rightStrength > 1 || duration < 0)
	{
		errno = EINVAL;
		return false;
	}

	if (!gameInput->triggerRumbleSupported)
	{
		errno = EPERM;
		return false;
	}

	dsApplication* application = gameInput->application;
	return application->setGameInputTriggerRumbleFunc(application, gameInput, leftStrength,
		rightStrength, duration);
}

bool dsGameInput_setLEDColor(dsGameInput* gameInput, dsColor color)
{
	if (!gameInput || !gameInput->application || !gameInput->application->setGameInputLEDColorFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (!gameInput->hasLED)
	{
		errno = EPERM;
		return false;
	}

	dsApplication* application = gameInput->application;
	return application->setGameInputLEDColorFunc(application, gameInput, color);
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
