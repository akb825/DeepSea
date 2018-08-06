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

#include "MockRenderSurface.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/RenderMock/Export.h>

DS_RENDERMOCK_EXPORT bool dsMockRenderSurface_changeSize;

dsRenderSurface* dsMockRenderSurface_create(dsRenderer* renderer,
	dsAllocator* allocator, void* osHandle, dsRenderSurfaceType type, const char* name)
{
	DS_ASSERT(renderer);
	DS_ASSERT(allocator);
	DS_UNUSED(osHandle);

	dsRenderSurface* renderSurface = DS_ALLOCATE_OBJECT(allocator, dsRenderSurface);
	if (!renderSurface)
		return NULL;

	renderSurface->renderer = renderer;
	renderSurface->allocator = dsAllocator_keepPointer(allocator);
	renderSurface->name = name;
	renderSurface->surfaceType = type;
	renderSurface->width = 1920;
	renderSurface->height = 1080;
	return renderSurface;
}

bool dsMockRenderSurface_update(dsRenderer* renderer, dsRenderSurface* renderSurface)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(renderSurface);

	if (dsMockRenderSurface_changeSize)
	{
		renderSurface->width -= 10;
		renderSurface->height -= 10;
		return true;
	}

	return false;
}

bool dsMockRenderSurface_beginDraw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderSurface* renderSurface)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(renderSurface);
	DS_UNUSED(renderSurface);
	return true;
}

bool dsMockRenderSurface_endDraw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderSurface* renderSurface)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(renderSurface);
	DS_UNUSED(renderSurface);
	return true;
}

bool dsMockRenderSurface_swapBuffers(dsRenderer* renderer, dsRenderSurface** renderSurfaces,
	size_t count)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(renderSurfaces);
	DS_UNUSED(renderSurfaces);
	DS_ASSERT(count > 0);
	DS_UNUSED(count);
	return true;
}

bool dsMockRenderSurface_destroy(dsRenderer* renderer, dsRenderSurface* renderSurface)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(renderSurface);

	if (renderSurface->allocator)
		return dsAllocator_free(renderSurface->allocator, renderSurface);
	return true;
}
