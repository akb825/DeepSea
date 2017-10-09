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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Application/Types.h>
#include <SDL2/SDL.h>

#define DS_APPLICATION_SDL_LOG_TAG "sdl"

typedef struct dsSDLWindow
{
	dsWindow window;
	SDL_Window* sdlWindow;
	uint32_t samples;
} dsSDLWindow;

bool dsSDLWindow_createComponents(dsWindow* window, const char* title, const dsVector2i* position,
	uint32_t width, uint32_t height, unsigned int flags);

dsWindow* dsSDLWindow_create(dsApplication* application, dsAllocator* allocator,
	const char* title, const dsVector2i* position, uint32_t width, uint32_t height,
	unsigned int flags);
dsWindow* dsSDLWindow_getFocusWindow(const dsApplication* application);
bool dsSDLWindow_setTitle(dsApplication* application, dsWindow* window, const char* title);
bool dsSDLWindow_setDisplayMode(dsApplication* application, dsWindow* window,
	const dsDisplayMode* displayMode);
bool dsSDLWindow_resize(dsApplication* application, dsWindow* window, uint32_t width,
	uint32_t height);
bool dsSDLWindow_getSize(uint32_t* outWidth, uint32_t* outHeight, const dsApplication* application,
	const dsWindow* window);
bool dsSDLWindow_setStyle(dsApplication* application, dsWindow* window, dsWindowStyle style);
bool dsSDLWindow_getPosition(dsVector2i* outPosition, const dsApplication* application,
	const dsWindow* window);
bool dsSDLWindow_setPosition(dsApplication* application, dsWindow* window,
	const dsVector2i* position, bool center);
bool dsSDLWindow_getHidden(const dsApplication* application, const dsWindow* window);
bool dsSDLWindow_setHidden(dsApplication* application, dsWindow* window, bool hidden);
bool dsSDLWindow_getMinimized(const dsApplication* application, const dsWindow* window);
bool dsSDLWindow_getMaximized(const dsApplication* application, const dsWindow* window);
bool dsSDLWindow_minimize(dsApplication* application, dsWindow* window);
bool dsSDLWindow_maximize(dsApplication* application, dsWindow* window);
bool dsSDLWindow_restore(dsApplication* application, dsWindow* window);
bool dsSDLWindow_getGrabbedInput(const dsApplication* application,
	const dsWindow* window);
bool dsSDLWindow_setGrabbedInput(dsApplication* application, dsWindow* window, bool grab);
bool dsSDLWindow_raise(dsApplication* application, dsWindow* window);
bool dsSDLWindow_destroy(dsApplication* application, dsWindow* window);
