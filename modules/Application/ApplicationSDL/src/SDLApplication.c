/*
 * Copyright 2017-2021 Aaron Barany
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

#include "SDLController.h"
#include "SDLKeyboard.h"
#include "SDLShared.h"
#include "SDLWindow.h"

#include <DeepSea/Application/Application.h>
#include <DeepSea/Application/Window.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/Timer.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/RenderSurface.h>

#include <math.h>
#include <SDL.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define DS_MAX_WINDOWS 100U

typedef struct dsSDLApplication
{
	dsApplication application;

	bool quit;
	int exitCode;
	SDL_Cursor* cursors[dsCursor_Count];
	dsCursor curCursor;
} dsSDLApplication;

static uint32_t showMessageBoxImpl(SDL_Window* parentWindow, dsMessageBoxType type,
	const char* title, const char* message, const char** buttons, uint32_t buttonCount,
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

	SDL_MessageBoxButtonData* buttonData = DS_ALLOCATE_STACK_OBJECT_ARRAY(SDL_MessageBoxButtonData,
		buttonCount);
	for (uint32_t i = 0; i < buttonCount; ++i)
	{
		buttonData[i].flags = 0;
		if (i == enterButton)
			buttonData[i].flags |= SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
		if (i == escapeButton)
			buttonData[i].flags |= SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
		buttonData[i].buttonid = i;
		buttonData[i].text = buttons[i];
	}
	messageBox.buttons = buttonData;

	int buttonId = 0;
	if (SDL_ShowMessageBox(&messageBox, &buttonId) != 0)
	{
		errno = EINVAL;
		return DS_MESSAGE_BOX_NO_BUTTON;
	}

	return buttonId;
}

static dsWindow* findWindow(dsApplication* application, uint32_t windowId)
{
	for (uint32_t i = 0; i < application->windowCount; ++i)
	{
		if (SDL_GetWindowID(((dsSDLWindow*)application->windows[i])->sdlWindow) == windowId)
			return application->windows[i];
	}

	return NULL;
}

static dsController* findController(dsApplication* application, SDL_JoystickID joystickId)
{
	for (uint32_t i = 0; i < application->controllerCount; ++i)
	{
		if (SDL_JoystickInstanceID(((dsSDLController*)application->controllers[i])->joystick) ==
			joystickId)
		{
			return application->controllers[i];
		}
	}

	return NULL;
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

static void updateWindowSamples(dsApplication* application)
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

	// Cache existing window values.
	for (unsigned int i = 0; i < application->windowCount; ++i)
	{
		dsWindow* window = application->windows[i];
		dsSDLWindow* sdlWindow = (dsSDLWindow*)window;

		setSamples = true;
		dsSDLWindow_getSize(&sdlWindow->curWidth, &sdlWindow->curHeight, application, window);
		dsSDLWindow_getPosition(&sdlWindow->curPosition, application, window);

		sdlWindow->curFlags = dsWindowFlags_DelaySurfaceCreate;
		if (dsSDLWindow_getHidden(application, window))
			sdlWindow->curFlags |= dsWindowFlags_Hidden;
		if (SDL_GetWindowFlags(sdlWindow->sdlWindow) & SDL_WINDOW_RESIZABLE)
			sdlWindow->curFlags |= dsWindowFlags_Resizeable;
		if (dsSDLWindow_getMinimized(application, window))
			sdlWindow->curFlags |= dsWindowFlags_Minimized;
		if (dsSDLWindow_getMaximized(application, window))
			sdlWindow->curFlags |= dsWindowFlags_Maximized;
		if (dsSDLWindow_getGrabbedInput(application, window))
			sdlWindow->curFlags |= dsWindowFlags_GrabInput;
		sdlWindow->hasFocus = dsSDLWindow_getFocusWindow(application) == window;
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
	setSamples = true;

	// Need to destroy the SDL windows before restarting video for X11 below.
	for (unsigned int i = 0; i < application->windowCount; ++i)
		dsSDLWindow_destroyComponents(application->windows[i]);

#if DS_LINUX && !DS_ANDROID
	dsRenderer* renderer = application->renderer;
	if (renderer->platform != dsGfxPlatform_Wayland && renderer->surfaceConfig)
	{
		// Need to restart video on X11 for new visual ID.
		SDL_VideoQuit();
		char visualId[20];
		snprintf(visualId, sizeof(visualId), "%d", (int)(size_t)renderer->surfaceConfig);
		setenv("SDL_VIDEO_X11_VISUALID", visualId, true);
		SDL_VideoInit("x11");

		// Windows were destroyed.
		for (unsigned int i = 0; i < application->windowCount; ++i)
		{
			dsWindow* window = application->windows[i];
			dsSDLWindow* sdlWindow = (dsSDLWindow*)window;
			sdlWindow->sdlWindow = NULL;
		}
	}
#endif

	// Re-create the windows with the new samples.
	for (unsigned int i = 0; i < application->windowCount; ++i)
	{
		dsWindow* window = application->windows[i];
		dsSDLWindow* sdlWindow = (dsSDLWindow*)window;

		const char* title = window->title;
		const char* surfaceName = sdlWindow->surfaceName;
		dsDisplayMode displayMode = window->displayMode;
		dsWindowStyle style = window->style;

		if (!dsSDLWindow_createComponents(window, title, surfaceName, &sdlWindow->curPosition,
				sdlWindow->curWidth, sdlWindow->curHeight, sdlWindow->curFlags))
		{
			DS_LOG_FATAL_F(DS_APPLICATION_SDL_LOG_TAG, "Couldn't allocate window: %s",
				dsErrorString(errno));
			abort();
		}

		DS_VERIFY(dsSDLWindow_setDisplayMode(application, window, &displayMode));
		if (style != dsWindowStyle_Normal)
			DS_VERIFY(dsSDLWindow_setStyle(application, window, style));

		if (!dsSDLWindow_createSurfaceInternal(window, sdlWindow->surfaceName))
		{
			DS_LOG_FATAL_F(DS_APPLICATION_SDL_LOG_TAG, "Couldn't allocate window surface: %s",
				dsErrorString(errno));
			abort();
		}

		if (sdlWindow->hasFocus)
			dsSDLWindow_raise(application, window);

		dsEvent event;
		event.type = dsAppEventType_SurfaceInvalidated;
		dsApplication_dispatchEvent(application, window, &event);
	}
}

#if DS_ANDROID
static void invalidateWindowSurfaces(dsApplication* application)
{
	for (unsigned int i = 0; i < application->windowCount; ++i)
	{
		dsWindow* window = application->windows[i];
		const char* surfaceName = window->surface->name;
		dsRenderSurface_destroy(window->surface);
		window->surface = NULL;
		dsSDLWindow_createSurfaceInternal(window, surfaceName);

		dsEvent event;
		event.type = dsAppEventType_SurfaceInvalidated;
		dsApplication_dispatchEvent(application, window, &event);
	}
}
#endif

bool dsSDLApplication_addCustomEvent(dsApplication* application, dsWindow* window,
	const dsCustomEvent* event)
{
	DS_UNUSED(application);
	DS_ASSERT(event);
	SDL_Event userEvent;
	userEvent.type = SDL_USEREVENT;
	if (window)
		userEvent.user.windowID = SDL_GetWindowID(((dsSDLWindow*)window)->sdlWindow);
	else
		userEvent.user.windowID = 0;
	userEvent.user.code = event->eventID;
	userEvent.user.data1 = event->userData;
	userEvent.user.data2 = event->cleanupFunc;

	return SDL_PushEvent(&userEvent) != 0;
}

uint32_t dsSDLApplication_showMessageBoxBase(dsApplication* application,
	dsWindow* parentWindow, dsMessageBoxType type, const char* title, const char* message,
	const char** buttons, uint32_t buttonCount, uint32_t enterButton, uint32_t escapeButton)
{
	DS_UNUSED(application);
	SDL_Window* sdlWindow = parentWindow ? ((dsSDLWindow*)parentWindow)->sdlWindow : NULL;
	return showMessageBoxImpl(sdlWindow, type, title, message, buttons, buttonCount, enterButton,
		escapeButton);
}

int dsSDLApplication_run(dsApplication* application)
{
	dsTimer timer = dsTimer_create();
	dsSDLApplication* sdlApplication = (dsSDLApplication*)application;
	double lastTime = dsTimer_time(timer);
	while (!sdlApplication->quit && application->windowCount > 0)
	{
		double curTime = dsTimer_time(timer);
		double lastFrameTime = curTime - lastTime;
		lastTime = curTime;

		DS_VERIFY(dsRenderer_beginFrame(application->renderer));

		DS_PROFILE_SCOPE_START("Process Events");
		// Check if any size has changed.
		SDL_PumpEvents();
		for (uint32_t i = 0; i < application->windowCount; ++i)
		{
			dsWindow* window = application->windows[i];

			dsSDLWindow* sdlWindow = (dsSDLWindow*)window;
			uint32_t newWidth = sdlWindow->curWidth, newHeight = sdlWindow->curHeight;
			dsSDLWindow_getSize(&newWidth, &newHeight, application, window);

			// NOTE: Sometimes the surface resize doesn't correspond with the window resize event.
			dsRenderSurface_update(window->surface);

			// Sometimes the surface will be updated during rendering, so use the cached versions
			// for compare rather than the surface values before update.
			if (window->surface->width != sdlWindow->curSurfaceWidth ||
				window->surface->height != sdlWindow->curSurfaceHeight ||
				window->surface->rotation != sdlWindow->curSurfaceRotation)
			{
				sdlWindow->curWidth = newWidth;
				sdlWindow->curHeight = newHeight;
				sdlWindow->curSurfaceWidth = window->surface->width;
				sdlWindow->curSurfaceHeight = window->surface->height;
				sdlWindow->curSurfaceRotation = window->surface->rotation;

				dsEvent event;
				event.type = dsAppEventType_WindowResized;
				event.resize.width = window->surface->width;
				event.resize.height = window->surface->height;
				dsApplication_dispatchEvent(application, window, &event);
			}
		}

		SDL_Event sdlEvent;
		while (SDL_PollEvent(&sdlEvent))
		{
			dsWindow* window = findWindow(application, sdlEvent.window.windowID);
			dsEvent event;
			switch (sdlEvent.type)
			{
				case SDL_QUIT:
				case SDL_APP_TERMINATING:
					return sdlApplication->exitCode;
				case SDL_APP_WILLENTERBACKGROUND:
					event.type = dsAppEventType_WillEnterBackground;
					break;
				case SDL_APP_DIDENTERBACKGROUND:
					event.type = dsAppEventType_DidEnterBackground;
					break;
				case SDL_APP_WILLENTERFOREGROUND:
					event.type = dsAppEventType_WillEnterForeground;
					break;
				case SDL_APP_DIDENTERFOREGROUND:
					event.type = dsAppEventType_DidEnterForeground;
#if DS_ANDROID
					invalidateWindowSurfaces(application);
					// Make sure invalidated surfaces fully go through the GPU.
					dsRenderer_waitUntilIdle(application->renderer);
#endif
					break;
				case SDL_WINDOWEVENT:
				{
					if (!window)
						continue;

					switch (sdlEvent.window.event)
					{
						case SDL_WINDOWEVENT_SHOWN:
							event.type = dsAppEventType_WindowShown;
							break;
						case SDL_WINDOWEVENT_HIDDEN:
							event.type = dsAppEventType_WindowHidden;
							break;
						case SDL_WINDOWEVENT_MINIMIZED:
							event.type = dsAppEventType_WindowMinimized;
							break;
						case SDL_WINDOWEVENT_RESTORED:
							event.type = dsAppEventType_WindowRestored;
							break;
						case SDL_WINDOWEVENT_ENTER:
							event.type = dsAppEventType_MouseEntered;
							break;
						case SDL_WINDOWEVENT_LEAVE:
							event.type = dsAppEventType_MouseLeft;
							break;
						case SDL_WINDOWEVENT_FOCUS_GAINED:
							event.type = dsAppEventType_FocusGained;
							break;
						case SDL_WINDOWEVENT_FOCUS_LOST:
							event.type = dsAppEventType_FocusLost;
							break;
						case SDL_WINDOWEVENT_CLOSE:
							if (!window->closeFunc ||
								window->closeFunc(window, window->closeUserData))
							{
								event.type = dsAppEventType_WindowClosed;
								dsWindow_setHidden(window, true);
							}
							else
								continue;
							break;
						default:
							continue;
					}
					break;
				}
				case SDL_KEYDOWN:
				case SDL_KEYUP:
					event.type = sdlEvent.type == SDL_KEYDOWN ? dsAppEventType_KeyDown :
						dsAppEventType_KeyUp;
					event.key.key = dsFromSDLScancode(sdlEvent.key.keysym.scancode);
					event.key.modifiers = dsFromSDLKeyMod((SDL_Keymod)sdlEvent.key.keysym.mod);
					event.key.repeat = sdlEvent.key.repeat != 0;
					break;
				case SDL_TEXTEDITING:
				{
					event.type = dsAppEventType_TextEdit;
					event.textEdit.cursor = sdlEvent.edit.start;
					event.textEdit.selectionLength = sdlEvent.edit.length;
					_Static_assert(sizeof(event.textEdit.text) >= sizeof(sdlEvent.edit.text),
						"Invalid SDL text size.");
					memcpy(event.textEdit.text, sdlEvent.edit.text, sizeof(sdlEvent.edit.text));
					break;
				}
				case SDL_TEXTINPUT:
				{
					event.type = dsAppEventType_TextInput;
					_Static_assert(sizeof(event.textInput.text) >= sizeof(sdlEvent.text.text),
						"Invalid SDL text size.");
					memcpy(event.textInput.text, sdlEvent.text.text, sizeof(sdlEvent.text.text));
					break;
				}
				case SDL_MOUSEMOTION:
					if (sdlEvent.motion.which == SDL_TOUCH_MOUSEID)
						continue;

					event.type = dsAppEventType_MouseMove;
					event.mouseMove.x = sdlEvent.motion.x;
					event.mouseMove.y = sdlEvent.motion.y;
					event.mouseMove.deltaX = sdlEvent.motion.xrel;
					event.mouseMove.deltaY = sdlEvent.motion.yrel;
					break;
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					if (sdlEvent.button.which == SDL_TOUCH_MOUSEID)
						continue;

					event.type = sdlEvent.type == SDL_MOUSEBUTTONUP ? dsAppEventType_MouseButtonUp :
						dsAppEventType_MouseButtonDown;
					event.mouseButton.button = DS_MOUSE_BUTTON(sdlEvent.button.button);
					event.mouseButton.x = sdlEvent.button.x;
					event.mouseButton.y = sdlEvent.button.y;
					break;
				case SDL_MOUSEWHEEL:
					if (sdlEvent.wheel.which == SDL_TOUCH_MOUSEID)
						continue;

					event.type = dsAppEventType_MouseWheel;
					SDL_GetMouseState(&event.mouseWheel.x, &event.mouseWheel.y);
					if (window)
					{
						int windowX, windowY;
						SDL_GetWindowPosition(((dsSDLWindow*)window)->sdlWindow, &windowX,
							&windowY);
						event.mouseWheel.x -= windowX;
						event.mouseWheel.y -= windowY;
					}
					event.mouseWheel.deltaX = sdlEvent.wheel.x;
					event.mouseWheel.deltaY = sdlEvent.wheel.y;
					event.mouseWheel.yFlipped = sdlEvent.wheel.direction == SDL_MOUSEWHEEL_FLIPPED;
					break;
				case SDL_JOYAXISMOTION:
					event.type = dsAppEventType_ControllerAxis;
					event.controllerAxis.controller = findController(application,
						sdlEvent.jaxis.which);
					DS_ASSERT(event.controllerAxis.controller);
					event.controllerAxis.axis = sdlEvent.jaxis.axis;
					event.controllerAxis.value = dsSDLController_getAxisValue(sdlEvent.jaxis.value);
					break;
				case SDL_JOYBALLMOTION:
					event.type = dsAppEventType_JoystickBall;
					event.joystickBall.controller = findController(application,
						sdlEvent.jaxis.which);
					DS_ASSERT(event.joystickBall.controller);
					event.joystickBall.deltaX = sdlEvent.jball.xrel;
					event.joystickBall.deltaY = sdlEvent.jball.yrel;
					break;
				case SDL_JOYHATMOTION:
					event.type = dsAppEventType_JoystickHat;
					event.joystickHat.controller = findController(application,
						sdlEvent.jhat.which);
					DS_ASSERT(event.joystickHat.controller);
					dsSDLController_convertHatDirection(&event.joystickHat.x, &event.joystickHat.y,
						sdlEvent.jhat.value);
					break;
				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:
					event.type = sdlEvent.type == SDL_JOYBUTTONUP ? dsAppEventType_ControllerButtonUp :
						dsAppEventType_ControllerButtonDown;
					event.controllerButton.controller = findController(application,
						sdlEvent.jbutton.which);
					DS_ASSERT(event.controllerButton.controller);
					event.controllerButton.button = sdlEvent.jbutton.button;
					break;
				case SDL_JOYDEVICEADDED:
				{
					dsController* controller = dsSDLController_add(application,
						sdlEvent.jdevice.which);
					if (!controller)
					{
						DS_LOG_ERROR_F(DS_APPLICATION_SDL_LOG_TAG,
							"Couldn't add controller: %s", dsErrorString(errno));
						continue;
					}

					event.type = dsAppEventType_ControllerConnected;
					event.controllerConnect.controller = controller;
					break;
				}
				case SDL_JOYDEVICEREMOVED:
					event.type = dsAppEventType_ControllerDisconnected;
					event.controllerConnect.controller = findController(application,
						sdlEvent.jdevice.which);
					DS_ASSERT(event.controllerConnect.controller);
					break;
				case SDL_FINGERDOWN:
				case SDL_FINGERUP:
				case SDL_FINGERMOTION:
					switch (sdlEvent.type)
					{
						case SDL_FINGERDOWN:
							event.type = dsAppEventType_TouchFingerDown;
							break;
						case SDL_FINGERUP:
							event.type = dsAppEventType_TouchFingerUp;
							break;
						case SDL_FINGERMOTION:
							event.type = dsAppEventType_TouchMoved;
							break;
						default:
							DS_ASSERT(false);
							break;
					}
					event.touch.touchID = sdlEvent.tfinger.touchId;
					event.touch.fingerID = sdlEvent.tfinger.fingerId;
					event.touch.x = sdlEvent.tfinger.x;
					event.touch.y = sdlEvent.tfinger.y;
					event.touch.deltaX = sdlEvent.tfinger.dx;
					event.touch.deltaY = sdlEvent.tfinger.dy;
					event.touch.pressure = sdlEvent.tfinger.pressure;
					break;
				case SDL_MULTIGESTURE:
					event.type = dsAppEventType_MultiTouch;
					event.multiTouch.touchID = sdlEvent.mgesture.touchId;
					event.multiTouch.rotation = sdlEvent.mgesture.dTheta;
					event.multiTouch.pinch = sdlEvent.mgesture.dDist;
					event.multiTouch.x = sdlEvent.mgesture.x;
					event.multiTouch.y = sdlEvent.mgesture.y;
					event.multiTouch.fingerCount = sdlEvent.mgesture.numFingers;
					break;
				case SDL_USEREVENT:
					event.type = dsAppEventType_Custom;
					event.custom.eventID = sdlEvent.user.code;
					event.custom.userData = sdlEvent.user.data1;
					event.custom.cleanupFunc = (dsCustomEventCleanupFunction)sdlEvent.user.data2;
					break;
				default:
					continue;
			}

			dsApplication_dispatchEvent(application, window, &event);

			// Some events require cleanup.
			if (sdlEvent.type == SDL_JOYDEVICEREMOVED)
				DS_VERIFY(dsSDLController_remove(application, sdlEvent.jdevice.which));
			else if (sdlEvent.type == SDL_USEREVENT && sdlEvent.user.data2)
			{
				((dsCustomEventCleanupFunction)sdlEvent.user.data2)(sdlEvent.user.code,
					sdlEvent.user.data1);
			}
		}
		DS_PROFILE_SCOPE_END();

		if (application->updateFunc)
		{
			DS_PROFILE_SCOPE_START("Update");
			application->updateFunc(application, lastFrameTime, application->updateUserData);
			DS_PROFILE_SCOPE_END();
		}

		// If the samples have changed, need to re-create the windows. Do between update and draw
		// since update is most likely to have changed the samples.
		updateWindowSamples(application);

		DS_PROFILE_SCOPE_START("Draw");
		uint32_t swapSurfaceCount = 0;
		dsRenderSurface* swapSurfaces[DS_MAX_WINDOWS];
		if (application->windowCount > DS_MAX_WINDOWS)
		{
			DS_LOG_ERROR_F(DS_APPLICATION_SDL_LOG_TAG, "A maximum of %u windows is supported.",
				DS_MAX_WINDOWS);
			abort();
		}

		dsCommandBuffer* commandBuffer = application->renderer->mainCommandBuffer;
		for (uint32_t i = 0; i < application->windowCount; ++i)
		{
			dsWindow* window = application->windows[i];
			if (!window->drawFunc && !window->surface)
				continue;

			if (dsRenderSurface_beginDraw(window->surface, commandBuffer))
			{
				window->drawFunc(application, window, window->drawUserData);
				dsRenderSurface_endDraw(window->surface, commandBuffer);
				swapSurfaces[swapSurfaceCount++] = window->surface;
			}

			// Flush between windows. This avoids render commands for multiple windows being batched
			// together, increasing the amount of GPU synchronization.
			if (i > 0 && i < application->windowCount - 1)
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

		DS_VERIFY(dsRenderer_endFrame(application->renderer));
	}

	return sdlApplication->exitCode;
}

void dsSDLApplication_quit(dsApplication* application, int exitCode)
{
	dsSDLApplication* sdlApplication = (dsSDLApplication*)application;
	sdlApplication->quit = true;
	sdlApplication->exitCode = exitCode;
}

void dsSDLApplication_getDisplayBounds(dsAlignedBox2i* outBounds, const dsApplication* application,
	uint32_t display)
{
	DS_UNUSED(application);
	SDL_Rect rect = {0, 0, 0, 0};
	SDL_GetDisplayBounds(display, &rect);
	outBounds->min.x = rect.x;
	outBounds->min.y = rect.y;
	outBounds->max.x = rect.x + rect.w;
	outBounds->max.y = rect.y + rect.h;
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
	return SDL_ShowCursor(-1);
}

bool dsSDLApplication_setCursorHidden(dsApplication* application, bool hidden)
{
	DS_UNUSED(application);
	SDL_ShowCursor(hidden);
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

bool dsSDLApplication_beginTextInput(dsApplication* application)
{
	DS_UNUSED(application);
	SDL_StartTextInput();
	return true;
}

bool dsSDLApplication_endTextInput(dsApplication* application)
{
	DS_UNUSED(application);
	SDL_StopTextInput();
	return true;
}

bool dsSDLApplication_setTextInputRect(dsApplication* application, const dsAlignedBox2i* bounds)
{
	DS_UNUSED(application);
	SDL_Rect rect = {bounds->min.x, bounds->min.y, bounds->max.x -bounds->max.x,
		bounds->max.y - bounds->min.y};
	SDL_SetTextInputRect(&rect);
	return true;
}

bool dsSDLApplication_getMousePosition(dsVector2i* outPosition, const dsApplication* application)
{
	DS_UNUSED(application);
	SDL_GetMouseState(&outPosition->x, &outPosition->y);
	return true;
}

bool dsSDLApplication_setMousePosition(dsApplication* application, dsWindow* window,
	const dsVector2i* position)
{
	DS_UNUSED(application);
	if (window)
		SDL_WarpMouseInWindow(((dsSDLWindow*)window)->sdlWindow, position->x, position->y);
	else
	{
		if (SDL_WarpMouseGlobal(position->x, position->y) != 0)
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
	return SDL_GetMouseState(NULL, NULL);
}

uint32_t dsSDLApplication_showMessageBox(dsMessageBoxType type, const char* title,
	const char* message, const char** buttons, uint32_t buttonCount, uint32_t enterButton,
	uint32_t escapeButton)
{
	if (!title || !message || !buttons || buttonCount == 0 ||
		(enterButton != DS_MESSAGE_BOX_NO_BUTTON && enterButton >= buttonCount) ||
		(escapeButton != DS_MESSAGE_BOX_NO_BUTTON && escapeButton >= buttonCount))
	{
		errno = EINVAL;
		return DS_MESSAGE_BOX_NO_BUTTON;
	}

	return showMessageBoxImpl(NULL, type, title, message, buttons, buttonCount, enterButton,
		escapeButton);
}

dsApplication* dsSDLApplication_create(dsAllocator* allocator, dsRenderer* renderer, int argc,
	const char** argv, const char* orgName, const char* appName)
{
	DS_UNUSED(argc);
	DS_UNUSED(argv);
	if (!allocator || !renderer)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_APPLICATION_SDL_LOG_TAG,
			"Application allocator must support freeing memory.");
		return NULL;
	}

	// When available, tell SDL we are using an external context. This should be guaranteed to be
	// available in the pre-built libraries for platforms it's required for.
#ifdef SDL_HINT_VIDEO_EXTERNAL_CONTEXT
	SDL_SetHint(SDL_HINT_VIDEO_EXTERNAL_CONTEXT, "1");
#endif

	if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC) != 0)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_APPLICATION_SDL_LOG_TAG, "Couldn't initialize SDL: %s", SDL_GetError());
		return NULL;
	}

	SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "1");
	const char* driver = NULL;
#if DS_LINUX && !DS_ANDROID
	if (renderer->platform == dsGfxPlatform_Wayland)
		driver = "wayland";
	else
	{
		setenv("SDL_VIDEO_X11_NODIRECTCOLOR", "1", true);

		// NOTE: Bypassing the compositor can wreck havok. Vulkan on KDE is especially bad, making
		// the whole system pretty much unusable.
		setenv("SDL_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR", "0", true);

		if (renderer->surfaceConfig)
		{
			char visualId[20];
			snprintf(visualId, sizeof(visualId), "%d", (int)(size_t)renderer->surfaceConfig);
			setenv("SDL_VIDEO_X11_VISUALID", visualId, true);
		}
		driver = "x11";
	}
#elif DS_WINDOWS
	driver = "windows";
#elif DS_MAC
	driver = "cocoa";
#endif
	if (SDL_VideoInit(driver) != 0)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_APPLICATION_SDL_LOG_TAG, "Couldn't initialize SDL video: %s",
			SDL_GetError());
		SDL_Quit();
		return NULL;
	}
	dsRenderer_restoreGlobalState(renderer);

	if (renderer->rendererID == DS_GL_RENDERER_ID ||
		renderer->rendererID == DS_GLES_RENDERER_ID)
	{
		if (!setGLAttributes(renderer))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_APPLICATION_SDL_LOG_TAG, "Invalid renderer attributes.");
			SDL_Quit();
			return NULL;
		}
	}

	dsSDLApplication* application = DS_ALLOCATE_OBJECT(allocator, dsSDLApplication);
	if (!application)
		return NULL;

	application->quit = false;
	application->exitCode = 0;
	memset(application->cursors, 0, sizeof(application->cursors));

	dsApplication* baseApplication = (dsApplication*)application;
	DS_VERIFY(dsApplication_initialize(baseApplication));
	baseApplication->renderer = renderer;
	baseApplication->allocator = allocator;

	baseApplication->displayCount = SDL_GetNumVideoDisplays();
	if (baseApplication->displayCount > 0)
	{
		dsDisplayInfo* displays = DS_ALLOCATE_OBJECT_ARRAY(allocator, dsDisplayInfo,
			baseApplication->displayCount);
		if (!displays)
		{
			dsSDLApplication_destroy(baseApplication);
			return NULL;
		}

		memset(displays, 0, sizeof(dsDisplayInfo)*baseApplication->displayCount);
		baseApplication->displays = displays;
		for (uint32_t i = 0; i < baseApplication->displayCount; ++i)
		{
			displays[i].name = SDL_GetDisplayName(i);

			SDL_DisplayMode defaultMode;
			DS_VERIFY(SDL_GetDesktopDisplayMode(i, &defaultMode) == 0);
			uint32_t displayModeTotal = SDL_GetNumDisplayModes(i);
			uint32_t displayModeCount = 0;
			for (uint32_t j = 0; j < displayModeTotal; ++j)
			{
				SDL_DisplayMode mode;
				DS_VERIFY(SDL_GetDisplayMode(i, j, &mode) == 0);
				if (mode.format == defaultMode.format)
					++displayModeCount;
			}

			displays[i].displayModeCount = displayModeCount;
			if (displays[i].displayModeCount > 0)
			{
				dsDisplayMode* displayModes = DS_ALLOCATE_OBJECT_ARRAY(allocator, dsDisplayMode,
					displays[i].displayModeCount);
				if (!displayModes)
				{
					dsSDLApplication_destroy(baseApplication);
					return NULL;
				}

				displays[i].displayModes = displayModes;
				displays[i].defaultMode = displayModeCount;
				uint32_t curIndex = 0;
				for (uint32_t j = 0; j < displayModeTotal; ++j)
				{
					SDL_DisplayMode mode;
					DS_VERIFY(SDL_GetDisplayMode(i, j, &mode) == 0);
					if (mode.format != defaultMode.format)
						continue;

					displayModes[curIndex].displayIndex = i;
					displayModes[curIndex].width = mode.w;
					displayModes[curIndex].height = mode.h;
					displayModes[curIndex].refreshRate = mode.refresh_rate;

					if (mode.format == defaultMode.format && mode.w == defaultMode.w &&
						mode.h == defaultMode.h && mode.refresh_rate == defaultMode.refresh_rate)
					{
						displays[i].defaultMode = curIndex;
					}

					++curIndex;
				}
				DS_ASSERT(curIndex == displayModeCount);
				DS_ASSERT(displays[i].defaultMode < displayModeCount);
			}

#if SDL_VERSION_ATLEAST(2, 0, 4)
			if (SDL_GetDisplayDPI(i, NULL, &displays[i].dpi, NULL) != 0)
#endif
				displays[i].dpi = DS_DEFAULT_DPI;
		}
	}

	application->cursors[dsCursor_Arrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	application->cursors[dsCursor_IBeam] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	application->cursors[dsCursor_Wait] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
	application->cursors[dsCursor_Crosshair] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	application->cursors[dsCursor_WaitArrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
	application->cursors[dsCursor_SizeTLBR] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
	application->cursors[dsCursor_SizeTRBL] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	application->cursors[dsCursor_SizeTB] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	application->cursors[dsCursor_SizeLR] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	application->cursors[dsCursor_SizeAll] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
	application->cursors[dsCursor_No] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
	application->cursors[dsCursor_Hand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	application->curCursor = dsCursor_Arrow;

	if (!dsSDLController_setup(baseApplication))
	{
		dsSDLApplication_destroy(baseApplication);
		return NULL;
	}

	baseApplication->addCustomEventFunc = &dsSDLApplication_addCustomEvent;
	baseApplication->showMessageBoxFunc = &dsSDLApplication_showMessageBoxBase;
	baseApplication->runFunc = &dsSDLApplication_run;
	baseApplication->quitFunc = &dsSDLApplication_quit;

	baseApplication->getDisplayBoundsfunc = &dsSDLApplication_getDisplayBounds;
	baseApplication->getCursorFunc = &dsSDLApplication_getCursor;
	baseApplication->setCursorFunc = &dsSDLApplication_setCursor;
	baseApplication->getCursorHiddenFunc = &dsSDLApplication_getCursorHidden;
	baseApplication->setCursorHiddenFunc = &dsSDLApplication_setCursorHidden;
	baseApplication->isKeyPressedFunc = &dsSDLApplication_isKeyPressed;
	baseApplication->getKeyModifiersFunc = &dsSDLApplication_getKeyModifiers;
	baseApplication->beginTextInputFunc = &dsSDLApplication_beginTextInput;
	baseApplication->endTextInputFunc = &dsSDLApplication_endTextInput;
	baseApplication->setTextInputRectFunc = &dsSDLApplication_setTextInputRect;
	baseApplication->getMousePositionFunc = &dsSDLApplication_getMousePosition;
	baseApplication->setMousePositionFunc = &dsSDLApplication_setMousePosition;
	baseApplication->getPressedMouseButtonsFunc = &dsSDLApplication_getPressedMouseButtons;

	baseApplication->createWindowFunc = &dsSDLWindow_create;
	baseApplication->destroyWindowFunc = &dsSDLWindow_destroy;
	baseApplication->createWindowSurfaceFunc = &dsSDLWindow_createSurface;
	baseApplication->getFocusWindowFunc = &dsSDLWindow_getFocusWindow;
	baseApplication->setWindowTitleFunc = &dsSDLWindow_setTitle;
	baseApplication->setWindowDisplayModeFunc = &dsSDLWindow_setDisplayMode;
	baseApplication->resizeWindowFunc = &dsSDLWindow_resize;
	baseApplication->getWindowSizeFunc = &dsSDLWindow_getSize;
	baseApplication->getWindowPixelSizeFunc = &dsSDLWindow_getPixelSize;
	baseApplication->setWindowStyleFunc = &dsSDLWindow_setStyle;
	baseApplication->getWindowPositionFunc = &dsSDLWindow_getPosition;
	baseApplication->getWindowHiddenFunc = &dsSDLWindow_getHidden;
	baseApplication->setWindowHiddenFunc = &dsSDLWindow_setHidden;
	baseApplication->getWindowMinimizedFunc = &dsSDLWindow_getMinimized;
	baseApplication->getWindowMaximizedFunc = &dsSDLWindow_getMaximized;
	baseApplication->minimizeWindowFunc = &dsSDLWindow_minimize;
	baseApplication->maximizeWindowFunc = &dsSDLWindow_maximize;
	baseApplication->restoreWindowFunc = &dsSDLWindow_restore;
	baseApplication->getWindowGrabbedInputFunc = &dsSDLWindow_getGrabbedInput;
	baseApplication->setWindowGrabbedInputFunc = &dsSDLWindow_setGrabbedInput;
	baseApplication->raiseWindowFunc = &dsSDLWindow_raise;

	baseApplication->getControllerAxisFunc = &dsSDLController_getAxis;
	baseApplication->isControllerButtonPressedFunc = &dsSDLController_isButtonPressed;
	baseApplication->getControllerHatDirectionFunc = &dsSDLController_getHatDirection;
	baseApplication->startControllerRumbleFunc = &dsSDLController_startRumble;
	baseApplication->stopControllerRumbleFunc = &dsSDLController_stopRumble;

#if DS_ANDROID
	DS_UNUSED(orgName);
	DS_UNUSED(appName);
	dsResourceStream_setContext(SDL_AndroidGetJNIEnv(), SDL_AndroidGetActivity(), "",
		SDL_AndroidGetInternalStoragePath(), SDL_AndroidGetExternalStoragePath());
#else
	char* basePath = SDL_GetBasePath();
	char* prefPath = SDL_GetPrefPath(orgName, appName);
	if (!prefPath)
	{
		DS_LOG_ERROR(DS_APPLICATION_SDL_LOG_TAG, "Couldn't create preference path.");
		SDL_free(basePath);
	}
	dsResourceStream_setContext(NULL, NULL, basePath, basePath, prefPath);
	SDL_free(basePath);
	SDL_free(prefPath);
#endif

	return baseApplication;
}

void dsSDLApplication_destroy(dsApplication* application)
{
	if (!application)
		return;

	if (application->displays)
	{
		for (uint32_t i = 0; i < application->displayCount; ++i)
		{
			DS_VERIFY(dsAllocator_free(application->allocator,
				(void*)application->displays[i].displayModes));
		}

		DS_VERIFY(dsAllocator_free(application->allocator, (void*)application->displays));
	}

	dsSDLApplication* sdlApplication = (dsSDLApplication*)application;
	for (int i = 0; i < dsCursor_Count; ++i)
	{
		if (sdlApplication->cursors[i])
			SDL_FreeCursor(sdlApplication->cursors[i]);
	}

	dsSDLController_freeAll(application->controllers, application->controllerCount);
	dsApplication_shutdown(application);
	dsAllocator_free(application->allocator, application);

	SDL_VideoQuit();
	SDL_Quit();
}
