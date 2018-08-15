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
#include <DeepSea/Render/Types.h>

dsRenderSurface* dsGLRenderSurface_create(dsRenderer* renderer, dsAllocator* allocator,
	const char* name, void* osHandle, dsRenderSurfaceType type);
bool dsGLRenderSurface_update(dsRenderer* renderer, dsRenderSurface* renderSurface);
bool dsGLRenderSurface_beginDraw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderSurface* renderSurface);
bool dsGLRenderSurface_endDraw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderSurface* renderSurface);
bool dsGLRenderSurface_swapBuffers(dsRenderer* renderer, dsRenderSurface** renderSurfaces,
	size_t count);
bool dsGLRenderSurface_destroy(dsRenderer* renderer, dsRenderSurface* renderSurface);
