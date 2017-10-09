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

#include <DeepSea/Application/Window.h>

#include <DeepSea/Application/Application.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

dsWindow* dsWindow_create(dsApplication* application, dsAllocator* allocator, const char* title,
	const dsVector2i* position, uint32_t width, uint32_t height, unsigned int flags)
{
	if (!application || (!allocator && application->allocator) || !application->createWindowFunc ||
		!application->destroyWindowFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = application->allocator;

	if ((flags & dsWindowFlags_Center) && position &&
		(uint32_t)position->x >= application->displayCount)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_APPLICATION_LOG_TAG,
			"Attempting to place a window on a non-existant display.");
		return NULL;
	}

	dsWindow* window = application->createWindowFunc(application, allocator, title, position, width,
		height, flags);
	if (!window)
		return NULL;

	if (!dsApplication_addWindow(application, window))
	{
		application->destroyWindowFunc(application, window);
		return NULL;
	}

	return window;
}

bool dsWindow_setTtile(dsWindow* window, const char* title)
{
	if (!window || !window->application || !window->application->setWindowTitleFunc)
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

bool dsWindow_getSize(uint32_t* outWidth, uint32_t* outHeight, const dsWindow* window)
{
	if (!window || !window->application || !window->application->getWindowSizeFunc)
	{
		errno = EINVAL;
		return false;
	}

	const dsApplication* application = window->application;
	return application->getWindowSizeFunc(outWidth, outHeight, application, window);
}

bool dsWindow_setStyle(dsWindow* window, dsWindowStyle style)
{
	if (!window || !window->application || !window->application->setWindowStyleFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->setWindowStyleFunc(application, window, style);
}

bool dsWindow_getPosition(dsVector2i* outPosition, const dsWindow* window)
{
	if (!outPosition || !window || !window->application ||
		!window->application->getWindowPositionFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->getWindowPositionFunc(outPosition, application, window);
}

bool dsWindow_setPosition(dsWindow* window, const dsVector2i* position, bool center)
{
	if (!window || !window->application || !window->application->setWindowPositionFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsApplication* application = window->application;
	return application->setWindowPositionFunc(application, window, position, center);
}

bool dsWindow_getHidden(const dsWindow* window)
{
	if (!window || !window->application || !window->application->getWindowHiddenFunc)
		return false;

	const dsApplication* application = window->application;
	return application->getWindowHiddenFunc(application, window);
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

bool dsWindow_getMinimized(const dsWindow* window)
{
	if (!window || !window->application || !window->application->getWindowMinimizedFunc)
		return false;

	const dsApplication* application = window->application;
	return application->getWindowMinimizedFunc(application, window);
}

bool dsWindow_getMaximized(const dsWindow* window)
{
	if (!window || !window->application || !window->application->getWindowMaximizedFunc)
		return false;

	const dsApplication* application = window->application;
	return application->getWindowMaximizedFunc(application, window);
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

bool dsWindow_getGrabbedInput(const dsWindow* window)
{
	if (!window || !window->application || !window->application->getWindowGrabbedInputFunc)
		return false;

	const dsApplication* application = window->application;
	return application->getWindowGrabbedInputFunc(application, window);
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

bool dsWindow_destroy(dsWindow* window)
{
	if (!window || !window->application || !window->application->destroyWindowFunc)
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
