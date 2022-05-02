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

#include <DeepSea/Application/Controller.h>
#include <DeepSea/Core/Error.h>

dsControllerBattery dsController_getBattery(const dsController* controller)
{
	if (!controller || !controller->application ||
		!controller->application->getControllerBatteryFunc)
	{
		return dsControllerBattery_Unknown;
	}

	const dsApplication* application = controller->application;
	return application->getControllerBatteryFunc(application, controller);
}

float dsController_getAxis(const dsController* controller, uint32_t axis)
{
	if (!controller || !controller->application ||
		!controller->application->getControllerAxisFunc || axis >= controller->axisCount)
	{
		return 0.0f;
	}

	const dsApplication* application = controller->application;
	return application->getControllerAxisFunc(application, controller, axis);
}

bool dsController_isButtonPressed(const dsController* controller, uint32_t button)
{
	if (!controller || !controller->application ||
		!controller->application->isControllerButtonPressedFunc ||
		button >= controller->buttonCount)
	{
		return false;
	}

	const dsApplication* application = controller->application;
	return application->isControllerButtonPressedFunc(application, controller, button);
}

bool dsController_getHatDirection(dsVector2i* outDirection, const dsController* controller,
	uint32_t hat)
{
	if (!outDirection || !controller || !controller->application ||
		!controller->application->getControllerHatDirectionFunc ||
		hat >= controller->hatCount)
	{
		errno = EINVAL;
		return false;
	}

	const dsApplication* application = controller->application;
	return application->getControllerHatDirectionFunc(outDirection, application, controller, hat);
}

bool dsController_startRumble(dsController* controller, float strength, float duration)
{
	if (!controller || !controller->rumbleSupported || !controller->application ||
		!controller->application->startControllerRumbleFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = controller->application;
	return application->startControllerRumbleFunc(application, controller, strength, duration);
}
