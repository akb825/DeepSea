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

#if ANYGL_HAS_WGL

void* dsGetWGLDisplay(void* osDisplay);
void dsReleaseWGLDisplay(void* osDisplay, void* gfxDisplay);
void* dsCreateWGLConfig(dsAllocator* allocator, void* display, const dsRendererOptions* options,
	GLContextType contextType);
void* dsGetPublicWGLConfig(void* display, void* config);
void dsDestroyWGLConfig(void* display, void* config);
void* dsCreateWGLContext(dsAllocator* allocator, void* display, void* config, void* shareContext);
void dsDestroyWGLContext(void* display, void* context);
void* dsCreateDummyWGLSurface(
	dsAllocator* allocator, void* display, void* config, void** osSurface);
void dsDestroyDummyWGLSurface(void* display, void* surface, void* osSurface);

void* dsCreateWGLSurface(dsAllocator* allocator, void* display, void* config,
	dsRenderSurfaceType surfaceType, void* handle);
bool dsGetWGLSurfaceSize(uint32_t* outWidth, uint32_t* outHeight, void* display,
	dsRenderSurfaceType surfaceType, void* surface);
void dsSwapWGLBuffers(void* display, dsRenderSurface** renderSurfaces, uint32_t count, bool vsync);
void dsDestroyWGLSurface(void* display, dsRenderSurfaceType surfaceType, void* surface);

bool dsBindWGLContext(void* display, void* context, void* surface);
void* dsGetCurrentWGLContext(void* display);

void dsSetWGLVSync(void* display, void* surface, bool vsync);

#endif // ANYGL_HAS_WGL
