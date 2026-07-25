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

#include <DeepSea/Application/Window.h>

#include <DeepSea/Application/Application.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Geometry/AlignedBox2.h>

static bool hasDisplayMode(const dsApplication* application, const dsDisplayMode* displayMode)
{
	const dsDisplayInfo* foundDisplay = NULL;
	for (uint32_t i = 0; i < application->displayCount; ++i)
	{
		const dsDisplayInfo* display = application->displays[i];
		if (display->id == displayMode->displayID)
		{
			foundDisplay = display;
			break;
		}
	}

	if (foundDisplay)
	{
		for (uint32_t i = 0; i < foundDisplay->displayModeCount; ++i)
		{
			const dsDisplayMode* thisDisplayMode = foundDisplay->displayModes + i;
			if (thisDisplayMode->width == displayMode->width &&
				thisDisplayMode->height == displayMode->height &&
				thisDisplayMode->refreshRate == displayMode->refreshRate)
			{
				return true;
			}
		}
	}

	DS_LOG_ERROR(
		DS_APPLICATION_LOG_TAG, "Window doesn't contain a valid full-screen display mode.");
	errno = ENOTFOUND;
	return false;
}

dsWindow* dsWindow_create(dsApplication* application, dsAllocator* allocator, const char* title,
	const char* surfaceName, const dsWindowInitPosition* position, uint32_t width, uint32_t height,
	dsWindowFlags flags, dsRenderSurfaceUsage renderSurfaceUsage)
{
	if (!application || (!allocator && !application->allocator) || !application->createWindowFunc ||
		!application->destroyWindowFunc || !title || (position &&
		(position->type < dsWindowInitPositionType_Default ||
		position->type > dsWindowInitPositionType_DisplayFullScreenBorderless ||
		(position->type == dsWindowInitPositionType_DisplayFullScreen && !position->displayMode))))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = application->allocator;

	if (!surfaceName)
		surfaceName = title;

	dsWindow* window = application->createWindowFunc(application, allocator, title, surfaceName,
		position, width, height, flags, renderSurfaceUsage);
	if (!window)
		return NULL;

	if (!dsApplication_addWindow(application, window))
	{
		application->destroyWindowFunc(application, window);
		return NULL;
	}

	return window;
}

bool dsWindow_createSurface(dsWindow* window)
{
	if (!window || !window->application || !window->application->createWindowSurfaceFunc)
	{
		errno = EINVAL;
		return false;
	}
	else if (window->surface)
		return true;

	dsApplication* application = window->application;
	return application->createWindowSurfaceFunc(application, window);
}

bool dsWindow_setDrawFunction(dsWindow* window, dsDrawWindowFunction drawFunc, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc)
{
	if (!window)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return false;
	}

	if (window->destroyDrawUserDataFunc)
		window->destroyDrawUserDataFunc(window->drawUserData);
	window->drawFunc = drawFunc;
	window->drawUserData = userData;
	window->destroyDrawUserDataFunc = destroyUserDataFunc;
	return true;
}

bool dsWindow_setCloseFunction(dsWindow* window, dsInterceptCloseWindowFunction closeFunc,
	void* userData, dsDestroyUserDataFunction destroyUserDataFunc)
{
	if (!window)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return false;
	}

	if (window->destroyCloseUserDataFunc)
		window->destroyCloseUserDataFunc(window->closeUserData);
	window->closeFunc = closeFunc;
	window->closeUserData = userData;
	window->destroyCloseUserDataFunc = destroyUserDataFunc;
	return true;
}

bool dsWindow_setTitle(dsWindow* window, const char* title)
{
	if (!window || !title || !window->application || !window->application->setWindowTitleFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->setWindowTitleFunc(application, window, title);
}

bool dsWindow_setDisplayMode(dsWindow* window, const dsDisplayMode* displayMode)
{
	if (!window || !window->application || !window->application->setWindowDisplayModeFunc ||
		!displayMode)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	if (!hasDisplayMode(application, displayMode))
		return false;

	return application->setWindowDisplayModeFunc(application, window, displayMode);
}

bool dsWindow_resize(dsWindow* window, uint32_t width, uint32_t height)
{
	if (!window || !window->application || !window->application->resizeWindowFunc || width == 0 ||
		height == 0)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->resizeWindowFunc(application, window, width, height);
}

bool dsWindow_setStyle(dsWindow* window, dsWindowStyle style)
{
	if (!window || !window->application || !window->application->setWindowStyleFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	if (style == dsWindowStyle_FullScreen)
	{
		// May have been invalidated if display was disconnected.
		if (!hasDisplayMode(application, &window->displayMode))
			return false;
	}

	return application->setWindowStyleFunc(application, window, style);
}

bool dsWindow_setPosition(dsWindow* window, const dsVector2i* position)
{
	if (!window || !window->application || !window->application->setWindowPositionFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->setWindowPositionFunc(application, window, position);
}

bool dsWindow_center(dsWindow* window, const dsDisplayInfo* display)
{
	if (!window || !window->application || !window->application->centerWindowFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->centerWindowFunc(application, window, display);
}

bool dsWindow_setHidden(dsWindow* window, bool hidden)
{
	if (!window || !window->application || !window->application->setWindowHiddenFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->setWindowHiddenFunc(application, window, hidden);
}

bool dsWindow_minimize(dsWindow* window)
{
	if (!window || !window->application || !window->application->maximizeWindowFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->maximizeWindowFunc(application, window);
}

bool dsWindow_maximize(dsWindow* window)
{
	if (!window || !window->application || !window->application->minimizeWindowFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->minimizeWindowFunc(application, window);
}

bool dsWindow_restore(dsWindow* window)
{
	if (!window || !window->application || !window->application->restoreWindowFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->restoreWindowFunc(application, window);
}

bool dsWindow_setGrabbedInput(dsWindow* window, bool grab)
{
	if (!window || !window->application || !window->application->setWindowGrabbedInputFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->setWindowGrabbedInputFunc(application, window, grab);
}

bool dsWindow_raise(dsWindow* window)
{
	if (!window || !window->application || !window->application->raiseWindowFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->raiseWindowFunc(application, window);
}

bool dsWindow_beginTextInput(
	dsWindow* window, dsWindowTextInputType inputType, dsWindowTextInputFlags inputFlags)
{
	if (!window || !window->application || !window->application->beginTextInputFunc ||
		!window->application->endTextInputFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->beginTextInputFunc(application, window, inputType, inputFlags);
}

bool dsApplication_endTextInput(dsWindow* window)
{
	if (!window || !window->application || !window->application->endTextInputFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->endTextInputFunc(application, window);
}

bool dsApplication_setTextInputRect(
	dsWindow* window, const dsAlignedBox2i* bounds, uint32_t cursorOffset)
{
	if (!window || !window->application->setTextInputAreaFunc || !bounds ||
		!dsAlignedBox2_isValid(*bounds) || cursorOffset > (uint32_t)(bounds->max.x - bounds->min.x))
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->setTextInputAreaFunc(application, window, bounds, cursorOffset);
}

bool dsWindow_destroy(dsWindow* window)
{
	if (!window)
		return true;

	if (!window->application || !window->application->destroyWindowFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	int prevErrno = errno;
	if (!dsApplication_removeWindow(application, window))
	{
		// Don't consider not found to be an error for destroying a window.
		if (errno != ENOTFOUND)
			return false;

		errno = prevErrno;
	}

	return application->destroyWindowFunc(application, window);
}
