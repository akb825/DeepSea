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

#include "SDLWindow.h"

#include "SDLShared.h"

#include <DeepSea/Application/Application.h>

#include <DeepSea/ApplicationSDL/Types.h>

#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/RenderSurface.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>

#include <string.h>

#if DS_APPLE
void* dsSDLWindow_getUsableWindowHandle(void* window);
void dsSDLWindow_releaseUsableWindowHandle(void* handle);
#elif DS_ANDROID
#include <android/native_window.h>
#endif

static dsWindowStyle getSdlPosition(dsVector2i* outSdlPosition,
	const dsWindowInitPosition* position, const dsDisplayInfo* fallbackDisplay)
{
	if (!position || position->type == dsWindowInitPositionType_Default)
	{
#if DS_ANDROID || DS_IOS
		// Expect fullscreen by default on mobile platforms.
		if (fallbackDisplay)
		{
			outSdlPosition->x = outSdlPosition->y =
				SDL_WINDOWPOS_CENTERED_DISPLAY((uint32_t)fallbackDisplay->id);
		}
		else
			outSdlPosition->x = outSdlPosition->y = SDL_WINDOWPOS_CENTERED;
		return dsWindowStyle_FullScreenBorderless;
#else
		outSdlPosition->x = outSdlPosition->y = SDL_WINDOWPOS_UNDEFINED;
		return dsWindowStyle_Normal;
#endif
	}

	switch (position->type)
	{
		case dsWindowInitPositionType_Location:
			outSdlPosition->x = position->position.x;
			outSdlPosition->y = position->position.y;
			return dsWindowStyle_Normal;
		case dsWindowInitPositionType_DisplayCenter:
		case dsWindowInitPositionType_DisplayFullScreenBorderless:
		{
			const dsDisplayInfo* display = position->display;
			if (!display)
				display = fallbackDisplay;

			if (display)
			{
				outSdlPosition->x = outSdlPosition->y =
					SDL_WINDOWPOS_CENTERED_DISPLAY((uint32_t)display->id);
			}
			else
				outSdlPosition->x = outSdlPosition->y = SDL_WINDOWPOS_CENTERED;
			return position->type == dsWindowInitPositionType_DisplayFullScreenBorderless ?
				dsWindowStyle_FullScreenBorderless : dsWindowStyle_Normal;
		}
		case dsWindowInitPositionType_DisplayFullScreen:
			outSdlPosition->x = outSdlPosition->y = SDL_WINDOWPOS_CENTERED_DISPLAY(
				(uint32_t)position->displayMode->displayID);
			return dsWindowStyle_FullScreen;
		default:
			DS_ASSERT(false);
			// Should never happen, but silence potential uninitialized warnings.
			outSdlPosition->x = outSdlPosition->y = SDL_WINDOWPOS_UNDEFINED;
			return dsWindowStyle_Normal;
	}
}

static bool findSDLDisplayMode(SDL_DisplayMode* outMode, const dsDisplayMode* displayMode)
{
	const SDL_DisplayMode* defaultMode = SDL_GetDesktopDisplayMode(
		(uint32_t)displayMode->displayID);
	if (!defaultMode)
	{
		DS_LOG_ERROR_F(
			DS_APPLICATION_SDL_LOG_TAG, "Couldn't get default display mode: %s", SDL_GetError());
		errno = EPERM;
		return false;
	}

	int displayModeCount;
	SDL_DisplayMode** displayModes = SDL_GetFullscreenDisplayModes(
		(uint32_t)displayMode->displayID, &displayModeCount);
	if (!defaultMode)
	{
		DS_LOG_ERROR_F(
			DS_APPLICATION_SDL_LOG_TAG, "Couldn't get display modes: %s", SDL_GetError());
		errno = EPERM;
		return false;
	}

	const SDL_DisplayMode* foundMode = NULL;
	for (int i = 0; i < displayModeCount; ++i)
	{
		const SDL_DisplayMode* candidate = displayModes[i];
		if (candidate->format == defaultMode->format && candidate->w == (int)displayMode->width &&
			candidate->h == (int)displayMode->height &&
			candidate->refresh_rate == displayMode->refreshRate)
		{
			foundMode = candidate;
			break;
		}
	}

	SDL_free(displayModes);
	if (!foundMode)
	{
		DS_LOG_ERROR(DS_APPLICATION_SDL_LOG_TAG, "Invalid display mode.");
		errno = EINVAL;
		return false;
	}

	*outMode = *foundMode;
	return true;
}

bool dsSDLWindow_createComponents(dsWindow* window, const dsVector2i* position, uint32_t width,
	uint32_t height, dsWindowFlags flags)
{
	DS_ASSERT(position);
	dsSDLWindow* sdlWindow = (dsSDLWindow*)window;
	dsApplication* application = window->application;

	unsigned int sdlFlags = SDL_WINDOW_HIGH_PIXEL_DENSITY;

	uint32_t platformID = application->renderer->platformID;
	if (flags & dsWindowFlags_Hidden)
		sdlFlags |= SDL_WINDOW_HIDDEN;
	if (flags & dsWindowFlags_Resizable)
		sdlFlags |= SDL_WINDOW_RESIZABLE;
	if (flags & dsWindowFlags_Minimized)
		sdlFlags |= SDL_WINDOW_MINIMIZED;
	if (flags & dsWindowFlags_Maximized)
		sdlFlags |= SDL_WINDOW_MAXIMIZED;
	if (flags & dsWindowFlags_GrabInput)
		sdlFlags |= SDL_WINDOW_MOUSE_GRABBED;
	if (platformID == DS_GLX_RENDERER_PLATFORM_ID)
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

	SDL_Window* internalWindow = NULL;
	SDL_PropertiesID windowProps = SDL_CreateProperties();
	if (windowProps)
	{
		SDL_SetStringProperty(windowProps, SDL_PROP_WINDOW_CREATE_TITLE_STRING, window->title);
		SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_X_NUMBER, position->x);
		SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_Y_NUMBER, position->y);
		SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
		SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
		SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, sdlFlags);
		SDL_SetBooleanProperty(
			windowProps, SDL_PROP_WINDOW_CREATE_EXTERNAL_GRAPHICS_CONTEXT_BOOLEAN, true);
		if (platformID == DS_EGL_RENDERER_PLATFORM_ID)
		{
			SDL_SetBooleanProperty(
				windowProps, SDL_PROP_WINDOW_CREATE_WAYLAND_CREATE_EGL_WINDOW_BOOLEAN, true);
		}
		internalWindow = SDL_CreateWindowWithProperties(windowProps);
		SDL_DestroyProperties(windowProps);
	}

	if (!internalWindow)
	{
		DS_LOG_ERROR_F(DS_APPLICATION_SDL_LOG_TAG, "Couldn't create window: %s", SDL_GetError());
		errno = EPERM;
		return false;
	}

	DS_ASSERT(!window->surface);
	sdlWindow->sdlWindow = internalWindow;
	sdlWindow->samples = application->renderer->surfaceSamples;

	// Respect the style if previously set, which should only happen when re-creating the window.
	// If this is required, we will need to synchronize the window state to query it from SDL.
	if (window->style != dsWindowStyle_Normal)
	{
		dsSDLWindow_setStyle(application, window, window->style);
		SDL_SyncWindow(internalWindow);
	}

	if (!(flags & dsWindowFlags_DelaySurfaceCreate) &&
		!dsSDLWindow_createSurfaceInternal(window))
	{
		DS_LOG_ERROR(DS_APPLICATION_SDL_LOG_TAG, "Couldn't create render surface.");
		dsSDLWindow_destroyComponents(window);
		return false;
	}

	SDL_GetWindowPosition(internalWindow, &window->position.x, &window->position.y);
	SDL_GetWindowSize(internalWindow, (int*)&window->width, (int*)&window->height);
	window->contentScale = SDL_GetWindowDisplayScale(internalWindow);

	SDL_Rect safeArea;
	SDL_GetWindowSafeArea(internalWindow, &safeArea);
	window->safeArea.min.x = safeArea.x;
	window->safeArea.min.y = safeArea.y;
	window->safeArea.max.x = safeArea.x + safeArea.w;
	window->safeArea.max.y = safeArea.y + safeArea.h;

	window->display = dsApplication_findDisplay(
		application, SDL_GetDisplayForWindow(internalWindow));
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

bool dsSDLWindow_createSurfaceInternal(dsWindow* window)
{
	dsSDLWindow* sdlWindow = (dsSDLWindow*)window;
	dsApplication* application = window->application;
	dsRenderer_restoreGlobalState(application->renderer);

	SDL_PropertiesID windowProps = SDL_GetWindowProperties(sdlWindow->sdlWindow);
	if (!windowProps)
	{
		DS_LOG_ERROR_F(
			DS_APPLICATION_SDL_LOG_TAG, "Couldn't get window properties: %s", SDL_GetError());
		errno = EPERM;
		return false;
	}

	void* displayHandle = NULL;
	void* windowHandle = NULL;

#if DS_WINDOWS
	windowHandle = SDL_GetPointerProperty(
		windowProps, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
#elif DS_IOS
	windowHandle = dsSDLWindow_getUsableWindowHandle(
		SDL_GetPointerProperty(windowProps, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, NULL));
#elif DS_MAC
	windowHandle = dsSDLWindow_getUsableWindowHandle(
		SDL_GetPointerProperty(windowProps, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL));
#elif DS_ANDROID
	windowHandle = SDL_GetPointerProperty(
		windowProps, SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, NULL);
#else
	displayHandle = SDL_GetPointerProperty(windowProps, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
	if (displayHandle)
	{
		windowHandle = (void*)SDL_GetNumberProperty(
			windowProps, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
	}
	else
	{
		displayHandle = SDL_GetPointerProperty(
			windowProps, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
		if (displayHandle)
		{
			if (application->renderer->rendererID == DS_GL_RENDERER_ID ||
				application->renderer->rendererID == DS_GLES_RENDERER_ID)
			{
				windowHandle = SDL_GetPointerProperty(
					windowProps, SDL_PROP_WINDOW_WAYLAND_EGL_WINDOW_POINTER, NULL);
			}
			else
			{
				windowHandle = SDL_GetPointerProperty(
					windowProps, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
			}
		}
	}
#endif

	if (!windowHandle)
	{
		DS_LOG_ERROR(DS_APPLICATION_SDL_LOG_TAG, "Unsupported video driver.");
		errno = EPERM;
		return false;
	}

	int pixelWidth, pixelHeight;
	SDL_GetWindowSizeInPixels(sdlWindow->sdlWindow, &pixelWidth, &pixelHeight);

	window->surface = dsRenderSurface_create(application->renderer, window->allocator,
		sdlWindow->surfaceName, displayHandle, windowHandle, dsRenderSurfaceType_Window,
		sdlWindow->renderSurfaceUsage, pixelWidth, pixelHeight);

	if (window->surface)
	{
		sdlWindow->curSurfaceWidth = window->surface->width;
		sdlWindow->curSurfaceHeight = window->surface->height;
		sdlWindow->curSurfaceRotation = window->surface->rotation;
	}

#if DS_APPLE
	dsSDLWindow_releaseUsableWindowHandle(windowHandle);
#endif

	return window->surface != NULL;
}

dsWindow* dsSDLWindow_create(dsApplication* application, dsAllocator* allocator,
	const char* title, const char* surfaceName, const dsWindowInitPosition* position,
	uint32_t width, uint32_t height, dsWindowFlags flags, dsRenderSurfaceUsage renderSurfaceUsage)
{
	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_APPLICATION_SDL_LOG_TAG,
			"Window allocator must support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

	if (!surfaceName)
		surfaceName = title;
	size_t surfaceNameLen = strlen(surfaceName) + 1;

	size_t fullSize = sizeof(dsSDLWindow);
	if (!dsAddAlignedSize(&fullSize, surfaceNameLen, DS_ALLOC_ALIGNMENT))
		return NULL;

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSDLWindow* window = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSDLWindow);
	DS_ASSERT(window);

	// Maintain our own copy of the title so it remains if the SDL window needs to be re-created.
	// This needs to be a separate allocation as it can be changed later.
	size_t titleLen = strlen(title) + 1;
	char* titleCopy = DS_ALLOCATE_OBJECT_ARRAY(allocator, char, titleLen);
	if (!titleCopy)
	{
		DS_VERIFY(dsAllocator_free(allocator, window));
		return NULL;
	}
	memcpy(titleCopy, title, titleLen);

	memset(window, 0, sizeof(dsSDLWindow));
	dsWindow* baseWindow = (dsWindow*)window;
	baseWindow->application = application;
	baseWindow->allocator = allocator;
	baseWindow->title = titleCopy;
	baseWindow->flags = flags & ~dsWindowFlags_InitOnlyMask;

	dsVector2i sdlPosition;
	baseWindow->style = getSdlPosition(&sdlPosition, position, application->primaryDisplay);

	window->curSurfaceWidth = 0;
	window->curSurfaceHeight = 0;
	window->curSurfaceRotation = dsRenderSurfaceRotation_0;
	window->renderSurfaceUsage = renderSurfaceUsage;

	char* surfaceNameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, surfaceNameLen);
	DS_ASSERT(surfaceNameCopy);
	memcpy(surfaceNameCopy, surfaceName, surfaceNameLen);
	window->surfaceName = surfaceNameCopy;

	// Remaining members are set when creating components. Use original flags to include init-only
	// flags.
	if (!dsSDLWindow_createComponents(baseWindow, &sdlPosition, width, height, flags))
	{
		DS_VERIFY(dsAllocator_free(allocator, window));
		DS_VERIFY(dsAllocator_free(allocator, titleCopy));
		return NULL;
	}

	return baseWindow;
}

bool dsSDLWindow_createSurface(dsApplication* application, dsWindow* window)
{
	DS_UNUSED(application);
	return window->surface || dsSDLWindow_createSurfaceInternal(window);
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
		dsWindow* window = application->windows[i];
		if (((dsSDLWindow*)window)->sdlWindow == sdlWindow)
			return window;
	}

	return NULL;
}

bool dsSDLWindow_setTitle(dsApplication* application, dsWindow* window, const char* title)
{
	DS_UNUSED(application);
	if (strcmp(window->title, title) == 0)
		return true;

	SDL_SetWindowTitle(((dsSDLWindow*)window)->sdlWindow, title);

	size_t titleLen = strlen(title) + 1;
	char* titleCopy = DS_ALLOCATE_OBJECT_ARRAY(window->allocator, char, titleLen);
	if (!titleCopy)
		return false;

	memcpy(titleCopy, title, titleLen);
	DS_VERIFY(dsAllocator_free(window->allocator, (void*)window->title));
	window->title = titleCopy;
	return true;
}

bool dsSDLWindow_setDisplayMode(
	dsApplication* application, dsWindow* window, const dsDisplayMode* displayMode)
{
	DS_UNUSED(application);

	if (window->style == dsWindowStyle_FullScreen)
	{
		SDL_DisplayMode sdlDisplayMode;
		if (!findSDLDisplayMode(&sdlDisplayMode, displayMode))
			return false;

		if (!SDL_SetWindowFullscreenMode(((dsSDLWindow*)window)->sdlWindow, &sdlDisplayMode))
		{
			DS_LOG_ERROR_F(
				DS_APPLICATION_SDL_LOG_TAG, "Couldn't set window display mode: %s", SDL_GetError());
			errno = EPERM;
			return false;
		}
	}

	window->displayMode = *displayMode;
	return true;
}

bool dsSDLWindow_resize(
	dsApplication* application, dsWindow* window, uint32_t width, uint32_t height)
{
	DS_UNUSED(application);
	SDL_SetWindowSize(((dsSDLWindow*)window)->sdlWindow, width, height);
	return true;
}

bool dsSDLWindow_setStyle(dsApplication* application, dsWindow* window, dsWindowStyle style)
{
	DS_UNUSED(application);
	dsSDLWindow* sdlWindow = (dsSDLWindow*)window;

	if (style == dsWindowStyle_FullScreen)
	{
		SDL_DisplayMode sdlDisplayMode;
		if (!findSDLDisplayMode(&sdlDisplayMode, &window->displayMode))
			return false;

		if (!SDL_SetWindowFullscreenMode(sdlWindow->sdlWindow, &sdlDisplayMode))
		{
			DS_LOG_ERROR_F(
				DS_APPLICATION_SDL_LOG_TAG, "Couldn't set window display mode: %s", SDL_GetError());
			errno = EPERM;
			return false;
		}
	}
	else
		SDL_SetWindowFullscreenMode(sdlWindow->sdlWindow, NULL);

	if (!SDL_SetWindowFullscreen(((dsSDLWindow*)window)->sdlWindow, style != dsWindowStyle_Normal))
	{
		DS_LOG_ERROR_F(
			DS_APPLICATION_SDL_LOG_TAG, "Couldn't set window display style: %s", SDL_GetError());
		errno = EPERM;
		return false;
	}

	// Wait until next update to pick up the change in window style.
	return true;
}

bool dsSDLWindow_setPosition(
	dsApplication* application, dsWindow* window, const dsVector2i* position)
{
	DS_UNUSED(application);
	int x, y;
	if (position)
	{
		x = position->x;
		y = position->y;
	}
	else
		x = y = SDL_WINDOWPOS_UNDEFINED;
	SDL_SetWindowPosition(((dsSDLWindow*)window)->sdlWindow, x, y);
	return true;
}

bool dsSDLWindow_center(dsApplication* application, dsWindow* window, const dsDisplayInfo* display)
{
	if (!display)
		display = application->primaryDisplay;

	int position;
	if (display)
		position = SDL_WINDOWPOS_CENTERED_DISPLAY((uint32_t)display->id);
	else
		position = SDL_WINDOWPOS_CENTERED;
	SDL_SetWindowPosition(((dsSDLWindow*)window)->sdlWindow, position, position);
	return true;
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

bool dsSDLWindow_setGrabbedInput(dsApplication* application, dsWindow* window, bool grab)
{
	DS_UNUSED(application);
	SDL_SetWindowMouseGrab(((dsSDLWindow*)window)->sdlWindow, grab);
	if (grab)
		window->flags |= dsWindowFlags_GrabInput;
	else
		window->flags &= ~dsWindowFlags_GrabInput;
	return true;
}

bool dsSDLWindow_setResizable(dsApplication* application, dsWindow* window, bool resizable)
{
	DS_UNUSED(application);
	SDL_SetWindowResizable(((dsSDLWindow*)window)->sdlWindow, resizable);
	if (resizable)
		window->flags |= dsWindowFlags_Resizable;
	else
		window->flags &= ~dsWindowFlags_Resizable;
	return true;
}

bool dsSDLWindow_raise(dsApplication* application, dsWindow* window)
{
	DS_UNUSED(application);
	SDL_RaiseWindow(((dsSDLWindow*)window)->sdlWindow);
	return true;
}

bool dsSDLWindow_beginTextInput(dsApplication* application, dsWindow* window,
	dsWindowTextInputType inputType, dsWindowTextInputFlags inputFlags)
{
	DS_UNUSED(application);

	SDL_PropertiesID textProps = SDL_CreateProperties();
	bool success = false;
	if (textProps)
	{
		if (inputFlags & dsWindowTextInputFlags_Password)
		{
			if (inputType == dsWindowTextInputType_Number)
			{
				SDL_SetNumberProperty(textProps, SDL_PROP_TEXTINPUT_TYPE_NUMBER,
					SDL_TEXTINPUT_TYPE_NUMBER_PASSWORD_HIDDEN);
			}
			else
			{
				SDL_SetNumberProperty(textProps, SDL_PROP_TEXTINPUT_TYPE_NUMBER,
					SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_HIDDEN);
			}
		}
		else
		{
			switch (inputType)
			{
				case dsWindowTextInputType_Default:
				case dsWindowTextInputType_Sentence:
				case dsWindowTextInputType_AllCaps:
					SDL_SetNumberProperty(
						textProps, SDL_PROP_TEXTINPUT_TYPE_NUMBER, SDL_TEXTINPUT_TYPE_TEXT);
					break;
				case dsWindowTextInputType_Name:
					SDL_SetNumberProperty(
						textProps, SDL_PROP_TEXTINPUT_TYPE_NUMBER, SDL_TEXTINPUT_TYPE_TEXT_NAME);
					break;
				case dsWindowTextInputType_Email:
					SDL_SetNumberProperty(
						textProps, SDL_PROP_TEXTINPUT_TYPE_NUMBER, SDL_TEXTINPUT_TYPE_TEXT_EMAIL);
					break;
				case dsWindowTextInputType_Number:
					SDL_SetNumberProperty(
						textProps, SDL_PROP_TEXTINPUT_TYPE_NUMBER, SDL_TEXTINPUT_TYPE_NUMBER);
					break;
			}
		}

		switch (inputType)
		{
			case dsWindowTextInputType_Default:
			case dsWindowTextInputType_Number:
			case dsWindowTextInputType_Email:
				SDL_SetNumberProperty(
					textProps, SDL_PROP_TEXTINPUT_CAPITALIZATION_NUMBER, SDL_CAPITALIZE_NONE);
				break;
			case dsWindowTextInputType_Name:
				SDL_SetNumberProperty(
					textProps, SDL_PROP_TEXTINPUT_CAPITALIZATION_NUMBER, SDL_CAPITALIZE_WORDS);
				break;
			case dsWindowTextInputType_Sentence:
				SDL_SetNumberProperty(
					textProps, SDL_PROP_TEXTINPUT_CAPITALIZATION_NUMBER, SDL_CAPITALIZE_SENTENCES);
				break;
			case dsWindowTextInputType_AllCaps:
				SDL_SetNumberProperty(
					textProps, SDL_PROP_TEXTINPUT_CAPITALIZATION_NUMBER, SDL_CAPITALIZE_LETTERS);
				break;
		}
		SDL_SetBooleanProperty(textProps, SDL_PROP_TEXTINPUT_AUTOCORRECT_BOOLEAN,
			(inputFlags & dsWindowTextInputFlags_Autocorrect) != 0);
		SDL_SetBooleanProperty(textProps, SDL_PROP_TEXTINPUT_MULTILINE_BOOLEAN,
			(inputFlags & dsWindowTextInputFlags_Multiline) != 0);
		success = SDL_StartTextInputWithProperties(((dsSDLWindow*)window)->sdlWindow, textProps);
		SDL_DestroyProperties(textProps);
	}

	if (!success)
	{
		DS_LOG_ERROR_F(DS_APPLICATION_SDL_LOG_TAG, "Couldn't start text input: %s", SDL_GetError());
		errno = EPERM;
		return false;
	}

	return true;
}

bool dsSDLWindow_endTextInput(dsApplication* application, dsWindow* window)
{
	DS_UNUSED(application);
	SDL_StopTextInput(((dsSDLWindow*)window)->sdlWindow);
	return true;
}

bool dsSDLWindow_setTextInputArea(dsApplication* application, dsWindow* window,
	const dsAlignedBox2i* bounds, uint32_t cursorOffset)
{
	DS_UNUSED(application);
	SDL_Rect rect = {bounds->min.x, bounds->min.y, bounds->max.x -bounds->max.x,
		bounds->max.y - bounds->min.y};
	SDL_SetTextInputArea(((dsSDLWindow*)window)->sdlWindow, &rect, cursorOffset);
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

	DS_VERIFY(dsAllocator_free(window->allocator, (void*)window->title));
	DS_VERIFY(dsAllocator_free(window->allocator, window));
	return true;
}
