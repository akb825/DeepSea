/*
 * Copyright 2025 Aaron Barany
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

#if ANYGL_HAS_EGL

void* dsGetEGLDisplay(void* osDisplay);
void dsReleaseEGLDisplay(void* osDisplay, void* gfxDisplay);
void* dsCreateEGLConfig(dsAllocator* allocator, void* display, const dsRendererOptions* options,
	GLContextType contextType);
void* dsGetPublicEGLConfig(void* display, void* config);
void dsDestroyEGLConfig(void* display, void* config);
void* dsCreateEGLContext(dsAllocator* allocator, void* display, void* config, void* shareContext);
void dsDestroyEGLContext(void* display, void* context);
void* dsCreateDummyEGLSurface(
	dsAllocator* allocator, void* display, void* config, void** osSurface);
void dsDestroyDummyEGLSurface(void* display, void* surface, void* osSurface);

void* dsCreateEGLSurface(dsAllocator* allocator, void* display, void* config,
	dsRenderSurfaceType surfaceType, void* handle);
bool dsGetEGLSurfaceSize(uint32_t* outWidth, uint32_t* outHeight, void* display,
	dsRenderSurfaceType surfaceType, void* surface);
void dsSwapEGLBuffers(void* display, dsRenderSurface** renderSurfaces, uint32_t count, bool vsync);
void dsDestroyEGLSurface(void* display, dsRenderSurfaceType surfaceType, void* surface);

bool dsBindEGLContext(void* display, void* context, void* surface);
void* dsGetCurrentEGLContext(void* display);

void dsSetEGLVSync(void* display, void* surface, bool vsync);

#endif
