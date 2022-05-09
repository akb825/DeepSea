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
