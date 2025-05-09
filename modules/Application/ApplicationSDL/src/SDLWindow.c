/*
 * Copyright 2017-2024 Aaron Barany
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

#include "SDLWindow.h"

#include "SDLShared.h"
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/RenderSurface.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>

#include <SDL_syswm.h>
#include <string.h>

#if DS_APPLE
void* dsSDLWindow_getUsableWindowHandle(void* window);
void dsSDLWindow_releaseUsableWindowHandle(void* handle);
#elif DS_ANDROID
#include <android/native_window.h>
#endif

static void getSdlPosition(int* outX, int* outY, const dsVector2i* position, bool center)
{
	if (center)
	{
		if (position)
			*outX = *outY = SDL_WINDOWPOS_CENTERED_DISPLAY(position->x);
		else
			*outX = *outY = SDL_WINDOWPOS_CENTERED;
	}
	else if (position)
	{
		*outX = position->x;
		*outY = position->y;
	}
	else
		*outX = *outY = SDL_WINDOWPOS_UNDEFINED;
}

bool dsSDLWindow_createComponents(dsWindow* window, const char* title, const char* surfaceName,
	const dsVector2i* position, uint32_t width, uint32_t height, dsWindowFlags flags)
{
	dsSDLWindow* sdlWindow = (dsSDLWindow*)window;
	dsApplication* application = window->application;

	unsigned int sdlFlags = SDL_WINDOW_ALLOW_HIGHDPI;
	int x, y;
	getSdlPosition(&x, &y, position, (flags & dsWindowFlags_Center) != 0);

	uint32_t platformID = application->renderer->platformID;
	if (flags & dsWindowFlags_Hidden)
		sdlFlags |= SDL_WINDOW_HIDDEN;
	if (flags & dsWindowFlags_Resizeable)
		sdlFlags |= SDL_WINDOW_RESIZABLE;
	if (flags & dsWindowFlags_Minimized)
		sdlFlags |= SDL_WINDOW_MINIMIZED;
	if (flags & dsWindowFlags_Maximized)
		sdlFlags |= SDL_WINDOW_MAXIMIZED;
	if (flags & dsWindowFlags_GrabInput)
		sdlFlags |= SDL_WINDOW_INPUT_GRABBED;
	if (platformID == DS_GLX_RENDERER_PLATFORM_ID || platformID == DS_WGL_RENDERER_PLATFORM_ID)
		sdlFlags |= SDL_WINDOW_OPENGL;

	if (!dsRenderSurface_destroy(window->surface))
		return false;
	window->surface = NULL;

	if (sdlWindow->sdlWindow)
	{
		SDL_DestroyWindow(sdlWindow->sdlWindow);
		dsRenderer_restoreGlobalState(application->renderer);
		sdlWindow->sdlWindow = NULL;
	}

	SDL_Window* internalWindow = SDL_CreateWindow(title, x, y, width, height, sdlFlags);
	if (!internalWindow)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_APPLICATION_SDL_LOG_TAG, "Couldn't create window: %s", SDL_GetError());
		return false;
	}

	window->surface = NULL;
	sdlWindow->surfaceName = surfaceName;
	sdlWindow->sdlWindow = internalWindow;
	sdlWindow->samples = application->renderer->surfaceSamples;
	if (!(flags & dsWindowFlags_DelaySurfaceCreate) &&
		!dsSDLWindow_createSurfaceInternal(window, surfaceName))
	{
		DS_LOG_ERROR(DS_APPLICATION_SDL_LOG_TAG, "Couldn't create render surface.");
		SDL_DestroyWindow(internalWindow);
		dsRenderer_restoreGlobalState(application->renderer);
		sdlWindow->sdlWindow = NULL;
		return false;
	}

	window->style = dsWindowStyle_Normal;
	DS_ASSERT(application->displayCount > 0);
	window->displayMode = application->displays[0].displayModes[
		application->displays[0].defaultMode];

	dsSDLWindow_getSize(&sdlWindow->curWidth, &sdlWindow->curHeight, application, window);
	sdlWindow->curSurfaceWidth = 0;
	sdlWindow->curSurfaceHeight = 0;
	sdlWindow->curSurfaceRotation = dsRenderSurfaceRotation_0;
	return true;
}

void dsSDLWindow_destroyComponents(dsWindow* window)
{
	dsSDLWindow* sdlWindow = (dsSDLWindow*)window;
	dsApplication* application = window->application;

	dsRenderSurface_destroy(window->surface);
	window->surface = NULL;
	if (sdlWindow->sdlWindow)
	{
		SDL_DestroyWindow(sdlWindow->sdlWindow);
		dsRenderer_restoreGlobalState(application->renderer);
		sdlWindow->sdlWindow = NULL;
	}
}

bool dsSDLWindow_createSurfaceInternal(dsWindow* window, const char* surfaceName)
{
	dsSDLWindow* sdlWindow = (dsSDLWindow*)window;
	dsApplication* application = window->application;
	dsRenderer_restoreGlobalState(application->renderer);

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	DS_VERIFY(SDL_GetWindowWMInfo(sdlWindow->sdlWindow, &info));
	void* displayHandle = NULL;
	void* windowHandle = NULL;
	switch (info.subsystem)
	{
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
		case SDL_SYSWM_WINDOWS:
			displayHandle = NULL;
			windowHandle = info.info.win.window;
			break;
#endif
#if defined(SDL_VIDEO_DRIVER_X11)
		case SDL_SYSWM_X11:
			displayHandle = info.info.x11.display;
			windowHandle = (void*)info.info.x11.window;
			break;
#endif
#if defined(SDL_VIDEO_DRIVER_WAYLAND)
		case SDL_SYSWM_WAYLAND:
			displayHandle = info.info.wl.display;
			if (application->renderer->rendererID == DS_GL_RENDERER_ID ||
				application->renderer->rendererID == DS_GLES_RENDERER_ID)
			{
				windowHandle = (void*)info.info.wl.egl_window;
			}
			else
				windowHandle = (void*)info.info.wl.surface;
			break;
#endif
#if defined(SDL_VIDEO_DRIVER_COCOA)
		case SDL_SYSWM_COCOA:
			displayHandle = NULL;
			windowHandle = dsSDLWindow_getUsableWindowHandle(info.info.cocoa.window);
			break;
#endif
#if defined(SDL_VIDEO_DRIVER_UIKIT)
		case SDL_SYSWM_UIKIT:
			displayHandle = NULL;
			windowHandle = dsSDLWindow_getUsableWindowHandle(info.info.uikit.window);
			break;
#endif
#if defined(SDL_VIDEO_DRIVER_ANDROID)
		case SDL_SYSWM_ANDROID:
			displayHandle = NULL;
			windowHandle = info.info.android.window;
			break;
#endif
		default:
			errno = EPERM;
			DS_LOG_ERROR(DS_APPLICATION_SDL_LOG_TAG, "Unsupported video driver.");
			return false;
	}

	window->surface = dsRenderSurface_create(application->renderer, window->allocator,
		surfaceName, displayHandle, windowHandle, dsRenderSurfaceType_Window,
		sdlWindow->renderSurfaceUsage, sdlWindow->curWidth, sdlWindow->curHeight);

	if (window->surface)
	{
		sdlWindow->curSurfaceWidth = window->surface->width;
		sdlWindow->curSurfaceHeight = window->surface->height;
		sdlWindow->curSurfaceRotation = window->surface->rotation;
	}

#if defined(SDL_VIDEO_DRIVER_COCOA)
	if (info.subsystem == SDL_SYSWM_COCOA)
		dsSDLWindow_releaseUsableWindowHandle(windowHandle);
#endif

	return window->surface != NULL;
}

dsWindow* dsSDLWindow_create(dsApplication* application, dsAllocator* allocator,
	const char* title, const char* surfaceName, const dsVector2i* position, uint32_t width,
	uint32_t height, dsWindowFlags flags, dsRenderSurfaceUsage renderSurfaceUsage)
{
	dsSDLWindow* window = DS_ALLOCATE_OBJECT(allocator, dsSDLWindow);
	if (!window)
		return NULL;

	memset(window, 0, sizeof(dsSDLWindow));
	dsWindow* baseWindow = (dsWindow*)window;
	baseWindow->application = application;
	baseWindow->allocator = dsAllocator_keepPointer(allocator);

	if (!dsSDLWindow_createComponents(baseWindow, title, surfaceName, position, width, height,
			flags))
	{
		DS_VERIFY(dsAllocator_free(allocator, window));
		return NULL;
	}

	// Ensure that the SDL window's display mode matches what we expect.
	dsSDLWindow_setDisplayMode(application, baseWindow, &baseWindow->displayMode);

	window->curPosition.x = 0;
	window->curPosition.y = 0;
	window->curFlags = 0;
	window->renderSurfaceUsage = renderSurfaceUsage;
	window->hasFocus = false;

	return baseWindow;
}

bool dsSDLWindow_createSurface(dsApplication* application, dsWindow* window)
{
	DS_UNUSED(application);
	if (window->surface)
		return true;

	dsSDLWindow* sdlWindow = (dsSDLWindow*)window;
	return dsSDLWindow_createSurfaceInternal(window, sdlWindow->surfaceName);
}

dsWindow* dsSDLWindow_getFocusWindow(const dsApplication* application)
{
	SDL_Window* sdlWindow = SDL_GetGrabbedWindow();
	if (!sdlWindow)
		sdlWindow = SDL_GetKeyboardFocus();
	if (!sdlWindow)
		return NULL;

	for (uint32_t i = 0; i < application->windowCount; ++i)
	{
		if (((dsSDLWindow*)application->windows[i])->sdlWindow == sdlWindow)
			return application->windows[i];
	}

	return NULL;
}

bool dsSDLWindow_setTitle(dsApplication* application, dsWindow* window, const char* title)
{
	DS_UNUSED(application);
	SDL_SetWindowTitle(((dsSDLWindow*)window)->sdlWindow, title);
	window->title = title;
	return true;
}

bool dsSDLWindow_setDisplayMode(dsApplication* application, dsWindow* window,
	const dsDisplayMode* displayMode)
{
	DS_UNUSED(application);

	SDL_DisplayMode defaultMode;
	DS_VERIFY(SDL_GetDesktopDisplayMode(displayMode->displayIndex, &defaultMode) == 0);
	uint32_t displayModeCount = SDL_GetNumDisplayModes(displayMode->displayIndex);

	SDL_DisplayMode foundMode;
	bool found = false;
	for (uint32_t i = 0; i < displayModeCount; ++i)
	{
		DS_VERIFY(SDL_GetDisplayMode(displayMode->displayIndex, i, &foundMode) == 0);
		if (foundMode.format == defaultMode.format && foundMode.w == (int)displayMode->width &&
			foundMode.h == (int)displayMode->height &&
			foundMode.refresh_rate == (int)displayMode->refreshRate)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_APPLICATION_SDL_LOG_TAG, "Invalid display mode.");
		return false;
	}

	if (SDL_SetWindowDisplayMode(((dsSDLWindow*)window)->sdlWindow, &foundMode) != 0)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_APPLICATION_SDL_LOG_TAG, "Couldn't set window display mode: %s",
			SDL_GetError());
		return false;
	}

	window->displayMode = *displayMode;
	return true;
}

bool dsSDLWindow_resize(dsApplication* application, dsWindow* window, uint32_t width,
	uint32_t height)
{
	DS_UNUSED(application);
	SDL_SetWindowSize(((dsSDLWindow*)window)->sdlWindow, width, height);
	return true;
}

bool dsSDLWindow_getSize(uint32_t* outWidth, uint32_t* outHeight, const dsApplication* application,
	const dsWindow* window)
{
	DS_UNUSED(application);
	SDL_Window* sdlWindow = ((const dsSDLWindow*)window)->sdlWindow;

	SDL_GetWindowSize(sdlWindow, (int*)outWidth, (int*)outHeight);
	return true;
}

bool dsSDLWindow_getPixelSize(uint32_t* outWidth, uint32_t* outHeight,
	const dsApplication* application, const dsWindow* window)
{
	DS_UNUSED(application);
	if (!window->surface)
		return false;

	if (outWidth)
		*outWidth = window->surface->width;
	if (outHeight)
		*outHeight = window->surface->height;
	return true;
}

bool dsSDLWindow_setStyle(dsApplication* application, dsWindow* window, dsWindowStyle style)
{
	DS_UNUSED(application);
	Uint32 flags = 0;
	switch (style)
	{
		case dsWindowStyle_Normal:
			flags = 0;
			break;
		case dsWindowStyle_FullScreen:
			flags = SDL_WINDOW_FULLSCREEN;
			break;
		case dsWindowStyle_FullScreenBorderless:
			flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
			break;
	}

	if (SDL_SetWindowFullscreen(((dsSDLWindow*)window)->sdlWindow, flags) != 0)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_APPLICATION_SDL_LOG_TAG, "Couldn't set window display style: %s",
			SDL_GetError());
		return false;
	}

	return true;
}

bool dsSDLWindow_getPosition(dsVector2i* outPosition, const dsApplication* application,
	const dsWindow* window)
{
	DS_UNUSED(application);
	SDL_GetWindowPosition(((dsSDLWindow*)window)->sdlWindow, &outPosition->x, &outPosition->y);
	return true;
}

bool dsSDLWindow_setPosition(dsApplication* application, dsWindow* window,
	const dsVector2i* position, bool center)
{
	DS_UNUSED(application);
	int x, y;
	getSdlPosition(&x, &y, position, center);
	SDL_SetWindowPosition(((dsSDLWindow*)window)->sdlWindow, x, y);
	return true;
}

bool dsSDLWindow_getHidden(const dsApplication* application, const dsWindow* window)
{
	DS_UNUSED(application);
	return (SDL_GetWindowFlags(((const dsSDLWindow*)window)->sdlWindow) & SDL_WINDOW_HIDDEN) != 0;
}

bool dsSDLWindow_setHidden(dsApplication* application, dsWindow* window, bool hidden)
{
	DS_UNUSED(application);
	if (hidden)
		SDL_HideWindow(((dsSDLWindow*)window)->sdlWindow);
	else
		SDL_ShowWindow(((dsSDLWindow*)window)->sdlWindow);
	return true;
}

bool dsSDLWindow_getMinimized(const dsApplication* application, const dsWindow* window)
{
	DS_UNUSED(application);
	return (SDL_GetWindowFlags(((const dsSDLWindow*)window)->sdlWindow) & SDL_WINDOW_MINIMIZED)
		!= 0;
}

bool dsSDLWindow_getMaximized(const dsApplication* application, const dsWindow* window)
{
	DS_UNUSED(application);
	return (SDL_GetWindowFlags(((const dsSDLWindow*)window)->sdlWindow) & SDL_WINDOW_MAXIMIZED)
		!= 0;
}

bool dsSDLWindow_minimize(dsApplication* application, dsWindow* window)
{
	DS_UNUSED(application);
	SDL_MinimizeWindow(((dsSDLWindow*)window)->sdlWindow);
	return true;
}

bool dsSDLWindow_maximize(dsApplication* application, dsWindow* window)
{
	DS_UNUSED(application);
	SDL_MaximizeWindow(((dsSDLWindow*)window)->sdlWindow);
	return true;
}

bool dsSDLWindow_restore(dsApplication* application, dsWindow* window)
{
	DS_UNUSED(application);
	SDL_RestoreWindow(((dsSDLWindow*)window)->sdlWindow);
	return true;
}

bool dsSDLWindow_getGrabbedInput(const dsApplication* application,
	const dsWindow* window)
{
	DS_UNUSED(application);
	return (SDL_GetWindowFlags(((const dsSDLWindow*)window)->sdlWindow) & SDL_WINDOW_INPUT_GRABBED)
		!= 0;
}

bool dsSDLWindow_setGrabbedInput(dsApplication* application, dsWindow* window, bool grab)
{
	DS_UNUSED(application);
	SDL_SetWindowGrab(((dsSDLWindow*)window)->sdlWindow, (SDL_bool)grab);
	return true;
}

bool dsSDLWindow_raise(dsApplication* application, dsWindow* window)
{
	DS_UNUSED(application);
	SDL_RaiseWindow(((dsSDLWindow*)window)->sdlWindow);
	return true;
}

bool dsSDLWindow_destroy(dsApplication* application, dsWindow* window)
{
	DS_UNUSED(application);
	dsSDLWindow* sdlWindow = (dsSDLWindow*)window;
	if (!dsRenderSurface_destroy(window->surface))
		return false;

	if (sdlWindow->sdlWindow)
		SDL_DestroyWindow(sdlWindow->sdlWindow);

	// Handle cases like OpenGL where the window could be globally bound.
	dsRenderer_restoreGlobalState(application->renderer);

	if (window->destroyDrawUserDataFunc)
		window->destroyDrawUserDataFunc(window->drawUserData);
	if (window->destroyCloseUserDataFunc)
		window->destroyCloseUserDataFunc(window->closeUserData);

	if (window->allocator)
		DS_VERIFY(dsAllocator_free(window->allocator, window));
	return true;
}
