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

bool dsGLPlatform_initialize(dsGLPlatform* platform, int anyglLoad);

void* dsGLPlatform_getDisplay(const dsGLPlatform* platform, void* osDisplay);
void dsGLPlatform_releaseDisplay(const dsGLPlatform* platform, void* osDisplay, void* gfxDisplay);
void* dsGLPlatform_createConfig(const dsGLPlatform* platform, dsAllocator* allocator, void* display,
	const dsRendererOptions* options, bool render);
void* dsGLPlatform_getPublicConfig(const dsGLPlatform* platform, void* display, void* config);
void dsGLPlatform_destroyConfig(const dsGLPlatform* platform, void* display, void* config);
void* dsGLPlatform_createContext(const dsGLPlatform* platform, dsAllocator* allocator,
	void* display, void* config, void* shareContext);
void dsGLPlatform_destroyContext(const dsGLPlatform* platform, void* display, void* context);
void* dsGLPlatform_createDummySurface(const dsGLPlatform* platform, dsAllocator* allocator,
	void* display, const dsRendererOptions* options, void* config, void** osSurface);
void dsGLPlatform_destroyDummySurface(
	const dsGLPlatform* platform, void* display, const dsRendererOptions* options, void* surface,
	void* osSurface);

void* dsGLPlatform_createSurface(const dsGLPlatform* platform, dsAllocator* allocator,
	void* display, void* config, dsRenderSurfaceType surfaceType, void* handle);
bool dsGLPlatform_getSurfaceSize(uint32_t* outWidth, uint32_t* outHeight,
	const dsGLPlatform* platform, void* display, dsRenderSurfaceType surfaceType, void* surface);
void dsGLPlatform_swapBuffers(const dsGLPlatform* platform, void* display,
	dsRenderSurface** renderSurfaces, uint32_t count, bool vsync);
void dsGLPlatform_destroySurface(
	const dsGLPlatform* platform, void* display, dsRenderSurfaceType surfaceType, void* surface);

bool dsGLPlatform_bindContext(
	const dsGLPlatform* platform, void* display, void* context, void* surface);
void* dsGLPlatform_getCurrentContext(const dsGLPlatform* platform, void* display);

void dsGLPlatform_setVSync(const dsGLPlatform* platform, void* display, void* surface, bool vsync);
