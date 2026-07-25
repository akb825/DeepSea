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

#include <DeepSea/Application/GameInput.h>
#include <DeepSea/Core/Error.h>

bool dsGameInput_hasControllerMapping(const dsGameInput* gameInput, dsGameControllerMap mapping)
{
	return gameInput && mapping > dsGameControllerMap_Invalid &&
		mapping < dsGameControllerMap_Count &&
		gameInput->controllerMapping[mapping].method != dsGameInputMethod_Invalid;
}

dsGameControllerMap dsGameInput_getAxisControllerMap(
	const dsGameInput* gameInput, uint32_t axis)
{
	if (!gameInput || !gameInput->axisControllerMaps || axis >= gameInput->axisCount)
		return dsGameControllerMap_Invalid;

	return gameInput->axisControllerMaps[axis];
}

dsGameControllerMap dsGameInput_getButtonControllerMap(
	const dsGameInput* gameInput, uint32_t button)
{
	if (!gameInput || !gameInput->buttonControllerMaps || button >= gameInput->buttonCount)
		return dsGameControllerMap_Invalid;

	return gameInput->buttonControllerMaps[button];
}

dsGameControllerMap dsGameInput_getDPadControllerMap(
	const dsGameInput* gameInput, uint32_t dpad, dsGameInputDirection direction)
{
	if (direction == dsGameInputDirection_XAxis || direction == dsGameInputDirection_InvXAxis)
		direction = dsGameInputDirection_Left;
	if (direction == dsGameInputDirection_YAxis || direction == dsGameInputDirection_InvYAxis)
		direction = dsGameInputDirection_Down;

	if (!gameInput || !gameInput->dpadControllerMaps || dpad >= gameInput->dpadCount ||
		direction < dsGameInputDirection_Left || direction > dsGameInputDirection_Up)
	{
		return dsGameControllerMap_Invalid;
	}

	return gameInput->dpadControllerMaps[dpad*4 + direction];
}

dsSystemPowerState dsGameInput_getPowerState(int* outBatteryPercent, const dsGameInput* gameInput)
{
	if (!gameInput || !gameInput->application ||
		!gameInput->application->getGameInputPowerStateFunc)
	{
		if (outBatteryPercent)
			*outBatteryPercent = -1;
		return dsSystemPowerState_Unknown;
	}

	const dsApplication* application = gameInput->application;
	return application->getGameInputPowerStateFunc(outBatteryPercent, application, gameInput);
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

bool dsGameInput_isControllerButtonPressed(
	const dsGameInput* gameInput, dsGameControllerMap mapping)
{
	if (!dsGameInput_hasControllerMapping(gameInput, mapping) || !gameInput->application ||
		!gameInput->application->isGameInputControllerButtonPressedFunc)
	{
		return false;
	}

	const dsApplication* application = gameInput->application;
	return application->isGameInputControllerButtonPressedFunc(application, gameInput, mapping);
}

bool dsGameInput_getDPadDirection(
	dsVector2i* outDirection, const dsGameInput* gameInput, uint32_t dpad)
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

bool dsGameInput_setBaselineRumble(dsGameInput* gameInput, dsGameInputRumble rumble, float strength)
{
	if (!gameInput || !gameInput->application ||
		!gameInput->application->setGameInputBaselineRumbleFunc || strength < 0 || strength > 1)
	{
		errno = EINVAL;
		return false;
	}

	switch (rumble)
	{
		case dsGameInputRumble_LowFrequency:
		case dsGameInputRumble_HighFrequency:
			if (!gameInput->rumbleSupported)
			{
				errno = EPERM;
				return false;
			}
			break;
		case dsGameInputRumble_LeftTrigger:
		case dsGameInputRumble_RightTrigger:
			if (!gameInput->triggerRumbleSupported)
			{
				errno = EPERM;
				return false;
			}
			break;
	}

	dsApplication* application = gameInput->application;
	return application->setGameInputBaselineRumbleFunc(application, gameInput, rumble, strength);
}

float dsGameInput_getBaselineRumble(const dsGameInput* gameInput, dsGameInputRumble rumble)
{
	if (!gameInput || !gameInput->application ||
		!gameInput->application->getGameInputBaselineRumbleFunc)
	{
		errno = EINVAL;
		return false;
	}

	switch (rumble)
	{
		case dsGameInputRumble_LowFrequency:
		case dsGameInputRumble_HighFrequency:
			if (!gameInput->rumbleSupported)
				return 0.0f;
			break;
		case dsGameInputRumble_LeftTrigger:
		case dsGameInputRumble_RightTrigger:
			if (!gameInput->triggerRumbleSupported)
				return 0.0f;
			break;
	}

	dsApplication* application = gameInput->application;
	return application->getGameInputBaselineRumbleFunc(application, gameInput, rumble);
}

bool dsGameInput_setTimedRumble(
	dsGameInput* gameInput, dsGameInputRumble rumble, float strength, float duration)
{
	if (!gameInput || !gameInput->application ||
		!gameInput->application->setGameInputBaselineRumbleFunc || strength < 0 || strength > 1 ||
		duration < 0)
	{
		errno = EINVAL;
		return false;
	}

	switch (rumble)
	{
		case dsGameInputRumble_LowFrequency:
		case dsGameInputRumble_HighFrequency:
			if (!gameInput->rumbleSupported)
			{
				errno = EPERM;
				return false;
			}
			break;
		case dsGameInputRumble_LeftTrigger:
		case dsGameInputRumble_RightTrigger:
			if (!gameInput->triggerRumbleSupported)
			{
				errno = EPERM;
				return false;
			}
			break;
	}

	dsApplication* application = gameInput->application;
	return application->setGameInputTimedRumbleFunc(
		application, gameInput, rumble, strength, duration);
}

float dsGameInput_getTimedRumble(
	float* outDuration, const dsGameInput* gameInput, dsGameInputRumble rumble)
{
	if (!gameInput || !gameInput->application ||
		!gameInput->application->getGameInputTimedRumbleFunc)
	{
		errno = EINVAL;
		return false;
	}

	switch (rumble)
	{
		case dsGameInputRumble_LowFrequency:
		case dsGameInputRumble_HighFrequency:
			if (!gameInput->rumbleSupported)
			{
				if (outDuration)
					*outDuration = 0.0f;
				return 0.0f;
			}
			break;
		case dsGameInputRumble_LeftTrigger:
		case dsGameInputRumble_RightTrigger:
			if (!gameInput->triggerRumbleSupported)
			{
				if (outDuration)
					*outDuration = 0.0f;
				return 0.0f;
			}
			break;
	}

	dsApplication* application = gameInput->application;
	return application->getGameInputTimedRumbleFunc(outDuration, application, gameInput, rumble);
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

bool dsGameInput_setPlayer(dsGameInput* gameInput, uint32_t player)
{
	if (!gameInput || !gameInput->application || !gameInput->application->setGameInputPlayerFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = gameInput->application;
	return application->setGameInputPlayerFunc(application, gameInput, player);
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

bool dsGameInput_getMotionSensorData(
	dsVector3f* outData, const dsGameInput* gameInput, dsMotionSensorType type)
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
