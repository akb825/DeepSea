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

#include <DeepSea/Application/Application.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <stdlib.h>
#include <string.h>

static uint32_t nextWindowResponderId;
static uint32_t nextEventResponderId;

static int compareEventResponders(const void* left, const void* right)
{
	return ((dsEventResponder*)left)->priority - ((dsEventResponder*)right)->priority;
}

uint32_t dsApplication_addWindowResponder(dsApplication* application,
	const dsWindowResponder* responder)
{
	if (!application || !responder)
	{
		errno = EINVAL;
		return 0;
	}

	uint32_t index = application->windowResponderCount;
	if (!dsResizeableArray_add(application->allocator, (void**)&application->windowResponders,
		&application->windowResponderCount, &application->windowResponderCapacity,
		sizeof(dsWindowResponder), 1))
	{
		return 0;
	}

	uint32_t id = ++nextWindowResponderId;
	application->windowResponders[index] = *responder;
	application->windowResponders[index].responderId = id;
	return id;
}

bool dsApplication_removeWindowResponder(dsApplication* application, uint32_t responderId)
{
	if (!application || responderId == 0)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < application->windowResponderCount; ++i)
	{
		if (application->windowResponders[i].responderId == responderId)
		{
			memmove(application->windowResponders + i, application->windowResponders + i + 1,
				sizeof(dsWindowResponder)*(application->windowResponderCount - i - 1));
			return true;
		}
	}

	errno = ENOTFOUND;
	return false;
}

uint32_t dsApplication_addEventResponder(dsApplication* application, const dsEventResponder* responder)
{
	if (!application || !responder || !responder->eventFunc)
	{
		errno = EINVAL;
		return 0;
	}

	uint32_t index = application->eventResponderCount;
	if (!dsResizeableArray_add(application->allocator, (void**)&application->eventResponders,
		&application->eventResponderCount, &application->eventResponderCapacity,
		sizeof(dsEventResponder), 1))
	{
		return 0;
	}

	uint32_t id = ++nextEventResponderId;
	application->eventResponders[index] = *responder;
	application->eventResponders[index].responderId = id;

	qsort(application->eventResponders, application->eventResponderCount, sizeof(dsEventResponder),
		&compareEventResponders);
	return id;
}

bool dsApplication_removeEventResponder(dsApplication* application, uint32_t responderId)
{
	if (!application || responderId == 0)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < application->eventResponderCount; ++i)
	{
		if (application->eventResponders[i].responderId == responderId)
		{
			memmove(application->eventResponders + i, application->eventResponders + i + 1,
				sizeof(dsEventResponder)*(application->eventResponderCount - i - 1));
			return true;
		}
	}

	errno = ENOTFOUND;
	return false;
}

bool dsApplication_setUpdateFunction(dsApplication* application,
	dsUpdateApplicationFunction function, void* userData)
{
	if (!application)
	{
		errno = EINVAL;
		return false;
	}

	application->updateFunc = function;
	application->updateUserData = userData;
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
	if (!dsResizeableArray_add(application->allocator, (void**)&application->windows,
		&application->windowCount, &application->windowCapacity, sizeof(dsWindow*), 1))
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
			return true;
		}
	}

	errno = ENOTFOUND;
	return false;
}

bool dsApplication_addCustomEvent(dsApplication* application, dsWindow* window,
	const dsCustomEvent* event)
{
	if (!application || !application->addCustomEventFunc || !event)
	{
		errno = EINVAL;
		return false;
	}

	return application->addCustomEventFunc(application, window, event);
}

uint32_t dsApplication_showMessageBox(dsApplication* application, dsWindow* parentWindow,
	dsMessageBoxType type, const char* title, const char* message, const char** buttons,
	uint32_t buttonCount, uint32_t enterButton, uint32_t escapeButton)
{
	if (!application || !application->showMessageBoxFunc || !title || !message || !buttons ||
		buttonCount == 0 ||
		(enterButton != DS_MESSAGE_BOX_NO_BUTTON && enterButton >= buttonCount) ||
		(escapeButton != DS_MESSAGE_BOX_NO_BUTTON && escapeButton >= buttonCount))
	{
		errno = EINVAL;
		return DS_MESSAGE_BOX_NO_BUTTON;
	}

	return application->showMessageBoxFunc(application, parentWindow, type, title, message, buttons,
		buttonCount, enterButton, escapeButton);
}

int dsApplication_run(dsApplication* application)
{
	if (!application || !application->runFunc)
		return -1;

	return application->runFunc(application);
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

bool dsApplication_beginTextInput(dsApplication* application)
{
	if (!application || !application->beginTextInputFunc || !application->endTextInputFunc)
	{
		errno = EINVAL;
		return false;
	}

	return application->beginTextInputFunc(application);
}

bool dsApplication_endTextInput(dsApplication* application)
{
	if (!application || !application->endTextInputFunc)
	{
		errno = EINVAL;
		return false;
	}

	return application->endTextInputFunc(application);
}

bool dsApplication_setTextInputRect(dsApplication* application, const dsAlignedBox2i* rect)
{
	if (!application || !application->setTextInputRectFunc || !rect)
	{
		errno = EINVAL;
		return false;
	}

	return application->setTextInputRectFunc(application, rect);
}

bool dsApplication_getMousePosition(dsVector2i* outPosition, const dsApplication* application)
{
	if (!outPosition || !application || !application->getMousePositionFunc)
	{
		errno = EINVAL;
		return false;
	}

	return application->getMousePositionFunc(outPosition, application);
}

bool dsApplication_setMousePosition(dsApplication* application, dsWindow* window,
	const dsVector2i* position)
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

bool dsApplication_dispatchEvent(dsApplication* application, dsWindow* window,
	const dsEvent* event)
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
		if (!responder->eventFunc(application, window, event, responder->userData))
			break;
	}

	if (event->type == dsEventType_Custom && event->custom.cleanupFunc)
		event->custom.cleanupFunc(event->custom.eventId, event->custom.userData);

	return true;
}

bool dsApplication_initialize(dsApplication* application)
{
	if (!application)
	{
		errno = EINVAL;
		return false;
	}

	memset(application, 0, sizeof(*application));
	return true;
}

void dsApplication_shutdown(dsApplication* application)
{
	if (!application)
		return;

	if (application->windowResponders)
		DS_VERIFY(dsAllocator_free(application->allocator, application->windowResponders));
	if (application->eventResponders)
		DS_VERIFY(dsAllocator_free(application->allocator, application->eventResponders));
	if (application->windows)
		DS_VERIFY(dsAllocator_free(application->allocator, application->windows));
}
