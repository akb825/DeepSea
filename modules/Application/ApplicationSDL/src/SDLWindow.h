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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Application/Types.h>

#include <SDL3/SDL.h>

typedef struct dsSDLWindow
{
	dsWindow window;
	const char* surfaceName;
	SDL_Window* sdlWindow;
	uint32_t samples;
	uint32_t curSurfaceWidth;
	uint32_t curSurfaceHeight;
	dsRenderSurfaceRotation curSurfaceRotation;
	dsRenderSurfaceUsage renderSurfaceUsage;
} dsSDLWindow;

bool dsSDLWindow_createComponents(dsWindow* window, const dsVector2i* position, uint32_t width,
	uint32_t height, dsWindowFlags flags);
void dsSDLWindow_destroyComponents(dsWindow* window);
bool dsSDLWindow_createSurfaceInternal(dsWindow* window);

dsWindow* dsSDLWindow_create(dsApplication* application, dsAllocator* allocator,
	const char* title, const char* surfaceName, const dsWindowInitPosition* position,
	uint32_t width, uint32_t height, dsWindowFlags flags, dsRenderSurfaceUsage renderSurfaceUsage);
bool dsSDLWindow_createSurface(dsApplication* application, dsWindow* window);
dsWindow* dsSDLWindow_getFocusWindow(const dsApplication* application);
bool dsSDLWindow_setTitle(dsApplication* application, dsWindow* window, const char* title);
bool dsSDLWindow_setDisplayMode(
	dsApplication* application, dsWindow* window, const dsDisplayMode* displayMode);
bool dsSDLWindow_resize(
	dsApplication* application, dsWindow* window, uint32_t width, uint32_t height);
bool dsSDLWindow_setStyle(dsApplication* application, dsWindow* window, dsWindowStyle style);
bool dsSDLWindow_setPosition(
	dsApplication* application, dsWindow* window, const dsVector2i* position);
bool dsSDLWindow_center(dsApplication* application, dsWindow* window, const dsDisplayInfo* display);
bool dsSDLWindow_setHidden(dsApplication* application, dsWindow* window, bool hidden);
bool dsSDLWindow_minimize(dsApplication* application, dsWindow* window);
bool dsSDLWindow_maximize(dsApplication* application, dsWindow* window);
bool dsSDLWindow_restore(dsApplication* application, dsWindow* window);
bool dsSDLWindow_setGrabbedInput(dsApplication* application, dsWindow* window, bool grab);
bool dsSDLWindow_setResizable(dsApplication* application, dsWindow* window, bool resizable);
bool dsSDLWindow_raise(dsApplication* application, dsWindow* window);
bool dsSDLWindow_beginTextInput(dsApplication* applicatin, dsWindow* window,
	dsWindowTextInputType inputType, dsWindowTextInputFlags inputFlags);
bool dsSDLWindow_endTextInput(dsApplication* applicatin, dsWindow* window);
bool dsSDLWindow_setTextInputArea(dsApplication* applicatin, dsWindow* window,
	const dsAlignedBox2i* bounds, uint32_t cursorOffset);
bool dsSDLWindow_destroy(dsApplication* application, dsWindow* window);
