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
#include "GLTypes.h"

#if DS_MAC

void* dsGetCocoaGLDisplay(void* osDisplay);
void dsReleaseCocoaGLDisplay(void* osDisplay, void* gfxDisplay);
void* dsCreateCocoaGLConfig(dsAllocator* allocator, void* display, const dsRendererOptions* options,
	bool render);
void* dsGetPublicCocoaGLConfig(void* display, void* config);
void dsDestroyCocoaGLConfig(void* display, void* config);
void* dsCreateCocoaGLContext(
	dsAllocator* allocator, void* display, void* config, void* shareContext);
void dsDestroyCocoaGLContext(void* display, void* context);
void* dsCreateDummyCocoaGLSurface(
	dsAllocator* allocator, void* display, void* config, void** osSurface);
void dsDestroyDummyCocoaGLSurface(void* display, void* surface, void* osSurface);

void* dsCreateCocoaGLSurface(dsAllocator* allocator, void* display, void* config,
	dsRenderSurfaceType surfaceType, void* handle);
bool dsGetCocoaGLSurfaceSize(uint32_t* outWidth, uint32_t* outHeight, void* display,
	dsRenderSurfaceType surfaceType, void* surface);
void dsSwapCocoaGLBuffers(
	void* display, dsRenderSurface** renderSurfaces, uint32_t count, bool vsync);
void dsDestroyCocoaGLSurface(void* display, dsRenderSurfaceType surfaceType, void* surface);

bool dsBindCocoaGLContext(void* display, void* context, void* surface);
void* dsGetCurrentCocoaGLContext(void* display);

void dsSetCocoaGLVSync(void* display, void* surface, bool vsync);

#endif // DS_MAC
