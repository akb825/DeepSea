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
#include "GLTypes.h"

void* dsGetGLDisplay(void);
void dsReleaseGLDisplay(void* display);
void* dsCreateGLConfig(dsAllocator* allocator, void* display, const dsRendererOptions* options,
	bool render);
void* dsGetPublicGLConfig(void* display, void* config);
void dsDestroyGLConfig(void* display, void* config);
void* dsCreateGLContext(dsAllocator* allocator, void* display, void* config, void* shareContext);
void dsDestroyGLContext(void* display, void* context);
void* dsCreateDummyGLSurface(dsAllocator* allocator, void* display, void* config, void** osSurface);
void dsDestroyDummyGLSurface(void* display, void* surface, void* osSurface);

void* dsCreateGLSurface(dsAllocator* allocator, void* display, void* config,
	dsRenderSurfaceType surfaceType, void* handle);
bool dsGetGLSurfaceSize(uint32_t* outWidth, uint32_t* outHeight, void* display,
	dsRenderSurfaceType surfaceType, void* surface);
void dsSwapGLBuffers(void* display, dsRenderSurface** renderSurfaces, uint32_t count, bool vsync);
void dsDestroyGLSurface(void* display, dsRenderSurfaceType surfaceType, void* surface);

bool dsBindGLContext(void* display, void* context, void* surface);
void* dsGetCurrentGLContext(void* display);
