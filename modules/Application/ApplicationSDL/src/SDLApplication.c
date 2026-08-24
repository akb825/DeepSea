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

#include <DeepSea/ApplicationSDL/SDLApplication.h>

#include "SDLApplicationInternal.h"
#include "SDLGameInput.h"
#include "SDLKeyboard.h"
#include "SDLMotionSensor.h"
#include "SDLShared.h"
#include "SDLWindow.h"

#include <DeepSea/Application/Application.h>
#include <DeepSea/Application/GameInput.h>
#include <DeepSea/Application/Window.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/Timer.h>

#include <DeepSea/Math/Vector2.h>

#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/RenderSurface.h>

#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

#define MAX_SWAP_WINDOWS 100U

// Need to swap middle and right buttons.
#define SDL_MOUSE_TO_DS_MOUSE_MASK(x) (((x) & ~(SDL_BUTTON_MMASK | SDL_BUTTON_RMASK)) | \
	(((x) & SDL_BUTTON_MMASK) << 1) | (((x) & SDL_BUTTON_RMASK) >> 1))

// Currently there is only a handful of window and window change flags, so use this to cache the
// changes when re-creating samples.
#define CACHED_CHANGE_MASK 0xFFFF0000
#define CACHED_CHANGE_SHIFT 16

static uint32_t showMessageBoxImpl(SDL_Window* parentWindow, dsMessageBoxType type,
	const char* title, const char* message, const char* const* buttons, uint32_t buttonCount,
	uint32_t enterButton, uint32_t escapeButton)
{
	SDL_MessageBoxData messageBox;
	switch (type)
	{
		case dsMessageBoxType_Info:
			messageBox.flags = SDL_MESSAGEBOX_INFORMATION;
			break;
		case dsMessageBoxType_Warning:
			messageBox.flags = SDL_MESSAGEBOX_WARNING;
			break;
		case dsMessageBoxType_Error:
			messageBox.flags = SDL_MESSAGEBOX_ERROR;
			break;
		default:
			messageBox.flags = 0;
	}

	messageBox.window = parentWindow;
	messageBox.title = title;
	messageBox.message = message;
	messageBox.numbuttons = buttonCount;
	messageBox.colorScheme = NULL;

	DS_ASSERT(buttonCount <= DS_MAX_MESSAGE_BOX_BUTTONS);
	SDL_MessageBoxButtonData* buttonData = DS_ALLOCATE_STACK_OBJECT_ARRAY(
		SDL_MessageBoxButtonData, buttonCount);
	for (uint32_t i = 0; i < buttonCount; ++i)
	{
		SDL_MessageBoxButtonData* button = buttonData + i;
		button->flags = 0;
		if (i == enterButton)
			button->flags |= SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
		if (i == escapeButton)
			button->flags |= SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
		button->buttonID = i;
		button->text = buttons[i];
	}
	messageBox.buttons = buttonData;

	int buttonId = 0;
	if (!SDL_ShowMessageBox(&messageBox, &buttonId))
	{
		errno = EINVAL;
		return DS_MESSAGE_BOX_NO_BUTTON;
	}

	return buttonId;
}

static void* createBackgroundGLWindow(void* userData, dsRenderSurfaceType surfaceType)
{
	DS_UNUSED(userData);
	DS_UNUSED(surfaceType);
	DS_ASSERT(surfaceType == dsRenderSurfaceType_Window);

	SDL_Window* window = NULL;
	SDL_PropertiesID windowProps = SDL_CreateProperties();
	if (windowProps)
	{
		SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, SDL_WINDOW_HIDDEN);
		SDL_SetBooleanProperty(
			windowProps, SDL_PROP_WINDOW_CREATE_WAYLAND_CREATE_EGL_WINDOW_BOOLEAN, true);
		SDL_SetBooleanProperty(
			windowProps, SDL_PROP_WINDOW_CREATE_EXTERNAL_GRAPHICS_CONTEXT_BOOLEAN, true);
		window = SDL_CreateWindowWithProperties(windowProps);
		SDL_DestroyProperties(windowProps);
	}
	return window;
}

static void destroyBackgroundGLWindow(
	void* userData, dsRenderSurfaceType surfaceType, void* surface)
{
	DS_UNUSED(userData);
	DS_UNUSED(surfaceType);
	DS_ASSERT(surfaceType == dsRenderSurfaceType_Window);
	DS_ASSERT(surface);
	SDL_DestroyWindow((SDL_Window*)surface);
}

static void* getWaylandGLWindowHandle(
	void* userData, dsRenderSurfaceType surfaceType, void* surface)
{
	DS_UNUSED(userData);
	DS_UNUSED(surfaceType);
	DS_ASSERT(surfaceType == dsRenderSurfaceType_Window);
	DS_ASSERT(surface);

	return SDL_GetPointerProperty(
		SDL_GetWindowProperties((SDL_Window*)surface), SDL_PROP_WINDOW_WAYLAND_EGL_WINDOW_POINTER,
		NULL);
}

static dsWindow* findWindow(dsApplication* application, uint32_t windowID)
{
	for (uint32_t i = 0; i < application->windowCount; ++i)
	{
		if (SDL_GetWindowID(((dsSDLWindow*)application->windows[i])->sdlWindow) == windowID)
			return application->windows[i];
	}

	return NULL;
}

inline static dsDisplayInfo* findDisplay(dsApplication* application, uint32_t displayID)
{
	// Mutable find, and also doesn't set errno if not found.
	for (uint32_t i = 0; i < application->displayCount; ++i)
	{
		dsDisplayInfo* display = application->displays[i];
		if (display->id == displayID)
			return display;
	}

	return NULL;
}

inline static dsRenderSurfaceRotation baseRotation(SDL_DisplayOrientation orientation)
{
	switch (orientation)
	{
		case SDL_ORIENTATION_UNKNOWN:
		case SDL_ORIENTATION_LANDSCAPE:
			return dsRenderSurfaceRotation_0;
		case SDL_ORIENTATION_LANDSCAPE_FLIPPED:
			return dsRenderSurfaceRotation_180;
		case SDL_ORIENTATION_PORTRAIT:
			return dsRenderSurfaceRotation_90;
		case SDL_ORIENTATION_PORTRAIT_FLIPPED:
			return dsRenderSurfaceRotation_270;
	}

	DS_ASSERT(false);
	return dsRenderSurfaceRotation_0;
}

static dsRenderSurfaceRotation displayOrientationToRotation(
	SDL_DisplayOrientation nativeOrientation, SDL_DisplayOrientation curOrientation)
{
	dsRenderSurfaceRotation nativeRotation = baseRotation(nativeOrientation);
	dsRenderSurfaceRotation curRotation = baseRotation(curOrientation);
	int rotationDiff = curRotation - nativeRotation;
	if (rotationDiff < 0)
		rotationDiff += 4;
	DS_ASSERT(rotationDiff >= dsRenderSurfaceRotation_0 &&
		rotationDiff <= dsRenderSurfaceRotation_270);
	return (dsRenderSurfaceRotation)rotationDiff;
}

inline static void rectToBounds(dsAlignedBox2i* outBounds, const SDL_Rect* rect)
{
	outBounds->min.x = rect->x;
	outBounds->min.y = rect->y;
	outBounds->max.x = rect->x + rect->w;
	outBounds->max.y = rect->y + rect->h;
}

dsDisplayInfo* createDisplay(dsAllocator* allocator, SDL_DisplayID displayID)
{
	const char* name = SDL_GetDisplayName(displayID);
	size_t nameLen = name ? strlen(name) + 1 : 0;

	const SDL_DisplayMode* defaultMode = SDL_GetDesktopDisplayMode(displayID);
	if (!defaultMode)
	{
		DS_LOG_ERROR_F(
			DS_APPLICATION_LOG_TAG, "Couldn't get default display mode: %s", SDL_GetError());
		errno = EPERM;
		return NULL;
	}

	int sdlModeCount;
	SDL_DisplayMode** sdlModes = SDL_GetFullscreenDisplayModes(displayID, &sdlModeCount);
	if (!sdlModes)
	{
		DS_LOG_ERROR_F(DS_APPLICATION_LOG_TAG, "Couldn't get display modes: %s", SDL_GetError());
		errno = EPERM;
		return NULL;
	}

	uint32_t displayModeCount = 0;
	for (int i = 0; i < sdlModeCount; ++i)
	{
		if (sdlModes[i]->format == defaultMode->format)
			++displayModeCount;
	}

	size_t fullSize = sizeof(dsDisplayInfo);
	dsMemorySize sizes[] =
	{
		{sizeof(char), nameLen},
		{sizeof(dsDisplayMode), displayModeCount}
	};
	if (!dsAccumulateAlignedSizes(&fullSize, sizes, DS_ARRAY_SIZE(sizes), DS_ALLOC_ALIGNMENT))
	{
		SDL_free(sdlModes);
		return NULL;
	}

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		SDL_free(sdlModes);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsDisplayInfo* display = DS_ALLOCATE_OBJECT(&bufferAlloc, dsDisplayInfo);
	DS_ASSERT(display);

	char* nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(nameCopy || !name);
	memcpy(nameCopy, name, nameLen);

	dsDisplayMode* displayModes = NULL;
	display->defaultMode = 0;
	if (displayModeCount > 0)
	{
		displayModes = DS_ALLOCATE_OBJECT_ARRAY(
			&bufferAlloc, dsDisplayMode, displayModeCount);
		DS_ASSERT(displayModes);
		for (int i = 0, j = 0; i < sdlModeCount; ++i)
		{
			const SDL_DisplayMode* sdlMode = sdlModes[i];
			if (sdlMode->format != defaultMode->format)
				continue;

			dsDisplayMode* displayMode = displayModes + j;
			displayMode->displayID = displayID;
			displayMode->width = sdlMode->w;
			displayMode->height = sdlMode->h;
			displayMode->refreshRate = sdlMode->refresh_rate;

			if (sdlMode->w == defaultMode->w && sdlMode->h == defaultMode->h &&
				sdlMode->refresh_rate == defaultMode->refresh_rate)
			{
				display->defaultMode = j;
			}

			++j;
		}
	}
	SDL_free(sdlModes);

	display->name = nameCopy;
	display->displayModes = displayModes;
	display->id = displayID;
	display->displayModeCount = displayModeCount;
	display->scale = SDL_GetDisplayContentScale(displayID);

	display->rotation = displayOrientationToRotation(
		SDL_GetNaturalDisplayOrientation(displayID), SDL_GetCurrentDisplayOrientation(displayID));

	SDL_Rect rect;
	DS_VERIFY(SDL_GetDisplayBounds(displayID, &rect));
	rectToBounds(&display->desktopBounds, &rect);

	DS_VERIFY(SDL_GetDisplayUsableBounds(displayID, &rect));
	rectToBounds(&display->usableBounds, &rect);

	return display;
}

static bool updateDisplayBounds(dsDisplayInfo* display)
{
	SDL_Rect rect;
	DS_VERIFY(SDL_GetDisplayBounds((uint32_t)display->id, &rect));

	dsAlignedBox2i bounds;
	rectToBounds(&bounds, &rect);
	bool changed = false;
	if (memcmp(&bounds, &display->desktopBounds, sizeof(dsAlignedBox2i)) != 0)
	{
		display->desktopBounds = bounds;
		changed = true;
	}

	DS_VERIFY(SDL_GetDisplayUsableBounds((uint32_t)display->id, &rect));
	rectToBounds(&bounds, &rect);
	if (memcmp(&bounds, &display->usableBounds, sizeof(dsAlignedBox2i)) != 0)
	{
		display->usableBounds = bounds;
		changed = true;
	}

	return changed;
}

static void updatePrimaryDisplay(dsApplication* application)
{
	SDL_DisplayID primaryDisplayID = SDL_GetPrimaryDisplay();
	if (application->primaryDisplay && application->primaryDisplay->id == primaryDisplayID)
		return;

	application->primaryDisplay = findDisplay(application, primaryDisplayID);
}

static bool setGLAttributes(dsRenderer* renderer)
{
	switch (renderer->surfaceColorFormat & dsGfxFormat_StandardMask)
	{
		case dsGfxFormat_R5G6B5:
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
			break;
		case dsGfxFormat_R8G8B8:
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
			break;
		case dsGfxFormat_R8G8B8A8:
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
			break;
		case dsGfxFormat_A2B10G10R10:
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 10);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 10);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 10);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 2);
			break;
		default:
			return false;
	}

	SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE,
		(renderer->surfaceColorFormat & dsGfxFormat_DecoratorMask) == dsGfxFormat_SRGB);

	switch (renderer->surfaceDepthStencilFormat)
	{
		case dsGfxFormat_Unknown:
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
			break;
		case dsGfxFormat_D16:
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
			break;
		case dsGfxFormat_X8D24:
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
			break;
		case dsGfxFormat_D16S8:
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
			break;
		case dsGfxFormat_D24S8:
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
			break;
		default:
			return false;
	}

	SDL_GL_SetAttribute(SDL_GL_STEREO, renderer->stereoscopic);
	if (renderer->surfaceSamples > 1)
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, renderer->surfaceSamples);
	}
	else
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
	}

	return true;
}

static bool updateWindowState(
	dsEvent* outEvent, dsApplication* application, dsWindow* window, bool validDisplayOnly)
{
	dsSDLWindow* sdlWindow = ((dsSDLWindow*)window);
	SDL_Window* internalWindow = sdlWindow->sdlWindow;

	SDL_WindowFlags sdlFlags = SDL_GetWindowFlags(internalWindow);

	dsWindowFlags flags = window->flags & ~dsWindowFlags_EventMask;
	if (sdlFlags & SDL_WINDOW_HIDDEN)
		flags |= dsWindowFlags_Hidden;
	if (sdlFlags & SDL_WINDOW_MAXIMIZED)
		flags |= dsWindowFlags_Maximized;
	if (sdlFlags & SDL_WINDOW_MINIMIZED)
		flags |= dsWindowFlags_Minimized;

	dsWindowStyle style;
	if (sdlFlags & SDL_WINDOW_FULLSCREEN)
	{
		if (SDL_GetWindowFullscreenMode(internalWindow))
			style = dsWindowStyle_FullScreen;
		else
			style = dsWindowStyle_FullScreenBorderless;
	}
	else
		style = dsWindowStyle_Normal;

	dsVector2i position;
	SDL_GetWindowPosition(internalWindow, &position.x, &position.y);

	uint32_t width, height;
	SDL_GetWindowSize(internalWindow, (int*)&width, (int*)&height);

	float contentScale = SDL_GetWindowDisplayScale(internalWindow);

	SDL_Rect safeArea;
	SDL_GetWindowSafeArea(internalWindow, &safeArea);
	dsAlignedBox2i safeBounds;
	rectToBounds(&safeBounds, &safeArea);

	SDL_DisplayID displayID = SDL_GetDisplayForWindow(internalWindow);

	dsWindowChangeFlags changeFlags = 0;

	// Check for display first to avoid sending events for invalid displays.
	if (!window->display || window->display->id != displayID)
	{
		const dsDisplayInfo* display = findDisplay(application, displayID);
		if (!display && validDisplayOnly)
			return false;

		// Sanity check if display continues to be invalid.
		if (window->display != display)
		{
			changeFlags |= dsWindowChangeFlags_Display;
			window->display = display;
		}
	}

	if (window->flags != flags)
	{
		if ((window->flags & dsWindowFlags_Hidden) != (flags & dsWindowFlags_Hidden))
			changeFlags |= dsWindowChangeFlags_Hidden;
		if ((window->flags & dsWindowFlags_Maximized) != (flags & dsWindowFlags_Maximized))
			changeFlags |= dsWindowChangeFlags_Maximized;
		if ((window->flags & dsWindowFlags_Minimized) != (flags & dsWindowFlags_Minimized))
			changeFlags |= dsWindowChangeFlags_Minimized;
		window->flags = flags;
	}

	if (window->style != style)
	{
		changeFlags |= dsWindowChangeFlags_Style;
		window->style = style;
	}

	if (!dsVector2_equal(window->position, position))
	{
		changeFlags |= dsWindowChangeFlags_Position;
		window->position = position;
	}

	if (window->width != width || window->height != height)
	{
		changeFlags |= dsWindowChangeFlags_Size;
		window->width = width;
		window->height = height;
	}

	const dsRenderSurface* surface = window->surface;
	if (surface && (sdlWindow->curSurfaceWidth != surface->width ||
		sdlWindow->curSurfaceHeight != surface->height ||
		sdlWindow->curSurfaceRotation != surface->rotation))
	{
		changeFlags |= dsWindowChangeFlags_SurfaceSize;
		sdlWindow->curSurfaceWidth = surface->width;
		sdlWindow->curSurfaceHeight = surface->height;
		sdlWindow->curSurfaceRotation = surface->rotation;
	}

	if (window->contentScale != contentScale)
	{
		changeFlags |= dsWindowChangeFlags_ContentScale;
		window->contentScale = contentScale;
	}

	if (!dsVector2_equal(window->safeArea.min, safeBounds.min) ||
		!dsVector2_equal(window->safeArea.max, safeBounds.max))
	{
		changeFlags |= dsWindowChangeFlags_SafeArea;
		window->safeArea = safeBounds;
	}

	if (changeFlags == 0)
		return false;

	outEvent->type = dsAppEventType_WindowChanged;
	outEvent->windowChange.window = window;
	outEvent->windowChange.flags = changeFlags;
	return true;
}

#if DS_ANDROID
static void invalidateWindowSurfaces(dsApplication* application)
{
	for (unsigned int i = 0; i < application->windowCount; ++i)
	{
		dsWindow* window = application->windows[i];
		dsRenderSurface_destroy(window->surface);
		window->surface = NULL;
		dsSDLWindow_createSurfaceInternal(window);

		dsEvent event;
		event.type = dsAppEventType_SurfaceInvalidated;
		event.window = window;
		dsApplication_dispatchEvent(application, &event);
	}
}
#endif

static void initializeFrame(dsApplication* application)
{
	dsSDLApplication* sdlApplication = (dsSDLApplication*)application;
	if (sdlApplication->hasFrameEvents)
		return;

	sdlApplication->hasFrameEvents = true;

	// Reference point to convert timestamps. Expect minor timing differences between calls should
	// be negligible.
	sdlApplication->inputTickRef = dsTimer_currentTicks();
	sdlApplication->inputNSRef = SDL_GetTicksNS();

	DS_VERIFY(dsRenderer_beginFrame(application->renderer));

	DS_PROFILE_SCOPE_START("Process Events");

	// Update the render surfaces before processing any events. This ensures that the size will
	// be updated even if no events were generated (e.g. different event timing, state
	// management when the window size doesn't match the pixel size), and window events will be
	// grouped so inter-dependent changes are processed at the same time.
	for (uint32_t i = 0; i < application->windowCount; ++i)
	{
		dsWindow* window = application->windows[i];
		if (!window->surface)
			continue;

		SDL_Window* sdlWindow = ((dsSDLWindow*)window)->sdlWindow;
		int pixelWidth, pixelHeight;
		SDL_GetWindowSizeInPixels(sdlWindow, &pixelWidth, &pixelHeight);
		dsRenderSurface_update(window->surface, pixelWidth, pixelHeight);
	}
}

static bool convertEvent(
	dsEvent* outEvent, dsApplication* application, dsWindow* focusWindow, const SDL_Event* sdlEvent)
{
	switch (sdlEvent->type)
	{
		case SDL_EVENT_WILL_ENTER_BACKGROUND:
			outEvent->type = dsAppEventType_WillEnterBackground;
			return true;
		case SDL_EVENT_DID_ENTER_BACKGROUND:
			outEvent->type = dsAppEventType_DidEnterBackground;
			return true;
		case SDL_EVENT_WILL_ENTER_FOREGROUND:
			outEvent->type = dsAppEventType_WillEnterForeground;
			return true;
		case SDL_EVENT_DID_ENTER_FOREGROUND:
			outEvent->type = dsAppEventType_DidEnterForeground;
			return true;
		case SDL_EVENT_DISPLAY_ORIENTATION:
		{
			dsDisplayInfo* display = findDisplay(application, sdlEvent->display.displayID);
			if (!display)
				return false;

			display->rotation = displayOrientationToRotation(
				SDL_GetNaturalDisplayOrientation(sdlEvent->display.displayID),
				(SDL_DisplayOrientation)sdlEvent->display.data1);

			outEvent->type = dsAppEventType_DisplayRotated;
			outEvent->display = display;
			return true;
		}
		case SDL_EVENT_DISPLAY_ADDED:
		{
			dsDisplayInfo* display = createDisplay(
				application->allocator, sdlEvent->display.displayID);
			uint32_t index = application->displayCount;
			if (!DS_CHECK(DS_APPLICATION_SDL_LOG_TAG, display != NULL &&
					DS_RESIZEABLE_ARRAY_ADD(application->allocator, application->displays,
					application->displayCount, application->displayCapacity, 1)))
			{
				return false;
			}

			updatePrimaryDisplay(application);
			application->displays[index] = display;
			outEvent->type = dsAppEventType_DisplayConnected;
			outEvent->display = display;
			return true;
		}
		case SDL_EVENT_DISPLAY_REMOVED:
			outEvent->display = findDisplay(application, sdlEvent->display.displayID);
			if (!outEvent->display)
				return false;

			updatePrimaryDisplay(application);
			outEvent->type = dsAppEventType_DisplayDisconnected;
			return true;
		case SDL_EVENT_DISPLAY_MOVED:
		case SDL_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED:
		{
			dsDisplayInfo* display = findDisplay(application, sdlEvent->display.displayID);
			if (!display || !updateDisplayBounds(display))
				return false;

			outEvent->type = dsAppEventType_DisplayBoundsChanged;
			outEvent->display = display;
			return true;
		}
		case SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED:
		{
			dsDisplayInfo* display = findDisplay(application, sdlEvent->display.displayID);
			const SDL_DisplayMode* sdlMode = SDL_GetDesktopDisplayMode(sdlEvent->display.displayID);
			if (!display || !sdlMode)
				return false;

			for (uint32_t i = 0; i < display->displayModeCount; ++i)
			{
				const dsDisplayMode* displayMode = display->displayModes + i;
				if ((int)displayMode->width == sdlMode->w &&
					(int)displayMode->height == sdlMode->h &&
					displayMode->refreshRate == sdlMode->refresh_rate)
				{
					display->defaultMode = i;
					outEvent->type = dsAppEventType_DefaultDisplayModeChanged;
					outEvent->display = display;
					return true;
				}
			}
			return false;
		}
		case SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED:
		{
			dsDisplayInfo* display = findDisplay(application, sdlEvent->display.displayID);
			if (!display)
				return false;

			display->scale = SDL_GetDisplayContentScale(sdlEvent->display.displayID);
			outEvent->type = dsAppEventType_DisplayScaleChanged;
			outEvent->display = display;
			return true;
		}
		case SDL_EVENT_WINDOW_SHOWN:
		case SDL_EVENT_WINDOW_HIDDEN:
		case SDL_EVENT_WINDOW_MOVED:
		case SDL_EVENT_WINDOW_RESIZED:
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
		case SDL_EVENT_WINDOW_MINIMIZED:
		case SDL_EVENT_WINDOW_MAXIMIZED:
		case SDL_EVENT_WINDOW_RESTORED:
		case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
		case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
		case SDL_EVENT_WINDOW_SAFE_AREA_CHANGED:
		case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
		case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
		{
#if DS_ANDROID
			if (sdlEvent->type == SDL_EVENT_WINDOW_RESTORED)
			{
				invalidateWindowSurfaces(application);
				// Make sure invalidated surfaces fully go through the GPU.
				dsRenderer_waitUntilIdle(application->renderer);
			}
#endif
			dsWindow* window = findWindow(application, sdlEvent->window.windowID);
			if (!window)
				return false;

			return updateWindowState(outEvent, application, window, true);
		}
		case SDL_EVENT_WINDOW_MOUSE_ENTER:
			outEvent->window = findWindow(application, sdlEvent->window.windowID);
			if (!outEvent->window)
				return false;

			outEvent->type = dsAppEventType_MouseEntered;
			return true;
		case SDL_EVENT_WINDOW_MOUSE_LEAVE:
			outEvent->window = findWindow(application, sdlEvent->window.windowID);
			if (!outEvent->window)
				return false;

			outEvent->type = dsAppEventType_MouseLeft;
			return true;
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			outEvent->window = findWindow(application, sdlEvent->window.windowID);
			if (!outEvent->window)
				return false;

			outEvent->type = dsAppEventType_FocusGained;
			return true;
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			outEvent->window = findWindow(application, sdlEvent->window.windowID);
			if (!outEvent->window)
				return false;

			outEvent->type = dsAppEventType_FocusLost;
			return true;
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		{
			dsWindow* window = findWindow(application, sdlEvent->window.windowID);
			if (!window)
				return false;

			if (!window->closeFunc || window->closeFunc(window, outEvent->window->closeUserData))
			{
				outEvent->type = dsAppEventType_WindowClosed;
				outEvent->window = window;
				dsWindow_setHidden(window, true);
				return true;
			}
			return false;
		}
		case SDL_EVENT_WINDOW_DESTROYED:
		{
			dsWindow* window = findWindow(application, sdlEvent->window.windowID);
			if (!window)
				return false;

			outEvent->type = dsAppEventType_WindowDestroyed;
			outEvent->window = window;
			return true;
		}
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
			outEvent->type = sdlEvent->type == SDL_EVENT_KEY_DOWN ? dsAppEventType_KeyDown :
				dsAppEventType_KeyUp;
			outEvent->key.window = findWindow(application, sdlEvent->key.windowID);
			if (!outEvent->key.window)
				outEvent->key.window = focusWindow;
			outEvent->key.key = dsFromSDLScancode(sdlEvent->key.scancode);
			outEvent->key.modifiers = dsFromSDLKeyMod((SDL_Keymod)sdlEvent->key.mod);
			outEvent->key.repeat = sdlEvent->key.repeat != 0;
			return true;
		case SDL_EVENT_TEXT_EDITING:
			outEvent->type = dsAppEventType_TextEdit;
			outEvent->textEdit.window = findWindow(application, sdlEvent->edit.windowID);
			if (!outEvent->textEdit.window)
				outEvent->textEdit.window = focusWindow;
			outEvent->textEdit.cursor = sdlEvent->edit.start;
			outEvent->textEdit.selectionLength = sdlEvent->edit.length;
			outEvent->textEdit.text = sdlEvent->edit.text;
			return true;
		case SDL_EVENT_TEXT_INPUT:
			outEvent->type = dsAppEventType_TextInput;
			outEvent->textInput.window = findWindow(application, sdlEvent->text.windowID);
			if (!outEvent->textInput.window)
				outEvent->textInput.window = focusWindow;
			outEvent->textInput.text = sdlEvent->text.text;
			return true;
		case SDL_EVENT_MOUSE_MOTION:
			if (sdlEvent->motion.which == SDL_TOUCH_MOUSEID)
				return false;

			outEvent->type = dsAppEventType_MouseMove;
			outEvent->mouseMove.window = findWindow(application, sdlEvent->motion.windowID);
			if (!outEvent->mouseMove.window)
				outEvent->mouseMove.window = focusWindow;
			outEvent->mouseMove.mouseID = sdlEvent->motion.which;
			outEvent->mouseMove.position.x = sdlEvent->motion.x;
			outEvent->mouseMove.position.y = sdlEvent->motion.y;
			outEvent->mouseMove.delta.x = sdlEvent->motion.xrel;
			outEvent->mouseMove.delta.y = sdlEvent->motion.yrel;
			return true;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (sdlEvent->button.which == SDL_TOUCH_MOUSEID)
				return false;

			outEvent->type = sdlEvent->type == SDL_EVENT_MOUSE_BUTTON_UP ?
				dsAppEventType_MouseButtonUp : dsAppEventType_MouseButtonDown;
			outEvent->mouseButton.window = findWindow(application, sdlEvent->button.windowID);
			if (!outEvent->mouseButton.window)
				outEvent->mouseButton.window = focusWindow;
			outEvent->mouseButton.mouseID = sdlEvent->button.which;
			outEvent->mouseButton.button = DS_MOUSE_BUTTON(sdlEvent->button.button);
			outEvent->mouseButton.button = SDL_MOUSE_TO_DS_MOUSE_MASK(outEvent->mouseButton.button);
			outEvent->mouseButton.position.x = sdlEvent->button.x;
			outEvent->mouseButton.position.y = sdlEvent->button.y;
			return true;
		case SDL_EVENT_MOUSE_WHEEL:
			if (sdlEvent->wheel.which == SDL_TOUCH_MOUSEID)
				return false;

			outEvent->type = dsAppEventType_MouseWheel;
			outEvent->mouseWheel.window = findWindow(application, sdlEvent->wheel.windowID);
			if (!outEvent->mouseWheel.window)
				outEvent->mouseWheel.window = focusWindow;
			outEvent->mouseWheel.mouseID = sdlEvent->wheel.which;
			outEvent->mouseWheel.position.x = sdlEvent->wheel.mouse_x;
			outEvent->mouseWheel.position.y = sdlEvent->wheel.mouse_y;
			outEvent->mouseWheel.delta.x = sdlEvent->wheel.x;
			outEvent->mouseWheel.delta.y = sdlEvent->wheel.y;
			outEvent->mouseWheel.yFlipped = sdlEvent->wheel.direction == SDL_MOUSEWHEEL_FLIPPED;
			return true;
		case SDL_EVENT_JOYSTICK_AXIS_MOTION:
		{
			const dsGameInput* gameInput = dsSDLGameInput_find(application, sdlEvent->jaxis.which);
			if (!gameInput || sdlEvent->jaxis.axis >= gameInput->axisCount)
				return false;

			outEvent->type = dsAppEventType_GameInputAxis;
			outEvent->gameInputAxis.window = focusWindow;
			outEvent->gameInputAxis.gameInput = gameInput;
			outEvent->gameInputAxis.mapping = dsGameInput_getAxisControllerMap(
				gameInput, sdlEvent->jaxis.axis);
			outEvent->gameInputAxis.axis = sdlEvent->jaxis.axis;
			outEvent->gameInputAxis.value = dsSDLGameInput_getAxisValue(sdlEvent->jaxis.value);
			return true;
		}
		case SDL_EVENT_JOYSTICK_BALL_MOTION:
		{
			const dsGameInput* gameInput =  dsSDLGameInput_find(application, sdlEvent->jball.which);
			if (!gameInput || sdlEvent->jball.ball >= gameInput->ballCount)
				return false;

			outEvent->type = dsAppEventType_GameInputBall;
			outEvent->gameInputAxis.window = focusWindow;
			outEvent->gameInputBall.gameInput = gameInput;
			outEvent->gameInputBall.delta.x = sdlEvent->jball.xrel;
			outEvent->gameInputBall.delta.y = sdlEvent->jball.yrel;
			return true;
		}
		case SDL_EVENT_JOYSTICK_HAT_MOTION:
		{
			dsGameInput* gameInput = dsSDLGameInput_find(application, sdlEvent->jhat.which);
			if (!gameInput || sdlEvent->jhat.hat >= gameInput->dpadCount)
				return false;

			// Controller mapped events are deployed as button events, which may be multiple
			// events. Avoid sending the dpad event if all the changes are mapped.
			if (dsSDLGameInput_dispatchControllerDPadEvents(gameInput, application,
					focusWindow, sdlEvent->jhat.hat, sdlEvent->jhat.value, outEvent->time))
			{
				return false;
			}

			outEvent->type = dsAppEventType_GameInputDPad;
			outEvent->gameInputAxis.window = focusWindow;
			outEvent->gameInputDPad.gameInput = gameInput;
			outEvent->gameInputDPad.dpad = sdlEvent->jhat.hat;
			dsSDLGameInput_convertHatDirection(
				&outEvent->gameInputDPad.direction, sdlEvent->jhat.value);
			return true;
		}
		case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
		case SDL_EVENT_JOYSTICK_BUTTON_UP:
		{
			const dsGameInput* gameInput = dsSDLGameInput_find(application, sdlEvent->jbutton.which);
			if (!gameInput || sdlEvent->jbutton.button >= gameInput->buttonCount)
				return false;

			outEvent->type = sdlEvent->type == SDL_EVENT_JOYSTICK_BUTTON_UP ?
				dsAppEventType_GameInputButtonUp : dsAppEventType_GameInputButtonDown;
			outEvent->gameInputButton.window = focusWindow;
			outEvent->gameInputButton.gameInput = gameInput;
			outEvent->gameInputAxis.mapping = dsGameInput_getButtonControllerMap(
				gameInput, sdlEvent->jbutton.button);
			outEvent->gameInputButton.button = sdlEvent->jbutton.button;
			return true;
		}
		case SDL_EVENT_JOYSTICK_ADDED:
		{
			dsGameInput* gameInput = dsSDLGameInput_add(
				application, sdlEvent->jdevice.which);
			if (!gameInput)
			{
				DS_LOG_ERROR_F(DS_APPLICATION_SDL_LOG_TAG,
					"Couldn't add gameInput: %s", dsErrorString(errno));
				return false;
			}

			outEvent->type = dsAppEventType_GameInputConnected;
			outEvent->gameInputConnect = gameInput;
			return true;
		}
		case SDL_EVENT_JOYSTICK_REMOVED:
			outEvent->type = dsAppEventType_GameInputDisconnected;
			outEvent->gameInputConnect = dsSDLGameInput_find(application, sdlEvent->jdevice.which);
			return outEvent->gameInputConnect != NULL;
		case SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN:
		case SDL_EVENT_GAMEPAD_TOUCHPAD_UP:
		case SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION:
			switch (sdlEvent->type)
			{
				case SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN:
					outEvent->type = dsAppEventType_TouchFingerDown;
					break;
				case SDL_EVENT_GAMEPAD_TOUCHPAD_UP:
					outEvent->type = dsAppEventType_TouchFingerUp;
					break;
				case SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION:
					outEvent->type = dsAppEventType_TouchMoved;
					break;
				default:
					DS_ASSERT(false);
					break;
			}
			outEvent->touch.gameInput = dsSDLGameInput_find(
				application, sdlEvent->gtouchpad.which);
			if (!outEvent->touch.gameInput)
				return false;

			outEvent->touch.window = focusWindow;
			outEvent->touch.touchID = sdlEvent->gtouchpad.touchpad;
			outEvent->touch.fingerID = sdlEvent->gtouchpad.finger;
			outEvent->touch.position.x = sdlEvent->gtouchpad.x;
			outEvent->touch.position.y = sdlEvent->gtouchpad.y;
			outEvent->touch.delta.x = 0;
			outEvent->touch.delta.y = 0;
			outEvent->touch.pressure = sdlEvent->gtouchpad.pressure;
			return true;
		case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:
			outEvent->type = dsAppEventType_MotionSensor;
			outEvent->motionSensor.window = focusWindow;
			outEvent->motionSensor.sensor = NULL;
			outEvent->motionSensor.gameInput = dsSDLGameInput_find(
				application, sdlEvent->gsensor.which);
			if (!outEvent->motionSensor.gameInput)
				return false;

			switch (sdlEvent->gsensor.sensor)
			{
				case SDL_SENSOR_ACCEL:
					outEvent->motionSensor.type = dsMotionSensorType_Accelerometer;
					break;
				case SDL_SENSOR_GYRO:
					outEvent->motionSensor.type = dsMotionSensorType_Gyroscope;
					break;
				case SDL_SENSOR_ACCEL_L:
					outEvent->motionSensor.type = dsMotionSensorType_AccelerometerLeft;
					break;
				case SDL_SENSOR_GYRO_L:
					outEvent->motionSensor.type = dsMotionSensorType_GyroscopeLeft;
					break;
				case SDL_SENSOR_ACCEL_R:
					outEvent->motionSensor.type = dsMotionSensorType_AccelerometerRight;
					break;
				case SDL_SENSOR_GYRO_R:
					outEvent->motionSensor.type = dsMotionSensorType_GyroscopeRight;
					break;
				default:
					return false;
			}
			memcpy(&outEvent->motionSensor.data, sdlEvent->gsensor.data, sizeof(dsVector3f));
			return true;
		case SDL_EVENT_FINGER_DOWN:
		case SDL_EVENT_FINGER_UP:
		case SDL_EVENT_FINGER_MOTION:
			switch (sdlEvent->type)
			{
				case SDL_EVENT_FINGER_DOWN:
					outEvent->type = dsAppEventType_TouchFingerDown;
					break;
				case SDL_EVENT_FINGER_UP:
					outEvent->type = dsAppEventType_TouchFingerUp;
					break;
				case SDL_EVENT_FINGER_MOTION:
					outEvent->type = dsAppEventType_TouchMoved;
					break;
				default:
					DS_ASSERT(false);
					break;
			}
			outEvent->touch.window = findWindow(application, sdlEvent->tfinger.windowID);
			if (!outEvent->touch.window)
				outEvent->touch.window = focusWindow;
			outEvent->touch.gameInput = NULL;
			outEvent->touch.touchID = sdlEvent->tfinger.touchID;
			outEvent->touch.fingerID = sdlEvent->tfinger.fingerID;
			outEvent->touch.position.x = sdlEvent->tfinger.x;
			outEvent->touch.position.y = sdlEvent->tfinger.y;
			outEvent->touch.delta.x = sdlEvent->tfinger.dx;
			outEvent->touch.delta.y = sdlEvent->tfinger.dy;
			outEvent->touch.pressure = sdlEvent->tfinger.pressure;
			return true;
		case SDL_EVENT_SENSOR_UPDATE:
			outEvent->type = dsAppEventType_MotionSensor;
			outEvent->motionSensor.sensor = dsSDLMotionSensor_find(
				application, sdlEvent->sensor.which);
			if (!outEvent->motionSensor.sensor)
				return false;

			outEvent->motionSensor.window = NULL;
			outEvent->motionSensor.gameInput = NULL;
			outEvent->motionSensor.type = outEvent->motionSensor.sensor->type;
			memcpy(&outEvent->motionSensor.data, sdlEvent->sensor.data, sizeof(dsVector3f));
			return true;
		case SDL_EVENT_USER:
			outEvent->type = dsAppEventType_Custom;
			outEvent->custom.eventID = sdlEvent->user.code;
			outEvent->custom.window = findWindow(application, sdlEvent->user.windowID);
			outEvent->custom.userData = sdlEvent->user.data1;
			outEvent->custom.cleanupFunc = (dsCustomEventCleanupFunction)sdlEvent->user.data2;
			return true;
		default:
			return false;
	}
}

static void finalizeEvents(dsApplication* application)
{
	dsSDLApplication* sdlApplication = (dsSDLApplication*)application;
	DS_ASSERT(sdlApplication->hasFrameEvents);

	// Sanity check for any windows that might have a missing display or render surface size
	// change didn't get caught by other events.
	for (uint32_t i = 0; i < application->windowCount; ++i)
	{
		dsWindow* window = application->windows[i];
		dsSDLWindow* sdlWindow = (dsSDLWindow*)window;
		const dsRenderSurface* surface = window->surface;
		if (!window->display || (surface && (sdlWindow->curSurfaceWidth != surface->width ||
			sdlWindow->curSurfaceHeight != surface->height ||
			sdlWindow->curSurfaceRotation != surface->rotation)))
		{
			dsEvent event;
			event.time = dsTimer_currentTicks();
			if (updateWindowState(&event, application, window, false))
				dsApplication_dispatchEvent(application, &event);
		}
	}

	DS_PROFILE_SCOPE_END();

	// Clear out for the next frame.
	sdlApplication->hasFrameEvents = false;
}

static void updateWindowSamples(dsApplication* application, uint64_t eventTime)
{
	if (application->windowCount == 0)
		return;

	bool setSamples = false;
	for (unsigned int i = 0; i < application->windowCount; ++i)
	{
		dsWindow* window = application->windows[i];
		dsSDLWindow* sdlWindow = (dsSDLWindow*)window;
		if (sdlWindow->samples != application->renderer->surfaceSamples)
			setSamples = true;
	}

	if (!setSamples)
		return;

	// Make absolutely sure that the state is fully up to date. Cache the change flags in the window
	// flag upper bits so events can be changed afterward.
	dsWindow* focusWindow = dsSDLWindow_getFocusWindow(application);
	for (uint32_t i = 0; i < application->windowCount; ++i)
	{
		dsWindow* window = application->windows[i];
		SDL_SyncWindow(((dsSDLWindow*)window)->sdlWindow);
		dsEvent dummyEvent;
		if (updateWindowState(&dummyEvent, application, window, false))
			window->flags = window->flags | (dummyEvent.windowChange.flags << CACHED_CHANGE_SHIFT);

		// If the surface creation was delayed and it still hasn't been created, keep track of that
		// state.
		if (!window->surface)
			window->flags |= dsWindowFlags_DelaySurfaceCreate;
	}

	if (application->renderer->surfaceSamples > 1)
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, application->renderer->surfaceSamples);
	}
	else
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
	}

	// Need to destroy the SDL windows before restarting video for X11 below.
	for (unsigned int i = 0; i < application->windowCount; ++i)
		dsSDLWindow_destroyComponents(application->windows[i]);

#if !DS_WINDOWS && !DS_APPLE && !DS_ANDROID
	dsRenderer* renderer = application->renderer;
	if (renderer->platform != dsGfxPlatform_Wayland && renderer->surfaceConfig)
	{
		// Need to update visual ID.
		char visualId[20];
		snprintf(visualId, sizeof(visualId), "%d", (int)(size_t)renderer->surfaceConfig);
		SDL_SetHintWithPriority(SDL_HINT_VIDEO_X11_WINDOW_VISUALID, visualId, SDL_HINT_OVERRIDE);
	}
#endif

	// Re-create the windows with the new samples.
	for (unsigned int i = 0; i < application->windowCount; ++i)
	{
		dsWindow* window = application->windows[i];

		// Pull out the cached change flags so we know what may have changed since the events were
		// processed.
		dsWindowChangeFlags changeFlags =
			(window->flags & CACHED_CHANGE_MASK) >> CACHED_CHANGE_SHIFT;
		window->flags &= ~CACHED_CHANGE_MASK;

		// Flag to delay surface creation shouldn't be kept beyond the cached state.
		dsWindowFlags componentFlags = window->flags;
		window->flags &= ~dsWindowFlags_DelaySurfaceCreate;

		// If the surface creation was delayed and not created yet,
		if (!dsSDLWindow_createComponents(
				window, &window->position, window->width, window->height, componentFlags))
		{
			DS_LOG_FATAL_F(
				DS_APPLICATION_SDL_LOG_TAG, "Couldn't allocate window: %s", dsErrorString(errno));
			abort();
		}

		dsEvent event;
		event.time = eventTime;
		if (!(componentFlags & dsWindowFlags_DelaySurfaceCreate))
		{
			event.type = dsAppEventType_SurfaceInvalidated;
			event.window = window;
			dsApplication_dispatchEvent(application, &event);
		}

		// The window may not have been exactly what was requested on re-creation.
		bool newUpdates = updateWindowState(&event, application, window, false);
		if (newUpdates || changeFlags != 0)
		{
			if (newUpdates)
				event.windowChange.flags |= changeFlags;
			else
			{
				event.type = dsAppEventType_WindowChanged;
				event.windowChange.window = window;
				event.windowChange.flags = changeFlags;
			}
			dsApplication_dispatchEvent(application, &event);
		}
	}

	if (focusWindow)
		dsSDLWindow_raise(application, focusWindow);
}

static void finishFrame(dsApplication* application)
{
	DS_VERIFY(dsRenderer_endFrame(application->renderer));
}

bool dsSDLApplication_setUpdateRate(dsApplication* application, float updateRate)
{
	char buffer[20];
	const char* value;
	if (updateRate < 0.0f)
		value = "waitevent";
	else if (updateRate == 0.0f)
		value = NULL;
	else
	{
		int written = snprintf(buffer, sizeof(buffer), "%.9g", updateRate);
		// Should be guaranteed to succeed, but just to be safe.
		if ((size_t)written >= sizeof(buffer))
		{
			errno = EINVAL;
			return false;
		}

		value = buffer;
	}

	SDL_SetHintWithPriority(SDL_HINT_MAIN_CALLBACK_RATE, value, SDL_HINT_OVERRIDE);
	application->updateRate = updateRate;
	return true;
}

uint32_t dsSDLApplication_showMessageBoxBase(dsApplication* application,
	dsWindow* parentWindow, dsMessageBoxType type, const char* title, const char* message,
	const char* const* buttons, uint32_t buttonCount, uint32_t enterButton, uint32_t escapeButton)
{
	DS_UNUSED(application);
	SDL_Window* sdlWindow = parentWindow ? ((dsSDLWindow*)parentWindow)->sdlWindow : NULL;
	return showMessageBoxImpl(
		sdlWindow, type, title, message, buttons, buttonCount, enterButton, escapeButton);
}

bool dsSDLApplication_prepareRendererOptions(dsRendererOptions* options, uint32_t rendererID)
{
	options->platform = dsRenderer_resolvePlatform(options->platform);
	// Only need special render surface handling for OpenGL on Wayland.
	if ((rendererID != DS_GL_RENDERER_ID && rendererID != DS_GLES_RENDERER_ID) ||
		options->platform != dsGfxPlatform_Wayland)
	{
		return true;
	}

	SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "wayland", SDL_HINT_OVERRIDE);
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
	{
		DS_LOG_ERROR_F(
			DS_APPLICATION_SDL_LOG_TAG, "Couldn't initialize SDL video: %s", SDL_GetError());
		SDL_Quit();
		errno = EPERM;
		return false;
	}

	options->backgroundSurfaceType = dsRenderSurfaceType_Window;
	options->osDisplay = SDL_GetPointerProperty(
		SDL_GetGlobalProperties(), SDL_PROP_GLOBAL_VIDEO_WAYLAND_WL_DISPLAY_POINTER, NULL);
	options->createBackgroundSurfaceFunc = &createBackgroundGLWindow;
	options->destroyBackgroundSurfaceFunc = &destroyBackgroundGLWindow;
	options->getBackgroundSurfaceHandleFunc = &getWaylandGLWindowHandle;
	return true;
}

void dsSDLApplication_quit(dsApplication* application, int exitCode)
{
	dsSDLApplication* sdlApplication = (dsSDLApplication*)application;
	sdlApplication->quit = true;
	sdlApplication->exitCode = exitCode;
}

bool dsSDLApplication_addCustomEvent(dsApplication* application, const dsCustomEvent* event)
{
	DS_UNUSED(application);
	DS_ASSERT(event);
	SDL_Event userEvent;
	userEvent.type = SDL_EVENT_USER;
	if (event->window)
		userEvent.user.windowID = SDL_GetWindowID(((dsSDLWindow*)event->window)->sdlWindow);
	else
		userEvent.user.windowID = 0;
	userEvent.user.code = event->eventID;
	userEvent.user.data1 = event->userData;
	userEvent.user.data2 = event->cleanupFunc;

	if (!SDL_PushEvent(&userEvent))
	{
		DS_LOG_ERROR_F(DS_APPLICATION_SDL_LOG_TAG, "Couldn't push event: %s", SDL_GetError());
		errno = EPERM;
		return false;
	}
	return true;
}

dsSystemPowerState dsSDLApplication_getPowerState(int* outRemainingTime, int* outBatteryPercent,
	const dsApplication* application)
{
	DS_UNUSED(application);
	switch (SDL_GetPowerInfo(outRemainingTime, outBatteryPercent))
	{
		case SDL_POWERSTATE_ERROR:
		case SDL_POWERSTATE_UNKNOWN:
			return dsSystemPowerState_Unknown;
		case SDL_POWERSTATE_ON_BATTERY:
			return dsSystemPowerState_OnBattery;
		case SDL_POWERSTATE_NO_BATTERY:
			return dsSystemPowerState_External;
		case SDL_POWERSTATE_CHARGING:
			return dsSystemPowerState_Charging;
		case SDL_POWERSTATE_CHARGED:
			return dsSystemPowerState_Charged;
	}

	DS_ASSERT(false);
	return dsSystemPowerState_Unknown;
}

dsCursor dsSDLApplication_getCursor(const dsApplication* application)
{
	return ((const dsSDLApplication*)application)->curCursor;
}

bool dsSDLApplication_setCursor(dsApplication* application, dsCursor cursor)
{
	if ((unsigned int)cursor >= (unsigned int)dsCursor_Count)
	{
		errno = EINDEX;
		return false;
	}

	dsSDLApplication* sdlApplication = (dsSDLApplication*)application;
	sdlApplication->curCursor = cursor;
	SDL_SetCursor(sdlApplication->cursors[cursor]);
	return true;
}

bool dsSDLApplication_getCursorHidden(const dsApplication* application)
{
	DS_UNUSED(application);
	return SDL_CursorVisible();
}

bool dsSDLApplication_setCursorHidden(dsApplication* application, bool hidden)
{
	DS_UNUSED(application);
	if (hidden)
		SDL_HideCursor();
	else
		SDL_ShowCursor();
	return true;
}

bool dsSDLApplication_isKeyPressed(const dsApplication* application, dsKeyCode key)
{
	DS_UNUSED(application);
	return SDL_GetKeyboardState(NULL)[dsToSDLScancode(key)];
}

dsKeyModifier dsSDLApplication_getKeyModifiers(const dsApplication* application)
{
	DS_UNUSED(application);
	return dsFromSDLKeyMod(SDL_GetModState());
}

bool dsSDLApplication_getMousePosition(dsVector2f* outPosition, const dsApplication* application)
{
	DS_UNUSED(application);
	SDL_GetMouseState(&outPosition->x, &outPosition->y);
	return true;
}

bool dsSDLApplication_setMousePosition(
	dsApplication* application, dsWindow* window, const dsVector2f* position)
{
	DS_UNUSED(application);
	if (window)
		SDL_WarpMouseInWindow(((dsSDLWindow*)window)->sdlWindow, position->x, position->y);
	else
	{
		if (!SDL_WarpMouseGlobal(position->x, position->y))
		{
			errno = EPERM;
			return false;
		}
	}

	return true;
}

uint32_t dsSDLApplication_getPressedMouseButtons(const dsApplication* application)
{
	DS_UNUSED(application);
	uint32_t sdlButtons = SDL_GetMouseState(NULL, NULL);
	return SDL_MOUSE_TO_DS_MOUSE_MASK(sdlButtons);
}

#if DS_ANDROID
bool dsSDLApplication_requestAndroidPermission(dsApplication* application, const char* permission,
	dsHandleAndroidPermissionResultFunction resultFunc, void* userData)
{
	DS_UNUSED(application);
	return SDL_RequestAndroidPermission(permission, resultFunc, userData);
}
#endif

void dsSDLApplication_destroy(dsApplication* application)
{
	if (!application)
		return;

	dsDestroyUserDataFunction finalizerFunc = application->finalizerFunc;
	void* finalizerUserData = application->userData;
	dsApplication_destroyUserData(application);

	if (application->displays)
	{
		for (uint32_t i = 0; i < application->displayCount; ++i)
			DS_VERIFY(dsAllocator_free(application->allocator, application->displays[i]));
	}

	for (uint32_t i = 0; i < application->windowCount; ++i)
		dsSDLWindow_destroy(application, application->windows[i]);

	dsSDLApplication* sdlApplication = (dsSDLApplication*)application;
	for (int i = 0; i < dsCursor_Count; ++i)
	{
		if (sdlApplication->cursors[i])
			SDL_DestroyCursor(sdlApplication->cursors[i]);
	}

	dsSDLGameInput_freeAll(application->gameInputs, application->gameInputCount);
	dsSDLMotionSensor_freeAll(application->motionSensors, application->motionSensorCount);
	dsApplication_shutdown(application);
	DS_VERIFY(dsRenderer_destroy(application->renderer));
	dsAllocator_free(application->allocator, application);

	SDL_Quit();

	if (finalizerFunc)
		finalizerFunc(finalizerUserData);
}

uint32_t dsSDLApplication_showMessageBox(dsMessageBoxType type, const char* title,
	const char* message, const char* const* buttons, uint32_t buttonCount, uint32_t enterButton,
	uint32_t escapeButton)
{
	if (!title || !message || !buttons || buttonCount == 0 ||
		buttonCount > DS_MAX_MESSAGE_BOX_BUTTONS ||
		(enterButton != DS_MESSAGE_BOX_NO_BUTTON && enterButton >= buttonCount) ||
		(escapeButton != DS_MESSAGE_BOX_NO_BUTTON && escapeButton >= buttonCount))
	{
		errno = EINVAL;
		return DS_MESSAGE_BOX_NO_BUTTON;
	}

	return showMessageBoxImpl(
		NULL, type, title, message, buttons, buttonCount, enterButton, escapeButton);
}

dsApplication* dsSDLApplication_create(dsAllocator* allocator, dsRenderer* renderer, int argc,
	const char* const* argv, const char* orgName, const char* appName, dsSDLApplicationFlags flags)
{
	DS_UNUSED(argc);
	DS_UNUSED(argv);
	if (!allocator || !renderer)
	{
		if (renderer)
			DS_VERIFY(dsRenderer_destroy(renderer));
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_APPLICATION_SDL_LOG_TAG,
			"Application allocator must support freeing memory.");
		DS_VERIFY(dsRenderer_destroy(renderer));
		errno = EINVAL;
		return NULL;
	}

	uint32_t initFlags = SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC;
	if (flags & dsSDLApplicationFlags_MotionSensors)
		initFlags |= SDL_INIT_SENSOR;
	if (!SDL_Init(initFlags))
	{
		DS_LOG_ERROR_F(DS_APPLICATION_SDL_LOG_TAG, "Couldn't initialize SDL: %s", SDL_GetError());
		DS_VERIFY(dsRenderer_destroy(renderer));
		errno = EPERM;
		return NULL;
	}

	const char* driver = NULL;
#if DS_WINDOWS
	driver = "windows";
#elif DS_MAC
	driver = "cocoa";
#elif DS_IOS
	driver = "uikit";
#elif !DS_ANDROID
	if (renderer->platform == dsGfxPlatform_Wayland)
	{
		SDL_SetHint(SDL_HINT_VIDEO_WAYLAND_MODE_SCALING, "aspect");
		driver = "wayland";
	}
	else
	{
		SDL_SetHintWithPriority(SDL_HINT_VIDEO_X11_NODIRECTCOLOR, "1", SDL_HINT_OVERRIDE);

		const char* compositorSetting = flags & dsSDLApplicationFlags_DisableCompositor ? "1" : "0";
		SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, compositorSetting);

		if (renderer->surfaceConfig)
		{
			char visualID[20];
			snprintf(visualID, sizeof(visualID), "%d", (int)(size_t)renderer->surfaceConfig);
			SDL_SetHintWithPriority(
				SDL_HINT_VIDEO_X11_WINDOW_VISUALID, visualID, SDL_HINT_OVERRIDE);
		}
		driver = "x11";
	}
#endif

	if (driver)
		SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, driver, SDL_HINT_OVERRIDE);

	// May have already been initialized when setting up renderer options.
	const char* curDriver = SDL_GetCurrentVideoDriver();
	bool shouldInitVideo = !curDriver || strcmp(curDriver, driver) != 0;
	if (shouldInitVideo && !SDL_InitSubSystem(SDL_INIT_VIDEO))
	{
		DS_LOG_ERROR_F(
			DS_APPLICATION_SDL_LOG_TAG, "Couldn't initialize SDL video: %s", SDL_GetError());
		DS_VERIFY(dsRenderer_destroy(renderer));
		SDL_Quit();
		errno = EPERM;
		return NULL;
	}
	dsRenderer_restoreGlobalState(renderer);

	if (renderer->rendererID == DS_GL_RENDERER_ID ||
		renderer->rendererID == DS_GLES_RENDERER_ID)
	{
		if (!setGLAttributes(renderer))
		{
			DS_LOG_ERROR(DS_APPLICATION_SDL_LOG_TAG, "Invalid renderer attributes.");
			DS_VERIFY(dsRenderer_destroy(renderer));
			SDL_Quit();
			errno = EINVAL;
			return NULL;
		}
	}

	int displayCount;
	SDL_DisplayID* displayIDs = SDL_GetDisplays(&displayCount);
	if (!displayIDs)
	{
		DS_LOG_ERROR_F(DS_APPLICATION_SDL_LOG_TAG, "Couldn't get SDL displays: %s", SDL_GetError());
		DS_VERIFY(dsRenderer_destroy(renderer));
		SDL_Quit();
		errno = EPERM;
		return NULL;
	}

	dsSDLApplication* application = DS_ALLOCATE_OBJECT(allocator, dsSDLApplication);
	if (!application)
	{
		DS_VERIFY(dsRenderer_destroy(renderer));
		SDL_free(displayIDs);
		SDL_Quit();
		return NULL;
	}

	application->useMotionSensors = (flags & dsSDLApplicationFlags_MotionSensors) != 0;
	application->quit = false;
	application->hasFrameEvents = false;
	application->exitCode = 0;
	memset(application->cursors, 0, sizeof(application->cursors));
	application->inputTickRef = 0;
	application->inputNSRef = 0;
	application->lastFrameTicks = dsTimer_currentTicks();

	dsApplication* baseApplication = (dsApplication*)application;
	DS_VERIFY(dsApplication_initialize(baseApplication, allocator));
	baseApplication->renderer = renderer;

	if (!DS_RESIZEABLE_ARRAY_ADD(allocator, baseApplication->displays,
			baseApplication->displayCount, baseApplication->displayCapacity, displayCount))
	{
		SDL_free(displayIDs);
		dsSDLApplication_destroy(baseApplication);
		return NULL;
	}

	SDL_DisplayID primaryDisplayID = SDL_GetPrimaryDisplay();
	for (int i = 0; i < displayCount; ++i)
	{
		dsDisplayInfo* display = createDisplay(allocator, displayIDs[i]);
		if (!display)
		{
			SDL_free(displayIDs);
			baseApplication->displayCount = i;
			dsSDLApplication_destroy(baseApplication);
			return NULL;
		}

		baseApplication->displays[i] = display;
		if (primaryDisplayID == display->id)
			baseApplication->primaryDisplay = display;
	}
	SDL_free(displayIDs);

	application->cursors[dsCursor_Arrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
	application->cursors[dsCursor_IBeam] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
	application->cursors[dsCursor_Wait] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
	application->cursors[dsCursor_Crosshair] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	application->cursors[dsCursor_WaitArrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_PROGRESS);
	application->cursors[dsCursor_SizeTLBR] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NWSE_RESIZE);
	application->cursors[dsCursor_SizeTRBL] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NESW_RESIZE);
	application->cursors[dsCursor_SizeTB] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);
	application->cursors[dsCursor_SizeLR] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
	application->cursors[dsCursor_SizeT] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_N_RESIZE);
	application->cursors[dsCursor_SizeB] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_S_RESIZE);
	application->cursors[dsCursor_SizeL] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_W_RESIZE);
	application->cursors[dsCursor_SizeR] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_E_RESIZE);
	application->cursors[dsCursor_Move] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
	application->cursors[dsCursor_No] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NOT_ALLOWED);
	application->cursors[dsCursor_Hand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
	application->curCursor = dsCursor_Arrow;

	if (!dsSDLGameInput_setup(baseApplication))
	{
		dsSDLApplication_destroy(baseApplication);
		return NULL;
	}

	if (application->useMotionSensors && !dsSDLMotionSensor_setup(baseApplication))
	{
		dsSDLApplication_destroy(baseApplication);
		return NULL;
	}

	baseApplication->setUpdateRateFunc = &dsSDLApplication_setUpdateRate;
	baseApplication->showMessageBoxFunc = &dsSDLApplication_showMessageBoxBase;
	baseApplication->quitFunc = &dsSDLApplication_quit;
	baseApplication->addCustomEventFunc = &dsSDLApplication_addCustomEvent;
	baseApplication->getPowerStateFunc = &dsSDLApplication_getPowerState;

	baseApplication->getCursorFunc = &dsSDLApplication_getCursor;
	baseApplication->setCursorFunc = &dsSDLApplication_setCursor;
	baseApplication->getCursorHiddenFunc = &dsSDLApplication_getCursorHidden;
	baseApplication->setCursorHiddenFunc = &dsSDLApplication_setCursorHidden;
	baseApplication->isKeyPressedFunc = &dsSDLApplication_isKeyPressed;
	baseApplication->getKeyModifiersFunc = &dsSDLApplication_getKeyModifiers;
	baseApplication->getMousePositionFunc = &dsSDLApplication_getMousePosition;
	baseApplication->setMousePositionFunc = &dsSDLApplication_setMousePosition;
	baseApplication->getPressedMouseButtonsFunc = &dsSDLApplication_getPressedMouseButtons;
#if DS_ANDROID
	baseApplication->requestAndroidPermissionFunc = &dsSDLApplication_requestAndroidPermission;
#endif
	baseApplication->destroyFunc = &dsSDLApplication_destroy;

	baseApplication->createWindowFunc = &dsSDLWindow_create;
	baseApplication->destroyWindowFunc = &dsSDLWindow_destroy;
	baseApplication->createWindowSurfaceFunc = &dsSDLWindow_createSurface;
	baseApplication->getFocusWindowFunc = &dsSDLWindow_getFocusWindow;
	baseApplication->setWindowTitleFunc = &dsSDLWindow_setTitle;
	baseApplication->setWindowDisplayModeFunc = &dsSDLWindow_setDisplayMode;
	baseApplication->resizeWindowFunc = &dsSDLWindow_resize;
	baseApplication->setWindowStyleFunc = &dsSDLWindow_setStyle;
	baseApplication->setWindowPositionFunc = &dsSDLWindow_setPosition;
	baseApplication->centerWindowFunc = &dsSDLWindow_center;
	baseApplication->setWindowHiddenFunc = &dsSDLWindow_setHidden;
	baseApplication->minimizeWindowFunc = &dsSDLWindow_minimize;
	baseApplication->maximizeWindowFunc = &dsSDLWindow_maximize;
	baseApplication->restoreWindowFunc = &dsSDLWindow_restore;
	baseApplication->setWindowGrabbedInputFunc = &dsSDLWindow_setGrabbedInput;
	baseApplication->setWindowResizableFunc = &dsSDLWindow_setResizable;
	baseApplication->raiseWindowFunc = &dsSDLWindow_raise;
	baseApplication->beginTextInputFunc = &dsSDLWindow_beginTextInput;
	baseApplication->endTextInputFunc = &dsSDLWindow_endTextInput;
	baseApplication->setTextInputAreaFunc = &dsSDLWindow_setTextInputArea;

	baseApplication->getGameInputPowerStateFunc = &dsSDLGameInput_getPowerState;
	baseApplication->getGameInputAxisFunc = &dsSDLGameInput_getAxis;
	baseApplication->getGameInputControllerAxisFunc = &dsSDLGameInput_getControllerAxis;
	baseApplication->isGameInputButtonPressedFunc = &dsSDLGameInput_isButtonPressed;
	baseApplication->isGameInputControllerButtonPressedFunc =
		&dsSDLGameInput_isControllerButtonPressed;
	baseApplication->getGameInputDPadDirectionFunc = &dsSDLGameInput_getDPadDirection;
	baseApplication->setGameInputBaselineRumbleFunc = &dsSDLGameInput_setBaselineRumble;
	baseApplication->getGameInputBaselineRumbleFunc = &dsSDLGameInput_getBaselineRumble;
	baseApplication->setGameInputTimedRumbleFunc = &dsSDLGameInput_setTimedRumble;
	baseApplication->getGameInputTimedRumbleFunc = &dsSDLGameInput_getTimedRumble;
	baseApplication->setGameInputLEDColorFunc = &dsSDLGameInput_setLEDColor;
	baseApplication->setGameInputPlayerFunc = &dsSDLGameInput_setPlayer;
	baseApplication->gameInputHasMotionSensorFunc = &dsSDLGameInput_hasMotionSensor;
	baseApplication->getGameInputMotionSensorDataFunc = &dsSDLGameInput_getMotionSensorData;

	baseApplication->getMotionSensorDataFunc = &dsSDLMotionSensor_getData;

#if DS_ANDROID
	DS_UNUSED(orgName);
	DS_UNUSED(appName);
	dsResourceStream_setContext(SDL_GetAndroidJNIEnv(), SDL_GetAndroidActivity(), "",
		SDL_GetAndroidInternalStoragePath(), SDL_GetAndroidExternalStoragePath());
#else
	const char* basePath = SDL_GetBasePath();
	char* prefPath = SDL_GetPrefPath(orgName, appName);
	if (!prefPath)
	{
		DS_LOG_ERROR(DS_APPLICATION_SDL_LOG_TAG, "Couldn't create preference path.");
		basePath = NULL;
	}
	dsResourceStream_setContext(NULL, NULL, basePath, basePath, prefPath);
	SDL_free(prefPath);
#endif

	return baseApplication;
}

bool dsSDLApplication_useMotionSensors(const dsApplication* application)
{
	return application && ((const dsSDLApplication*)application)->useMotionSensors;
}

bool dsSDLApplication_iterate(dsApplication* application)
{
	dsSDLApplication* sdlApplication = (dsSDLApplication*)application;
	initializeFrame(application);
	finalizeEvents(application);
	if (sdlApplication->quit)
	{
		finishFrame(application);
		return false;
	}

	// Functions above may block if the app is paused, so get the current time here.
	uint64_t curTicks = dsTimer_currentTicks();
	uint64_t frameTicks = curTicks - sdlApplication->lastFrameTicks;
	sdlApplication->lastFrameTicks = curTicks;

	// Update game inputs, primarily to maintain the rumble state.
	for (uint32_t i = 0; i < application->gameInputCount; ++i)
		dsSDLGameInput_update(application->gameInputs[i], frameTicks);

	if (application->updateFunc)
	{
		DS_PROFILE_SCOPE_START("Update");
		application->updateFunc(application, curTicks, frameTicks, application->updateUserData);
		DS_PROFILE_SCOPE_END();
	}

	// Quit may have been requested during the update.
	if (sdlApplication->quit)
	{
		finishFrame(application);
		return false;
	}

	// If the samples have changed, need to re-create the windows. Do between update and draw
	// since update is most likely to have changed the samples.
	updateWindowSamples(application, dsTimer_currentTicks());

	DS_PROFILE_SCOPE_START("Draw");
	uint32_t swapSurfaceCount = 0;
	dsRenderSurface* swapSurfaces[MAX_SWAP_WINDOWS];

	dsCommandBuffer* commandBuffer = application->renderer->mainCommandBuffer;
	for (uint32_t i = 0; i < application->windowCount; ++i)
	{
		dsWindow* window = application->windows[i];
		if (!window->drawFunc || !window->surface)
			continue;

		if (!dsRenderSurface_beginDraw(window->surface, commandBuffer))
			continue;

		window->drawFunc(application, window, window->drawUserData);
		dsRenderSurface_endDraw(window->surface, commandBuffer);
		swapSurfaces[swapSurfaceCount++] = window->surface;

		// Flush between windows. This avoids render commands for multiple windows being batched
		// together, allowing for render commands to be executed on the GPU sooner.
		// Force a swap buffers if we've exceeded the maximum number of windows to swap at once.
		if (swapSurfaceCount >= MAX_SWAP_WINDOWS)
		{
			dsRenderSurface_swapBuffers(swapSurfaces, swapSurfaceCount);
			swapSurfaceCount = 0;
		}
		else if (i < application->windowCount - 1)
			dsRenderer_flush(application->renderer);
	}
	DS_PROFILE_SCOPE_END();

	if (application->finishFrameFunc)
	{
		DS_PROFILE_SCOPE_START("Finish Frame");
		application->finishFrameFunc(application, application->finishFrameUserData);
		DS_PROFILE_SCOPE_END();
	}

	// Swap the buffers for all the window surfaces at the end.
	dsRenderSurface_swapBuffers(swapSurfaces, swapSurfaceCount);

	finishFrame(application);
	return !sdlApplication->quit;
}

bool dsSDLApplication_event(dsApplication* application, SDL_Event* sdlEvent)
{
	dsSDLApplication* sdlApplication = (dsSDLApplication*)application;
	initializeFrame(application);

	if (sdlEvent->type == SDL_EVENT_QUIT || sdlEvent->type == SDL_EVENT_TERMINATING)
		return false;

	dsEvent event;

	// Convert timestamp to ticks.
	int64_t relativeEventNS = ((SDL_CommonEvent*)sdlEvent)->timestamp - sdlApplication->inputNSRef;
	double relativeEventTime = (double)relativeEventNS*1e-9;
	event.time = sdlApplication->inputTickRef +
		dsTimer_secondsToTicks(application->timer, relativeEventTime);

	dsWindow* focusWindow = dsSDLWindow_getFocusWindow(application);
	const dsDisplayInfo* prevPrimaryDisplay = application->primaryDisplay;
	if (!convertEvent(&event, application, focusWindow, sdlEvent))
		return true;

	dsApplication_dispatchEvent(application, &event);

	// Must dispatch primary display change after the original event.
	if (application->primaryDisplay != prevPrimaryDisplay)
	{
		event.type = dsAppEventType_PrimaryDisplayChanged;
		event.display = application->primaryDisplay;
		dsApplication_dispatchEvent(application, &event);
	}

	// Some events require cleanup.
	if (sdlEvent->type == SDL_EVENT_DISPLAY_REMOVED)
	{
		// Sanity check: clear out any windows that reference the removed display.
		for (uint32_t i = 0; i < application->windowCount; ++i)
		{
			dsWindow* window = application->windows[i];
			if (window->display && window->display->id == sdlEvent->display.displayID)
				window->display = NULL;
		}

		for (uint32_t i = 0; i < application->displayCount; ++i)
		{
			dsDisplayInfo* display = application->displays[i];
			if (display->id == sdlEvent->display.displayID)
			{
				// Constant-time removal.
				application->displays[i] =
					application->displays[application->displayCount - 1];
				--application->displayCount;
				DS_VERIFY(dsAllocator_free(application->allocator, display));
				break;
			}
		}
	}
	else if (sdlEvent->type == SDL_EVENT_WINDOW_DESTROYED)
		dsWindow_destroy(findWindow(application, sdlEvent->window.windowID));
	else if (sdlEvent->type == SDL_EVENT_JOYSTICK_REMOVED)
		DS_VERIFY(dsSDLGameInput_remove(application, sdlEvent->jdevice.which));
	else if (sdlEvent->type == SDL_EVENT_USER && sdlEvent->user.data2)
	{
		((dsCustomEventCleanupFunction)sdlEvent->user.data2)(
			sdlEvent->user.code, sdlEvent->user.data1);
	}

	return !sdlApplication->quit;
}
