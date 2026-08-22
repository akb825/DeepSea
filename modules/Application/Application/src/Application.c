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

#include <DeepSea/Application/Application.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Sort.h>
#include <DeepSea/Core/Timer.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Math/Round.h>

#include <stdlib.h>
#include <string.h>

static uint32_t nextWindowResponderID;
static uint32_t nextEventResponderID;

static void applicationLogWrapper(void* userData, dsLogLevel level, const char* tag,
	const char* file, unsigned int line, const char* function, const char* message)
{
	dsLog_defaultPrint(level, tag, file,  line, function, message);

	if (level == dsLogLevel_Fatal)
	{
		const char* name = "Exit";
		dsApplication* application = (dsApplication*)userData;
		dsApplication_showMessageBox(application, NULL, dsMessageBoxType_Error, "Fatal Error",
			message, &name, 1, 0, DS_MESSAGE_BOX_NO_BUTTON);
		abort();
	}
}

static int compareEventResponders(const void* left, const void* right)
{
	return DS_CMP(((dsEventResponder*)left)->priority, ((dsEventResponder*)right)->priority);
}

bool dsApplication_setUserData(
	dsApplication* application, void* userData, dsDestroyUserDataFunction destroyUserDataFunc)
{
	if (!application)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return false;
	}

	if (application->destroyUserDataFunc)
		application->destroyUserDataFunc(application->userData);

	application->userData = userData;
	application->destroyUserDataFunc = destroyUserDataFunc;
	return true;
}

uint32_t dsApplication_addWindowResponder(
	dsApplication* application, const dsWindowResponder* responder)
{
	if (!application || !responder)
	{
		if (responder && responder->destroyUserDataFunc)
			responder->destroyUserDataFunc(responder->userData);
		errno = EINVAL;
		return 0;
	}

	uint32_t index = application->windowResponderCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(application->allocator, application->windowResponders,
			application->windowResponderCount, application->windowResponderCapacity, 1))
	{
		if (responder->destroyUserDataFunc)
			responder->destroyUserDataFunc(responder->userData);
		return 0;
	}

	uint32_t id = ++nextWindowResponderID;
	application->windowResponders[index] = *responder;
	application->windowResponders[index].responderID = id;
	return id;
}

bool dsApplication_removeWindowResponder(dsApplication* application, uint32_t responderID)
{
	if (!application || responderID == 0)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < application->windowResponderCount; ++i)
	{
		dsWindowResponder* responder = application->windowResponders + i;
		if (responder->responderID != responderID)
			continue;

		if (responder->destroyUserDataFunc)
			responder->destroyUserDataFunc(responder->userData);

		memmove(application->windowResponders + i, application->windowResponders + i + 1,
			sizeof(dsWindowResponder)*(application->windowResponderCount - i - 1));
		--application->windowResponderCount;
		return true;
	}

	errno = ENOTFOUND;
	return false;
}

uint32_t dsApplication_addEventResponder(
	dsApplication* application, const dsEventResponder* responder)
{
	if (!application || !responder || !responder->eventFunc)
	{
		if (responder && responder->destroyUserDataFunc)
			responder->destroyUserDataFunc(responder->userData);
		errno = EINVAL;
		return 0;
	}

	uint32_t index = application->eventResponderCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(application->allocator, application->eventResponders,
			application->eventResponderCount, application->eventResponderCapacity, 1))
	{
		if (responder->destroyUserDataFunc)
			responder->destroyUserDataFunc(responder->userData);
		return 0;
	}

	uint32_t id = ++nextEventResponderID;
	application->eventResponders[index] = *responder;
	application->eventResponders[index].responderID = id;

	qsort(application->eventResponders, application->eventResponderCount, sizeof(dsEventResponder),
		&compareEventResponders);
	return id;
}

bool dsApplication_removeEventResponder(dsApplication* application, uint32_t responderID)
{
	if (!application || responderID == 0)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < application->eventResponderCount; ++i)
	{
		dsEventResponder* responder = application->eventResponders + i;
		if (responder->responderID != responderID)
			continue;

		if (responder->destroyUserDataFunc)
			responder->destroyUserDataFunc(responder->userData);

		memmove(application->eventResponders + i, application->eventResponders + i + 1,
			sizeof(dsEventResponder)*(application->eventResponderCount - i - 1));
		--application->eventResponderCount;
		return true;
	}

	errno = ENOTFOUND;
	return false;
}

bool dsApplication_setUpdateRate(dsApplication* application, float updateRate)
{
	if (!application)
	{
		errno = EINVAL;
		return false;
	}

	if (!application->setUpdateRateFunc)
	{
		application->updateRate = updateRate;
		return true;
	}

	return application->setUpdateRateFunc(application, updateRate);
}

bool dsApplication_setUpdateFunction(dsApplication* application,
	dsUpdateApplicationFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc)
{
	if (!application)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return false;
	}

	if (application->destroyUpdateUserDataFunc)
		application->destroyUpdateUserDataFunc(application->updateUserData);
	application->updateFunc = function;
	application->updateUserData = userData;
	application->destroyUpdateUserDataFunc = destroyUserDataFunc;
	return true;
}

bool dsApplication_setFinishFrameFunction(dsApplication* application,
	dsFinishApplicationFrameFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc)
{
	if (!application)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return false;
	}

	if (application->destroyFinishFrameUserDataFunc)
		application->destroyFinishFrameUserDataFunc(application->finishFrameUserData);
	application->finishFrameFunc = function;
	application->finishFrameUserData = userData;
	application->destroyFinishFrameUserDataFunc = destroyUserDataFunc;
	return true;
}

bool dsApplication_addWindow(dsApplication* application, dsWindow* window)
{
	if (!application || !application->destroyWindowFunc || !window ||
		window->application != application)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < application->windowCount; ++i)
	{
		if (application->windows[i] == window)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_APPLICATION_LOG_TAG, "Window has already been added.");
			return false;
		}
	}

	uint32_t index = application->windowCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(application->allocator, application->windows,
		application->windowCount, application->windowCapacity, 1))
	{
		return false;
	}

	application->windows[index] = window;
	for (uint32_t i = 0; i < application->windowResponderCount; ++i)
	{
		const dsWindowResponder* responder = application->windowResponders + i;
		if (responder->windowAddedFunc)
			responder->windowAddedFunc(application, window, responder->userData);
	}
	return true;
}

bool dsApplication_removeWindow(dsApplication* application, dsWindow* window)
{
	if (!application || !window)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < application->windowCount; ++i)
	{
		if (application->windows[i] == window)
		{
			for (uint32_t i = 0; i < application->windowResponderCount; ++i)
			{
				const dsWindowResponder* responder = application->windowResponders + i;
				if (responder->windowRemovedFunc)
					responder->windowRemovedFunc(application, window, responder->userData);
			}
			memmove(application->windows + i, application->windows + i + 1,
				sizeof(dsWindow*)*(application->windowCount - i - 1));
			--application->windowCount;
			return true;
		}
	}

	errno = ENOTFOUND;
	return false;
}

uint32_t dsApplication_showMessageBox(dsApplication* application, dsWindow* parentWindow,
	dsMessageBoxType type, const char* title, const char* message, const char* const* buttons,
	uint32_t buttonCount, uint32_t enterButton, uint32_t escapeButton)
{
	if (!application || !application->showMessageBoxFunc || !title || !message || !buttons ||
		buttonCount == 0 || buttonCount > DS_MAX_MESSAGE_BOX_BUTTONS ||
		(enterButton != DS_MESSAGE_BOX_NO_BUTTON && enterButton >= buttonCount) ||
		(escapeButton != DS_MESSAGE_BOX_NO_BUTTON && escapeButton >= buttonCount))
	{
		errno = EINVAL;
		return DS_MESSAGE_BOX_NO_BUTTON;
	}

	return application->showMessageBoxFunc(application, parentWindow, type, title, message, buttons,
		buttonCount, enterButton, escapeButton);
}

bool dsApplication_quit(dsApplication* application, int exitCode)
{
	if (!application || !application->quitFunc)
	{
		errno = EINVAL;
		return false;
	}

	application->quitFunc(application, exitCode);
	return true;
}

bool dsApplication_addCustomEvent(dsApplication* application, const dsCustomEvent* event)
{
	if (!application || !application->addCustomEventFunc || !event)
	{
		errno = EINVAL;
		return false;
	}

	return application->addCustomEventFunc(application, event);
}

dsSystemPowerState dsApplication_getPowerState(
	int* outRemainingTime, int* outBatteryPercent, const dsApplication* application)
{
	if (!application || !application->getPowerStateFunc)
	{
		if (outRemainingTime)
			*outRemainingTime = -1;
		if (outBatteryPercent)
			*outBatteryPercent = -1;
		return dsSystemPowerState_Unknown;
	}

	return application->getPowerStateFunc(outRemainingTime, outBatteryPercent, application);
}

const dsDisplayInfo* dsApplication_findDisplay(const dsApplication* application, uint64_t displayID)
{
	if (!application)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < application->displayCount; ++i)
	{
		const dsDisplayInfo* display = application->displays[i];
		if (display->id == displayID)
			return display;
	}

	errno = ENOTFOUND;
	return NULL;
}

uint32_t dsApplication_adjustWindowSize(
	const dsApplication* application, const dsDisplayInfo* display, uint32_t size)
{
	if (!application)
		return size;

	if (!display)
		display = application->primaryDisplay;
	if (!display)
		return size;

	return (uint32_t)dsRoundf((float)size*display->scale);
}

dsCursor dsApplication_getCursor(const dsApplication* application)
{
	if (!application || !application->getCursorFunc)
		return dsCursor_Arrow;

	return application->getCursorFunc(application);
}

bool dsApplication_setCursor(dsApplication* application, dsCursor cursor)
{
	if (!application || !application->setCursorFunc)
	{
		errno = EINVAL;
		return false;
	}

	return application->setCursorFunc(application, cursor);
}

bool dsApplication_getCursorHidden(const dsApplication* application)
{
	if (!application || !application->getCursorHiddenFunc)
		return false;

	return application->getCursorHiddenFunc(application);
}

bool dsApplication_setCursorHidden(dsApplication* application, bool hidden)
{
	if (!application || !application->setCursorHiddenFunc)
	{
		errno = EINVAL;
		return false;
	}

	return application->setCursorHiddenFunc(application, hidden);
}

bool dsApplication_isKeyPressed(const dsApplication* application, dsKeyCode key)
{
	if (!application || !application->isKeyPressedFunc)
		return false;

	return application->isKeyPressedFunc(application, key);
}

dsKeyModifier dsApplication_getKeyModifiers(const dsApplication* application)
{
	if (!application || !application->getKeyModifiersFunc)
		return dsKeyModifier_None;

	return application->getKeyModifiersFunc(application);
}

bool dsApplication_getMousePosition(dsVector2f* outPosition, const dsApplication* application)
{
	if (!outPosition || !application || !application->getMousePositionFunc)
	{
		errno = EINVAL;
		return false;
	}

	return application->getMousePositionFunc(outPosition, application);
}

bool dsApplication_setMousePosition(
	dsApplication* application, dsWindow* window, const dsVector2f* position)
{
	if (!application || !application->setMousePositionFunc || !position)
	{
		errno = EINVAL;
		return false;
	}

	return application->setMousePositionFunc(application, window, position);
}

uint32_t dsApplication_getPressedMouseButtons(const dsApplication* application)
{
	if (!application || !application->getPressedMouseButtonsFunc)
		return 0;

	return application->getPressedMouseButtonsFunc(application);
}

dsWindow* dsApplication_getFocusWindow(const dsApplication* application)
{
	if (!application || !application->getFocusWindowFunc)
		return NULL;

	return application->getFocusWindowFunc(application);
}

bool dsApplication_dispatchEvent(dsApplication* application, const dsEvent* event)
{
	if (!application || !event)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < application->eventResponderCount; ++i)
	{
		const dsEventResponder* responder = application->eventResponders + i;
		DS_ASSERT(responder->eventFunc);
		if (!responder->eventFunc(application, event, responder->userData))
			break;
	}

	if (event->type == dsAppEventType_Custom && event->custom.cleanupFunc)
		event->custom.cleanupFunc(event->custom.eventID, event->custom.userData);

	return true;
}

#if DS_ANDROID
bool dsApplication_requestAndroidPermission(dsApplication* application,
	const char* permission, dsHandleAndroidPermissionResultFunction resultFunc, void* userData)
{
	if (!application || !application->requestAndroidPermissionFunc || !permission || !resultFunc)
	{
		errno = EINVAL;
		return false;
	}

	return application->requestAndroidPermissionFunc(application, permission, resultFunc, userData);
}
#endif

void dsApplication_destroy(dsApplication* application)
{
	if (!application)
		return;

	// Expected to always be set by the application, otherwise we can't cleanly exit.
	DS_ASSERT(application->destroyFunc);
	application->destroyFunc(application);
}

bool dsApplication_initialize(dsApplication* application, dsAllocator* allocator)
{
	if (!application)
	{
		errno = EINVAL;
		return false;
	}

	memset(application, 0, sizeof(*application));
	application->allocator = dsAllocator_keepPointer(allocator);
	application->timer = dsTimer_create();
	if (!dsUniqueNameID_isInitialized())
	{
		if (!dsUniqueNameID_initialize(allocator, DS_DEFAULT_INITIAL_UNIQUE_NAME_ID_LIMIT))
			return false;
		application->uniqueNameIDInitialized = true;
	}
	if (!dsLog_getFunction())
		dsLog_setFunction(application, &applicationLogWrapper);
	return true;
}

bool dsApplication_addGameInput(dsApplication* application, dsGameInput* gameInput)
{
	if (!application || !gameInput || gameInput->application != application)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < application->gameInputCount; ++i)
	{
		if (application->gameInputs[i] == gameInput)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_APPLICATION_LOG_TAG, "GameInput has already been added.");
			return false;
		}
	}

	uint32_t index = application->gameInputCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(application->allocator, application->gameInputs,
			application->gameInputCount, application->gameInputCapacity, 1))
	{
		return false;
	}

	application->gameInputs[index] = gameInput;
	return true;
}

bool dsApplication_removeGameInput(dsApplication* application, dsGameInput* gameInput)
{
	if (!application || !gameInput)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < application->gameInputCount; ++i)
	{
		if (application->gameInputs[i] == gameInput)
		{
			memmove(application->gameInputs + i, application->gameInputs + i + 1,
				sizeof(dsGameInput*)*(application->gameInputCount - i - 1));
			--application->gameInputCount;
			return true;
		}
	}

	errno = ENOTFOUND;
	return false;
}

bool dsApplication_addMotionSensor(dsApplication* application, dsMotionSensor* sensor)
{
	if (!application || !sensor || sensor->application != application)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < application->motionSensorCount; ++i)
	{
		if (application->motionSensors[i] == sensor)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_APPLICATION_LOG_TAG, "MotionSensor has already been added.");
			return false;
		}
	}

	uint32_t index = application->motionSensorCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(application->allocator, application->motionSensors,
			application->motionSensorCount, application->motionSensorCapacity, 1))
	{
		return false;
	}

	application->motionSensors[index] = sensor;
	return true;
}

bool dsApplication_removeMotionSensor(dsApplication* application, dsMotionSensor* sensor)
{
	if (!application || !sensor)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < application->motionSensorCount; ++i)
	{
		if (application->motionSensors[i] == sensor)
		{
			memmove(application->motionSensors + i, application->motionSensors + i + 1,
				sizeof(dsMotionSensor*)*(application->motionSensorCount - i - 1));
			--application->motionSensorCount;
			return true;
		}
	}

	errno = ENOTFOUND;
	return false;
}

void dsApplication_shutdown(dsApplication* application)
{
	if (!application)
		return;

	for (uint32_t i = 0; i < application->windowResponderCount; ++i)
	{
		dsWindowResponder* responder = application->windowResponders + i;
		if (responder->destroyUserDataFunc)
			responder->destroyUserDataFunc(responder->userData);
	}

	for (uint32_t i = 0; i < application->eventResponderCount; ++i)
	{
		dsEventResponder* responder = application->eventResponders + i;
		if (responder->destroyUserDataFunc)
			responder->destroyUserDataFunc(responder->userData);
	}

	if (application->destroyUpdateUserDataFunc)
		application->destroyUpdateUserDataFunc(application->updateUserData);
	if (application->destroyFinishFrameUserDataFunc)
		application->destroyFinishFrameUserDataFunc(application->finishFrameUserData);
	if (application->destroyUserDataFunc)
		application->destroyUserDataFunc(application->userData);

	DS_VERIFY(dsAllocator_free(application->allocator, application->windowResponders));
	DS_VERIFY(dsAllocator_free(application->allocator, application->eventResponders));
	DS_VERIFY(dsAllocator_free(application->allocator, application->windows));
	DS_VERIFY(dsAllocator_free(application->allocator, application->gameInputs));
	DS_VERIFY(dsAllocator_free(application->allocator, application->motionSensors));

	if (dsLog_getFunction() == &applicationLogWrapper)
		dsLog_clearFunction();
	if (application->uniqueNameIDInitialized)
		dsUniqueNameID_shutdown();
}
