/*
 * Copyright 2017-2025 Aaron Barany
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
#include "AnyGL/AnyGLConfig.h"
#include "GLTypes.h"

#if ANYGL_HAS_GLX

void* dsGetGLXDisplay(void* osDisplay);
void dsReleaseGLXDisplay(void* osDisplay, void* gfxDisplay);
void* dsCreateGLXConfig(dsAllocator* allocator, void* display, const dsRendererOptions* options,
	GLContextType contextType);
void* dsGetPublicGLXConfig(void* display, void* config);
void dsDestroyGLXConfig(void* display, void* config);
void* dsCreateGLXContext(dsAllocator* allocator, void* display, void* config, void* shareContext);
void dsDestroyGLXContext(void* display, void* context);
void* dsCreateDummyGLXSurface(
	dsAllocator* allocator, void* display, void* config, void** osSurface);
void dsDestroyDummyGLXSurface(void* display, void* surface, void* osSurface);

void* dsCreateGLXSurface(dsAllocator* allocator, void* display, void* config,
	dsRenderSurfaceType surfaceType, void* handle);
bool dsGetGLXSurfaceSize(uint32_t* outWidth, uint32_t* outHeight, void* display,
	dsRenderSurfaceType surfaceType, void* surface);
void dsSwapGLXBuffers(void* display, dsRenderSurface** renderSurfaces, uint32_t count, bool vsync);
void dsDestroyGLXSurface(void* display, dsRenderSurfaceType surfaceType, void* surface);

bool dsBindGLXContext(void* display, void* context, void* surface);
void* dsGetCurrentGLXContext(void* display);

void dsSetGLXVSync(void* display, void* surface, bool vsync);

#endif // ANYGL_HAS_GLX
